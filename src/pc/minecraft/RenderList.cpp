#include "net/minecraft/src/RenderList.h"

#include "net/minecraft/src/WorldRenderer.h"
#include "platform/RenderAPI.h"

RenderList::RenderList()
{
    originX = 0;
    originY = 0;
    originZ = 0;
    viewerX = 0.0;
    viewerY = 0.0;
    viewerZ = 0.0;
    displayListIds.reserve(0x10000);
    initialized = false;
}

void RenderList::setup(int i, int j, int k, double d, double d1, double d2)
{
    initialized = true;
    displayListIds.clear();
    originX = i;
    originY = j;
    originZ = k;
    viewerX = d;
    viewerY = d1;
    viewerZ = d2;
}

bool RenderList::matchesPos(int i, int j, int k)
{
    return initialized && i == originX && j == originY && k == originZ;
}

void RenderList::addTerrainRenderer(WorldRenderer *renderer, int_t pass)
{
    if (renderer == nullptr)
        return;

    const int_t list = renderer->getGLCallListForPass(pass);
    if (list < 0)
        return;

    displayListIds.push_back(list);
    if (displayListIds.size() >= displayListIds.capacity())
        render();
}

void RenderList::render()
{
    if (!initialized)
        return;
    if (displayListIds.empty())
        return;

    const float translateX = static_cast<float>(static_cast<double>(originX) - viewerX);
    const float translateY = static_cast<float>(static_cast<double>(originY) - viewerY);
    const float translateZ = static_cast<float>(static_cast<double>(originZ) - viewerZ);
    renderPushMatrix();
    renderTranslate(translateX, translateY, translateZ);
    renderCallDisplayLists(static_cast<int>(displayListIds.size()), displayListIds.data());
    renderPopMatrix();
}

void RenderList::reset()
{
    initialized = false;
    displayListIds.clear();
}
