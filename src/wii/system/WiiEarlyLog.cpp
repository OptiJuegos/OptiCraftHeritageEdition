#ifdef WII_PLATFORM

#include "wii/system/WiiEarlyLog.h"
#include "platform/Log.h"
#include "wii/WiiEarlyInit.h"

#include <cstdio>
#include <cstring>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

namespace
{
bool g_bootPhase = true;
bool g_logAtLineStart = true;
u32 g_logSequence = 0;
}

namespace WiiEarlyLog
{

void endBootPhase()
{
	g_bootPhase = false;
}

} // namespace WiiEarlyLog

void wiiPlatformLogEndBootPhase()
{
	WiiEarlyLog::endBootPhase();
}

extern "C" void wiiPlatformLogWrite(const char* message)
{
#if MC_LOG_LEVEL == 0
	(void)message;
#else
	if (!message)
		return;

	char line[576];
	if (g_logAtLineStart)
	{
		const u32 elapsedMs = static_cast<u32>(ticks_to_millisecs(gettime()));
		const u32 sequence = ++g_logSequence;
		std::snprintf(line, sizeof(line), "[%06u.%03u][%05u] %s",
		              elapsedMs / 1000u, elapsedMs % 1000u, sequence, message);
	}
	else
	{
		std::snprintf(line, sizeof(line), "%s", message);
	}

	const size_t lineLen = std::strlen(line);
	g_logAtLineStart = lineLen > 0 && line[lineLen - 1] == '\n';

	if (g_bootPhase)
		std::fputs(line, stdout);

	SYS_Report("%s", line);

#endif
}

#endif // WII_PLATFORM
