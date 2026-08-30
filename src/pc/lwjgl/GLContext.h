#pragma once

#include <string>
#include <set>

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include "SDL.h"
#include "glad/glad.h"
#endif

namespace lwjgl
{
namespace GLContext
{

// Detail implementation
namespace detail
{

// GL capabilities
struct GLCapabilities
{
private:
	std::set<std::string> caps;

public:
	void add(const std::string &cap)
	{
		caps.insert(cap);
	}

	bool operator[](const std::string &cap) const
	{
		return caps.find(cap) != caps.end();
	}
};

// Context singletons (desktop only; consoles own the framebuffer directly).
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
SDL_Window *getWindow();
SDL_GLContext getGLContext();
#endif

}

// Context functions
// Must be called before instantiate(); requested sample count is clamped to SDL-supported values.
void setRequestedSamples(int samples);
int getRequestedSamples();
void instantiate();
const detail::GLCapabilities &getCapabilities();

}
}
