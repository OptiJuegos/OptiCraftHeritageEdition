// Keyboard_wii.cpp — Wii implementation of lwjgl::Keyboard.
//
// Same shape as the PS2 version: an event queue plus a key-state bitfield, fed
// from outside by pushKey / pushChar. Two producers feed it here:
//
//   1. src/wii/input/WiiPadState.cpp, which maps pad buttons onto key codes so the
//      existing GUI/gameplay key handling works from a controller.
//   2. libwiikeyboard, when a real USB keyboard is plugged in. That is why the
//      PS2's on-screen virtual keyboard has no equivalent here yet -- on Wii,
//      sign and chat text can come from actual keys.
#ifdef WII_PLATFORM

#include "lwjgl/Keyboard.h"
#include "lwjgl/KeyNames.h"
#include "wii/input/WiiPadKeyCodes.h"

#include <queue>
#include <string>

namespace lwjgl
{
namespace Keyboard
{

namespace detail
{

struct Event
{
	int  key;
	int  character;
	bool down;
};

static Event             s_current = {};
static std::queue<Event> s_queue;

// isKeyDown state per LWJGL key code. The enum tops out below 256 (KEY_SLEEP is
// 0xDF), so a flat array is both sufficient and cheaper than a set.
static bool s_keyState[256] = {};

void pushKey(int lwjglKey, bool down)
{
	if (lwjglKey >= 0 && lwjglKey < 256)
		s_keyState[lwjglKey] = down;
	s_queue.push({lwjglKey, 0, down});
}

void pushChar(int character)
{
	s_queue.push({KEY_NONE, character, true});
}

} // namespace detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

jstring getKeyName(int_t key)
{
	if (const char *padName = wiiPadKeyName(key))
		return padName;
	if (const char *name = lwjglKeyDisplayName(key))
		return name;
	return "KEY " + std::to_string(key);
}

static bool s_repeatEvents = false;

bool next()
{
	if (detail::s_queue.empty()) return false;
	detail::s_current = detail::s_queue.front();
	detail::s_queue.pop();
	return true;
}

void enableRepeatEvents(bool repeat) { s_repeatEvents = repeat; }
bool areRepeatEventsEnabled()        { return s_repeatEvents; }

char_t getEventCharacter() { return (char_t)detail::s_current.character; }
int_t  getEventKey()       { return detail::s_current.key; }
bool   getEventKeyState()  { return detail::s_current.down; }

// Input is pushed from the frame's pad/keyboard poll, so there is nothing to
// pull here.
void poll() {}

bool isKeyDown(int_t key)
{
	if (key < 0 || key >= 256) return false;
	return detail::s_keyState[key];
}

} // namespace Keyboard
} // namespace lwjgl

#endif // WII_PLATFORM
