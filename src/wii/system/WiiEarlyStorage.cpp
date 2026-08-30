#ifdef WII_PLATFORM

#include "platform/Log.h"
#include "wii/WiiEarlyInit.h"
#include "wii/system/WiiEarlyLog.h"
#include "platform/storage/PosixFileSystem.h"

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <unistd.h>

#include <fat.h>
#include <ogc/usbstorage.h>

#if __has_include(<dvm.h>)
#	include <dvm.h>
#	define WII_HAVE_DVM 1
#endif

namespace
{
bool g_storageTried = false;
bool g_storageReady = false;
char g_appDir[128] = {};
bool g_appDirHasData = false;

const char* const kDevices[] = { "sd", "usb", "usb2", "usb3", "usb4", "carda", "cardb" };
const char* const kAppSubDir = "/apps/BetaPlusPlus";
const char* const kProbeFile = "/data/assets/font.txt";
constexpr int kUsbMountAttempts = 4;
constexpr int kUsbRetryDelayUs = 500 * 1000;

bool fileExists(const char* path)
{
	return path && PlatformStorage::fileReadable(path);
}

bool dirExists(const char* path)
{
	return path && PlatformStorage::directoryAvailable(path);
}

bool hasGameData(const char* dir)
{
	char path[256];
	std::snprintf(path, sizeof(path), "%s%s", dir, kProbeFile);
	return fileExists(path);
}

bool getLaunchDir(char* out, size_t outSize)
{
	if (!getcwd(out, outSize) || !std::strchr(out, ':'))
		return false;

	size_t length = std::strlen(out);
	while (length > 1 && out[length - 1] == '/' && out[length - 2] != ':')
		out[--length] = '\0';
	return length > 0 && out[length - 1] != '/';
}

void resolveAppDir()
{
	char candidate[sizeof(g_appDir)];
	if (getLaunchDir(candidate, sizeof(candidate)) && hasGameData(candidate))
	{
		std::snprintf(g_appDir, sizeof(g_appDir), "%s", candidate);
		g_appDirHasData = true;
		return;
	}

	for (const char* device : kDevices)
	{
		std::snprintf(candidate, sizeof(candidate), "%s:%s", device, kAppSubDir);
		if (hasGameData(candidate))
		{
			std::snprintf(g_appDir, sizeof(g_appDir), "%s", candidate);
			g_appDirHasData = true;
			return;
		}
	}

	g_appDirHasData = false;
	if (getLaunchDir(candidate, sizeof(candidate)))
	{
		std::snprintf(g_appDir, sizeof(g_appDir), "%s", candidate);
		return;
	}

	for (const char* device : kDevices)
	{
		char root[16];
		std::snprintf(root, sizeof(root), "%s:/", device);
		if (dirExists(root))
		{
			std::snprintf(g_appDir, sizeof(g_appDir), "%s:%s", device, kAppSubDir);
			return;
		}
	}

	std::snprintf(g_appDir, sizeof(g_appDir), "sd:%s", kAppSubDir);
}

bool tryMountUsb()
{
#ifdef WII_HAVE_DVM
	return dvmProbeMountDiscIface("usb", &__io_usbstorage, 4, 64) > 0;
#else
	return fatMountSimple("usb", &__io_usbstorage);
#endif
}

bool mountUsbWithRetries()
{
	MC_LOG_INFO("wii", "  no game data yet; waiting for USB...\n");
	for (int attempt = 1; attempt <= kUsbMountAttempts; ++attempt)
	{
		if (tryMountUsb())
		{
			MC_LOG_INFO("wii", "  usb mounted on attempt %d\n", attempt);
			return true;
		}
		if (attempt < kUsbMountAttempts)
			usleep(kUsbRetryDelayUs);
	}
	MC_LOG_INFO("wii", "  usb did not answer after %d attempts\n", kUsbMountAttempts);
	return false;
}

void reportStorageLayout()
{
	MC_LOG_INFO("wii", "  mounted:");
	bool any = false;
	for (const char* device : kDevices)
	{
		char root[16];
		std::snprintf(root, sizeof(root), "%s:/", device);
		if (dirExists(root))
		{
			MC_LOG_INFO("wii", " %s", root);
			any = true;
		}
	}
	if (!any)
		MC_LOG_INFO("wii", " (none)");
	MC_LOG_INFO("wii", "\n");

	MC_LOG_INFO("wii", "  app dir: %s%s\n", g_appDir, g_appDirHasData ? "" : "  *** NO GAME DATA ***");

	char deviceRoot[sizeof(g_appDir)];
	std::snprintf(deviceRoot, sizeof(deviceRoot), "%s", g_appDir);
	if (char* slash = std::strchr(deviceRoot, '/'))
		slash[1] = '\0';

	DIR* root = opendir(deviceRoot);
	if (!root)
	{
		MC_LOG_INFO("wii", "  %s will not open\n", deviceRoot);
	}
	else
	{
		MC_LOG_INFO("wii", "  %s contains:", deviceRoot);
		int shown = 0;
		struct dirent* entry;
		while ((entry = readdir(root)) != nullptr && shown < 8)
		{
			if (entry->d_name[0] == '.')
				continue;
			MC_LOG_INFO("wii", " %s", entry->d_name);
			++shown;
		}
		if (shown == 0)
			MC_LOG_INFO("wii", " (empty)");
		MC_LOG_INFO("wii", "\n");
		closedir(root);
	}

	if (!g_appDirHasData)
	{
		MC_LOG_INFO("wii", "  -> nothing staged on any mounted device.\n");
		MC_LOG_INFO("wii", "     Copy bin\\wii\\sd\\apps\\BetaPlusPlus to an SD card\n");
		MC_LOG_INFO("wii", "     or to a USB drive (either works), and make sure\n");
		MC_LOG_INFO("wii", "     you ran: build wii.bat data\n");
		MC_LOG_INFO("wii", "     Dolphin: Config > Wii > SD Card,\n");
		MC_LOG_INFO("wii", "     tick 'Automatically Sync with Folder',\n");
		MC_LOG_INFO("wii", "     set it to bin\\wii\\sd, then RESTART emulation.\n");
	}
}
} // namespace

bool wiiEnsureStorage()
{
	if (g_storageTried)
		return g_storageReady;
	g_storageTried = true;

	wiiEnsureEarlyVideo();
	MC_LOG_INFO("wii", "mounting storage...\n");
	g_storageReady = fatInitDefault();
	resolveAppDir();

	if (!g_appDirHasData && !dirExists("usb:/") && mountUsbWithRetries())
	{
		g_storageReady = true;
		resolveAppDir();
	}

	if (g_storageReady)
	{
		if (!McLog::openSessionFile(g_appDir))
			MC_LOG_WARN("wii", "could not create debug.log in %s\n", g_appDir);
		MC_LOG_INFO("wii", "storage mounted\n");
		reportStorageLayout();
	}
	else
	{
		MC_LOG_INFO("wii", "*** NO SD/USB STORAGE ***\n");
		MC_LOG_INFO("wii", "Insert an SD card or plug in a USB drive holding\n");
		MC_LOG_INFO("wii", "  apps/BetaPlusPlus/ , or in Dolphin:\n");
		MC_LOG_INFO("wii", "  Config > Wii > SD Card, enable Insert SD Card\n");
		MC_LOG_INFO("wii", "  and Sync with Folder, point it at bin\\wii\\sd\n");
	}

	return g_storageReady;
}

const char* wiiGetAppDir()
{
	wiiEnsureStorage();
	if (g_appDir[0] == '\0')
		std::snprintf(g_appDir, sizeof(g_appDir), "sd:%s", kAppSubDir);
	return g_appDir;
}

bool wiiHasGameData()
{
	wiiEnsureStorage();
	return g_appDirHasData;
}

#endif // WII_PLATFORM
