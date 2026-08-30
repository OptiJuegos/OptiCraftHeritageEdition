#pragma once

#include "java/Type.h"

class BufferedImage;
class World;

// net.minecraft.src.IsoImageBuffer
class IsoImageBuffer
{
public:
	IsoImageBuffer(World *world, int_t i, int_t j);

	void setChunkPosition(int_t i, int_t j);
	void setWorldAndChunkPosition(World *world, int_t i, int_t j);

	BufferedImage *field_1348_a;
	World *worldObj;
	int_t field_1354_c;
	int_t field_1353_d;
	bool field_1352_e;
	bool field_1351_f;
	int_t field_1350_g;
	bool field_1349_h;
};
