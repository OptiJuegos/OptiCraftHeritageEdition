#ifdef WII_PLATFORM

#include "wii/input/GuiWiiControls.h"

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "platform/Input.h"
#include "wii/input/WiiPadKeyCodes.h"

namespace
{
	WiiPadFamily wiiPadFamilyFor(GuiWiiControls::Family family)
	{
		if (family == GuiWiiControls::FAMILY_GC) return WiiPadFamily::GameCube;
		if (family == GuiWiiControls::FAMILY_WM) return WiiPadFamily::Wiimote;
		return WiiPadFamily::Classic;
	}
}

GuiWiiControls::GuiWiiControls(GuiScreen *parent, GameSettings *settings)
	: parentScreen(parent), options(settings), listeningAction(-1), listeningFamily(-1)
{
	for (int_t action = 0; action < ACTION_COUNT; ++action)
		for (int_t family = 0; family < FAMILY_COUNT; ++family)
			slotButtons[action][family] = nullptr;
}

int_t GuiWiiControls::slotId(Action action, Family family)
{
	return (int_t)action * FAMILY_COUNT + (int_t)family;
}

int_t *GuiWiiControls::fieldFor(Action action, Family family) const
{
	switch (action)
	{
		case ACTION_JUMP:
			if (family == FAMILY_GC) return &options->wiiGcJump;
			if (family == FAMILY_WM) return &options->wiiWmJump;
			if (family == FAMILY_CC) return &options->wiiCcJump;
			break;
		case ACTION_SNEAK:
			if (family == FAMILY_GC) return &options->wiiGcSneak;
			if (family == FAMILY_WM) return &options->wiiWmSneak;
			if (family == FAMILY_CC) return &options->wiiCcSneak;
			break;
		case ACTION_ATTACK:
			if (family == FAMILY_GC) return &options->wiiGcAttack;
			if (family == FAMILY_WM) return &options->wiiWmAttack;
			if (family == FAMILY_CC) return &options->wiiCcAttack;
			break;
		case ACTION_USE:
			if (family == FAMILY_GC) return &options->wiiGcUse;
			if (family == FAMILY_WM) return &options->wiiWmUse;
			if (family == FAMILY_CC) return &options->wiiCcUse;
			break;
		case ACTION_DROP:
			if (family == FAMILY_GC) return &options->wiiGcDrop;
			if (family == FAMILY_WM) return &options->wiiWmDrop;
			if (family == FAMILY_CC) return &options->wiiCcDrop;
			break;
		case ACTION_INVENTORY:
			if (family == FAMILY_GC) return &options->wiiGcInventory;
			if (family == FAMILY_WM) return &options->wiiWmInventory;
			if (family == FAMILY_CC) return &options->wiiCcInventory;
			break;
		case ACTION_THIRDPERSON:
			if (family == FAMILY_GC) return &options->wiiGcThirdPerson;
			if (family == FAMILY_WM) return &options->wiiWmThirdPerson;
			if (family == FAMILY_CC) return &options->wiiCcThirdPerson;
			break;
		default:
			break;
	}
	return nullptr;
}

std::string GuiWiiControls::labelFor(Action action) const
{
	switch (action)
	{
		case ACTION_JUMP: return "Jump";
		case ACTION_SNEAK: return "Sneak";
		case ACTION_ATTACK: return "Attack";
		case ACTION_USE: return "Use";
		case ACTION_DROP: return "Drop";
		case ACTION_INVENTORY: return "Inventory";
		case ACTION_THIRDPERSON: return "3rd Person";
		default: return "";
	}
}

std::string GuiWiiControls::familyName(Family family) const
{
	switch (family)
	{
		case FAMILY_GC: return "GameCube";
		case FAMILY_WM: return "Wiimote";
		case FAMILY_CC: return "Classic";
		default: return "";
	}
}

void GuiWiiControls::updateSlotText(Action action, Family family)
{
	GuiButton *button = slotButtons[action][family];
	if (button == nullptr)
		return;
	const int_t *field = fieldFor(action, family);
	if (field == nullptr)
		return;
	const char *name = wiiPadButtonName(wiiPadFamilyFor(family), (unsigned)*field);
	button->displayString = name != nullptr ? name : "?";
}

void GuiWiiControls::cancelListening()
{
	listeningAction = -1;
	listeningFamily = -1;
	platformSetPadRebindExclusive(false);
}

void GuiWiiControls::initGui()
{
	controlList.clear();
	cancelListening();

	const int_t colWidth = 70, colGap = 10;
	const int_t startX = width / 2 - (colWidth * FAMILY_COUNT + colGap * (FAMILY_COUNT - 1)) / 2;
	const int_t rowStartY = height / 6 + 36;
	// Shrink to fit ACTION_COUNT rows plus the Done button above the bottom of
	// the screen instead of a fixed 24px: on the common 240-tall scaled GUI
	// (Wii NTSC at guiScale auto) 7 rows at 24px pushed Done off the bottom of
	// the visible area entirely. Caps at 24 so taller screens keep the
	// original spacing.
	int_t rowSpacing = (height - rowStartY - 14 - 20) / ACTION_COUNT;
	if (rowSpacing > 24) rowSpacing = 24;
	if (rowSpacing < 12) rowSpacing = 12;

	for (int_t action = 0; action < ACTION_COUNT; ++action)
	{
		const int_t rowY = rowStartY + action * rowSpacing;
		for (int_t family = 0; family < FAMILY_COUNT; ++family)
		{
			slotButtons[action][family] = nullptr;
			if (fieldFor((Action)action, (Family)family) == nullptr)
				continue;
			const int_t x = startX + family * (colWidth + colGap);
			GuiButton *button = new GuiButton(slotId((Action)action, (Family)family), x, rowY, colWidth, 20, "");
			controlList.push_back(button);
			slotButtons[action][family] = button;
			updateSlotText((Action)action, (Family)family);
		}
	}

	controlList.push_back(new GuiButton(100, width / 2 - 35, rowStartY + ACTION_COUNT * rowSpacing + 14, 70, 20, "Done"));
}

void GuiWiiControls::actionPerformed(GuiButton *button)
{
	if (listeningAction >= 0)
		updateSlotText((Action)listeningAction, (Family)listeningFamily);

	if (button->id == 100)
	{
		cancelListening();
		options->saveOptions();
		mc->displayGuiScreen(parentScreen);
		return;
	}

	for (int_t action = 0; action < ACTION_COUNT; ++action)
	{
		for (int_t family = 0; family < FAMILY_COUNT; ++family)
		{
			if (slotButtons[action][family] == button)
			{
				listeningAction = action;
				listeningFamily = family;
				button->displayString = "> " + button->displayString + " <";
				platformSetPadRebindExclusive(true);
				return;
			}
		}
	}
}

void GuiWiiControls::keyTyped(char_t c, int_t key)
{
	if (listeningAction < 0)
	{
		GuiScreen::keyTyped(c, key);
		return;
	}

	// Not a pad-button capture event (see WiiGameCubePad.cpp / WiiRemote.cpp),
	// or it came from a different controller family than the slot being
	// edited -- keep listening either way.
	const WiiPadFamily *family = wiiPadKeyFamily(key);
	if (family == nullptr || *family != wiiPadFamilyFor((Family)listeningFamily))
		return;

	int_t *field = fieldFor((Action)listeningAction, (Family)listeningFamily);
	if (field != nullptr)
	{
		*field = (int_t)wiiPadKeyButtonMask(key);
		options->syncControllerBindingsToPlatform();
	}

	const Action editedAction = (Action)listeningAction;
	const Family editedFamily = (Family)listeningFamily;
	cancelListening();
	updateSlotText(editedAction, editedFamily);
}

void GuiWiiControls::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, "Wii Pad Bindings", width / 2, 20, 0xffffff);

	const int_t colWidth = 70, colGap = 10;
	const int_t startX = width / 2 - (colWidth * FAMILY_COUNT + colGap * (FAMILY_COUNT - 1)) / 2;
	const int_t headerY = height / 6 + 20;
	for (int_t family = 0; family < FAMILY_COUNT; ++family)
	{
		const int_t x = startX + family * (colWidth + colGap);
		drawCenteredString(fontRenderer, familyName((Family)family), x + colWidth / 2, headerY, 0xa0a0a0);
	}

	const int_t rowStartY = height / 6 + 36;
	int_t rowSpacing = (height - rowStartY - 14 - 20) / ACTION_COUNT;
	if (rowSpacing > 24) rowSpacing = 24;
	if (rowSpacing < 12) rowSpacing = 12;
	for (int_t action = 0; action < ACTION_COUNT; ++action)
		drawString(fontRenderer, labelFor((Action)action), startX - 74, rowStartY + action * rowSpacing + 6, 0xffffff);

	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

#endif // WII_PLATFORM
