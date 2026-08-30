#pragma once

#include "java/Random.h"

class Entity;
class World;

// net.minecraft.src.Teleporter
class Teleporter
{
public:
	Teleporter();

	void placeInPortal(World *world, Entity *entity);
	bool placeInExistingPortal(World *world, Entity *entity);
	bool createPortal(World *world, Entity *entity);

private:
	Random field_4232_a;
};
