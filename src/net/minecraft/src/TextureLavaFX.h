#pragma once

#include "TextureFX.h"

// net.minecraft.src.TextureLavaFX
class TextureLavaFX : public TextureFX
{
public:
	TextureLavaFX();

	void onTick() override;
	bool isAnimationEnabled() const override;

private:
	float green[256]; // frame1
	float red[256]; // frame2
	float offset[256]; // frame3
	float scale[256]; // frame4
};
