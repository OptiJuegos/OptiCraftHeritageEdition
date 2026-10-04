// DsBringup.cpp — Nintendo 3DS toolchain/diagnostic bring-up.
//
// This is NOT the game entry point. It is the smoke test that proves the
// devkitARM/libctru toolchain is wired correctly, and it reports the numbers
// every later decision on this port depends on:
//
//   1. Old 3DS or New 3DS. APT_CheckNew3DS answers it directly, and the answer
//      decides the whole tuning split: the New 3DS runs the same GPU at 268 MHz
//      but gives the app 4 cores at 804 MHz and 124 MB of RAM instead of 64 MB.
//      Minecraft: New Nintendo 3DS Edition shipped New-3DS-only for exactly
//      this; our port targets both, so this is the first fact to see.
//
//   2. Application heap size, from osGetMemRegionSize/Free. The spec sheet says
//      64 MB for apps on Old 3DS (96 MB claimable via the RSF APPMEMTYPE flag),
//      124 MB on New 3DS -- but the spec sheet is the chip, not the budget, and
//      every tuning constant in the 3DS port gets sized against what this
//      prints, the same way the Wii budgets were sized against its real arena.
//
//   3. Byte order and type sizes. The ARM11 is little-endian unlike the Wii's
//      Broadway, so this is the cheap confirmation that nothing in the toolchain
//      did something surprising (Tessellator.cpp and java/IOUtil.cpp are
//      shift-based and expect LE on this target).
//
//   4. SD card access through the sdmc stdio device. The port reads its assets
//      and writes its worlds to sd:/opticraft/, so a failure here is fatal for
//      the real port even though the smoke test can survive it.
//
// The bottom screen shows the console; the top screen animates a colour bar,
// which proves the GPU/display path and gives a crude frame-rate read.
//
// Built only when 3DS_BRINGUP=ON. The normal 3DS build uses src/3ds/main_3ds.cpp.

#include <3ds.h>

#include <cstdio>
#include <cstring>

namespace
{

// fsInit() must pair with fsExit(); services in libctru are ref-counted, but
// keeping the pairing explicit avoids a leak if this file ever grows an exit
// path that outlives a single probe.
bool g_fsReady = false;

void reportModel()
{
	bool isNew3DS = false;
	const Result rc = APT_CheckNew3DS(&isNew3DS);
	if (R_FAILED(rc))
	{
		std::printf("MODEL   *** APT_CheckNew3DS failed: %08lX\n",
		            static_cast<unsigned long>(rc));
		return;
	}

	std::printf("MODEL   %s\n", isNew3DS ? "New 3DS/New 2DS XL (4 cores @ 804 MHz, 124 MB)"
	                                     : "Old 3DS/2DS (2 cores @ 268 MHz, 64 MB)");
	std::printf("        ^ the tuning split in src/3ds/ keys off this at runtime\n");
}

// The application memregion is what malloc sees. Newlib's heap sits on top of
// it, so the usable figure printed here is the honest budget for chunk caches,
// NBT and texture residency -- not the 64/124 MB from the datasheet.
void reportMemory()
{
	const u32 total = osGetMemRegionSize(MEMREGION_APPLICATION);
	const u32 free_ = osGetMemRegionFree(MEMREGION_APPLICATION);

	std::printf("MEM     application region %u KB total, %u KB free\n",
	            total / 1024u, free_ / 1024u);
	std::printf("        (datasheet: 64 MB apps Old3DS / 124 MB New3DS; PS2 had 32 MB total)\n");
}

// Confirms both halves of the endianness story at once: that the compiler
// agrees about the target, and that an actual store/load round-trip behaves the
// way the colour packing and the NBT readers assume.
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

	std::printf("ENDIAN   runtime=%s compile=%s %s\n",
	            bigEndianAtRuntime ? "BE" : "LE",
	            bigEndianAtCompileTime ? "BE" : "LE",
	            (bigEndianAtRuntime == bigEndianAtCompileTime) ? "OK" : "*** MISMATCH ***");

	std::printf("TYPES    float=%u double=%u long=%u long long=%u ptr=%u\n",
	            static_cast<unsigned>(sizeof(float)),
	            static_cast<unsigned>(sizeof(double)),
	            static_cast<unsigned>(sizeof(long)),
	            static_cast<unsigned>(sizeof(long long)),
	            static_cast<unsigned>(sizeof(void *)));
}

// The port reads assets from and writes saves to sd:/opticraft/. Probe the raw
// device with a write-read-delete round trip at the SD root: the directory the
// game wants does not exist yet (creating it is Resources_3DS/StorageBackend's
// job in Phase 1), and a root-level probe works for both the .3dsx and the
// future .cia launch paths.
void reportStorage()
{
	const Result rc = fsInit();
	if (R_FAILED(rc))
	{
		std::printf("STORAGE  *** fsInit() failed: %08lX (no SD access)\n",
		            static_cast<unsigned long>(rc));
		return;
	}
	g_fsReady = true;

	const char *probePath = "sdmc:/opticraft_probe.tmp";
	FILE *f = std::fopen(probePath, "wb");
	if (!f)
	{
		std::printf("STORAGE  *** cannot create %s (no SD card, or write-protected)\n", probePath);
		return;
	}
	std::fputs("ok", f);
	std::fclose(f);

	f = std::fopen(probePath, "rb");
	char buf[4] = {0, 0, 0, 0};
	const bool readBack = (f != nullptr);
	if (f)
	{
		const size_t got = std::fread(buf, 1, 2, f);
		std::fclose(f);
		(void)got;
	}
	std::remove(probePath);

	if (readBack && buf[0] == 'o' && buf[1] == 'k')
		std::printf("STORAGE  sdmc: read/write OK (game data will live in sd:/opticraft/)\n");
	else
		std::printf("STORAGE  *** sdmc: write/read round trip failed\n");
}

void reportScreens()
{
	// The panel geometry never changes on this console; print it so the first
	// ClientPlatformPolicy numbers (400x240 top) are traceable to a report
	// rather than to a comment nobody can verify at runtime.
	std::printf("SCREEN   top 400x240 (5:3, render target), bottom 320x240 (touch)\n");
	std::printf("3D       slider position: %.2f (port renders 2D only, by decision)\n",
	            osGet3DSliderState());
}

} // namespace

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	gfxInitDefault();
	// Console on the BOTTOM screen: the top one is reserved for the animated
	// colour bar, which is the cheapest possible display-path proof.
	consoleInit(GFX_BOTTOM, nullptr);

	std::printf("\nOptiCraft - 3DS bring-up\n");
	std::printf("---------------------------\n\n");

	reportModel();
	reportMemory();
	reportByteOrder();
	reportStorage();
	reportScreens();

	std::printf("\nINPUT    touch the screen / move the Circle Pad to probe\n");
	std::printf("         START or B to exit\n\n");

	u32 frame = 0;
	while (aptMainLoop())
	{
		hidScanInput();
		const u32 kDown = hidKeysDown();
		const u32 kHeld = hidKeysHeld();

		if (kDown & (KEY_START | KEY_B))
			break;

		// Live input probe. The 3DS touch is a single point (resistive panel),
		// so multi-touch buttons later will be region-based, not finger-count
		// based -- worth confirming single-point behaviour right here.
		if (kHeld & KEY_TOUCH)
		{
			touchPosition touch;
			hidTouchRead(&touch);
			std::printf("\x1b[17;1HTOUCH   px=%3u py=%3u   \n",
			            static_cast<unsigned>(touch.px),
			            static_cast<unsigned>(touch.py));
		}
		else
		{
			std::printf("\x1b[17;1H                            \n");
		}

		{
			circlePosition pad;
			hidCircleRead(&pad);
			if (pad.dx != 0 || pad.dy != 0)
				std::printf("\x1b[18;1HPAD     dx=%4d dy=%4d   \n",
				            static_cast<int>(pad.dx), static_cast<int>(pad.dy));
			else
				std::printf("\x1b[18;1H                    \n");
		}

		// Top screen: an animated colour bar. Its speed is the frame rate,
		// its presence proves the display transfer path end to end.
		u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, nullptr, nullptr);
		const int bar = static_cast<int>(frame % 240u);
		for (int y = 0; y < 240; ++y)
		{
			const u8 v = (y >= bar && y < bar + 12) ? 0xFF : 0x20;
			for (int x = 0; x < 400; ++x)
			{
				// The top framebuffer is stored 240 wide x 400 tall and the
				// LCD scans it sideways: panel pixel (x, y) lives at flat
				// offset x*240 + (239 - y), in strides of 3 bytes for BGR8 --
				// the same mapping libctru's console.c draws text with, so
				// the bar lands upright and sweeps top to bottom. (Walking
				// the offset as x*240 + y instead mirrors the panel
				// vertically.)
				const size_t i = (static_cast<size_t>(x) * 240 + (239 - y)) * 3u;
				fb[i + 0] = v;                       // B
				fb[i + 1] = static_cast<u8>(v / 2);   // G
				fb[i + 2] = static_cast<u8>(frame % 256u); // R drifts with time
			}
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		++frame;
	}

	if (g_fsReady)
		fsExit();
	gfxExit();
	// Returning from main hands control back to the Homebrew Launcher (or the
	// emulator's menu).
	return 0;
}
