#include "ScaledResolution.h"

#include <cmath>
#include "java/Arithmetic.h"
#include "GameSettings.h"
#include "client/Minecraft.h"
#include "platform/ConsoleAspectRatio.h"
#include "platform/PlatformTuning.h"

ScaledResolution::ScaledResolution(GameSettings *gamesettings, int_t i, int_t j)
{
	const bool widescreen = gamesettings != nullptr && gamesettings->widescreen;
	Minecraft *mc = Minecraft::getMinecraft();
	const bool isSplit = (mc != nullptr && mc->isSplitScreenActive() && j <= 256);

	if (isSplit && widescreen)
	{
		// In vertical split-screen, physical height (j) is half of the TV display height (j * 2).
		// Anamorphic widescreen logical width must be computed from the full TV height,
		// otherwise the logical width is halved, squashing the HUD horizontally.
		scaledWidth = ConsoleAspectRatio::getLogicalWidth(i, j * 2, widescreen);
	}
	else
	{
		scaledWidth = ConsoleAspectRatio::getLogicalWidth(i, j, widescreen);
	}
	scaledHeight = ConsoleAspectRatio::getLogicalHeight(j);
	scaleFactor = 1;
	exactScaleFactor = 1.0;
	if (gamesettings != nullptr && gamesettings->legacyUI && PLATFORM_LEGACY_GUI_SCALE > 0.0)
	{
		if (isSplit)
		{
			// Adapt HUD scale in split-screen (70% of legacy scale = 1.4) so health, hunger,
			// hotbar, and button tooltips leave ample field of view on both 4:3 and 16:9 displays.
			exactScaleFactor = PLATFORM_LEGACY_GUI_SCALE * 0.70;
		}
		else
		{
			exactScaleFactor = PLATFORM_LEGACY_GUI_SCALE;
		}
	}
	else
	{
#if PLATFORM_CONSOLE_LOW && PLATFORM_FORCE_GUI_SCALE > 0
		scaleFactor = PLATFORM_FORCE_GUI_SCALE;
		while (scaleFactor > 1 && (scaledWidth / scaleFactor < 1 || scaledHeight / scaleFactor < 1))
			scaleFactor--;
		exactScaleFactor = static_cast<double>(scaleFactor);
#else
		int_t k = gamesettings->guiScale;
		if (k == 0)
			k = 1000;
		for (; scaleFactor < k && scaledWidth / (scaleFactor + 1) >= 320 && scaledHeight / (scaleFactor + 1) >= 240; scaleFactor++)
		{
		}
		exactScaleFactor = static_cast<double>(scaleFactor);
#endif
	}
	field_25121_a = (double)scaledWidth / exactScaleFactor;
	field_25120_b = (double)scaledHeight / exactScaleFactor;
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


double ScaledResolution::getScaleFactorExact() const
{
	return exactScaleFactor;
}
