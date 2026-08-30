#pragma once

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)

#include "java/Type.h"

class GuiContainer;
class Slot;

// D-pad slot navigation for the container screens (inventory, workbench, chest,
// furnace, dispenser) on PS2 and Wii.
//
// Both consoles drive the shared GUI code through a simulated mouse: the PS2
// integrates a cursor from the pad, the Wii points at the screen with the IR
// sensor. That is fine for a screen made of 200x20 buttons and awkward for a
// 16x16 slot grid -- landing on the 2x2 crafting square while the stick drifts
// is the slowest part of crafting on a pad.
//
// This moves the CURSOR rather than adding a second, parallel selection: the
// hover highlight, the tooltip and the click path in GuiContainer all keep
// reading lwjgl::Mouse exactly as before, so there is nothing new to draw and
// nothing to keep in sync. Pressing a direction warps the cursor to the centre
// of the next slot that way; pointing with the IR or the stick keeps working at
// the same time and simply wins the frame it is used in.
//
// It is a singleton for the same reason VirtualKeyboard is one: the pad poll
// (src/ps2/input/Ps2Input.cpp, src/wii/input/WiiPadState.cpp) has to reach it without
// knowing anything about which screen is open, and it has to be asked whether it
// is active before the poll decides what the D-pad means this frame.
class ContainerSlotNavigator
{
public:
	// Screen geometry the navigator cannot reach on its own: xSize/ySize are
	// protected in GuiContainer, and the cursor lives in physical framebuffer
	// pixels while the slots are laid out in the scaled GUI canvas.
	struct Layout
	{
		int_t guiLeft = 0;
		int_t guiTop = 0;
		int_t screenWidth = 0;
		int_t screenHeight = 0;
		int_t displayWidth = 0;
		int_t displayHeight = 0;
	};

	static ContainerSlotNavigator& instance();

	// Called by GuiContainer::drawScreen() every frame it is the open screen,
	// and by onGuiClosed()/~GuiContainer() when it goes away. Re-publishing the
	// layout is what keeps a resolution change -- which rebuilds the scaled
	// canvas under the same screen object -- from stepping against stale
	// coordinates.
	void notifyOpen(GuiContainer *screen, const Layout &layout);
	void notifyClosed(const GuiContainer *screen);
	bool isActive() const { return screen != nullptr; }

	// Per pad poll while active: read the D-pad and warp the cursor.
	void tick();

private:
	ContainerSlotNavigator() = default;

	const Slot *pickSlot(int_t originX, int_t originY, int_t dirX, int_t dirY) const;

	GuiContainer *screen = nullptr;
	Layout layout;
	int nextRepeatMs = 0;
};

#endif // PS2_PLATFORM || WII_PLATFORM
