#include "StructureStrongholdStart.h"

#include <vector>

#include "ComponentStrongholdStairs2.h"
#include "StructureComponent.h"
#include "StructureStrongholdPieces.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

StructureStrongholdStart::StructureStrongholdStart(
	World *world, Random &random, int_t chunkX, int_t chunkZ)
{
	StructureStrongholdPieces::prepareStructurePieces();
	ComponentStrongholdStairs2 *start = new ComponentStrongholdStairs2(
		0, random, JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 2),
		JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 2));
	components.push_back(start);
	start->buildComponent(start, components, random);

	std::vector<StructureComponent *> &pending = start->field_35037_b;
	while (!pending.empty())
	{
		const int_t index = random.nextInt(static_cast<int_t>(pending.size()));
		StructureComponent *component = pending[static_cast<std::size_t>(index)];
		pending.erase(pending.begin() + index);
		component->buildComponent(start, components, random);
	}

	updateBoundingBox();
	markAvailableHeight(world, random, 10);
}
