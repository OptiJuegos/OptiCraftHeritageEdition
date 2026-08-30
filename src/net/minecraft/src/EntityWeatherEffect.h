#pragma once

#include "Entity.h"

class World;

// net.minecraft.src.EntityWeatherEffect
class EntityWeatherEffect : public Entity
{
public:
	EntityWeatherEffect(World *world) : Entity(world) {}
};
