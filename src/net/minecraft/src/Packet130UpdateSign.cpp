#include "Packet130UpdateSign.h"

#include "NetHandler.h"
#include "java/String.h"

Packet130UpdateSign::Packet130UpdateSign()
{
	isChunkDataPacket = true;
}

void Packet130UpdateSign::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readShort(is);
	zPosition = IOUtil::readInt(is);
	for (int_t i = 0; i < 4; i++)
	{
		signLines[i] = Packet::readString(is, 15);
	}
}

void Packet130UpdateSign::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeShort(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	for (int_t i = 0; i < 4; i++)
	{
		Packet::writeString(signLines[i], os);
	}
}

void Packet130UpdateSign::processPacket(NetHandler &nethandler)
{
	nethandler.handleUpdateSign(*this);
}

int_t Packet130UpdateSign::getPacketSize()
{
	int_t size = 0;
	for (int_t i = 0; i < 4; i++)
		size += String::utf16Length(signLines[i]);
	return size;
}
