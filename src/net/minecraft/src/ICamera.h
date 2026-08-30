#pragma once

#include "AxisAlignedBB.h"

// net.minecraft.src.ICamera
class ICamera
{
public:
	virtual ~ICamera() = default;
	virtual bool isBoundingBoxInFrustum(AxisAlignedBB *axisalignedbb) = 0;
	virtual void setPosition(double d, double d1, double d2) = 0;

	// 0 = outside, 1 = intersecting, 2 = entirely inside the frustum.
	//
	// One call answers both "should this be drawn?" and "can the per-vertex clip
	// be skipped?", which is why the PS2 renderer uses this instead of calling
	// isBoundingBoxInFrustum and a separate fully-inside test.
	//
	// The default never reports 2. That is always safe -- callers treat anything
	// below 2 as "clip normally" -- so a camera implementation that has no cheap
	// fully-inside test does not have to provide one.
	virtual int classifyBoundingBox(AxisAlignedBB *axisalignedbb)
	{
		return isBoundingBoxInFrustum(axisalignedbb) ? 1 : 0;
	}
};
