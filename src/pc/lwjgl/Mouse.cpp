#include "lwjgl/Mouse.h"

#include <queue>

#include "lwjgl/GLContext.h"
#include "lwjgl/Display.h"

#include "external/SDLException.h"

#include "SDL_video.h"
#include "SDL_mouse.h"

namespace lwjgl
{
namespace Mouse
{

static int_t staging_dx = 0;
static int_t staging_dy = 0;
static int_t staging_dz = 0;

// SDL orders the first three buttons as left, middle, right (1, 2, 3),
// while LWJGL orders them as left, right, middle (0, 1, 2).
static int_t buttonSDLToLWJGL(Uint8 button)
{
	switch (button)
	{
		case SDL_BUTTON_LEFT:   return 0;
		case SDL_BUTTON_RIGHT:  return 1;
		case SDL_BUTTON_MIDDLE: return 2;
		default:                return button > 0 ? button - 1 : -1;
	}
}

static Uint8 buttonLWJGLToSDL(int_t button)
{
	switch (button)
	{
		case 0:  return SDL_BUTTON_LEFT;
		case 1:  return SDL_BUTTON_RIGHT;
		case 2:  return SDL_BUTTON_MIDDLE;
		default: return button >= 0 ? static_cast<Uint8>(button + 1) : 0;
	}
}

namespace detail
{

struct Event
{
	Sint8 button, down;
	Sint32 x, y;
	Sint32 xrel, yrel;
	Sint32 wheel;

	Event(Sint8 button = 0, Sint8 down = 0, Sint32 x = 0, Sint32 y = 0, Sint32 xrel = 0, Sint32 yrel = 0, Sint32 wheel = 0)
		: button(button), down(down), x(x), y(y), xrel(xrel), yrel(yrel), wheel(wheel)
	{ }
};

static Event event_current = {};
static std::queue<Event> event_queue;

void pushEvent(const SDL_Event &e)
{
	switch (e.type)
	{
		case SDL_MOUSEMOTION:
			staging_dx += e.motion.xrel;
			staging_dy -= e.motion.yrel;
			event_queue.emplace(-1, 0, e.motion.x, Display::getHeight() - e.motion.y - 1, e.motion.xrel, -e.motion.yrel, 0);
			break;
		case SDL_MOUSEWHEEL:
			event_queue.emplace(-1, 0, e.wheel.mouseX, Display::getHeight() - e.wheel.mouseY - 1, 0, 0, e.wheel.y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			event_queue.emplace(buttonSDLToLWJGL(e.button.button), 1, e.button.x, Display::getHeight() - e.button.y - 1, 0, 0, 0);
			break;
		case SDL_MOUSEBUTTONUP:
			event_queue.emplace(buttonSDLToLWJGL(e.button.button), 0, e.button.x, Display::getHeight() - e.button.y - 1, 0, 0, 0);
			break;
	}
}

}

void setCursorPosition(int_t x, int_t y)
{
	SDL_WarpMouseInWindow(GLContext::detail::getWindow(), x, y);
}

// Event handling
bool next()
{
	if (detail::event_queue.empty())
		return false;
	detail::event_current = detail::event_queue.front();
	detail::event_queue.pop();
	return true;
}

int_t getEventButton()
{
	return detail::event_current.button;
}
bool getEventButtonState()
{
	return detail::event_current.down != 0;
}

int_t getEventDX()
{
	return detail::event_current.xrel;
}
int_t getEventDY()
{
	return detail::event_current.yrel;
}

int_t getEventX()
{
	return detail::event_current.x;
}
int_t getEventY()
{
	return detail::event_current.y;
}

int_t getEventDWheel()
{
	return detail::event_current.wheel;
}

// State
int_t getX()
{
	int x;
	SDL_GetMouseState(&x, nullptr);
	return x;
}

int_t getY()
{
	int y;
	SDL_GetMouseState(nullptr, &y);
	return lwjgl::Display::getHeight() - y - 1;
}

int_t getDX()
{
	int_t result = staging_dx;
	staging_dx = 0;
	return result;
}

int_t getDY()
{
	int_t result = staging_dy;
	staging_dy = 0;
	return result;
}

int_t getDWheel()
{
	int_t result = staging_dz;
	staging_dz = 0;
	return result;
}

void clearDeltas()
{
	staging_dx = 0;
	staging_dy = 0;
	staging_dz = 0;

	// SDL may generate relative motion when switching to relative mode or
	// warping the cursor. Drain SDL's own relative accumulator too.
	int dx = 0, dy = 0;
	SDL_GetRelativeMouseState(&dx, &dy);
}

bool isButtonDown(int_t button)
{
	Uint8 sdlButton = buttonLWJGLToSDL(button);
	return sdlButton != 0 && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(sdlButton)) != 0;
}

bool isGrabbed()
{
	return SDL_GetRelativeMouseMode() == SDL_TRUE;
	// return SDL_GetWindowFlags(GLContext::detail::getWindow()) & SDL_WINDOW_MOUSE_CAPTURE;
	// return SDL_GetWindowGrab(GLContext::detail::getWindow()) == SDL_TRUE;
}

void setGrabbed(bool grabbed)
{
	staging_dx = 0;
	staging_dy = 0;

	if (SDL_ShowCursor(grabbed ? SDL_DISABLE : SDL_ENABLE) < 0)
		throw SDLException();
	SDL_SetRelativeMouseMode(grabbed ? SDL_TRUE : SDL_FALSE);
	clearDeltas();
	// SDL_CaptureMouse(grabbed ? SDL_TRUE : SDL_FALSE);
	// SDL_SetWindowMouseGrab(GLContext::detail::getWindow(), grabbed ? SDL_TRUE : SDL_FALSE);
}

}
}
