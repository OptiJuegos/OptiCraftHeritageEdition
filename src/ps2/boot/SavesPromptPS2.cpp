#ifdef PS2_PLATFORM

#include "ps2/boot/SavesPromptPS2.h"
#include "ps2/boot/Ps2BootRenderer.h"
#include "ps2/storage/save/McSavePS2.h"
#include "ps2/storage/save/MassSavePS2.h"

#include <delaythread.h>
#include <libpad.h>
#include <tamtypes.h>

namespace
{

static void sp_bg_draw(const Ps2BootRenderer::Texture& background, float width, float height, int z)
{
    Ps2BootRenderer::clear({0, 0, 0, 0x80});
    if (!Ps2BootRenderer::textureValid(background))
        return;

    Ps2BootRenderer::setTextureRepeat(true);
    Ps2BootRenderer::drawTexture(background,
                                 0.0f, 0.0f, 0.0f, 0.0f,
                                 width, height, width, height,
                                 z,
                                 {0x20, 0x20, 0x20, 0x80});
    Ps2BootRenderer::setTextureRepeat(false);
}

static void sp_str(const Ps2BootRenderer::Font& font,
                   float x, float y, int z,
                   const char* text, float scale, Ps2BootRenderer::Color color)
{
    Ps2BootRenderer::drawText(font, x, y, z, text, scale, color);
}

static void sp_str_cx(const Ps2BootRenderer::Font& font,
                      float centerX, float y, int z,
                      const char* text, float scale, Ps2BootRenderer::Color color)
{
    Ps2BootRenderer::drawTextCentered(font, centerX, y, z, text, scale, color);
}

// ---------------------------------------------------------------------------
// Pad input
// ---------------------------------------------------------------------------

static u16 s_sp_prev = 0xFFFF;
static void sp_pad_flush() { s_sp_prev = 0xFFFF; }

static u16 sp_pad_just() {
    int st = padGetState(0, 0);
    if (st == PAD_STATE_DISCONN || st == PAD_STATE_ERROR) return 0;
    struct padButtonStatus pad = {};
    if (!padRead(0, 0, &pad)) return 0;
    if (pad.mode == 0 || pad.btns == 0x0000) return 0;
    u16 held = (u16)(0xFFFF ^ pad.btns);
    u16 just = held & ~s_sp_prev;
    s_sp_prev = held;
    return just;
}

} // namespace

// ---------------------------------------------------------------------------
// ps2_show_saves_prompt
// ---------------------------------------------------------------------------

SaveLocation ps2_show_saves_prompt() {
    if (!Ps2BootRenderer::ready()) return SAVE_LOC_NONE;

    const float W = static_cast<float>(Ps2BootRenderer::width());
    const float H = static_cast<float>(Ps2BootRenderer::height());
    const int   Z = 0xFFFF;

    bool has_mass = ps2_mass_connected();

    u32 vram_base = Ps2BootRenderer::checkpointVram();
    Ps2BootRenderer::Font font; bool has_font = Ps2BootRenderer::loadFontAsset("assets/font/default.png", font);
    Ps2BootRenderer::Texture bg; Ps2BootRenderer::loadTextureAsset("assets/gui/background.png", bg, Ps2BootRenderer::TextureFilter::Nearest, Ps2BootRenderer::TextureAlphaMode::SourceAlpha);

    const Ps2BootRenderer::Color C_PANEL = {8, 20, 58, 0x80};
    const Ps2BootRenderer::Color C_TITLE = {12, 56, 148, 0x80};
    const Ps2BootRenderer::Color C_BORD = {80, 80, 80, 0x80};
    const Ps2BootRenderer::Color C_SEL = {255, 220, 0, 0x80};
    const Ps2BootRenderer::Color C_WHITE = {255, 255, 255, 0x80};
    const Ps2BootRenderer::Color C_LGRAY = {180, 180, 180, 0x80};
    const Ps2BootRenderer::Color C_MC_ON = {0, 110, 40, 0x80};
    const Ps2BootRenderer::Color C_MC_OFF = {10, 40, 15, 0x80};
    const Ps2BootRenderer::Color C_PD_ON = {160, 80, 0, 0x80};
    const Ps2BootRenderer::Color C_PD_OFF = {55, 28, 0, 0x80};
    const Ps2BootRenderer::Color C_NO_ON = {130, 20, 20, 0x80};
    const Ps2BootRenderer::Color C_NO_OFF = {40, 10, 10, 0x80};

    const float PX = 40.0f, PY = 60.0f;
    const float PW = W - 80.0f, PH = H - 120.0f; // fill most of the screen
    const float TITLE_H = 54.0f;
    const float B = 4.0f;
    const float TTS = 2.5f;   // title scale
    const float NS  = 1.8f;   // body text scale
    const float BTS = 1.6f;   // button label scale
    const float CS = has_font ? (float)font.cell : 8.0f;

    const int   BTN_COUNT = has_mass ? 3 : 2;
    const float BTN_W     = has_mass ? 150.0f : 200.0f;
    const float BTN_H     = 58.0f;
    const float BTN_Y     = PY + PH - BTN_H - 22.0f;

    float btn_cx[3];
    if (!has_mass) {
        btn_cx[0] = PX + PW * 0.25f; // MC
        btn_cx[1] = PX + PW * 0.75f; // No guardar
        btn_cx[2] = 0.0f;
    } else {
        btn_cx[0] = PX + PW * 0.17f; // MC
        btn_cx[1] = PX + PW * 0.50f; // Pendrive
        btn_cx[2] = PX + PW * 0.83f; // No guardar
    }

    const int SEL_MC   = 0;
    const int SEL_MASS = 1;
    const int SEL_NONE = has_mass ? 2 : 1;

    int sel = SEL_MC, done = 0;
    sp_pad_flush();

    while (!done) {
        u16 just = sp_pad_just();
        if ((just & (PAD_LEFT  | PAD_L1)) && sel > 0)               sel--;
        if ((just & (PAD_RIGHT | PAD_R1)) && sel < BTN_COUNT - 1)   sel++;
        if (just & PAD_CROSS)                                        done = 1;
        if (just & (PAD_CIRCLE | PAD_TRIANGLE | PAD_START)) { sel = SEL_NONE; done = 1; }

        sp_bg_draw(bg, W, H, Z);
        Ps2BootRenderer::drawRect(PX-B, PY-B, PX+PW+B, PY+PH+B, Z, C_BORD);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+PH,   Z, C_PANEL);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+TITLE_H, Z, C_TITLE);

        if (has_font) {
            sp_str_cx(font, PX + PW * 0.5f,
                PY + (TITLE_H - CS * TTS) * 0.5f, Z, "SAVES & WORLDS", TTS, C_WHITE);

            float cx = PX + PW * 0.5f, ty = PY + TITLE_H + 22.0f;
            sp_str_cx(font, cx, ty, Z,
                "Donde deseas guardar tus mundos?", NS, C_WHITE);
            ty += CS * NS + 10.0f;
            sp_str_cx(font, cx, ty, Z,
                "Where do you want to save your worlds?", NS, C_LGRAY);
            ty += CS * NS + 10.0f;
            sp_str_cx(font, cx, ty, Z,
                "WORLDS SIZES 400KB!!", NS, C_LGRAY);
            ty += CS * NS + 5.0f;
            sp_str_cx(font, cx, ty, Z,
                "LOS MUNDOS PESAN 400KB!!", NS, C_LGRAY);

            if (!has_mass) {
                ty += CS * NS + 16.0f;
                sp_str_cx(font, cx, ty, Z,
                    "(pendrive undetected)", NS, C_LGRAY);
            }
        }

        // MC button
        {
            bool a = (sel == SEL_MC);
            float bx = btn_cx[0] - BTN_W * 0.5f;
            Ps2BootRenderer::drawRect(bx-B, BTN_Y-B, bx+BTN_W+B, BTN_Y+BTN_H+B, Z, a?C_SEL:C_BORD);
            Ps2BootRenderer::drawRect(bx,   BTN_Y,   bx+BTN_W,   BTN_Y+BTN_H,   Z, a?C_MC_ON:C_MC_OFF);
            if (has_font) sp_str_cx(font, btn_cx[0],
                BTN_Y + (BTN_H - CS * BTS) * 0.5f, Z, "MEM CARD", BTS, C_WHITE);
        }

        // Pendrive button (only if has_mass)
        if (has_mass) {
            bool a = (sel == SEL_MASS);
            float bx = btn_cx[1] - BTN_W * 0.5f;
            Ps2BootRenderer::drawRect(bx-B, BTN_Y-B, bx+BTN_W+B, BTN_Y+BTN_H+B, Z, a?C_SEL:C_BORD);
            Ps2BootRenderer::drawRect(bx,   BTN_Y,   bx+BTN_W,   BTN_Y+BTN_H,   Z, a?C_PD_ON:C_PD_OFF);
            if (has_font) sp_str_cx(font, btn_cx[1],
                BTN_Y + (BTN_H - CS * BTS) * 0.5f, Z, "PENDRIVE", BTS, C_WHITE);
        }

        // No guardar button
        {
            int bi = has_mass ? 2 : 1;
            bool a = (sel == SEL_NONE);
            float bx = btn_cx[bi] - BTN_W * 0.5f;
            Ps2BootRenderer::drawRect(bx-B, BTN_Y-B, bx+BTN_W+B, BTN_Y+BTN_H+B, Z, a?C_SEL:C_BORD);
            Ps2BootRenderer::drawRect(bx,   BTN_Y,   bx+BTN_W,   BTN_Y+BTN_H,   Z, a?C_NO_ON:C_NO_OFF);
            if (has_font) sp_str_cx(font, btn_cx[bi],
                BTN_Y + (BTN_H - CS * BTS) * 0.5f, Z, "NO GUARDAR", BTS, C_WHITE);
        }

        Ps2BootRenderer::present();
    }

    Ps2BootRenderer::destroyFont(font);
    Ps2BootRenderer::destroyTexture(bg);
    Ps2BootRenderer::restoreVram(vram_base);
    Ps2BootRenderer::resetAlpha();

    if (sel == SEL_MC)                  return SAVE_LOC_MC;
    if (has_mass && sel == SEL_MASS)    return SAVE_LOC_MASS;
    return SAVE_LOC_NONE;
}

// ---------------------------------------------------------------------------
// ps2_show_format_prompt
// ---------------------------------------------------------------------------

bool ps2_show_format_prompt() {
    if (!Ps2BootRenderer::ready()) return false;

    const float W = static_cast<float>(Ps2BootRenderer::width());
    const float H = static_cast<float>(Ps2BootRenderer::height());
    const int   Z = 0xFFFF;

    u32 vram_base = Ps2BootRenderer::checkpointVram();
    Ps2BootRenderer::Font font; bool has_font = Ps2BootRenderer::loadFontAsset("assets/font/default.png", font);
    Ps2BootRenderer::Texture bg; Ps2BootRenderer::loadTextureAsset("assets/gui/background.png", bg, Ps2BootRenderer::TextureFilter::Nearest, Ps2BootRenderer::TextureAlphaMode::SourceAlpha);

    const Ps2BootRenderer::Color C_PANEL = {8, 20, 58, 0x80};
    const Ps2BootRenderer::Color C_WARN = {160, 80, 0, 0x80};
    const Ps2BootRenderer::Color C_BORD = {80, 80, 80, 0x80};
    const Ps2BootRenderer::Color C_SEL = {255, 220, 0, 0x80};
    const Ps2BootRenderer::Color C_WHITE = {255, 255, 255, 0x80};
    const Ps2BootRenderer::Color C_LGRAY = {180, 180, 180, 0x80};
    const Ps2BootRenderer::Color C_YELLOW = {255, 220, 0, 0x80};
    const Ps2BootRenderer::Color C_YESON = {130, 20, 20, 0x80};
    const Ps2BootRenderer::Color C_YESOF = {50, 10, 10, 0x80};
    const Ps2BootRenderer::Color C_NOON = {0, 110, 40, 0x80};
    const Ps2BootRenderer::Color C_NOOF = {10, 40, 15, 0x80};

    const float PX = 80.0f, PY = 95.0f;
    const float PW = W - 160.0f, PH = 280.0f;
    const float TITLE_H = 44.0f;
    const float B = 3.0f;
    const float TS = 2.0f, NS = 1.5f, HS = 1.0f;
    const float CS = has_font ? (float)font.cell : 8.0f;

    const float BTN_W = 130.0f, BTN_H = 44.0f;
    const float BTN_Y = PY + PH - BTN_H - 16.0f;
    const float BYX   = PX + PW * 0.25f - BTN_W * 0.5f;
    const float BNX   = PX + PW * 0.75f - BTN_W * 0.5f;

    int sel = 1, done = 0; // default on NO (safe)
    sp_pad_flush();

    while (!done) {
        u16 just = sp_pad_just();
        if (just & (PAD_LEFT  | PAD_L1))                    sel = 0;
        if (just & (PAD_RIGHT | PAD_R1))                    sel = 1;
        if (just & PAD_CROSS)                               done = 1;
        if (just & (PAD_CIRCLE | PAD_TRIANGLE | PAD_START)) { sel = 1; done = 1; }

        sp_bg_draw(bg, W, H, Z);
        Ps2BootRenderer::drawRect(PX-B, PY-B, PX+PW+B, PY+PH+B, Z, C_BORD);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+PH,   Z, C_PANEL);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+TITLE_H, Z, C_WARN);

        if (has_font) {
            sp_str_cx(font, PX + PW * 0.5f,
                PY + (TITLE_H - CS * TS) * 0.5f, Z, "FORMAT MEMORY CARD", TS, C_WHITE);

            float tx = PX + 22.0f, ty = PY + TITLE_H + 18.0f;
            sp_str(font, tx, ty, Z,
                "Memory Card is not formatted.", NS, C_WHITE);
            ty += CS * NS + 5.0f;
            sp_str(font, tx, ty, Z,
                "Do you want to format it?", NS, C_WHITE);
            ty += CS * NS + 16.0f;
            sp_str(font, tx, ty, Z,
                "La Memory Card no esta formateada.", NS, C_LGRAY);
            ty += CS * NS + 5.0f;
            sp_str(font, tx, ty, Z,
                "Quieres formatearla?", NS, C_LGRAY);
            ty += CS * NS + 10.0f;
            sp_str_cx(font, PX + PW * 0.5f, ty, Z,
                "WARNING: erases all data!", NS, C_YELLOW);

            sp_str_cx(font, PX + PW * 0.5f, PY + PH - CS * HS - 6.0f, Z,
                "X:Confirm  O/Start:Cancel", HS, C_LGRAY);
        }

        // YES = red (destructive), NO = green (safe)
        {
            bool a = (sel == 0);
            Ps2BootRenderer::drawRect(BYX-B, BTN_Y-B, BYX+BTN_W+B, BTN_Y+BTN_H+B, Z, a?C_SEL:C_BORD);
            Ps2BootRenderer::drawRect(BYX,   BTN_Y,   BYX+BTN_W,   BTN_Y+BTN_H,   Z, a?C_YESON:C_YESOF);
            if (has_font) sp_str_cx(font,
                BYX + BTN_W * 0.5f, BTN_Y + (BTN_H - CS * TS) * 0.5f, Z, "YES", TS, C_WHITE);
        }
        {
            bool a = (sel == 1);
            Ps2BootRenderer::drawRect(BNX-B, BTN_Y-B, BNX+BTN_W+B, BTN_Y+BTN_H+B, Z, a?C_SEL:C_BORD);
            Ps2BootRenderer::drawRect(BNX,   BTN_Y,   BNX+BTN_W,   BTN_Y+BTN_H,   Z, a?C_NOON:C_NOOF);
            if (has_font) sp_str_cx(font,
                BNX + BTN_W * 0.5f, BTN_Y + (BTN_H - CS * TS) * 0.5f, Z, "NO", TS, C_WHITE);
        }

        Ps2BootRenderer::present();
    }

    Ps2BootRenderer::destroyFont(font);
    Ps2BootRenderer::destroyTexture(bg);
    Ps2BootRenderer::restoreVram(vram_base);
    Ps2BootRenderer::resetAlpha();

    return (sel == 0);
}

// ---------------------------------------------------------------------------
// ps2_do_format_mc — progress screen, delegates operation to McSavePS2
// ---------------------------------------------------------------------------

bool ps2_do_format_mc() {
    if (!Ps2BootRenderer::ready()) return false;

    const float W = static_cast<float>(Ps2BootRenderer::width());
    const float H = static_cast<float>(Ps2BootRenderer::height());
    const int   Z = 0xFFFF;

    u32 vram_base = Ps2BootRenderer::checkpointVram();
    Ps2BootRenderer::Font font; bool has_font = Ps2BootRenderer::loadFontAsset("assets/font/default.png", font);
    Ps2BootRenderer::Texture bg; Ps2BootRenderer::loadTextureAsset("assets/gui/background.png", bg, Ps2BootRenderer::TextureFilter::Nearest, Ps2BootRenderer::TextureAlphaMode::SourceAlpha);

    const Ps2BootRenderer::Color C_PANEL = {8, 20, 58, 0x80};
    const Ps2BootRenderer::Color C_TITLE = {160, 80, 0, 0x80};
    const Ps2BootRenderer::Color C_BORD = {80, 80, 80, 0x80};
    const Ps2BootRenderer::Color C_WHITE = {255, 255, 255, 0x80};
    const Ps2BootRenderer::Color C_LGRAY = {180, 180, 180, 0x80};

    const float PX = 80.0f, PY = 160.0f;
    const float PW = W - 160.0f, PH = 160.0f;
    const float TITLE_H = 44.0f;
    const float B = 3.0f;
    const float TS = 2.0f, NS = 1.5f;
    const float CS = has_font ? (float)font.cell : 8.0f;

    if (!ps2_mc_format_start()) {
        Ps2BootRenderer::destroyFont(font);
        Ps2BootRenderer::destroyTexture(bg);
        Ps2BootRenderer::restoreVram(vram_base);
        return false;
    }

    static const char* const DOTS[] = { ".", "..", "...", "...." };
    int frame = 0;
    bool success = false;

    for (int tick = 0; tick < 600; tick++) {
        int result = -1;
        int status = ps2_mc_format_poll(&result);
        if (status == 1)  { success = (result >= 0); break; }
        if (status == -1) { break; }

        sp_bg_draw(bg, W, H, Z);
        Ps2BootRenderer::drawRect(PX-B, PY-B, PX+PW+B, PY+PH+B, Z, C_BORD);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+PH,   Z, C_PANEL);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+TITLE_H, Z, C_TITLE);

        if (has_font) {
            sp_str_cx(font, PX + PW * 0.5f,
                PY + (TITLE_H - CS * TS) * 0.5f, Z, "FORMATTING", TS, C_WHITE);
            float ty = PY + TITLE_H + 22.0f;
            sp_str_cx(font, PX + PW * 0.5f, ty, Z,
                "Formatting Memory Card", NS, C_WHITE);
            ty += CS * NS + 8.0f;
            sp_str_cx(font, PX + PW * 0.5f, ty, Z,
                DOTS[frame & 3], NS, C_LGRAY);
        }

        Ps2BootRenderer::present();

        frame++;
        DelayThread(50000);
    }

    Ps2BootRenderer::destroyFont(font);
    Ps2BootRenderer::destroyTexture(bg);
    Ps2BootRenderer::restoreVram(vram_base);
    Ps2BootRenderer::resetAlpha();

    return success;
}

#endif // PS2_PLATFORM
