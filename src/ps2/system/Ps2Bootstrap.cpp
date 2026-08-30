#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/system/Ps2Bootstrap.h"

#include "ps2/boot/Ps2BootScreen.h"
#include "ps2/boot/startup_ps2.h"
#include "ps2/input/Ps2Input.h"
#include "ps2/render/Ps2Graphics.h"
#include "ps2/storage/assets/Ps2Assets.h"
#include "ps2/storage/save/Ps2SaveSetup.h"
#include "ps2/system/Ps2EarlyCrash.h"
#include "ps2/system/Ps2Iop.h"
#include "ps2/system/Ps2LibcLocks.h"
#include "platform/storage/PathUtils.h"

#include <cstdio>
#include <string>

extern "C" void ps2_dbg_init_memory();

namespace Ps2Bootstrap
{

bool initialize(int argc, char** argv)
{
    Ps2LibcLocks::initialize();
    // Before anything that can throw. Until this is installed an escaped
    // exception aborts silently, which on this console is indistinguishable
    // from a freeze.
    Ps2EarlyCrash::install();
    ps2_dbg_init_memory();
    MC_LOG_INFO("platform", "[PS2] main() reached\n");

    Ps2Iop::initFileServices();
    MC_LOG_DEBUG("ps2.boot", "[PS2] bootstrap: IOP file services ready\n");

    Ps2Graphics::initialize();
    MC_LOG_DEBUG("ps2.boot", "[PS2] bootstrap: graphics and VU ready\n");
    MC_LOG_INFO("platform", "[PS2] graphics OK - %dx%d\n", Ps2Graphics::width(), Ps2Graphics::height());
    ps2BootFlash(255, 0, 0);
    MC_LOG_DEBUG("ps2.boot", "[PS2] RED flash done (if you saw RED, GS output works)\n");

    Ps2Input::initialize();
    Ps2Input::waitUntilReady();
    ps2BootFlash(0, 0, 255);
    MC_LOG_DEBUG("ps2.boot", "[PS2] pad init done (BLUE flash)\n");

    Ps2Assets::init(argc, argv);
    if (!Ps2Assets::available())
    {
        MC_LOG_ERROR("platform", "[PS2] FATAL: game data not found. Place data/ beside the ELF\n");
        MC_LOG_INFO("platform", "[PS2]        or under a supported fallback install root.\n");
        MC_LOG_DEBUG("ps2.boot", "[PS2] FATAL: no data/ directory on any readable device\n");
        ps2HaltNoData();
    }

    const std::string dataDir = PlatformStorage::normalizeSlashes(Ps2Assets::dataDir());
    std::string installDir = PlatformStorage::parent(dataDir);
    // host:data has no slash, unlike mass:/data. Preserve the PS2 device root
    // instead of losing it to parent().
    if (installDir.empty())
    {
        const std::size_t colon = dataDir.find(':');
        if (colon != std::string::npos)
            installDir = dataDir.substr(0, colon + 1);
    }
    if (!McLog::openSessionFile(installDir.c_str()))
        MC_LOG_WARN("platform", "[PS2] could not create debug.log in %s\n", installDir.c_str());

    MC_LOG_INFO("platform", "[PS2] assets: %s (%s)\n", Ps2Assets::dataDir(), Ps2Assets::sourceName());
    MC_LOG_DEBUG("ps2.boot", "[PS2] assets resolved\n");

    ps2_run_startup_logos();
    ps2BootFlash(255, 255, 0);
    MC_LOG_DEBUG("ps2.boot", "[PS2] startup logos done (YELLOW flash)\n");

    Ps2SaveSetup::selectStorage();
    return true;
}

[[noreturn]] void finishGame()
{
    MC_LOG_ERROR("platform", "[PS2] ERROR: Minecraft::start returned; halting instead of returning to browser\n");
    ps2HaltBlack();
}

} // namespace Ps2Bootstrap

#endif // PS2_PLATFORM
