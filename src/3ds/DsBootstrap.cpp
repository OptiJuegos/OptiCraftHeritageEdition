// DsBootstrap.cpp -- 3DS implementation of the DsBootstrap.h lifecycle.
//
// Same take-the-mount-once shape as src/wii/system/WiiEarlyStorage.cpp,
// without the device probing: this console has exactly one storage device
// (the SD card, always "sdmc:"), so there is nothing to search for -- only to
// mount, and to give a fresh card somewhere to copy the data into.
#ifdef CTR_PLATFORM

#include "3ds/DsBootstrap.h"

#include <3ds.h>

#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

#include "platform/Log.h"

namespace
{
// One attempt, ever. java::File objects and Resource lookups can run from
// static initialisers before main(), and main_3ds.cpp also calls this
// deliberately; a second fsInit()/mount would double-count libctru's
// ref-counted fs service and re-register an already-mounted stdio device.
bool g_storageTried = false;
bool g_storageMounted = false;

bool statPath(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0;
}
} // namespace

bool dsEnsureStorage()
{
	if (g_storageTried)
		return g_storageMounted;
	g_storageTried = true;

	if (R_FAILED(fsInit()))
	{
		MC_LOG_WARN("3ds", "fsInit() failed; SD card unavailable\n");
		return false;
	}

	// DsBootstrap.h names fsdevMountSdmc(); the installed libctru (2.7.0)
	// carries no fsdev API at all -- archiveMountSdmc() is its replacement
	// and registers the same "sdmc" stdio device, which is why every path in
	// this port is spelled "sdmc:/...", never "sd:/...".
	//
	// The call is means, not the goal: a usable "sdmc:" device is. archiveMount
	// documents -1 as a generic failure, which covers the device already being
	// registered as much as the archive failing to open, so a failed call that
	// still leaves the device stat-able means something mounted it first and
	// this port should carry on rather than strand the player on a blank card
	// that is in fact mounted.
	const Result mounted = archiveMountSdmc();
	if (R_FAILED(mounted) && !statPath("sdmc:/"))
	{
		MC_LOG_WARN("3ds", "archiveMountSdmc() failed: %08lX (sdmc:/ not stat-able)\n",
		            static_cast<unsigned long>(mounted));
		return false;
	}
	if (R_FAILED(mounted))
	{
		MC_LOG_INFO("3ds", "archiveMountSdmc() %08lX but sdmc: already usable; continuing\n",
		            static_cast<unsigned long>(mounted));
	}

	// Best effort, deliberately not fatal: EEXIST just means the directory is
	// already staged (from a previous run or by hand), and any other failure
	// surfaces as dsHasGameData() == false, which main_3ds.cpp already knows
	// how to explain to the player.
	if (mkdir(dsGetAppDir(), 0777) != 0 && errno != EEXIST)
		MC_LOG_WARN("3ds", "could not create %s (errno %d)\n", dsGetAppDir(), errno);

	// Same order as WiiEarlyStorage.cpp: open the sink BEFORE the "mounted"
	// line below, so the fsInit/mount diagnostics already sitting in McLog's
	// early buffer are replayed ahead of it, and a run that hangs later still
	// leaves a debug.log whose last line names the last thing that ran. This
	// is what gives the 3DS a log file at all -- nothing else on this port
	// ever called openSessionFile(), so McLog::write() had stdout as its only
	// sink. A no-op at MC_LOG_LEVEL 0 (openSessionFile returns false and
	// writes nothing), which is why the warning is harmless there too.
	if (!McLog::openSessionFile(dsGetAppDir()))
		MC_LOG_WARN("3ds", "could not create %s/debug.log\n", dsGetAppDir());

	MC_LOG_INFO("3ds", "storage mounted: %s\n", dsGetAppDir());
	g_storageMounted = true;
	return true;
}

const char* dsGetAppDir()
{
	// Pure constant, per DsBootstrap.h: the string is meaningful even before
	// the mount exists (opens simply fail until then), so unlike
	// wiiGetAppDir() this must NOT trigger dsEnsureStorage().
	return "sdmc:/opticraft";
}

bool dsHasGameData()
{
	// stat() can only work with the device mounted, and this may be the call
	// that mounts it (static initialisers run before main_3ds.cpp).
	if (!dsEnsureStorage())
		return false;

	// The loose layout first, then the packed one -- the two paths
	// DsBootstrap.h promises, resolved under the fixed app dir above.
	char path[256];
	std::snprintf(path, sizeof(path), "%s/data/assets/font.txt", dsGetAppDir());
	if (statPath(path))
		return true;

	std::snprintf(path, sizeof(path), "%s/assets.pak", dsGetAppDir());
	return statPath(path);
}

bool dsIsNew3DS()
{
	// Cached for the process lifetime: the answer cannot change while the
	// title runs, and the caller is a per-tick packet budget.
	static int cachedModel = -1;
	if (cachedModel < 0)
	{
		bool isNew3ds = false;
		if (R_FAILED(APT_CheckNew3DS(&isNew3ds)))
			cachedModel = 0; // fails safe: the Old-3DS profile
		else
			cachedModel = isNew3ds ? 1 : 0;
	}
	return cachedModel != 0;
}

#endif // CTR_PLATFORM
