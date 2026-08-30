#ifdef WII_PLATFORM

#include "lwjgl/GLContext.h"

namespace lwjgl
{
namespace GLContext
{
namespace detail
{
static GLCapabilities &wiiCaps()
{
	static GLCapabilities caps;
	return caps;
}
} // namespace detail

void instantiate()
{
	// GX is not a GL context and has no extension string. The capability set is
	// left empty on purpose so the desktop-only paths keyed off it -- ARB
	// occlusion queries and VBOs -- stay disabled. GX has no occlusion query at
	// all, and vertex data is fed straight from main memory, so a VBO would be
	// an extra copy rather than an optimisation.
}

const detail::GLCapabilities &getCapabilities()
{
	return detail::wiiCaps();
}

} // namespace GLContext
} // namespace lwjgl

#endif // WII_PLATFORM
