#include "IsoImageBuffer.h"

IsoImageBuffer::IsoImageBuffer(World *world, int_t i, int_t j)
	: field_1348_a(nullptr)
	, worldObj(world)
	, field_1354_c(0)
	, field_1353_d(0)
	, field_1352_e(false)
	, field_1351_f(false)
	, field_1350_g(0)
	, field_1349_h(false)
{
	setChunkPosition(i, j);
}

void IsoImageBuffer::setChunkPosition(int_t i, int_t j)
{
	field_1352_e = false;
	field_1354_c = i;
	field_1353_d = j;
	field_1350_g = 0;
	field_1349_h = false;
}

void IsoImageBuffer::setWorldAndChunkPosition(World *world, int_t i, int_t j)
{
	worldObj = world;
	setChunkPosition(i, j);
}
