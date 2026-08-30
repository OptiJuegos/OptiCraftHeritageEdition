#include "ItemEnderEye.h"

#include "java/Arithmetic.h"

#include <memory>

#include "Block.h"
#include "BlockEndPortalFrame.h"
#include "ChunkPosition.h"
#include "Direction.h"
#include "EntityEnderEye.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "MovingObjectPosition.h"
#include "PlayerCapabilities.h"
#include "World.h"

ItemEnderEye::ItemEnderEye(int id)
    : Item(id)
{
}

bool ItemEnderEye::onItemUse(ItemStack *stack, EntityPlayer *player, World *world, int x, int y, int z, int)
{
    const int_t blockId = world->getBlockId(x, y, z);
    const int_t metadata = world->getBlockMetadata(x, y, z);
    if (!player->canPlayerEdit(x, y, z) || blockId != Block::endPortalFrame->blockID || BlockEndPortalFrame::isEnderEyeInserted(metadata))
        return false;

    if (world->multiplayerWorld)
        return true;

    world->setBlockMetadataWithNotify(x, y, z, metadata + 4);
    --stack->stackSize;

    for (int_t i = 0; i < 16; ++i)
    {
        const double px = static_cast<double>(static_cast<float>(x) + (5.0f + itemRand.nextFloat() * 6.0f) / 16.0f);
        const double py = static_cast<double>(static_cast<float>(y) + 13.0f / 16.0f);
        const double pz = static_cast<double>(static_cast<float>(z) + (5.0f + itemRand.nextFloat() * 6.0f) / 16.0f);
        world->spawnParticle("smoke", px, py, pz, 0.0, 0.0, 0.0);
    }

    const int_t frameDirection = metadata & 3;
    int_t firstFrame = 0;
    int_t lastFrame = 0;
    bool foundFrame = false;
    bool complete = true;
    const int_t axisDirection = Direction::enderEyeMetaToDirection[frameDirection];

    for (int_t offset = -2; offset <= 2; ++offset)
    {
        const int_t frameX = x + Direction::offsetX[axisDirection] * offset;
        const int_t frameZ = z + Direction::offsetZ[axisDirection] * offset;
        const int_t currentId = world->getBlockId(frameX, y, frameZ);
        if (currentId == Block::endPortalFrame->blockID)
        {
            const int_t currentMeta = world->getBlockMetadata(frameX, y, frameZ);
            if (!BlockEndPortalFrame::isEnderEyeInserted(currentMeta))
            {
                complete = false;
                break;
            }

            if (!foundFrame)
            {
                firstFrame = offset;
                lastFrame = offset;
                foundFrame = true;
            }
            else
            {
                lastFrame = offset;
            }
        }
    }

    if (complete && lastFrame == firstFrame + 2)
    {
        for (int_t offset = firstFrame; offset <= lastFrame; ++offset)
        {
            int_t frameX = x + Direction::offsetX[axisDirection] * offset;
            int_t frameZ = z + Direction::offsetZ[axisDirection] * offset;
            frameX += Direction::offsetX[frameDirection] * 4;
            frameZ += Direction::offsetZ[frameDirection] * 4;
            const int_t currentId = world->getBlockId(frameX, y, frameZ);
            const int_t currentMeta = world->getBlockMetadata(frameX, y, frameZ);
            if (currentId != Block::endPortalFrame->blockID || !BlockEndPortalFrame::isEnderEyeInserted(currentMeta))
            {
                complete = false;
                break;
            }
        }

        for (int_t offset = firstFrame - 1; complete && offset <= lastFrame + 1; offset += 4)
        {
            for (int_t side = 1; side <= 3; ++side)
            {
                int_t frameX = x + Direction::offsetX[axisDirection] * offset;
                int_t frameZ = z + Direction::offsetZ[axisDirection] * offset;
                frameX += Direction::offsetX[frameDirection] * side;
                frameZ += Direction::offsetZ[frameDirection] * side;
                const int_t currentId = world->getBlockId(frameX, y, frameZ);
                const int_t currentMeta = world->getBlockMetadata(frameX, y, frameZ);
                if (currentId != Block::endPortalFrame->blockID || !BlockEndPortalFrame::isEnderEyeInserted(currentMeta))
                {
                    complete = false;
                    break;
                }
            }
        }

        if (complete)
        {
            for (int_t offset = firstFrame; offset <= lastFrame; ++offset)
            {
                for (int_t side = 1; side <= 3; ++side)
                {
                    int_t portalX = x + Direction::offsetX[axisDirection] * offset;
                    int_t portalZ = z + Direction::offsetZ[axisDirection] * offset;
                    portalX += Direction::offsetX[frameDirection] * side;
                    portalZ += Direction::offsetZ[frameDirection] * side;
                    world->setBlockWithNotify(portalX, y, portalZ, Block::endPortal->blockID);
                }
            }
        }
    }

    return true;
}

ItemStack *ItemEnderEye::onItemRightClick(ItemStack *stack, World *world, EntityPlayer *player)
{
    std::unique_ptr<MovingObjectPosition> hit(getMovingObjectPositionFromPlayer(world, player, false));
    if (hit != nullptr && hit->typeOfHit == EnumMovingObjectType::TILE &&
        world->getBlockId(hit->blockX, hit->blockY, hit->blockZ) == Block::endPortalFrame->blockID)
    {
        return stack;
    }

    if (!world->multiplayerWorld)
    {
        std::unique_ptr<ChunkPosition> position(world->findClosestStructure("Stronghold",
            JavaArithmetic::doubleToInt(player->posX), JavaArithmetic::doubleToInt(player->posY), JavaArithmetic::doubleToInt(player->posZ)));
        if (position != nullptr)
        {
            EntityEnderEye *eye = new EntityEnderEye(world, player->posX,
                player->posY + 1.62 - static_cast<double>(player->yOffset), player->posZ);
            eye->moveTowards(static_cast<double>(position->x), position->y, static_cast<double>(position->z));
            bool spawned = world->spawnEntityInWorld(eye);
            world->playSoundAtEntity(player, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
            world->playAuxSFXAtEntity(nullptr, 1002, JavaArithmetic::doubleToInt(player->posX), JavaArithmetic::doubleToInt(player->posY), JavaArithmetic::doubleToInt(player->posZ), 0);
            if (!player->capabilities.isCreativeMode)
                --stack->stackSize;
            if (!spawned)
                delete eye;
        }
    }
    return stack;
}
