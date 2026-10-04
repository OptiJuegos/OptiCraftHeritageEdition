// Display companion for the 3DS: the console half of lwjgl::GLContext.
// Mirrors src/wii/lwjgl/GLContext_wii.cpp -- on CTR_PLATFORM the header does
// not declare the SDL window/context singletons (they are PC-only behind the
// guard in GLContext.h), so only the context functions below exist here.
//
// instantiate() is a deliberate no-op: the citro3d context comes up with the
// display instead (Display_3ds.cpp::create() calls ds::init(), the same shape
// as the Wii bringing wiigl_init() up in Display_wii.cpp). pc/Main.cpp is the
// only port that calls this at all. There is no GL extension string to record
// afterwards either -- the capability set stays empty on purpose so the
// desktop-only paths keyed off it (ARB occlusion queries, VBOs) stay
// disabled; vertex data is staged through linear-heap arenas by DsRender.cpp,
// so a VBO would be an extra copy rather than an optimisation.
#ifdef CTR_PLATFORM

#include "lwjgl/GLContext.h"

namespace lwjgl
{
namespace GLContext
{
namespace detail
{
namespace
{
GLCapabilities &dsCaps()
{
	static GLCapabilities caps;
	return caps;
}
} // namespace
} // namespace detail

void setRequestedSamples(int)
{
	// No GL context, so no pre-context MSAA knob to record: the citro3d
	// renderer would use the PICA200's fragment-buffer MSAA, not a sample
	// count handed over before instantiate(). Defined here rather than left
	// to the desktop GLContext.cpp (which the 3DS target does not compile)
	// so any console-side caller still links.
}

int getRequestedSamples()
{
	return 0;
}

void instantiate()
{
	// Nothing to do here -- see the file header. ds::init() already ran in
	// Display::create() by the time anything could reach the renderer.
}

const detail::GLCapabilities &getCapabilities()
{
	return detail::dsCaps();
}

} // namespace GLContext
} // namespace lwjgl

#endif // CTR_PLATFORM
