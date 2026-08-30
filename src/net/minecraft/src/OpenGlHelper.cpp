#include "OpenGlHelper.h"

#include "platform/RenderAPI.h"

int OpenGlHelper::defaultTexUnit = 0x84C0;
int OpenGlHelper::lightmapTexUnit = 0x84C1;
bool OpenGlHelper::useMultitextureARB = false;

void OpenGlHelper::initializeTextures()
{
    defaultTexUnit = 0x84C0;
    lightmapTexUnit = 0x84C1;
    useMultitextureARB = false;
}

void OpenGlHelper::setActiveTexture(int textureUnit)
{
    renderSetActiveTextureUnit(textureUnit);
}

void OpenGlHelper::setClientActiveTexture(int textureUnit)
{
    renderSetClientActiveTextureUnit(textureUnit);
}

void OpenGlHelper::setLightmapTextureCoords(int textureUnit, float u, float v)
{
    renderSetMultiTextureCoord(textureUnit, u, v);
}
