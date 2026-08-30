#include "TextureWatchFX.h"

#include <cmath>
#include "Minecraft.h"
#include "Item.h"
#include "World.h"
#include "WorldProvider.h"
#include "RenderEngine.h"
#include "java/Math.h"
#include <algorithm>
#include <vector>

TextureWatchFX::TextureWatchFX(Minecraft *minecraft) :
	TextureFX(Item::pocketSundial->getIconFromDamage(0)),
	mc(minecraft),
	compassFrame(0),
	compassAngle(0)
{
	tileImage = 1;
	std::fill(watchIconImageData, watchIconImageData + 256, 0);
	std::fill(dialImageData, dialImageData + 256, 0);
	if (mc != nullptr && mc->renderEngine != nullptr)
	{
		const std::vector<int_t> items = mc->renderEngine->readTextureImageData("/gui/items.png");
		if (items.size() >= 256u * 256u)
		{
			const int_t startX = (iconIndex % 16) * 16;
			const int_t startY = (iconIndex / 16) * 16;
			for (int_t y = 0; y < 16; ++y)
				for (int_t x = 0; x < 16; ++x)
					watchIconImageData[y * 16 + x] = items[(startY + y) * 256 + startX + x];
		}

		const std::vector<int_t> dial = mc->renderEngine->readTextureImageData("/misc/dial.png");
		if (dial.size() >= 256)
			std::copy(dial.begin(), dial.begin() + 256, dialImageData);
	}
}

void TextureWatchFX::onTick()
{
	double d = 0.0;
	if (mc->theWorld != nullptr && mc->thePlayer != nullptr)
	{
		float f = mc->theWorld->getCelestialAngle(1.0f);
		d = -f * 3.1415927f * 2.0f;
		if (mc->theWorld->worldProvider->isNether)
		{
			d = Math::random() * 3.1415927410125732 * 2.0;
		}
	}

	double d1;
	for (d1 = d - compassFrame; d1 < -3.1415926535897931; d1 += 6.2831853071795862) {}
	for (; d1 >= 3.1415926535897931; d1 -= 6.2831853071795862) {}
	if (d1 < -1.0) d1 = -1.0;
	if (d1 > 1.0) d1 = 1.0;

	compassAngle += d1 * 0.10000000000000001;
	compassAngle *= 0.80000000000000004;
	compassFrame += compassAngle;

	const float d2 = sinf(static_cast<float>(compassFrame));
	const float d3 = cosf(static_cast<float>(compassFrame));

	for (int_t i = 0; i < 256; i++)
	{
		int_t j = (watchIconImageData[i] >> 24) & 0xff;
		int_t k = (watchIconImageData[i] >> 16) & 0xff;
		int_t l = (watchIconImageData[i] >> 8) & 0xff;
		int_t i1 = (watchIconImageData[i] >> 0) & 0xff;
		if (k == i1 && l == 0 && i1 > 0)
		{
			const float d4 = -(static_cast<float>(i % 16) / 15.0f - 0.5f);
			const float d5 = static_cast<float>(i / 16) / 15.0f - 0.5f;
			int_t i2 = k;
			int_t j2 = static_cast<int_t>((d4 * d3 + d5 * d2 + 0.5f) * 16.0f);
			int_t k2 = static_cast<int_t>(((d5 * d3 - d4 * d2) + 0.5f) * 16.0f);
			int_t l2 = (j2 & 0xf) + (k2 & 0xf) * 16;
			j = (dialImageData[l2] >> 24) & 0xff;
			k = ((dialImageData[l2] >> 16) & 0xff) * i2 / 255;
			l = ((dialImageData[l2] >> 8) & 0xff) * i2 / 255;
			i1 = ((dialImageData[l2] >> 0) & 0xff) * i2 / 255;
		}
		if (anaglyphEnabled)
		{
			int_t j1 = (k * 30 + l * 59 + i1 * 11) / 100;
			int_t k1 = (k * 30 + l * 70) / 100;
			int_t l1 = (k * 30 + i1 * 70) / 100;
			k = j1;
			l = k1;
			i1 = l1;
		}
		imageData[i * 4 + 0] = (byte_t)k;
		imageData[i * 4 + 1] = (byte_t)l;
		imageData[i * 4 + 2] = (byte_t)i1;
		imageData[i * 4 + 3] = (byte_t)j;
	}
}
