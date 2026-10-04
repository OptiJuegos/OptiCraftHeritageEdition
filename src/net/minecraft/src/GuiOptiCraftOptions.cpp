#include "net/minecraft/src/UiStrings.h"
#include "GuiOptiCraftOptions.h"

#include "EnumOptions.h"
#include "EntityRenderer.h"
#include "GameSettings.h"
#include "GuiButton.h"
#include "GuiSmallButton.h"
#include "GuiDeadzoneSettings.h"
#include "GuiTextField.h"
#include "GuiTextFieldSelector.h"
#include "Minecraft.h"
#include "ScaledResolution.h"
#include "Session.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformUserSettings.h"
#include "net/minecraft/src/legacy/LegacyUiPolicy.h"

namespace
{
constexpr int_t BUTTON_DONE = 200;
constexpr int_t BUTTON_ALTERNATIVE_CONTROLS = 201;
constexpr int_t BUTTON_DEADZONE = 202;
constexpr int_t BUTTON_ASPECT_RATIO = 203;
constexpr int_t BUTTON_LEGACY_UI = 204;
constexpr int_t BUTTON_LEGACY_LOOK = 205;
constexpr int_t BUTTON_EDIT_PLAYER_NAME = 206;
constexpr int_t BUTTON_LEGACY_CRAFTING = 207;
constexpr int_t BUTTON_LEGACY_CREATIVE = 208;
#if defined(CTR_PLATFORM)
// The face-button camera toggle (see DsInput.cpp): the option only exists
// on the 3DS, where it remaps gameplay controls; on the other platforms the
// diamond already has dedicated actions and the field is meaningless.
constexpr int_t BUTTON_FACE_CAMERA = 209;
// The 3DS dual-screen HUD options below it: the touch map slot and the
// coordinates strip.
constexpr int_t BUTTON_TOUCH_MAP = 210;
constexpr int_t BUTTON_TOUCH_COORDS = 211;
// The auto-jump toggle (see EntityPlayerSP::queueAutoJump): the option only
// exists on the 3DS, paired with the face-button camera in the same row.
constexpr int_t BUTTON_AUTO_JUMP = 212;
// The Pocket-Edition touch gestures toggle (see DsInput.cpp's pad gestures):
// tap to place/swing, hold to break/use with the stylus. 3DS only. Shown as
// "Touch Click"; the stored settings key stays pocketTouch so existing
// options.txt files keep their value.
constexpr int_t BUTTON_POCKET_TOUCH = 213;
// The toggle-sneak option (see GameSettings::toggleShift): the option only
// exists on the 3DS, paired with Touch Click in the same row.
constexpr int_t BUTTON_TOGGLE_SHIFT = 214;
#endif
}

GuiOptiCraftOptions::GuiOptiCraftOptions(GuiScreen *parent, GameSettings *options)
	: parentScreen(parent), settings(options), nameField(nullptr)
{
}

GuiOptiCraftOptions::~GuiOptiCraftOptions()
{
	delete nameField;
}

void GuiOptiCraftOptions::initGui()
{
	controlList.clear();
	delete nameField;

	int_t topY = height / 6 - 8;
	if (topY < 18) topY = 18;

	nameField = new GuiTextField(this, fontRenderer,
		width / 2 - 100, topY + 14, 200, 20, settings->playerName);
	nameField->setMaxStringLength(16);
	nameField->setFocused(false);
	controlList.push_back(new GuiTextFieldSelector(BUTTON_EDIT_PLAYER_NAME,
		width / 2 - 100, topY + 14, 200, 20));

	int_t by = topY + 40;
	// Row 1: Legacy UI & Legacy Look
	controlList.push_back(new GuiSmallButton(BUTTON_LEGACY_UI, width / 2 - 155, by,
		legacyUiOptionLabel(settings->legacyUI)));
	controlList.push_back(new GuiSmallButton(BUTTON_LEGACY_LOOK, width / 2 + 5, by,
		uiText("Legacy Look: ") + std::string(settings->legacyLook ? uiText("ON") : uiText("OFF"))));
	by += 24;

	// Row 2: Legacy Crafting & Legacy Creative
	controlList.push_back(new GuiSmallButton(BUTTON_LEGACY_CRAFTING, width / 2 - 155, by,
		uiText("Legacy Crafting: ") + std::string(settings->legacyCrafting ? uiText("ON") : uiText("OFF"))));
	controlList.push_back(new GuiSmallButton(BUTTON_LEGACY_CREATIVE, width / 2 + 5, by,
		uiText("Legacy Creative: ") + std::string(settings->legacyCreative ? uiText("ON") : uiText("OFF"))));
	by += 24;

#if defined(CTR_PLATFORM)
	controlList.push_back(new GuiSmallButton(BUTTON_FACE_CAMERA, width / 2 - 155, by,
		uiText("Face-Button Camera: ") + std::string(settings->faceButtonCamera ? uiText("ON") : uiText("OFF"))));
	controlList.push_back(new GuiSmallButton(BUTTON_AUTO_JUMP, width / 2 + 5, by,
		uiText("Auto Jump: ") + std::string(settings->autoJump ? uiText("ON") : uiText("OFF"))));
	by += 24;
	controlList.push_back(new GuiSmallButton(BUTTON_TOUCH_MAP, width / 2 - 155, by,
		uiText("Touch Map: ") + std::string(settings->touchMap ? uiText("ON") : uiText("OFF"))));
	// Toggle Shift sits directly under Auto Jump (same column as it).
	controlList.push_back(new GuiSmallButton(BUTTON_TOGGLE_SHIFT, width / 2 + 5, by,
		uiText("Toggle Shift: ") + std::string(settings->toggleShift ? uiText("ON") : uiText("OFF"))));
	by += 24;
	controlList.push_back(new GuiSmallButton(BUTTON_TOUCH_COORDS, width / 2 - 155, by,
		uiText("Touch Coords: ") + std::string(settings->touchCoords ? uiText("ON") : uiText("OFF"))));
	controlList.push_back(new GuiSmallButton(BUTTON_POCKET_TOUCH, width / 2 + 5, by,
		uiText("Touch Click: ") + std::string(settings->pocketTouch ? uiText("ON") : uiText("OFF"))));
	by += 24;
#endif

#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	controlList.push_back(new GuiSmallButton(BUTTON_ASPECT_RATIO, width / 2 - 155, by,
		settings->getKeyBinding(EnumOptions::ASPECT_RATIO)));
#ifdef WII_PLATFORM
	controlList.push_back(new GuiSmallButton(BUTTON_ALTERNATIVE_CONTROLS, width / 2 + 5, by,
		uiText("Alt Controls: ") + std::string(settings->alternativeControllerLayout ? uiText("ON") : uiText("OFF"))));
	by += 24;
	controlList.push_back(new GuiButton(BUTTON_DEADZONE, width / 2 - 100, by, uiText("Deadzone Settings...")));
	by += 24;
#elif PLATFORM_HAS_CONTROLLER_CALIBRATION
	controlList.push_back(new GuiSmallButton(BUTTON_DEADZONE, width / 2 + 5, by, uiText("Deadzone Settings...")));
	by += 24;
#else
	by += 24;
#endif
#else
#ifdef WII_PLATFORM
	controlList.push_back(new GuiSmallButton(BUTTON_ALTERNATIVE_CONTROLS, width / 2 - 155, by,
		uiText("Alt Controls: ") + std::string(settings->alternativeControllerLayout ? uiText("ON") : uiText("OFF"))));
	controlList.push_back(new GuiSmallButton(BUTTON_DEADZONE, width / 2 + 5, by, uiText("Deadzone Settings...")));
	by += 24;
#elif PLATFORM_HAS_CONTROLLER_CALIBRATION
	controlList.push_back(new GuiButton(BUTTON_DEADZONE, width / 2 - 100, by, uiText("Deadzone Settings...")));
	by += 24;
#endif
#endif

	controlList.push_back(new GuiButton(BUTTON_DONE, width / 2 - 100, by + 4, uiText("Done")));
}

void GuiOptiCraftOptions::updateScreen()
{
    GuiScreen::updateScreen();
	if (nameField != nullptr)
		nameField->updateCursorCounter();
}

void GuiOptiCraftOptions::onGuiClosed()
{
	if (nameField != nullptr)
		nameField->setFocused(false);
}

std::string GuiOptiCraftOptions::sanitizeName(const std::string &name)
{
	std::string result;
	for (char c : name)
	{
		bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '_';
		if (valid && result.size() < 16)
			result.push_back(c);
	}
	return result.empty() ? "Player" : result;
}

void GuiOptiCraftOptions::saveIdentity()
{
	settings->playerName = sanitizeName(nameField != nullptr ? nameField->getText() : "");
	if (nameField != nullptr)
		nameField->setText(settings->playerName);
	if (mc->session != nullptr)
		mc->session->username = settings->playerName;
}

void GuiOptiCraftOptions::saveAndClose()
{
	saveIdentity();
	settings->saveOptions();
	mc->displayGuiScreen(parentScreen);
}

void GuiOptiCraftOptions::keyTyped(char_t c, int_t key)
{
	if (c == '\r' || key == lwjgl::Keyboard::KEY_RETURN)
	{
		saveAndClose();
		return;
	}
	if (nameField != nullptr)
		nameField->textboxKeyTyped(c, key);
}

void GuiOptiCraftOptions::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	if (nameField != nullptr)
		nameField->mouseClicked(x, y, button);
}

void GuiOptiCraftOptions::actionPerformed(GuiButton *button)
{
	if (button == nullptr || !button->enabled)
		return;
	if (button->id == BUTTON_EDIT_PLAYER_NAME)
	{
		if (nameField != nullptr)
			nameField->setFocused(true);
		return;
	}

	// Any option can save or rebuild this screen. Commit the field first so a
	// freshly-created text box and options.txt both see the edited identity.
	saveIdentity();
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	if (button->id == BUTTON_ASPECT_RATIO)
	{
		settings->setOptionValue(EnumOptions::ASPECT_RATIO, 1);
		ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
		setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
		return;
	}
#endif
	if (button->id == BUTTON_LEGACY_UI)
	{
		const int_t previousScale = settings->guiScale;
		settings->setLegacyUiEnabled(!settings->legacyUI);
		settings->saveOptions();
		if (settings->guiScale != previousScale && mc != nullptr)
		{
			ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
			setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
			return;
		}
		button->displayString = legacyUiOptionLabel(settings->legacyUI);
		return;
	}
	if (button->id == BUTTON_LEGACY_LOOK)
	{
		settings->legacyLook = !settings->legacyLook;
		button->displayString = uiText("Legacy Look: ") + std::string(settings->legacyLook ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		if (mc != nullptr && mc->entityRenderer != nullptr)
			mc->entityRenderer->updateWorldLightLevels();
		return;
	}
	if (button->id == BUTTON_LEGACY_CRAFTING)
	{
		settings->legacyCrafting = !settings->legacyCrafting;
		settings->applyLegacyCraftingBindings();
		button->displayString = uiText("Legacy Crafting: ") + std::string(settings->legacyCrafting ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
	if (button->id == BUTTON_LEGACY_CREATIVE)
	{
		settings->legacyCreative = !settings->legacyCreative;
		button->displayString = uiText("Legacy Creative: ") + std::string(settings->legacyCreative ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
#if defined(CTR_PLATFORM)
	if (button->id == BUTTON_FACE_CAMERA)
	{
		settings->setFaceButtonCamera(!settings->faceButtonCamera);
		button->displayString = uiText("Face-Button Camera: ") +
			std::string(settings->faceButtonCamera ? uiText("ON") : uiText("OFF"));
		return;
	}
	if (button->id == BUTTON_TOUCH_MAP)
	{
		settings->touchMap = !settings->touchMap;
		button->displayString = uiText("Touch Map: ") +
			std::string(settings->touchMap ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
	if (button->id == BUTTON_TOUCH_COORDS)
	{
		settings->touchCoords = !settings->touchCoords;
		button->displayString = uiText("Touch Coords: ") +
			std::string(settings->touchCoords ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
	if (button->id == BUTTON_AUTO_JUMP)
	{
		settings->autoJump = !settings->autoJump;
		button->displayString = uiText("Auto Jump: ") +
			std::string(settings->autoJump ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
	if (button->id == BUTTON_POCKET_TOUCH)
	{
		settings->setPocketTouch(!settings->pocketTouch);
		button->displayString = uiText("Touch Click: ") +
			std::string(settings->pocketTouch ? uiText("ON") : uiText("OFF"));
		return;
	}
	if (button->id == BUTTON_TOGGLE_SHIFT)
	{
		settings->toggleShift = !settings->toggleShift;
		button->displayString = uiText("Toggle Shift: ") +
			std::string(settings->toggleShift ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
#endif
#ifdef WII_PLATFORM
	if (button->id == BUTTON_ALTERNATIVE_CONTROLS)
	{
		settings->alternativeControllerLayout = !settings->alternativeControllerLayout;
		PlatformUserSettings::setAlternativeControls(settings->alternativeControllerLayout);
		button->displayString = uiText("Alt Controls: ") +
			std::string(settings->alternativeControllerLayout ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		return;
	}
#endif
#if PLATFORM_HAS_CONTROLLER_CALIBRATION
	if (button->id == BUTTON_DEADZONE)
	{
		settings->saveOptions();
		mc->displayGuiScreen(new GuiDeadzoneSettings(this, settings));
		return;
	}
#endif
	if (button->id == BUTTON_DONE)
		saveAndClose();
}

void GuiOptiCraftOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	int_t topY = height / 6 - 8;
	if (topY < 18) topY = 18;
	drawCenteredString(fontRenderer, uiText("OptiCraft Options"), width / 2, topY - 12, 0xffffff);
	drawString(fontRenderer, uiText("Player name"), width / 2 - 100, topY + 2, 0xa0a0a0);
	if (nameField != nullptr)
		nameField->drawTextBox();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
