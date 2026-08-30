#pragma once

#include "TextureFX.h"

// net.minecraft.src.TextureWaterFX
class TextureWaterFX : public TextureFX
{
public:
	TextureWaterFX();

	void onTick() override;
	bool isAnimationEnabled() const override;

private:
	float contrast[256]; // frame1
	float brightness[256]; // frame2
	float alpha[256]; // frame3
	float blue[256]; // frame4
	int_t tickCounter;
};
