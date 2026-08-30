#include "platform/Log.h"
#include "java/JavaNetwork.h"

// Both console ports build with NO_NETWORK; multiplayer is out of scope for
// them, so the whole SDL_net-backed implementation is replaced by stubs.
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)

#include <istream>
#include <ostream>

namespace JavaNetwork
{
std::unique_ptr<Socket> createSocket()                                    { return nullptr; }
std::unique_ptr<std::istream> createInputStream(Socket &)                 { return nullptr; }
std::unique_ptr<std::ostream> createOutputStream(Socket &)                { return nullptr; }
bool readUrl(const std::string &, std::vector<unsigned char> &)           { return false; }
int  getResponseCode(const std::string &)                                 { return -1; }
bool postUrl(const std::string &, const std::string &, const std::string &,
             std::vector<unsigned char> &)                              { return false; }
}

#else

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <streambuf>

#include <SDL_net.h>

namespace JavaNetwork
{

namespace
{

// SDL_net needs a one-time init before any socket call. SDL itself is already
// initialised by main(); SDLNet_Init() only sets up the networking subsystem.
void ensureNetInit()
{
	static std::once_flag flag;
	static bool ok = false;
	std::call_once(flag, []()
	{
		ok = SDLNet_Init() == 0;
		if (!ok)
			MC_LOG_ERROR("network", "SDLNet_Init failed: %s\n", SDLNet_GetError());
	});
}

// Opens a blocking client TCP connection. Returns nullptr on failure.
TCPsocket openConnection(const std::string &host, int port)
{
	ensureNetInit();
	IPaddress address;
	if (SDLNet_ResolveHost(&address, host.c_str(), static_cast<Uint16>(port)) != 0)
		return nullptr;
	return SDLNet_TCP_Open(&address);
}

class SdlNetSocket : public Socket
{
public:
	~SdlNetSocket() override
	{
		releaseSocket();
	}

	bool connect(const std::string &host, int port) override
	{
		releaseSocket();
		closing.store(false, std::memory_order_release);
		readInterrupted.store(false, std::memory_order_release);
		remoteSocketAddress = host + ":" + std::to_string(port);
		socket = openConnection(host, port);
		if (socket == nullptr)
			return false;

		socketSet = SDLNet_AllocSocketSet(1);
		if (socketSet == nullptr || SDLNet_TCP_AddSocket(socketSet, socket) < 0)
		{
			releaseSocket();
			return false;
		}
		socketInSet = true;
		return true;
	}

	int read(char *buffer, int length) override
	{
		if (socket == nullptr || socketSet == nullptr || buffer == nullptr || length <= 0)
			return -1;

		while (!closing.load(std::memory_order_acquire) &&
		       !readInterrupted.load(std::memory_order_acquire))
		{
			std::lock_guard<std::mutex> guard(socketIoLock);
			if (closing.load(std::memory_order_acquire) ||
			    readInterrupted.load(std::memory_order_acquire))
				return -1;

			const int ready = SDLNet_CheckSockets(socketSet, 100);
			if (ready < 0)
				return -1;
			if (ready == 0)
				continue;
			if (SDLNet_SocketReady(socket))
				return SDLNet_TCP_Recv(socket, buffer, length);
		}
		return -1;
	}

	bool write(const char *buffer, int length) override
	{
		if (socket == nullptr || buffer == nullptr || closing.load(std::memory_order_acquire))
			return false;
		if (length <= 0)
			return true;

		std::lock_guard<std::mutex> guard(socketIoLock);
		int offset = 0;
		while (offset < length)
		{
			if (closing.load(std::memory_order_acquire))
				return false;
			const int sent = SDLNet_TCP_Send(socket, buffer + offset, length - offset);
			if (sent <= 0)
				return false;
			offset += sent;
		}
		return true;
	}

	bool flush() override
	{
		return socket != nullptr && !closing.load(std::memory_order_acquire);
	}

	void interruptRead() override
	{
		readInterrupted.store(true, std::memory_order_release);
	}

	void close() override
	{
		closing.store(true, std::memory_order_release);
		readInterrupted.store(true, std::memory_order_release);
	}

	std::string getRemoteSocketAddress() const override
	{
		return remoteSocketAddress;
	}

private:
	void releaseSocket()
	{
		closing.store(true, std::memory_order_release);
		readInterrupted.store(true, std::memory_order_release);
		if (socketSet != nullptr)
		{
			if (socketInSet && socket != nullptr)
				SDLNet_TCP_DelSocket(socketSet, socket);
			SDLNet_FreeSocketSet(socketSet);
			socketSet = nullptr;
			socketInSet = false;
		}
		if (socket != nullptr)
		{
			SDLNet_TCP_Close(socket);
			socket = nullptr;
		}
	}

	TCPsocket socket = nullptr;
	SDLNet_SocketSet socketSet = nullptr;
	bool socketInSet = false;
	std::mutex socketIoLock;
	std::atomic_bool closing{false};
	std::atomic_bool readInterrupted{false};
	std::string remoteSocketAddress;
};

class SocketInputBuffer : public std::streambuf
{
public:
	explicit SocketInputBuffer(Socket &socket)
		: socket(socket)
	{
		setg(buffer, buffer, buffer);
	}

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

class SocketOutputBuffer : public std::streambuf
{
public:
	explicit SocketOutputBuffer(Socket &socket)
		: socket(socket)
	{
		setp(buffer, buffer + sizeof(buffer));
	}

	~SocketOutputBuffer() override
	{
		sync();
	}

protected:
	std::streamsize xsputn(const char *s, std::streamsize n) override
	{
		std::streamsize written = 0;
		while (written < n)
		{
			std::streamsize space = epptr() - pptr();
			if (space == 0)
			{
				if (!flushBuffer())
					return written;
				space = epptr() - pptr();
			}

			const std::streamsize remaining = n - written;
			const std::streamsize count = remaining < space ? remaining : space;
			std::memcpy(pptr(), s + written, static_cast<std::size_t>(count));
			pbump(static_cast<int>(count));
			written += count;
		}
		return written;
	}

	int_type overflow(int_type ch) override
	{
		if (traits_type::eq_int_type(ch, traits_type::eof()))
			return traits_type::not_eof(ch);
		if (!flushBuffer())
			return traits_type::eof();

		*pptr() = traits_type::to_char_type(ch);
		pbump(1);
		return ch;
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

class SocketInputStream : public std::istream
{
public:
	explicit SocketInputStream(Socket &socket)
		: std::istream(nullptr)
		, buffer(socket)
	{
		rdbuf(&buffer);
	}

private:
	SocketInputBuffer buffer;
};

class SocketOutputStream : public std::ostream
{
public:
	explicit SocketOutputStream(Socket &socket)
		: std::ostream(nullptr)
		, buffer(socket)
	{
		rdbuf(&buffer);
	}

private:
	SocketOutputBuffer buffer;
};

}

std::unique_ptr<Socket> createSocket()
{
	return std::make_unique<SdlNetSocket>();
}

std::unique_ptr<std::istream> createInputStream(Socket &socket)
{
	return std::make_unique<SocketInputStream>(socket);
}

std::unique_ptr<std::ostream> createOutputStream(Socket &socket)
{
	return std::make_unique<SocketOutputStream>(socket);
}

namespace
{

// Minimal HTTP/1.0 GET over SDL_net. Returns the HTTP status code (or -1 on a
// connection/parse failure) and fills body with the response payload.
// Only plain http:// is supported — SDL_net has no TLS, so https is rejected.
int httpGet(const std::string &url, std::vector<unsigned char> &body)
{
	const std::string scheme = "http://";
	if (url.rfind(scheme, 0) != 0)
		return -1;

	std::string rest = url.substr(scheme.size());
	std::string::size_type pathStart = rest.find('/');
	std::string hostPort = pathStart == std::string::npos ? rest : rest.substr(0, pathStart);
	std::string path = pathStart == std::string::npos ? "/" : rest.substr(pathStart);

	std::string host = hostPort;
	int port = 80;
	std::string::size_type colon = hostPort.find(':');
	if (colon != std::string::npos)
	{
		host = hostPort.substr(0, colon);
		port = std::atoi(hostPort.c_str() + colon + 1);
	}

	TCPsocket socket = openConnection(host, port);
	if (socket == nullptr)
		return -1;

	std::string request =
		"GET " + path + " HTTP/1.0\r\n" +
		"Host: " + host + "\r\n" +
		"User-Agent: Minecraft\r\n" +
		"Connection: close\r\n\r\n";
	int requestOffset = 0;
	while (requestOffset < static_cast<int>(request.size()))
	{
		const int sent = SDLNet_TCP_Send(
			socket, request.data() + requestOffset, static_cast<int>(request.size()) - requestOffset);
		if (sent <= 0)
		{
			SDLNet_TCP_Close(socket);
			return -1;
		}
		requestOffset += sent;
	}

	std::string raw;
	char chunk[2048];
	for (;;)
	{
		int count = SDLNet_TCP_Recv(socket, chunk, sizeof(chunk));
		if (count <= 0)
			break;
		raw.append(chunk, count);
	}
	SDLNet_TCP_Close(socket);

	if (raw.empty())
		return -1;

	int status = -1;
	std::string::size_type sp = raw.find(' ');
	if (sp != std::string::npos)
		status = std::atoi(raw.c_str() + sp + 1);

	std::string::size_type headerEnd = raw.find("\r\n\r\n");
	if (headerEnd != std::string::npos)
	{
		const char *bodyStart = raw.data() + headerEnd + 4;
		std::size_t bodyLen = raw.size() - (headerEnd + 4);
		body.assign(bodyStart, bodyStart + bodyLen);
	}
	return status;
}


int httpPost(const std::string &url, const std::string &contentType,
             const std::string &body, std::vector<unsigned char> &response)
{
	const std::string scheme = "http://";
	if (url.rfind(scheme, 0) != 0)
		return -1;

	std::string rest = url.substr(scheme.size());
	std::string::size_type pathStart = rest.find('/');
	std::string hostPort = pathStart == std::string::npos ? rest : rest.substr(0, pathStart);
	std::string path = pathStart == std::string::npos ? "/" : rest.substr(pathStart);
	std::string host = hostPort;
	int port = 80;
	const std::string::size_type colon = hostPort.find(':');
	if (colon != std::string::npos)
	{
		host = hostPort.substr(0, colon);
		port = std::atoi(hostPort.c_str() + colon + 1);
	}

	TCPsocket socket = openConnection(host, port);
	if (socket == nullptr)
		return -1;

	const std::string request =
		"POST " + path + " HTTP/1.0\r\n" +
		"Host: " + host + "\r\n" +
		"User-Agent: Minecraft\r\n" +
		"Content-Type: " + contentType + "\r\n" +
		"Content-Length: " + std::to_string(body.size()) + "\r\n" +
		"Content-Language: en-US\r\n" +
		"Connection: close\r\n\r\n" + body;

	int offset = 0;
	while (offset < (int)request.size())
	{
		const int sent = SDLNet_TCP_Send(socket, request.data() + offset,
		                                (int)request.size() - offset);
		if (sent <= 0)
		{
			SDLNet_TCP_Close(socket);
			return -1;
		}
		offset += sent;
	}

	std::string raw;
	char chunk[2048];
	for (;;)
	{
		const int count = SDLNet_TCP_Recv(socket, chunk, sizeof(chunk));
		if (count <= 0)
			break;
		raw.append(chunk, count);
	}
	SDLNet_TCP_Close(socket);
	if (raw.empty())
		return -1;

	int status = -1;
	const std::string::size_type sp = raw.find(' ');
	if (sp != std::string::npos)
		status = std::atoi(raw.c_str() + sp + 1);

	const std::string::size_type headerEnd = raw.find("\r\n\r\n");
	response.clear();
	if (headerEnd != std::string::npos)
	{
		const char *begin = raw.data() + headerEnd + 4;
		response.assign(begin, begin + (raw.size() - headerEnd - 4));
	}
	return status;
}

}

bool readUrl(const std::string &url, std::vector<unsigned char> &data)
{
	if (url.rfind("http://", 0) == 0)
	{
		int status = httpGet(url, data);
		return status >= 200 && status < 300;
	}

	std::string file = url;
	if (file.rfind("file://", 0) == 0)
		file = file.substr(7);

	if (file.find("://") != std::string::npos)
		return false;

	std::ifstream input(file, std::ios::binary);
	if (!input)
		return false;

	input.seekg(0, std::ios::end);
	std::streamoff size = input.tellg();
	input.seekg(0, std::ios::beg);
	if (size < 0)
		return false;

	data.resize((std::size_t)size);
	if (!data.empty())
		input.read(reinterpret_cast<char *>(data.data()), size);
	return true;
}

int getResponseCode(const std::string &url)
{
	if (url.rfind("http://", 0) == 0)
	{
		std::vector<unsigned char> body;
		return httpGet(url, body);
	}

	std::vector<unsigned char> data;
	return readUrl(url, data) ? 200 : -1;
}


bool postUrl(const std::string &url, const std::string &contentType,
             const std::string &body, std::vector<unsigned char> &response)
{
	if (url.rfind("http://", 0) != 0)
		return false;
	const int status = httpPost(url, contentType, body, response);
	return status >= 200 && status < 300;
}

}

#endif // !PS2_PLATFORM
