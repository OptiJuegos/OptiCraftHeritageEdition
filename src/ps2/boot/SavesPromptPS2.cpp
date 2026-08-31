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

// Right-aligned text: drawText/drawTextCentered cover left and center, but
// nothing right-aligns, and the language indicator needs to hug the panel's
// right edge regardless of which language's string is longer.
static void sp_str_rx(const Ps2BootRenderer::Font& font,
                      float rightX, float y, int z,
                      const char* text, float scale, Ps2BootRenderer::Color color)
{
    const float w = Ps2BootRenderer::textWidth(font, text, scale);
    Ps2BootRenderer::drawText(font, rightX - w, y, z, text, scale, color);
}

// A fixed scale is only ever right for one language's phrasing of a given
// line -- "Where do you want to save your worlds?" and its shorter Spanish
// equivalent do not need the same size to both fit the same box. Shrink
// toward (never past) maxWidth using the font's own measured width instead
// of a guessed constant, so both languages fit regardless of string length.
static float sp_fit_scale(const Ps2BootRenderer::Font& font, const char* text,
                          float desiredScale, float maxWidth)
{
    if (desiredScale <= 0.0f || text == nullptr || maxWidth <= 0.0f)
        return desiredScale;
    const float unitWidth = Ps2BootRenderer::textWidth(font, text, 1.0f);
    if (unitWidth <= 0.0f)
        return desiredScale;
    const float fitScale = maxWidth / unitWidth;
    return fitScale < desiredScale ? fitScale : desiredScale;
}

// Cheap drop shadow: the renderer has no blur, so a soft edge is faked with a
// few offset, low-alpha rects instead of one hard-edged one. Color.a is the
// GS 0..128 scale (0x80 = fully opaque), not 0..255, so "low alpha" here
// means small values against that 128 ceiling.
static void sp_soft_shadow(float x0, float y0, float x1, float y1, int z)
{
    const Ps2BootRenderer::Color s1 = {0, 0, 0, 0x14};
    const Ps2BootRenderer::Color s2 = {0, 0, 0, 0x0A};
    Ps2BootRenderer::drawRect(x0 - 6.0f, y0 - 6.0f, x1 + 10.0f, y1 + 10.0f, z, s2);
    Ps2BootRenderer::drawRect(x0 - 3.0f, y0 - 3.0f, x1 + 6.0f,  y1 + 6.0f,  z, s1);
}

// ---------------------------------------------------------------------------
// Localized strings for ps2_show_saves_prompt. English is the default; SELECT
// toggles to Spanish and back. Spanish strings deliberately avoid accents and
// inverted punctuation (?/!) -- the existing prompts in this file already did
// the same, which only makes sense if the bitmap font (assets/font/default.png)
// does not cover those glyphs, so new strings follow the same convention
// rather than risk drawing missing/wrong characters.
struct SavePromptStrings
{
    const char* eyebrow;
    const char* title;
    const char* subtitle;
    const char* noMassNote;
    const char* mcLabel;
    const char* mcDesc;
    const char* massLabel;
    const char* massDesc;
    const char* noneLabel;
    const char* noneDesc;
    const char* hint;
    const char* langToggle; // what SELECT does, shown so it's discoverable
};

static const SavePromptStrings kSavePromptEn = {
    "SAVE LOCATION",
    "Where do you want to save your worlds?",
    "World saves are small, about 400KB each",
    "(USB drive not detected)",
    "MEMORY CARD",
    "Save to the PS2 memory card",
    "USB DRIVE",
    "Save to the USB drive",
    "DON'T SAVE",
    "Play without saving progress",
    "LEFT/RIGHT Choose   X Confirm   START Skip",
    "SELECT: ESPANOL"
};

static const SavePromptStrings kSavePromptEs = {
    "UBICACION DE GUARDADO",
    "Donde quieres guardar tus mundos?",
    "Los mundos ocupan poco, unos 400KB cada uno",
    "(Pendrive no detectado)",
    "MEMORY CARD",
    "Guardar en la memory card del PS2",
    "PENDRIVE USB",
    "Guardar en el pendrive USB",
    "NO GUARDAR",
    "Jugar sin guardar el progreso",
    "IZQ/DER Elegir   X Confirmar   START Omitir",
    "SELECT: ENGLISH"
};

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

    // Flat, dark palette. Color.a is the GS 0..128 scale, not 0..255 -- every
    // color below is fully opaque (0x80) except the shadow layers, which are
    // deliberately low against that 128 ceiling to fake a soft edge (the
    // renderer has no blur primitive).
    const Ps2BootRenderer::Color C_PANEL   = {16, 19, 26, 0x80};
    const Ps2BootRenderer::Color C_ACCENT  = {90, 170, 255, 0x80}; // top accent bar, eyebrow
    const Ps2BootRenderer::Color C_BORD    = {46, 50, 60, 0x80};
    const Ps2BootRenderer::Color C_SEL     = {255, 196, 40, 0x80}; // selection glow
    const Ps2BootRenderer::Color C_WHITE   = {240, 242, 246, 0x80};
    const Ps2BootRenderer::Color C_LGRAY   = {146, 152, 166, 0x80};
    const Ps2BootRenderer::Color C_MC_ACC  = {46, 204, 113, 0x80};
    const Ps2BootRenderer::Color C_MC_FILL = {26, 46, 36, 0x80};
    const Ps2BootRenderer::Color C_MC_SEL  = {30, 92, 58, 0x80};
    const Ps2BootRenderer::Color C_US_ACC  = {66, 158, 235, 0x80};
    const Ps2BootRenderer::Color C_US_FILL = {22, 38, 52, 0x80};
    const Ps2BootRenderer::Color C_US_SEL  = {26, 76, 110, 0x80};
    const Ps2BootRenderer::Color C_NO_ACC  = {200, 90, 90, 0x80};
    const Ps2BootRenderer::Color C_NO_FILL = {44, 28, 28, 0x80};
    const Ps2BootRenderer::Color C_NO_SEL  = {96, 42, 42, 0x80};

    const float PX = 40.0f, PY = 44.0f;
    const float PW = W - 80.0f, PH = H - 88.0f;
    const float B = 3.0f;
    const float ACCENT_H = 4.0f;
    const float ES  = 1.1f;  // eyebrow / hint scale
    const float TTS = 2.3f;  // title scale
    const float NS  = 1.5f;  // subtitle scale
    const float LS  = 1.7f;  // card label scale
    const float HS  = 1.4f;  // helper-line scale
    const float CS = has_font ? (float)font.cell : 8.0f;
    const float cx = PX + PW * 0.5f;

    int s_lang = 0; // 0 = English (default every time the prompt opens), 1 = Spanish
    const SavePromptStrings* T = &kSavePromptEn;

    const int   BTN_COUNT = has_mass ? 3 : 2;
    const float CARD_H    = 92.0f;
    const float CARD_GAP  = 18.0f;
    const float innerX0 = PX + 24.0f, innerX1 = PX + PW - 24.0f;
    const float innerW  = innerX1 - innerX0;
    const float cardW = has_mass ? (innerW - 2.0f * CARD_GAP) / 3.0f
                                  : (innerW - CARD_GAP) / 2.0f;
    const float cardY = PY + PH - 16.0f /*bottom pad*/ - (CS * ES + 8.0f) /*hint*/
                       - 12.0f /*separator gap*/ - (CS * HS + 14.0f) /*helper*/
                       - CARD_H;

    float card_x0[3];
    card_x0[0] = innerX0;
    card_x0[1] = innerX0 + cardW + CARD_GAP;
    card_x0[2] = has_mass ? innerX0 + (cardW + CARD_GAP) * 2.0f : 0.0f;

    const int SEL_MC   = 0;
    const int SEL_MASS = 1;
    const int SEL_NONE = has_mass ? 2 : 1;

    int sel = SEL_MC, done = 0;
    sp_pad_flush();

    while (!done) {
        u16 just = sp_pad_just();
        if ((just & (PAD_LEFT  | PAD_L1)) && sel > 0)               sel--;
        if ((just & (PAD_RIGHT | PAD_R1)) && sel < BTN_COUNT - 1)   sel++;
        if (just & PAD_SELECT)                              s_lang ^= 1;
        if (just & PAD_CROSS)                                        done = 1;
        if (just & (PAD_CIRCLE | PAD_TRIANGLE | PAD_START)) { sel = SEL_NONE; done = 1; }

        T = (s_lang == 0) ? &kSavePromptEn : &kSavePromptEs;

        sp_bg_draw(bg, W, H, Z);
        sp_soft_shadow(PX, PY, PX + PW, PY + PH, Z);
        Ps2BootRenderer::drawRect(PX-B, PY-B, PX+PW+B, PY+PH+B, Z, C_BORD);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+PH,   Z, C_PANEL);
        Ps2BootRenderer::drawRect(PX,   PY,   PX+PW,   PY+ACCENT_H, Z, C_ACCENT);

        // Inset max widths a fixed scale is never guaranteed to fit: two
        // languages phrase the same line at different lengths, so every
        // scale below is fitted per-string via sp_fit_scale rather than
        // assumed from the English string's length alone.
        const float maxTextW = PW - 48.0f;
        const float maxCardTextW = cardW - 16.0f;

        float ty = PY + ACCENT_H + 22.0f;
        if (has_font) {
            sp_str_cx(font, cx, ty, Z, T->eyebrow, ES, C_ACCENT);
            sp_str_rx(font, PX + PW - 4.0f, ty, Z, T->langToggle, ES, C_LGRAY);
            ty += CS * ES + 12.0f;

            const float titleScale = sp_fit_scale(font, T->title, TTS, maxTextW);
            sp_str_cx(font, cx, ty, Z, T->title, titleScale, C_WHITE);
            ty += CS * titleScale + 10.0f;

            const float subtitleScale = sp_fit_scale(font, T->subtitle, NS, maxTextW);
            sp_str_cx(font, cx, ty, Z, T->subtitle, subtitleScale, C_LGRAY);
            ty += CS * subtitleScale + 8.0f;

            if (!has_mass) {
                const float noteScale = sp_fit_scale(font, T->noMassNote, NS, maxTextW);
                sp_str_cx(font, cx, ty, Z, T->noMassNote, noteScale, C_LGRAY);
            }
        }

        // Option cards: left accent bar + fill, selected card gets a bright
        // glow border and a brighter fill of the same hue.
        auto draw_card = [&](float x0, bool selected,
                             Ps2BootRenderer::Color accent,
                             Ps2BootRenderer::Color fill,
                             Ps2BootRenderer::Color fillSel,
                             const char* label) {
            const float x1 = x0 + cardW;
            Ps2BootRenderer::drawRect(x0-B, cardY-B, x1+B, cardY+CARD_H+B, Z,
                selected ? C_SEL : C_BORD);
            Ps2BootRenderer::drawRect(x0, cardY, x1, cardY+CARD_H, Z, selected ? fillSel : fill);
            const float barW = selected ? 6.0f : 4.0f;
            Ps2BootRenderer::drawRect(x0, cardY, x0+barW, cardY+CARD_H, Z, accent);
            if (has_font) {
                const float labelScale = sp_fit_scale(font, label, LS, maxCardTextW);
                const float labelY = cardY + (CARD_H - CS * labelScale) * 0.5f;
                sp_str_cx(font, x0 + cardW * 0.5f, labelY, Z, label, labelScale, C_WHITE);
            }
        };

        draw_card(card_x0[0], sel == SEL_MC, C_MC_ACC, C_MC_FILL, C_MC_SEL, T->mcLabel);
        if (has_mass)
            draw_card(card_x0[1], sel == SEL_MASS, C_US_ACC, C_US_FILL, C_US_SEL, T->massLabel);
        draw_card(card_x0[has_mass ? 2 : 1], sel == SEL_NONE, C_NO_ACC, C_NO_FILL, C_NO_SEL, T->noneLabel);

        // Helper line: describes whichever card is currently highlighted,
        // colored to match that card's accent so the connection is obvious.
        if (has_font) {
            const char* helperText = T->noneDesc;
            Ps2BootRenderer::Color helperColor = C_NO_ACC;
            if (sel == SEL_MC)               { helperText = T->mcDesc;   helperColor = C_MC_ACC; }
            else if (has_mass && sel == SEL_MASS) { helperText = T->massDesc; helperColor = C_US_ACC; }

            const float helperScale = sp_fit_scale(font, helperText, HS, maxTextW);
            float helperY = cardY + CARD_H + 14.0f;
            sp_str_cx(font, cx, helperY, Z, helperText, helperScale, helperColor);

            float sepY = helperY + CS * helperScale + 10.0f;
            Ps2BootRenderer::drawRect(innerX0, sepY, innerX1, sepY + 1.0f, Z, C_BORD);

            const float hintScale = sp_fit_scale(font, T->hint, ES, maxTextW);
            float hintY = sepY + 10.0f;
            sp_str_cx(font, cx, hintY, Z, T->hint, hintScale, C_LGRAY);
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
