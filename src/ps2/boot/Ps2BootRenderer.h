#pragma once

#ifdef PS2_PLATFORM

#include <tamtypes.h>

namespace Ps2BootRenderer
{

struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

enum class TextureFilter
{
    Nearest,
    Linear
};

enum class TextureAlphaMode
{
    SourceAlpha,
    SourceAlphaBlackTransparent
};

struct Texture
{
    void* implementation;
    int width;
    int height;

    Texture() : implementation(nullptr), width(0), height(0) {}
};

struct Font
{
    Texture texture;
    int cell;

    Font() : cell(0) {}
};

bool ready();
int width();
int height();

u32 checkpointVram();
void restoreVram(u32 pointer);

bool loadTextureAsset(const char* assetName,
                      Texture& texture,
                      TextureFilter filter,
                      TextureAlphaMode alphaMode);
void destroyTexture(Texture& texture);
bool textureValid(const Texture& texture);

bool loadFontAsset(const char* assetName, Font& font);
void destroyFont(Font& font);
bool fontValid(const Font& font);
float textWidth(const Font& font, const char* text, float scale);

void clear(Color color);
void drawRect(float x0, float y0, float x1, float y1, int z, Color color);
void drawTexture(const Texture& texture,
                 float x0, float y0, float u0, float v0,
                 float x1, float y1, float u1, float v1,
                 int z, Color color);
void drawText(const Font& font,
              float x, float y, int z,
              const char* text, float scale, Color color);
void drawTextCentered(const Font& font,
                      float centerX, float y, int z,
                      const char* text, float scale, Color color);

void setTextureRepeat(bool repeat);
void setAlphaBlend(bool enabled);
void present();
void resetAlpha();

} // namespace Ps2BootRenderer

#endif // PS2_PLATFORM
