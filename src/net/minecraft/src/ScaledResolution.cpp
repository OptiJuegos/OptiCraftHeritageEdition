#include "ScaledResolution.h"

#include <cmath>
#include "java/Arithmetic.h"
#include "GameSettings.h"
#include "platform/ConsoleAspectRatio.h"
#include "platform/PlatformTuning.h"

ScaledResolution::ScaledResolution(GameSettings *gamesettings, int_t i, int_t j)
{
	const bool widescreen = gamesettings != nullptr && gamesettings->widescreen;
	scaledWidth = ConsoleAspectRatio::getLogicalWidth(i, j, widescreen);
	scaledHeight = ConsoleAspectRatio::getLogicalHeight(j);
	scaleFactor = 1;
#if PLATFORM_CONSOLE_LOW && PLATFORM_FORCE_GUI_SCALE > 0
	// The vanilla auto-scale loop below needs height/(scale+1) >= 240 to leave
	// scale 1; the 640x448 NTSC framebuffer gives 448/2 = 224 < 240, so the HUD
	// is otherwise stuck tiny. Force a fixed factor, clamped so the virtual
	// canvas never collapses below 1px in either axis.
	(void)gamesettings;
	scaleFactor = PLATFORM_FORCE_GUI_SCALE;
	while (scaleFactor > 1 && (scaledWidth / scaleFactor < 1 || scaledHeight / scaleFactor < 1))
		scaleFactor--;
#else
	int_t k = gamesettings->guiScale;
	if (k == 0)
		k = 1000;
	for (; scaleFactor < k && scaledWidth / (scaleFactor + 1) >= 320 && scaledHeight / (scaleFactor + 1) >= 240; scaleFactor++)
	{
	}
#endif
	field_25121_a = (double)scaledWidth / (double)scaleFactor;
	field_25120_b = (double)scaledHeight / (double)scaleFactor;
	scaledWidth = JavaArithmetic::doubleToInt(std::ceil(field_25121_a));
	scaledHeight = JavaArithmetic::doubleToInt(std::ceil(field_25120_b));
}

int_t ScaledResolution::getScaledWidth()
{
	return scaledWidth;
}

int_t ScaledResolution::getScaledHeight()
{
	return scaledHeight;
}
