#include "CustomAnimationFrame.h"

CustomAnimationFrame::CustomAnimationFrame(int_t frameIndex, int_t frameDuration)
	: index(frameIndex)
	, duration(frameDuration > 0 ? frameDuration : 1)
	, counter(0)
{
}
