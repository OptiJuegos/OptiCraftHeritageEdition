#pragma once

#include "java/Type.h"

class RenderEngine;

// net.minecraft.src.TextureFX
class TextureFX
{
public:
	TextureFX(int_t iconIndex);
	virtual ~TextureFX() = default;

	virtual void onTick();
	virtual bool isAnimationEnabled() const;
	virtual void bindImage(RenderEngine *renderengine);

	byte_t imageData[1024];
	int_t iconIndex;
	bool anaglyphEnabled;
	int_t textureId;
	int_t tileSize;
	int_t tileImage;
};
