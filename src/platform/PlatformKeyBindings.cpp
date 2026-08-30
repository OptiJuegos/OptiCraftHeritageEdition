#include "platform/PlatformKeyBindings.h"
#include "lwjgl/Keyboard.h"

namespace PlatformKeyBindings
{
namespace
{
	// Matches GameSettings::setDefaults() so a console mapper polling before
	// GameSettings has loaded still behaves like vanilla.
	Snapshot s_snapshot = {
		lwjgl::Keyboard::KEY_W, lwjgl::Keyboard::KEY_S,
		lwjgl::Keyboard::KEY_A, lwjgl::Keyboard::KEY_D,
		lwjgl::Keyboard::KEY_SPACE, lwjgl::Keyboard::KEY_LSHIFT,
		lwjgl::Keyboard::KEY_Q, lwjgl::Keyboard::KEY_E,
	};
	unsigned s_version = 0;
}

void set(const Snapshot &snapshot)
{
	s_snapshot = snapshot;
	++s_version;
}

const Snapshot &get()
{
	return s_snapshot;
}

unsigned version()
{
	return s_version;
}

}
