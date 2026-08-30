#include "TexturedQuad.h"

#include "PositionTextureVertex.h"
#include "Vec3D.h"
#include "Tessellator.h"

TexturedQuad::TexturedQuad(PositionTextureVertex *apositiontexturevertex[]) :
	vertexPositions(nullptr),
	nVertices(0),
	invertNormal(false),
	ownsVertexObjects(false)
{
	// TexturedQuad owns the pointer array passed by ModelRenderer.
	// It does NOT own the PositionTextureVertex objects yet, because at this
	// point they usually point to ModelRenderer::corners.
	vertexPositions = apositiontexturevertex;
	nVertices = 4; // array length
}

TexturedQuad::TexturedQuad(PositionTextureVertex *apositiontexturevertex[], int i, int j, int k, int l) :
	TexturedQuad(apositiontexturevertex)
{
	const float insetU = 0.0015625f;
	const float insetV = 0.003125f;
	vertexPositions[0] = vertexPositions[0]->setTexturePosition((float)k / 64.0f - insetU, (float)j / 32.0f + insetV);
	vertexPositions[1] = vertexPositions[1]->setTexturePosition((float)i / 64.0f + insetU, (float)j / 32.0f + insetV);
	vertexPositions[2] = vertexPositions[2]->setTexturePosition((float)i / 64.0f + insetU, (float)l / 32.0f - insetV);
	vertexPositions[3] = vertexPositions[3]->setTexturePosition((float)k / 64.0f - insetU, (float)l / 32.0f - insetV);
	ownsVertexObjects = true;
}

TexturedQuad::TexturedQuad(PositionTextureVertex *apositiontexturevertex[], int i, int j, int k, int l, float textureWidth, float textureHeight) :
	TexturedQuad(apositiontexturevertex)
{
	vertexPositions[0] = vertexPositions[0]->setTexturePosition((float)k / textureWidth, (float)j / textureHeight);
	vertexPositions[1] = vertexPositions[1]->setTexturePosition((float)i / textureWidth, (float)j / textureHeight);
	vertexPositions[2] = vertexPositions[2]->setTexturePosition((float)i / textureWidth, (float)l / textureHeight);
	vertexPositions[3] = vertexPositions[3]->setTexturePosition((float)k / textureWidth, (float)l / textureHeight);
	ownsVertexObjects = true;
}

TexturedQuad::~TexturedQuad()
{
	if (vertexPositions != nullptr)
	{
		if (ownsVertexObjects)
		{
			for (int_t i = 0; i < nVertices; ++i)
			{
				delete vertexPositions[i];
				vertexPositions[i] = nullptr;
			}
		}

		delete[] vertexPositions;
		vertexPositions = nullptr;
	}
}

void TexturedQuad::flipFace()
{
	PositionTextureVertex **oldVertexPositions = vertexPositions;
	PositionTextureVertex **apositiontexturevertex = new PositionTextureVertex*[nVertices]();
	for (int_t i = 0; i < nVertices; i++)
	{
		apositiontexturevertex[i] = oldVertexPositions[nVertices - i - 1];
	}
	vertexPositions = apositiontexturevertex;
	delete[] oldVertexPositions;
}

void TexturedQuad::emitInto(Tessellator *tessellator, float f)
{
	if (!normalComputed)
	{
		// Geometry is fixed once the box is built, so compute the face normal
		// exactly once (double Vec3D math + sqrt) and cache the float result.
		Vec3D *vec3d = vertexPositions[1]->vector3D->subtract(vertexPositions[0]->vector3D);
		Vec3D *vec3d1 = vertexPositions[1]->vector3D->subtract(vertexPositions[2]->vector3D);
		Vec3D *vec3d2 = vec3d1->crossProduct(vec3d)->normalize();
		normalX = (float)vec3d2->xCoord;
		normalY = (float)vec3d2->yCoord;
		normalZ = (float)vec3d2->zCoord;
		normalComputed = true;
	}

	if (invertNormal)
	{
		tessellator->setNormal(-normalX, -normalY, -normalZ);
	}
	else
	{
		tessellator->setNormal(normalX, normalY, normalZ);
	}
	for (int_t i = 0; i < 4; i++)
	{
		PositionTextureVertex *positiontexturevertex = vertexPositions[i];
		tessellator->addVertexWithUV(
			positiontexturevertex->positionX * f,
			positiontexturevertex->positionY * f,
			positiontexturevertex->positionZ * f,
			positiontexturevertex->texturePositionX,
			positiontexturevertex->texturePositionY);
	}
}

void TexturedQuad::draw(Tessellator *tessellator, float f)
{
	tessellator->startDrawingQuads();
	emitInto(tessellator, f);
	tessellator->draw();
}
