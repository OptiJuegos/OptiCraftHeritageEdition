#include "GenLayerVoronoiZoom.h"

#include "IntCache.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

#include <algorithm>

namespace
{
#if PLATFORM_FLOAT_BIOME_NOISE
using VoronoiReal = float;
#else
using VoronoiReal = double;
#endif
}

GenLayerVoronoiZoom::GenLayerVoronoiZoom(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerVoronoiZoom::getInts(int_t x, int_t z, int_t width, int_t height)
{
    x = JavaArithmetic::intSub(x, 2);
    z = JavaArithmetic::intSub(z, 2);
    constexpr int_t zoomBits = 2;
    constexpr int_t zoom = 1 << zoomBits;
    const int_t parentX = JavaArithmetic::intShr(x, zoomBits);
    const int_t parentZ = JavaArithmetic::intShr(z, zoomBits);
    const int_t parentWidth = JavaArithmetic::intAdd(JavaArithmetic::intShr(width, zoomBits), 3);
    const int_t parentHeight = JavaArithmetic::intAdd(JavaArithmetic::intShr(height, zoomBits), 3);
    const IntArray &source = parent->getInts(parentX, parentZ, parentWidth, parentHeight);
    const int_t expandedWidth = JavaArithmetic::intShl(parentWidth, zoomBits);
    const int_t expandedHeight = JavaArithmetic::intShl(parentHeight, zoomBits);
    IntArray &expanded = IntCache::getIntCache(JavaArithmetic::intMul(expandedWidth, expandedHeight));

    for (int_t parentRow = 0; parentRow < parentHeight - 1; ++parentRow)
    {
        int_t topLeftBiome = source[parentRow * parentWidth];
        int_t bottomLeftBiome = source[(parentRow + 1) * parentWidth];

        for (int_t parentColumn = 0; parentColumn < parentWidth - 1; ++parentColumn)
        {
            constexpr VoronoiReal inverseJitterRange = static_cast<VoronoiReal>(1.0 / 1024.0);
            constexpr VoronoiReal spread = static_cast<VoronoiReal>(zoom) * static_cast<VoronoiReal>(0.9);

            initChunkSeed(static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(parentColumn, parentX), zoomBits)),
                          static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(parentRow, parentZ), zoomBits)));
            const VoronoiReal topLeftX = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread;
            const VoronoiReal topLeftZ = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread;

            initChunkSeed(static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(JavaArithmetic::intAdd(parentColumn, parentX), 1), zoomBits)),
                          static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(parentRow, parentZ), zoomBits)));
            const VoronoiReal topRightX = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread + static_cast<VoronoiReal>(zoom);
            const VoronoiReal topRightZ = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread;

            initChunkSeed(static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(parentColumn, parentX), zoomBits)),
                          static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(JavaArithmetic::intAdd(parentRow, parentZ), 1), zoomBits)));
            const VoronoiReal bottomLeftX = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread;
            const VoronoiReal bottomLeftZ = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread + static_cast<VoronoiReal>(zoom);

            initChunkSeed(static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(JavaArithmetic::intAdd(parentColumn, parentX), 1), zoomBits)),
                          static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(JavaArithmetic::intAdd(parentRow, parentZ), 1), zoomBits)));
            const VoronoiReal bottomRightX = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread + static_cast<VoronoiReal>(zoom);
            const VoronoiReal bottomRightZ = (static_cast<VoronoiReal>(nextInt(1024)) * inverseJitterRange - static_cast<VoronoiReal>(0.5)) * spread + static_cast<VoronoiReal>(zoom);

            const int_t topRightBiome = source[parentColumn + 1 + parentRow * parentWidth];
            const int_t bottomRightBiome = source[parentColumn + 1 + (parentRow + 1) * parentWidth];

            for (int_t localZ = 0; localZ < zoom; ++localZ)
            {
                int_t outputIndex = ((parentRow << zoomBits) + localZ) * expandedWidth + (parentColumn << zoomBits);
                for (int_t localX = 0; localX < zoom; ++localX)
                {
                    const VoronoiReal dTopLeft = (localZ - topLeftZ) * (localZ - topLeftZ) + (localX - topLeftX) * (localX - topLeftX);
                    const VoronoiReal dTopRight = (localZ - topRightZ) * (localZ - topRightZ) + (localX - topRightX) * (localX - topRightX);
                    const VoronoiReal dBottomLeft = (localZ - bottomLeftZ) * (localZ - bottomLeftZ) + (localX - bottomLeftX) * (localX - bottomLeftX);
                    const VoronoiReal dBottomRight = (localZ - bottomRightZ) * (localZ - bottomRightZ) + (localX - bottomRightX) * (localX - bottomRightX);

                    if (dTopLeft < dTopRight && dTopLeft < dBottomLeft && dTopLeft < dBottomRight)
                        expanded[outputIndex++] = topLeftBiome;
                    else if (dTopRight < dTopLeft && dTopRight < dBottomLeft && dTopRight < dBottomRight)
                        expanded[outputIndex++] = topRightBiome;
                    else if (dBottomLeft < dTopLeft && dBottomLeft < dTopRight && dBottomLeft < dBottomRight)
                        expanded[outputIndex++] = bottomLeftBiome;
                    else
                        expanded[outputIndex++] = bottomRightBiome;
                }
            }

            topLeftBiome = topRightBiome;
            bottomLeftBiome = bottomRightBiome;
        }
    }

    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));
    for (int_t row = 0; row < height; ++row)
    {
        const int_t sourceIndex = (row + (z & (zoom - 1))) * expandedWidth + (x & (zoom - 1));
        std::copy_n(expanded.begin() + sourceIndex, width, result.begin() + row * width);
    }
    return result;
}
