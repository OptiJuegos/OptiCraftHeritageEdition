#pragma once

// net.minecraft.src.OpenGlHelper
class OpenGlHelper
{
public:
    static void initializeTextures();
    static void setActiveTexture(int textureUnit);
    static void setClientActiveTexture(int textureUnit);
    static void setLightmapTextureCoords(int textureUnit, float u, float v);

    static int defaultTexUnit;
    static int lightmapTexUnit;
    static bool useMultitextureARB;
};
