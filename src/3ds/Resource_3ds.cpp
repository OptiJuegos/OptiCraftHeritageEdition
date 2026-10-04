// Resource_3ds.cpp -- 3DS implementation of Resource::getResource().
//
// Resources live on the SD card instead of inside the .3dsx: a 3DS app image
// is copied into application RAM whole, so embedding ~10 MB of assets would
// spend memory (64 MB total on an Old 3DS) to save nothing. They are read
// from sdmc:/opticraft/ (resolved by GameResources/Resources_3DS), which also
// means they can be replaced or updated without rebuilding the binary.
//
// Callers ask for paths rooted at the resource directory ("/terrain.png",
// "/gui/logo.png"), matching the desktop contract, so the leading slash is
// dropped and the rest is resolved under <app>/data/assets.
//
// The returned stream is owned by the caller, as on every other platform.
#ifdef CTR_PLATFORM

#include "java/Resource.h"
#include "java/String.h"
#include "net/minecraft/src/GameResources.h"
#include "3ds/DsBootstrap.h"
#include "platform/storage/PathUtils.h"

#include <stdexcept>
#include <string>

namespace Resource
{

std::istream *getResource(const jstring &name)
{
	// This runs from static initialisers (SharedConstants,
	// ChatAllowedCharacters) long before main(), so the SD mount cannot be
	// assumed to be up yet -- same reason as Resource_wii.cpp.
	dsEnsureStorage();

	auto input = GameResources::open(static_cast<const std::string &>(name));
	if (!input)
	{
		const std::string path = PlatformStorage::join(
			GameResources::getAssetsDir(), static_cast<const std::string &>(name));
		throw std::runtime_error(
			"Missing game resource:\n" + path +
			"\n\n3DS setup:\n"
			"1. Stage the data tree:\n"
			"   cmake --build <build-dir> --target 3ds-data\n"
			"2. Copy bin/3ds/sd/opticraft/ to the ROOT of your SD card,\n"
			"   giving sdmc:/opticraft/data/assets/...\n"
			"3. Eject and reinsert the SD card, then relaunch.\n\n"
			"Expected: sdmc:/opticraft/data/assets\n"
			"  (or sdmc:/opticraft/assets.pak for a packed install).");
	}

	return input.release();
}

} // namespace Resource

#endif // CTR_PLATFORM
