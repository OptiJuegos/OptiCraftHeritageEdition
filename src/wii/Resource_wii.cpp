// Resource_wii.cpp — Wii implementation of Resource::getResource().
//
// Wii resources live on the libfat filesystem instead of inside the DOL. A DOL
// is loaded whole into the scarce MEM1, so embedding ~10 MB of assets would spend
// memory to save nothing. Resources are read from the SD card or USB drive the
// game was installed on (GameResources::getExeDir() resolves which -- see
// WiiEarlyInit.cpp), which also means they can be replaced without rebuilding.
//
// Callers ask for paths rooted at the resource directory ("/terrain.png",
// "/gui/logo.png"), matching the desktop contract, so the leading slash is
// dropped and the rest is resolved under <app>/data/assets.
//
// The returned stream is owned by the caller, as on every other platform.
#ifdef WII_PLATFORM

#include "java/Resource.h"
#include "java/String.h"
#include "net/minecraft/src/GameResources.h"
#include "wii/WiiEarlyInit.h"
#include "platform/storage/PathUtils.h"

#include <stdexcept>
#include <string>

namespace Resource
{

std::istream *getResource(const jstring &name)
{
	// This runs from static initialisers (SharedConstants, ChatAllowedCharacters)
	// long before main(), so the filesystem cannot be assumed to be up yet.
	wiiEnsureStorage();

	auto input = GameResources::open(static_cast<const std::string &>(name));
	if (!input)
	{
		const std::string path = PlatformStorage::join(
			GameResources::getAssetsDir(), static_cast<const std::string &>(name));
		throw std::runtime_error(
			"Missing game resource:\n" + path +
			"\n\nDolphin setup:\n"
			"1. Open Config > Wii > SD Card.\n"
			"2. Enable 'Automatically Sync with Folder'.\n"
			"3. Select the folder containing apps/BetaPlusPlus.\n"
			"4. Restart emulation.\n\n"
			"Expected: apps/BetaPlusPlus/data/assets\n"
			"Real Wii: copy the complete BetaPlusPlus app folder again.");
	}

	return input.release();
}

} // namespace Resource

#endif // WII_PLATFORM
