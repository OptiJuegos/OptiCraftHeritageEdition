#pragma once

#include "java/Type.h"

// Shared vertical world layout used by Minecraft 1.2.5. Keeping these values in
// one header prevents the chunk, world, network and render paths from drifting
// back to the Beta 1.7.3 height independently.
namespace WorldHeight
{
	static constexpr int_t MIN_Y = 0;
	static constexpr int_t HEIGHT = 256;
	static constexpr int_t MAX_Y = HEIGHT - 1;
	static constexpr int_t SECTION_HEIGHT = 16;
	static constexpr int_t SECTION_COUNT = HEIGHT / SECTION_HEIGHT;
}
