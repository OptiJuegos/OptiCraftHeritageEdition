// RenderList.cpp — Nintendo 3DS implementation of RenderList.
//
// RenderGlobal's non-PS2 terrain batch path groups the visible sections by
// origin and replays them through RenderList; the 3DS shares that path with the
// Wii and the desktop, so the storage here is the PC-shaped `displayListIds`
// vector (RenderList.h widens its PLATFORM_PC guard to include the 3DS).
//
// What differs from src/pc/minecraft/RenderList.cpp is only the reserve size.
// The PC reserves 0x10000 so it never flushes mid-frame while genuinely
// recording GL lists; here the ids resolve to RenderAPI_CTR_3DS's display-list
// stubs, so a flush costs a no-op and there is nothing to protect. A quarter of
// a megabyte of reserved vector on an Old 3DS's 64 MB application heap would be
// a much worse trade.
//
// TODO(phase 2): replace with the citro3d submission, as the Wii replaced this
// with its native GX handle batch.
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
	displayListIds.reserve(256);
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
