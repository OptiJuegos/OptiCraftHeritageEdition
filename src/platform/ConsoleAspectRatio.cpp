#include "platform/ConsoleAspectRatio.h"

#include "platform/PlatformConfig.h"

namespace
{
constexpr int_t kWidescreenWidthUnits = 16;
constexpr int_t kWidescreenHeightUnits = 9;

int_t clampDimension(int_t value)
{
	return value > 0 ? value : 1;
}

bool useWidescreen(bool requested)
{
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	return requested;
#else
	(void)requested;
	return false;
#endif
}
}

namespace ConsoleAspectRatio
{
bool getDefaultWidescreen()
{
#if defined(WII_PLATFORM) && defined(WII_WIDESCREEN)
	return true;
#else
	return false;
#endif
}

int_t getLogicalWidth(int_t physicalWidth, int_t physicalHeight, bool widescreen)
{
	physicalWidth = clampDimension(physicalWidth);
	physicalHeight = clampDimension(physicalHeight);
	if (!useWidescreen(widescreen))
		return physicalWidth;

	// Console widescreen is anamorphic: only the logical canvas changes. The
	// backend keeps its native framebuffer and viewport dimensions.
	const long_t numerator = static_cast<long_t>(physicalHeight) * kWidescreenWidthUnits;
	return static_cast<int_t>((numerator + kWidescreenHeightUnits / 2) /
	                          kWidescreenHeightUnits);
}

int_t getLogicalHeight(int_t physicalHeight)
{
	return clampDimension(physicalHeight);
}

double getProjectionAspect(int_t physicalWidth, int_t physicalHeight, bool widescreen)
{
	physicalWidth = clampDimension(physicalWidth);
	physicalHeight = clampDimension(physicalHeight);
	if (useWidescreen(widescreen))
		return static_cast<double>(kWidescreenWidthUnits) / kWidescreenHeightUnits;

	return static_cast<double>(physicalWidth) / physicalHeight;
}
}
