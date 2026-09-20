#include "net/minecraft/src/UiStrings.h"
#include "GuiControls.h"
#include "GuiSmallButton.h"
#include "GuiButton.h"
#include "GuiDeadzoneSettings.h"
#include "GameSettings.h"
#include "KeyBinding.h"
#include "StringTranslate.h"
#include "Minecraft.h"
#include "platform/Input.h"
#include "platform/PlatformConfig.h"
#include "wii/input/GuiWiiControls.h"

GuiControls::GuiControls(GuiScreen *parent, GameSettings *settings)
	: screenTitle(uiText("Controls"))
	, buttonId(-1)
	, parentScreen(parent)
	, options(settings)
{
}

int_t GuiControls::getLeftEdge() const
{
	return width / 2 - 155;
}

int_t GuiControls::bindingTop() const
{
	return PLATFORM_PS2 ? 32 : height / 6;
}

int_t GuiControls::bindingStride() const
{
	return PLATFORM_PS2 ? 20 : 24;
}

void GuiControls::initGui()
{
	StringTranslate *tr = StringTranslate::getInstance();
	int_t left = getLeftEdge();
	for (int_t j = 0; j < (int_t)options->keyBindings.size(); j++)
	{
		controlList.push_back(new GuiSmallButton(j,
			left + (j % 2) * 160, bindingTop() + bindingStride() * (j >> 1),
			70, 20, options->getOptionDisplayString(j)));
	}
	int_t nextY = bindingTop() + bindingStride() * (((int_t)options->keyBindings.size() + 1) / 2);
#ifdef WII_PLATFORM
	controlList.push_back(new GuiButton(201, width / 2 - 100, nextY, uiText("Wii Pad Bindings...")));
	nextY += 24;
#endif
#if PLATFORM_PS2
	controlList.push_back(new GuiButton(202, left, nextY, 150, 20, uiText("Deadzone Settings...")));
	controlList.push_back(new GuiButton(200, left + 160, nextY, 150, 20, tr->translateKey("gui.done")));
#else
	controlList.push_back(new GuiButton(200, width / 2 - 100, nextY, tr->translateKey("gui.done")));
#endif
	screenTitle = tr->translateKey("controls.title");
}

void GuiControls::actionPerformed(GuiButton *button)
{
	for (int_t i = 0; i < (int_t)options->keyBindings.size(); i++)
		controlList[i]->displayString = options->getOptionDisplayString(i);

#if PLATFORM_PS2
	if (button->id == 202)
	{
		buttonId = -1;
		platformSetPadRebindExclusive(false);
		options->saveOptions();
		mc->displayGuiScreen(new GuiDeadzoneSettings(this, options));
		return;
	}
#endif
	if (button->id == 200)
	{
		// Guard against uiText("Done") being clicked while a binding is still
		// listening (buttonId >= 0) -- otherwise this screen goes away with
		// pad-rebind-exclusive left stuck on, permanently breaking every
		// other menu's button synthesis on PS2.
		platformSetPadRebindExclusive(false);
		mc->displayGuiScreen(parentScreen);
		return;
	}
#ifdef WII_PLATFORM
	if (button->id == 201)
	{
		options->saveOptions();
		mc->displayGuiScreen(new GuiWiiControls(this, options));
		return;
	}
#endif
	if (button->id >= 0 && button->id < (int_t)options->keyBindings.size())
	{
		buttonId = button->id;
		button->displayString = "> " + options->getOptionDisplayString(button->id) + " <";
		platformSetPadRebindExclusive(true);
	}
}

void GuiControls::keyTyped(char_t c, int_t key)
{
	if (buttonId >= 0)
	{
		options->setKeyBinding(buttonId, key);
		controlList[buttonId]->displayString = options->getOptionDisplayString(buttonId);
		buttonId = -1;
		platformSetPadRebindExclusive(false);
	}
	else
	{
		GuiScreen::keyTyped(c, key);
	}
}


void GuiControls::mouseClicked(int_t x, int_t y, int_t button)
{
	if (buttonId >= 0)
	{
		options->setKeyBinding(buttonId, -100 + button);
		controlList[buttonId]->displayString = options->getOptionDisplayString(buttonId);
		buttonId = -1;
		platformSetPadRebindExclusive(false);
		return;
	}
	GuiScreen::mouseClicked(x, y, button);
}

void GuiControls::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, screenTitle, width / 2, 20, 0xffffff);
	int_t left = getLeftEdge();
	for (int_t l = 0; l < (int_t)options->keyBindings.size(); l++)
	{
		bool duplicate = false;
		for (int_t other = 0; other < (int_t)options->keyBindings.size(); ++other)
		{
			if (other != l && options->keyBindings[l]->keyCode == options->keyBindings[other]->keyCode)
			{
				duplicate = true;
				break;
			}
		}

		if (buttonId == l)
			controlList[l]->displayString = "\xc2\xa7" "f> " "\xc2\xa7" "e??? " "\xc2\xa7" "f<";
		else if (duplicate)
			controlList[l]->displayString = std::string("\xc2\xa7" "c") + options->getOptionDisplayString(l);
		else
			controlList[l]->displayString = options->getOptionDisplayString(l);

		drawString(fontRenderer, options->getKeyBindingDescription(l),
		           left + (l % 2) * 160 + 70 + 6, bindingTop() + bindingStride() * (l >> 1) + 7, -1);
	}
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
