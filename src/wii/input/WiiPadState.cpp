#ifdef WII_PLATFORM

#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include "platform/Input.h"
#include "platform/PlatformKeyBindings.h"
#include "wii/input/WiiGameCubePad.h"
#include "wii/input/WiiInputDebug.h"
#include "wii/input/WiiPadInternal.h"
#include "wii/input/WiiPadPoll.h"
#include "wii/input/WiiPadState.h"
#include "wii/input/WiiPointer.h"
#include "wii/input/WiiRemote.h"

namespace
{
WiiTextInputSnapshot g_textInputSnapshot = {};
WiiStickSnapshot g_stickSnapshot = {};
u32 g_textInputLastHeld = 0;
bool g_alternativeControls = false;
float g_stickDeadzone = 0.20f;
u32 g_previousKeys = 0;
u32 g_previousMouse = 0;

// The first 8 entries (Forward..Drop) mirror GameSettings' live keyBindings so
// remapping an action in the Controls menu actually changes what the pad
// sends, instead of the pad always synthesizing the vanilla defaults. The
// rest (menu/debug/thirdperson/on-screen-nav) aren't player-rebindable.
int kVirtualKeyCodes[WiiPadInternal::VK_COUNT] = {
	lwjgl::Keyboard::KEY_W,
	lwjgl::Keyboard::KEY_S,
	lwjgl::Keyboard::KEY_A,
	lwjgl::Keyboard::KEY_D,
	lwjgl::Keyboard::KEY_SPACE,
	lwjgl::Keyboard::KEY_LSHIFT,
	lwjgl::Keyboard::KEY_E,
	lwjgl::Keyboard::KEY_Q,
	lwjgl::Keyboard::KEY_ESCAPE,
	lwjgl::Keyboard::KEY_F3,
	lwjgl::Keyboard::KEY_F5,
	lwjgl::Keyboard::KEY_UP,
	lwjgl::Keyboard::KEY_DOWN,
	lwjgl::Keyboard::KEY_LEFT,
	lwjgl::Keyboard::KEY_RIGHT,
};
unsigned g_bindingsVersion = 0;

void syncVirtualKeyCodes()
{
	const unsigned version = PlatformKeyBindings::version();
	if (version == g_bindingsVersion)
		return;

	// A rebind mid-hold must not leave the outgoing key stuck down, nor
	// silently eat the button because its VK bit looked unchanged to
	// flushKeysAndButtons (it XORs against the *bit*, not the key code).
	static const WiiPadInternal::VirtualKey kRemappable[] = {
		WiiPadInternal::VK_FORWARD, WiiPadInternal::VK_BACK,
		WiiPadInternal::VK_LEFT, WiiPadInternal::VK_RIGHT,
		WiiPadInternal::VK_JUMP, WiiPadInternal::VK_SNEAK,
		WiiPadInternal::VK_INVENTORY, WiiPadInternal::VK_DROP,
	};
	for (WiiPadInternal::VirtualKey vk : kRemappable)
	{
		const u32 bit = 1u << vk;
		if (g_previousKeys & bit)
		{
			lwjgl::Keyboard::detail::pushKey(kVirtualKeyCodes[vk], false);
			g_previousKeys &= ~bit;
		}
	}

	const PlatformKeyBindings::Snapshot &bindings = PlatformKeyBindings::get();
	kVirtualKeyCodes[WiiPadInternal::VK_FORWARD] = bindings.forward;
	kVirtualKeyCodes[WiiPadInternal::VK_BACK] = bindings.back;
	kVirtualKeyCodes[WiiPadInternal::VK_LEFT] = bindings.left;
	kVirtualKeyCodes[WiiPadInternal::VK_RIGHT] = bindings.right;
	kVirtualKeyCodes[WiiPadInternal::VK_JUMP] = bindings.jump;
	kVirtualKeyCodes[WiiPadInternal::VK_SNEAK] = bindings.sneak;
	kVirtualKeyCodes[WiiPadInternal::VK_INVENTORY] = bindings.inventory;
	kVirtualKeyCodes[WiiPadInternal::VK_DROP] = bindings.drop;
	g_bindingsVersion = version;
}

void updateTextInputSnapshot(u32 held)
{
	g_textInputSnapshot.held = held;
	g_textInputSnapshot.pressed = held & ~g_textInputLastHeld;
	g_textInputLastHeld = held;
}

void flushMovement(WiiPadInternal::FrameState& state)
{
	float x = state.moveX;
	float y = state.moveY;
	if (x > 1.0f) x = 1.0f; else if (x < -1.0f) x = -1.0f;
	if (y > 1.0f) y = 1.0f; else if (y < -1.0f) y = -1.0f;

	state.setKey(WiiPadInternal::VK_FORWARD, y > 0.0f);
	state.setKey(WiiPadInternal::VK_BACK, y < 0.0f);
	state.setKey(WiiPadInternal::VK_LEFT, x < 0.0f);
	state.setKey(WiiPadInternal::VK_RIGHT, x > 0.0f);
}

void flushKeysAndButtons(const WiiPadInternal::FrameState& state)
{
	const u32 changed = state.keys ^ g_previousKeys;
	if (changed)
	{
		for (int i = 0; i < WiiPadInternal::VK_COUNT; ++i)
		{
			const u32 bit = 1u << i;
			if (changed & bit)
				lwjgl::Keyboard::detail::pushKey(kVirtualKeyCodes[i], (state.keys & bit) != 0);
		}
	}
	g_previousKeys = state.keys;

	if ((state.mouse ^ g_previousMouse) & WiiPadInternal::VM_ATTACK)
		lwjgl::Mouse::detail::pushButton(0, (state.mouse & WiiPadInternal::VM_ATTACK) != 0,
		                                 WiiPointer::cursorX(), WiiPointer::cursorY());
	if ((state.mouse ^ g_previousMouse) & WiiPadInternal::VM_USE)
		lwjgl::Mouse::detail::pushButton(1, (state.mouse & WiiPadInternal::VM_USE) != 0,
		                                 WiiPointer::cursorX(), WiiPointer::cursorY());
	g_previousMouse = state.mouse;

	if (state.wheel != 0)
		lwjgl::Mouse::detail::pushWheel(state.wheel, WiiPointer::cursorX(), WiiPointer::cursorY());
}
} // namespace

void wiiSetAlternativeControls(bool enabled)
{
	g_alternativeControls = enabled;
}

void wiiSetStickDeadzone(float deadzone)
{
	if (deadzone < 0.05f)
		deadzone = 0.05f;
	if (deadzone > 0.35f)
		deadzone = 0.35f;
	g_stickDeadzone = deadzone;
}

WiiTextInputSnapshot wiiTextInputSnapshot()
{
	return g_textInputSnapshot;
}

WiiStickSnapshot wiiStickSnapshot()
{
	return g_stickSnapshot;
}

void wiiSetCursorPosition(int x, int y)
{
	WiiPointer::setCursorPosition(x, y);
}

void wiiPadPoll(std::uint32_t gameCubeConnected)
{
	syncVirtualKeyCodes();
	WiiPointer::beginFrame();
	wiiInputDebugReset();

	WiiPadInternal::FrameState state;
	state.alternativeControls = g_alternativeControls;
	state.stickDeadzone = g_stickDeadzone;

	WiiGameCubePad::poll(gameCubeConnected, state);
	WiiRemote::poll(state);

	g_textInputSnapshot = state.textInputSnapshot;
	g_stickSnapshot = state.stickSnapshot;
	updateTextInputSnapshot(state.textInputHeld);

	if (platformTextInputExclusive())
	{
		state.keys = 0;
		state.mouse = 0;
		state.wheel = 0;
	}

	flushMovement(state);
	WiiPointer::flush();
	flushKeysAndButtons(state);
}

#endif // WII_PLATFORM
