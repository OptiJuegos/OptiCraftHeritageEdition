#include "TextureLavaFX.h"
#include "Config.h"
#include "java/Math.h"

#include <cmath>
#include "Block.h"
#include "MathHelper.h"

TextureLavaFX::TextureLavaFX() :
	TextureFX(0)
{
	iconIndex = Block::lavaMoving->blockIndexInTexture;
	for (int_t i = 0; i < 256; i++)
	{
		green[i] = 0.0f;
		red[i] = 0.0f;
		offset[i] = 0.0f;
		scale[i] = 0.0f;
	}
}

bool TextureLavaFX::isAnimationEnabled() const
{
	return Config::isAnimatedLava();
}

void TextureLavaFX::onTick()
{
	for (int_t i = 0; i < 16; i++)
	{
		for (int_t j = 0; j < 16; j++)
		{
			float f = 0.0f;
			int_t l = (int)(MathHelper::sin(((float)j * 3.1415927f * 2.0f) / 16.0f) * 1.2f);
			int_t i1 = (int)(MathHelper::sin(((float)i * 3.1415927f * 2.0f) / 16.0f) * 1.2f);
			for (int_t k1 = i - 1; k1 <= i + 1; k1++)
			{
				for (int_t i2 = j - 1; i2 <= j + 1; i2++)
				{
					int_t k2 = k1 + l & 0xf;
					int_t i3 = i2 + i1 & 0xf;
					f += green[k2 + i3 * 16];
				}
			}
			red[i + j * 16] = f / 10.0f + ((offset[(i + 0 & 0xf) + (j + 0 & 0xf) * 16]
				+ offset[(i + 1 & 0xf) + (j + 0 & 0xf) * 16]
				+ offset[(i + 1 & 0xf) + (j + 1 & 0xf) * 16]
				+ offset[(i + 0 & 0xf) + (j + 1 & 0xf) * 16]) / 4.0f) * 0.8f;
			offset[i + j * 16] += scale[i + j * 16] * 0.01f;
			if (offset[i + j * 16] < 0.0f)
			{
				offset[i + j * 16] = 0.0f;
			}
			scale[i + j * 16] -= 0.06f;
			if (Math::random() < 0.0050000000000000001)
			{
				scale[i + j * 16] = 1.5f;
			}
		}
	}

	std::swap(red, green);

	for (int_t k = 0; k < 256; k++)
	{
		float f1 = green[k] * 2.0f;
		if (f1 > 1.0f) f1 = 1.0f;
		if (f1 < 0.0f) f1 = 0.0f;
		float f2 = f1;
		int_t j1 = (int)(f2 * 100.0f + 155.0f);
		int_t l1 = (int)(f2 * f2 * 255.0f);
		int_t j2 = (int)(f2 * f2 * f2 * f2 * 128.0f);
		if (anaglyphEnabled)
		{
			int_t l2 = (j1 * 30 + l1 * 59 + j2 * 11) / 100;
			int_t j3 = (j1 * 30 + l1 * 70) / 100;
			int_t k3 = (j1 * 30 + j2 * 70) / 100;
			j1 = l2;
			l1 = j3;
			j2 = k3;
		}
		imageData[k * 4 + 0] = (byte_t)j1;
		imageData[k * 4 + 1] = (byte_t)l1;
		imageData[k * 4 + 2] = (byte_t)j2;
		imageData[k * 4 + 3] = (byte_t)0xff;
	}
}
