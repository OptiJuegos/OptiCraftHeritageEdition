#pragma once

#include <string>

#include "java/Type.h"

// net.minecraft.src.ScreenShotHelper
class ScreenShotHelper
{
public:
	static std::string saveScreenshot(const std::string &file, int_t i, int_t j);
};
