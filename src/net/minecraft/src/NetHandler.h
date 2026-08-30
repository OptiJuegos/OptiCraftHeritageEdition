#pragma once

#include <string>
#include <vector>

class Packet;
class Packet0KeepAlive;
class Packet1Login;
class Packet2Handshake;
class Packet3Chat;
class Packet4UpdateTime;
class Packet5PlayerInventory;
class Packet6SpawnPosition;
class Packet7UseEntity;
class Packet8UpdateHealth;
class Packet9Respawn;
class Packet10Flying;
class Packet14BlockDig;
class Packet15Place;
class Packet16BlockItemSwitch;
class Packet17Sleep;
class Packet18Animation;
class Packet19EntityAction;
class Packet20NamedEntitySpawn;
class Packet21PickupSpawn;
class Packet22Collect;
class Packet23VehicleSpawn;
class Packet24MobSpawn;
class Packet25EntityPainting;
class Packet26EntityExpOrb;
class Packet27Position;
class Packet28EntityVelocity;
class Packet29DestroyEntity;
class Packet30Entity;
class Packet34EntityTeleport;
class Packet35EntityHeadRotation;
class Packet38EntityStatus;
class Packet39AttachEntity;
class Packet40EntityMetadata;
class Packet41EntityEffect;
class Packet42RemoveEntityEffect;
class Packet43Experience;
class Packet50PreChunk;
class Packet51MapChunk;
class Packet52MultiBlockChange;
class Packet53BlockChange;
class Packet54PlayNoteBlock;
class Packet60Explosion;
class Packet61DoorChange;
class Packet70Bed;
class Packet71Weather;
class Packet100OpenWindow;
class Packet101CloseWindow;
class Packet102WindowClick;
class Packet103SetSlot;
class Packet104WindowItems;
class Packet105UpdateProgressbar;
class Packet106Transaction;
class Packet107CreativeSetSlot;
class Packet108EnchantItem;
class Packet130UpdateSign;
class Packet131MapData;
class Packet132TileEntityData;
class Packet200Statistic;
class Packet201PlayerInfo;
class Packet202PlayerAbilities;
class Packet250CustomPayload;
class Packet254ServerPing;
class Packet255KickDisconnect;

// net.minecraft.src.NetHandler
class NetHandler
{
public:
	virtual ~NetHandler() = default;
	virtual bool isServerHandler() { return false; }
	virtual void registerPacket(Packet &) {}
	virtual void handleErrorMessage(const std::string &, const std::vector<std::string> &) {}
	virtual void handleKeepAlive(Packet0KeepAlive &) {}
	virtual void handleHandshake(Packet2Handshake &) {}
	virtual void handleLogin(Packet1Login &) {}
	virtual void handlePickupSpawn(Packet21PickupSpawn &) {}
	virtual void handleVehicleSpawn(Packet23VehicleSpawn &) {}
	virtual void handleWeather(Packet71Weather &) {}
	virtual void handleEntityPainting(Packet25EntityPainting &) {}
	virtual void handleEntityExpOrb(Packet26EntityExpOrb &) {}
	virtual void handleEntityVelocity(Packet28EntityVelocity &) {}
	virtual void handleEntityMetadata(Packet40EntityMetadata &) {}
	virtual void handleEntityEffect(Packet41EntityEffect &) {}
	virtual void handleRemoveEntityEffect(Packet42RemoveEntityEffect &) {}
	virtual void handleExperience(Packet43Experience &) {}
	virtual void handleNamedEntitySpawn(Packet20NamedEntitySpawn &) {}
	virtual void handleEntityTeleport(Packet34EntityTeleport &) {}
	virtual void handleEntityHeadRotation(Packet35EntityHeadRotation &) {}
	virtual void handleEntity(Packet30Entity &) {}
	virtual void handleDestroyEntity(Packet29DestroyEntity &) {}
	virtual void handleFlying(Packet10Flying &) {}
	virtual void handlePreChunk(Packet50PreChunk &) {}
	virtual void handleMultiBlockChange(Packet52MultiBlockChange &) {}
	virtual void handleMapChunk(Packet51MapChunk &) {}
	virtual void func_48487_a(Packet51MapChunk &p) { handleMapChunk(p); }
	virtual void handleBlockChange(Packet53BlockChange &) {}
	virtual void handleKickDisconnect(Packet255KickDisconnect &) {}
	virtual void handleCollect(Packet22Collect &) {}
	virtual void handleChat(Packet3Chat &) {}
	virtual void handleArmAnimation(Packet18Animation &) {}
	virtual void handleAnimation(Packet18Animation &p) { handleArmAnimation(p); }
	virtual void handleSleep(Packet17Sleep &) {}
	virtual void handleMobSpawn(Packet24MobSpawn &) {}
	virtual void handleUpdateTime(Packet4UpdateTime &) {}
	virtual void handleSpawnPosition(Packet6SpawnPosition &) {}
	virtual void handleAttachEntity(Packet39AttachEntity &) {}
	virtual void handleEntityStatus(Packet38EntityStatus &) {}
	virtual void handleHealth(Packet8UpdateHealth &) {}
	virtual void handleUpdateHealth(Packet8UpdateHealth &p) { handleHealth(p); }
	virtual void handleRespawn(Packet9Respawn &) {}
	virtual void handleExplosion(Packet60Explosion &) {}
	virtual void handleOpenWindow(Packet100OpenWindow &) {}
	virtual void handleSetSlot(Packet103SetSlot &) {}
	virtual void handleTransaction(Packet106Transaction &) {}
	virtual void handleCreativeSetSlot(Packet107CreativeSetSlot &) {}
	virtual void handleWindowItems(Packet104WindowItems &) {}
	virtual void handleUpdateSign(Packet130UpdateSign &) {}
	virtual void handleUpdateProgressbar(Packet105UpdateProgressbar &) {}
	virtual void handlePlayerInventory(Packet5PlayerInventory &) {}
	virtual void handleNotePlay(Packet54PlayNoteBlock &) {}
	virtual void handlePlayNoteBlock(Packet54PlayNoteBlock &p) { handleNotePlay(p); }
	virtual void handleBed(Packet70Bed &) {}
	virtual void handleMapData(Packet131MapData &) {}
	virtual void handleTileEntityData(Packet132TileEntityData &) {}
	virtual void handleDoorChange(Packet61DoorChange &) {}
	virtual void handleStatistic(Packet200Statistic &) {}
	virtual void handlePlayerInfo(Packet201PlayerInfo &) {}
	virtual void handlePlayerAbilities(Packet202PlayerAbilities &) {}
	virtual void func_50100_a(Packet202PlayerAbilities &p) { handlePlayerAbilities(p); }
	virtual void handleCustomPayload(Packet250CustomPayload &) {}
	virtual void handleServerPing(Packet254ServerPing &) {}
	virtual void handleCloseWindow(Packet101CloseWindow &) {}
	virtual void handleWindowClick(Packet102WindowClick &) {}
	virtual void handleEnchantItem(Packet108EnchantItem &) {}
	virtual void handleBlockDig(Packet14BlockDig &) {}
	virtual void handlePlace(Packet15Place &) {}
	virtual void handleBlockItemSwitch(Packet16BlockItemSwitch &) {}
	virtual void handleUseEntity(Packet7UseEntity &) {}
	virtual void handleEntityAction(Packet19EntityAction &) {}
	virtual void handleVehicleControl(Packet27Position &) {}
};
