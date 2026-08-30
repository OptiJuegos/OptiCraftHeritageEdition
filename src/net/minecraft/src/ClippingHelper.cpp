#include "ClippingHelper.h"
#include "platform/PlatformConfig.h"

ClippingHelper::ClippingHelper()
{
	for (int i = 0; i < 16; i++)
	{
		projectionMatrix[i] = 0.0f;
		modelviewMatrix[i] = 0.0f;
		clippingMatrix[i] = 0.0f;
		for (int j = 0; j < 16; j++)
			frustum[i][j] = 0.0f;
	}
}

// A box lies entirely behind a plane exactly when the corner that maximises the
// plane equation is still behind it, so one selected corner replaces the eight
// equivalent Java tests. Preserve Java's double arithmetic by default; the
// low-CPU profile keeps the original PS2 single-precision shortcut.
bool ClippingHelper::isBoxInFrustum(double d, double d1, double d2, double d3, double d4, double d5)
{
#if PLATFORM_CONSOLE_LOW
	using frustum_real_t = float;
#else
	using frustum_real_t = double;
#endif
	const frustum_real_t minX = (frustum_real_t)d, minY = (frustum_real_t)d1, minZ = (frustum_real_t)d2;
	const frustum_real_t maxX = (frustum_real_t)d3, maxY = (frustum_real_t)d4, maxZ = (frustum_real_t)d5;

	for (int i = 0; i < 6; i++)
	{
		const frustum_real_t a = (frustum_real_t)frustum[i][0];
		const frustum_real_t b = (frustum_real_t)frustum[i][1];
		const frustum_real_t c = (frustum_real_t)frustum[i][2];

		const frustum_real_t px = a > 0 ? maxX : minX;
		const frustum_real_t py = b > 0 ? maxY : minY;
		const frustum_real_t pz = c > 0 ? maxZ : minZ;

		if (a * px + b * py + c * pz + (frustum_real_t)frustum[i][3] <= 0)
			return false;
	}
	return true;
}

// Same maximising-corner argument, plus its mirror: the box is entirely in FRONT
// of a plane exactly when the corner that MINIMISES the plane equation is still
// in front of it. Checking both corners per plane classifies the box in one pass.
//
// The planes ClippingHelperImpl builds are not normalised, which is fine here --
// only the sign of the plane equation is read, never a distance.
int ClippingHelper::classifyBox(double d, double d1, double d2, double d3, double d4, double d5)
{
#if PLATFORM_CONSOLE_LOW
	using frustum_real_t = float;
#else
	using frustum_real_t = double;
#endif
	const frustum_real_t minX = (frustum_real_t)d, minY = (frustum_real_t)d1, minZ = (frustum_real_t)d2;
	const frustum_real_t maxX = (frustum_real_t)d3, maxY = (frustum_real_t)d4, maxZ = (frustum_real_t)d5;

	bool fully = true;

	for (int i = 0; i < 6; i++)
	{
		const frustum_real_t a = (frustum_real_t)frustum[i][0];
		const frustum_real_t b = (frustum_real_t)frustum[i][1];
		const frustum_real_t c = (frustum_real_t)frustum[i][2];
		const frustum_real_t w = (frustum_real_t)frustum[i][3];

		const frustum_real_t px = a > 0 ? maxX : minX;
		const frustum_real_t py = b > 0 ? maxY : minY;
		const frustum_real_t pz = c > 0 ? maxZ : minZ;

		if (a * px + b * py + c * pz + w <= 0)
			return 0;

		// Once one plane cuts the box the answer can only be "intersecting", so
		// stop paying for the second corner.
		if (fully)
		{
			const frustum_real_t nx = a > 0 ? minX : maxX;
			const frustum_real_t ny = b > 0 ? minY : maxY;
			const frustum_real_t nz = c > 0 ? minZ : maxZ;

			if (a * nx + b * ny + c * nz + w <= 0)
				fully = false;
		}
	}

	return fully ? 2 : 1;
}
