#include "TextureWaterFX.h"
#include "Config.h"
#include "java/Math.h"

#include <cmath>
#include "Block.h"

TextureWaterFX::TextureWaterFX() :
	TextureFX(0),
	tickCounter(0)
{
	iconIndex = Block::waterMoving->blockIndexInTexture;
	for (int_t i = 0; i < 256; i++)
	{
		contrast[i] = 0.0f;
		brightness[i] = 0.0f;
		alpha[i] = 0.0f;
		blue[i] = 0.0f;
	}
}

bool TextureWaterFX::isAnimationEnabled() const
{
	return Config::isAnimatedWater();
}

void TextureWaterFX::onTick()
{
	tickCounter++;
	for (int_t i = 0; i < 16; i++)
	{
		for (int_t k = 0; k < 16; k++)
		{
			float f = 0.0f;
			for (int_t j1 = i - 1; j1 <= i + 1; j1++)
			{
				int_t k1 = j1 & 0xf;
				int_t i2 = k & 0xf;
				f += contrast[k1 + i2 * 16];
			}
			brightness[i + k * 16] = f / 3.3f + alpha[i + k * 16] * 0.8f;
		}
	}

	for (int_t j = 0; j < 16; j++)
	{
		for (int_t l = 0; l < 16; l++)
		{
			alpha[j + l * 16] += blue[j + l * 16] * 0.05f;
			if (alpha[j + l * 16] < 0.0f)
			{
				alpha[j + l * 16] = 0.0f;
			}
			blue[j + l * 16] -= 0.1f;
			if (Math::random() < 0.050000000000000003)
			{
				blue[j + l * 16] = 0.5f;
			}
		}
	}

	std::swap(brightness, contrast);

	for (int_t i1 = 0; i1 < 256; i1++)
	{
		float f1 = contrast[i1];
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
