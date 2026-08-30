#pragma once

#include "TextureFX.h"

// net.minecraft.src.TextureFlamesFX
class TextureFlamesFX : public TextureFX
{
public:
	TextureFlamesFX(int_t i);

	void onTick() override;
	bool isAnimationEnabled() const override;

private:
	float currentFrame[320];
	float nextFrame[320];
};
