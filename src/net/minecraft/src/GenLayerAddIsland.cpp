#include "GenLayerAddIsland.h"
#include "java/Arithmetic.h"

#include "BiomeGenBase.h"
#include "IntCache.h"

GenLayerAddIsland::GenLayerAddIsland(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerAddIsland::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const int_t parentX = JavaArithmetic::intSub(x, 1);
    const int_t parentZ = JavaArithmetic::intSub(z, 1);
    const int_t parentWidth = JavaArithmetic::intAdd(width, 2);
    const int_t parentHeight = JavaArithmetic::intAdd(height, 2);
    const IntArray &source = parent->getInts(parentX, parentZ, parentWidth, parentHeight);
    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));

    for (int_t localZ = 0; localZ < height; ++localZ)
    {
        for (int_t localX = 0; localX < width; ++localX)
        {
            const int_t nw = source[localX + (localZ)*parentWidth];
            const int_t ne = source[localX + 2 + (localZ)*parentWidth];
            const int_t sw = source[localX + (localZ + 2) * parentWidth];
            const int_t se = source[localX + 2 + (localZ + 2) * parentWidth];
            const int_t center = source[localX + 1 + (localZ + 1) * parentWidth];
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intAdd(localX, x)), static_cast<long_t>(JavaArithmetic::intAdd(localZ, z)));

            if (center == 0 && (nw != 0 || ne != 0 || sw != 0 || se != 0))
            {
                int_t candidates = 1;
                int_t selected = 1;
                if (nw != 0 && nextInt(candidates++) == 0) selected = nw;
                if (ne != 0 && nextInt(candidates++) == 0) selected = ne;
                if (sw != 0 && nextInt(candidates++) == 0) selected = sw;
                if (se != 0 && nextInt(candidates++) == 0) selected = se;

                if (nextInt(3) == 0)
                    result[localX + localZ * width] = selected;
                else if (selected == BiomeGenBase::icePlains->biomeID)
                    result[localX + localZ * width] = BiomeGenBase::frozenOcean->biomeID;
                else
                    result[localX + localZ * width] = 0;
            }
            else if (center > 0 && (nw == 0 || ne == 0 || sw == 0 || se == 0))
            {
                if (nextInt(5) == 0)
                {
                    result[localX + localZ * width] = center == BiomeGenBase::icePlains->biomeID
                        ? BiomeGenBase::frozenOcean->biomeID
                        : 0;
                }
                else
                    result[localX + localZ * width] = center;
            }
            else
                result[localX + localZ * width] = center;
        }
    }

    return result;
}
