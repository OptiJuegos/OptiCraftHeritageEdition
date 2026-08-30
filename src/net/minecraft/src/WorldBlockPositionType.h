#pragma once

#include "java/Type.h"

class WorldClient;

// net.minecraft.src.WorldBlockPositionType
class WorldBlockPositionType
{
public:
	WorldBlockPositionType(WorldClient *worldclient, int_t i, int_t j, int_t k, int_t l, int_t i1)
		: field_1203_g(worldclient), field_1202_a(i), field_1201_b(j),
		  field_1207_c(k), field_1206_d(80), field_1205_e(l), field_1204_f(i1) {}

	int_t field_1202_a;
	int_t field_1201_b;
	int_t field_1207_c;
	int_t field_1206_d;
	int_t field_1205_e;
	int_t field_1204_f;
	WorldClient *const field_1203_g;
};
