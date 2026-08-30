#pragma once
#ifdef WII_PLATFORM

#include "net/minecraft/src/GuiScreen.h"
#include <string>

class GameSettings;

// Not in Java. Wii has three independent controller families (GameCube pad,
// the Wiimote's own buttons, a Classic Controller) that can all be plugged in
// at once and today all trigger the same action from their own "A" button --
// see WiiButtonBindings.h. GuiControls' one-row-per-action layout has no room
// for three independent bindings per action, so this is a separate screen:
// one row per action, one small button per family that is listening for a
// press from exactly that family. Lives under src/wii/ (not
// net/minecraft/src, where every other GuiScreen lives) because nothing but
// Wii ever constructs it -- see GuiControls.cpp.
class GuiWiiControls : public GuiScreen
{
public:
	enum Action { ACTION_JUMP = 0, ACTION_SNEAK, ACTION_ATTACK, ACTION_USE, ACTION_DROP,
	              ACTION_INVENTORY, ACTION_THIRDPERSON, ACTION_COUNT };
	enum Family { FAMILY_GC = 0, FAMILY_WM, FAMILY_CC, FAMILY_COUNT };

	GuiWiiControls(GuiScreen *parent, GameSettings *settings);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	static int_t slotId(Action action, Family family);
	int_t *fieldFor(Action action, Family family) const;
	std::string labelFor(Action action) const;
	std::string familyName(Family family) const;
	void updateSlotText(Action action, Family family);
	void cancelListening();

	GuiScreen *parentScreen;
	GameSettings *options;
	GuiButton *slotButtons[ACTION_COUNT][FAMILY_COUNT];
	int_t listeningAction;
	int_t listeningFamily;
};

#endif // WII_PLATFORM
