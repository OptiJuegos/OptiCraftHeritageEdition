#include "StructureMineshaftStart.h"

#include "ComponentMineshaftRoom.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

StructureMineshaftStart::StructureMineshaftStart(
	World *world, Random &random, int_t chunkX, int_t chunkZ)
{
	const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 2);
	const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 2);
	auto *room = new ComponentMineshaftRoom(0, random, x, z);
	addComponent(room);
	room->buildComponent(room, components, random);
	updateBoundingBox();
	markAvailableHeight(world, random, 10);
}
