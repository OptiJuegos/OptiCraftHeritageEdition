#pragma once

#include <vector>
#include "java/Type.h"
#include "platform/PlatformConfig.h"

// net.minecraft.src.GLAllocation
class GLAllocation
{
public:
#if PLATFORM_PC
    static int_t generateDisplayLists(int_t count);
    static void deleteDisplayLists(int_t first);
#endif
    static void generateTextureNames(std::vector<int_t> &names);
    static void deleteTexturesAndDisplayLists();

private:
#if PLATFORM_PC
    static std::vector<int_t> displayLists;
#endif
    static std::vector<int_t> textureNames;
};
