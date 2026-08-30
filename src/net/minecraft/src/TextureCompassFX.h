#pragma once

#include "TextureFX.h"
#include <vector>

class Minecraft;

// net.minecraft.src.TextureCompassFX
class TextureCompassFX : public TextureFX
{
public:
	TextureCompassFX(Minecraft *minecraft);

	void onTick() override;

private:
	Minecraft *mc;
	int_t compassIconImageData[256];
	double watchAngle; // angle
	double watchFrame; // angleDelta
};
