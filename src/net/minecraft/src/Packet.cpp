#include "Packet.h"
#include "java/String.h"
#include "Item.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "CompressedStreamTools.h"

#include <iostream>
#include <chrono>
#include <locale>
#include <mutex>
#include <codecvt>
#include "PacketCounter.h"
#include "java/IOUtil.h"
#include "Packet0KeepAlive.h"
#include "Packet1Login.h"
#include "Packet2Handshake.h"
#include "Packet3Chat.h"
#include "Packet4UpdateTime.h"
#include "Packet5PlayerInventory.h"
#include "Packet6SpawnPosition.h"
#include "Packet7UseEntity.h"
#include "Packet8UpdateHealth.h"
#include "Packet9Respawn.h"
#include "Packet10Flying.h"
#include "Packet11PlayerPosition.h"
#include "Packet12PlayerLook.h"
#include "Packet13PlayerLookMove.h"
#include "Packet14BlockDig.h"
#include "Packet15Place.h"
#include "Packet16BlockItemSwitch.h"
#include "Packet17Sleep.h"
#include "Packet18Animation.h"
#include "Packet19EntityAction.h"
#include "Packet20NamedEntitySpawn.h"
#include "Packet21PickupSpawn.h"
#include "Packet22Collect.h"
#include "Packet23VehicleSpawn.h"
#include "Packet24MobSpawn.h"
#include "Packet25EntityPainting.h"
#include "Packet26EntityExpOrb.h"
#include "Packet27Position.h"
#include "Packet28EntityVelocity.h"
#include "Packet29DestroyEntity.h"
#include "Packet30Entity.h"
#include "Packet31RelEntityMove.h"
#include "Packet32EntityLook.h"
#include "Packet33RelEntityMoveLook.h"
#include "Packet34EntityTeleport.h"
#include "Packet35EntityHeadRotation.h"
#include "Packet38EntityStatus.h"
#include "Packet39AttachEntity.h"
#include "Packet40EntityMetadata.h"
#include "Packet41EntityEffect.h"
#include "Packet42RemoveEntityEffect.h"
#include "Packet43Experience.h"
#include "Packet50PreChunk.h"
#include "Packet51MapChunk.h"
#include "Packet52MultiBlockChange.h"
#include "Packet53BlockChange.h"
#include "Packet54PlayNoteBlock.h"
#include "Packet60Explosion.h"
#include "Packet61DoorChange.h"
#include "Packet70Bed.h"
#include "Packet71Weather.h"
#include "Packet100OpenWindow.h"
#include "Packet101CloseWindow.h"
#include "Packet102WindowClick.h"
#include "Packet103SetSlot.h"
#include "Packet104WindowItems.h"
#include "Packet105UpdateProgressbar.h"
#include "Packet106Transaction.h"
#include "Packet107CreativeSetSlot.h"
#include "Packet108EnchantItem.h"
#include "Packet130UpdateSign.h"
#include "Packet131MapData.h"
#include "Packet132TileEntityData.h"
#include "Packet200Statistic.h"
#include "Packet201PlayerInfo.h"
#include "Packet202PlayerAbilities.h"
#include "Packet250CustomPayload.h"
#include "Packet254ServerPing.h"
#include "Packet255KickDisconnect.h"

std::map<int_t, std::function<std::unique_ptr<Packet>()>> Packet::packetIdToFactoryMap;
std::map<std::type_index, int_t> Packet::packetClassToIdMap;
std::set<int_t> Packet::clientPacketIdList;
std::set<int_t> Packet::serverPacketIdList;
std::map<int_t, std::unique_ptr<PacketCounter>> Packet::packetStats;
int_t Packet::totalPacketsCount = 0;

namespace
{

std::once_flag packetMappingsOnce;

template<typename T>
std::unique_ptr<Packet> createPacket()
{
	return std::make_unique<T>();
}

void initPacketMappings()
{
	std::call_once(packetMappingsOnce, []()
	{
		Packet::addIdClassMapping(0, true, true, createPacket<Packet0KeepAlive>);
		Packet::addIdClassMapping(1, true, true, createPacket<Packet1Login>);
		Packet::addIdClassMapping(2, true, true, createPacket<Packet2Handshake>);
		Packet::addIdClassMapping(3, true, true, createPacket<Packet3Chat>);
		Packet::addIdClassMapping(4, true, false, createPacket<Packet4UpdateTime>);
		Packet::addIdClassMapping(5, true, false, createPacket<Packet5PlayerInventory>);
		Packet::addIdClassMapping(6, true, false, createPacket<Packet6SpawnPosition>);
		Packet::addIdClassMapping(7, false, true, createPacket<Packet7UseEntity>);
		Packet::addIdClassMapping(8, true, false, createPacket<Packet8UpdateHealth>);
		Packet::addIdClassMapping(9, true, true, createPacket<Packet9Respawn>);
		Packet::addIdClassMapping(10, true, true, createPacket<Packet10Flying>);
		Packet::addIdClassMapping(11, true, true, createPacket<Packet11PlayerPosition>);
		Packet::addIdClassMapping(12, true, true, createPacket<Packet12PlayerLook>);
		Packet::addIdClassMapping(13, true, true, createPacket<Packet13PlayerLookMove>);
		Packet::addIdClassMapping(14, false, true, createPacket<Packet14BlockDig>);
		Packet::addIdClassMapping(15, false, true, createPacket<Packet15Place>);
		Packet::addIdClassMapping(16, false, true, createPacket<Packet16BlockItemSwitch>);
		Packet::addIdClassMapping(17, true, false, createPacket<Packet17Sleep>);
		Packet::addIdClassMapping(18, true, true, createPacket<Packet18Animation>);
		Packet::addIdClassMapping(19, false, true, createPacket<Packet19EntityAction>);
		Packet::addIdClassMapping(20, true, false, createPacket<Packet20NamedEntitySpawn>);
		Packet::addIdClassMapping(21, true, false, createPacket<Packet21PickupSpawn>);
		Packet::addIdClassMapping(22, true, false, createPacket<Packet22Collect>);
		Packet::addIdClassMapping(23, true, false, createPacket<Packet23VehicleSpawn>);
		Packet::addIdClassMapping(24, true, false, createPacket<Packet24MobSpawn>);
		Packet::addIdClassMapping(25, true, false, createPacket<Packet25EntityPainting>);
		Packet::addIdClassMapping(26, true, false, createPacket<Packet26EntityExpOrb>);
		Packet::addIdClassMapping(28, true, false, createPacket<Packet28EntityVelocity>);
		Packet::addIdClassMapping(29, true, false, createPacket<Packet29DestroyEntity>);
		Packet::addIdClassMapping(30, true, false, createPacket<Packet30Entity>);
		Packet::addIdClassMapping(31, true, false, createPacket<Packet31RelEntityMove>);
		Packet::addIdClassMapping(32, true, false, createPacket<Packet32EntityLook>);
		Packet::addIdClassMapping(33, true, false, createPacket<Packet33RelEntityMoveLook>);
		Packet::addIdClassMapping(34, true, false, createPacket<Packet34EntityTeleport>);
		Packet::addIdClassMapping(35, true, false, createPacket<Packet35EntityHeadRotation>);
		Packet::addIdClassMapping(38, true, false, createPacket<Packet38EntityStatus>);
		Packet::addIdClassMapping(39, true, false, createPacket<Packet39AttachEntity>);
		Packet::addIdClassMapping(40, true, false, createPacket<Packet40EntityMetadata>);
		Packet::addIdClassMapping(41, true, false, createPacket<Packet41EntityEffect>);
		Packet::addIdClassMapping(42, true, false, createPacket<Packet42RemoveEntityEffect>);
		Packet::addIdClassMapping(43, true, false, createPacket<Packet43Experience>);
		Packet::addIdClassMapping(50, true, false, createPacket<Packet50PreChunk>);
		Packet::addIdClassMapping(51, true, false, createPacket<Packet51MapChunk>);
		Packet::addIdClassMapping(52, true, false, createPacket<Packet52MultiBlockChange>);
		Packet::addIdClassMapping(53, true, false, createPacket<Packet53BlockChange>);
		Packet::addIdClassMapping(54, true, false, createPacket<Packet54PlayNoteBlock>);
		Packet::addIdClassMapping(60, true, false, createPacket<Packet60Explosion>);
		Packet::addIdClassMapping(61, true, false, createPacket<Packet61DoorChange>);
		Packet::addIdClassMapping(70, true, false, createPacket<Packet70Bed>);
		Packet::addIdClassMapping(71, true, false, createPacket<Packet71Weather>);
		Packet::addIdClassMapping(100, true, false, createPacket<Packet100OpenWindow>);
		Packet::addIdClassMapping(101, true, true, createPacket<Packet101CloseWindow>);
		Packet::addIdClassMapping(102, false, true, createPacket<Packet102WindowClick>);
		Packet::addIdClassMapping(103, true, false, createPacket<Packet103SetSlot>);
		Packet::addIdClassMapping(104, true, false, createPacket<Packet104WindowItems>);
		Packet::addIdClassMapping(105, true, false, createPacket<Packet105UpdateProgressbar>);
		Packet::addIdClassMapping(106, true, true, createPacket<Packet106Transaction>);
		Packet::addIdClassMapping(107, true, true, createPacket<Packet107CreativeSetSlot>);
		Packet::addIdClassMapping(108, false, true, createPacket<Packet108EnchantItem>);
		Packet::addIdClassMapping(130, true, true, createPacket<Packet130UpdateSign>);
		Packet::addIdClassMapping(131, true, false, createPacket<Packet131MapData>);
		Packet::addIdClassMapping(132, true, false, createPacket<Packet132TileEntityData>);
		Packet::addIdClassMapping(200, true, false, createPacket<Packet200Statistic>);
		Packet::addIdClassMapping(201, true, false, createPacket<Packet201PlayerInfo>);
		Packet::addIdClassMapping(202, true, true, createPacket<Packet202PlayerAbilities>);
		Packet::addIdClassMapping(250, true, true, createPacket<Packet250CustomPayload>);
		Packet::addIdClassMapping(254, false, true, createPacket<Packet254ServerPing>);
		Packet::addIdClassMapping(255, true, true, createPacket<Packet255KickDisconnect>);
	});
}

}

Packet::Packet() :
	isChunkDataPacket(false)
{
	creationTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
}

void Packet::addIdClassMapping(int_t id, bool clientPacket, bool serverPacket, std::function<std::unique_ptr<Packet>()> factory)
{
	if (packetIdToFactoryMap.find(id) != packetIdToFactoryMap.end())
	{
		throw std::invalid_argument("Duplicate packet id:" + std::to_string(id));
	}
	std::unique_ptr<Packet> sample = factory();
	std::type_index packetType(typeid(*sample));
	if (packetClassToIdMap.find(packetType) != packetClassToIdMap.end())
	{
		throw std::invalid_argument("Duplicate packet class");
	}

	packetIdToFactoryMap[id] = factory;
	packetClassToIdMap[packetType] = id;

	if (clientPacket)
		clientPacketIdList.insert(id);
	if (serverPacket)
		serverPacketIdList.insert(id);
}

int_t Packet::getPacketId() const
{
	initPacketMappings();
	auto it = packetClassToIdMap.find(std::type_index(typeid(*this)));
	if (it == packetClassToIdMap.end())
		throw std::runtime_error("Unregistered packet class");
	return it->second;
}

std::unique_ptr<Packet> Packet::getNewPacket(int_t id)
{
	initPacketMappings();
	auto it = packetIdToFactoryMap.find(id);
	if (it == packetIdToFactoryMap.end())
		return nullptr;
	return it->second();
}

std::unique_ptr<Packet> Packet::readPacket(std::istream &is, bool flag)
{
	initPacketMappings();
	int_t i = is.get();
	if (i == -1)
		return nullptr;

	if ((flag && serverPacketIdList.find(i) == serverPacketIdList.end()) ||
	    (!flag && clientPacketIdList.find(i) == clientPacketIdList.end()))
	{
		throw std::runtime_error("Bad packet id " + std::to_string(i));
	}

	auto packet = getNewPacket(i);
	if (!packet)
	{
		throw std::runtime_error("Bad packet id " + std::to_string(i));
	}

	packet->readPacketData(is);

	auto it = packetStats.find(i);
	if (it == packetStats.end())
	{
		packetStats[i] = std::make_unique<PacketCounter>();
		it = packetStats.find(i);
	}
	it->second->addPacket(packet->getPacketSize());

	totalPacketsCount++;

	return packet;
}

void Packet::writePacket(Packet *packet, std::ostream &os)
{
	if (packet == nullptr)
		return;
	IOUtil::writeByte(os, packet->getPacketId());
	packet->writePacketData(os);
}

void Packet::writeString(const jstring &s, std::ostream &os)
{
	// Beta 1.7.3 protocol: writeShort(char count) + writeChars (UTF-16BE, two
	// bytes per char). Must mirror readString — not Java's writeUTF (which is a
	// byte-length-prefixed modified-UTF-8 and would desync the stream).
	// jstring is UTF-8 in memory; convert to UTF-16 for the wire.
	const std::vector<char_t> u16 = String::toUtf16(s);
	if (u16.size() > 32767)
		throw std::runtime_error("String too big");
	IOUtil::writeShort(os, static_cast<int_t>(u16.size()));
	for (char_t c : u16)
		IOUtil::writeChar(os, static_cast<int_t>(c));
}

jstring Packet::readString(std::istream &is, int_t maxLen)
{
	short_t len = IOUtil::readShort(is);
	if (len > maxLen)
	{
		throw std::runtime_error("Received string length longer than maximum allowed (" + std::to_string(len) + " > " + std::to_string(maxLen) + ")");
	}
	if (len < 0)
	{
		throw std::runtime_error("Received string length is less than zero! Weird string!");
	}

	// Read UTF-16BE code units from the wire, convert to UTF-8 for in-memory jstring.
	std::vector<char_t> u16;
	u16.reserve(len);
	for (int_t i = 0; i < len; i++)
		u16.push_back(static_cast<char_t>(IOUtil::readChar(is)));
	return String::fromUtf16(u16);
}


ItemStack *Packet::readItemStack(std::istream &is)
{
	const short_t itemId = IOUtil::readShort(is);
	if (itemId < 0)
		return nullptr;
	if (itemId >= Item::ITEM_LIST_SIZE || Item::itemsList[itemId] == nullptr)
		throw std::runtime_error("Invalid item id in packet: " + std::to_string(itemId));

	const byte_t count = IOUtil::readByte(is);
	const short_t damage = IOUtil::readShort(is);
	ItemStack *stack = new ItemStack(itemId, count, damage);
	Item *item = Item::itemsList[itemId];
	if (item->isDamagable() || item->func_46056_k())
		stack->setTagCompound(readNBTTagCompound(is));
	return stack;
}

void Packet::writeItemStack(ItemStack *itemStack, std::ostream &os)
{
	if (itemStack == nullptr)
	{
		IOUtil::writeShort(os, -1);
		return;
	}

	IOUtil::writeShort(os, itemStack->itemID);
	IOUtil::writeByte(os, itemStack->stackSize);
	IOUtil::writeShort(os, itemStack->getItemDamage());
	Item *item = itemStack->getItem();
	if (item != nullptr && (item->isDamagable() || item->func_46056_k()))
		writeNBTTagCompound(itemStack->getTagCompound(), os);
}

NBTTagCompound *Packet::readNBTTagCompound(std::istream &is)
{
	const short_t length = IOUtil::readShort(is);
	if (length < 0)
		return nullptr;
	if (length == 0)
		throw std::runtime_error("Invalid zero-length NBT payload");

	std::vector<char> compressed(static_cast<size_t>(length));
	is.read(compressed.data(), static_cast<std::streamsize>(compressed.size()));
	if (!is)
		throw std::runtime_error("Unexpected EOF while reading NBT payload");
	return CompressedStreamTools::decompress(compressed);
}

void Packet::writeNBTTagCompound(NBTTagCompound *compound, std::ostream &os)
{
	if (compound == nullptr)
	{
		IOUtil::writeShort(os, -1);
		return;
	}

	const std::vector<char> compressed = CompressedStreamTools::compress(compound);
	if (compressed.empty() || compressed.size() > 32767)
		throw std::runtime_error("NBT payload is outside protocol limits");
	IOUtil::writeShort(os, static_cast<int_t>(compressed.size()));
	os.write(compressed.data(), static_cast<std::streamsize>(compressed.size()));
	if (!os)
		throw std::runtime_error("Failed to write NBT payload");
}
