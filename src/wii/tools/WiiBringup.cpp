// WiiBringup.cpp — Nintendo Wii toolchain/diagnostic bring-up.
//
// This is NOT the game entry point. It is the smoke test that proves the
// devkitPPC/libogc/libfat toolchain is wired correctly, and it reports the four
// numbers every later decision on this port depends on:
//
//   1. MEM1 / MEM2 arena sizes actually handed to us at runtime. The "24 MB +
//      64 MB" figure is the chip, not the budget: IOS, the framebuffers and
//      libogc's own reservations come off the top first. Everything in
//      src/wii/WiiTuning.h will be sized against what this prints, the same way
//      the PS2 budgets were sized against its real 32 MB.
//   2. Byte order. The Broadway is big-endian, unlike every target this port has
//      run on so far. Tessellator.cpp already has a big-endian branch and
//      java/IOUtil.cpp is shift-based, so NBT and region files should be safe --
//      but "should be" is not "is", and this is the cheapest place to confirm it.
//   3. Which storage devices libfat mounts -- SD card, USB drive, or a GameCube
//      SD adapter -- and which of them holds the staged install, since that is
//      where assets, worlds and screenshots all live.
//   4. The video mode libogc picks (NTSC / PAL / progressive), which decides the
//      render target size and the GUI scale.
//
// Built only when WII_BRINGUP=ON. The normal Wii build uses src/wii/main_wii.cpp.

#include "wii/system/WiiConsole.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <dirent.h>
#include <unistd.h>

#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>

namespace
{

GXRModeObj *videoMode = nullptr;
void *frameBuffer = nullptr;

// libogc wants the framebuffer address as an uncached (K1) pointer: the VI DMAs
// straight out of main memory and never snoops the CPU's data cache, so writing
// console text through a cached mapping would leave it sitting in L1.
void initVideoAndConsole()
{
	VIDEO_Init();
	videoMode = VIDEO_GetPreferredMode(nullptr);

	frameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(videoMode));
	console_init(frameBuffer, 20, 20, videoMode->fbWidth, videoMode->xfbHeight,
	             videoMode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(videoMode);
	VIDEO_SetNextFramebuffer(frameBuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	// On a progressive mode the flush lands on the following field, so a second
	// wait is needed before the first frame is actually on screen.
	if (videoMode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

// libogc packs the video mode as VI_TVMODE(fmt, scan) == (fmt << 2) + scan and
// ships no accessor macros to take it apart again, so unpack it here.
inline u32 tvModeFormat(u32 viTVMode) { return viTVMode >> 2; }
inline u32 tvModeScan(u32 viTVMode)   { return viTVMode & 3; }

const char *tvModeName(u32 viTVMode)
{
	switch (tvModeFormat(viTVMode))
	{
		case VI_NTSC:     return "NTSC";
		case VI_PAL:      return "PAL";
		case VI_MPAL:     return "MPAL";
		case VI_EURGB60:  return "EURGB60";
		default:          return "unknown";
	}
}

const char *scanModeName(u32 viTVMode)
{
	switch (tvModeScan(viTVMode))
	{
		case VI_INTERLACE:     return "interlaced";
		case VI_NON_INTERLACE: return "non-interlaced";
		case VI_PROGRESSIVE:   return "progressive";
		default:               return "unknown-scan";
	}
}

void reportVideo()
{
	WiiConsole::write("VIDEO   %s %s  fb %dx%d  vi %dx%d\n",
	            tvModeName(videoMode->viTVMode),
	            scanModeName(videoMode->viTVMode),
	            videoMode->fbWidth, videoMode->xfbHeight,
	            videoMode->viWidth, videoMode->viHeight);
}

// MEM1 is 1T-SRAM: small, low latency, and where devkitPPC's newlib heap lives
// by default. MEM2 is GDDR3: much larger, noticeably slower, and NOT part of the
// malloc heap -- it has to be claimed from the arena by hand. The intended split
// for the game is MEM1 for meshes/textures/render state and MEM2 for chunks,
// NBT and the world cache.
void reportMemory()
{
	const u32 arena1 = static_cast<u32>(SYS_GetArena1Size());
	const u32 arena2 = static_cast<u32>(SYS_GetArena2Size());

	WiiConsole::write("MEM1    %u KB free  [%p .. %p]\n",
	            arena1 / 1024u, SYS_GetArena1Lo(), SYS_GetArena1Hi());
	WiiConsole::write("MEM2    %u KB free  [%p .. %p]\n",
	            arena2 / 1024u, SYS_GetArena2Lo(), SYS_GetArena2Hi());
	WiiConsole::write("TOTAL   %u KB  (PS2 had 32768 KB for everything)\n",
	            (arena1 + arena2) / 1024u);
}

// Confirms both halves of the endianness story at once: that the compiler agrees
// the target is big-endian, and that an actual store/load round-trip behaves the
// way Tessellator's colour packing and the NBT readers assume.
void reportByteOrder()
{
	const unsigned int probe = 0x01020304u;
	unsigned char bytes[4];
	std::memcpy(bytes, &probe, sizeof(bytes));
	const bool bigEndianAtRuntime = (bytes[0] == 0x01);

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
	const bool bigEndianAtCompileTime = (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
#else
	const bool bigEndianAtCompileTime = false;
#endif

	WiiConsole::write("ENDIAN  runtime=%s compile=%s %s\n",
	            bigEndianAtRuntime ? "BE" : "LE",
	            bigEndianAtCompileTime ? "BE" : "LE",
	            (bigEndianAtRuntime == bigEndianAtCompileTime) ? "OK" : "*** MISMATCH ***");

	// The PS2 had to emulate `double` in software, which is what made worldgen
	// spike (PS2_FLOAT_ORE_VEINS). The 750CL does it in hardware; print the
	// sizes so a surprise ABI difference shows up here and not in a save file.
	WiiConsole::write("TYPES   float=%u double=%u long=%u long long=%u ptr=%u\n",
	            (unsigned)sizeof(float), (unsigned)sizeof(double),
	            (unsigned)sizeof(long), (unsigned)sizeof(long long),
	            (unsigned)sizeof(void *));
}

// One fatInitDefault() mounts every device libfat knows: SD, USB, and the two
// GameCube SD adapters. SD versus USB is therefore not a choice this port makes
// at build time -- the install is *found*, not assumed, and this smoke test
// reports the same set of devices the real boot path in WiiEarlyStorage.cpp walks.
//
// Everything the game needs -- assets, worlds, options -- lives under the
// directory found here, so a failure is fatal for the real port even though the
// smoke test survives it.
void reportStorage()
{
	if (!fatInitDefault())
	{
		WiiConsole::write("STORAGE *** fatInitDefault() FAILED -- no SD/USB ***\n");
		return;
	}

	// The Homebrew Channel and wiiload pass argv[0], and libfat chdir()s to its
	// directory, so the cwd names the device this build was launched from. That
	// is the primary way the game answers "SD or USB?".
	char cwd[128];
	if (getcwd(cwd, sizeof(cwd)))
		WiiConsole::write("STORAGE launched from %s\n", cwd);
	else
		WiiConsole::write("STORAGE launched from (no argv -- direct .elf boot?)\n");

	// libfat exposes no "which devices did I mount" query, so probe the mount
	// points it can create. usb2 is a USB drive whose FAT partition is not the
	// first one, which is common on drives shared with WBFS.
	const char *devices[] = { "sd", "usb", "usb2", "carda", "cardb" };
	const char *foundAppDir = nullptr;
	char appDir[64];

	for (const char *device : devices)
	{
		char root[16];
		std::snprintf(root, sizeof(root), "%s:/", device);

		DIR *dir = opendir(root);
		if (!dir)
		{
			WiiConsole::write("STORAGE %-7s -\n", root);
			continue;
		}
		closedir(dir);

		// An install is only real if data/ was staged into it; a bare directory
		// of the right name is the classic "copied boot.dol, forgot the data"
		// mistake and would otherwise report as success.
		char probe[128];
		std::snprintf(probe, sizeof(probe),
		              "%s:/apps/BetaPlusPlus/data/assets/font.txt", device);
		FILE *f = std::fopen(probe, "rb");
		if (f) std::fclose(f);

		WiiConsole::write("STORAGE %-7s mounted%s\n", root, f ? "  <- game data here" : "");

		if (f && !foundAppDir)
		{
			std::snprintf(appDir, sizeof(appDir), "%s:/apps/BetaPlusPlus", device);
			foundAppDir = appDir;
		}
	}

	if (!foundAppDir)
	{
		WiiConsole::write("STORAGE *** no apps/BetaPlusPlus/data on any device ***\n");
		WiiConsole::write("STORAGE copy bin/wii/sd/* to an SD card or USB drive\n");
		return;
	}

	// A write probe matters more than a read one: worlds are saved here, and a
	// read-only or full card fails at exactly this call.
	char probePath[128];
	std::snprintf(probePath, sizeof(probePath), "%s/write_probe.tmp", foundAppDir);
	FILE *probe = std::fopen(probePath, "wb");
	if (probe)
	{
		std::fputs("ok", probe);
		std::fclose(probe);
		std::remove(probePath);
		WiiConsole::write("STORAGE write probe OK (%s)\n", foundAppDir);
	}
	else
	{
		WiiConsole::write("STORAGE *** write probe FAILED (read-only or full) ***\n");
	}
}

} // namespace

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	initVideoAndConsole();
	WPAD_Init();

	WiiConsole::write("\n\x1b[2;0H");
	WiiConsole::write("BetaPlusPlus - Wii bring-up\n");
	WiiConsole::write("---------------------------\n\n");

	reportVideo();
	reportMemory();
	reportByteOrder();
	reportStorage();

	WiiConsole::write("\nHOME (Wiimote) or START (GameCube pad) to exit.\n");

	// PAD_Init covers a GameCube controller in port 1, which is the easier pad to
	// test with under Dolphin.
	PAD_Init();

	while (true)
	{
		WPAD_ScanPads();
		PAD_ScanPads();

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
			break;
		if (PAD_ButtonsDown(0) & PAD_BUTTON_START)
			break;

		VIDEO_WaitVSync();
	}

	// Returning from main hands control back to the Homebrew Channel.
	return 0;
}
