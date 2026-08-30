#pragma once

#include "ICamera.h"

class ClippingHelper;

// net.minecraft.src.Frustrum
class Frustrum : public ICamera
{
public:
	Frustrum();

	void setPosition(double d, double d1, double d2) override;
	bool isBoxInFrustum(double d, double d1, double d2, double d3, double d4, double d5);
	bool isBoundingBoxInFrustum(AxisAlignedBB *axisalignedbb) override;
	int classifyBoundingBox(AxisAlignedBB *axisalignedbb) override;

private:
	ClippingHelper *clippingHelper;
	double xPosition;
	double yPosition;
	double zPosition;
};
