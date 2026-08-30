#include "Packet131MapData.h"

#include "NetHandler.h"
#include <stdexcept>


Packet131MapData::Packet131MapData()
{
	isChunkDataPacket = true;
}

void Packet131MapData::readPacketData(std::istream &is)
{
	itemId = IOUtil::readShort(is);
	mapId = IOUtil::readShort(is);
	int_t len = IOUtil::readByte(is) & 0xff;
	data.resize((size_t)len);
	is.read(reinterpret_cast<char*>(data.data()), len);
	if (!is)
		throw std::runtime_error("Truncated map data packet");
}

void Packet131MapData::writePacketData(std::ostream &os)
{
	IOUtil::writeShort(os, itemId);
	IOUtil::writeShort(os, mapId);
	if (data.size() > 255)
		throw std::runtime_error("Map data packet payload exceeds 255 bytes");
	IOUtil::writeByte(os, (byte_t)data.size());
	os.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void Packet131MapData::processPacket(NetHandler &nethandler)
{
	nethandler.handleMapData(*this);
}

int_t Packet131MapData::getPacketSize()
{
	return 4 + (int_t)data.size();
}
