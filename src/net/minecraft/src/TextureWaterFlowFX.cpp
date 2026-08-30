#include "TextureWaterFlowFX.h"
#include "Config.h"
#include "java/Math.h"

#include <cmath>
#include "Block.h"

TextureWaterFlowFX::TextureWaterFlowFX() :
	TextureFX(0),
	iconIndex(0)
{
	TextureFX::iconIndex = Block::waterMoving->blockIndexInTexture + 1;
	tileSize = 2;
	for (int_t i = 0; i < 256; i++)
	{
		currentFrame[i] = 0.0f;
		frameIndex[i] = 0.0f;
		animationFrames[i] = 0.0f;
		tickCounter[i] = 0.0f;
	}
}

bool TextureWaterFlowFX::isAnimationEnabled() const
{
	return Config::isAnimatedWater();
}

void TextureWaterFlowFX::onTick()
{
	iconIndex++;
	for (int_t i = 0; i < 16; i++)
	{
		for (int_t k = 0; k < 16; k++)
		{
			float f = 0.0f;
			for (int_t j1 = k - 2; j1 <= k; j1++)
			{
				int_t k1 = i & 0xf;
				int_t i2 = j1 & 0xf;
				f += currentFrame[k1 + i2 * 16];
			}
			frameIndex[i + k * 16] = f / 3.2f + animationFrames[i + k * 16] * 0.8f;
		}
	}

	for (int_t j = 0; j < 16; j++)
	{
		for (int_t l = 0; l < 16; l++)
		{
			animationFrames[j + l * 16] += tickCounter[j + l * 16] * 0.05f;
			if (animationFrames[j + l * 16] < 0.0f)
			{
				animationFrames[j + l * 16] = 0.0f;
			}
			tickCounter[j + l * 16] -= 0.3f;
			if (Math::random() < 0.20000000000000001)
			{
				tickCounter[j + l * 16] = 0.5f;
			}
		}
	}

	std::swap(frameIndex, currentFrame);

	for (int_t i1 = 0; i1 < 256; i1++)
	{
		float f1 = currentFrame[i1 - iconIndex * 16 & 0xff];
		if (f1 > 1.0f) f1 = 1.0f;
		if (f1 < 0.0f) f1 = 0.0f;
		float f2 = f1 * f1;
		int_t l1 = (int)(32.0f + f2 * 32.0f);
		int_t j2 = (int)(50.0f + f2 * 64.0f);
		int_t k2 = 255;
		int_t l2 = (int)(146.0f + f2 * 50.0f);
		if (anaglyphEnabled)
		{
			int_t i3 = (l1 * 30 + j2 * 59 + k2 * 11) / 100;
			int_t j3 = (l1 * 30 + j2 * 70) / 100;
			int_t k3 = (l1 * 30 + k2 * 70) / 100;
			l1 = i3;
			j2 = j3;
			k2 = k3;
		}
		imageData[i1 * 4 + 0] = (byte_t)l1;
		imageData[i1 * 4 + 1] = (byte_t)j2;
		imageData[i1 * 4 + 2] = (byte_t)k2;
		imageData[i1 * 4 + 3] = (byte_t)l2;
	}
}
