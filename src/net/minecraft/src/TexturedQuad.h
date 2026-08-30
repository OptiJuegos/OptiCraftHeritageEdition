#pragma once

#include "java/Type.h"

class PositionTextureVertex;
class Tessellator;
class Vec3D;

// net.minecraft.src.TexturedQuad
class TexturedQuad
{
public:
	TexturedQuad(PositionTextureVertex *apositiontexturevertex[]);
	TexturedQuad(PositionTextureVertex *apositiontexturevertex[], int i, int j, int k, int l);
	TexturedQuad(PositionTextureVertex *apositiontexturevertex[], int i, int j, int k, int l, float textureWidth, float textureHeight);
	~TexturedQuad();

	void flipFace();
	void draw(Tessellator *tessellator, float f);
	// Emit this quad's normal + 4 vertices into an already-open Tessellator
	// batch (caller does startDrawingQuads/draw). Lets a whole box's 6 faces
	// share one draw call instead of 6 separate glDrawArrays kicks.
	void emitInto(Tessellator *tessellator, float f);

	PositionTextureVertex **vertexPositions;
	int_t nVertices;
	bool invertNormal;

private:
	bool ownsVertexObjects;
	// The face normal is a function of the (static) box geometry only, so cache
	// it after the first emit instead of recomputing the double-precision
	// subtract/cross/normalize (with a sqrt) every frame on consoles.
	bool normalComputed = false;
	float normalX = 0.0f;
	float normalY = 0.0f;
	float normalZ = 0.0f;
};
