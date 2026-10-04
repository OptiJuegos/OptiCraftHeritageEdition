// Keyboard_3ds.cpp — Nintendo 3DS implementation of lwjgl::Keyboard.
//
// There is no physical keyboard on this console, so every key event is
// synthesised upstream and pushed in through detail::pushKey / detail::pushChar:
//
//   src/3ds/input/DsInput.cpp  the button mapping (see its header table for
//                              the menu/gameplay split), including the
//                              START -> KEY_ESCAPE menu edge and the pad
//                              buttons that arrive as DS_KEY_* codes.
//   the virtual keyboard       the on-screen keyboard the game opens for chat
//   (PLATFORM_HAS_VIRTUAL_KEYBOARD) and sign/other text entry, which types
//                              characters through pushChar().
//
// The storage below is therefore identical to the PS2/Wii console versions and
// deliberately so: this file only records what it is given.
#ifdef CTR_PLATFORM

#include "lwjgl/Keyboard.h"
#include "lwjgl/KeyNames.h"
#include "3ds/input/DsPadKeyCodes.h"

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

// Simple bitfield: isKeyDown per LWJGL key code (max 256).
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
	// Pad buttons first: a bound DS_KEY_* code falls outside 0..KEY_MAX, so
	// lwjglKeyDisplayName would miss it and the Controls screen would print
	// "KEY 224" instead of "A". Same order as Keyboard_ps2.cpp.
	if (const char *padName = dsPadKeyName(key))
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

// Input is pushed from DsInput during Display::processMessages(), so there is
// nothing to pull here.
void poll() {}

bool isKeyDown(int_t key)
{
	if (key < 0 || key >= 256) return false;
	return detail::s_keyState[key];
}

} // namespace Keyboard
} // namespace lwjgl

#endif // CTR_PLATFORM
