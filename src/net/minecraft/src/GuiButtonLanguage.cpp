#include "GuiButtonLanguage.h"

#include "Minecraft.h"
#include "RenderEngine.h"
#include "platform/RenderAPI.h"

GuiButtonLanguage::GuiButtonLanguage(int_t id, int_t x, int_t y)
    : GuiButton(id, x, y, 20, 20, "")
{
}

void GuiButtonLanguage::drawButton(Minecraft *minecraft, int_t mouseX, int_t mouseY)
{
    if (!enabled2)
        return;

    renderBindTexture(minecraft->renderEngine->getTexture("/gui/gui.png"));
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    bool hovered = mouseX >= xPosition && mouseY >= yPosition
        && mouseX < xPosition + width && mouseY < yPosition + height;
    int_t textureY = 106 + (hovered ? height : 0);
    drawTexturedModalRect(xPosition, yPosition, 0, textureY, width, height);
}
