#include "ClippingHelperImpl.h"

#include "MathHelper.h"
#include "platform/RenderAPI.h"

ClippingHelperImpl ClippingHelperImpl::instance;

ClippingHelperImpl::ClippingHelperImpl()
{
	for (int i = 0; i < 16; i++)
	{
		projectionMatrixBuffer[i] = 0.0f;
		modelviewMatrixBuffer[i] = 0.0f;
		field_1691_h[i] = 0.0f;
	}
}

ClippingHelper *ClippingHelperImpl::getInstance()
{
	instance.init();
	return &instance;
}

void ClippingHelperImpl::normalize(float af[16][16], int i)
{
	float f = MathHelper::sqrt_float(af[i][0] * af[i][0] + af[i][1] * af[i][1] + af[i][2] * af[i][2]);
	af[i][0] /= f;
	af[i][1] /= f;
	af[i][2] /= f;
	af[i][3] /= f;
}

void ClippingHelperImpl::init()
{
	renderGetMatrix(RenderMatrixQuery::Projection, projectionMatrixBuffer);
	renderGetMatrix(RenderMatrixQuery::ModelView, modelviewMatrixBuffer);
	for (int i = 0; i < 16; i++)
	{
		projectionMatrix[i] = projectionMatrixBuffer[i];
		modelviewMatrix[i] = modelviewMatrixBuffer[i];
	}

	clippingMatrix[0] = modelviewMatrix[0] * projectionMatrix[0] + modelviewMatrix[1] * projectionMatrix[4] + modelviewMatrix[2] * projectionMatrix[8] + modelviewMatrix[3] * projectionMatrix[12];
	clippingMatrix[1] = modelviewMatrix[0] * projectionMatrix[1] + modelviewMatrix[1] * projectionMatrix[5] + modelviewMatrix[2] * projectionMatrix[9] + modelviewMatrix[3] * projectionMatrix[13];
	clippingMatrix[2] = modelviewMatrix[0] * projectionMatrix[2] + modelviewMatrix[1] * projectionMatrix[6] + modelviewMatrix[2] * projectionMatrix[10] + modelviewMatrix[3] * projectionMatrix[14];
	clippingMatrix[3] = modelviewMatrix[0] * projectionMatrix[3] + modelviewMatrix[1] * projectionMatrix[7] + modelviewMatrix[2] * projectionMatrix[11] + modelviewMatrix[3] * projectionMatrix[15];
	clippingMatrix[4] = modelviewMatrix[4] * projectionMatrix[0] + modelviewMatrix[5] * projectionMatrix[4] + modelviewMatrix[6] * projectionMatrix[8] + modelviewMatrix[7] * projectionMatrix[12];
	clippingMatrix[5] = modelviewMatrix[4] * projectionMatrix[1] + modelviewMatrix[5] * projectionMatrix[5] + modelviewMatrix[6] * projectionMatrix[9] + modelviewMatrix[7] * projectionMatrix[13];
	clippingMatrix[6] = modelviewMatrix[4] * projectionMatrix[2] + modelviewMatrix[5] * projectionMatrix[6] + modelviewMatrix[6] * projectionMatrix[10] + modelviewMatrix[7] * projectionMatrix[14];
	clippingMatrix[7] = modelviewMatrix[4] * projectionMatrix[3] + modelviewMatrix[5] * projectionMatrix[7] + modelviewMatrix[6] * projectionMatrix[11] + modelviewMatrix[7] * projectionMatrix[15];
	clippingMatrix[8] = modelviewMatrix[8] * projectionMatrix[0] + modelviewMatrix[9] * projectionMatrix[4] + modelviewMatrix[10] * projectionMatrix[8] + modelviewMatrix[11] * projectionMatrix[12];
	clippingMatrix[9] = modelviewMatrix[8] * projectionMatrix[1] + modelviewMatrix[9] * projectionMatrix[5] + modelviewMatrix[10] * projectionMatrix[9] + modelviewMatrix[11] * projectionMatrix[13];
	clippingMatrix[10] = modelviewMatrix[8] * projectionMatrix[2] + modelviewMatrix[9] * projectionMatrix[6] + modelviewMatrix[10] * projectionMatrix[10] + modelviewMatrix[11] * projectionMatrix[14];
	clippingMatrix[11] = modelviewMatrix[8] * projectionMatrix[3] + modelviewMatrix[9] * projectionMatrix[7] + modelviewMatrix[10] * projectionMatrix[11] + modelviewMatrix[11] * projectionMatrix[15];
	clippingMatrix[12] = modelviewMatrix[12] * projectionMatrix[0] + modelviewMatrix[13] * projectionMatrix[4] + modelviewMatrix[14] * projectionMatrix[8] + modelviewMatrix[15] * projectionMatrix[12];
	clippingMatrix[13] = modelviewMatrix[12] * projectionMatrix[1] + modelviewMatrix[13] * projectionMatrix[5] + modelviewMatrix[14] * projectionMatrix[9] + modelviewMatrix[15] * projectionMatrix[13];
	clippingMatrix[14] = modelviewMatrix[12] * projectionMatrix[2] + modelviewMatrix[13] * projectionMatrix[6] + modelviewMatrix[14] * projectionMatrix[10] + modelviewMatrix[15] * projectionMatrix[14];
	clippingMatrix[15] = modelviewMatrix[12] * projectionMatrix[3] + modelviewMatrix[13] * projectionMatrix[7] + modelviewMatrix[14] * projectionMatrix[11] + modelviewMatrix[15] * projectionMatrix[15];

	frustum[0][0] = clippingMatrix[3] - clippingMatrix[0];
	frustum[0][1] = clippingMatrix[7] - clippingMatrix[4];
	frustum[0][2] = clippingMatrix[11] - clippingMatrix[8];
	frustum[0][3] = clippingMatrix[15] - clippingMatrix[12];
	normalize(frustum, 0);
	frustum[1][0] = clippingMatrix[3] + clippingMatrix[0];
	frustum[1][1] = clippingMatrix[7] + clippingMatrix[4];
	frustum[1][2] = clippingMatrix[11] + clippingMatrix[8];
	frustum[1][3] = clippingMatrix[15] + clippingMatrix[12];
	normalize(frustum, 1);
	frustum[2][0] = clippingMatrix[3] + clippingMatrix[1];
	frustum[2][1] = clippingMatrix[7] + clippingMatrix[5];
	frustum[2][2] = clippingMatrix[11] + clippingMatrix[9];
	frustum[2][3] = clippingMatrix[15] + clippingMatrix[13];
	normalize(frustum, 2);
	frustum[3][0] = clippingMatrix[3] - clippingMatrix[1];
	frustum[3][1] = clippingMatrix[7] - clippingMatrix[5];
	frustum[3][2] = clippingMatrix[11] - clippingMatrix[9];
	frustum[3][3] = clippingMatrix[15] - clippingMatrix[13];
	normalize(frustum, 3);
	frustum[4][0] = clippingMatrix[3] - clippingMatrix[2];
	frustum[4][1] = clippingMatrix[7] - clippingMatrix[6];
	frustum[4][2] = clippingMatrix[11] - clippingMatrix[10];
	frustum[4][3] = clippingMatrix[15] - clippingMatrix[14];
	normalize(frustum, 4);
	frustum[5][0] = clippingMatrix[3] + clippingMatrix[2];
	frustum[5][1] = clippingMatrix[7] + clippingMatrix[6];
	frustum[5][2] = clippingMatrix[11] + clippingMatrix[10];
	frustum[5][3] = clippingMatrix[15] + clippingMatrix[14];
	normalize(frustum, 5);
}
