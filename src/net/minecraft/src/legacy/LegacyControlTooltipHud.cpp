#include "net/minecraft/src/UiStrings.h"
#include "LegacyControlTooltipHud.h"

#include "LegacyHudLayout.h"
#include "LegacyMenuHints.h"

#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/Minecraft.h"
#include "platform/LegacyControlPromptBackend.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"

#include <algorithm>

namespace
{
constexpr int_t PROMPT_COUNT = 4;

std::string actionName(LegacyControlAction action)
{
    switch (action)
    {
    case LegacyControlAction::Inventory: return uiText("Inventory");
    case LegacyControlAction::Drop: return uiText("Drop");
    case LegacyControlAction::Jump: return uiText("Jump");
    case LegacyControlAction::Attack: return uiText("Attack");
    case LegacyControlAction::Use: return uiText("Use");
    }
    return "";
}

LegacyControlAction actionAt(int_t index)
{
    switch (index)
    {
    case 0: return LegacyControlAction::Inventory;
    case 1: return LegacyControlAction::Jump;
    case 2: return LegacyControlAction::Attack;
    default: return LegacyControlAction::Use;
    }
}

std::string prompt(const GameSettings &settings, LegacyControlAction action)
{
    return legacyFormatControlPrompt(legacyControlPromptLabel(settings, action), actionName(action));
}

int_t contentWidth(FontRenderer *font, const std::string *texts, int_t count)
{
    int_t width = 0;
    for (int_t i = 0; i < count; ++i)
    {
        if (!texts[i].empty())
            width += font->getStringWidth(texts[i]);
    }
    return width;
}

int_t visiblePromptCount(const std::string *texts, int_t count)
{
    int_t visible = 0;
    for (int_t i = 0; i < count; ++i)
    {
        if (!texts[i].empty())
            ++visible;
    }
    return visible;
}

// The row this HUD draws is a pure function of the four displayed control labels the
// platform backend reports and the screen size, and neither moves while the
// player is simply playing. Rebuilding it per frame cost four formatted
// std::strings plus eight FontRenderer::getStringWidth calls, and every one of
// those allocates a jstring and a vector<char_t> to walk the text -- roughly
// fifty heap operations a frame on an arena this fragmentation-sensitive, all
// to produce a byte-identical row.
//
// Measured 2026-09-07 on hardware: with the row already batched into one
// Tessellator draw, hudHints still held at 10.3 ms of a 12.5 ms HUD phase in a
// 22 ms render, i.e. batching the glyph submission had not moved it. The setup
// is what the phase is made of.
//
// The cache is keyed on the LABELS rather than on the bindings behind them,
// because the three backends disagree about what a label depends on: PS2 reads
// GameSettings for the displayed actions, PC reads them from settings, and Wii
// ignores GameSettings entirely and follows the active pad
// family, which changes when the player picks up a different controller. Keying
// on backend output is therefore the only form that is correct on all three.
// Asking for the labels every frame is what makes that affordable: they are
// short enough ("R2", uiText("Square"), "E") to live inside the string's own storage,
// so the comparison costs no allocation, while a hit still skips the
// formatting, the width sweep and the layout.
struct PromptRow
{
    std::string language;
    std::string labels[PROMPT_COUNT];
    std::string texts[PROMPT_COUNT];
    int_t x[PROMPT_COUNT];
    int_t y;
    int_t screenWidth;
    int_t screenHeight;
    FontRenderer *fontOwner;
    unsigned int fontRevision;
    RenderCapturedMesh captured;
    bool capturedValid;
    bool valid;

    PromptRow() : y(0), screenWidth(-1), screenHeight(-1), fontOwner(nullptr),
                  fontRevision(0), capturedValid(false), valid(false)
    {
        for (int_t i = 0; i < PROMPT_COUNT; ++i)
            x[i] = 0;
    }
};

PromptRow s_row;

// True when the cached row can be drawn as it stands. The labels are read into
// the row either way, so a miss leaves them already refreshed for the rebuild.
bool refreshRowKey(const GameSettings &settings, FontRenderer *font, PromptRow &row,
                   int_t screenWidth, int_t screenHeight)
{
    const unsigned int fontRevision = font != nullptr ? font->getTextCacheRevision() : 0u;
    bool hit = row.valid && row.screenWidth == screenWidth && row.screenHeight == screenHeight &&
        row.fontOwner == font && row.fontRevision == fontRevision && row.language == settings.language;
    row.language = settings.language;
    for (int_t i = 0; i < PROMPT_COUNT; ++i)
    {
        std::string label = legacyControlPromptLabel(settings, actionAt(i));
        if (hit && label != row.labels[i])
            hit = false;
        row.labels[i] = label;
    }
    return hit;
}

void rebuildRow(const GameSettings &settings, FontRenderer *font, PromptRow &row,
                int_t screenWidth, int_t screenHeight)
{
    for (int_t i = 0; i < PROMPT_COUNT; ++i)
    {
        row.texts[i] = prompt(settings, actionAt(i));
        row.x[i] = 0;
    }

    row.y = legacyHintRowY(screenHeight);
    row.screenWidth = screenWidth;
    row.screenHeight = screenHeight;
    row.fontOwner = font;
    row.fontRevision = font != nullptr ? font->getTextCacheRevision() : 0u;
    row.captured.clear();
    row.capturedValid = false;
    row.valid = true;

    const int_t visible = visiblePromptCount(row.texts, PROMPT_COUNT);
    if (visible <= 0)
        return;

    const int_t textWidth = contentWidth(font, row.texts, PROMPT_COUNT);
    const int_t availableWidth = std::max<int_t>(0, screenWidth - LEGACY_HINT_MARGIN * 2);
    int_t gap = LEGACY_HINT_GAP;
    if (visible > 1 && textWidth + gap * (visible - 1) > availableWidth)
        gap = std::max<int_t>(1, (availableWidth - textWidth) / (visible - 1));

    const int_t rowWidth = textWidth + gap * std::max<int_t>(0, visible - 1);
    int_t x = std::max<int_t>(LEGACY_HINT_MARGIN, (screenWidth - rowWidth) / 2);
    for (int_t i = 0; i < PROMPT_COUNT; ++i)
    {
        if (row.texts[i].empty())
            continue;
        row.x[i] = x;
        x += font->getStringWidth(row.texts[i]) + gap;
    }
}

void emitRow(FontRenderer *font, const PromptRow &row)
{
    for (int_t i = 0; i < PROMPT_COUNT; ++i)
    {
        if (row.texts[i].empty())
            continue;
        font->drawStringWithShadow(row.texts[i], row.x[i], row.y, 0xffffff);
    }
}

bool captureRow(FontRenderer *font, PromptRow &row)
{
#if PLATFORM_PS2 && PS2_CACHE_LEGACY_HINT_TEXT
    if (font == nullptr || visiblePromptCount(row.texts, PROMPT_COUNT) <= 0)
        return false;

    font->beginTextBatch();
    emitRow(font, row);
    row.capturedValid = font->captureTextBatch(row.captured);
    return row.capturedValid;
#else
    (void)font;
    (void)row;
    return false;
#endif
}

void drawRowImmediate(FontRenderer *font, const PromptRow &row)
{
    // One batch for the whole row. The cached path below removes this glyph
    // tessellation entirely on PS2; this remains the correctness fallback.
    font->beginTextBatch();
    emitRow(font, row);
    font->endTextBatch();
}

void drawRow(FontRenderer *font, PromptRow &row)
{
    if (visiblePromptCount(row.texts, PROMPT_COUNT) <= 0)
        return;

#if PLATFORM_PS2 && PS2_CACHE_LEGACY_HINT_TEXT
    if (!row.capturedValid)
        (void)captureRow(font, row);
    if (row.capturedValid && font->drawCapturedText(row.captured))
        return;
#endif

    drawRowImmediate(font, row);
}
}

void LegacyControlTooltipHud::render(Minecraft *mc, int_t screenWidth, int_t screenHeight)
{
    if (mc == nullptr || mc->gameSettings == nullptr || mc->fontRenderer == nullptr)
        return;
    if (!mc->gameSettings->legacyUI || mc->gameSettings->hideGUI || mc->currentScreen != nullptr)
        return;
    if (mc->theWorld == nullptr || mc->thePlayer == nullptr)
        return;

    const GameSettings &settings = *mc->gameSettings;
    FontRenderer *font = mc->fontRenderer;

    if (!refreshRowKey(settings, font, s_row, screenWidth, screenHeight))
    {
        rebuildRow(settings, font, s_row, screenWidth, screenHeight);
#if PLATFORM_PS2 && PS2_CACHE_LEGACY_HINT_TEXT
        (void)captureRow(font, s_row);
#endif
    }

    drawRow(font, s_row);
}
