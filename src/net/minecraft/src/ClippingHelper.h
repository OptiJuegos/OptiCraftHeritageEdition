#pragma once

// net.minecraft.src.ClippingHelper
class ClippingHelper
{
public:
	ClippingHelper();

	bool isBoxInFrustum(double d, double d1, double d2, double d3, double d4, double d5);

	// Three-way version of the same test: 0 = outside, 1 = intersecting,
	// 2 = entirely inside every plane. The extra answer is what lets the PS2 draw
	// path skip per-vertex clip outcodes for a chunk section that cannot produce
	// an out-of-frustum vertex (see ps2_draw_3d_set_fully_inside). It costs one
	// more plane evaluation per plane than isBoxInFrustum, so it stays a separate
	// entry point rather than replacing it on the desktop path.
	int classifyBox(double d, double d1, double d2, double d3, double d4, double d5);

	float frustum[16][16];
	float projectionMatrix[16];
	float modelviewMatrix[16];
	float clippingMatrix[16];
};
