#include "GuiScreen.h"
#include "SoundManager.h"
#include "GameSettings.h"
#include "GuiButton.h"
#include "GuiParticle.h"
#include "GuiTextField.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"
#include "platform/PlatformTuning.h"
#include "platform/Input.h"
#if !PLATFORM_PS2 && !PLATFORM_WII
#include "SDL_clipboard.h"
#endif
#if PLATFORM_PS2 || PLATFORM_WII
#include "VirtualKeyboard.h"
#include "ContainerSlotNavigator.h"
#endif

// Minecraft forward-included via header chain
class Minecraft;
#include "Minecraft.h"
#include "legacy/LegacySelectionCursor.h"

namespace
{
// The legacy UI is a D-pad design because the PS2 has nothing to point with.
// The Wii does, so there it is gated on whether the remote pointer currently
// owns menu input instead of on legacyUI. Ownership already moves away from the
// pointer as soon as a D-pad direction or a stick is used (see
// updateMenuInputOwner in WiiPadState.cpp), so both work in the same screen:
// aim to use the cursor, press a direction to go back to navigation. The legacy
// screens were written for this -- legacyHoveredButton() and the create-world
// screen already have PLATFORM_WII pointer branches that this used to keep
// permanently switched off.
bool menuPointerInputSuppressed(Minecraft *mc)
{
#if PLATFORM_PS2 || PLATFORM_WII
	if (mc != nullptr && mc->currentScreen != nullptr && mc->currentScreen->suppressesPlatformPointerInput())
		return true;
#endif
#if PLATFORM_PS2
	return mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI &&
	       (mc->currentScreen == nullptr || !mc->currentScreen->allowsPlatformPointerInput());
#elif PLATFORM_WII
	(void)mc;
	return !platformMenuPointerActive();
#else
	(void)mc;
	return false;
#endif
}

bool menuCursorSuppressed(Minecraft *mc)
{
#if PLATFORM_PS2 || PLATFORM_WII
	if (mc != nullptr && mc->currentScreen != nullptr && mc->currentScreen->suppressesPlatformPointerInput())
		return true;
#endif
#if PLATFORM_PS2
	if (!platformMenuCursorVisible())
		return true;
	if (mc == nullptr || mc->gameSettings == nullptr || !mc->gameSettings->legacyUI)
		return false;
	if (mc->currentScreen == nullptr || !mc->currentScreen->allowsPlatformPointerInput())
		return true;
	// Keep the D-pad slot cursor visible until the first pointer-motion event
	// clears its selection in GuiContainer::mouseMovedOrUp().
	return ContainerSlotNavigator::instance().controllerSelectionActive();
#elif PLATFORM_WII
	// Drawn only while the pointer is the active owner, so the cursor is not left
	// sitting on screen through a D-pad-driven menu.
	(void)mc;
	return !platformMenuCursorVisible() || !platformMenuPointerActive();
#else
	(void)mc;
	return false;
#endif
}
}



GuiScreen::GuiScreen()
	: mc(nullptr)
	, width(0)
	, height(0)
	, field_948_f(false)
	, fontRenderer(nullptr)
	, guiParticles(nullptr)
	, selectedButton(nullptr)
	, keyboardSelectedControlIndex(-1)
	, focusedTextField(nullptr)
{
}

GuiScreen::~GuiScreen()
{
	// Buttons and the particle system are owned solely by this screen.
	for (GuiButton *btn : controlList)
		delete btn;
	controlList.clear();
	delete guiParticles;
	guiParticles = nullptr;
}

void GuiScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	if (isJavaUiKeyboardNavigationEnabled())
		syncKeyboardSelection();
	const bool suppressPointerInput = menuPointerInputSuppressed(mc);
	const bool suppressCursor = menuCursorSuppressed(mc);
	const bool suppressMouseHover = keyboardSelectedControlIndex >= 0 && isJavaUiKeyboardNavigationEnabled();
	const int_t effectiveMouseX = suppressPointerInput || suppressMouseHover ? -10000 : mouseX;
	const int_t effectiveMouseY = suppressPointerInput || suppressMouseHover ? -10000 : mouseY;
	for (int_t i = 0; i < (int_t)controlList.size(); i++)
	{
		controlList[i]->drawButton(mc, effectiveMouseX, effectiveMouseY);
	}

#if PLATFORM_SOFTWARE_CURSOR
	(void)partialTick;
	if (!suppressCursor)
	{
		// [Issue #3 Fix / Cursor Enhancement]:
		// Instead of binding /cursor.png unconditionally (which triggered the missingTextureImage
		// checkerboard whenever cursor.png was absent or not yet resident), use the unified
		// legacyDrawSelectionCursorCentered helper. This safely renders the textured reticle if
		// available, or automatically falls back to the authentic procedural vector crosshair.
		legacyDrawSelectionCursorCentered(mc, mouseX, mouseY, PLATFORM_CURSOR_SIZE, zLevel + 100.0f);
	}
#endif
}

void GuiScreen::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		if (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
			mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
}

jstring GuiScreen::getClipboardString()
{
	// SDL clipboard
#if !PLATFORM_PS2 && !PLATFORM_WII
	char *text = SDL_GetClipboardText();
	if (text)
	{
		std::string s(text);
		SDL_free(text);
		return s;
	}
#endif
	return jstring(nullptr);
}

void GuiScreen::setClipboardString(const std::string &text)
{
#if !PLATFORM_PS2 && !PLATFORM_WII
	SDL_SetClipboardText(text.c_str());
#else
	(void)text;
#endif
}

bool GuiScreen::isCtrlKeyDown()
{
	return lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LCONTROL) ||
	       lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RCONTROL);
}

bool GuiScreen::isShiftKeyDown()
{
	return lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) ||
	       lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT);
}

void GuiScreen::mouseClicked(int_t x, int_t y, int_t button)
{
	if (button == 0)
	{
		for (int_t i = 0; i < (int_t)controlList.size(); i++)
		{
			GuiButton *btn = controlList[i];
			if (btn->mousePressed(mc, x, y))
			{
				selectedButton = btn;
				const char *sound = mc->gameSettings != nullptr && mc->gameSettings->legacyUI
					? "random.action" : "random.click";
				mc->sndManager->playSoundFX(sound, 1.0f, 1.0f);
				actionPerformed(btn);

				// An action is allowed to rebuild this very screen (video/aspect
				// options call setWorldAndResolution) or replace currentScreen.
				// Continuing the loop would then iterate a different controlList
				// during the same physical click. One mouse-down can activate at
				// most one widget, matching the Java/LWJGL behaviour we want here.
				return;
			}
		}
	}
}

void GuiScreen::mouseMovedOrUp(int_t x, int_t y, int_t button)
{
	if (selectedButton != nullptr && button == 0)
	{
		selectedButton->mouseReleased(x, y);
		selectedButton = nullptr;
	}
}

void GuiScreen::actionPerformed(GuiButton *button)
{
}

void GuiScreen::setWorldAndResolution(Minecraft *minecraft, int_t w, int_t h)
{
	// Called again on every resize; free the previous frame's buttons/particles
	// instead of leaking them (Java relied on GC).
	delete guiParticles;
	guiParticles = new GuiParticle(minecraft);
	mc = minecraft;
	fontRenderer = minecraft->fontRenderer;
	width = w;
	height = h;
	// selectedButton siempre apunta a un boton de controlList; al destruirlos quedaria
	// colgante y mouseMovedOrUp haria use-after-free (crash al soltar el raton tras
	// una accion que reconstruye la pantalla, p.ej. togglear una opcion). Anularlo.
	selectedButton = nullptr;
	keyboardSelectedControlIndex = -1;
	focusedTextField = nullptr;
	for (GuiButton *btn : controlList)
		delete btn;
	controlList.clear();
	initGui();
}

void GuiScreen::clearControlList()
{
	selectedButton = nullptr;
	for (GuiButton *button : controlList)
		delete button;
	controlList.clear();
}

void GuiScreen::initGui()
{
}

void GuiScreen::handleInput()
{
	handleSpecializedMenuInput();
#if PLATFORM_PS2 || PLATFORM_WII
	// Console GUI helpers consume the platform snapshot here, after the native
	// backend has published this frame's controller state and before queued
	// mouse/keyboard events are dispatched to the screen. Keeping this routing
	// in shared GUI code prevents Wii/PS2 input backends from depending on
	// Minecraft screen classes.
	VirtualKeyboard::instance().tick();
	if (!platformTextInputExclusive())
		ContainerSlotNavigator::instance().tick();
#if PLATFORM_PS2 || PLATFORM_WII
	handleConsoleJavaUiNavigation();
#endif
#endif
	while (lwjgl::Mouse::next()) handleMouseInput();
	while (lwjgl::Keyboard::next()) handleKeyboardInput();
}

void GuiScreen::handleMouseInput()
{
	if (menuPointerInputSuppressed(mc))
		return;

	if (lwjgl::Mouse::getEventDX() != 0 || lwjgl::Mouse::getEventDY() != 0)
		clearKeyboardSelectionFromPointer();
#if !PLATFORM_PS2 && !PLATFORM_WII
	if (lwjgl::Mouse::getEventButtonState())
		clearKeyboardSelectionFromPointer();
#endif

	if (lwjgl::Mouse::getEventButtonState())
	{
		int_t x = (lwjgl::Mouse::getEventX() * width) / mc->displayWidth;
		int_t y = height - (lwjgl::Mouse::getEventY() * height) / mc->displayHeight - 1;
		mouseClicked(x, y, lwjgl::Mouse::getEventButton());
	}
	else
	{
		int_t x = (lwjgl::Mouse::getEventX() * width) / mc->displayWidth;
		int_t y = height - (lwjgl::Mouse::getEventY() * height) / mc->displayHeight - 1;
		mouseMovedOrUp(x, y, lwjgl::Mouse::getEventButton());
	}
}

void GuiScreen::handleKeyboardInput()
{
	if (lwjgl::Keyboard::getEventKeyState())
	{
		if (lwjgl::Keyboard::getEventKey() == 87)
		{
			mc->toggleFullscreen();
			return;
		}
		if (handleJavaUiNavigationKey(lwjgl::Keyboard::getEventKey()))
			return;
		keyTyped(lwjgl::Keyboard::getEventCharacter(), lwjgl::Keyboard::getEventKey());
	}
}

void GuiScreen::handleSpecializedMenuInput()
{
}

bool GuiScreen::usesSpecializedMenuNavigation() const
{
	return false;
}

bool GuiScreen::isJavaUiKeyboardNavigationEnabled() const
{
	if (mc == nullptr || usesSpecializedMenuNavigation())
		return false;
	if (platformPadRebindExclusive() || platformContainerNavigationActive())
		return false;
#if PLATFORM_PS2 || PLATFORM_WII
	if (platformTextInputExclusive())
		return false;
#endif
	return true;
}

void GuiScreen::syncKeyboardSelection()
{
	if (keyboardSelectedControlIndex < 0)
	{
		for (GuiButton *button : controlList)
		{
			if (button != nullptr)
				button->setKeyboardSelected(false);
		}
		return;
	}

	const int_t count = static_cast<int_t>(controlList.size());
	if (keyboardSelectedControlIndex >= count || controlList[keyboardSelectedControlIndex] == nullptr ||
		!controlList[keyboardSelectedControlIndex]->enabled || !controlList[keyboardSelectedControlIndex]->enabled2)
	{
		keyboardSelectedControlIndex = -1;
		for (int_t i = 0; i < count; ++i)
		{
			GuiButton *button = controlList[i];
			if (button != nullptr && button->enabled && button->enabled2)
			{
				keyboardSelectedControlIndex = i;
				break;
			}
		}
	}

	for (int_t i = 0; i < count; ++i)
	{
		if (controlList[i] != nullptr)
			controlList[i]->setKeyboardSelected(i == keyboardSelectedControlIndex);
	}
}

bool GuiScreen::moveKeyboardSelection(int_t direction)
{
	if (direction == 0 || controlList.empty())
		return false;

	if (focusedTextField != nullptr)
		focusedTextField->setFocused(false);

	const int_t count = static_cast<int_t>(controlList.size());
	int_t index = keyboardSelectedControlIndex;
	if (index < 0 || index >= count)
		index = direction > 0 ? -1 : 0;

	for (int_t attempt = 0; attempt < count; ++attempt)
	{
		index = (index + (direction > 0 ? 1 : -1) + count) % count;
		GuiButton *button = controlList[index];
		if (button != nullptr && button->enabled && button->enabled2)
		{
			keyboardSelectedControlIndex = index;
			syncKeyboardSelection();
			moveMenuCursorToKeyboardSelection();
			return true;
		}
	}
	return false;
}

bool GuiScreen::activateKeyboardSelection()
{
	if (focusedTextField != nullptr || controlList.empty())
		return false;

	if (keyboardSelectedControlIndex < 0)
	{
		for (int_t i = 0; i < static_cast<int_t>(controlList.size()); ++i)
		{
			GuiButton *button = controlList[i];
			if (button != nullptr && button->enabled && button->enabled2)
			{
				keyboardSelectedControlIndex = i;
				break;
			}
		}
	}
	syncKeyboardSelection();
	if (keyboardSelectedControlIndex < 0 || keyboardSelectedControlIndex >= static_cast<int_t>(controlList.size()))
		return false;

	GuiButton *button = controlList[keyboardSelectedControlIndex];
	if (button == nullptr || !button->enabled || !button->enabled2)
		return false;

	if (mc != nullptr && mc->sndManager != nullptr)
	{
		const char *sound = mc->gameSettings != nullptr && mc->gameSettings->legacyUI
			? "random.action" : "random.click";
		mc->sndManager->playSoundFX(sound, 1.0f, 1.0f);
	}
	actionPerformed(button);
	return true;
}

bool GuiScreen::adjustKeyboardSelection(int_t direction)
{
	if (direction == 0 || focusedTextField != nullptr)
		return false;
	syncKeyboardSelection();
	if (keyboardSelectedControlIndex < 0 || keyboardSelectedControlIndex >= static_cast<int_t>(controlList.size()))
		return false;

	GuiButton *button = controlList[keyboardSelectedControlIndex];
	if (button == nullptr || !button->enabled || !button->enabled2)
		return false;
	const bool adjusted = button->adjustKeyboard(mc, direction);
	moveMenuCursorToKeyboardSelection();
	return adjusted;
}

bool GuiScreen::handleJavaUiNavigationKey(int_t key)
{
	if (!isJavaUiKeyboardNavigationEnabled())
		return false;

	if (key == lwjgl::Keyboard::KEY_UP)
		return moveKeyboardSelection(-1);
	if (key == lwjgl::Keyboard::KEY_DOWN)
		return moveKeyboardSelection(1);
	if (key == lwjgl::Keyboard::KEY_LEFT)
		return adjustKeyboardSelection(-1);
	if (key == lwjgl::Keyboard::KEY_RIGHT)
		return adjustKeyboardSelection(1);
	if (key == lwjgl::Keyboard::KEY_RETURN)
		return activateKeyboardSelection();
	return false;
}

void GuiScreen::moveMenuCursorToKeyboardSelection()
{
#if PLATFORM_PS2 || PLATFORM_WII
	if (mc == nullptr || keyboardSelectedControlIndex < 0 ||
		keyboardSelectedControlIndex >= static_cast<int_t>(controlList.size()) || width <= 0 || height <= 0)
		return;

	GuiButton *button = controlList[keyboardSelectedControlIndex];
	if (button == nullptr)
		return;
	const int_t guiX = button->xPosition + button->getButtonWidth() / 2;
	const int_t guiY = button->yPosition + button->getButtonHeight() / 2;
	const int_t physicalX = guiX * mc->displayWidth / width;
	const int_t physicalY = guiY * mc->displayHeight / height;
	platformSetMenuCursor(physicalX, physicalY);
#endif
}

void GuiScreen::clearKeyboardSelectionFromPointer()
{
	if (keyboardSelectedControlIndex < 0)
		return;
	keyboardSelectedControlIndex = -1;
	for (GuiButton *button : controlList)
	{
		if (button != nullptr)
			button->setKeyboardSelected(false);
	}
}

#if PLATFORM_PS2 || PLATFORM_WII
void GuiScreen::handleConsoleJavaUiNavigation()
{
	if (!isJavaUiKeyboardNavigationEnabled())
		return;

	const bool suppressPointerInput = mc != nullptr && mc->currentScreen != nullptr &&
		mc->currentScreen->suppressesPlatformPointerInput();

#if PLATFORM_WII
	if (!suppressPointerInput && platformMenuPointerActive())
		return;
	const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
	if ((pad.pressed & PLATFORM_TEXT_UP) != 0)
		moveKeyboardSelection(-1);
	else if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0)
		moveKeyboardSelection(1);
	else if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
		adjustKeyboardSelection(-1);
	else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
		adjustKeyboardSelection(1);
	if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
		activateKeyboardSelection();
#else
	const PlatformGamepadSnapshot stick = platformGamepadSnapshot(platformMenuPad());
	if (!suppressPointerInput &&
		(stick.leftX < -0.20f || stick.leftX > 0.20f || stick.leftY < -0.20f || stick.leftY > 0.20f))
	{
		clearKeyboardSelectionFromPointer();
		return;
	}

	const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
	if ((pad.pressed & PLATFORM_TEXT_UP) != 0)
		moveKeyboardSelection(-1);
	else if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0)
		moveKeyboardSelection(1);
	else if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
		adjustKeyboardSelection(-1);
	else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
		adjustKeyboardSelection(1);

	if (menuPointerInputSuppressed(mc) && (pad.pressed & PLATFORM_TEXT_TYPE) != 0)
		activateKeyboardSelection();
#endif
}
#endif

void GuiScreen::notifyTextFieldFocus(GuiTextField *field, bool focused)
{
	if (focused)
	{
		focusedTextField = field;
		clearKeyboardSelectionFromPointer();
	}
	else if (focusedTextField == field)
	{
		focusedTextField = nullptr;
	}
}

void GuiScreen::updateScreen()
{
}

void GuiScreen::onGuiClosed()
{
}

void GuiScreen::drawDefaultBackground()
{
	drawWorldBackground(0);
}

void GuiScreen::drawWorldBackground(int_t ticks)
{
	if (mc->theWorld != nullptr)
	{
		drawGradientRect(0, 0, width, height, 0xc0101010, 0xd0101010);
	}
	else
	{
		drawBackground(ticks);
	}
}

void GuiScreen::drawBackground(int_t ticks)
{
	renderDisable(RenderCapability::Lighting);
	renderDisable(RenderCapability::Fog);
	Tessellator *tess = &Tessellator::instance;
	renderBindTexture(mc->renderEngine->getTexture("/gui/background.png"));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	float_t f = 32.0f;
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0x404040);
	tess->addVertexWithUV(0,     height, 0.0, 0.0,              (float_t)height / f + (float_t)ticks);
	tess->addVertexWithUV(width, height, 0.0, (float_t)width / f, (float_t)height / f + (float_t)ticks);
	tess->addVertexWithUV(width, 0,      0.0, (float_t)width / f, 0 + ticks);
	tess->addVertexWithUV(0,     0,      0.0, 0.0,              0 + ticks);
	tess->draw();
}

bool GuiScreen::doesGuiPauseGame()
{
	return true;
}

void GuiScreen::deleteWorld(bool confirmed, int_t worldNum)
{
}

void GuiScreen::confirmClicked(bool confirmed, int_t id)
{
	deleteWorld(confirmed, id);
}

void GuiScreen::selectNextField()
{
}
