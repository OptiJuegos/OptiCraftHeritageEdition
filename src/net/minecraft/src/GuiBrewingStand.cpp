#include "GuiBrewingStand.h"

#include "ContainerBrewingStand.h"
#include "FontRenderer.h"
#include "InventoryPlayer.h"
#include "Minecraft.h"
#include "RenderEngine.h"
#include "StatCollector.h"
#include "TileEntityBrewingStand.h"
#include "platform/RenderAPI.h"

GuiBrewingStand::GuiBrewingStand(InventoryPlayer *inventory, TileEntityBrewingStand *brewingStandIn)
    : GuiContainer(new ContainerBrewingStand(inventory, brewingStandIn), true)
    , brewingStand(brewingStandIn)
{
}

void GuiBrewingStand::drawGuiContainerForegroundLayer()
{
    fontRenderer->drawString(StatCollector::translateToLocal("container.brewing"), 56, 6, 0x404040);
    fontRenderer->drawString(StatCollector::translateToLocal("container.inventory"), 8, ySize - 94, 0x404040);
}

void GuiBrewingStand::drawGuiContainerBackgroundLayer(float_t)
{
    int_t texture = mc->renderEngine->getTexture("/gui/alchemy.png");
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    mc->renderEngine->bindTexture(texture);
    int_t guiX = (width - xSize) / 2;
    int_t guiY = (height - ySize) / 2;
    drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);

    int_t brewTime = brewingStand != nullptr ? brewingStand->getBrewTime() : 0;
    if (brewTime <= 0)
        return;

    int_t progress = (int_t)(28.0f * (1.0f - (float_t)brewTime / 400.0f));
    if (progress > 0)
        drawTexturedModalRect(guiX + 97, guiY + 16, 176, 0, 9, progress);

    int_t bubbles = brewTime / 2 % 7;
    static const int_t bubbleHeight[7] = {29, 24, 20, 16, 11, 6, 0};
    int_t height = bubbleHeight[bubbles];
    if (height > 0)
        drawTexturedModalRect(guiX + 65, guiY + 43 - height, 185, 29 - height, 12, height);
}
