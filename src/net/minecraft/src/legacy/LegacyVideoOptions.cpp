#include "net/minecraft/src/UiStrings.h"
#include "LegacyVideoOptions.h"

#include "LegacyGuiButton.h"
#include "LegacyOptionCheckbox.h"
#include "LegacyOptionSlider.h"
#include "LegacyOptionState.h"
#include "LegacyOptionMetrics.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/ScaledResolution.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformUserSettings.h"

namespace
{
enum LegacyVideoButtonId
{
    BUTTON_RENDER_DISTANCE = 300,
    BUTTON_GRAPHICS = 301,
    BUTTON_SMOOTH_LIGHTING = 302,
    BUTTON_VIEW_BOBBING = 303,
    BUTTON_CLOUDS = 304,
    BUTTON_FOG = 305,
    BUTTON_BRIGHTNESS = 306,
    BUTTON_DEFLICKER = 307,
    BUTTON_ASPECT_RATIO = 308,
    BUTTON_DONE = 399
};

}

LegacyVideoOptions::LegacyVideoOptions(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue), graphicsCheckbox(nullptr), smoothLightingCheckbox(nullptr),
      viewBobbingCheckbox(nullptr), cloudsCheckbox(nullptr), fogCheckbox(nullptr), deflickerCheckbox(nullptr)
{
}

void LegacyVideoOptions::initGui()
{
#if PLATFORM_WII
    const int_t rowCount = 7;
#elif PLATFORM_PS2
    const int_t rowCount = 7;
#else
    const int_t rowCount = 8;
#endif
    configureLegacyLayout(rowCount, true, LegacyOptionsLayoutPreset::Compact);
    const int_t x = legacyLayout.contentX;
    const int_t w = legacyLayout.contentWidth;
    const int_t h = legacyLayout.rowHeight;
    int_t row = 0;

    graphicsCheckbox = new LegacyOptionCheckbox(BUTTON_GRAPHICS, x, legacyLayout.rowY(row++), w, h,
        uiText("Fancy Graphics"), settings->fancyGraphics);
    smoothLightingCheckbox = new LegacyOptionCheckbox(BUTTON_SMOOTH_LIGHTING, x, legacyLayout.rowY(row++), w, h,
        uiText("Smooth Lighting"), legacySmoothLightingChecked(settings->ofAoLevel));
    viewBobbingCheckbox = new LegacyOptionCheckbox(BUTTON_VIEW_BOBBING, x, legacyLayout.rowY(row++), w, h,
        uiText("View Bobbing"), settings->viewBobbing);

    controlList.push_back(graphicsCheckbox);
    controlList.push_back(smoothLightingCheckbox);
    controlList.push_back(viewBobbingCheckbox);

#if !(PLATFORM_PS2 || PLATFORM_WII)
    cloudsCheckbox = new LegacyOptionCheckbox(BUTTON_CLOUDS, x, legacyLayout.rowY(row++), w, h,
        uiText("Render Clouds"), legacyCloudsChecked(settings->ofClouds));
    fogCheckbox = new LegacyOptionCheckbox(BUTTON_FOG, x, legacyLayout.rowY(row++), w, h,
        uiText("Fog"), legacyFogChecked(settings->ofFogOff));
    controlList.push_back(cloudsCheckbox);
    controlList.push_back(fogCheckbox);
#else
    cloudsCheckbox = nullptr;
    fogCheckbox = nullptr;
#endif

#if PLATFORM_WII
    // The vertical copy filter is what reads as "anti-aliasing" on the Wii:
    // it steadies a 480i picture at the cost of a vertical blur.
    deflickerCheckbox = new LegacyOptionCheckbox(BUTTON_DEFLICKER, x, legacyLayout.rowY(row++), w, h,
        uiText("Deflicker Filter"), settings->wiiDeflicker);
    controlList.push_back(deflickerCheckbox);
#else
    deflickerCheckbox = nullptr;
#endif

    controlList.push_back(new LegacyOptionSlider(BUTTON_RENDER_DISTANCE, x, legacyLayout.rowY(row++), w, h,
        settings, EnumOptions::RENDER_DISTANCE_FINE));
    controlList.push_back(new LegacyOptionSlider(BUTTON_BRIGHTNESS, x, legacyLayout.rowY(row++), w, h,
        settings, EnumOptions::BRIGHTNESS));
#if PLATFORM_PS2
    controlList.push_back(new LegacyGuiButton(BUTTON_ASPECT_RATIO, x, legacyLayout.rowY(row++), w, h,
        settings->getKeyBinding(EnumOptions::ASPECT_RATIO)));
#endif
    controlList.push_back(new LegacyGuiButton(BUTTON_DONE, x, legacyLayout.rowY(row), w, h, uiText("Done")));
}

void LegacyVideoOptions::syncCheckboxes()
{
    graphicsCheckbox->setChecked(settings->fancyGraphics);
    smoothLightingCheckbox->setChecked(legacySmoothLightingChecked(settings->ofAoLevel));
    viewBobbingCheckbox->setChecked(settings->viewBobbing);
    if (cloudsCheckbox != nullptr)
        cloudsCheckbox->setChecked(legacyCloudsChecked(settings->ofClouds));
    if (fogCheckbox != nullptr)
        fogCheckbox->setChecked(legacyFogChecked(settings->ofFogOff));
    if (deflickerCheckbox != nullptr)
        deflickerCheckbox->setChecked(settings->wiiDeflicker);
}

void LegacyVideoOptions::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

    switch (button->id)
    {
    case BUTTON_GRAPHICS:
        settings->setOptionValue(EnumOptions::GRAPHICS, 1);
        syncCheckboxes();
        return;
    case BUTTON_SMOOTH_LIGHTING:
        settings->setOptionFloatValue(EnumOptions::AO_LEVEL,
            legacySmoothLightingToggleValue(settings->ofAoLevel));
        syncCheckboxes();
        return;
    case BUTTON_VIEW_BOBBING:
        settings->setOptionValue(EnumOptions::VIEW_BOBBING, 1);
        syncCheckboxes();
        return;
    case BUTTON_CLOUDS:
        settings->ofClouds = legacyCloudsToggledValue(settings->ofClouds);
        settings->saveOptions();
        syncCheckboxes();
        return;
    case BUTTON_FOG:
        settings->ofFogOff = legacyFogToggledOff(settings->ofFogOff);
        settings->saveOptions();
        syncCheckboxes();
        return;
    case BUTTON_DEFLICKER:
        settings->wiiDeflicker = !settings->wiiDeflicker;
        PlatformUserSettings::setDisplayDeflicker(settings->wiiDeflicker);
        settings->saveOptions();
        syncCheckboxes();
        return;
#if PLATFORM_PS2
    case BUTTON_ASPECT_RATIO:
    {
        settings->setOptionValue(EnumOptions::ASPECT_RATIO, 1);
        ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
        setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
        syncLegacySelection();
        return;
    }
#endif
    case BUTTON_DONE:
        returnToParent();
        return;
    default:
        return;
    }
}

void LegacyVideoOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawLegacyBackground(partialTick);
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
