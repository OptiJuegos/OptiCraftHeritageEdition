#include "platform/Log.h"
#include "3ds/DsNetwork.h"

#include <malloc.h>
#include <mutex>

#include <3ds.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace DsNetwork
{
namespace
{
std::mutex stateMutex;
bool ready = false;
std::string address;

// Same shape devkitPro's own sockets example uses
// (examples/3ds/network/sockets): page aligned, a multiple of 0x1000. After
// socInit() the SOC kernel owns the block and maps it no-access to the app,
// so it must stay untouched and allocated until socExit() hands it back --
// hence the raw memalign() allocation living here instead of inside one
// call. It is created on the first connect attempt, so a session that never
// joins a server never pays the megabyte.
constexpr unsigned int contextAlign = 0x1000;
constexpr unsigned int contextSize = 0x100000;
u32 *contextBuffer = nullptr;
}

bool initialize()
{
	std::lock_guard<std::mutex> guard(stateMutex);
	if (ready)
		return true;

	contextBuffer = static_cast<u32 *>(memalign(contextAlign, contextSize));
	if (contextBuffer == nullptr)
	{
		MC_LOG_ERROR("3ds", "network: soc context allocation failed (%u bytes)\n", contextSize);
		return false;
	}

	const Result result = socInit(contextBuffer, contextSize);
	if (result != 0)
	{
		MC_LOG_ERROR("3ds", "network: socInit failed (0x%08lX)\n",
		             static_cast<unsigned long>(result));
		free(contextBuffer);
		contextBuffer = nullptr;
		return false;
	}

	// Informational only: only the log line consumes it today (no screen
	// prints the local IP yet). The SOC service answers 0.0.0.0 while no
	// interface is up, which is fine -- the connect() attempt then reports
	// the real problem.
	struct in_addr ip = {};
	struct in_addr netmask = {};
	struct in_addr broadcast = {};
	if (SOCU_GetIPInfo(&ip, &netmask, &broadcast) == 0)
		address = inet_ntoa(ip);
	else
		address = "0.0.0.0";

	ready = true;
	MC_LOG_INFO("3ds", "network: ready, ip=%s\n", address.c_str());
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
	socExit();
	free(contextBuffer);
	contextBuffer = nullptr;
	ready = false;
	address.clear();
}

// One ACU_GetWifiStatus round-trip through the ac:u service (ref-counted by
// libctru, so an init/exit pair per query is cheap and safe even beside
// another user). 0 is the service's "wireless disabled" answer.
namespace
{
enum class WifiRadioState
{
	Unknown,
	Off,
	On,
};

WifiRadioState queryWifiRadioState()
{
	u32 wifiStatus = 0;
	if (R_FAILED(acInit()))
		return WifiRadioState::Unknown;
	const Result status = ACU_GetWifiStatus(&wifiStatus);
	acExit();
	if (R_FAILED(status))
		return WifiRadioState::Unknown;
	return wifiStatus != 0 ? WifiRadioState::On : WifiRadioState::Off;
}
} // namespace

std::string wifiPreflightError()
{
	const WifiRadioState radio = queryWifiRadioState();
	if (radio != WifiRadioState::Off)
		return std::string(); // on, or unknowable -- never block the attempt

	MC_LOG_WARN("3ds", "network: Wi-Fi is switched off\n");

	// svcSetWifiEnabled exists from kernel 2.55 (system 11.4). Older kernels
	// simply keep the message below, which is all they can do.
	if (osGetKernelVersion() >= SYSTEM_VERSION(2, 55, 0))
	{
		const Result enabledResult = svcSetWifiEnabled(true);
		if (R_SUCCEEDED(enabledResult))
		{
			// Give the radio a moment to come up before believing it.
			for (int attempt = 0; attempt < 10; ++attempt)
			{
				if (queryWifiRadioState() == WifiRadioState::On)
				{
					MC_LOG_INFO("3ds", "network: Wi-Fi re-enabled by the game\n");
					return "Wi-Fi was switched off. It has been turned back on; please connect again.";
				}
				svcSleepThread(100 * 1000 * 1000LL);
			}
		}
		else
		{
			MC_LOG_WARN("3ds", "network: svcSetWifiEnabled failed (%08lX)\n",
			            static_cast<unsigned long>(enabledResult));
		}
	}

	return "Wi-Fi is switched off. Enable wireless (HOME menu toggle on a New 3DS, "
	       "the side switch on an Old 3DS) and connect again.";
}

}
