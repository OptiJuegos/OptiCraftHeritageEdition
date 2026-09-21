#include "GuiContainer.h"
#include "mods/ModManager.h"
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
#include "legacy/LegacySlotCursor.h"
#include "PlayerController.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformConfig.h"
#include "pc/lwjgl/Keyboard.h"
#include <algorithm>

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
#include "ContainerSlotNavigator.h"
#include "platform/Input.h"
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
	ContainerSlotNavigator &navigator = ContainerSlotNavigator::instance();
	Slot *controllerSlot = nullptr;
	if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
	{
		ContainerSlotNavigator::Layout navigation;
		navigation.guiLeft = guiX;
		navigation.guiTop = guiY;
		navigation.screenWidth = width;
		navigation.screenHeight = height;
		navigation.displayWidth = mc->displayWidth;
		navigation.displayHeight = mc->displayHeight;
		navigator.notifyOpen(this, navigation);
		navigator.notePointerSlot(platformMenuPointerActive() ? getSlotAtPosition(mouseX, mouseY) : nullptr);
		if (navigator.controllerSelectionActive())
			controllerSlot = navigator.selectedSlot();
	}
	else
	{
		navigator.notifyClosed(this);
	}

	Slot *selectedSlot = controllerSlot;
	if (selectedSlot != nullptr && navigator.consumePrimaryClick())
		handleMouseClick(selectedSlot, selectedSlot->slotNumber, 0, false);
	if (selectedSlot != nullptr && navigator.consumeSecondaryClick())
		handleMouseClick(selectedSlot, selectedSlot->slotNumber, 1, false);
#endif

	drawGuiContainerBackgroundLayer(partialTick);

#if PLATFORM_GUI_FORCE_DEPTH_DISABLED
	// The console GUI pass starts with depth disabled, but 3D item icons need
	// their own faces depth-sorted before the 2D overlay/foreground pass.
	renderEnable(RenderCapability::DepthTest);
	renderDepthMask(true);
	renderDepthFunc(RenderCompare::LessEqual);
#endif

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
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		const bool selectedByController = slot == controllerSlot;
		const bool selectedByPointer = controllerSlot == nullptr && getIsMouseOverSlot(slot, mouseX, mouseY);
		if (selectedByController || selectedByPointer)
#else
		if (getIsMouseOverSlot(slot, mouseX, mouseY))
#endif
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
		int_t carriedX = mouseX - guiX - 8;
		int_t carriedY = mouseY - guiY - 8;
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		if (controllerSlot != nullptr)
		{
			carriedX = controllerSlot->xDisplayPosition;
			carriedY = controllerSlot->yDisplayPosition;
		}
#endif
		renderTranslate(0.0f, 0.0f, 32.0f);
		itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, inv->getItemStack(), carriedX, carriedY);
		itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, inv->getItemStack(), carriedX, carriedY);
	}

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	if (controllerSlot != nullptr)
	{
		renderDisable(RenderCapability::Lighting);
		renderDisable(RenderCapability::DepthTest);
		legacyDrawSlotCursor(mc, controllerSlot, zLevel + 128.0f);
		renderEnable(RenderCapability::DepthTest);
		renderEnable(RenderCapability::Lighting);
	}
#endif

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
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
			if (controllerSlot != nullptr)
			{
				tooltipX = controllerSlot->xDisplayPosition + 22;
				tooltipY = controllerSlot->yDisplayPosition - 10;
			}
#endif
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
	ModManager::getInstance().onDrawContainer(this, mouseX, mouseY);
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

Slot *GuiContainer::getControllerNavigationTarget(Slot *, int_t, int_t)
{
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
#if PLATFORM_PS2 || PLATFORM_WII
	ContainerSlotNavigator &navigator = ContainerSlotNavigator::instance();
	// Console confirm buttons are exposed both as controller input and mouse
	// clicks. When D-pad selection owns the inventory, ignore the synthesized
	// mouse edge so the selected slot is activated exactly once.
	if (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI
	    && navigator.controllerSelectionActive() && (button == 0 || button == 1))
		return;
	navigator.notePointerActivity();
#endif
	GuiScreen::mouseClicked(x, y, button);
	if (ModManager::getInstance().onContainerMouseClicked(this, x, y, button))
		return;

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
#if PLATFORM_PS2 || PLATFORM_WII
	// Button release is not pointer motion. Only actual movement should take
	// authority away from the controller-selected slot.
	if (button < 0)
		ContainerSlotNavigator::instance().notePointerActivity();
#endif
	(void)x;
	(void)y;
	(void)button;
}

void GuiContainer::keyTyped(char_t c, int_t key)
{
	if (ModManager::getInstance().onContainerKeyTyped(c, key))
		return;

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
