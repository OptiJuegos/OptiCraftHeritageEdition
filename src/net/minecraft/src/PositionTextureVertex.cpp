#include "PositionTextureVertex.h"

#include "Vec3D.h"

PositionTextureVertex::PositionTextureVertex(float f, float f1, float f2, float f3, float f4) :
	vector3D(Vec3D::createVectorHelper(f, f1, f2)),
	positionX(f),
	positionY(f1),
	positionZ(f2),
	texturePositionX(f3),
	texturePositionY(f4),
	ownsVector3D(true)
{
}

PositionTextureVertex *PositionTextureVertex::setTexturePosition(float f, float f1)
{
	return new PositionTextureVertex(this, f, f1);
}

PositionTextureVertex::PositionTextureVertex(PositionTextureVertex *positiontexturevertex, float f, float f1)
{
	ownsVector3D = false;
	vector3D = positiontexturevertex->vector3D;
	positionX = positiontexturevertex->positionX;
	positionY = positiontexturevertex->positionY;
	positionZ = positiontexturevertex->positionZ;
	texturePositionX = f;
	texturePositionY = f1;
}

PositionTextureVertex::PositionTextureVertex(Vec3D *vec3d, float f, float f1)
{
	ownsVector3D = false;
	vector3D = vec3d;
	positionX = vec3d != nullptr ? (float)vec3d->xCoord : 0.0f;
	positionY = vec3d != nullptr ? (float)vec3d->yCoord : 0.0f;
	positionZ = vec3d != nullptr ? (float)vec3d->zCoord : 0.0f;
	texturePositionX = f;
	texturePositionY = f1;
}

PositionTextureVertex::~PositionTextureVertex()
{
	if (ownsVector3D)
	{
		delete vector3D;
		vector3D = nullptr;
	}
}
