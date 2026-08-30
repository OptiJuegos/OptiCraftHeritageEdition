#ifdef PS2_PLATFORM

#include "ps2/system/Ps2EarlyCrash.h"

#include <cstdlib>
#include <exception>

#include "platform/Diagnostics.h"
#include "platform/Log.h"
#include "platform/WorldLoadTrace.h"

// The PS2 counterpart of src/wii/system/WiiEarlyCrash.cpp, and the same shape.
//
// Minecraft::run() catches MinecraftException, bad_alloc, std::exception and
// (...), so an exception raised inside the game loop is already reported. What
// had no handler on this console is everything that never reaches a catch:
// thrown before the loop is entered, thrown while unwinding, or thrown out of a
// noexcept context. All of those go to std::terminate, whose default is
// abort(), and abort() on the EE stops with the last framebuffer still on
// screen and nothing in debug.log. That is indistinguishable from a hang, which
// is exactly the failure this port has been chasing.
//
// The Wii side routes through CrashHandler::Crash(); that is a PC/Wii path, so
// this one writes the same facts to the log instead, which is the sink that
// survives when the renderer no longer does.
namespace
{
void terminateHandler()
{
	// Reinstall the default first: if anything below throws, the second
	// terminate must not re-enter this handler and recurse.
	std::set_terminate(std::abort);

	const char* what = nullptr;
	if (std::current_exception())
	{
		try
		{
			std::rethrow_exception(std::current_exception());
		}
		catch (const std::exception& exception)
		{
			what = exception.what();
		}
		catch (...)
		{
			what = "(non-standard exception)";
		}
	}

	MC_LOG_ERROR("crash", "[PS2] terminate: %s\n",
	             what != nullptr ? what : "(no active exception)");

	const char* stage = WorldLoadTrace::currentStage();
	if (stage[0] != '\0')
		MC_LOG_ERROR("crash", "[PS2] terminate during world load: %s\n", stage);

	platformMemoryCheckpoint("terminate");

	// Only the lines an actual allocation failure already captured. Capturing
	// here would stamp "OUT OF MEMORY" on every terminate, including the ones
	// that have nothing to do with the heap.
	for (int i = 0; i < 5; ++i)
	{
		const char* line = platformOomDiagnosticLine(i);
		if (line != nullptr && line[0] != '\0')
			MC_LOG_ERROR("crash", "[PS2] last alloc failure: %s\n", line);
	}

	std::abort();
}
}

namespace Ps2EarlyCrash
{
void install()
{
	std::set_terminate(terminateHandler);
}
}

#endif // PS2_PLATFORM
