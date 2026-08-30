#include "Frustrum.h"

#include "AxisAlignedBB.h"
#include "ClippingHelper.h"
#include "ClippingHelperImpl.h"

Frustrum::Frustrum()
{
	clippingHelper = ClippingHelperImpl::getInstance();
	xPosition = 0.0;
	yPosition = 0.0;
	zPosition = 0.0;
}

void Frustrum::setPosition(double d, double d1, double d2)
{
	xPosition = d;
	yPosition = d1;
	zPosition = d2;
}

bool Frustrum::isBoxInFrustum(double d, double d1, double d2, double d3, double d4, double d5)
{
	return clippingHelper->isBoxInFrustum(d - xPosition, d1 - yPosition, d2 - zPosition, d3 - xPosition, d4 - yPosition, d5 - zPosition);
}

bool Frustrum::isBoundingBoxInFrustum(AxisAlignedBB *axisalignedbb)
{
	return isBoxInFrustum(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ, axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->maxZ);
}

int Frustrum::classifyBoundingBox(AxisAlignedBB *axisalignedbb)
{
	// The planes are camera-relative, so the box has to be too -- same shift
	// isBoxInFrustum applies.
	return clippingHelper->classifyBox(
		axisalignedbb->minX - xPosition, axisalignedbb->minY - yPosition, axisalignedbb->minZ - zPosition,
		axisalignedbb->maxX - xPosition, axisalignedbb->maxY - yPosition, axisalignedbb->maxZ - zPosition);
}
