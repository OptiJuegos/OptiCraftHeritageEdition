#pragma once

#include "java/Type.h"

class GameSettings;

// net.minecraft.src.ScaledResolution
class ScaledResolution
{
public:
	ScaledResolution(GameSettings *gamesettings, int_t i, int_t j);

	int_t getScaledWidth();
	int_t getScaledHeight();

private:
	int_t scaledWidth;
	int_t scaledHeight;

public:
	double field_25121_a;
	double field_25120_b;
	int_t scaleFactor;
};
