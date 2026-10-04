#include "LegacyOptionSlider.h"

#include <algorithm>

#include "LegacyGuiButtonStyle.h"
#include "LegacyGuiSprites.h"
#include "LegacyOptionMetrics.h"
#include "LegacyOptionState.h"
#include "LegacyOptionText.h"
#include "LegacyUiTheme.h"
#include "net/minecraft/src/Config.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "platform/RenderAPI.h"

namespace
{
// GuiSlider maps the pointer to the knob centre and lets the knob travel the full
// widget width; the Legacy slider keeps that mapping so a click lands where Java
// puts it.
int_t sliderTravel(int_t width, int_t knobWidth)
{
    return std::max<int_t>(1, width - knobWidth);
}
}

LegacyOptionSlider::LegacyOptionSlider(int_t id, int_t x, int_t y, int_t width, int_t height,
    GameSettings *settingsValue, EnumOptions *optionValue)
    : GuiButton(id, x, y, width, height, ""), settings(settingsValue), option(optionValue),
      sliderValue(0.0f), dragging(false), selected(false)
{
    refreshFromSettings();
}

float_t LegacyOptionSlider::readValue() const
{
    return settings->getOptionFloatValue(option);
}

void LegacyOptionSlider::writeValue(float_t value)
{
    settings->setOptionFloatValue(option, value);
}

std::string LegacyOptionSlider::buildLabel() const
{
    if (option == EnumOptions::RENDER_DISTANCE_FINE)
        return legacyRenderDistanceLabel(settings->ofRenderDistanceFine);
    if (option == EnumOptions::FOV)
        return legacyFovLabel(settings->getOptionFloatValue(option));
    if (option == EnumOptions::SENSITIVITY)
        return legacySensitivityLabel(settings->getOptionFloatValue(option));
    return settings->getKeyBinding(option);
}

void LegacyOptionSlider::refreshLabel()
{
    if (settings == nullptr || option == nullptr)
    {
        displayString.clear();
        return;
    }
    displayString = buildLabel();
}

void LegacyOptionSlider::refreshFromSettings()
{
    if (settings == nullptr || option == nullptr)
        return;
    sliderValue = std::max<float_t>(0.0f, std::min<float_t>(1.0f, readValue()));
    refreshLabel();
}

void LegacyOptionSlider::updateFromMouse(Minecraft *mc, int_t mouseX)
{
    if (mc == nullptr || settings == nullptr || option == nullptr)
        return;

    const int_t knobWidth = legacyOptionSliderKnobWidth();
    sliderValue = static_cast<float_t>(mouseX - (xPosition + knobWidth / 2)) /
        static_cast<float_t>(sliderTravel(width, knobWidth));
    sliderValue = std::max<float_t>(0.0f, std::min<float_t>(1.0f, sliderValue));
    writeValue(sliderValue);
    // Read back rather than keep the pointer position: a discrete slider snaps the
    // value to its nearest stop and the knob has to follow it.
    sliderValue = std::max<float_t>(0.0f, std::min<float_t>(1.0f, readValue()));
    refreshLabel();
}

bool LegacyOptionSlider::mousePressed(Minecraft *mc, int_t mouseX, int_t mouseY)
{
    if (!GuiButton::mousePressed(mc, mouseX, mouseY))
        return false;
    updateFromMouse(mc, mouseX);
    dragging = true;
    return true;
}

void LegacyOptionSlider::mouseReleased(int_t, int_t)
{
    dragging = false;
}

void LegacyOptionSlider::mouseDragged(Minecraft *mc, int_t mouseX, int_t)
{
    if (!dragging)
        return;
    // The -1000 test rejects GuiScreen::drawScreen's suppressed-pointer
    // sentinel (-10000): this drag is driven from drawButton every frame, and
    // on the 3DS the touch-release event can still be queued when a render
    // runs with the finger already up (runTick skips frames at 60 fps) -- the
    // sentinel would slam the value to 0 on the very frame the touch ends
    // (music/sound volume went to 0 on release, 2026-09-29). Only a real
    // on-screen position may move the knob.
    if (mouseX < -1000)
        return;
    updateFromMouse(mc, mouseX);
}

void LegacyOptionSlider::drawButton(Minecraft *mc, int_t mouseX, int_t mouseY)
{
    if (!enabled2 || mc == nullptr || mc->fontRenderer == nullptr || mc->renderEngine == nullptr)
        return;

    if (!dragging)
        refreshFromSettings();
    else
        mouseDragged(mc, mouseX, mouseY);

    const LegacyUiTheme &theme = legacyUiTheme();
    const bool hovered = selected || (enabled && mouseX >= xPosition && mouseY >= yPosition &&
        mouseX < xPosition + width && mouseY < yPosition + height);
    const bool active = hovered || dragging;
    const LegacyGuiButtonVisual visual = legacyGuiButtonVisual(enabled, active);

    // GuiSlider keeps the track on the unlit button frame so the knob reads as the
    // only raised part of the widget; the Legacy selection tint goes on top of it,
    // exactly as LegacyGuiButton layers it over a normal button.
    renderBindTexture(mc->renderEngine->getTexture("/gui/gui.png"));
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    legacyDrawVanillaButtonBase(xPosition, yPosition, width, height, 0, zLevel);

    if (visual.hovered)
    {
        drawGradientRect(xPosition + 1, yPosition + 1, xPosition + width - 1, yPosition + height - 1,
            visual.hoverFillTop, visual.hoverFillBottom);
        drawRect(xPosition + 2, yPosition + 2, xPosition + width - 2, yPosition + 3,
            visual.hoverHighlightColor);
        drawRect(xPosition + 2, yPosition + height - 3, xPosition + width - 2, yPosition + height - 2,
            visual.hoverShadowColor);
    }

    const int_t knobWidth = theme.sliderKnobWidth;
    const int_t knobX = xPosition +
        static_cast<int_t>(sliderValue * static_cast<float_t>(sliderTravel(width, knobWidth)));
    // drawRect/drawGradientRect leave the fill colour behind; the atlas quad needs
    // white again, and the binding above survived because they only toggle Texture2D.
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    legacyDrawVanillaSliderKnob(knobX, yPosition, knobWidth, height, zLevel);

    const int_t textColor = !enabled ? theme.disabledTextColor
        : (active ? theme.selectedTextColor : static_cast<int_t>(0xffffffffu));
    legacyDrawCenteredOptionText(mc->fontRenderer, displayString, xPosition + width / 2,
        legacyOptionTextY(yPosition, height), textColor);

}

void LegacyOptionSlider::setKeyboardSelected(bool selectedValue)
{
    selected = selectedValue;
}

float_t LegacyOptionSlider::keyboardStep() const
{
    if (option == EnumOptions::RENDER_DISTANCE_FINE)
    {
        // GameSettings truncates the fine distance down to a 16-block stop, so a
        // step below one stop only ever moves the value down. One and a half
        // stops lands on the next stop in either direction whatever the float
        // rounding of the previous value does.
        const int_t range = Config::getMaxRenderDistanceFine() - 32;
        if (range <= 0)
            return 1.0f;
        return 24.0f / static_cast<float_t>(range);
    }
    return 0.05f;
}

bool LegacyOptionSlider::adjustKeyboard(Minecraft *mc, int_t direction)
{
    (void)mc;
    if (direction == 0 || settings == nullptr || option == nullptr)
        return false;

    const float_t previous = readValue();
    const float_t step = direction < 0 ? -keyboardStep() : keyboardStep();
    const float_t requested = std::max<float_t>(0.0f, std::min<float_t>(1.0f, previous + step));
    writeValue(requested);
    refreshFromSettings();
    return sliderValue != previous;
}

