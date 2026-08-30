#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;
class EntityLiving;
class DataWatcher;
class WatchableObject;

// net.minecraft.src.Packet24MobSpawn
class Packet24MobSpawn : public Packet
{
public:
	Packet24MobSpawn();
	~Packet24MobSpawn() override;
	Packet24MobSpawn(EntityLiving *entityliving);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	std::vector<WatchableObject *> getMetadata();

	int_t entityId = 0;
	int_t type = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t yaw = 0;
	byte_t pitch = 0;
	byte_t headYaw = 0; // field_48169_h

private:
	DataWatcher *metaData = nullptr;
	std::vector<WatchableObject *> receivedMetadata;
};
