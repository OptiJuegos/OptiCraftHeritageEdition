#include "GuiContainer.h"
#include "java/String.h"
#include "EntityPlayerSP.h"
#include "Container.h"
#include "Slot.h"
#include "RenderItem.h"
#include "RenderHelper.h"
#include "InventoryPlayer.h"
#include "StringTranslate.h"
#include "ItemStack.h"
#include "EnumRarity.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "OpenGlHelper.h"
#include "Minecraft.h"
#include "GameSettings.h"
#include "KeyBinding.h"
#include "PlayerController.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Keyboard.h"
#include <algorithm>

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
#include "ContainerSlotNavigator.h"
#endif

RenderItem *GuiContainer::itemRenderer = new RenderItem();

GuiContainer::GuiContainer(Container *container, bool ownsContainer)
	: xSize(176)
	, ySize(166)
	, guiLeft(0)
	, guiTop(0)
	, inventorySlots(container)
	, ownsInventorySlots(ownsContainer)
{
}

GuiContainer::~GuiContainer()
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	// onGuiClosed() is the normal exit, but a container screen can also be
	// destroyed while it is still the one the navigator points at (world change,
	// shutdown), and that pointer is read from the pad poll rather than from a
	// tick -- so it has to stop being live here too.
	ContainerSlotNavigator::instance().notifyClosed(this);
#endif

	if (ownsInventorySlots)
	{
		delete inventorySlots;
		inventorySlots = nullptr;
	}
}

void GuiContainer::initGui()
{
	GuiScreen::initGui();
	guiLeft = (width - xSize) / 2;
	guiTop = (height - ySize) / 2;
	mc->thePlayer->craftingInventory = inventorySlots;
}

void GuiContainer::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	int_t guiX = guiLeft;
	int_t guiY = guiTop;

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	// Publish the geometry the console D-pad navigation steps against, from the
	// one place that already has the final per-screen geometry for this frame.
	// Keeping navigation on the same guiLeft/guiTop used by rendering and hit
	// testing prevents shifted containers from exposing a different slot grid.
	ContainerSlotNavigator::Layout navigation;
	navigation.guiLeft = guiX;
	navigation.guiTop = guiY;
	navigation.screenWidth = width;
	navigation.screenHeight = height;
	navigation.displayWidth = mc->displayWidth;
	navigation.displayHeight = mc->displayHeight;
	ContainerSlotNavigator::instance().notifyOpen(this, navigation);
#endif

	drawGuiContainerBackgroundLayer(partialTick);

	RenderHelper::enableGUIStandardItemLighting();

	renderPushMatrix();
	renderTranslate((float_t)guiX, (float_t)guiY, 0.0f);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderEnable(RenderCapability::RescaleNormal);
	OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, 240.0f, 240.0f);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Slot *hoveredSlot = nullptr;
	for (int_t i = 0; i < (int_t)inventorySlots->slots.size(); i++)
	{
		Slot *slot = inventorySlots->slots[i];
		drawSlotInventory(slot);
		if (getIsMouseOverSlot(slot, mouseX, mouseY))
		{
			hoveredSlot = slot;
			renderDisable(RenderCapability::Lighting);
			renderDisable(RenderCapability::DepthTest);
			int_t sx = slot->xDisplayPosition;
			int_t sy = slot->yDisplayPosition;
			drawGradientRect(sx, sy, sx + 16, sy + 16, 0x80ffffff, 0x80ffffff);
			renderEnable(RenderCapability::Lighting);
			renderEnable(RenderCapability::DepthTest);
		}
	}

	InventoryPlayer *inv = mc->thePlayer->inventory;
	if (inv->getItemStack() != nullptr)
	{
		renderTranslate(0.0f, 0.0f, 32.0f);
		itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, inv->getItemStack(),
		                                mouseX - guiX - 8, mouseY - guiY - 8);
		itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, inv->getItemStack(),
		                                       mouseX - guiX - 8, mouseY - guiY - 8);
	}

	renderDisable(RenderCapability::RescaleNormal);
	RenderHelper::disableStandardItemLighting();
	renderDisable(RenderCapability::Lighting);
	renderDisable(RenderCapability::DepthTest);
	drawGuiContainerForegroundLayer();

	if (inv->getItemStack() == nullptr && hoveredSlot != nullptr && hoveredSlot->getHasStack())
	{
		ItemStack *hoveredStack = hoveredSlot->getStack();
		std::vector<std::string> information = hoveredStack->getItemNameandInformation();
		if (!information.empty())
		{
			int_t tooltipWidth = 0;
			for (const std::string& line : information)
				tooltipWidth = std::max(tooltipWidth, fontRenderer->getStringWidth(line));

			int_t tooltipX = mouseX - guiX + 12;
			int_t tooltipY = mouseY - guiY - 12;
			int_t tooltipHeight = 8;
			if (information.size() > 1)
				tooltipHeight += 2 + (static_cast<int_t>(information.size()) - 1) * 10;

			const int_t background = static_cast<int_t>(0xf0100010u);
			drawGradientRect(tooltipX - 3, tooltipY - 4, tooltipX + tooltipWidth + 3, tooltipY - 3, background, background);
			drawGradientRect(tooltipX - 3, tooltipY + tooltipHeight + 3, tooltipX + tooltipWidth + 3, tooltipY + tooltipHeight + 4, background, background);
			drawGradientRect(tooltipX - 3, tooltipY - 3, tooltipX + tooltipWidth + 3, tooltipY + tooltipHeight + 3, background, background);
			drawGradientRect(tooltipX - 4, tooltipY - 3, tooltipX - 3, tooltipY + tooltipHeight + 3, background, background);
			drawGradientRect(tooltipX + tooltipWidth + 3, tooltipY - 3, tooltipX + tooltipWidth + 4, tooltipY + tooltipHeight + 3, background, background);

			const int_t borderTop = 0x505000ff;
			const int_t borderBottom = (borderTop & 0x00fefefe) >> 1 | (borderTop & static_cast<int_t>(0xff000000u));
			drawGradientRect(tooltipX - 3, tooltipY - 2, tooltipX - 2, tooltipY + tooltipHeight + 2, borderTop, borderBottom);
			drawGradientRect(tooltipX + tooltipWidth + 2, tooltipY - 2, tooltipX + tooltipWidth + 3, tooltipY + tooltipHeight + 2, borderTop, borderBottom);
			drawGradientRect(tooltipX - 3, tooltipY - 3, tooltipX + tooltipWidth + 3, tooltipY - 2, borderTop, borderTop);
			drawGradientRect(tooltipX - 3, tooltipY + tooltipHeight + 2, tooltipX + tooltipWidth + 3, tooltipY + tooltipHeight + 3, borderBottom, borderBottom);

			const char *hex = "0123456789abcdef";
			for (std::size_t i = 0; i < information.size(); ++i)
			{
				std::string line = information[i];
				if (i == 0)
				{
					int_t color = hoveredStack->getRarity().nameColor & 15;
					line = std::string(u8"§") + hex[color] + line;
				}
				else
				{
					line = std::string(u8"§7") + line;
				}

				fontRenderer->drawStringWithShadow(line, tooltipX, tooltipY, -1);
				if (i == 0)
					tooltipY += 2;
				tooltipY += 10;
			}
		}
	}

	renderPopMatrix();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
	renderEnable(RenderCapability::Lighting);
	renderEnable(RenderCapability::DepthTest);
}

void GuiContainer::drawGuiContainerForegroundLayer()
{
}

void GuiContainer::drawSlotInventory(Slot *slot)
{
	int_t x = slot->xDisplayPosition;
	int_t y = slot->yDisplayPosition;
	ItemStack *stack = slot->getStack();
	if (stack == nullptr)
	{
		int_t icon = slot->getBackgroundIconIndex();
		if (icon >= 0)
		{
			renderDisable(RenderCapability::Lighting);
			mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/gui/items.png"));
			drawTexturedModalRect(x, y, (icon % 16) * 16, (icon / 16) * 16, 16, 16);
			renderEnable(RenderCapability::Lighting);
			return;
		}
	}
	itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, stack, x, y);
	itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, stack, x, y);
}

Slot *GuiContainer::getSlotAtPosition(int_t mouseX, int_t mouseY)
{
	for (int_t i = 0; i < (int_t)inventorySlots->slots.size(); i++)
	{
		Slot *slot = inventorySlots->slots[i];
		if (getIsMouseOverSlot(slot, mouseX, mouseY))
			return slot;
	}
	return nullptr;
}

bool GuiContainer::getIsMouseOverSlot(Slot *slot, int_t mouseX, int_t mouseY)
{
	int_t guiX = guiLeft;
	int_t guiY = guiTop;
	int_t rx = mouseX - guiX;
	int_t ry = mouseY - guiY;
	return rx >= slot->xDisplayPosition - 1 && rx < slot->xDisplayPosition + 16 + 1
	    && ry >= slot->yDisplayPosition - 1 && ry < slot->yDisplayPosition + 16 + 1;
}

void GuiContainer::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	if (button == 0 || button == 1)
	{
		Slot *slot = getSlotAtPosition(x, y);
		int_t guiX = guiLeft;
		int_t guiY = guiTop;
		bool outsideGui = x < guiX || y < guiY || x >= guiX + xSize || y >= guiY + ySize;
		int_t slotId = -1;
		if (slot != nullptr) slotId = slot->slotNumber;
		if (outsideGui)      slotId = -999;
		if (slotId != -1)
		{
			bool shift = slotId != -999 && (lwjgl::Keyboard::isKeyDown(42) || lwjgl::Keyboard::isKeyDown(54));
			handleMouseClick(slot, slotId, button, shift);
		}
	}
}


void GuiContainer::handleMouseClick(Slot *slot, int_t slotId, int_t button, bool shift)
{
	if (slot != nullptr)
		slotId = slot->slotNumber;
	delete mc->playerController->windowClick(inventorySlots->windowId, slotId, button, shift, mc->thePlayer);
}

void GuiContainer::mouseMovedOrUp(int_t x, int_t y, int_t button)
{
	// no-op when button != 0
}

void GuiContainer::keyTyped(char_t c, int_t key)
{
	if (key == 1 || key == mc->gameSettings->keyBindInventory->keyCode)
	{
		mc->thePlayer->closeScreen();
	}
}

void GuiContainer::onGuiClosed()
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	// Before the thePlayer guard below: the navigator has to be released even on
	// the paths that return early here.
	ContainerSlotNavigator::instance().notifyClosed(this);
#endif

	if (mc->thePlayer == nullptr) return;
	inventorySlots->onCraftGuiClosed(mc->thePlayer);
	mc->playerController->closeWindow(inventorySlots->windowId, mc->thePlayer);
}

bool GuiContainer::doesGuiPauseGame()
{
	return false;
}

void GuiContainer::updateScreen()
{
	GuiScreen::updateScreen();
	if (!mc->thePlayer->isEntityAlive() || mc->thePlayer->isDead)
	{
		mc->thePlayer->closeScreen();
	}
}
