#include "Vec3D.h"

#include "MathHelper.h"

#include <memory>
#include <vector>

Vec3D Vec3D::pool[Vec3D::POOL_SIZE];
int_t Vec3D::nextVector = 0;

// Java's temporary vector pool could allocate extra Vec3D instances and let the
// GC reclaim them. The C++ port must own overflow entries explicitly.
static std::vector<std::unique_ptr<Vec3D>> g_vec3DOverflowPool;

Vec3D *Vec3D::createVectorHelper(double d, double d1, double d2)
{
	return new Vec3D(d, d1, d2);
}

void Vec3D::initialize()
{
	nextVector = 0;
}

void Vec3D::trimVectorPool()
{
	nextVector = 0;
	g_vec3DOverflowPool.clear();
	g_vec3DOverflowPool.shrink_to_fit();
}

Vec3D *Vec3D::createVector(double d, double d1, double d2)
{
	int_t index = nextVector++;
	if (index < POOL_SIZE)
		return pool[index].setComponents(d, d1, d2);

	size_t overflowIndex = static_cast<size_t>(index - POOL_SIZE);
	if (overflowIndex >= g_vec3DOverflowPool.size())
		g_vec3DOverflowPool.emplace_back(new Vec3D(d, d1, d2));
	return g_vec3DOverflowPool[overflowIndex]->setComponents(d, d1, d2);
}

Vec3D::Vec3D(double d, double d1, double d2)
{
	if (d == -0.0)
		d = 0.0;
	if (d1 == -0.0)
		d1 = 0.0;
	if (d2 == -0.0)
		d2 = 0.0;
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
}

Vec3D *Vec3D::setComponents(double d, double d1, double d2)
{
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
	return this;
}

Vec3D *Vec3D::subtract(Vec3D *vec3d)
{
	return createVector(vec3d->xCoord - xCoord, vec3d->yCoord - yCoord, vec3d->zCoord - zCoord);
}

Vec3D *Vec3D::normalize()
{
#if PLATFORM_FLOAT_VECTOR_MATH
	// The three divisions are what this is really about: a double divide has no
	// hardware behind it on the EE, and normalize() issues one per axis.
	const float x = (float)xCoord;
	const float y = (float)yCoord;
	const float z = (float)zCoord;
	const float d = MathHelper::sqrt_float(x * x + y * y + z * z);
	if (d < 0.0001f)
		return createVector(0.0, 0.0, 0.0);
	const float inv = 1.0f / d;
	return createVector((double)(x * inv), (double)(y * inv), (double)(z * inv));
#else
	double d = MathHelper::sqrt_double(xCoord * xCoord + yCoord * yCoord + zCoord * zCoord);
	if (d < 0.0001)
		return createVector(0.0, 0.0, 0.0);
	else
		return createVector(xCoord / d, yCoord / d, zCoord / d);
#endif
}

Vec3D *Vec3D::crossProduct(Vec3D *vec3d)
{
	return createVector(yCoord * vec3d->zCoord - zCoord * vec3d->yCoord, zCoord * vec3d->xCoord - xCoord * vec3d->zCoord, xCoord * vec3d->yCoord - yCoord * vec3d->xCoord);
}

double Vec3D::dotProduct(Vec3D *vec3d)
{
	return xCoord * vec3d->xCoord + yCoord * vec3d->yCoord + zCoord * vec3d->zCoord;
}

Vec3D *Vec3D::addVector(double d, double d1, double d2)
{
	return createVector(xCoord + d, yCoord + d1, zCoord + d2);
}

double Vec3D::distanceTo(Vec3D *vec3d)
{
#if PLATFORM_FLOAT_VECTOR_MATH
	// The differences are taken at double width so two distant points still
	// subtract exactly; only the squaring and the root narrow. The result was
	// already passing through float, since sqrt_double returns one.
	const float d = (float)(vec3d->xCoord - xCoord);
	const float d1 = (float)(vec3d->yCoord - yCoord);
	const float d2 = (float)(vec3d->zCoord - zCoord);
	return (double)MathHelper::sqrt_float(d * d + d1 * d1 + d2 * d2);
#else
	double d = vec3d->xCoord - xCoord;
	double d1 = vec3d->yCoord - yCoord;
	double d2 = vec3d->zCoord - zCoord;
	return (double)MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
#endif
}

double Vec3D::squareDistanceTo(Vec3D *vec3d)
{
	double d = vec3d->xCoord - xCoord;
	double d1 = vec3d->yCoord - yCoord;
	double d2 = vec3d->zCoord - zCoord;
	return d * d + d1 * d1 + d2 * d2;
}

double Vec3D::squareDistanceTo(double d, double d1, double d2)
{
	double d3 = d - xCoord;
	double d4 = d1 - yCoord;
	double d5 = d2 - zCoord;
	return d3 * d3 + d4 * d4 + d5 * d5;
}

double Vec3D::lengthVector()
{
#if PLATFORM_FLOAT_VECTOR_MATH
	const float x = (float)xCoord;
	const float y = (float)yCoord;
	const float z = (float)zCoord;
	return (double)MathHelper::sqrt_float(x * x + y * y + z * z);
#else
	return (double)MathHelper::sqrt_double(xCoord * xCoord + yCoord * yCoord + zCoord * zCoord);
#endif
}

Vec3D *Vec3D::getIntermediateWithXValue(Vec3D *vec3d, double d)
{
	double d1 = vec3d->xCoord - xCoord;
	double d2 = vec3d->yCoord - yCoord;
	double d3 = vec3d->zCoord - zCoord;
	if (d1 * d1 < 1.0000000116860974E-007)
		return nullptr;
	double d4 = (d - xCoord) / d1;
	if (d4 < 0.0 || d4 > 1.0)
		return nullptr;
	else
		return createVector(xCoord + d1 * d4, yCoord + d2 * d4, zCoord + d3 * d4);
}

Vec3D *Vec3D::getIntermediateWithYValue(Vec3D *vec3d, double d)
{
	double d1 = vec3d->xCoord - xCoord;
	double d2 = vec3d->yCoord - yCoord;
	double d3 = vec3d->zCoord - zCoord;
	if (d2 * d2 < 1.0000000116860974E-007)
		return nullptr;
	double d4 = (d - yCoord) / d2;
	if (d4 < 0.0 || d4 > 1.0)
		return nullptr;
	else
		return createVector(xCoord + d1 * d4, yCoord + d2 * d4, zCoord + d3 * d4);
}

Vec3D *Vec3D::getIntermediateWithZValue(Vec3D *vec3d, double d)
{
	double d1 = vec3d->xCoord - xCoord;
	double d2 = vec3d->yCoord - yCoord;
	double d3 = vec3d->zCoord - zCoord;
	if (d3 * d3 < 1.0000000116860974E-007)
		return nullptr;
	double d4 = (d - zCoord) / d3;
	if (d4 < 0.0 || d4 > 1.0)
		return nullptr;
	else
		return createVector(xCoord + d1 * d4, yCoord + d2 * d4, zCoord + d3 * d4);
}

jstring Vec3D::toString()
{
	return "(" + String::toString(xCoord) + ", " + String::toString(yCoord) + ", " + String::toString(zCoord) + ")";
}

void Vec3D::rotateAroundX(float f)
{
	float f1 = MathHelper::cos(f);
	float f2 = MathHelper::sin(f);
	double d = xCoord;
	double d1 = yCoord * (double)f1 + zCoord * (double)f2;
	double d2 = zCoord * (double)f1 - yCoord * (double)f2;
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
}

void Vec3D::rotateAroundY(float f)
{
	float f1 = MathHelper::cos(f);
	float f2 = MathHelper::sin(f);
	double d = xCoord * (double)f1 + zCoord * (double)f2;
	double d1 = yCoord;
	double d2 = zCoord * (double)f1 - xCoord * (double)f2;
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
}
