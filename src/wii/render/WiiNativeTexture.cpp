#include "platform/Log.h"
#include "wii/render/WiiNativeTexture.h"

#if defined(WII_PLATFORM)
#include "wii/render/WiiNativeStateSnapshot.h"
#include "wii/render/WiiRenderTypes.h"
#include "wii/WiiEarlyInit.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <malloc.h>
#include <unordered_map>

namespace
{
    struct NativeTexture
    {
        GXTexObj object;
        void *pixels = nullptr;
        int width = 0;
        int height = 0;
        int maxLevel = 0;
        uint8_t format = GX_TF_RGB5A3;
        bool linear = false;
        bool clamp = false;
        bool mipmapLinear = true;
        int anisotropy = 1;
    };

    std::unordered_map<int, NativeTexture> s_textures;
    int s_currentTexture = -1;
    unsigned int s_nextTextureId = 1;

    uint16_t packRgb5A3(const unsigned char *rgba)
    {
        const uint8_t red = rgba[0];
        const uint8_t green = rgba[1];
        const uint8_t blue = rgba[2];
        const uint8_t alpha = rgba[3];

        if (alpha >= 237) {
            return static_cast<uint16_t>(0x8000 |
                ((red & 0xf8) << 7) |
                ((green & 0xf8) << 2) |
                ((blue & 0xf8) >> 3));
        }

        return static_cast<uint16_t>(
            ((alpha & 0xe0) << 7) |
            ((red & 0xf0) << 4) |
            (green & 0xf0) |
            ((blue & 0xf0) >> 4));
    }

    void uploadRgb5A3(const unsigned char *src, int width, int height, void *dst)
    {
        const int blocksWide = (width + 3) / 4;
        unsigned char *dstBytes = static_cast<unsigned char *>(dst);

        for (int blockY = 0; blockY < height; blockY += 4) {
            const int blockRow = blockY / 4;
            for (int blockX = 0; blockX < width; blockX += 4) {
                const int blockColumn = blockX / 4;
                unsigned char *block = dstBytes +
                    (blockRow * blocksWide + blockColumn) * 32;

                const int rows = height - blockY < 4 ? height - blockY : 4;
                const int columns = width - blockX < 4 ? width - blockX : 4;
                for (int y = 0; y < rows; ++y) {
                    uint16_t *dstRow = reinterpret_cast<uint16_t *>(block + y * 8);
                    const unsigned char *srcRow = src +
                        ((blockY + y) * width + blockX) * 4;
                    for (int x = 0; x < columns; ++x)
                        dstRow[x] = packRgb5A3(srcRow + x * 4);
                }
            }
        }
    }

    void uploadRgb5A3SubRect(const unsigned char *src, int srcWidth, int srcHeight,
                               int dstWidth, int dstHeight, int dstX, int dstY, void *dst)
    {
        unsigned char *dstBytes = static_cast<unsigned char *>(dst);
        const int blocksWide = (dstWidth + 3) / 4;
        for (int y = 0; y < srcHeight; ++y) {
            const int py = dstY + y;
            if (py < 0 || py >= dstHeight)
                continue;
            for (int x = 0; x < srcWidth; ++x) {
                const int px = dstX + x;
                if (px < 0 || px >= dstWidth)
                    continue;
                const int blockX = px >> 2;
                const int blockY = py >> 2;
                const int inBlockX = px & 3;
                const int inBlockY = py & 3;
                uint16_t *dstPixel = reinterpret_cast<uint16_t *>(
                    dstBytes + (blockY * blocksWide + blockX) * 32 +
                    (inBlockY * 4 + inBlockX) * 2);
                *dstPixel = packRgb5A3(src + (y * srcWidth + x) * 4);
            }
        }
    }

    uint32_t mipLevelOffset(const NativeTexture &texture, int level)
    {
        uint32_t offset = 0;
        int width = texture.width;
        int height = texture.height;
        for (int currentLevel = 0; currentLevel < level; ++currentLevel)
        {
            offset += GX_GetTexBufferSize((u16)width, (u16)height, texture.format, GX_FALSE, 0);
            width = width > 1 ? width >> 1 : 1;
            height = height > 1 ? height >> 1 : 1;
        }
        return offset;
    }

    void releaseTexture(NativeTexture &texture)
    {
        if (texture.pixels != nullptr)
            free(texture.pixels);
        texture.pixels = nullptr;
    }
}
#endif

void wii_native_texture_generate_names(int count, int *textures)
{
#if defined(WII_PLATFORM)
    if (count <= 0 || textures == nullptr)
        return;
    for (int i = 0; i < count; ++i)
        textures[i] = static_cast<int>(s_nextTextureId++);
#else
    (void)count;
    (void)textures;
#endif
}

void wii_native_texture_delete_names(int count, const int *textures)
{
#if defined(WII_PLATFORM)
    if (count <= 0 || textures == nullptr)
        return;
    for (int i = 0; i < count; ++i)
        wii_native_texture_remove(textures[i]);
#else
    (void)count;
    (void)textures;
#endif
}

bool wii_native_texture_refresh(int texture)
{
#if defined(WII_PLATFORM)
    return texture >= 0 && s_textures.find(texture) != s_textures.end();
#else
    (void)texture;
    return false;
#endif
}

bool wii_native_texture_begin_upload(int texture, int width, int height, int maxLevel,
                                     bool linear, bool clamp)
{
#if defined(WII_PLATFORM)
    if (texture < 0 || width <= 0 || height <= 0 || maxLevel < 0 ||
        width > static_cast<int>(std::numeric_limits<u16>::max()) ||
        height > static_cast<int>(std::numeric_limits<u16>::max()) ||
        maxLevel > static_cast<int>(std::numeric_limits<u8>::max()) ||
        maxLevel >= std::numeric_limits<int>::digits)
    {
#if MC_LOG_LEVEL >= 2
        MC_LOG_INFO("wii", "[WII][GX][TEXFAIL] begin id=%d size=%dx%d max=%d invalid\n",
               texture, width, height, maxLevel);
#endif
        return false;
    }

    const uint8_t format = GX_TF_RGB5A3;
    const uint32_t size = GX_GetTexBufferSize((u16)width, (u16)height, format,
                                              maxLevel > 0 ? GX_TRUE : GX_FALSE,
                                              (u8)maxLevel);
    void *pixels = memalign(32, size);
    if (pixels == nullptr)
    {
        MC_LOG_INFO("wii", "[WII][GX][TEXFAIL] alloc id=%d size=%dx%d bytes=%u max=%d\n",
               texture, width, height, (unsigned int)size, maxLevel);
        return false;
    }
    std::memset(pixels, 0, size);

    NativeTexture &native = s_textures[texture];
    releaseTexture(native);
    native.pixels = pixels;
    native.width = width;
    native.height = height;
    native.maxLevel = maxLevel;
    native.format = format;
    native.linear = linear;
    native.clamp = clamp;
    native.mipmapLinear = true;
    native.anisotropy = 1;

    const u8 wrap = clamp ? GX_CLAMP : GX_REPEAT;
    GX_InitTexObj(&native.object, pixels, (u16)width, (u16)height, format,
                  wrap, wrap, maxLevel > 0 ? GX_TRUE : GX_FALSE);
    const u8 minFilter = linear ? GX_LINEAR : (maxLevel > 0 ? GX_NEAR_MIP_LIN : GX_NEAR);
    const u8 magFilter = linear ? GX_LINEAR : GX_NEAR;
    GX_InitTexObjLOD(&native.object, minFilter, magFilter, 0.0f, (f32)maxLevel,
                     0.0f, GX_ENABLE, GX_ENABLE, GX_ANISO_1);
    DCFlushRange(pixels, size);
    GX_InvalidateTexAll();
#if MC_LOG_LEVEL >= 2
    {
        static unsigned int loggedTextures = 0;
        if (loggedTextures < 32)
        {
            MC_LOG_INFO("wii", "[WII][GX][TEX] n=%u id=%d size=%dx%d bytes=%u max=%d linear=%d clamp=%d ptr=%p\n",
                   loggedTextures, texture, width, height, (unsigned int)size, maxLevel,
                   linear ? 1 : 0, clamp ? 1 : 0, pixels);
            ++loggedTextures;
        }
    }
#endif
    return true;
#else
    (void)texture; (void)width; (void)height; (void)maxLevel; (void)linear; (void)clamp;
    return false;
#endif
}

bool wii_native_texture_set_parameters(int texture, bool linear, bool clamp)
{
#if defined(WII_PLATFORM)
    auto it = s_textures.find(texture);
    if (it == s_textures.end() || it->second.pixels == nullptr)
        return false;

    NativeTexture &native = it->second;
    native.linear = linear;
    native.clamp = clamp;
    const u8 wrap = clamp ? GX_CLAMP : GX_REPEAT;
    GX_InitTexObj(&native.object, native.pixels, (u16)native.width, (u16)native.height,
                  native.format, wrap, wrap, native.maxLevel > 0 ? GX_TRUE : GX_FALSE);
    const u8 minFilter = linear ? (native.maxLevel > 0 ? GX_LIN_MIP_LIN : GX_LINEAR)
                                : (native.maxLevel > 0 ? (native.mipmapLinear ? GX_NEAR_MIP_LIN : GX_NEAR_MIP_NEAR) : GX_NEAR);
    const u8 magFilter = linear ? GX_LINEAR : GX_NEAR;
    const u8 maxAniso = native.anisotropy >= 4 ? GX_ANISO_4 : native.anisotropy >= 2 ? GX_ANISO_2 : GX_ANISO_1;
    GX_InitTexObjLOD(&native.object, minFilter, magFilter, 0.0f, (f32)native.maxLevel,
                     0.0f, GX_ENABLE, GX_ENABLE, maxAniso);
    GX_InvalidateTexAll();
    return true;
#else
    (void)texture; (void)linear; (void)clamp;
    return false;
#endif
}

bool wii_native_texture_set_quality(int texture, bool mipmapLinear, int anisotropy)
{
#if defined(WII_PLATFORM)
    auto it = s_textures.find(texture);
    if (it == s_textures.end() || it->second.pixels == nullptr)
        return false;
    NativeTexture &native = it->second;
    native.mipmapLinear = mipmapLinear;
    native.anisotropy = anisotropy < 1 ? 1 : (anisotropy > 4 ? 4 : anisotropy);
    return wii_native_texture_set_parameters(texture, native.linear, native.clamp);
#else
    (void)texture; (void)mipmapLinear; (void)anisotropy;
    return false;
#endif
}

bool wii_native_texture_upload_level_rgba(int texture, int level, int width, int height,
                                          const unsigned char *pixels)
{
#if defined(WII_PLATFORM)
    auto it = s_textures.find(texture);
    if (it == s_textures.end() || pixels == nullptr || level < 0 || level > it->second.maxLevel)
    {
#if MC_LOG_LEVEL >= 2
        MC_LOG_INFO("wii", "[WII][GX][TEXFAIL] upload id=%d level=%d size=%dx%d resident=%d pixels=%p\n",
               texture, level, width, height, it != s_textures.end() ? 1 : 0, pixels);
#endif
        return false;
    }

    NativeTexture &native = it->second;
    if (native.pixels == nullptr)
        return false;

    const int expectedWidth = native.width >> level;
    const int expectedHeight = native.height >> level;
    if (width != expectedWidth || height != expectedHeight)
    {
#if MC_LOG_LEVEL >= 2
        MC_LOG_INFO("wii", "[WII][GX][TEXFAIL] dims id=%d level=%d got=%dx%d expected=%dx%d\n",
               texture, level, width, height, expectedWidth, expectedHeight);
#endif
        return false;
    }

    unsigned char *dst = static_cast<unsigned char *>(native.pixels) + mipLevelOffset(native, level);

    uploadRgb5A3(pixels, width, height, dst);
#if MC_LOG_LEVEL >= 2
    if (level == 0)
    {
        static unsigned int loggedLevel0 = 0;
        if (loggedLevel0 < 32)
        {
            unsigned int minAlpha = 255, maxAlpha = 0, passAlpha = 0;
            const size_t texels = (size_t)width * (size_t)height;
            for (size_t i = 0; i < texels; ++i)
            {
                const unsigned int a = pixels[i * 4u + 3u];
                if (a < minAlpha) minAlpha = a;
                if (a > maxAlpha) maxAlpha = a;
                if (a > 25u) ++passAlpha;
            }
            MC_LOG_INFO("wii", "[WII][GX][TEXDATA] n=%u id=%d alpha=%u..%u pass>25=%u/%u\n",
                   loggedLevel0, texture, minAlpha, maxAlpha, passAlpha,
                   (unsigned int)texels);
            ++loggedLevel0;
        }
    }
#endif
    const uint32_t levelSize = GX_GetTexBufferSize((u16)width, (u16)height, native.format,
                                                   GX_FALSE, 0);
    DCFlushRange(dst, levelSize);
    GX_InvalidateTexAll();
    return true;
#else
    (void)texture; (void)level; (void)width; (void)height; (void)pixels;
    return false;
#endif
}

bool wii_native_texture_upload_sub_rgba(int texture, int level, int x, int y,
                                        int width, int height, const unsigned char *pixels)
{
#if defined(WII_PLATFORM)
    auto it = s_textures.find(texture);
    if (it == s_textures.end() || pixels == nullptr || level < 0 ||
        level > it->second.maxLevel || width <= 0 || height <= 0)
        return false;

    NativeTexture &native = it->second;
    if (native.pixels == nullptr)
        return false;

    const int levelWidth = native.width >> level;
    const int levelHeight = native.height >> level;
    if (x < 0 || y < 0)
        return false;
    const std::int64_t endX = static_cast<std::int64_t>(x) + static_cast<std::int64_t>(width);
    const std::int64_t endY = static_cast<std::int64_t>(y) + static_cast<std::int64_t>(height);
    if (endX > static_cast<std::int64_t>(levelWidth) ||
        endY > static_cast<std::int64_t>(levelHeight))
        return false;

    unsigned char *dst = static_cast<unsigned char *>(native.pixels) + mipLevelOffset(native, level);

    uploadRgb5A3SubRect(pixels, width, height, levelWidth, levelHeight, x, y, dst);
    const uint32_t levelSize = GX_GetTexBufferSize((u16)levelWidth, (u16)levelHeight,
                                                   native.format, GX_FALSE, 0);
    DCFlushRange(dst, levelSize);
    GX_InvalidateTexAll();
    return true;
#else
    (void)texture; (void)level; (void)x; (void)y; (void)width; (void)height; (void)pixels;
    return false;
#endif
}

bool wii_native_texture_bind(int texture)
{
#if defined(WII_PLATFORM)
    if (texture < 0)
        return false;
    s_currentTexture = texture;
    wii_gx_native_set_bound_texture_id((unsigned int)texture);
    return true;
#else
    (void)texture;
    return false;
#endif
}

void wii_native_texture_remove(int texture)
{
#if defined(WII_PLATFORM)
    auto it = s_textures.find(texture);
    if (it != s_textures.end()) {
        releaseTexture(it->second);
        s_textures.erase(it);
    }
    if (s_currentTexture == texture)
        s_currentTexture = -1;
#else
    (void)texture;
#endif
}

void wii_native_texture_clear_all()
{
#if defined(WII_PLATFORM)
    for (auto &entry : s_textures)
        releaseTexture(entry.second);
    s_textures.clear();
    s_currentTexture = -1;
#endif
}

#if defined(WII_PLATFORM)
bool wii_native_texture_get_current(int *texture, GXTexObj *texobj)
{
    if (s_currentTexture < 0 || texobj == nullptr)
        return false;
    auto it = s_textures.find(s_currentTexture);
    if (it == s_textures.end())
        return false;
    if (texture != nullptr)
        *texture = s_currentTexture;
    *texobj = it->second.object;
    return true;
}


extern "C" int wii_native_texture_get_texobj_c(int texture, GXTexObj *texobj)
{
    if (texobj == nullptr)
        return 0;
    auto it = s_textures.find(texture);
    if (it == s_textures.end())
        return 0;
    *texobj = it->second.object;
    return 1;
}
#endif
