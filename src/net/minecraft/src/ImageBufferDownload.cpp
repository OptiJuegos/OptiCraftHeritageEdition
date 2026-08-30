#include "ImageBufferDownload.h"

#include <algorithm>
#include <cstddef>
#include "java/BufferedImage.h"

ImageBufferDownload::ImageBufferDownload()
    : imageWidth(0), imageHeight(0)
{
}

// func_884_b - make all pixels in region opaque (set alpha = 0xFF)
void ImageBufferDownload::setOpaque(int_t i, int_t j, int_t k, int_t l)
{
    for (int_t i1 = i; i1 < k; i1++)
    {
        for (int_t j1 = j; j1 < l; j1++)
        {
            // RGBA: alpha is byte index 3
            imageData[(i1 + j1 * imageWidth) * 4 + 3] = 0xFF;
        }
    }
}

// func_885_a - clear alpha channel in region (make transparent)
void ImageBufferDownload::clearAlpha(int_t i, int_t j, int_t k, int_t l)
{
    if (hasTransparentPixel(i, j, k, l))
        return;

    for (int_t i1 = i; i1 < k; i1++)
    {
        for (int_t j1 = j; j1 < l; j1++)
        {
            imageData[(i1 + j1 * imageWidth) * 4 + 3] = 0;
        }
    }
}

// func_886_c - check if any pixel in region is semi-transparent (alpha < 128)
bool ImageBufferDownload::hasTransparentPixel(int_t i, int_t j, int_t k, int_t l)
{
    for (int_t i1 = i; i1 < k; i1++)
    {
        for (int_t j1 = j; j1 < l; j1++)
        {
            unsigned char alpha = imageData[(i1 + j1 * imageWidth) * 4 + 3];
            if (alpha < 128)
                return true;
        }
    }
    return false;
}

BufferedImage *ImageBufferDownload::parseUserSkin(BufferedImage *bufferedimage)
{
    if (bufferedimage == nullptr)
        return nullptr;

    imageWidth  = 64;
    imageHeight = 32;

    // Allocate pixel buffer (RGBA, 4 bytes per pixel).
    imageData.assign(BufferedImage::checkedRgbaByteCount(imageWidth, imageHeight), 0);

    // drawImage equivalent: copy source pixels into our buffer at (0,0)
    int_t srcW = bufferedimage->getWidth();
    int_t srcH = bufferedimage->getHeight();
    int_t copyW = std::min(srcW, imageWidth);
    int_t copyH = std::min(srcH, imageHeight);

    std::vector<unsigned char> srcPixels(BufferedImage::checkedRgbaByteCount(copyW, copyH));
    bufferedimage->getRGB(0, 0, copyW, copyH, srcPixels.data());

    for (int_t y = 0; y < copyH; y++)
    {
        for (int_t x = 0; x < copyW; x++)
        {
            const std::size_t srcIdx = (static_cast<std::size_t>(x) + static_cast<std::size_t>(y) * static_cast<std::size_t>(copyW)) * 4u;
            const std::size_t dstIdx = (static_cast<std::size_t>(x) + static_cast<std::size_t>(y) * static_cast<std::size_t>(imageWidth)) * 4u;
            imageData[dstIdx + 0] = srcPixels[srcIdx + 0];
            imageData[dstIdx + 1] = srcPixels[srcIdx + 1];
            imageData[dstIdx + 2] = srcPixels[srcIdx + 2];
            imageData[dstIdx + 3] = srcPixels[srcIdx + 3];
        }
    }

    // Java: func_884_b(0, 0, 32, 16)  - set opaque
    setOpaque(0, 0, 32, 16);
    // Java: func_885_a(32, 0, 64, 32) - clear alpha (only if not already transparent)
    clearAlpha(32, 0, 64, 32);
    // Java: func_884_b(0, 16, 64, 32) - set opaque
    setOpaque(0, 16, 64, 32);

    bool flag = false;
    for (int_t i = 32; i < 64; i++)
    {
        for (int_t k = 0; k < 16; k++)
        {
            unsigned char alpha = imageData[(i + k * 64) * 4 + 3];
            if (alpha < 128)
                flag = true;
        }
    }

    if (!flag)
    {
        for (int_t j = 32; j < 64; j++)
        {
            for (int_t l = 0; l < 16; l++)
            {
                // flag1 set but never read (dead code in Java too)
                (void)imageData[(j + l * 64) * 4 + 3];
            }
        }
    }

    // Build result BufferedImage from our modified pixel data
    BufferedImage *result = new BufferedImage(imageWidth, imageHeight);
    result->setRGB(0, 0, imageWidth, imageHeight, imageData.data());
    return result;
}
