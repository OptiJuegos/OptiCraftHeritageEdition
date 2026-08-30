#include "java/JavaNetwork.h"

#ifdef WII_PLATFORM

#include <atomic>
#include <cstring>
#include <istream>
#include <ostream>
#include <streambuf>

#include <network.h>

#include "wii/WiiNetwork.h"

namespace JavaNetwork
{
namespace
{

class WiiSocket final : public Socket
{
public:
	~WiiSocket() override { releaseSocket(); }

	bool connect(const std::string &host, int port) override
	{
		releaseSocket();
		closing.store(false, std::memory_order_release);
		remoteAddress = host + ":" + std::to_string(port);
		if (port < 1 || port > 65535 || !WiiNetwork::initialize())
			return false;

		hostent *resolved = net_gethostbyname(host.c_str());
		if (resolved == nullptr || resolved->h_addr_list == nullptr || resolved->h_addr_list[0] == nullptr)
			return false;

		const s32 socketFd = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (socketFd < 0)
			return false;
		fd.store(socketFd, std::memory_order_release);

		sockaddr_in target;
		std::memset(&target, 0, sizeof(target));
		target.sin_family = AF_INET;
		target.sin_port = htons(static_cast<u16>(port));
		std::memcpy(&target.sin_addr, resolved->h_addr_list[0], sizeof(target.sin_addr));

		if (net_connect(socketFd, reinterpret_cast<sockaddr *>(&target), sizeof(target)) < 0)
		{
			close();
			return false;
		}
		return true;
	}

	int read(char *buffer, int length) override
	{
		const s32 socketFd = fd.load(std::memory_order_acquire);
		if (socketFd < 0 || buffer == nullptr || length <= 0 ||
		    closing.load(std::memory_order_acquire))
			return -1;
		return static_cast<int>(net_recv(socketFd, buffer, length, 0));
	}

	bool write(const char *buffer, int length) override
	{
		const s32 socketFd = fd.load(std::memory_order_acquire);
		if (socketFd < 0 || buffer == nullptr || closing.load(std::memory_order_acquire))
			return false;
		int offset = 0;
		while (offset < length)
		{
			if (closing.load(std::memory_order_acquire))
				return false;
			s32 count = net_send(socketFd, buffer + offset, length - offset, 0);
			if (count <= 0)
				return false;
			offset += static_cast<int>(count);
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
		const s32 socketFd = fd.load(std::memory_order_acquire);
		if (socketFd >= 0)
			net_shutdown(socketFd, 0);
	}

	void close() override
	{
		closing.store(true, std::memory_order_release);
		const s32 socketFd = fd.load(std::memory_order_acquire);
		if (socketFd >= 0)
			net_shutdown(socketFd, 2);
	}

	std::string getRemoteSocketAddress() const override { return remoteAddress; }

private:
	void releaseSocket()
	{
		closing.store(true, std::memory_order_release);
		const s32 socketFd = fd.exchange(-1, std::memory_order_acq_rel);
		if (socketFd >= 0)
		{
			net_shutdown(socketFd, 2);
			net_close(socketFd);
		}
	}

	std::atomic<s32> fd{-1};
	std::atomic_bool closing{true};
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
	char buffer[512];
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

std::unique_ptr<Socket> createSocket() { return std::make_unique<WiiSocket>(); }
std::unique_ptr<std::istream> createInputStream(Socket &socket) { return std::make_unique<SocketInputStream>(socket); }
std::unique_ptr<std::ostream> createOutputStream(Socket &socket) { return std::make_unique<SocketOutputStream>(socket); }

// Multiplayer only needs the TCP socket API. HTTPS resource/auth downloads are
// deliberately left disabled until the Wii has a TLS backend.
bool readUrl(const std::string &, std::vector<unsigned char> &) { return false; }
int getResponseCode(const std::string &) { return -1; }

}

#endif
