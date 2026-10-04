#pragma once

// Early 3DS lifecycle: SD mount and runtime-data discovery.
//
// Mirrors src/wii/WiiEarlyInit.h: java::File objects and Resource lookups can
// run before main(), so every function here is idempotent and safe to call at
// any time (the mount is taken once, on first use).
//
// The runtime data root is "sdmc:/opticraft" -- the staging tree the 3DS data
// step lays out under bin/3ds/sd/opticraft for the user to copy onto the SD
// card. fsdevMountSdmc() registers the "sdmc" device, so paths must always be
// spelled "sdmc:/...", never "sd:/...".

// Mount the SD card (fsInit + fsdevMountSdmc) on first call and try to create
// the app directory, so a fresh card has somewhere to copy the data into.
// True when "sdmc:" is mounted and usable.
bool dsEnsureStorage();

// Runtime data root, "sdmc:/opticraft". A pure constant: the string is
// meaningful even before the mount exists (opens simply fail until then).
const char* dsGetAppDir();

// True when <dsGetAppDir()>/data/assets/font.txt or <dsGetAppDir()>/assets.pak
// exists -- the loose layout and the scripts/make_pak.py archive.
bool dsHasGameData();

// True on New 3DS/New 2DS XL hardware (cached: one APT_CheckNew3DS round-trip
// on the first call). The 3DS networking stack itself is model-independent --
// soc:U, connect(), select() behave identically on both -- so the only place
// the model matters is performance profiles: the New 3DS's 804 MHz ARM11
// spends measurably less time per map-chunk import than the 268 MHz one, and
// a runtime profile can size per-tick packet budgets accordingly.
bool dsIsNew3DS();
