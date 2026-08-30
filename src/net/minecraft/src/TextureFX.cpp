#include "TextureFX.h"

#include "RenderEngine.h"
#include "platform/RenderAPI.h"

TextureFX::TextureFX(int_t iconIndex) :
	iconIndex(iconIndex),
	anaglyphEnabled(false),
	textureId(0),
	tileSize(1),
	tileImage(0)
{
	for (int_t i = 0; i < 1024; i++)
		imageData[i] = 0;
}

void TextureFX::onTick()
{
}

bool TextureFX::isAnimationEnabled() const
{
	return true;
}

void TextureFX::bindImage(RenderEngine *renderengine)
{
	if (tileImage == 0)
	{
		renderBindTexture(renderengine->getTexture("/terrain.png"));
	}
	else if (tileImage == 1)
	{
		renderBindTexture(renderengine->getTexture("/gui/items.png"));
	}
}
