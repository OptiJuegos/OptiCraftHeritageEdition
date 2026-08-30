#pragma once

#include <vector>
#include "java/Type.h"
#include "ImageBuffer.h"

class BufferedImage;

// net.minecraft.src.ImageBufferDownload
class ImageBufferDownload : public ImageBuffer
{
public:
    ImageBufferDownload();

    BufferedImage *parseUserSkin(BufferedImage *bufferedimage) override;

private:
    void setOpaque(int_t i, int_t j, int_t k, int_t l);
    void clearAlpha(int_t i, int_t j, int_t k, int_t l);
    bool hasTransparentPixel(int_t i, int_t j, int_t k, int_t l);

    std::vector<unsigned char> imageData;
    int_t imageWidth;
    int_t imageHeight;
};
