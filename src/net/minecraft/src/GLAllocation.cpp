#include "GLAllocation.h"

#include <algorithm>
#include <stdexcept>
#include "platform/RenderAPI.h"

#if PLATFORM_PC
std::vector<int_t> GLAllocation::displayLists;
#endif
std::vector<int_t> GLAllocation::textureNames;

#if PLATFORM_PC
int_t GLAllocation::generateDisplayLists(int_t count)
{
    if (count <= 0)
        throw std::invalid_argument("Display-list count must be positive");

    const int_t first = renderGenerateDisplayLists(count);
    if (first == 0)
        throw std::runtime_error("Renderer could not allocate display-list handles");
    displayLists.push_back(first);
    displayLists.push_back(count);
    return first;
}

void GLAllocation::deleteDisplayLists(int_t first)
{
    auto it = std::find(displayLists.begin(), displayLists.end(), first);
    if (it == displayLists.end())
        return;
    const int_t index = static_cast<int_t>(it - displayLists.begin());
    renderDeleteDisplayLists(displayLists[index], displayLists[index + 1]);
    displayLists.erase(displayLists.begin() + index, displayLists.begin() + index + 2);
}
#endif

void GLAllocation::generateTextureNames(std::vector<int_t> &names)
{
    renderGenerateTextures(static_cast<int>(names.size()), names.data());
    for (int_t value : names)
        textureNames.push_back(value);
}

void GLAllocation::deleteTexturesAndDisplayLists()
{
    renderResetResources();
#if PLATFORM_PC
    for (int_t i = 0; i < static_cast<int_t>(displayLists.size()); i += 2)
        renderDeleteDisplayLists(displayLists[i], displayLists[i + 1]);
    displayLists.clear();
#endif

    if (!textureNames.empty())
        renderDeleteTextures(static_cast<int>(textureNames.size()), textureNames.data());
    textureNames.clear();
}
