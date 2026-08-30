#ifndef PNGLOADER_H__
#define PNGLOADER_H__

#include <cstddef>

// Minimal PS2 texture payload used by the save prompt/startup UI.
// The original source referenced a TextureData type from another renderer tree;
// this port only needs RGBA8 pixels plus width/height.
#ifndef TEXF_UNCOMPRESSED_8888
#define TEXF_UNCOMPRESSED_8888 1
#endif

struct TextureData
{
    int w = 0;
    int h = 0;
    unsigned char* data = nullptr;
    int numBytes = 0;
    int format = TEXF_UNCOMPRESSED_8888;
    bool transparent = false;
};

/// Decode a PNG (from memory) into a TextureData.
/// Returns an empty TextureData on failure.
TextureData loadPngFromMemory(const unsigned char* data, size_t size);

#endif // PNGLOADER_H__
