#pragma once

#include "TextureFX.h"

class Minecraft;

// net.minecraft.src.TextureWatchFX
class TextureWatchFX : public TextureFX
{
public:
	TextureWatchFX(Minecraft *minecraft);

	void onTick() override;

private:
	Minecraft *mc;
	int_t watchIconImageData[256];
	int_t dialImageData[256];
	double compassFrame; // angle
	double compassAngle; // angleDelta
};
