#include "LegacyGuiButton.h"
#include "net/minecraft/src/FontRenderer.h"

#include "LegacyGuiButtonStyle.h"
#include "LegacyGuiSprites.h"
#include "LegacyOptionText.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/Tessellator.h"
#include "platform/RenderAPI.h"

LegacyGuiButton::LegacyGuiButton(int_t id, int_t x, int_t y, int_t width, int_t height, const std::string &text,
    float_t opacityValue)
    : GuiButton(id, x, y, width, height, text), opacity(legacyGuiButtonClampOpacity(opacityValue)), selected(false)
{
}

void LegacyGuiButton::setMenuIcon(const std::string &path, float u0, float v0, float u1, float v1)
{
    menuIconPath = path;
    menuIconU0 = u0;
    menuIconV0 = v0;
    menuIconU1 = u1;
    menuIconV1 = v1;
}

void LegacyGuiButton::drawButton(Minecraft *mc, int_t mouseX, int_t mouseY)
{
    if (!enabled2 || mc == nullptr || mc->fontRenderer == nullptr || mc->renderEngine == nullptr)
        return;

    const bool hovered = mouseX >= xPosition && mouseY >= yPosition &&
        mouseX < xPosition + width && mouseY < yPosition + height;
    const LegacyGuiButtonVisual visual = legacyGuiButtonVisual(enabled, hovered || selected);

    const LegacyGuiButtonBorderRects border = legacyGuiButtonBorderRects(xPosition, yPosition, width, height);

    renderBindTexture(mc->renderEngine->getTexture("/gui/gui.png"));
    const bool translucent = opacity < 0.999f;
    if (translucent)
    {
        renderEnable(RenderCapability::Blend);
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    }
    renderColor4f(1.0f, 1.0f, 1.0f, opacity);
    legacyDrawVanillaButtonBase(xPosition, yPosition, width, height, visual.vanillaState, zLevel);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    if (translucent)
        renderDisable(RenderCapability::Blend);

    // A fully transparent colour still costs four blended quads per button per
    // frame, so skip the ring outright when the style asks for none.
    if (visual.outerBorderColor != 0)
    {
        drawRect(border.top.left, border.top.top, border.top.right, border.top.bottom, visual.outerBorderColor);
        drawRect(border.bottom.left, border.bottom.top, border.bottom.right, border.bottom.bottom, visual.outerBorderColor);
        drawRect(border.left.left, border.left.top, border.left.right, border.left.bottom, visual.outerBorderColor);
        drawRect(border.right.left, border.right.top, border.right.right, border.right.bottom, visual.outerBorderColor);
    }

    if (visual.hovered)
    {
        drawGradientRect(xPosition + 1, yPosition + 1, xPosition + width - 1, yPosition + height - 1,
            visual.hoverFillTop, visual.hoverFillBottom);
        drawRect(xPosition + 2, yPosition + 2, xPosition + width - 2, yPosition + 3, visual.hoverHighlightColor);
        drawRect(xPosition + 2, yPosition + height - 3, xPosition + width - 2, yPosition + height - 2, visual.hoverShadowColor);
    }

    mouseDragged(mc, mouseX, mouseY);
    if (!menuIconPath.empty())
    {
        // 3DS main-menu style: an icon from the game's own assets first,
        // then a left-aligned label beside it (Legacy Console's list look).
        // The icon is a plain textured quad, so any atlas/UV handed to
        // setMenuIcon draws -- terrain blocks, the player face, the vanilla
        // language glyph.
        constexpr int_t ICON_SIZE = 12;
        const int_t iconX = xPosition + 4;
        const int_t iconY = yPosition + (height - ICON_SIZE) / 2;
        renderBindTexture(mc->renderEngine->getTexture(menuIconPath));
        Tessellator &iconTess = Tessellator::instance;
        iconTess.startDrawingQuads();
        iconTess.setColorOpaque_I(0xffffff);
        iconTess.addVertexWithUV(iconX, iconY + ICON_SIZE, zLevel, menuIconU0, menuIconV1);
        iconTess.addVertexWithUV(iconX + ICON_SIZE, iconY + ICON_SIZE, zLevel, menuIconU1, menuIconV1);
        iconTess.addVertexWithUV(iconX + ICON_SIZE, iconY, zLevel, menuIconU1, menuIconV0);
        iconTess.addVertexWithUV(iconX, iconY, zLevel, menuIconU0, menuIconV0);
        iconTess.draw();
        legacyDrawOptionText(mc->fontRenderer,
            mc->fontRenderer->trimStringToWidth(displayString, width - 24),
            iconX + ICON_SIZE + 4, legacyGuiButtonTextY(yPosition, height), visual.textColor);
    }
    else
    {
        // The same crisp emboss the sliders and the panel labels use. Java's soft
        // 38 % black shadow smeared the glyphs into the button frame, which read as a
        // dark, muddy label next to a slider drawn right above it.
        legacyDrawCenteredOptionText(mc->fontRenderer, mc->fontRenderer->trimStringToWidth(displayString, width - 8), xPosition + width / 2,
            legacyGuiButtonTextY(yPosition, height), visual.textColor);
    }
}

void LegacyGuiButton::setSelected(bool selectedValue)
{
    selected = selectedValue;
}

void LegacyGuiButton::setKeyboardSelected(bool selectedValue)
{
    setSelected(selectedValue);
}
