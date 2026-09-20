#include "java/JavaNetwork.h"

#if defined(PS2_PLATFORM) && defined(PS2_ENABLE_NETWORK)

#include "ps2/network/Ps2Network.h"
#include "platform/Log.h"
#include "platform/Mutex.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <istream>
#include <mutex>
#include <ostream>
#include <streambuf>

namespace JavaNetwork
{
namespace
{
class Ps2Socket final : public Socket
{
public:
    ~Ps2Socket() override { releaseSocket(); }

    bool connect(const std::string &host, int port) override
    {
        releaseSocket();
        if (port < 1 || port > 65535 || host.empty())
            return false;

        MC_LOG_INFO("network", "[PS2] socket request %s:%d\n", host.c_str(), port);
        McLog::flush();
        if (!Ps2Network::initialize())
        {
            MC_LOG_WARN("network", "[PS2] network initialization not ready\n");
            return false;
        }

        closing.store(false, std::memory_order_release);
        receivedBytes.store(0, std::memory_order_release);
        sentBytes.store(0, std::memory_order_release);
        remoteAddress = host + ":" + std::to_string(port);

        sockaddr_in target{};
        target.sin_family = AF_INET;
        target.sin_port = htons(static_cast<unsigned short>(port));

        if (inet_aton(host.c_str(), &target.sin_addr) == 0)
        {
            hostent *resolved = gethostbyname(host.c_str());
            if (resolved == nullptr || resolved->h_addr_list == nullptr ||
                resolved->h_addr_list[0] == nullptr)
                return false;
            std::memcpy(&target.sin_addr, resolved->h_addr_list[0], sizeof(target.sin_addr));
        }

        const int newFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (newFd < 0)
            return false;

        // Player movement and interaction packets are small and latency-sensitive.
        // Disable Nagle so lwIP does not deliberately hold them while waiting for
        // another packet to coalesce. Failure is non-fatal on older SDK builds.
        const int noDelay = 1;
        (void)::setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));

        fd.store(newFd, std::memory_order_release);

        MC_LOG_INFO("network", "[PS2] TCP connect() fd=%d -> %s:%d\n", newFd, host.c_str(), port);
        McLog::flush();
        if (::connect(newFd, reinterpret_cast<sockaddr *>(&target), sizeof(target)) < 0)
        {
            const int errorCode = errno;
            MC_LOG_WARN("network", "[PS2] TCP connect failed errno=%d\n", errorCode);
            releaseSocket();
            return false;
        }
        MC_LOG_INFO("network", "[PS2] TCP connected to %s:%d\n", host.c_str(), port);
        McLog::flush();
        return true;
    }

    int read(char *buffer, int length) override
    {
        const int socketFd = fd.load(std::memory_order_acquire);
        if (socketFd < 0 || buffer == nullptr || length <= 0 ||
            closing.load(std::memory_order_acquire))
            return -1;

        const int count = static_cast<int>(::recv(socketFd, buffer, static_cast<std::size_t>(length), 0));
        if (count > 0)
            receivedBytes.fetch_add(static_cast<std::size_t>(count), std::memory_order_relaxed);
        return count;
    }

    bool write(const char *buffer, int length) override
    {
        if (buffer == nullptr)
            return false;
        if (length <= 0)
            return true;

        std::lock_guard<PlatformMutex> guard(writeLock);
        int offset = 0;
        while (offset < length)
        {
            const int socketFd = fd.load(std::memory_order_acquire);
            if (socketFd < 0 || closing.load(std::memory_order_acquire))
                return false;
            const int count = static_cast<int>(::send(socketFd, buffer + offset,
                                                      static_cast<std::size_t>(length - offset), 0));
            if (count <= 0)
                return false;
            sentBytes.fetch_add(static_cast<std::size_t>(count), std::memory_order_relaxed);
            offset += count;
        }
        return true;
    }

    bool flush() override
    {
        return fd.load(std::memory_order_acquire) >= 0 &&
               !closing.load(std::memory_order_acquire);
    }

    void interruptRead() override
    {
        const int socketFd = fd.load(std::memory_order_acquire);
        if (socketFd >= 0)
            ::shutdown(socketFd, SHUT_RD);
    }

    void close() override
    {
        closing.store(true, std::memory_order_release);
        const int socketFd = fd.load(std::memory_order_acquire);
        if (socketFd >= 0)
            ::shutdown(socketFd, SHUT_RDWR);
    }

    std::string getRemoteSocketAddress() const override { return remoteAddress; }
    std::size_t getReceivedByteCount() const override { return receivedBytes.load(std::memory_order_relaxed); }
    std::size_t getSentByteCount() const override { return sentBytes.load(std::memory_order_relaxed); }

private:
    void releaseSocket()
    {
        closing.store(true, std::memory_order_release);
        const int socketFd = fd.exchange(-1, std::memory_order_acq_rel);
        if (socketFd >= 0)
        {
            ::shutdown(socketFd, SHUT_RDWR);
            ::close(socketFd);
        }
    }

    std::atomic<int> fd{-1};
    std::atomic_bool closing{true};
    std::atomic<std::size_t> receivedBytes{0};
    std::atomic<std::size_t> sentBytes{0};
    PlatformMutex writeLock;
    std::string remoteAddress;
};

class SocketInputBuffer final : public std::streambuf
{
public:
    explicit SocketInputBuffer(Socket &value) : socket(value) { setg(buffer, buffer, buffer); }

protected:
    int_type underflow() override
    {
        if (gptr() < egptr())
            return traits_type::to_int_type(*gptr());
        const int count = socket.read(buffer, sizeof(buffer));
        if (count <= 0)
            return traits_type::eof();
        setg(buffer, buffer, buffer + count);
        return traits_type::to_int_type(*gptr());
    }

private:
    Socket &socket;
    char buffer[1024];
};

class SocketOutputBuffer final : public std::streambuf
{
public:
    explicit SocketOutputBuffer(Socket &value) : socket(value) { setp(buffer, buffer + sizeof(buffer)); }
    ~SocketOutputBuffer() override { sync(); }

protected:
    std::streamsize xsputn(const char *data, std::streamsize length) override
    {
        std::streamsize written = 0;
        while (written < length)
        {
            std::streamsize space = epptr() - pptr();
            if (space == 0)
            {
                if (!flushBuffer())
                    return written;
                space = epptr() - pptr();
            }
            const std::streamsize remaining = length - written;
            const std::streamsize count = remaining < space ? remaining : space;
            std::memcpy(pptr(), data + written, static_cast<std::size_t>(count));
            pbump(static_cast<int>(count));
            written += count;
        }
        return written;
    }

    int_type overflow(int_type value) override
    {
        if (traits_type::eq_int_type(value, traits_type::eof()))
            return traits_type::not_eof(value);
        if (!flushBuffer())
            return traits_type::eof();
        *pptr() = traits_type::to_char_type(value);
        pbump(1);
        return value;
    }

    int sync() override
    {
        return flushBuffer() && socket.flush() ? 0 : -1;
    }

private:
    bool flushBuffer()
    {
        const std::streamsize count = pptr() - pbase();
        if (count > 0 && !socket.write(pbase(), static_cast<int>(count)))
            return false;
        pbump(-static_cast<int>(count));
        return true;
    }

    Socket &socket;
    // Large enough to coalesce packet headers/body writes, small enough to keep
    // two network streams cheap in the PS2's 32 MB main RAM.
    char buffer[4096];
};

class SocketInputStream final : public std::istream
{
public:
    explicit SocketInputStream(Socket &socket) : std::istream(nullptr), buffer(socket) { rdbuf(&buffer); }
private:
    SocketInputBuffer buffer;
};

class SocketOutputStream final : public std::ostream
{
public:
    explicit SocketOutputStream(Socket &socket) : std::ostream(nullptr), buffer(socket) { rdbuf(&buffer); }
private:
    SocketOutputBuffer buffer;
};
}

std::unique_ptr<Socket> createSocket() { return std::make_unique<Ps2Socket>(); }
std::unique_ptr<std::istream> createInputStream(Socket &socket) { return std::make_unique<SocketInputStream>(socket); }
std::unique_ptr<std::ostream> createOutputStream(Socket &socket) { return std::make_unique<SocketOutputStream>(socket); }

// The PS2 multiplayer path only needs raw TCP. Keep HTTP/HTTPS disabled so
// enabling multiplayer does not re-enable desktop resource/auth traffic.
bool readUrl(const std::string &, std::vector<unsigned char> &) { return false; }
int getResponseCode(const std::string &) { return -1; }
bool postUrl(const std::string &, const std::string &, const std::string &,
             std::vector<unsigned char> &) { return false; }
}

#endif
