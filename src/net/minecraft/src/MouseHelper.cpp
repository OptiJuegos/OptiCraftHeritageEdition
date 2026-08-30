#include "MouseHelper.h"

#include "pc/lwjgl/Display.h"
#include "pc/lwjgl/Mouse.h"

#include <cstdlib>
#include <cstdio>

MouseHelper::MouseHelper(void *component)
	: deltaX(0)
	, deltaY(0)
	, field_1117_c(component)
	, cursor(nullptr)
	, field_1115_e(10)
{
}

void MouseHelper::grabMouseCursor()
{
	lwjgl::Mouse::setGrabbed(true);
	lwjgl::Mouse::clearDeltas();
	deltaX = 0;
	deltaY = 0;
	field_1115_e = 12;
}

void MouseHelper::ungrabMouseCursor()
{
	(void)field_1117_c;
	lwjgl::Mouse::setCursorPosition(lwjgl::Display::getWidth() / 2, lwjgl::Display::getHeight() / 2);
	lwjgl::Mouse::setGrabbed(false);
	lwjgl::Mouse::clearDeltas();
	deltaX = 0;
	deltaY = 0;
	field_1115_e = 0;
}

void MouseHelper::mouseXYChange()
{
	int_t rawX = lwjgl::Mouse::getDX();
	int_t rawY = lwjgl::Mouse::getDY();

	// When the mouse is grabbed SDL/LWJGL can report old warp/menu deltas for
	// a few frames.  If those are fed into Entity::turnEntity the camera appears
	// to spin by itself.  Drain them after focus changes.
	if (field_1115_e > 0)
	{
		--field_1115_e;
		deltaX = 0;
		deltaY = 0;
		return;
	}

	// Safety clamp: real user input should not need hundreds of pixels in one
	// frame on PS2.  This also protects PC from relative-mode warp spikes.
	const int_t maxDelta = 80;
	if (rawX >  maxDelta) rawX =  maxDelta;
	if (rawX < -maxDelta) rawX = -maxDelta;
	if (rawY >  maxDelta) rawY =  maxDelta;
	if (rawY < -maxDelta) rawY = -maxDelta;

	deltaX = rawX;
	deltaY = rawY;
}
