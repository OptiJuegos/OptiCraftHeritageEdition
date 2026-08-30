#ifdef PS2_PLATFORM

#include "ps2/boot/Ps2BootRenderer.h"

#include "ps2/render/PngLoader.h"
#include "ps2/render/Ps2Graphics.h"
#include "ps2/storage/assets/Ps2Assets.h"

#include <gsKit.h>
#include <gsPrimitive.h>

#include <cstdlib>
#include <cstring>
#include <malloc.h>

namespace
{

struct BootTextureImplementation
{
    GSTEXTURE texture;
    u16* cpuPixels;

    BootTextureImplementation() : cpuPixels(nullptr)
    {
        std::memset(&texture, 0, sizeof(texture));
    }
};

static u64 toGsColor(Ps2BootRenderer::Color color)
{
    return GS_SETREG_RGBAQ(color.r, color.g, color.b, color.a, 0);
}

static u16 rgbaToCt16(const unsigned char* pixel,
                      Ps2BootRenderer::TextureAlphaMode alphaMode)
{
    if (pixel[3] < 128)
        return 0;

    const u16 r = static_cast<u16>(pixel[0] >> 3);
    const u16 g = static_cast<u16>(pixel[1] >> 3);
    const u16 b = static_cast<u16>(pixel[2] >> 3);

    if (alphaMode == Ps2BootRenderer::TextureAlphaMode::SourceAlphaBlackTransparent &&
        r == 0 && g == 0 && b == 0)
    {
        return 0;
    }

    return static_cast<u16>(0x8000u | (b << 10) | (g << 5) | r);
}

static bool heapCanUpload(const GSTEXTURE& texture)
{
    const u32 required =
        gsKit_texture_size_ee(static_cast<int>(texture.Width),
                              static_cast<int>(texture.Height),
                              static_cast<int>(texture.PSM)) +
        1024;

    void* probe = memalign(64, required);
    if (!probe)
        return false;

    std::free(probe);
    return true;
}

static BootTextureImplementation* implementation(const Ps2BootRenderer::Texture& texture)
{
    return static_cast<BootTextureImplementation*>(texture.implementation);
}

} // namespace

namespace Ps2BootRenderer
{

bool ready()
{
    return Ps2Graphics::context() != nullptr;
}

int width()
{
    return Ps2Graphics::width();
}

int height()
{
    return Ps2Graphics::height();
}

u32 checkpointVram()
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    return graphics ? graphics->CurrentPointer : 0;
}

void restoreVram(u32 pointer)
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (graphics)
        graphics->CurrentPointer = pointer;
}

bool loadTextureAsset(const char* assetName,
                      Texture& texture,
                      TextureFilter filter,
                      TextureAlphaMode alphaMode)
{
    destroyTexture(texture);

    GSGLOBAL* graphics = Ps2Graphics::context();
    if (!graphics || !assetName)
        return false;

    unsigned int assetSize = 0;
    unsigned char* raw = Ps2Assets::loadAsset(assetName, &assetSize);
    if (!raw || assetSize == 0)
        return false;

    TextureData decoded = loadPngFromMemory(raw, static_cast<size_t>(assetSize));
    std::free(raw);
    if (!decoded.data)
        return false;

    BootTextureImplementation* impl = new BootTextureImplementation();
    const int pixelCount = decoded.w * decoded.h;
    impl->cpuPixels = static_cast<u16*>(memalign(64, static_cast<size_t>(pixelCount) * sizeof(u16)));
    if (!impl->cpuPixels)
    {
        delete[] decoded.data;
        delete impl;
        return false;
    }

    for (int i = 0; i < pixelCount; ++i)
        impl->cpuPixels[i] = rgbaToCt16(decoded.data + i * 4, alphaMode);
    delete[] decoded.data;

    impl->texture.Width = static_cast<u32>(decoded.w);
    impl->texture.Height = static_cast<u32>(decoded.h);
    impl->texture.PSM = GS_PSM_CT16;
    impl->texture.Filter = filter == TextureFilter::Linear ? GS_FILTER_LINEAR : GS_FILTER_NEAREST;
    impl->texture.ClutStorageMode = GS_CLUT_NONE;
    impl->texture.Clut = nullptr;
    impl->texture.VramClut = 0;
    impl->texture.ClutPSM = 0;
    impl->texture.Delayed = 0;
    gsKit_setup_tbw(&impl->texture);

    const u32 vramSize = gsKit_texture_size(decoded.w, decoded.h, GS_PSM_CT16);
    impl->texture.Vram = gsKit_vram_alloc(graphics, vramSize, GSKIT_ALLOC_USERBUFFER);
    if (impl->texture.Vram == GSKIT_ALLOC_ERROR || !heapCanUpload(impl->texture))
    {
        std::free(impl->cpuPixels);
        delete impl;
        return false;
    }

    impl->texture.Mem = reinterpret_cast<u32*>(impl->cpuPixels);
    gsKit_texture_upload(graphics, &impl->texture);
    impl->texture.Mem = nullptr;

    texture.implementation = impl;
    texture.width = decoded.w;
    texture.height = decoded.h;
    return true;
}

void destroyTexture(Texture& texture)
{
    BootTextureImplementation* impl = implementation(texture);
    if (impl)
    {
        if (impl->cpuPixels)
            std::free(impl->cpuPixels);
        delete impl;
    }

    texture.implementation = nullptr;
    texture.width = 0;
    texture.height = 0;
}

bool textureValid(const Texture& texture)
{
    return texture.implementation != nullptr;
}

bool loadFontAsset(const char* assetName, Font& font)
{
    destroyFont(font);

    if (!loadTextureAsset(assetName,
                          font.texture,
                          TextureFilter::Nearest,
                          TextureAlphaMode::SourceAlphaBlackTransparent))
    {
        return false;
    }

    font.cell = font.texture.width / 16;
    if (font.cell <= 0)
    {
        destroyFont(font);
        return false;
    }

    return true;
}

void destroyFont(Font& font)
{
    destroyTexture(font.texture);
    font.cell = 0;
}

bool fontValid(const Font& font)
{
    return font.cell > 0 && textureValid(font.texture);
}

float textWidth(const Font& font, const char* text, float scale)
{
    if (!fontValid(font) || !text)
        return 0.0f;

    return static_cast<float>(std::strlen(text)) * static_cast<float>(font.cell) * scale;
}

void clear(Color color)
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (graphics)
        gsKit_clear(graphics, toGsColor(color));
}

void drawRect(float x0, float y0, float x1, float y1, int z, Color color)
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (!graphics)
        return;

    gsKit_prim_sprite(graphics, x0, y0, x1, y1, z, toGsColor(color));
}

void drawTexture(const Texture& texture,
                 float x0, float y0, float u0, float v0,
                 float x1, float y1, float u1, float v1,
                 int z, Color color)
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    BootTextureImplementation* impl = implementation(texture);
    if (!graphics || !impl)
        return;

    gsKit_prim_sprite_texture(graphics,
                              &impl->texture,
                              x0, y0, u0, v0,
                              x1, y1, u1, v1,
                              z,
                              toGsColor(color));
}

void drawText(const Font& font,
              float x, float y, int z,
              const char* text, float scale, Color color)
{
    if (!fontValid(font) || !text)
        return;

    setAlphaBlend(true);

    const float cellSize = static_cast<float>(font.cell) * scale;
    while (*text)
    {
        const int code = static_cast<unsigned char>(*text++);
        if (code >= 0x20 && code <= 0x7E)
        {
            const float u0 = static_cast<float>((code % 16) * font.cell);
            const float v0 = static_cast<float>((code / 16) * font.cell);
            drawTexture(font.texture,
                        x, y, u0, v0,
                        x + cellSize, y + cellSize,
                        u0 + font.cell, v0 + font.cell,
                        z,
                        color);
        }
        x += cellSize;
    }
}

void drawTextCentered(const Font& font,
                      float centerX, float y, int z,
                      const char* text, float scale, Color color)
{
    drawText(font, centerX - textWidth(font, text, scale) * 0.5f, y, z, text, scale, color);
}

void setTextureRepeat(bool repeat)
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (graphics)
        gsKit_set_clamp(graphics, repeat ? GS_CMODE_REPEAT : GS_CMODE_CLAMP);
}

void setAlphaBlend(bool enabled)
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (!graphics)
        return;

    if (enabled)
    {
        graphics->PrimAlphaEnable = GS_SETTING_ON;
        graphics->PrimAlpha = GS_SETREG_ALPHA(0, 1, 0, 1, 0);
        gsKit_set_primalpha(graphics, graphics->PrimAlpha, 0);
    }
    else
    {
        graphics->PrimAlphaEnable = GS_SETTING_OFF;
    }
}

void present()
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (!graphics)
        return;

    gsKit_queue_exec(graphics);
    gsKit_sync_flip(graphics);
    gsKit_queue_reset(graphics->Os_Queue);
}

void resetAlpha()
{
    GSGLOBAL* graphics = Ps2Graphics::context();
    if (!graphics)
        return;

    graphics->PrimAlpha = GS_SETREG_ALPHA(0, 1, 0, 1, 0);
    gsKit_set_primalpha(graphics, graphics->PrimAlpha, 0);
    graphics->PrimAlphaEnable = GS_SETTING_OFF;
}

} // namespace Ps2BootRenderer

#endif // PS2_PLATFORM
