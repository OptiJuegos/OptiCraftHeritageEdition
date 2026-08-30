#pragma once

#include "java/Type.h"

// One OptiFine custom-animation timeline entry.
struct CustomAnimationFrame
{
	CustomAnimationFrame(int_t frameIndex = 0, int_t frameDuration = 1);

	int_t index;
	int_t duration;
	int_t counter;
};
