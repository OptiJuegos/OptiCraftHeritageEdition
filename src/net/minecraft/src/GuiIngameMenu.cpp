#include "net/minecraft/src/UiStrings.h"
#include "GuiIngameMenu.h"
#include "GuiButton.h"
#include "GuiOptions.h"
#include "GameSettings.h"
#include "legacy/LegacyHelpOptions.h"
#include "legacy/LegacyGuiButton.h"
#include "legacy/LegacyMainMenuLayout.h"
#include "legacy/LegacyMenuHints.h"
#include "legacy/LegacyMenuNavigation.h"
#include "legacy/LegacyPauseStyle.h"
#include "legacy/LegacyUiAssets.h"
#include "GuiMainMenu.h"
#include "GuiAchievements.h"
#include "GuiStats.h"
#include "StatCollector.h"
#include "StatList.h"
#include "MathHelper.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "StatFileWriter.h"
#include "World.h"
#include "SoundManager.h"
#include "java/System.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/Input.h"
#include "platform/PlatformConfig.h"

GuiIngameMenu::GuiIngameMenu()
	: updateCounter2(0)
	, updateCounter(0)
	, selectedControlIndex(-1)
	, hoveredControlIndex(-1)
#if PLATFORM_PS2 || PLATFORM_WII
	, legacyPauseOpenedAtMillis(System::currentTimeMillis())
#endif
#if PLATFORM_PS2
	, ps2PauseStartReleaseLatch(true)
#endif
{
}

bool GuiIngameMenu::usesSpecializedMenuNavigation() const
{
	return mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
}

void GuiIngameMenu::initGui()
{
	updateCounter2 = 0;
	controlList.clear();

	const bool legacyPause = mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
	if (legacyPause)
	{
		const LegacyMainMenuLayout layout = legacyMainMenuLayout(width, height, legacyPauseButtonCount());
		const int_t stride = layout.buttonHeight + layout.buttonSpacing;
		const float_t alpha = legacyPauseButtonOpacity();
		int_t row = 0;

		auto addLegacyButton = [&](int_t id, const std::string &label)
		{
			controlList.push_back(new LegacyGuiButton(id, layout.buttonX,
				layout.firstButtonY + row * stride, layout.buttonWidth, layout.buttonHeight, label, alpha));
			++row;
		};

		addLegacyButton(4, uiText("Resume Game"));
		addLegacyButton(0, uiText("Help & Options"));
		addLegacyButton(5, uiText("Achievements"));
		addLegacyButton(6, uiText("Statistics"));
		addLegacyButton(1, mc->isMultiplayerWorld() ? uiText("Disconnect") : uiText("Save & Quit"));
		hoveredControlIndex = -1;
		syncLegacySelection();
		return;
	}

	int_t off = -16;
	controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 120 + off, StatCollector::translateToLocal("menu.returnToMenu")));
	if (mc->isMultiplayerWorld())
		controlList[0]->displayString = StatCollector::translateToLocal("menu.disconnect");
	controlList.push_back(new GuiButton(4, width / 2 - 100, height / 4 + 24 + off, StatCollector::translateToLocal("menu.returnToGame")));
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 96 + off, StatCollector::translateToLocal("menu.options")));
	controlList.push_back(new GuiButton(5, width / 2 - 100, height / 4 + 48 + off, 98, 20, StatCollector::translateToLocal("gui.achievements")));
	controlList.push_back(new GuiButton(6, width / 2 + 2,   height / 4 + 48 + off, 98, 20, StatCollector::translateToLocal("gui.stats")));
}


void GuiIngameMenu::syncLegacySelection()
{
	if (selectedControlIndex < 0 || selectedControlIndex >= static_cast<int_t>(controlList.size()) ||
		controlList[selectedControlIndex] == nullptr || !controlList[selectedControlIndex]->enabled ||
		!controlList[selectedControlIndex]->enabled2)
	{
		selectedControlIndex = legacyFirstSelectableButton(controlList);
	}
	legacyApplyMenuSelection(controlList, hoveredControlIndex >= 0 ? -1 : selectedControlIndex);
}

void GuiIngameMenu::moveLegacySelection(int_t direction)
{
	if (hoveredControlIndex >= 0)
		return;
	syncLegacySelection();
	const int_t previous = selectedControlIndex;
	selectedControlIndex = legacyNextSelectableButton(controlList, selectedControlIndex, direction);
	legacyApplyMenuSelection(controlList, selectedControlIndex);
	if (selectedControlIndex != previous && mc != nullptr && mc->sndManager != nullptr)
		mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
}

void GuiIngameMenu::activateLegacySelection()
{
	syncLegacySelection();
	const int_t targetIndex = hoveredControlIndex >= 0 ? hoveredControlIndex : selectedControlIndex;
	if (targetIndex < 0 || targetIndex >= static_cast<int_t>(controlList.size()))
		return;
	if (mc != nullptr && mc->sndManager != nullptr)
		mc->sndManager->playSoundFX("random.action", 1.0f, 1.0f);
	actionPerformed(controlList[targetIndex]);
}

void GuiIngameMenu::keyTyped(char_t c, int_t key)
{
	const bool legacyPause = mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
	if (legacyPause)
	{
#if !PLATFORM_PS2 && !PLATFORM_WII
		if (key == lwjgl::Keyboard::KEY_UP)
		{
			moveLegacySelection(-1);
			return;
		}
		if (key == lwjgl::Keyboard::KEY_DOWN)
		{
			moveLegacySelection(1);
			return;
		}
		if (key == lwjgl::Keyboard::KEY_RETURN)
		{
			activateLegacySelection();
			return;
		}
#endif
	}
	GuiScreen::keyTyped(c, key);
}

void GuiIngameMenu::actionPerformed(GuiButton *button)
{
	if (button->id == 0)
	{
		if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
			mc->displayGuiScreen(new LegacyHelpOptions(this, mc->gameSettings, LegacyOptionsBackgroundMode::PausedWorld));
		else
			mc->displayGuiScreen(new GuiOptions(this, mc->gameSettings));
	}
	if (button->id == 1)
	{
		mc->statFileWriter->readStat(StatList::leaveGameStat, 1);
		if (mc->isMultiplayerWorld())
			mc->theWorld->sendQuittingDisconnectingPacket();
		mc->changeWorld1(nullptr);
		mc->displayGuiScreen(new GuiMainMenu());
	}
	if (button->id == 4)
	{
		closeLegacyPause();
	}
	if (button->id == 5)
	{
		mc->displayGuiScreen(new GuiAchievements(mc->statFileWriter));
	}
	if (button->id == 6)
	{
		mc->displayGuiScreen(new GuiStats(this, mc->statFileWriter));
	}
}

void GuiIngameMenu::closeLegacyPause()
{
	if (mc == nullptr)
		return;
	mc->displayGuiScreen(nullptr);
	mc->setIngameFocus();
}

void GuiIngameMenu::handleSpecializedMenuInput()
{
#if PLATFORM_PS2
	const bool legacyPause = mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
	if (!legacyPause)
		return;

	// Consume PS2 pause navigation before GuiScreen::handleInput() runs any
	// generic console helpers. platformTextInputSnapshot() consumes the latched
	// pressed bits, so the specialized owner must be first in the chain.
	const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
	if (!pad.connected)
		return;
	if (!legacyPauseInputDelayElapsed(legacyPauseOpenedAtMillis, System::currentTimeMillis()))
		return;

	// Start is also the gameplay button that opened this pause screen.  The
	// opening edge can still be present in the PS2 pad latch on the first GUI
	// frame, so do not let that same press immediately resume the game.  Rearm
	// Start only after the player has physically released it once.
	std::uint32_t pressed = pad.pressed;
	if (ps2PauseStartReleaseLatch)
	{
		pressed &= ~PLATFORM_TEXT_ENTER;
		if ((pad.held & PLATFORM_TEXT_ENTER) == 0)
			ps2PauseStartReleaseLatch = false;
	}

	if ((pressed & (PLATFORM_TEXT_ENTER | PLATFORM_TEXT_CLOSE | PLATFORM_TEXT_SHIFT)) != 0)
	{
		if (mc->sndManager != nullptr)
			mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
		closeLegacyPause();
		return;
	}

	if ((pressed & PLATFORM_TEXT_UP) != 0)
		moveLegacySelection(-1);
	else if ((pressed & PLATFORM_TEXT_DOWN) != 0)
		moveLegacySelection(1);

	if ((pressed & PLATFORM_TEXT_TYPE) != 0)
		activateLegacySelection();
#endif
}

void GuiIngameMenu::updateScreen()
{
	GuiScreen::updateScreen();
	updateCounter++;
	const bool legacyPause = mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
	if (!legacyPause)
		return;

	syncLegacySelection();
#if PLATFORM_WII
	const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
	if (!legacyPauseInputDelayElapsed(legacyPauseOpenedAtMillis, System::currentTimeMillis()))
		return;
	if ((pad.pressed & PLATFORM_TEXT_UP) != 0)
		moveLegacySelection(-1);
	else if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0)
		moveLegacySelection(1);
	if (!platformMenuPointerActive() && (pad.pressed & PLATFORM_TEXT_TYPE) != 0)
		activateLegacySelection();
	if ((pad.pressed & PLATFORM_TEXT_BACK) != 0)
	{
		if (mc->sndManager != nullptr)
			mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
#endif
}

void GuiIngameMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	const bool legacyPause = mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
	hoveredControlIndex = legacyPause ? legacyHoveredSelectableButton(controlList, mouseX, mouseY) : -1;
	if (hoveredControlIndex >= 0)
		selectedControlIndex = hoveredControlIndex;
	if (legacyPause)
	{
		// Keep the paused world readable like Legacy Console instead of using
		// Java's very dark 0xC0/0xD0 default overlay.
		drawGradientRect(0, 0, width, height, legacyPauseOverlayTopColor(), legacyPauseOverlayBottomColor());

		const LegacyMainMenuLayout layout = legacyMainMenuLayout(width, height, legacyPauseButtonCount());
		if (!legacyDrawTitleTexture(mc, layout, width, zLevel, nullptr))
			drawCenteredString(fontRenderer, "HERITAGE EDITION", width / 2, layout.titleY + 8, 0xffffff);

		bool saving = !mc->theWorld->isSafeToSave(updateCounter2++);
		if (saving || updateCounter < 20)
		{
			float_t f1 = ((float_t)(updateCounter % 10) + partialTick) / 10.0f;
			f1 = MathHelper::sin(f1 * 3.1415927f * 2.0f) * 0.2f + 0.8f;
			int_t k = (int_t)(255.0f * f1);
			drawString(fontRenderer, uiText("Saving level.."), 8, height - 16, k << 16 | k << 8 | k);
		}

		syncLegacySelection();
		drawLegacyMenuHints(fontRenderer, width, height, true);
		GuiScreen::drawScreen(mouseX, mouseY, partialTick);
		return;
	}

	drawDefaultBackground();
	bool saving = !mc->theWorld->isSafeToSave(updateCounter2++);
	if (saving || updateCounter < 20)
	{
		float_t f1 = ((float_t)(updateCounter % 10) + partialTick) / 10.0f;
		f1 = MathHelper::sin(f1 * 3.1415927f * 2.0f) * 0.2f + 0.8f;
		int_t k = (int_t)(255.0f * f1);
		drawString(fontRenderer, uiText("Saving level.."), 8, height - 16, k << 16 | k << 8 | k);
	}
	drawCenteredString(fontRenderer, uiText("Game menu"), width / 2, 40, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
