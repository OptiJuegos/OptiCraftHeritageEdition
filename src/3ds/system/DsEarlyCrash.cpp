// DsEarlyCrash.cpp -- 3DS terminate handler, the counterpart of
// wii/system/WiiEarlyCrash.cpp. Without it an uncaught C++ exception dies
// through libstdc++'s default terminate -- a one-line stderr note the player
// may never read, and nothing of the reason survives the run. Routing through
// CrashHandler::Crash() puts the exception's own message on the bottom screen
// (held until START) and, via that handler's file log, in
// sdmc:/opticraft/debug.log where the next debugging pass reads it.
#ifdef CTR_PLATFORM

#include "3ds/system/DsEarlyCrash.h"

#include <cstdlib>
#include <exception>
#include <string>

#include "pc/CrashHandler.h"

namespace
{
void terminateHandler()
{
	std::string message = "Unhandled exception before the game started";
	if (std::current_exception())
	{
		try
		{
			std::rethrow_exception(std::current_exception());
		}
		catch (const std::exception& exception)
		{
			message = std::string("Unhandled exception: ") + exception.what();
		}
		catch (...)
		{
			message = "Unhandled non-standard exception";
		}
	}

	std::set_terminate(std::abort);
	CrashHandler::Crash(message);
	std::abort();
}
}

namespace DsEarlyCrash
{
void install()
{
	std::set_terminate(terminateHandler);
}
}

#endif // CTR_PLATFORM
