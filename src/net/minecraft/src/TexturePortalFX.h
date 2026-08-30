#pragma once

#include "TextureFX.h"

// net.minecraft.src.TexturePortalFX
class TexturePortalFX : public TextureFX
{
public:
	TexturePortalFX();

	void onTick() override;
	bool isAnimationEnabled() const override;

private:
	int_t portalTickCounter;
	byte_t portalTextureData[32][1024];
};
