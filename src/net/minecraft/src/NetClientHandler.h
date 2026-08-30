#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "java/Random.h"
#include "java/String.h"
#include <stdexcept>

// Forward declarations
class Minecraft;
class MapStorage;
class NetworkManager;
class WorldClient;
class Packet;
class Packet0KeepAlive;
class Packet1Login;
class Packet21PickupSpawn;
class Packet23VehicleSpawn;
class Packet71Weather;
class Packet25EntityPainting;
class Packet26EntityExpOrb;
class Packet28EntityVelocity;
class Packet40EntityMetadata;
class Packet41EntityEffect;
class Packet42RemoveEntityEffect;
class Packet43Experience;
class Packet20NamedEntitySpawn;
class Packet34EntityTeleport;
class Packet35EntityHeadRotation;
class Packet30Entity;
class Packet29DestroyEntity;
class Packet10Flying;
class Packet50PreChunk;
class Packet52MultiBlockChange;
class Packet51MapChunk;
class Packet53BlockChange;
class Packet255KickDisconnect;
class Packet22Collect;
class Packet3Chat;
class Packet18Animation;
class Packet17Sleep;
class Packet2Handshake;
class Packet24MobSpawn;
class Packet4UpdateTime;
class Packet6SpawnPosition;
class Packet39AttachEntity;
class Packet38EntityStatus;
class Packet8UpdateHealth;
class Packet9Respawn;
class Packet60Explosion;
class Packet100OpenWindow;
class Packet103SetSlot;
class Packet106Transaction;
class Packet104WindowItems;
class Packet130UpdateSign;
class Packet105UpdateProgressbar;
class Packet5PlayerInventory;
class Packet101CloseWindow;
class Packet54PlayNoteBlock;
class Packet70Bed;
class Packet131MapData;
class Packet132TileEntityData;
class Packet61DoorChange;
class Packet200Statistic;
class Packet201PlayerInfo;
class Packet202PlayerAbilities;
class Entity;
class EntityLiving;
class EntityItem;
class EntityMinecart;
class EntityFish;
class EntityArrow;
class EntitySnowball;
class EntityFireball;
class EntityEgg;
class EntityBoat;
class EntityTNTPrimed;
class EntityFallingSand;
class EntityLightningBolt;
class EntityPainting;
class EntityOtherPlayerMP;
class EntityPlayer;
class EntityPlayerSP;
class EntityClientPlayerMP;
class Chunk;
class InventoryBasic;
class TileEntityFurnace;
class TileEntityDispenser;
class TileEntitySign;
class Container;
class ItemStack;
class Explosion;
class ChunkCoordinates;
class GuiPlayerInfo;

#include "NetHandler.h"

class NetClientHandler : public NetHandler
{
public:
    NetClientHandler(Minecraft* minecraft, const std::string& host, int port);
    ~NetClientHandler();

    void processReadPackets();
    
    // Packet handlers - nombres basados en los de Java, renombrando ofuscados
    void handleLogin(Packet1Login* packet);
    void handlePickupSpawn(Packet21PickupSpawn* packet);
    void handleVehicleSpawn(Packet23VehicleSpawn* packet);
    void handleWeather(Packet71Weather* packet);
    void handlePaintingSpawn(Packet25EntityPainting* packet);           // func_21146_a
    void handleEntityExpOrb(Packet26EntityExpOrb* packet);
    void handleEntityVelocity(Packet28EntityVelocity* packet);          // func_6498_a
    void handleEntityMetadata(Packet40EntityMetadata* packet);          // func_21148_a
    void handleEntityEffect(Packet41EntityEffect* packet);
    void handleRemoveEntityEffect(Packet42RemoveEntityEffect* packet);
    void handleExperience(Packet43Experience* packet);
    void handleNamedEntitySpawn(Packet20NamedEntitySpawn* packet);
    void handleEntityTeleport(Packet34EntityTeleport* packet);
    void handleEntityHeadRotation(Packet35EntityHeadRotation* packet);
    void handleEntityMovement(Packet30Entity* packet);                  // handleEntity
    void handleDestroyEntity(Packet29DestroyEntity* packet);
    void handleFlying(Packet10Flying* packet);
    void handlePreChunk(Packet50PreChunk* packet);
    void handleMultiBlockChange(Packet52MultiBlockChange* packet);
    void handleMapChunk(Packet51MapChunk* packet);
    void handleBlockChange(Packet53BlockChange* packet);
    void handleKickDisconnect(Packet255KickDisconnect* packet);
    void handleErrorMessage(const std::string& message, const std::vector<std::string>& args);
    void quitWithPacket(Packet* packet);
    void sendPacketAndFlush(Packet* packet);                            // func_28117_a
    void addToSendQueue(Packet* packet);
    void handleCollect(Packet22Collect* packet);
    void handleChat(Packet3Chat* packet);
    void handleArmAnimation(Packet18Animation* packet);
    void handleSleep(Packet17Sleep* packet);                              // func_22186_a
    void handleHandshake(Packet2Handshake* packet);
    void disconnect();
    void handleKeepAlive(Packet0KeepAlive* packet);
    void handleMobSpawn(Packet24MobSpawn* packet);
    void handleUpdateTime(Packet4UpdateTime* packet);
    void handleSpawnPosition(Packet6SpawnPosition* packet);
    void handleAttachEntity(Packet39AttachEntity* packet);              // func_6497_a
    void handleEntityStatus(Packet38EntityStatus* packet);              // func_9447_a
    void handleHealth(Packet8UpdateHealth* packet);
    void handleRespawn(Packet9Respawn* packet);                         // func_9448_a
    void handleExplosion(Packet60Explosion* packet);                      // func_12245_a
    void handleOpenWindow(Packet100OpenWindow* packet);                 // func_20087_a
    void handleSetSlot(Packet103SetSlot* packet);                       // func_20088_a
    void handleTransaction(Packet106Transaction* packet);               // func_20089_a
    void handleWindowItems(Packet104WindowItems* packet);               // func_20094_a
    void handleSignUpdate(Packet130UpdateSign* packet);
    void handleUpdateProgressbar(Packet105UpdateProgressbar* packet);   // func_20090_a
    void handlePlayerInventory(Packet5PlayerInventory* packet);
    void handleCloseWindow(Packet101CloseWindow* packet);               // func_20092_a
    void handleNotePlay(Packet54PlayNoteBlock* packet);
    void handleBedEvent(Packet70Bed* packet);                           // func_25118_a
    void handleMapData(Packet131MapData* packet);                       // func_28116_a
    void handleTileEntityData(Packet132TileEntityData* packet);
    void handleDoorChange(Packet61DoorChange* packet);                    // func_28115_a
    void handleStatistic(Packet200Statistic* packet);                    // func_27245_a
    void handlePlayerInfo(Packet201PlayerInfo* packet);
    void handlePlayerAbilities(Packet202PlayerAbilities* packet);
    
    bool isServerHandler() override;

    // ── NetHandler virtual overrides ──────────────────────────────────────────
    // NetHandler declares handlers as `virtual void handleXxx(Packet &)`.
    // The pointer implementations above are the real bodies; these thin wrappers
    // satisfy the vtable so packet dispatch (packet->processPacket(*netHandler))
    // actually calls this handler instead of the empty NetHandler base.
    void handleHandshake(Packet2Handshake &p) override               { handleHandshake(&p); }
    void handleLogin(Packet1Login &p) override                        { handleLogin(&p); }
    void handlePickupSpawn(Packet21PickupSpawn &p) override           { handlePickupSpawn(&p); }
    void handleVehicleSpawn(Packet23VehicleSpawn &p) override         { handleVehicleSpawn(&p); }
    void handleWeather(Packet71Weather &p) override                   { handleWeather(&p); }
    void handleEntityPainting(Packet25EntityPainting &p) override     { handlePaintingSpawn(&p); }
    void handleEntityExpOrb(Packet26EntityExpOrb &p) override           { handleEntityExpOrb(&p); }
    void handleEntityVelocity(Packet28EntityVelocity &p) override     { handleEntityVelocity(&p); }
    void handleEntityMetadata(Packet40EntityMetadata &p) override     { handleEntityMetadata(&p); }
    void handleEntityEffect(Packet41EntityEffect &p) override           { handleEntityEffect(&p); }
    void handleRemoveEntityEffect(Packet42RemoveEntityEffect &p) override { handleRemoveEntityEffect(&p); }
    void handleExperience(Packet43Experience &p) override               { handleExperience(&p); }
    void handleNamedEntitySpawn(Packet20NamedEntitySpawn &p) override { handleNamedEntitySpawn(&p); }
    void handleEntityTeleport(Packet34EntityTeleport &p) override     { handleEntityTeleport(&p); }
    void handleEntityHeadRotation(Packet35EntityHeadRotation &p) override { handleEntityHeadRotation(&p); }
    void handleEntity(Packet30Entity &p) override                     { handleEntityMovement(&p); }
    void handleDestroyEntity(Packet29DestroyEntity &p) override       { handleDestroyEntity(&p); }
    void handleFlying(Packet10Flying &p) override                     { handleFlying(&p); }
    void handlePreChunk(Packet50PreChunk &p) override                 { handlePreChunk(&p); }
    void handleMultiBlockChange(Packet52MultiBlockChange &p) override { handleMultiBlockChange(&p); }
    void handleMapChunk(Packet51MapChunk &p) override                 { handleMapChunk(&p); }
    void handleBlockChange(Packet53BlockChange &p) override           { handleBlockChange(&p); }
    void handleKickDisconnect(Packet255KickDisconnect &p) override    { handleKickDisconnect(&p); }
    void handleCollect(Packet22Collect &p) override                   { handleCollect(&p); }
    void handleChat(Packet3Chat &p) override                          { handleChat(&p); }
    void handleArmAnimation(Packet18Animation &p) override            { handleArmAnimation(&p); }
    void handleSleep(Packet17Sleep &p) override                       { handleSleep(&p); }
    void handleKeepAlive(Packet0KeepAlive &p) override                { handleKeepAlive(&p); }
    void handleMobSpawn(Packet24MobSpawn &p) override                 { handleMobSpawn(&p); }
    void handleUpdateTime(Packet4UpdateTime &p) override              { handleUpdateTime(&p); }
    void handleSpawnPosition(Packet6SpawnPosition &p) override        { handleSpawnPosition(&p); }
    void handleAttachEntity(Packet39AttachEntity &p) override         { handleAttachEntity(&p); }
    void handleEntityStatus(Packet38EntityStatus &p) override         { handleEntityStatus(&p); }
    void handleHealth(Packet8UpdateHealth &p) override                { handleHealth(&p); }
    void handleRespawn(Packet9Respawn &p) override                    { handleRespawn(&p); }
    void handleExplosion(Packet60Explosion &p) override               { handleExplosion(&p); }
    void handleOpenWindow(Packet100OpenWindow &p) override            { handleOpenWindow(&p); }
    void handleSetSlot(Packet103SetSlot &p) override                  { handleSetSlot(&p); }
    void handleTransaction(Packet106Transaction &p) override          { handleTransaction(&p); }
    void handleWindowItems(Packet104WindowItems &p) override          { handleWindowItems(&p); }
    void handleUpdateSign(Packet130UpdateSign &p) override            { handleSignUpdate(&p); }
    void handleUpdateProgressbar(Packet105UpdateProgressbar &p) override { handleUpdateProgressbar(&p); }
    void handlePlayerInventory(Packet5PlayerInventory &p) override    { handlePlayerInventory(&p); }
    void handleNotePlay(Packet54PlayNoteBlock &p) override            { handleNotePlay(&p); }
    void handleBed(Packet70Bed &p) override                           { handleBedEvent(&p); }
    void handleMapData(Packet131MapData &p) override                  { handleMapData(&p); }
    void handleTileEntityData(Packet132TileEntityData &p) override      { handleTileEntityData(&p); }
    void handleDoorChange(Packet61DoorChange &p) override             { handleDoorChange(&p); }
    void handleStatistic(Packet200Statistic &p) override              { handleStatistic(&p); }
    void handlePlayerInfo(Packet201PlayerInfo &p) override              { handlePlayerInfo(&p); }
    void handlePlayerAbilities(Packet202PlayerAbilities &p) override    { handlePlayerAbilities(&p); }
    void handleCloseWindow(Packet101CloseWindow &p) override          { handleCloseWindow(&p); }
    // handleErrorMessage already has the correct reference signature above — no wrapper needed.

    // Getters
    NetworkManager* getNetworkManager() const { return netManager; }
    const std::string& getServerHostname() const { return serverHostname; }
    WorldClient* getWorldClient() const { return worldClient; }
    bool isDisconnected() const { return disconnected; }
    bool isOwnedByPlayerController() const { return playerControllerOwnsHandler; }
    MapStorage* getMapStorage() const { return mapStorage; }
    const std::vector<GuiPlayerInfo*>& getPlayerNames() const { return playerNames; }
    int_t currentServerMaxPlayers = 20;

private:
    Entity* getEntityByID(int entityId);

    bool disconnected;
    bool playerControllerOwnsHandler = false;
    NetworkManager* netManager;
    std::string serverHostname;  // field_1209_a
    Minecraft* mc;
    WorldClient* worldClient = nullptr;
    bool terrainDownloaded;      // field_1210_g - true cuando se ha descargado el terreno inicial
    MapStorage* mapStorage;      // field_28118_b
    Random rand;
    std::unordered_map<jstring, std::unique_ptr<GuiPlayerInfo>> playerInfoMap;
    std::vector<GuiPlayerInfo*> playerNames;
};
