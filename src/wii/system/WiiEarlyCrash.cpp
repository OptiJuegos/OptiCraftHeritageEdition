#ifdef WII_PLATFORM

#include "wii/system/WiiEarlyCrash.h"

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

namespace WiiEarlyCrash
{
void install()
{
	std::set_terminate(terminateHandler);
}
}

#endif // WII_PLATFORM
