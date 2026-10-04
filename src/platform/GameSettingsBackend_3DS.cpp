#include "platform/GameSettingsBackend.h"

#include <algorithm>
#include "3ds/input/DsPadKeyCodes.h"
#include "lwjgl/Keyboard.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/KeyBinding.h"
#include "platform/PlatformTuning.h"

namespace
{
// Same helper the PS2 uses: an options.txt written by another build carries
// real keyboard codes, which no 3DS button will ever emit, so swap them for
// the pad button that now owns the action. A code already at or above
// KEY_MAX is a DS_KEY_* and is left alone -- that is exactly what makes
// running this twice (initialize, then finalize-load) safe.
void migrateKey(KeyBinding* binding, int_t fallback)
{
	if (binding->keyCode < lwjgl::Keyboard::KEY_MAX)
		binding->keyCode = fallback;
}

// The decided 3DS layout (see the table in src/3ds/input/DsInput.cpp). The
// circle pad carries ALL movement: DsInput no longer emits DS_KEY_DPAD_* at
// all (in gameplay the D-pad steps the hotbar and L/R click instead), so the
// four digital movement binds below are dead -- they stay assigned only so an
// options.txt from an older build and the Controls screen keep finding a sane
// value to show, until the mapping UI learns to report "no button".
//
// Attack and Use are deliberately NOT touched: they stay on the mouse
// pseudo-keys (-100 / -99), because DsInput emits them as mouse button 0/1
// from X/R and B/L and from the touch tap, which is the plumbing clickMouse()
// and the right-click path already read. Only keyboard-shaped actions are
// rebound.
void applyDefaultBindings(GameSettings& settings)
{
	settings.keyBindForward->keyCode = DS_KEY_DPAD_UP;
	settings.keyBindLeft->keyCode = DS_KEY_DPAD_LEFT;
	settings.keyBindBack->keyCode = DS_KEY_DPAD_DOWN;
	settings.keyBindRight->keyCode = DS_KEY_DPAD_RIGHT;
	settings.keyBindJump->keyCode = DS_KEY_A;
	settings.keyBindInventory->keyCode = DS_KEY_Y;
	settings.keyBindSneak->keyCode = DS_KEY_SELECT;
	// keyBindDrop keeps its keyboard default: no button in the decided layout
	// is spare to carry a drop, so nothing on this console emits one and the
	// prompt reports no label for it rather than a button that does nothing.
	// See LegacyControlPromptBackend_3DS.cpp.
}
} // namespace

void platformGameSettingsInitialize(GameSettings& settings) { applyDefaultBindings(settings); }
void platformGameSettingsResetControlBindings(GameSettings& settings) { applyDefaultBindings(settings); }

// The legacy-crafting toggle rebinds the craft action on the platforms that
// have a button to give it to (PC: KEY_C, PS2: SQUARE -- see their backends).
// The decided 3DS layout has none: Y owns inventory and the 2x2 screen is
// reached through it (GuiInventory::keyTyped / EntityPlayerSP::
// displayWorkbenchGUI), so there is nothing to rebind -- a no-op, as on Wii.
void platformGameSettingsApplyLegacyCrafting(GameSettings&) {}

// The tuning half mirrors the PS2 file: both are fixed-grid bounded-world
// consoles, so the render-distance surface is whatever the PLATFORM_* tuning
// table implies -- a single default distance (Cycle and Clamp are pinned to
// it), the fine distance clamped to the visible chunk radius, and no
// post-cycle writeback.
int_t platformGameSettingsDefaultChunkUpdates() { return static_cast<int_t>(PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME); }
int_t platformGameSettingsDefaultConnectedTextures() { return 3; }
int_t platformGameSettingsCycleRenderDistance(int_t, int_t) { return PLATFORM_DEFAULT_RENDER_DISTANCE; }
int_t platformGameSettingsClampRenderDistance(int_t) { return PLATFORM_DEFAULT_RENDER_DISTANCE; }
int_t platformGameSettingsClampFineRenderDistance(int_t value)
{
	return value < 32 ? 32 : (value > PLATFORM_VISIBLE_CHUNK_RADIUS * 16 ? PLATFORM_VISIBLE_CHUNK_RADIUS * 16 : value);
}
void platformGameSettingsUpdateRenderDistanceFromFine(int_t, int_t&) {}
bool platformGameSettingsAnaglyphValue(bool, bool requested) { return requested; }
bool platformGameSettingsLoadOption(GameSettings&, const std::string&, const std::string&) { return false; }

void platformGameSettingsFinalizeLoad(GameSettings& settings)
{
	// Same floor the PS2 applies: an options.txt written by another build can
	// ask for fewer renderer updates per frame than this bounded-world profile
	// needs to make progress.
	settings.ofChunkUpdates = std::max(settings.ofChunkUpdates, static_cast<int_t>(PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME));

	// Anything still carrying a keyboard code is a file written before this
	// layout existed (or by another platform) -- give the action back to the
	// button that can actually emit it. DS_KEY_* codes are >= KEY_MAX, so
	// applyDefaultBindings()' output passes through untouched.
	migrateKey(settings.keyBindForward, DS_KEY_DPAD_UP);
	migrateKey(settings.keyBindLeft, DS_KEY_DPAD_LEFT);
	migrateKey(settings.keyBindBack, DS_KEY_DPAD_DOWN);
	migrateKey(settings.keyBindRight, DS_KEY_DPAD_RIGHT);
	migrateKey(settings.keyBindJump, DS_KEY_A);
	migrateKey(settings.keyBindInventory, DS_KEY_Y);
	migrateKey(settings.keyBindSneak, DS_KEY_SELECT);

	// keyBindings[].keyCode was assigned in place (both here and by the
	// key_* reader above), so rebuild KeyBinding's lookup table now rather
	// than depending on whichever build ran last having produced these exact
	// codes: KeyBinding::setKeyBindState() resolves through that table, and a
	// hash still holding a foreign file's keyboard code would make the bound
	// pad button silently do nothing.
	KeyBinding::resetKeyBindingArrayAndHash();
}

void platformGameSettingsSyncControllerBindings(const GameSettings&) {}
void platformGameSettingsAddKnownKeys(std::unordered_set<std::string>&) {}
void platformGameSettingsWriteOptions(const GameSettings&, std::ostream&) {}
