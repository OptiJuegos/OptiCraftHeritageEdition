#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/input/Ps2PadRuntime.h"
#include "ps2/input/Ps2PadState.h"
#include "ps2/system/Ps2Iop.h"

#include <delaythread.h>
#include <kernel.h>
#include <libpad.h>
#include <stdio.h>
#include <tamtypes.h>

namespace {

static const int CENTER_SAMPLES = 16;

struct PortState {
    u8 buffer[256] __attribute__((aligned(64)));
    bool open;
    bool analogTried;
    u16 previousButtons;
    bool calibrated;
    unsigned char centerLX, centerLY, centerRX, centerRY;
    int samples;
    int sumLX, sumLY, sumRX, sumRY;
    int badReads;
};

static PortState s_ports[2] = {};
static volatile bool s_ready = false;
static volatile bool s_initDone = false;
static u8 s_padInitStack[8192] __attribute__((aligned(16)));
static u8 s_initStack[8192] __attribute__((aligned(16)));

static bool axisCanCalibrate(unsigned char v) { return v >= 96 && v <= 160; }

static void resetCalibration(PortState& p) {
    p.calibrated = false;
    p.centerLX = p.centerLY = p.centerRX = p.centerRY = 128;
    p.samples = 0;
    p.sumLX = p.sumLY = p.sumRX = p.sumRY = 0;
}

static void calibrate(int port, PortState& p, const padButtonStatus& pad) {
    if (p.calibrated || !axisCanCalibrate(pad.ljoy_h) || !axisCanCalibrate(pad.ljoy_v)) return;
    p.sumLX += pad.ljoy_h;
    p.sumLY += pad.ljoy_v;
    p.sumRX += axisCanCalibrate(pad.rjoy_h) ? pad.rjoy_h : 128;
    p.sumRY += axisCanCalibrate(pad.rjoy_v) ? pad.rjoy_v : 128;
    if (++p.samples < CENTER_SAMPLES) return;
    p.centerLX = (unsigned char)(p.sumLX / p.samples);
    p.centerLY = (unsigned char)(p.sumLY / p.samples);
    p.centerRX = (unsigned char)(p.sumRX / p.samples);
    p.centerRY = (unsigned char)(p.sumRY / p.samples);
    p.calibrated = true;
    MC_LOG_INFO("input", "[PS2] pad%d calibrated: L=%u,%u R=%u,%u (%d samples)\n", port,
           (unsigned)p.centerLX, (unsigned)p.centerLY,
           (unsigned)p.centerRX, (unsigned)p.centerRY, p.samples);
}

static float normalize(unsigned char value, unsigned char center) {
    int c = (int)value - (int)center;
    if (c < -127) c = -127;
    if (c > 127) c = 127;
    return (float)c / 127.0f;
}


static void waitStable(int port) {
    for (int i = 0; i < 500; ++i) {
        if (padGetState(port, 0) == PAD_STATE_STABLE) return;
        DelayThread(1000);
    }
}

static void waitRequest(int port) {
    for (int i = 0; i < 500; ++i) {
        int state = padGetReqState(port, 0);
        if (state == PAD_RSTAT_COMPLETE || state == PAD_RSTAT_FAILED) return;
        DelayThread(1000);
    }
}

static bool openPort(int port) {
    PortState& p = s_ports[port];
    resetCalibration(p);
    if (padPortOpen(port, 0, p.buffer) == 0) return false;
    p.open = true;
    if (port == 0) {
        waitStable(port);
        bool analog = false;
        for (int attempt = 0; attempt < 30 && !analog; ++attempt) {
            padSetMainMode(port, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
            waitRequest(port);
            waitStable(port);
            analog = padInfoMode(port, 0, PAD_MODECURID, 0) == PAD_TYPE_DUALSHOCK;
            if (!analog) DelayThread(50000);
        }
        MC_LOG_ERROR("input", "[PS2] analog %s\n", analog ? "ENABLED" : "FAILED");
    }
    return true;
}

static void initThread(void*) {
    Ps2Iop::ensureRomModule(Ps2Iop::RomModule::Pad);
    if (padInit(0) == 1 && openPort(0)) s_ready = true;
    else MC_LOG_ERROR("input", "[PS2] pad init failed\n");
    if (openPort(1)) MC_LOG_INFO("input", "[PS2] pad port 1 open\n");
    s_initDone = true;
    ExitThread();
}

static void disconnect(int port) {
    PortState& p = s_ports[port];
    p.previousButtons = 0;
    p.analogTried = false;
    resetCalibration(p);
    ps2PadDisconnect(port);
}

static void pollPort(int port) {
    PortState& p = s_ports[port];
    if (!p.open || (port == 0 && !s_ready)) {
        if (++p.badReads >= 5) { p.badReads = 0; disconnect(port); }
        return;
    }
    int state = padGetState(port, 0);
    if (state == PAD_STATE_DISCONN || state == PAD_STATE_ERROR) {
        if (++p.badReads >= 5) { p.badReads = 0; disconnect(port); }
        return;
    }
    if (port == 1 && !p.analogTried && state == PAD_STATE_STABLE) {
        padSetMainMode(port, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
        p.analogTried = true;
    }
    padButtonStatus pad = {};
    if (!padRead(port, 0, &pad) || pad.mode == 0 || pad.btns == 0x0000) {
        if (++p.badReads >= 5) { p.badReads = 0; disconnect(port); }
        return;
    }
    p.badReads = 0;
    calibrate(port, p, pad);
    const u16 held = (u16)(0xFFFF ^ pad.btns);
    const u16 pressed = held & ~p.previousButtons;
    const u16 released = p.previousButtons & ~held;
    p.previousButtons = held;
    float lx = 0, ly = 0, rx = 0, ry = 0;
    if (p.calibrated) {
        lx = normalize(pad.ljoy_h, p.centerLX);
        ly = normalize(pad.ljoy_v, p.centerLY);
        rx = normalize(pad.rjoy_h, p.centerRX);
        ry = normalize(pad.rjoy_v, p.centerRY);
    }
    ps2PadUpdateSnapshot(port, true, lx, ly, rx, ry, held, pressed, released);
}

} // namespace

namespace Ps2PadRuntime {

void initialize() {
    ee_thread_t th = {};
    th.func = (void*)initThread;
    th.stack = s_initStack;
    th.stack_size = sizeof(s_initStack);
    th.gp_reg = &_gp;
    th.initial_priority = 100;
    int tid = CreateThread(&th);
    if (tid >= 0) { StartThread(tid, NULL); MC_LOG_INFO("input", "[PS2] pad init thread %d\n", tid); }
    else MC_LOG_ERROR("input", "[PS2] CreateThread failed: %d\n", tid);
}

bool waitUntilReady() {
    for (int i = 0; i < 12000; ++i) {
        if (s_initDone) return true;
        DelayThread(1000);
    }
    MC_LOG_INFO("input", "[PS2] pad init timeout\n");
    return false;
}

void poll() {
    pollPort(0);
    pollPort(1);
}

} // namespace Ps2PadRuntime

#endif
