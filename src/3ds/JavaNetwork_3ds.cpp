#include "java/JavaNetwork.h"

#if defined(CTR_PLATFORM) && defined(CTR_ENABLE_NETWORK)

#include "3ds/DsNetwork.h"
#include "platform/Log.h"
#include "platform/Mutex.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <istream>
#include <memory>
#include <mutex>
#include <ostream>
#include <streambuf>
#include <string>
#include <thread>

namespace JavaNetwork
{
namespace
{

// libctru's connect() is one synchronous kernel IPC: it returns only once the
// SOC service finishes the TCP handshake or gives up, and neither fcntl nor
// FIONBIO shortens it -- there is no non-blocking branch to poll, so the
// Wii's "connect() + select(4 s)" pattern has no EINPROGRESS to wait on.
// Two callers would otherwise absorb the kernel's whole SYN-retry window:
// GuiConnecting's destructor join()s the connect thread when the player
// cancels (the UI would sit in that join until the kernel gives up), and a
// server-list poll would hold its worker just as long. Run the whole
// blocking attempt -- literal-IP check, DNS, socket(), connect() -- on a
// helper thread and cap the owner's wait at the same 4 seconds the Wii
// enforces. The helper owns the descriptor it creates until the hand-off: on
// timeout the owner marks the attempt abandoned and the helper closes the
// socket whenever the kernel answers, so a late success can neither leak an
// fd nor hand a recycled descriptor to a caller that already gave up.
constexpr int connectTimeoutSeconds = 4;

struct ConnectAttempt
{
	std::mutex mutex;
	std::condition_variable condition;
	bool finished = false;
	bool abandoned = false;
	int fd = -1;
	int error = 0;
};

int openBlockingConnection(const std::string &host, int port)
{
	sockaddr_in target{};
	target.sin_family = AF_INET;
	target.sin_port = htons(static_cast<unsigned short>(port));

	// Literal IPs first: the SOC service can spend seconds in DNS for a name
	// that will never resolve, and a raw address is the common case.
	if (inet_aton(host.c_str(), &target.sin_addr) == 0)
	{
		hostent *resolved = gethostbyname(host.c_str());
		if (resolved == nullptr || resolved->h_addr_list == nullptr ||
		    resolved->h_addr_list[0] == nullptr)
		{
			// No DNS-specific code exists in newlib; report the closest one so
			// the owner's log line still says why the attempt never started.
			errno = EHOSTUNREACH;
			return -1;
		}
		std::memcpy(&target.sin_addr, resolved->h_addr_list[0], sizeof(target.sin_addr));
	}

	const int newFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newFd < 0)
		return -1;

	// Player movement and interaction packets are small and latency-sensitive:
	// disable Nagle so the stack does not deliberately hold them waiting for
	// a coalescing partner, same as the PS2 backend. Non-fatal.
	const int noDelay = 1;
	(void)::setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));

	// Bounded, interruptible sends (see DsSocket::write, which replaced the
	// original unbounded blocking send: a stalled peer must fail the write
	// instead of wedging the write thread forever, or the outbound pipe dies
	// with no code ever noticing -- blocks come back and mobs ignore you).
	(void)0;

	if (::connect(newFd, reinterpret_cast<sockaddr *>(&target), sizeof(target)) < 0)
	{
		const int errorCode = errno;
		::close(newFd);
		errno = errorCode;
		return -1;
	}
	return newFd;
}

// Runs on the detached helper thread. The outcome is published under the
// attempt's mutex, so it races neither with the owner's timeout decision nor
// with an abandoned descriptor being closed behind the owner's back.
void runConnectAttempt(const std::shared_ptr<ConnectAttempt> &attempt,
                       const std::string &host, int port)
{
	const int newFd = openBlockingConnection(host, port);
	const int errorCode = newFd < 0 ? errno : 0;
	int closeLate = -1;
	{
		std::lock_guard<std::mutex> guard(attempt->mutex);
		if (attempt->abandoned)
			closeLate = newFd;
		else
		{
			attempt->fd = newFd;
			attempt->error = errorCode;
			attempt->finished = true;
		}
	}
	if (closeLate >= 0)
	{
		::shutdown(closeLate, SHUT_RDWR);
		::close(closeLate);
	}
	attempt->condition.notify_one();
}

class DsSocket final : public Socket
{
public:
	~DsSocket() override { releaseSocket(); }

	bool connect(const std::string &host, int port) override
	{
		releaseSocket();
		closing.store(false, std::memory_order_release);
		readInterrupted.store(false, std::memory_order_release);
		receivedBytes.store(0, std::memory_order_release);
		sentBytes.store(0, std::memory_order_release);
		remoteAddress = host + ":" + std::to_string(port);
		if (host.empty() || port < 1 || port > 65535 || !DsNetwork::initialize())
			return false;

		MC_LOG_INFO("network", "[3DS] TCP connect: %s\n", remoteAddress.c_str());
		McLog::flush();

		const std::shared_ptr<ConnectAttempt> attempt = std::make_shared<ConnectAttempt>();
		// Detached: the helper can outlive this call by the kernel's retry
		// window (see the comment above connectTimeoutSeconds). It keeps the
		// attempt state alive through the shared_ptr std::thread copies, and
		// libctru frees a detached thread's own stack when it exits
		// (threadExit() tears down thread->detached threads with threadFree).
		std::thread(&runConnectAttempt, attempt, host, port).detach();

		std::unique_lock<std::mutex> lock(attempt->mutex);
		const bool finishedInTime = attempt->condition.wait_for(
			lock, std::chrono::seconds(connectTimeoutSeconds),
			[&attempt] { return attempt->finished; });
		if (!finishedInTime)
		{
			// The kernel still owns the attempt; the helper closes the fd if
			// it ever answers.
			attempt->abandoned = true;
			lock.unlock();
			MC_LOG_WARN("network", "[3DS] TCP connect timed out: %s\n",
			            remoteAddress.c_str());
			releaseSocket();
			return false;
		}
		const int connectedFd = attempt->fd;
		const int errorCode = attempt->error;
		lock.unlock();

		if (connectedFd < 0)
		{
			MC_LOG_WARN("network", "[3DS] TCP connect failed: %s (errno=%d)\n",
			            remoteAddress.c_str(), errorCode);
			releaseSocket();
			return false;
		}

		fd.store(connectedFd, std::memory_order_release);
		MC_LOG_INFO("network", "[3DS] TCP connected: %s\n", remoteAddress.c_str());
		return true;
	}

	int read(char *buffer, int length) override
	{
		const int socketFd = fd.load(std::memory_order_acquire);
		if (socketFd < 0 || buffer == nullptr || length <= 0 ||
		    closing.load(std::memory_order_acquire))
			return -1;

		// The Wii's interruptible shape: wait in select slices so a
		// close()/interruptRead() from another thread takes effect promptly
		// instead of after a whole blocking recv. Because select() says
		// "readable" only when data or EOF is queued, the recv below returns
		// without hanging, which also keeps it clear of a close() that lands
		// while it is in flight.
		//
		// The slice is 20 ms, not the 100 ms the Wii path uses: on a 3DS the
		// reader thread shares the second core with the writer, and this
		// slice is pure added latency for every inbound packet that arrives
		// while the socket was idle -- up to a tenth of a second before the
		// server's entity/chat/keepalive traffic even reaches the decode,
		// on top of the radio RTT. 20 ms keeps close() responsiveness at a
		// worst case of one slice while capping that tax at a thirtieth of
		// a second; the extra wake-ups cost nothing on a core the game
		// thread never runs on.
		while (!closing.load(std::memory_order_acquire) &&
		       !readInterrupted.load(std::memory_order_acquire))
		{
			fd_set readSet;
			struct timeval tv{};
			tv.tv_sec = 0;
			tv.tv_usec = 20000; // 20 ms slices

			FD_ZERO(&readSet);
			FD_SET(socketFd, &readSet);

			const int ready = ::select(socketFd + 1, &readSet, nullptr, nullptr, &tv);
			// Re-check after the wait: close() may have retired this
			// descriptor while select() slept, and its number can already have
			// been recycled by a later connection.
			if (fd.load(std::memory_order_acquire) != socketFd ||
			    closing.load(std::memory_order_acquire) ||
			    readInterrupted.load(std::memory_order_acquire))
				return -1;
			if (ready < 0)
				return -1;
			if (ready == 0)
				continue; // slice elapsed, re-check the flags

			if (FD_ISSET(socketFd, &readSet))
			{
				const int count = static_cast<int>(
					::recv(socketFd, buffer, static_cast<std::size_t>(length), 0));
				if (count > 0)
				{
					receivedBytes.fetch_add(static_cast<std::size_t>(count),
					                         std::memory_order_relaxed);
					return count;
				}
				return -1; // peer closed or read error
			}
		}
		return -1;
	}

	bool write(const char *buffer, int length) override
	{
		if (buffer == nullptr)
			return false;
		if (length <= 0)
			return true;

		// Serialized like the PS2 backend: the write thread and any
		// disconnect flush can both push bytes through this socket.
		std::lock_guard<PlatformMutex> guard(writeLock);
		int offset = 0;
		while (offset < length)
		{
			const int socketFd = fd.load(std::memory_order_acquire);
			if (socketFd < 0 || closing.load(std::memory_order_acquire))
				return false;

			const int count = static_cast<int>(
				::send(socketFd, buffer + offset, static_cast<std::size_t>(length - offset), 0));
			if (count > 0)
			{
				sentBytes.fetch_add(static_cast<std::size_t>(count), std::memory_order_relaxed);
				offset += count;
				continue;
			}
			if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
				return false;
			// EWOULDBLOCK (or a bare 0): the kernel buffer is full. The SOC
			// stack has no SO_SNDTIMEO, so wait for writability in 100 ms
			// slices -- close() takes effect within one slice, and a peer
			// that stops draining for good surfaces as a failed write (a
			// disconnect) instead of a wedged thread. The budget resets on
			// progress only, so a healthy but slow peer is never cut off.
			int waitedMs = 0;
			while (!closing.load(std::memory_order_acquire))
			{
				fd_set writeSet;
				struct timeval tv{};
				tv.tv_sec = 0;
				tv.tv_usec = 100000; // 100 ms slices

				FD_ZERO(&writeSet);
				FD_SET(socketFd, &writeSet);

				const int ready = ::select(socketFd + 1, nullptr, &writeSet, nullptr, &tv);
				if (fd.load(std::memory_order_acquire) != socketFd ||
				    closing.load(std::memory_order_acquire))
					return false;
				if (ready < 0)
					return false;
				if (ready > 0)
					break; // writable again -- retry the send

				waitedMs += 100;
				if (waitedMs >= 10000)
					return false;
			}
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
		// Flag only, like the Wii: the select-sliced reader notices within
		// one slice, and this avoids shutdown()ing a descriptor another
		// thread may be retiring concurrently.
		readInterrupted.store(true, std::memory_order_release);
	}

	void close() override
	{
		closing.store(true, std::memory_order_release);
		readInterrupted.store(true, std::memory_order_release);
		const int socketFd = fd.exchange(-1, std::memory_order_acq_rel);
		if (socketFd >= 0)
		{
			::shutdown(socketFd, SHUT_RDWR);
			::close(socketFd);
		}
	}

	std::string getRemoteSocketAddress() const override { return remoteAddress; }
	std::size_t getReceivedByteCount() const override { return receivedBytes.load(std::memory_order_relaxed); }
	std::size_t getSentByteCount() const override { return sentBytes.load(std::memory_order_relaxed); }

private:
	void releaseSocket()
	{
		closing.store(true, std::memory_order_release);
		readInterrupted.store(true, std::memory_order_release);
		const int socketFd = fd.exchange(-1, std::memory_order_acq_rel);
		if (socketFd >= 0)
		{
			::shutdown(socketFd, SHUT_RDWR);
			::close(socketFd);
		}
	}

	std::atomic<int> fd{-1};
	std::atomic_bool closing{true};
	std::atomic_bool readInterrupted{false};
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
		int count = socket.read(buffer, sizeof(buffer));
		if (count <= 0)
			return traits_type::eof();
		setg(buffer, buffer, buffer + count);
		return traits_type::to_int_type(*gptr());
	}

private:
	Socket &socket;
	// 8 KiB, not the 512 bytes this used to be: every underflow() is one
	// select() + recv() IPC round-trip into the SOC service, and the
	// istream-driven Packet reader asks in streambuf-sized gulps. A login
	// burst or a map-chunk packet (Packet51 payloads run to tens of KiB
	// compressed) crossed that in hundreds of round-trips -- each with its
	// own kernel context switch -- which was a large slice of the "multi-
	// player feels far worse than singleplayer" gap on this console. 8 KiB
	// divides the same traffic by 16, and the istream member lives on the
	// heap, so the growth is not stack budget.
	char buffer[8192];
};

class SocketOutputBuffer final : public std::streambuf
{
public:
	explicit SocketOutputBuffer(Socket &value) : socket(value)
	{
		setp(buffer, buffer + sizeof(buffer));
	}
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
	char buffer[5120];
};

class SocketInputStream final : public std::istream
{
public:
	explicit SocketInputStream(Socket &socket) : std::istream(nullptr), buffer(socket)
	{
		rdbuf(&buffer);
	}
private:
	SocketInputBuffer buffer;
};

class SocketOutputStream final : public std::ostream
{
public:
	explicit SocketOutputStream(Socket &socket) : std::ostream(nullptr), buffer(socket)
	{
		rdbuf(&buffer);
	}
private:
	SocketOutputBuffer buffer;
};

}

std::unique_ptr<Socket> createSocket() { return std::make_unique<DsSocket>(); }
std::unique_ptr<std::istream> createInputStream(Socket &socket) { return std::make_unique<SocketInputStream>(socket); }
std::unique_ptr<std::ostream> createOutputStream(Socket &socket) { return std::make_unique<SocketOutputStream>(socket); }

// The multiplayer path only needs raw TCP. Keep HTTP/HTTPS disabled so
// enabling multiplayer does not re-enable desktop resource/auth traffic
// (the Wii and PS2 backends do the same).
bool readUrl(const std::string &, std::vector<unsigned char> &) { return false; }
int getResponseCode(const std::string &) { return -1; }
bool postUrl(const std::string &, const std::string &, const std::string &,
             std::vector<unsigned char> &) { return false; }

}

#endif
