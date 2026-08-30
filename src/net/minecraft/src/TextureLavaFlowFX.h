#pragma once

#include "TextureFX.h"

// net.minecraft.src.TextureLavaFlowFX
class TextureLavaFlowFX : public TextureFX
{
public:
	TextureLavaFlowFX();

	void onTick() override;
	bool isAnimationEnabled() const override;

private:
	float textureHeight[256]; // frame1
	float textureWidth[256]; // frame2
	float frameCounter[256]; // frame3
	float frameTime[256]; // frame4
	int_t maxFrames;      // tickCounter
};
