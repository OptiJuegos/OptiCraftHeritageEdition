#include "ContainerSlotNavigator.h"

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)

#include "platform/ConsoleInputClock.h"
#include "platform/Input.h"
#include "Container.h"
#include "GuiContainer.h"
#include "Slot.h"
#include "lwjgl/Mouse.h"

#if defined(PS2_PLATFORM)
#else
#endif

namespace
{
	// A slot is 16x16 and Slot::xDisplayPosition is its top-left corner, so this
	// is the offset to the middle of it -- the point the cursor is warped to and
	// the point every distance below is measured from. Aiming at the centre and
	// not the corner is what lets the hit test in GuiContainer accept the warp
	// even after the rounding of two coordinate-space conversions.
	const int_t SLOT_CENTER = 8;

	// How much a slot off the pressed axis is penalised, relative to one pixel
	// along it. The container screens are not one uniform grid -- the crafting
	// square, the result slot, the armour column and the 9-wide inventory each
	// have their own alignment -- so "nearest in that direction" on its own
	// would happily step diagonally into the next region. At 4, a slot has to be
	// four times closer off-axis to beat one that is straight ahead, which keeps
	// a row walk on its row while still allowing the jump from the crafting
	// square down into the inventory, where nothing is straight ahead.
	const int_t PERPENDICULAR_WEIGHT = 4;

	// Auto-repeat, the same shape and the same numbers as VirtualKeyboard: long
	// enough that a single press cannot double-step, short enough to cross the
	// 9-wide inventory row by holding the direction.
	const int REPEAT_DELAY_MS = 250;
	const int REPEAT_INTERVAL_MS = 90;

	int_t absInt(int_t v)
	{
		return v < 0 ? -v : v;
	}
}

ContainerSlotNavigator& ContainerSlotNavigator::instance()
{
	static ContainerSlotNavigator s_navigator;
	return s_navigator;
}

void ContainerSlotNavigator::notifyOpen(GuiContainer *guiContainer, const Layout &guiLayout)
{
	if (guiContainer == nullptr)
		return;

	platformSetContainerNavigationActive(true);
	if (screen != guiContainer)
	{
		screen = guiContainer;
		// Arm the repeat as if the direction had just been pressed. Opening the
		// inventory with a direction already held -- walking while pressing the
		// inventory button -- would otherwise repeat from the first frame.
		nextRepeatMs = consoleInputNowMs() + REPEAT_DELAY_MS;
	}
	layout = guiLayout;
}

void ContainerSlotNavigator::notifyClosed(const GuiContainer *guiContainer)
{
	if (screen == guiContainer)
	{
		screen = nullptr;
		platformSetContainerNavigationActive(false);
	}
}

// Nearest slot in the pressed direction, or the far edge if there is none.
//
// The wrap is not a convenience: without it the last press in a direction is
// simply ignored, which reads as the pad having stopped responding. Wrapping
// from the right of a row back to the left of it is what the hotbar already
// does everywhere else in the game.
const Slot *ContainerSlotNavigator::pickSlot(int_t originX, int_t originY, int_t dirX, int_t dirY) const
{
	const std::vector<Slot *> &slots = screen->inventorySlots->slots;

	const Slot *forward = nullptr;
	int_t forwardScore = 0;
	const Slot *wrapped = nullptr;
	int_t wrappedScore = 0;

	for (int_t i = 0; i < (int_t)slots.size(); i++)
	{
		const Slot *slot = slots[i];
		const int_t x = layout.guiLeft + slot->xDisplayPosition + SLOT_CENTER;
		const int_t y = layout.guiTop + slot->yDisplayPosition + SLOT_CENTER;

		// Distance along the pressed axis, and off it. Only one of dirX/dirY is
		// ever non-zero, so these are the two axes swapped rather than a
		// projection.
		const int_t along = (x - originX) * dirX + (y - originY) * dirY;
		const int_t perpendicular = (dirX != 0) ? absInt(y - originY) : absInt(x - originX);

		if (along > 0)
		{
			const int_t score = along + perpendicular * PERPENDICULAR_WEIGHT;
			if (forward == nullptr || score < forwardScore)
			{
				forward = slot;
				forwardScore = score;
			}
		}
		else
		{
			// Everything not ahead is a wrap candidate, scored by how far it sits
			// against the direction of travel: pressing right wraps to the
			// smallest x, pressing left to the largest. Same off-axis penalty, so
			// the wrap stays on the row the player was walking along.
			const int_t score = (x * dirX + y * dirY) + perpendicular * PERPENDICULAR_WEIGHT;
			if (wrapped == nullptr || score < wrappedScore)
			{
				wrapped = slot;
				wrappedScore = score;
			}
		}
	}

	return (forward != nullptr) ? forward : wrapped;
}

void ContainerSlotNavigator::tick()
{
	if (!isActive())
		return;
	if (layout.displayWidth <= 0 || layout.displayHeight <= 0 ||
		layout.screenWidth <= 0 || layout.screenHeight <= 0)
		return;
	if (screen->inventorySlots == nullptr || screen->inventorySlots->slots.empty())
		return;

	unsigned int held = 0;
	unsigned int pressed = 0;
	const int menuPad = platformMenuPad();
	const PlatformTextInputSnapshot pad = platformTextInputSnapshot(menuPad);
	if (!pad.connected)
		return;
	held = pad.held;
	pressed = pad.pressed;
	const unsigned int keyLeft = PLATFORM_TEXT_LEFT, keyRight = PLATFORM_TEXT_RIGHT;
	const unsigned int keyUp = PLATFORM_TEXT_UP, keyDown = PLATFORM_TEXT_DOWN;

	// One axis per step. A diagonal on a D-pad is mostly an accident of the
	// pivot, and stepping twice for it would overshoot.
	int_t dirX = 0;
	int_t dirY = 0;
	if (pressed & keyLeft)       dirX = -1;
	else if (pressed & keyRight) dirX = 1;
	else if (pressed & keyUp)    dirY = -1;
	else if (pressed & keyDown)  dirY = 1;

	const int now = consoleInputNowMs();
	const unsigned int heldDpad = held & (keyLeft | keyRight | keyUp | keyDown);
	if (dirX == 0 && dirY == 0 && heldDpad != 0 && now >= nextRepeatMs)
	{
		if (held & keyLeft)       dirX = -1;
		else if (held & keyRight) dirX = 1;
		else if (held & keyUp)    dirY = -1;
		else if (held & keyDown)  dirY = 1;
		nextRepeatMs = now + REPEAT_INTERVAL_MS;
	}
	else if (pressed & heldDpad)
	{
		nextRepeatMs = now + REPEAT_DELAY_MS;
	}

	if (dirX == 0 && dirY == 0)
		return;

	// Where the cursor is now, in the scaled canvas the slots are laid out in.
	// This is the conversion GuiScreen::handleMouseInput does on the way from
	// the (bottom-left origin) lwjgl coordinates into the GUI.
	const int_t cursorX = lwjgl::Mouse::getX() * layout.screenWidth / layout.displayWidth;
	const int_t cursorY = layout.screenHeight
		- (lwjgl::Mouse::getY() * layout.screenHeight / layout.displayHeight) - 1;

	// Step from the centre of the slot under the cursor, not from the cursor
	// itself: the pointer can rest anywhere inside a 16x16 slot, and measuring
	// from wherever it happens to be lets that offset accumulate across presses
	// until a step lands on the wrong row. Off the slots -- pointer parked in
	// the middle of the window, or the screen just opened -- the cursor is the
	// origin, so the first press snaps onto the grid.
	int_t originX = cursorX;
	int_t originY = cursorY;
	const Slot *hovered = screen->getSlotAtPosition(cursorX, cursorY);
	if (hovered != nullptr)
	{
		originX = layout.guiLeft + hovered->xDisplayPosition + SLOT_CENTER;
		originY = layout.guiTop + hovered->yDisplayPosition + SLOT_CENTER;
	}

	const Slot *target = pickSlot(originX, originY, dirX, dirY);
	if (target == nullptr)
		return;

	// Back to physical framebuffer pixels. The +1 over a doubled denominator
	// aims at the middle of the scaled pixel rather than its corner, so the
	// round trip back through handleMouseInput lands on the slot that was
	// chosen even at GUI scale 3.
	const int_t targetX = layout.guiLeft + target->xDisplayPosition + SLOT_CENTER;
	const int_t targetY = layout.guiTop + target->yDisplayPosition + SLOT_CENTER;
	const int cursorPixelX = (int)((targetX * 2 + 1) * layout.displayWidth / (layout.screenWidth * 2));
	const int cursorPixelY = (int)((targetY * 2 + 1) * layout.displayHeight / (layout.screenHeight * 2));

#if defined(PS2_PLATFORM)
	platformSetMenuCursor(cursorPixelX, cursorPixelY);
#else
	platformSetMenuCursor(cursorPixelX, cursorPixelY);
#endif
}

#endif // PS2_PLATFORM || WII_PLATFORM
