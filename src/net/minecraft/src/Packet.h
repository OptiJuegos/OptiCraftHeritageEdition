#pragma once

#include <map>
#include <typeindex>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include "java/Type.h"
#include "java/String.h"
#include "java/IOUtil.h"

class NetHandler;
class PacketCounter;
class ItemStack;
class NBTTagCompound;

// net.minecraft.src.Packet
class Packet
{
public:
	Packet();
	virtual ~Packet() = default;

	virtual void readPacketData(std::istream &is) = 0;
	virtual void writePacketData(std::ostream &os) = 0;
	virtual void processPacket(NetHandler &nethandler) = 0;
	virtual int_t getPacketSize() = 0;

	static void addIdClassMapping(int_t id, bool clientPacket, bool serverPacket, std::function<std::unique_ptr<Packet>()> factory);
	int_t getPacketId() const;
	static std::unique_ptr<Packet> getNewPacket(int_t id);
	static std::unique_ptr<Packet> readPacket(std::istream &is, bool flag);
	static void writePacket(Packet *packet, std::ostream &os);
	static void writeString(const jstring &s, std::ostream &os);
	static jstring readString(std::istream &is, int_t maxLen);

protected:
	ItemStack *readItemStack(std::istream &is);
	void writeItemStack(ItemStack *itemStack, std::ostream &os);
	NBTTagCompound *readNBTTagCompound(std::istream &is);
	void writeNBTTagCompound(NBTTagCompound *compound, std::ostream &os);

public:
	bool isChunkDataPacket;
	long_t creationTimeMillis;

private:
	static std::map<int_t, std::function<std::unique_ptr<Packet>()>> packetIdToFactoryMap;
	static std::map<std::type_index, int_t> packetClassToIdMap;
	static std::set<int_t> clientPacketIdList;
	static std::set<int_t> serverPacketIdList;
	static std::map<int_t, std::unique_ptr<PacketCounter>> packetStats;
	static int_t totalPacketsCount;
};
