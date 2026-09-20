#include "net/minecraft/src/UiStrings.h"
#include "GuiOptiCraftOptions.h"

#include "EnumOptions.h"
#include "EntityRenderer.h"
#include "GameSettings.h"
#include "GuiButton.h"
#include "GuiDeadzoneSettings.h"
#ifdef PS2_PLATFORM
#include "GuiWorldStorage.h"
#endif
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
constexpr int_t BUTTON_EDIT_PLAYER_NAME = 206;
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
	nameField = new GuiTextField(this, fontRenderer,
		width / 2 - 100, height / 2 - 20, 200, 20, settings->playerName);
	nameField->setMaxStringLength(16);
	nameField->setFocused(false);
	controlList.push_back(new GuiTextFieldSelector(BUTTON_EDIT_PLAYER_NAME,
		width / 2 - 100, height / 2 - 20, 200, 20));

	int_t buttonY = height / 2 + 4;
#if PLATFORM_HAS_ASPECT_RATIO_OPTION && !PLATFORM_PS2
	controlList.push_back(new GuiButton(203, width / 2 - 100, buttonY,
		settings->getKeyBinding(EnumOptions::ASPECT_RATIO)));
	buttonY += 20;
#endif
	controlList.push_back(new GuiButton(204, width / 2 - 100, buttonY,
		legacyUiOptionLabel(settings->legacyUI)));
	buttonY += 20;
	controlList.push_back(new GuiButton(205, width / 2 - 100, buttonY,
		uiText("Legacy Look: ") + std::string(settings->legacyLook ? uiText("ON") : uiText("OFF"))));
	buttonY += 20;
#ifdef WII_PLATFORM
	controlList.push_back(new GuiButton(201, width / 2 - 100, buttonY,
		uiText("Alternative controls: ") + std::string(settings->alternativeControllerLayout ? uiText("ON") : uiText("OFF"))));
	buttonY += 20;
	controlList.push_back(new GuiButton(202, width / 2 - 100, buttonY, uiText("Deadzone Settings...")));
	buttonY += 20;
#elif PLATFORM_HAS_CONTROLLER_CALIBRATION && !PLATFORM_PS2
	controlList.push_back(new GuiButton(202, width / 2 - 100, buttonY, uiText("Deadzone Settings...")));
	buttonY += 20;
#endif
#ifdef PS2_PLATFORM
    controlList.push_back(new GuiButton(207, width / 2 - 100, buttonY, uiText("World Storage...")));
    buttonY += 20;
#endif
	controlList.push_back(new GuiButton(200, width / 2 - 100, buttonY, uiText("Done")));
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
#ifdef PS2_PLATFORM
    if (button->id == 207)
    {
        saveIdentity();
        settings->saveOptions();
        mc->displayGuiScreen(new GuiWorldStorage(this, settings));
        return;
    }
#endif
	if (button->id == BUTTON_EDIT_PLAYER_NAME)
	{
		if (nameField != nullptr)
			nameField->setFocused(true);
		return;
	}

	// Any option can save or rebuild this screen. Commit the field first so a
	// freshly-created text box and options.txt both see the edited identity.
	saveIdentity();
#if PLATFORM_HAS_ASPECT_RATIO_OPTION && !PLATFORM_PS2
	if (button->id == 203)
	{
		settings->setOptionValue(EnumOptions::ASPECT_RATIO, 1);
		ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
		setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
		return;
	}
#endif
	if (button->id == 204)
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
	if (button->id == 205)
	{
		settings->legacyLook = !settings->legacyLook;
		button->displayString = uiText("Legacy Look: ") + std::string(settings->legacyLook ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
		if (mc != nullptr && mc->entityRenderer != nullptr)
			mc->entityRenderer->updateWorldLightLevels();
		return;
	}
#ifdef WII_PLATFORM
	if (button->id == 201)
	{
		settings->alternativeControllerLayout = !settings->alternativeControllerLayout;
		PlatformUserSettings::setAlternativeControls(settings->alternativeControllerLayout);
		button->displayString = uiText("Alternative controls: ") +
			std::string(settings->alternativeControllerLayout ? uiText("ON") : uiText("OFF"));
		settings->saveOptions();
	}
#endif
#if PLATFORM_HAS_CONTROLLER_CALIBRATION && !PLATFORM_PS2
	if (button->id == 202)
	{
		settings->saveOptions();
		mc->displayGuiScreen(new GuiDeadzoneSettings(this, settings));
		return;
	}
#endif
	if (button->id == 200)
		saveAndClose();
}

void GuiOptiCraftOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, PLATFORM_PS2 ? uiText("Game Options") : uiText("OptiCraft Options"), width / 2, 30, 0xffffff);
	drawString(fontRenderer, uiText("Player name"), width / 2 - 100, height / 2 - 32, 0xa0a0a0);
#ifdef WII_PLATFORM
	drawCenteredString(fontRenderer, uiText("D-pad: move / Nunchuk: camera"), width / 2, height / 2 - 50, 0xa0a0a0);
#elif defined(PS2_PLATFORM)
	drawCenteredString(fontRenderer, uiText("Left stick: move / Right stick: camera"), width / 2, height / 2 - 50, 0xa0a0a0);
#endif
	if (nameField != nullptr)
		nameField->drawTextBox();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
