#ifdef PS2_PLATFORM
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"
#include "ps2/render/PngLoader.h"
#include <cstring>
#include <limits>
#include <new>

TextureData loadPngFromMemory(const unsigned char* data, size_t size) {
    TextureData td;
    if (data == nullptr || size == 0 || size > static_cast<size_t>(std::numeric_limits<int>::max()))
        return td;

    int w, h, channels;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &channels, 4);
    if (!pixels) return td;

    if (w <= 0 || h <= 0 ||
        static_cast<size_t>(w) > std::numeric_limits<size_t>::max() / static_cast<size_t>(h)) {
        stbi_image_free(pixels);
        return td;
    }
    const size_t pixelCount = static_cast<size_t>(w) * static_cast<size_t>(h);
    if (pixelCount > static_cast<size_t>(std::numeric_limits<int>::max()) / 4u) {
        stbi_image_free(pixels);
        return td;
    }
    const int numBytes = static_cast<int>(pixelCount * 4u);
    unsigned char* ownedPixels = new (std::nothrow) unsigned char[static_cast<size_t>(numBytes)];
    if (!ownedPixels) {
        stbi_image_free(pixels);
        return td;
    }
    memcpy(ownedPixels, pixels, static_cast<size_t>(numBytes));
    stbi_image_free(pixels);

    td.w = w;
    td.h = h;
    td.data = ownedPixels;
    td.numBytes = numBytes;
    td.format = TEXF_UNCOMPRESSED_8888;
    td.transparent = true;
    return td;
}
#endif
