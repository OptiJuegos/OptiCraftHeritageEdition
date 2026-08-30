#include "ActiveRenderInfo.h"

#include "Block.h"
#include "BlockFluid.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "Material.h"
#include "Vec3D.h"
#include "World.h"
#include "platform/RenderAPI.h"

#include <cmath>

namespace
{
void multiplyMatrix(const float* a, const float* b, float* out)
{
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            out[column * 4 + row] =
                a[0 * 4 + row] * b[column * 4 + 0] +
                a[1 * 4 + row] * b[column * 4 + 1] +
                a[2 * 4 + row] * b[column * 4 + 2] +
                a[3 * 4 + row] * b[column * 4 + 3];
        }
    }
}

bool invertMatrix(const float* m, float* out)
{
    float inv[16];
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15]
        + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15]
        - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15]
        + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14]
        - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15]
        - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15]
        + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15]
        - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14]
        + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15]
        + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15]
        - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15]
        + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14]
        - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11]
        - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11]
        + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11]
        - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10]
        + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    const float determinant = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (std::fabs(determinant) < 1.0e-8f)
        return false;
    const float inverseDeterminant = 1.0f / determinant;
    for (int i = 0; i < 16; ++i)
        out[i] = inv[i] * inverseDeterminant;
    return true;
}

bool unProject(float windowX, float windowY, float windowZ, const float* modelView,
               const float* projection, const int* viewport, float* object)
{
    if (viewport[2] <= 0 || viewport[3] <= 0)
        return false;
    float combined[16];
    float inverse[16];
    multiplyMatrix(projection, modelView, combined);
    if (!invertMatrix(combined, inverse))
        return false;

    const float input[4] = {
        (windowX - static_cast<float>(viewport[0])) / static_cast<float>(viewport[2]) * 2.0f - 1.0f,
        (windowY - static_cast<float>(viewport[1])) / static_cast<float>(viewport[3]) * 2.0f - 1.0f,
        windowZ * 2.0f - 1.0f,
        1.0f
    };
    float output[4] = {};
    for (int row = 0; row < 4; ++row)
        output[row] = inverse[row] * input[0] + inverse[4 + row] * input[1]
            + inverse[8 + row] * input[2] + inverse[12 + row] * input[3];
    if (std::fabs(output[3]) < 1.0e-8f)
        return false;
    object[0] = output[0] / output[3];
    object[1] = output[1] / output[3];
    object[2] = output[2] / output[3];
    return true;
}
}

float ActiveRenderInfo::objectX = 0.0f;
float ActiveRenderInfo::objectY = 0.0f;
float ActiveRenderInfo::objectZ = 0.0f;
float ActiveRenderInfo::rotationX = 0.0f;
float ActiveRenderInfo::rotationXZ = 0.0f;
float ActiveRenderInfo::rotationZ = 0.0f;
float ActiveRenderInfo::rotationYZ = 0.0f;
float ActiveRenderInfo::rotationXY = 0.0f;

void ActiveRenderInfo::updateRenderInfo(EntityPlayer* player, bool reverseView)
{
    if (player == nullptr)
        return;
    float modelView[16];
    float projection[16];
    int viewport[4];
    renderGetMatrix(RenderMatrixQuery::ModelView, modelView);
    renderGetMatrix(RenderMatrixQuery::Projection, projection);
    renderGetViewport(viewport);
    float object[3] = {};
    const float centerX = static_cast<float>((viewport[0] + viewport[2]) / 2);
    const float centerY = static_cast<float>((viewport[1] + viewport[3]) / 2);
    if (unProject(centerX, centerY, 0.0f, modelView, projection, viewport, object))
    {
        objectX = object[0];
        objectY = object[1];
        objectZ = object[2];
    }

    const float sign = reverseView ? -1.0f : 1.0f;
    const float pitch = player->rotationPitch * 3.14159265358979323846f / 180.0f;
    const float yaw = player->rotationYaw * 3.14159265358979323846f / 180.0f;
    rotationX = MathHelper::cos(yaw) * sign;
    rotationZ = MathHelper::sin(yaw) * sign;
    rotationYZ = -rotationZ * MathHelper::sin(pitch) * sign;
    rotationXY = rotationX * MathHelper::sin(pitch) * sign;
    rotationXZ = MathHelper::cos(pitch);
}

Vec3D* ActiveRenderInfo::projectViewFromEntity(EntityLiving* entity, double partialTick)
{
    if (entity == nullptr)
        return Vec3D::createVector(0.0, 0.0, 0.0);
    const double x = entity->prevPosX + (entity->posX - entity->prevPosX) * partialTick;
    const double y = entity->prevPosY + (entity->posY - entity->prevPosY) * partialTick + entity->getEyeHeight();
    const double z = entity->prevPosZ + (entity->posZ - entity->prevPosZ) * partialTick;
    return Vec3D::createVector(x + objectX, y + objectY, z + objectZ);
}

int ActiveRenderInfo::getBlockIdAtEntityViewpoint(World* world, EntityLiving* entity, float partialTick)
{
    if (world == nullptr || entity == nullptr)
        return 0;
    Vec3D* view = projectViewFromEntity(entity, partialTick);
    const int x = MathHelper::floor_double(view->xCoord);
    const int y = MathHelper::floor_double(view->yCoord);
    const int z = MathHelper::floor_double(view->zCoord);
    int blockId = world->getBlockId(x, y, z);
    if (blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[blockId] != nullptr
        && Block::blocksList[blockId]->blockMaterial->getIsLiquid())
    {
        const float fluidHeight = BlockFluid::getFluidHeightPercent(world->getBlockMetadata(x, y, z)) - 1.0f / 9.0f;
        const float surface = static_cast<float>(y + 1) - fluidHeight;
        if (view->yCoord >= static_cast<double>(surface))
            blockId = world->getBlockId(x, y + 1, z);
    }
    // projectViewFromEntity() hands back a Vec3D::createVector() slot, which is
    // either an element of the static Vec3D::pool array or an entry owned by the
    // overflow pool's unique_ptr. Neither is a heap block this function may free:
    // deleting the former passed a .bss address to RtlFreeHeap (STATUS_HEAP_CORRUPTION
    // on Windows), and deleting the latter double-freed it when the pool was trimmed.
    // Vec3D::initialize() reclaims the slot.
    return blockId;
}
