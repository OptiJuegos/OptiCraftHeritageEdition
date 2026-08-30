#include "GuiBetaOptions.h"

#include "EnumOptions.h"
#include "GameSettings.h"
#include "GuiButton.h"
#include "GuiDeadzoneSettings.h"
#include "GuiTextField.h"
#include "Minecraft.h"
#include "ScaledResolution.h"
#include "Session.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformUserSettings.h"

GuiBetaOptions::GuiBetaOptions(GuiScreen *parent, GameSettings *options)
	: parentScreen(parent), settings(options), nameField(nullptr)
{
}

GuiBetaOptions::~GuiBetaOptions()
{
	delete nameField;
}

void GuiBetaOptions::initGui()
{
	controlList.clear();
	delete nameField;
	nameField = new GuiTextField(this, fontRenderer,
		width / 2 - 100, height / 2 - 20, 200, 20, settings->playerName);
	nameField->setMaxStringLength(16);
	nameField->setFocused(false);
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	controlList.push_back(new GuiButton(203, width / 2 - 100, height / 2 + 8,
		settings->getKeyBinding(EnumOptions::ASPECT_RATIO)));
#endif
#ifdef WII_PLATFORM
	controlList.push_back(new GuiButton(201, width / 2 - 100, height / 2 + 32,
		"Alternative controls: " + std::string(settings->alternativeControllerLayout ? "ON" : "OFF")));
	controlList.push_back(new GuiButton(202, width / 2 - 100, height / 2 + 56,
		"Deadzone Settings..."));
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 2 + 80, "Done"));
#elif PLATFORM_HAS_CONTROLLER_CALIBRATION
	controlList.push_back(new GuiButton(202, width / 2 - 100, height / 2 + 32,
		"Deadzone Settings..."));
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 2 + 56, "Done"));
#elif PLATFORM_HAS_ASPECT_RATIO_OPTION
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 2 + 32, "Done"));
#else
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 2 + 20, "Done"));
#endif
}

void GuiBetaOptions::updateScreen()
{
	if (nameField != nullptr)
		nameField->updateCursorCounter();
}

void GuiBetaOptions::onGuiClosed()
{
	if (nameField != nullptr)
		nameField->setFocused(false);
}

std::string GuiBetaOptions::sanitizeName(const std::string &name)
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

void GuiBetaOptions::saveAndClose()
{
	settings->playerName = sanitizeName(nameField != nullptr ? nameField->getText() : "");
	if (mc->session != nullptr)
		mc->session->username = settings->playerName;
	settings->saveOptions();
	mc->displayGuiScreen(parentScreen);
}

void GuiBetaOptions::keyTyped(char_t c, int_t key)
{
	if (c == '\r')
	{
		saveAndClose();
		return;
	}
	if (nameField != nullptr)
		nameField->textboxKeyTyped(c, key);
}

void GuiBetaOptions::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	if (nameField != nullptr)
		nameField->mouseClicked(x, y, button);
}

void GuiBetaOptions::actionPerformed(GuiButton *button)
{
	if (button == nullptr || !button->enabled)
		return;
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	if (button->id == 203)
	{
		settings->playerName = sanitizeName(nameField != nullptr ? nameField->getText() : "");
		if (mc->session != nullptr)
			mc->session->username = settings->playerName;
		settings->setOptionValue(EnumOptions::ASPECT_RATIO, 1);
		ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
		setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
		return;
	}
#endif
#ifdef WII_PLATFORM
	if (button->id == 201)
	{
		settings->alternativeControllerLayout = !settings->alternativeControllerLayout;
		PlatformUserSettings::setAlternativeControls(settings->alternativeControllerLayout);
		button->displayString = "Alternative controls: " +
			std::string(settings->alternativeControllerLayout ? "ON" : "OFF");
		settings->saveOptions();
	}
#endif
#if PLATFORM_HAS_CONTROLLER_CALIBRATION
	if (button->id == 202)
	{
		settings->playerName = sanitizeName(nameField != nullptr ? nameField->getText() : "");
		if (mc->session != nullptr)
			mc->session->username = settings->playerName;
		settings->saveOptions();
		mc->displayGuiScreen(new GuiDeadzoneSettings(this, settings));
		return;
	}
#endif
	if (button->id == 200)
		saveAndClose();
}

void GuiBetaOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, "Release++ Options", width / 2, 30, 0xffffff);
	drawString(fontRenderer, "Player name", width / 2 - 100, height / 2 - 32, 0xa0a0a0);
#ifdef WII_PLATFORM
	drawCenteredString(fontRenderer, "D-pad: move / Nunchuk: camera", width / 2, height / 2 - 50, 0xa0a0a0);
#elif defined(PS2_PLATFORM)
	drawCenteredString(fontRenderer, "Left stick: move / Right stick: camera", width / 2, height / 2 - 50, 0xa0a0a0);
#endif
	if (nameField != nullptr)
		nameField->drawTextBox();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
