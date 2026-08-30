#include "TexturePortalFX.h"
#include "Config.h"

#include <cmath>
#include "Block.h"
#include "MathHelper.h"
#include "BlockPortal.h"
#include "java/Random.h"

TexturePortalFX::TexturePortalFX() :
	TextureFX(0),
	portalTickCounter(0)
{
	iconIndex = Block::portal->blockIndexInTexture;
	Random random(100LL);
	for (int_t i = 0; i < 32; i++)
	{
		for (int_t j = 0; j < 16; j++)
		{
			for (int_t k = 0; k < 16; k++)
			{
				float f = 0.0f;
				for (int_t l = 0; l < 2; l++)
				{
					float f1 = l * 8;
					float f2 = l * 8;
					float f3 = (((float)j - f1) / 16.0f) * 2.0f;
					float f4 = (((float)k - f2) / 16.0f) * 2.0f;
					if (f3 < -1.0f) f3 += 2.0f;
					if (f3 >= 1.0f) f3 -= 2.0f;
					if (f4 < -1.0f) f4 += 2.0f;
					if (f4 >= 1.0f) f4 -= 2.0f;
					float f5 = f3 * f3 + f4 * f4;
					float f6 = (float)atan2(f4, f3) + ((((float)i / 32.0f) * 3.1415927f * 2.0f - f5 * 10.0f) + (float)(l * 2)) * (float)(l * 2 - 1);
					f6 = (MathHelper::sin(f6) + 1.0f) / 2.0f;
					f6 /= f5 + 1.0f;
					f += f6 * 0.5f;
				}

				f += random.nextFloat() * 0.1f;
				int_t i1 = (int)(f * 100.0f + 155.0f);
				int_t j1 = (int)(f * f * 200.0f + 55.0f);
				int_t k1 = (int)(f * f * f * f * 255.0f);
				int_t l1 = (int)(f * 100.0f + 155.0f);
				int_t i2 = k * 16 + j;
				portalTextureData[i][i2 * 4 + 0] = (byte_t)j1;
				portalTextureData[i][i2 * 4 + 1] = (byte_t)k1;
				portalTextureData[i][i2 * 4 + 2] = (byte_t)i1;
				portalTextureData[i][i2 * 4 + 3] = (byte_t)l1;
			}
		}
	}
}

bool TexturePortalFX::isAnimationEnabled() const
{
	return Config::isAnimatedPortal();
}

void TexturePortalFX::onTick()
{
	portalTickCounter++;
	byte_t *abyte0 = portalTextureData[portalTickCounter & 0x1f];
	for (int_t i = 0; i < 256; i++)
	{
		int_t j = abyte0[i * 4 + 0] & 0xff;
		int_t k = abyte0[i * 4 + 1] & 0xff;
		int_t l = abyte0[i * 4 + 2] & 0xff;
		int_t i1 = abyte0[i * 4 + 3] & 0xff;
		if (anaglyphEnabled)
		{
			int_t j1 = (j * 30 + k * 59 + l * 11) / 100;
			int_t k1 = (j * 30 + k * 70) / 100;
			int_t l1 = (j * 30 + l * 70) / 100;
			j = j1;
			k = k1;
			l = l1;
		}
		imageData[i * 4 + 0] = (byte_t)j;
		imageData[i * 4 + 1] = (byte_t)k;
		imageData[i * 4 + 2] = (byte_t)l;
		imageData[i * 4 + 3] = (byte_t)i1;
	}
}
