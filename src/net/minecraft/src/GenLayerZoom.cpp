#include "GenLayerZoom.h"

#include "IntCache.h"
#include "java/Arithmetic.h"

#include <algorithm>

GenLayerZoom::GenLayerZoom(long_t seed, Ptr parentLayer) : GenLayer(seed)
{
    parent = std::move(parentLayer);
}

GenLayer::IntArray &GenLayerZoom::getInts(int_t x, int_t z, int_t width, int_t height)
{
    const int_t parentX = JavaArithmetic::intShr(x, 1);
    const int_t parentZ = JavaArithmetic::intShr(z, 1);
    const int_t parentWidth = JavaArithmetic::intAdd(JavaArithmetic::intShr(width, 1), 3);
    const int_t parentHeight = JavaArithmetic::intAdd(JavaArithmetic::intShr(height, 1), 3);
    const IntArray &source = parent->getInts(parentX, parentZ, parentWidth, parentHeight);
    IntArray &expanded = IntCache::getIntCache(JavaArithmetic::intMul(JavaArithmetic::intMul(parentWidth, 2), JavaArithmetic::intMul(parentHeight, 2)));
    const int_t expandedWidth = JavaArithmetic::intShl(parentWidth, 1);

    for (int_t parentRow = 0; parentRow < parentHeight - 1; ++parentRow)
    {
        int_t outputIndex = (parentRow << 1) * expandedWidth;
        int_t topLeft = source[parentRow * parentWidth];
        int_t bottomLeft = source[(parentRow + 1) * parentWidth];

        for (int_t parentColumn = 0; parentColumn < parentWidth - 1; ++parentColumn)
        {
            initChunkSeed(static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(parentColumn, parentX), 1)),
                          static_cast<long_t>(JavaArithmetic::intShl(JavaArithmetic::intAdd(parentRow, parentZ), 1)));
            const int_t topRight = source[parentColumn + 1 + parentRow * parentWidth];
            const int_t bottomRight = source[parentColumn + 1 + (parentRow + 1) * parentWidth];

            expanded[outputIndex] = topLeft;
            expanded[outputIndex++ + expandedWidth] = choose(topLeft, bottomLeft);
            expanded[outputIndex] = choose(topLeft, topRight);
            expanded[outputIndex++ + expandedWidth] = choose(topLeft, topRight, bottomLeft, bottomRight);
            topLeft = topRight;
            bottomLeft = bottomRight;
        }
    }

    IntArray &result = IntCache::getIntCache(IntCache::checkedAreaSize(width, height));
    for (int_t row = 0; row < height; ++row)
    {
        const int_t sourceIndex = (row + (z & 1)) * expandedWidth + (x & 1);
        std::copy_n(expanded.begin() + sourceIndex, width, result.begin() + row * width);
    }
    return result;
}

int_t GenLayerZoom::choose(int_t first, int_t second)
{
    return nextInt(2) == 0 ? first : second;
}

int_t GenLayerZoom::choose(int_t first, int_t second, int_t third, int_t fourth)
{
    return func_35514_b(first, second, third, fourth);
}

int_t GenLayerZoom::func_35514_b(int_t first, int_t second, int_t third, int_t fourth)
{
    if (second == third && third == fourth) return second;
    if (first == second && first == third) return first;
    if (first == second && first == fourth) return first;
    if (first == third && first == fourth) return first;
    if (first == second && third != fourth) return first;
    if (first == third && second != fourth) return first;
    if (first == fourth && second != third) return first;
    if (second == first && third != fourth) return second;
    if (second == third && first != fourth) return second;
    if (second == fourth && first != third) return second;
    if (third == first && second != fourth) return third;
    if (third == second && first != fourth) return third;
    if (third == fourth && first != second) return third;
    if (fourth == first && second != third) return third;
    if (fourth == second && first != third) return third;
    if (fourth == third && first != second) return third;

    const int_t selected = nextInt(4);
    return selected == 0 ? first : selected == 1 ? second : selected == 2 ? third : fourth;
}

GenLayer::Ptr GenLayerZoom::func_35515_a(long_t seed, Ptr layer, int_t count)
{
    for (int_t i = 0; i < count; ++i)
        layer = std::make_shared<GenLayerZoom>(JavaArithmetic::longAdd(seed, static_cast<long_t>(i)), layer);
    return layer;
}
