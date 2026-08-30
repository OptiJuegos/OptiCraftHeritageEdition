#ifndef RENDERHELPER_H
#define RENDERHELPER_H

#include <vector>

// net.minecraft.src.RenderHelper
class RenderHelper {
public:
    RenderHelper();

    static void disableStandardItemLighting();
    static void enableStandardItemLighting();
    static void enableGUIStandardItemLighting();

private:
    static std::vector<float> createFloatBuffer(float f, float f1, float f2, float f3);
    static std::vector<float> createFloatBufferFromDoubles(double d, double d1, double d2, double d3);

    static std::vector<float> field_1695_a;
};

#endif