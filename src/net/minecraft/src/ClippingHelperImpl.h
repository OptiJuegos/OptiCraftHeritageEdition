#pragma once

#include "ClippingHelper.h"

// net.minecraft.src.ClippingHelperImpl
class ClippingHelperImpl : public ClippingHelper
{
public:
	ClippingHelperImpl();

	static ClippingHelper *getInstance();

private:
	void normalize(float af[16][16], int i);
	void init();

	static ClippingHelperImpl instance;
	float projectionMatrixBuffer[16];
	float modelviewMatrixBuffer[16];
	float field_1691_h[16];
};
