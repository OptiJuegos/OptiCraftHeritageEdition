#pragma once

#include <map>
#include <memory>
#include <vector>

#include "java/Type.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformTuning.h"

// Scalar the geometry emitters use for vertex positions and texture
// coordinates. The vertex buffer has always stored both as 32-bit floats, so on
// a CPU without a double FPU the double intermediates are pure software cost;
// PLATFORM_FLOAT_VERTEX_MATH drops them. See the knob in Ps2Tuning.h for the
// precision range that trade accepts.
#if PLATFORM_FLOAT_VERTEX_MATH
typedef float tess_coord_t;
#else
typedef double tess_coord_t;
#endif


struct TessellatorTextureMesh
{
	int_t textureId = 0;
	RenderCapturedMesh mesh;
};

// net.minecraft.src.Tessellator
class Tessellator
{
public:
	static bool convertQuadsToTriangles;

	static Tessellator instance;

	Tessellator(int bufferSize);
	~Tessellator();

	int_t draw();
	void startDrawingQuads();
	void startDrawing(int mode);
	void setTextureUV(tess_coord_t u, tess_coord_t v);
	void setBrightness(int value);
	void setColorOpaque_F(float r, float g, float b);
	void setColorRGBA_F(float r, float g, float b, float a);
	void setColorOpaque(int r, int g, int b);
	void setColorRGBA(int r, int g, int b, int a);
	void addVertexWithUV(tess_coord_t x, tess_coord_t y, tess_coord_t z, tess_coord_t u, tess_coord_t v);
	void addVertex(tess_coord_t x, tess_coord_t y, tess_coord_t z);
	void setColorOpaque_I(int color);
	void setColorRGBA_I(int color, int alpha);
	void disableColor();
	void setNormal(float x, float y, float z);
	void setTranslation(double x, double y, double z);
	void addTranslation(float x, float y, float z);
	void setTranslationD(double x, double y, double z);
	void setTranslationF(float x, float y, float z);

	// Consume the currently-open batch without exposing backend-specific mesh types.
	bool capture(RenderCapturedMesh& out, bool append = false);
#if PLATFORM_PERSISTENT_RENDER_MESH
	bool finishPersistentMesh(int handle);
#endif
	bool finishStaticMesh(RenderStaticMesh& mesh);
	void cancelDrawing();

	// OptiFine CTM uses one tessellator stream per atlas. The streams keep the
	// same vertex state/translation as the primary batch and are drained by the
	// terrain builder, keeping texture grouping above GL/GX/GS.
	Tessellator *getSubTessellator(int_t textureId);
	void captureTextureGroups(std::vector<TessellatorTextureMesh> &out, bool append = false);
	bool hasPendingVertices() const { return isDrawing && vertexCount > 0; }

private:
	void reset();
	void ensureRawBufferCapacity(int_t additionalInts);

	int_t* rawBuffer;
	int_t bufferSize;

	int_t vertexCount;
	tess_coord_t textureU;
	tess_coord_t textureV;
	int_t brightness;
	int_t color;
	bool hasColor;
	bool hasTexture;
	bool hasBrightness;
	bool hasNormals;
	int_t rawBufferIndex;
	int_t addedVertices;
	bool isColorDisabled;
	int_t drawMode;
	// setTranslationD keeps its double signature: every caller passes a negated
	// world position and only this class needs to know at what precision the
	// offset is finally applied.
	tess_coord_t xOffset;
	tess_coord_t yOffset;
	tess_coord_t zOffset;
	int_t normal;
	bool isDrawing;
	std::map<int_t, std::unique_ptr<Tessellator>> subTessellators;
};

