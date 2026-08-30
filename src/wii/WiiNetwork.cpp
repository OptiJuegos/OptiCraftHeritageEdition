#include "platform/Log.h"
#include "wii/WiiNetwork.h"

#include <cerrno>
#include <cstdio>
#include <mutex>

#include <network.h>
#include <unistd.h>

#include "wii/WiiEarlyInit.h"

namespace WiiNetwork
{
namespace
{
std::mutex stateMutex;
bool ready = false;
std::string address;
}

bool initialize()
{
	std::lock_guard<std::mutex> guard(stateMutex);
	if (ready)
		return true;

	// IOS can report EAGAIN while the interface configured in the Wii System
	// Menu is still coming up. Keep this bounded so a bad access point does not
	// leave the connection screen stuck forever.
	s32 result = -EAGAIN;
	for (int attempt = 0; attempt < 100 && result == -EAGAIN; ++attempt)
	{
		result = net_init();
		if (result == -EAGAIN)
			usleep(100000);
	}

	if (result < 0)
	{
		MC_LOG_INFO("wii", "network: net_init failed (%d)\n", static_cast<int>(result));
		return false;
	}

	u32 host = net_gethostip();
	char text[16];
	std::snprintf(text, sizeof(text), "%u.%u.%u.%u",
		static_cast<unsigned>((host >> 24) & 0xff),
		static_cast<unsigned>((host >> 16) & 0xff),
		static_cast<unsigned>((host >> 8) & 0xff),
		static_cast<unsigned>(host & 0xff));
	address = text;
	ready = true;
	MC_LOG_INFO("wii", "network: ready, ip=%s\n", address.c_str());
	return true;
}

bool isReady()
{
	std::lock_guard<std::mutex> guard(stateMutex);
	return ready;
}

const std::string &localAddress()
{
	return address;
}

void shutdown()
{
	std::lock_guard<std::mutex> guard(stateMutex);
	if (!ready)
		return;
	net_deinit();
	ready = false;
	address.clear();
}

}
