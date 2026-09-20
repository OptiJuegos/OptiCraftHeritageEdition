#include "platform/Log.h"
#include "NetClientHandler.h"
#include "java/Arithmetic.h"
#include <algorithm>
#include <iostream>
#include <cctype>
#include <stdexcept>
#include "java/String.h"
#include "java/JavaNetwork.h"
#include "NetHandler.h"
#include "Minecraft.h"
#include "MapStorage.h"
#include "NetworkManager.h"
#include "WorldClient.h"
#include "WorldHeight.h"
#include "WorldProviderSurface.h"
#include "PlayerControllerMP.h"
#include "StatList.h"
#include "StatFileWriter.h"
#include "GuiDownloadTerrain.h"
#include "EntityItem.h"
#include "EntityXPOrb.h"
#include "ItemStack.h"
#include "EntityMinecart.h"
#include "EntityFish.h"
#include "EntityArrow.h"
#include "EntitySnowball.h"
#include "EntityFireball.h"
#include "EntityEgg.h"
#include "EntityEnderPearl.h"
#include "EntityEnderEye.h"
#include "EntitySmallFireball.h"
#include "EntityPotion.h"
#include "EntityExpBottle.h"
#include "EntityEnderCrystal.h"
#include "EntityBoat.h"
#include "EntityTNTPrimed.h"
#include "EntityFallingSand.h"
#include "Block.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityDragon.h"
#include "EntityDragonPart.h"
#include "EntityLightningBolt.h"
#include "EntityPainting.h"
#include "EntityOtherPlayerMP.h"
#include "InventoryPlayer.h"
#include "EntityPlayer.h"
#include "FoodStats.h"
#include "EntityPlayerSP.h"
#include "EntityClientPlayerMP.h"
#include "Chunk.h"
#include "GuiConnectFailed.h"
#include "EntityPickupFX.h"
#include "EntityCrit2FX.h"
#include "EffectRenderer.h"
#include "GuiIngame.h"
#include "Session.h"
#include "EntityList.h"
#include "ChunkCoordinates.h"
#include "WorldInfo.h"
#include "WorldSettings.h"
#include "WorldType.h"
#include "Explosion.h"
#include "InventoryBasic.h"
#include "Container.h"
#include "TileEntityFurnace.h"
#include "TileEntityDispenser.h"
#include "TileEntityBrewingStand.h"
#include "MathHelper.h"
#include "Slot.h"
#include "TileEntitySign.h"
#include "TileEntity.h"
#include "TileEntityMobSpawner.h"
#include "ItemMap.h"
#include "Item.h"
#include "MapData.h"
#include "StatList.h"
#include "DataWatcher.h"
#include "PotionEffect.h"
#include "GuiPlayerInfo.h"
#include "ChunkPosition.h"
#include "AxisAlignedBB.h"
#include "GuiWinGame.h"

// Packets
#include "Packet0KeepAlive.h"
#include "Packet.h"
#include "Packet1Login.h"
#include "Packet21PickupSpawn.h"
#include "Packet23VehicleSpawn.h"
#include "Packet71Weather.h"
#include "Packet25EntityPainting.h"
#include "Packet26EntityExpOrb.h"
#include "Packet28EntityVelocity.h"
#include "Packet40EntityMetadata.h"
#include "Packet41EntityEffect.h"
#include "Packet42RemoveEntityEffect.h"
#include "Packet43Experience.h"
#include "Packet20NamedEntitySpawn.h"
#include "Packet34EntityTeleport.h"
#include "Packet35EntityHeadRotation.h"
#include "Packet30Entity.h"
#include "Packet29DestroyEntity.h"
#include "Packet10Flying.h"
#include "Packet13PlayerLookMove.h"
#include "Packet50PreChunk.h"
#include "Packet52MultiBlockChange.h"
#include "Packet51MapChunk.h"
#include "Packet53BlockChange.h"
#include "Packet255KickDisconnect.h"
#include "Packet22Collect.h"
#include "Packet3Chat.h"
#include "Packet18Animation.h"
#include "Packet17Sleep.h"
#include "Packet2Handshake.h"
#include "Packet24MobSpawn.h"
#include "Packet4UpdateTime.h"
#include "Packet6SpawnPosition.h"
#include "Packet39AttachEntity.h"
#include "Packet38EntityStatus.h"
#include "Packet8UpdateHealth.h"
#include "Packet9Respawn.h"
#include "Packet60Explosion.h"
#include "Packet100OpenWindow.h"
#include "Packet103SetSlot.h"
#include "Packet106Transaction.h"
#include "Packet104WindowItems.h"
#include "Packet130UpdateSign.h"
#include "Packet105UpdateProgressbar.h"
#include "Packet5PlayerInventory.h"
#include "Packet101CloseWindow.h"
#include "Packet54PlayNoteBlock.h"
#include "Packet70Bed.h"
#include "Packet131MapData.h"
#include "Packet132TileEntityData.h"
#include "Packet61DoorChange.h"
#include "Packet200Statistic.h"
#include "Packet201PlayerInfo.h"
#include "Packet202PlayerAbilities.h"

// Networking uses the platform socket implementation selected by NetworkManager.

NetClientHandler::NetClientHandler(Minecraft* minecraft, const std::string& host, int port)
{
    disconnected = false;
    terrainDownloaded = false;
    netManager = nullptr;
    mapStorage = new MapStorage(nullptr);
    
    mc = minecraft;
    serverHostname = host;
    
    try
    {
        netManager = new NetworkManager(host, port, "Client", this);
    }
    catch (...)
    {
        delete mapStorage;
        mapStorage = nullptr;
        throw;
    }
}

NetClientHandler::~NetClientHandler()
{
    disconnected = true;
    delete netManager;
    netManager = nullptr;
    delete mapStorage;
    mapStorage = nullptr;
}

void NetClientHandler::processReadPackets()
{
    if (!disconnected)
    {
        netManager->processReadPackets();
    }
    netManager->wakeThreads();
}

void NetClientHandler::handleLogin(Packet1Login* packet)
{
    delete mc->playerController;
    mc->playerController = new PlayerControllerMP(mc, this);
    playerControllerOwnsHandler = true;
    mc->statFileWriter->readStat(StatList::joinMultiplayerStat, 1);
    
    WorldType::initialize();
    WorldSettings settings(0LL, packet->serverMode, false, false, packet->terrainType != nullptr ? packet->terrainType : WorldType::DEFAULT);
    worldClient = new WorldClient(this, settings, packet->dimension, packet->difficultySetting);
    worldClient->multiplayerWorld = true;
    
    mc->changeWorld1(worldClient);
    mc->thePlayer->dimension = packet->dimension;
    mc->displayGuiScreen(new GuiDownloadTerrain(this));
    mc->thePlayer->entityId = packet->protocolVersion;
    currentServerMaxPlayers = static_cast<int_t>(static_cast<unsigned char>(packet->maxPlayers));
    static_cast<PlayerControllerMP *>(mc->playerController)->setCreative(packet->serverMode == 1);
}

void NetClientHandler::handlePickupSpawn(Packet21PickupSpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    EntityItem* entityItem = new EntityItem(
        worldClient, posX, posY, posZ,
        new ItemStack(packet->itemID, packet->count, packet->itemDamage)
    );
    
    entityItem->motionX = (double)packet->rotation / 128.0;
    entityItem->motionY = (double)packet->pitch / 128.0;
    entityItem->motionZ = (double)packet->roll / 128.0;
    
    entityItem->serverPosX = packet->xPosition;
    entityItem->serverPosY = packet->yPosition;
    entityItem->serverPosZ = packet->zPosition;
    
    worldClient->addEntityToWorld(packet->entityId, entityItem);
}

void NetClientHandler::handleVehicleSpawn(Packet23VehicleSpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    Entity* entity = nullptr;
    
    if (packet->type == 10)
    {
        entity = new EntityMinecart(worldClient, posX, posY, posZ, 0);
    }
    else if (packet->type == 11)
    {
        entity = new EntityMinecart(worldClient, posX, posY, posZ, 1);
    }
    else if (packet->type == 12)
    {
        entity = new EntityMinecart(worldClient, posX, posY, posZ, 2);
    }
    else if (packet->type == 90)
    {
        entity = new EntityFish(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 60)
    {
        entity = new EntityArrow(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 61)
    {
        entity = new EntitySnowball(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 65)
    {
        entity = new EntityEnderPearl(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 72)
    {
        entity = new EntityEnderEye(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 63)
    {
        entity = new EntityFireball(
            worldClient, posX, posY, posZ,
            (double)packet->speedX / 8000.0,
            (double)packet->speedY / 8000.0,
            (double)packet->speedZ / 8000.0
        );
        packet->throwerEntityId = 0;
    }
    else if (packet->type == 64)
    {
        entity = new EntitySmallFireball(
            worldClient, posX, posY, posZ,
            (double)packet->speedX / 8000.0,
            (double)packet->speedY / 8000.0,
            (double)packet->speedZ / 8000.0
        );
        packet->throwerEntityId = 0;
    }
    else if (packet->type == 62)
    {
        entity = new EntityEgg(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 73)
    {
        entity = new EntityPotion(worldClient, posX, posY, posZ, packet->throwerEntityId);
        packet->throwerEntityId = 0;
    }
    else if (packet->type == 75)
    {
        entity = new EntityExpBottle(worldClient, posX, posY, posZ);
        packet->throwerEntityId = 0;
    }
    else if (packet->type == 1)
    {
        entity = new EntityBoat(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 50)
    {
        entity = new EntityTNTPrimed(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 51)
    {
        entity = new EntityEnderCrystal(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 70)
    {
        entity = new EntityFallingSand(worldClient, posX, posY, posZ, Block::sand->blockID);
    }
    else if (packet->type == 71)
    {
        entity = new EntityFallingSand(worldClient, posX, posY, posZ, Block::gravel->blockID);
    }
    else if (packet->type == 74)
    {
        entity = new EntityFallingSand(worldClient, posX, posY, posZ, Block::dragonEgg->blockID);
    }
    
    if (entity != nullptr)
    {
        entity->serverPosX = packet->xPosition;
        entity->serverPosY = packet->yPosition;
        entity->serverPosZ = packet->zPosition;
        entity->rotationYaw = 0.0f;
        entity->rotationPitch = 0.0f;

        const std::vector<Entity *> parts = entity->getParts();
        if (!parts.empty())
        {
            const int_t idOffset = JavaArithmetic::intSub(packet->entityId, entity->entityId);
            for (Entity *part : parts)
            {
                if (part != nullptr)
                    part->entityId = JavaArithmetic::intAdd(part->entityId, idOffset);
            }
        }

        entity->entityId = packet->entityId;
        worldClient->addEntityToWorld(packet->entityId, entity);

        if (packet->throwerEntityId > 0)
        {
            if (packet->type == 60)
            {
                Entity* thrower = getEntityByID(packet->throwerEntityId);
                EntityLiving* throwerLiving = (thrower && thrower->isLiving()) ? static_cast<EntityLiving*>(thrower) : nullptr;
                if (throwerLiving != nullptr)
                {
                    EntityArrow* arrow = (entity->getEntityClassID() == EntityArrow::CLASS_ID) ? static_cast<EntityArrow*>(entity) : nullptr;
                    if (arrow != nullptr)
                    {
                        arrow->setShootingEntity(throwerLiving);
                    }
                }
            }

            entity->setVelocity(
                (double)packet->speedX / 8000.0,
                (double)packet->speedY / 8000.0,
                (double)packet->speedZ / 8000.0
            );
        }
    }
}

void NetClientHandler::handleWeather(Packet71Weather* packet)
{
    double posX = (double)packet->posX / 32.0;
    double posY = (double)packet->posY / 32.0;
    double posZ = (double)packet->posZ / 32.0;
    
    EntityLightningBolt* lightningBolt = nullptr;
    
    if (packet->weatherType == 1)
    {
        lightningBolt = new EntityLightningBolt(worldClient, posX, posY, posZ);
    }
    
    if (lightningBolt != nullptr)
    {
        lightningBolt->serverPosX = packet->posX;
        lightningBolt->serverPosY = packet->posY;
        lightningBolt->serverPosZ = packet->posZ;
        lightningBolt->rotationYaw = 0.0f;
        lightningBolt->rotationPitch = 0.0f;
        lightningBolt->entityId = packet->entityId;
        
        worldClient->addWeatherEffect(lightningBolt);
    }
}

void NetClientHandler::handlePaintingSpawn(Packet25EntityPainting* packet)
{
    EntityPainting* painting = new EntityPainting(
        worldClient,
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->direction,
        packet->title
    );
    
    worldClient->addEntityToWorld(packet->entityId, painting);
}

void NetClientHandler::handleEntityExpOrb(Packet26EntityExpOrb* packet)
{
    if (packet == nullptr || worldClient == nullptr)
        return;

    EntityXPOrb *orb = new EntityXPOrb(worldClient, static_cast<double>(packet->posX), static_cast<double>(packet->posY), static_cast<double>(packet->posZ), packet->xpValue);
    orb->serverPosX = packet->posX;
    orb->serverPosY = packet->posY;
    orb->serverPosZ = packet->posZ;
    orb->rotationYaw = 0.0f;
    orb->rotationPitch = 0.0f;
    orb->entityId = packet->entityId;
    worldClient->addEntityToWorld(packet->entityId, orb);
}

void NetClientHandler::handleEntityVelocity(Packet28EntityVelocity* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->setVelocity(
        (double)packet->motionX / 8000.0,
        (double)packet->motionY / 8000.0,
        (double)packet->motionZ / 8000.0
    );
}

void NetClientHandler::handleEntityMetadata(Packet40EntityMetadata* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity != nullptr)
    {
        auto metaList = packet->getMetadata();
        if (!metaList.empty())
            entity->getDataWatcher()->updateWatchedObjectsFromList(metaList);
    }
}

void NetClientHandler::handleNamedEntitySpawn(Packet20NamedEntitySpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    float yaw = (float)(packet->rotation * 360) / 256.0f;
    float pitch = (float)(packet->pitch * 360) / 256.0f;
    
    EntityOtherPlayerMP* otherPlayer = new EntityOtherPlayerMP(mc->theWorld, packet->name);
    
    otherPlayer->prevPosX = otherPlayer->lastTickPosX = otherPlayer->serverPosX = packet->xPosition;
    otherPlayer->prevPosY = otherPlayer->lastTickPosY = otherPlayer->serverPosY = packet->yPosition;
    otherPlayer->prevPosZ = otherPlayer->lastTickPosZ = otherPlayer->serverPosZ = packet->zPosition;
    
    int currentItem = packet->currentItem;
    
    if (currentItem == 0)
    {
        otherPlayer->inventory->mainInventory[otherPlayer->inventory->currentItem] = nullptr;
    }
    else
    {
        otherPlayer->inventory->mainInventory[otherPlayer->inventory->currentItem] = new ItemStack(currentItem, 1, 0);
    }
    
    otherPlayer->setPositionAndRotation(posX, posY, posZ, yaw, pitch);
    worldClient->addEntityToWorld(packet->entityId, otherPlayer);
}

void NetClientHandler::handleEntityTeleport(Packet34EntityTeleport* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->serverPosX = packet->xPosition;
    entity->serverPosY = packet->yPosition;
    entity->serverPosZ = packet->zPosition;
    
    double posX = (double)entity->serverPosX / 32.0;
    double posY = (double)entity->serverPosY / 32.0 + 0.015625;
    double posZ = (double)entity->serverPosZ / 32.0;
    
    float yaw = (float)(packet->yaw * 360) / 256.0f;
    float pitch = (float)(packet->pitch * 360) / 256.0f;
    
    entity->setPositionAndRotation2(posX, posY, posZ, yaw, pitch, 3);
}

void NetClientHandler::handleEntityMovement(Packet30Entity* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->serverPosX += packet->xPosition;
    entity->serverPosY += packet->yPosition;
    entity->serverPosZ += packet->zPosition;
    
    double posX = (double)entity->serverPosX / 32.0;
    double posY = (double)entity->serverPosY / 32.0;
    double posZ = (double)entity->serverPosZ / 32.0;
    
    float yaw = packet->rotating ? (float)(packet->yaw * 360) / 256.0f : entity->rotationYaw;
    float pitch = packet->rotating ? (float)(packet->pitch * 360) / 256.0f : entity->rotationPitch;
    
    entity->setPositionAndRotation2(posX, posY, posZ, yaw, pitch, 3);
}

void NetClientHandler::handleEntityHeadRotation(Packet35EntityHeadRotation* packet)
{
    if (packet == nullptr)
        return;
    Entity *entity = getEntityByID(packet->entityId);
    if (entity != nullptr)
        entity->func_48079_f(static_cast<float>(packet->headRotationYaw) * 360.0f / 256.0f);
}

void NetClientHandler::handleDestroyEntity(Packet29DestroyEntity* packet)
{
    worldClient->removeEntityFromWorld(packet->entityId);
}

void NetClientHandler::handleFlying(Packet10Flying* packet)
{
    EntityPlayerSP* playerSP = mc->thePlayer;
    EntityPlayer* player = playerSP;
    
    double posX = player->posX;
    double posY = player->posY;
    double posZ = player->posZ;
    float yaw = player->rotationYaw;
    float pitch = player->rotationPitch;
    
    if (packet->moving)
    {
        posX = packet->xPosition;
        posY = packet->yPosition;
        posZ = packet->zPosition;
    }
    
    if (packet->rotating)
    {
        yaw = packet->yaw;
        pitch = packet->pitch;
    }
    
    playerSP->ySize = 0.0f;
    playerSP->motionX = playerSP->motionY = playerSP->motionZ = 0.0;
    playerSP->setPositionAndRotation(posX, posY, posZ, yaw, pitch);
    worldClient->prioritizePlayerChunk(
        JavaArithmetic::intShr(MathHelper::floor_double(posX), 4),
        JavaArithmetic::intShr(MathHelper::floor_double(posZ), 4));

    // Java re-sends the received packet object back to the server. In C++ that
    // packet is owned by processReadPackets' unique_ptr (which deletes it after
    // this call), and addToSendQueue takes ownership too -> double free / crash
    // when deleting packet id 13. Send a fresh copy instead and let the received
    // packet be freed normally.
    netManager->addToSendQueue(new Packet13PlayerLookMove(
        player->posX,
        player->boundingBox->minY,
        player->posY,            // stance
        player->posZ,
        yaw,
        pitch,
        packet->onGround));
    
    if (!terrainDownloaded)
    {
        mc->thePlayer->prevPosX = mc->thePlayer->posX;
        mc->thePlayer->prevPosY = mc->thePlayer->posY;
        mc->thePlayer->prevPosZ = mc->thePlayer->posZ;
        terrainDownloaded = true;
        mc->displayGuiScreen(nullptr);
    }
}

void NetClientHandler::handlePreChunk(Packet50PreChunk* packet)
{
    if (packet->mode)
        preChunkLoadCount++;
    else
        preChunkUnloadCount++;

    worldClient->doPreChunk(packet->xPosition, packet->yPosition, packet->mode);
}

void NetClientHandler::handleMultiBlockChange(Packet52MultiBlockChange* packet)
{
    if (packet == nullptr || packet->metadataArray.size() < (size_t)packet->size * 4)
        return;

    const int_t baseX = JavaArithmetic::intMul(packet->xPosition, 16);
    const int_t baseZ = JavaArithmetic::intMul(packet->zPosition, 16);

#if PLATFORM_MP_DEFERRED_CHUNKS
    const bool keepChunk = worldClient->shouldKeepChunk(packet->xPosition, packet->zPosition);
#endif

    for (int_t i = 0; i < packet->size; ++i)
    {
        const size_t offset = (size_t)i * 4;
        const ushort_t coordinate = (ushort_t)(((ubyte_t)packet->metadataArray[offset] << 8) |
                                                (ubyte_t)packet->metadataArray[offset + 1]);
        const ushort_t blockData = (ushort_t)(((ubyte_t)packet->metadataArray[offset + 2] << 8) |
                                               (ubyte_t)packet->metadataArray[offset + 3]);

        const int_t localX = (coordinate >> 12) & 0xf;
        const int_t localZ = (coordinate >> 8) & 0xf;
        const int_t y = coordinate & 0xff;
        const int_t blockId = (blockData & 0x0fff) >> 4;
        const int_t metadata = blockData & 0xf;

#if PLATFORM_MP_DEFERRED_CHUNKS
        worldClient->deferBlockChange(JavaArithmetic::intAdd(baseX, localX), y, JavaArithmetic::intAdd(baseZ, localZ), blockId, metadata);
        if (!keepChunk)
            continue;
#endif

        worldClient->setBlockAndMetadataAndInvalidate(JavaArithmetic::intAdd(baseX, localX), y, JavaArithmetic::intAdd(baseZ, localZ), blockId, metadata);
    }
}

void NetClientHandler::handleMapChunk(Packet51MapChunk* packet)
{
    mapChunkCount++;
#if PLATFORM_MP_DEFERRED_CHUNKS
    // Keep the initialize packet plus subsequent section deltas compressed for
    // bounded console caches. This lets an evicted 1.2.5 column be reconstructed
    // without asking the server to resend a chunk it still considers loaded.
    // Move every payload directly into the cache. Nearby data is inflated first,
    // so it can still be applied below without retaining a second compressed copy.
    const bool keepChunk = worldClient->shouldKeepChunk(packet->xCh, packet->zCh);
    if (keepChunk && !packet->ensureDecompressed())
    {
        netManager->networkShutdown("disconnect.genericReason", {"Invalid compressed chunk data"});
        return;
    }
    worldClient->cacheCompressedChunk(
        packet->xCh, packet->zCh, packet->includeInitialize,
        packet->yChMin, packet->yChMax, packet->takeCompressedData());
    if (!keepChunk)
        return;
#endif

    worldClient->invalidateBlockReceiveRegion(
        JavaArithmetic::intShl(packet->xCh, 4), 0, JavaArithmetic::intShl(packet->zCh, 4),
        JavaArithmetic::intAdd(JavaArithmetic::intShl(packet->xCh, 4), 15), WorldHeight::HEIGHT,
        JavaArithmetic::intAdd(JavaArithmetic::intShl(packet->zCh, 4), 15));

    Chunk *chunk = worldClient->getChunkFromChunkCoords(packet->xCh, packet->zCh);

    // Packet50PreChunk normally creates the client chunk before the map data
    // arrives. Match Java 1.2.5 and only materialize a column here when the
    // provider is still returning its EmptyChunk fallback; replacing an already
    // loaded chunk would discard entities/tile entities attached to it.
    if (packet->includeInitialize && (chunk == nullptr || chunk->isEmptyChunk()))
    {
        worldClient->doPreChunk(packet->xCh, packet->zCh, true);
        chunk = worldClient->getChunkFromChunkCoords(packet->xCh, packet->zCh);
    }

    if (chunk == nullptr || chunk->isEmptyChunk())
        return;

    if (!packet->ensureDecompressed() ||
        !chunk->func_48494_a(packet->chunkData.data(), packet->chunkData.size(),
                             packet->yChMin, packet->yChMax, packet->includeInitialize))
    {
        netManager->networkShutdown("disconnect.genericReason", {"Invalid compressed chunk data"});
        return;
    }

    worldClient->markBlocksDirty(
        JavaArithmetic::intShl(packet->xCh, 4), 0, JavaArithmetic::intShl(packet->zCh, 4),
        JavaArithmetic::intAdd(JavaArithmetic::intShl(packet->xCh, 4), 15), WorldHeight::HEIGHT,
        JavaArithmetic::intAdd(JavaArithmetic::intShl(packet->zCh, 4), 15));

    if (!packet->includeInitialize || dynamic_cast<WorldProviderSurface *>(worldClient->worldProvider) == nullptr)
        chunk->resetRelightChecks();
}

void NetClientHandler::handleBlockChange(Packet53BlockChange* packet)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
	worldClient->deferBlockChange(packet->xPosition, packet->yPosition,
		packet->zPosition, packet->type, packet->metadata);
	if (!worldClient->shouldKeepChunk(JavaArithmetic::intShr(packet->xPosition, 4), JavaArithmetic::intShr(packet->zPosition, 4)))
		return;
#endif
    worldClient->setBlockAndMetadataAndInvalidate(
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->type,
        packet->metadata
    );
}

void NetClientHandler::handleKickDisconnect(Packet255KickDisconnect* packet)
{
    netManager->networkShutdown("disconnect.kicked", {});
    disconnected = true;
    
    mc->changeWorld1(nullptr);
    mc->displayGuiScreen(new GuiConnectFailed(
        "disconnect.disconnected",
        "disconnect.genericReason",
        packet->reason
    ));
}

void NetClientHandler::handleErrorMessage(const std::string& message, const std::vector<std::string>& args)
{
    if (disconnected)
    {
        return;
    }
    
    disconnected = true;
    mc->changeWorld1(nullptr);
    mc->displayGuiScreen(new GuiConnectFailed("disconnect.lost", message, args.empty() ? std::string() : args[0]));
}

void NetClientHandler::quitWithPacket(Packet* packet)
{
    if (disconnected)
    {
        delete packet;
        return;
    }

    netManager->addToSendQueue(packet);
    netManager->serverShutdown();
}

void NetClientHandler::sendPacketAndFlush(Packet* packet)
{
    quitWithPacket(packet);
}

void NetClientHandler::addToSendQueue(Packet* packet)
{
    if (disconnected)
    {
        delete packet;
        return;
    }
    
    netManager->addToSendQueue(packet);
}

void NetClientHandler::handleCollect(Packet22Collect* packet)
{
    Entity* collected = getEntityByID(packet->collectedEntityId);
    Entity* collectorRaw = getEntityByID(packet->collectorEntityId);
    EntityLiving* collector = (collectorRaw && collectorRaw->isLiving()) ? static_cast<EntityLiving*>(collectorRaw) : nullptr;
    
    if (collector == nullptr)
    {
        collector = mc->thePlayer;
    }
    
    if (collected != nullptr)
    {
        const char *pickupSound = dynamic_cast<EntityXPOrb *>(collected) != nullptr ? "random.orb" : "random.pop";
        worldClient->playSoundAtEntity(collected, pickupSound, 0.2f,
            (rand.nextFloatDifference() * 0.7f + 1.0f) * 2.0f);
        
        mc->effectRenderer->addEffect(new EntityPickupFX(
            mc->theWorld, collected, collector, -0.5f
        ));
        
        worldClient->removeEntityFromWorld(packet->collectedEntityId);
    }
}

void NetClientHandler::handleChat(Packet3Chat* packet)
{
    mc->ingameGUI->addChatMessage(packet->message);
}

void NetClientHandler::handleArmAnimation(Packet18Animation* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    if (packet->animate == 1)
    {
        EntityPlayer* player = entity->isPlayer() ? static_cast<EntityPlayer*>(entity) : nullptr;
        if (player != nullptr)
        {
            player->swingItem();
        }
    }
    else if (packet->animate == 2)
    {
        entity->performHurtAnimation();
    }
    else if (packet->animate == 3)
    {
        EntityPlayer* player = entity->isPlayer() ? static_cast<EntityPlayer*>(entity) : nullptr;
        if (player != nullptr)
        {
            player->wakeUpPlayer(false, false, false);
        }
    }
    else if (packet->animate == 4)
    {
        EntityPlayer* player = entity->isPlayer() ? static_cast<EntityPlayer*>(entity) : nullptr;
        if (player != nullptr)
            player->func_6420_o();
    }
    else if (packet->animate == 6)
    {
        mc->effectRenderer->addEffect(new EntityCrit2FX(mc->theWorld, entity));
    }
    else if (packet->animate == 7)
    {
        mc->effectRenderer->addEffect(new EntityCrit2FX(mc->theWorld, entity, "magicCrit"));
    }
    // Animation 5 is intentionally ignored for EntityOtherPlayerMP in 1.2.5.
}

void NetClientHandler::handleSleep(Packet17Sleep* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    if (packet->sleepState == 0)
    {
        EntityPlayer* player = entity->isPlayer() ? static_cast<EntityPlayer*>(entity) : nullptr;
        if (player != nullptr)
        {
            player->sleepInBedAt(packet->bedX, packet->bedY, packet->bedZ);
        }
    }
}

void NetClientHandler::handleHandshake(Packet2Handshake* packet)
{
    if (packet == nullptr || mc == nullptr || mc->session == nullptr)
        return;

    // Minecraft 1.2.5 validates the server key before attempting session
    // authentication. "-" denotes an offline-mode server.
    bool validServerKey = !packet->username.empty();
    if (validServerKey && packet->username != "-")
    {
        try
        {
            size_t consumed = 0;
            (void)std::stoll(packet->username, &consumed, 16);
            validServerKey = consumed == packet->username.size();
        }
        catch (const std::exception &)
        {
            validServerKey = false;
        }
    }

    if (!validServerKey)
    {
        netManager->networkShutdown("disconnect.genericReason", {
            "The server responded with an invalid server key"
        });
        return;
    }

    if (packet->username == "-")
    {
        addToSendQueue(new Packet1Login(mc->session->username, 29));
        return;
    }

    try
    {
        const std::string url = "http://session.minecraft.net/game/joinserver.jsp?user="
                              + mc->session->username
                              + "&sessionId=" + mc->session->sessionId
                              + "&serverId=" + packet->username;

        std::vector<unsigned char> responseBytes;
        if (!JavaNetwork::readUrl(url, responseBytes))
            throw std::runtime_error("Unable to contact Minecraft session server");

        std::string response(responseBytes.begin(), responseBytes.end());
        const size_t lineEnd = response.find_first_of("\r\n");
        if (lineEnd != std::string::npos)
            response.resize(lineEnd);

        std::string responseLower = response;
        for (char &c : responseLower)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (responseLower == "ok")
            addToSendQueue(new Packet1Login(mc->session->username, 29));
        else
            netManager->networkShutdown("disconnect.loginFailedInfo", {response});
    }
    catch (const std::exception &e)
    {
        netManager->networkShutdown("disconnect.genericReason", {
            "Internal client error: " + std::string(e.what())
        });
    }
}

void NetClientHandler::disconnect()
{
    disconnected = true;
    netManager->wakeThreads();
    netManager->networkShutdown("disconnect.closed", {});
}

void NetClientHandler::handleKeepAlive(Packet0KeepAlive* packet)
{
    if (packet != nullptr)
        addToSendQueue(new Packet0KeepAlive(packet->randomId));
}

void NetClientHandler::handleMobSpawn(Packet24MobSpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    float yaw = (float)(packet->yaw * 360) / 256.0f;
    float pitch = (float)(packet->pitch * 360) / 256.0f;
    
    Entity* entityCreated = EntityList::createEntity(packet->type, mc->theWorld);
    EntityLiving* entityLiving = (entityCreated && entityCreated->isLiving()) ? static_cast<EntityLiving*>(entityCreated) : nullptr;
	if (entityLiving == nullptr)
	{
		delete entityCreated;
		return;
	}
    
    entityLiving->serverPosX = packet->xPosition;
    entityLiving->serverPosY = packet->yPosition;
    entityLiving->serverPosZ = packet->zPosition;
    entityLiving->rotationYawHead = (float)(packet->headYaw * 360) / 256.0f;

    // Multipart entities allocate IDs for their parts when constructed. Java
    // shifts those IDs by the same delta as the root entity before replacing
    // the root ID with the server-provided value. Keep this generic so future
    // multipart entities do not need special cases here.
    const std::vector<Entity *> parts = entityLiving->getParts();
    if (!parts.empty())
    {
        const int_t idOffset = JavaArithmetic::intSub(packet->entityId, entityLiving->entityId);
        for (Entity *part : parts)
        {
            if (part != nullptr)
                part->entityId = JavaArithmetic::intAdd(part->entityId, idOffset);
        }
    }

    entityLiving->entityId = packet->entityId;
    entityLiving->setPositionAndRotation(posX, posY, posZ, yaw, pitch);
    entityLiving->isMultiplayerEntity = true;
    
    worldClient->addEntityToWorld(packet->entityId, entityLiving);
    
    auto metadata = packet->getMetadata();
    if (!metadata.empty())
    {
        entityLiving->getDataWatcher()->updateWatchedObjectsFromList(metadata);
    }
}

void NetClientHandler::handleUpdateTime(Packet4UpdateTime* packet)
{
    mc->theWorld->setWorldTime(packet->time);
}

void NetClientHandler::handleSpawnPosition(Packet6SpawnPosition* packet)
{
    ChunkCoordinates spawn(packet->xPosition, packet->yPosition, packet->zPosition);
    mc->thePlayer->setPlayerSpawnCoordinate(&spawn);
    
    mc->theWorld->getWorldInfo()->setSpawn(
        packet->xPosition,
        packet->yPosition,
        packet->zPosition
    );
}

void NetClientHandler::handleAttachEntity(Packet39AttachEntity* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    Entity* vehicle = getEntityByID(packet->vehicleEntityId);
    
    if (packet->entityId == mc->thePlayer->entityId)
    {
        entity = mc->thePlayer;
    }
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->mountEntity(vehicle);
}

void NetClientHandler::handleEntityStatus(Packet38EntityStatus* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity != nullptr)
    {
        entity->handleHealthUpdate(packet->entityStatus);
    }
}

Entity* NetClientHandler::getEntityByID(int entityId)
{
    if (entityId == mc->thePlayer->entityId)
    {
        return mc->thePlayer;
    }
    
    return worldClient->getEntityByID(entityId);
}

void NetClientHandler::handleHealth(Packet8UpdateHealth* packet)
{
    if (packet == nullptr || mc == nullptr || mc->thePlayer == nullptr)
        return;

    mc->thePlayer->setHealth(packet->healthMP);
    FoodStats *foodStats = mc->thePlayer->getFoodStats();
    if (foodStats != nullptr)
    {
        foodStats->setFoodLevel(packet->food);
        foodStats->setFoodSaturationLevel(packet->foodSaturation);
    }
}

void NetClientHandler::handleEntityEffect(Packet41EntityEffect* packet)
{
    if (packet == nullptr)
        return;
    Entity *entity = getEntityByID(packet->entityId);
    EntityLiving *living = entity != nullptr && entity->isLiving() ? static_cast<EntityLiving *>(entity) : nullptr;
    if (living != nullptr)
        living->addPotionEffect(new PotionEffect(packet->effectId, packet->duration, packet->effectAmp));
}

void NetClientHandler::handleRemoveEntityEffect(Packet42RemoveEntityEffect* packet)
{
    if (packet == nullptr)
        return;
    Entity *entity = getEntityByID(packet->entityId);
    EntityLiving *living = entity != nullptr && entity->isLiving() ? static_cast<EntityLiving *>(entity) : nullptr;
    if (living != nullptr)
        living->removePotionEffect(packet->effectId);
}

void NetClientHandler::handleExperience(Packet43Experience* packet)
{
    if (packet != nullptr && mc != nullptr && mc->thePlayer != nullptr)
        mc->thePlayer->setXPStats(packet->experience, packet->experienceTotal, packet->experienceLevel);
}

void NetClientHandler::handleRespawn(Packet9Respawn* packet)
{
    if (packet == nullptr || mc == nullptr || mc->thePlayer == nullptr)
        return;

    if (packet->respawnDimension != mc->thePlayer->dimension)
    {
        terrainDownloaded = false;

        WorldSettings settings(0L, packet->creativeMode, false, false, packet->terrainType);
        worldClient = new WorldClient(this, settings, packet->respawnDimension, packet->difficulty);
        worldClient->multiplayerWorld = true;
        mc->changeWorld1(worldClient);
        mc->thePlayer->dimension = packet->respawnDimension;
        mc->displayGuiScreen(new GuiDownloadTerrain(this));
    }

    mc->respawn(true, packet->respawnDimension, false);
    PlayerControllerMP *controller = dynamic_cast<PlayerControllerMP *>(mc->playerController);
    if (controller != nullptr)
        controller->setCreative(packet->creativeMode == 1);
}

void NetClientHandler::handleExplosion(Packet60Explosion* packet)
{
    Explosion* explosion = new Explosion(
        mc->theWorld,
        nullptr,
        packet->explosionX,
        packet->explosionY,
        packet->explosionZ,
        packet->explosionSize
    );
    
    for (const ChunkPosition &pos : packet->destroyedBlockPositions.valuesInIterationOrder())
        explosion->destroyedBlockPositions.add({pos.x, pos.y, pos.z});
    explosion->doExplosionB(true);
	delete explosion;
}

void NetClientHandler::handleOpenWindow(Packet100OpenWindow* packet)
{
    if (packet->inventoryType == 0)
    {
        // Chest
        InventoryBasic* inventory = new InventoryBasic(packet->windowTitle, packet->slotsCount);
        mc->thePlayer->displayGUIChest(inventory);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 2)
    {
        // Furnace
        TileEntityFurnace* furnace = new TileEntityFurnace();
        mc->thePlayer->displayGUIFurnace(furnace);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 3)
    {
        // Dispenser
        TileEntityDispenser* dispenser = new TileEntityDispenser();
        mc->thePlayer->displayGUIDispenser(dispenser);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 1)
    {
        // Crafting table
        EntityPlayerSP* playerSP = mc->thePlayer;
        EntityPlayer* player = playerSP;
        
        mc->thePlayer->displayWorkbenchGUI(
            MathHelper::floor_double(player->posX),
            MathHelper::floor_double(player->posY),
            MathHelper::floor_double(player->posZ)
        );
        
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 4)
    {
        // Enchantment table
        EntityPlayer *player = mc->thePlayer;
        mc->thePlayer->displayGUIEnchantment(
            MathHelper::floor_double(player->posX),
            MathHelper::floor_double(player->posY),
            MathHelper::floor_double(player->posZ));
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 5)
    {
        TileEntityBrewingStand *brewingStand = new TileEntityBrewingStand();
        mc->thePlayer->displayGUIBrewingStand(brewingStand);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
}

void NetClientHandler::handleSetSlot(Packet103SetSlot* packet)
{
    if (packet == nullptr)
        return;

    if (packet->windowId == -1)
    {
        ItemStack *oldStack = mc->thePlayer->inventory->getItemStack();
        ItemStack *newStack = packet->releaseItemStack();
        mc->thePlayer->inventory->setItemStack(newStack);
        if (oldStack != newStack)
            delete oldStack;
    }
    else if (packet->windowId == 0 && packet->itemSlot >= 36 && packet->itemSlot < 45)
    {
        Slot *slot = mc->thePlayer->inventorySlots->getSlot(packet->itemSlot);
        ItemStack *oldStack = slot->getStack();
        ItemStack *newStack = packet->releaseItemStack();
        if (newStack != nullptr &&
            (oldStack == nullptr || oldStack->stackSize < newStack->stackSize))
        {
            newStack->animationsToGo = 5;
        }
        slot->putStack(newStack);
    }
    else if (packet->windowId == mc->thePlayer->craftingInventory->windowId)
    {
        Container *container = mc->thePlayer->craftingInventory;
        if (packet->itemSlot >= 0 && (size_t)packet->itemSlot < container->slots.size())
        {
            Slot *slot = container->getSlot(packet->itemSlot);
            ItemStack *newStack = packet->releaseItemStack();
            slot->putStack(newStack);
        }
    }
}


void NetClientHandler::handleTransaction(Packet106Transaction* packet)
{
    Container* container = nullptr;
    
    if (packet->windowId == 0)
    {
        container = mc->thePlayer->inventorySlots;
    }
    else if (packet->windowId == mc->thePlayer->craftingInventory->windowId)
    {
        container = mc->thePlayer->craftingInventory;
    }
    
    if (container != nullptr)
    {
        if (packet->accepted)
        {
            container->updateTransaction(packet->action);
        }
        else
        {
            container->onTransactionResponse(packet->action);
            addToSendQueue(new Packet106Transaction(packet->windowId, packet->action, true));
        }
    }
}

void NetClientHandler::handleWindowItems(Packet104WindowItems* packet)
{
    if (packet == nullptr)
        return;

    Container *container = nullptr;
    if (packet->windowId == 0)
        container = mc->thePlayer->inventorySlots;
    else if (packet->windowId == mc->thePlayer->craftingInventory->windowId)
        container = mc->thePlayer->craftingInventory;

    if (container == nullptr)
        return;

    size_t count = std::min(packet->itemStack.size(), container->slots.size());
    for (size_t i = 0; i < count; ++i)
    {
        Slot *slot = container->getSlot((int_t)i);
        ItemStack *newStack = packet->releaseItemStack(i);
        slot->putStack(newStack);
    }
    // Entries beyond the real container size remain owned by the packet and
    // are released by Packet104WindowItems::~Packet104WindowItems().
}


void NetClientHandler::handleSignUpdate(Packet130UpdateSign* packet)
{
    if (mc->theWorld->blockExists(packet->xPosition, packet->yPosition, packet->zPosition))
    {
        TileEntity* tileEntity = mc->theWorld->getBlockTileEntity(
            packet->xPosition,
            packet->yPosition,
            packet->zPosition
        );
        
        TileEntitySign* sign = dynamic_cast<TileEntitySign*>(tileEntity);
        
        if (sign != nullptr && sign->isEditable())
        {
            for (int i = 0; i < 4; i++)
            {
                sign->signText[i] = packet->signLines[i];
            }
            
            sign->onInventoryChanged();
        }
    }
}

void NetClientHandler::handleTileEntityData(Packet132TileEntityData* packet)
{
    if (packet == nullptr || mc == nullptr || mc->theWorld == nullptr)
        return;
    if (!mc->theWorld->blockExists(packet->xPosition, packet->yPosition, packet->zPosition))
        return;

    TileEntity *tileEntity = mc->theWorld->getBlockTileEntity(packet->xPosition, packet->yPosition, packet->zPosition);
    TileEntityMobSpawner *spawner = dynamic_cast<TileEntityMobSpawner *>(tileEntity);
    if (spawner != nullptr && packet->actionType == 1)
        spawner->setMobID(EntityList::getStringFromID(packet->customParam1));
}

void NetClientHandler::handleUpdateProgressbar(Packet105UpdateProgressbar* packet)
{
    if (mc->thePlayer->craftingInventory != nullptr &&
        mc->thePlayer->craftingInventory->windowId == packet->windowId)
    {
        mc->thePlayer->craftingInventory->updateProgressBar(
            packet->progressBar,
            packet->progressBarValue
        );
    }
}

void NetClientHandler::handlePlayerInventory(Packet5PlayerInventory* packet)
{
    Entity* entity = getEntityByID(packet->entityID);
    
    if (entity != nullptr)
    {
        entity->outfitWithItem(packet->slot, packet->itemID, packet->itemDamage);
    }
}

void NetClientHandler::handleCloseWindow(Packet101CloseWindow* packet)
{
    mc->thePlayer->closeScreen();
}

void NetClientHandler::handleNotePlay(Packet54PlayNoteBlock* packet)
{
    mc->theWorld->playNoteAt(
        packet->xLocation,
        packet->yLocation,
        packet->zLocation,
        packet->instrumentType,
        packet->pitch
    );
}

void NetClientHandler::handleBedEvent(Packet70Bed* packet)
{
    int eventType = packet->bedState;

    if (eventType >= 0 && eventType < Packet70Bed::bedMessageCount &&
        !Packet70Bed::bedMessages[eventType].empty())
    {
        mc->thePlayer->addChatMessage(Packet70Bed::bedMessages[eventType]);
    }

    if (eventType == 1)
    {
        worldClient->getWorldInfo()->setRaining(true);
        worldClient->setRainStrength(1.0f);
    }
    else if (eventType == 2)
    {
        worldClient->getWorldInfo()->setRaining(false);
        worldClient->setRainStrength(0.0f);
    }
    else if (eventType == 3)
    {
        PlayerControllerMP *controller = dynamic_cast<PlayerControllerMP *>(mc->playerController);
        if (controller != nullptr)
            controller->setCreative(packet->gameMode == 1);
    }
    else if (eventType == 4)
    {
        mc->displayGuiScreen(new GuiWinGame());
    }
}

void NetClientHandler::handleMapData(Packet131MapData* packet)
{
    if (packet->itemId == Item::mapItem->shiftedIndex)
    {
        MapData *mapdata = ItemMap::getMapData(packet->mapId, mc->theWorld);
        if (mapdata != nullptr)
            mapdata->handleMapPacket(packet->data);
    }
    else
    {
        MC_LOG_WARN("network", "Unknown itemid: %d\n", packet->mapId);
    }
}

void NetClientHandler::handleDoorChange(Packet61DoorChange* packet)
{
    // playAuxSFX(type, x, y, z, data): doorType = soundType, doorState = soundData
    mc->theWorld->playAuxSFX(
        packet->soundType,
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->soundData
    );
}

void NetClientHandler::handleStatistic(Packet200Statistic* packet)
{
    EntityClientPlayerMP* clientPlayer = (mc->thePlayer->getEntityClassID() == EntityClientPlayerMP::CLASS_ID) ? static_cast<EntityClientPlayerMP*>(mc->thePlayer) : nullptr;
    
    if (clientPlayer != nullptr)
    {
        clientPlayer->incrementStat(
            StatList::getStatById(packet->statId),
            packet->amount
        );
    }
}

void NetClientHandler::handlePlayerInfo(Packet201PlayerInfo* packet)
{
    if (packet == nullptr)
        return;

    auto found = playerInfoMap.find(packet->playerName);
    if (found == playerInfoMap.end() && packet->isConnected)
    {
        auto info = std::make_unique<GuiPlayerInfo>(packet->playerName);
        GuiPlayerInfo *raw = info.get();
        playerInfoMap.emplace(packet->playerName, std::move(info));
        playerNames.push_back(raw);
        found = playerInfoMap.find(packet->playerName);
    }

    if (found != playerInfoMap.end() && !packet->isConnected)
    {
        GuiPlayerInfo *raw = found->second.get();
        playerNames.erase(std::remove(playerNames.begin(), playerNames.end(), raw), playerNames.end());
        playerInfoMap.erase(found);
        return;
    }

    if (packet->isConnected && found != playerInfoMap.end())
        found->second->responseTime = packet->ping;
}

void NetClientHandler::handlePlayerAbilities(Packet202PlayerAbilities* packet)
{
    if (packet == nullptr || mc == nullptr || mc->thePlayer == nullptr)
        return;
    mc->thePlayer->capabilities.isFlying = packet->isFlying;
    mc->thePlayer->capabilities.isCreativeMode = packet->isCreativeMode;
    mc->thePlayer->capabilities.disableDamage = packet->disableDamage;
    mc->thePlayer->capabilities.allowFlying = packet->allowFlying;
}

bool NetClientHandler::isServerHandler()
{
    return false;
}
