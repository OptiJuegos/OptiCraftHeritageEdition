// Keyboard_ps2.cpp — PS2 implementation of lwjgl::Keyboard.
// Event queue fed by pad-poll helpers via pushKey / pushChar.
#ifdef PS2_PLATFORM

#include "lwjgl/Keyboard.h"
#include "lwjgl/KeyNames.h"
#include "ps2/input/Ps2PadKeyCodes.h"

#include <queue>
#include <tamtypes.h>
#include <string.h>

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

static Event            s_current = {};
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
    if (const char *padName = ps2PadKeyName(key))
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

char_t  getEventCharacter() { return (char_t)detail::s_current.character; }
int_t   getEventKey()       { return detail::s_current.key; }
bool    getEventKeyState()  { return detail::s_current.down; }

void poll() {}

bool isKeyDown(int_t key)
{
    if (key < 0 || key >= 256) return false;
    return detail::s_keyState[key];
}

} // namespace Keyboard
} // namespace lwjgl

#endif // PS2_PLATFORM
