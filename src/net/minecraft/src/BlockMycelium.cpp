#include "BlockMycelium.h"

#include "Block.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "World.h"
#include "java/Random.h"

BlockMycelium::BlockMycelium(int_t id)
    : Block(id, Material::grassMaterial)
{
    blockIndexInTexture = 77;
    setTickOnLoad(true);
}

int_t BlockMycelium::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
    return side == 1 ? 78 : (side == 0 ? 2 : 77);
}

int_t BlockMycelium::getBlockTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side)
{
    if (side == 1)
        return 78;
    if (side == 0)
        return 2;

    Material *above = blockAccess->getBlockMaterial(x, y + 1, z);
    return above != Material::snow && above != Material::builtSnow ? 77 : 68;
}

void BlockMycelium::updateTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
    if (world->multiplayerWorld)
        return;

    const int_t aboveId = world->getBlockId(x, y + 1, z);
    if (world->getBlockLightValue(x, y + 1, z) < 4 && Block::lightOpacity[aboveId] > 2)
    {
        world->setBlockWithNotify(x, y, z, Block::dirt->blockID);
        return;
    }

    if (world->getBlockLightValue(x, y + 1, z) < 9)
        return;

    for (int_t i = 0; i < 4; ++i)
    {
        const int_t spreadX = x + random.nextInt(3) - 1;
        const int_t spreadY = y + random.nextInt(5) - 3;
        const int_t spreadZ = z + random.nextInt(3) - 1;
        const int_t spreadAboveId = world->getBlockId(spreadX, spreadY + 1, spreadZ);
        if (world->getBlockId(spreadX, spreadY, spreadZ) == Block::dirt->blockID &&
            world->getBlockLightValue(spreadX, spreadY + 1, spreadZ) >= 4 &&
            Block::lightOpacity[spreadAboveId] <= 2)
        {
            world->setBlockWithNotify(spreadX, spreadY, spreadZ, blockID);
        }
    }
}

void BlockMycelium::randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
    Block::randomDisplayTick(world, x, y, z, random);
    if (random.nextInt(10) == 0)
    {
        const double particleX = (double)((float)x + random.nextFloat());
        const double particleZ = (double)((float)z + random.nextFloat());
        world->spawnParticle("townaura", particleX, (double)((float)y + 1.1f), particleZ, 0.0, 0.0, 0.0);
    }
}

int_t BlockMycelium::idDropped(int_t metadata, Random &random)
{
    return Block::dirt->idDropped(0, random);
}

int_t BlockMycelium::idDropped(int_t metadata, Random &random, int_t fortune)
{
    (void)metadata;
    return Block::dirt->idDropped(0, random, fortune);
}
