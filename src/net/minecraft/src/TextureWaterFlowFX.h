#pragma once

#include "TextureFX.h"

// net.minecraft.src.TextureWaterFlowFX
class TextureWaterFlowFX : public TextureFX
{
public:
	TextureWaterFlowFX();

	void onTick() override;
	bool isAnimationEnabled() const override;

private:
	float currentFrame[256]; // frame1
	float frameIndex[256]; // frame2
	float animationFrames[256]; // frame3
	float tickCounter[256]; // frame4
	int_t iconIndex;      // tickCounter
};
