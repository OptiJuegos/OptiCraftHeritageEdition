#pragma once

class Vec3D;

// net.minecraft.src.PositionTextureVertex
class PositionTextureVertex
{
public:
	PositionTextureVertex(float f, float f1, float f2, float f3, float f4);
	PositionTextureVertex(PositionTextureVertex *positiontexturevertex, float f, float f1);
	PositionTextureVertex(Vec3D *vec3d, float f, float f1);
	~PositionTextureVertex();

	PositionTextureVertex *setTexturePosition(float f, float f1);

	Vec3D *vector3D;
	// The same value vector3D holds, narrowed once at construction instead of on
	// every emit.
	//
	// Model geometry is fixed once a box is built -- nothing writes back through
	// vector3D -- so this is exactly the float the emitters used to compute, not
	// an approximation of it. What it saves is three libgcc narrowing calls plus
	// a pointer chase into a separately allocated 24-byte Vec3D per vertex, per
	// quad, per box, per entity, per frame. On a CPU with no double FPU and an
	// 8 KB data cache both halves matter.
	//
	// vector3D stays: TexturedQuad still derives its face normal from it, once
	// per quad for the lifetime of the model, where double costs nothing.
	float positionX;
	float positionY;
	float positionZ;
	float texturePositionX;
	float texturePositionY;

private:
	bool ownsVector3D;
};
