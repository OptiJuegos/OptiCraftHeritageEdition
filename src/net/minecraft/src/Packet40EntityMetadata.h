#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;
class DataWatcher;
class WatchableObject;

// net.minecraft.src.Packet40EntityMetadata
class Packet40EntityMetadata : public Packet
{
public:
	Packet40EntityMetadata() = default;
	~Packet40EntityMetadata() override;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	std::vector<WatchableObject *> getMetadata();

	int_t entityId = 0;

private:
	std::vector<WatchableObject *> metadataList;
};
