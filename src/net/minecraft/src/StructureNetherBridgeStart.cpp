#include "StructureNetherBridgeStart.h"

#include "ComponentNetherBridgeStartPiece.h"
#include "StructureComponent.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

StructureNetherBridgeStart::StructureNetherBridgeStart(
	World *world, Random &random, int_t chunkX, int_t chunkZ)
{
	auto *start = new ComponentNetherBridgeStartPiece(
		random, JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 2),
		JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 2));
	components.push_back(start);
	start->buildComponent(start, components, random);

	while (!start->pendingChildren.empty())
	{
		const int_t index = random.nextInt(static_cast<int_t>(start->pendingChildren.size()));
		StructureComponent *component = start->pendingChildren[static_cast<std::size_t>(index)];
		start->pendingChildren.erase(start->pendingChildren.begin() + index);
		if (component != nullptr)
			component->buildComponent(start, components, random);
	}

	updateBoundingBox();
	setRandomHeight(world, random, 48, 70);
}
