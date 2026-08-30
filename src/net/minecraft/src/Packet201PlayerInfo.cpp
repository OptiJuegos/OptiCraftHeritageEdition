#include "Packet201PlayerInfo.h"

#include "NetHandler.h"

void Packet201PlayerInfo::readPacketData(std::istream &is)
{
	playerName = readString(is, 16);
	isConnected = IOUtil::readByte(is) != 0;
	ping = IOUtil::readShort(is);
}

void Packet201PlayerInfo::writePacketData(std::ostream &os)
{
	writeString(playerName, os);
	IOUtil::writeByte(os, isConnected ? 1 : 0);
	IOUtil::writeShort(os, ping);
}

void Packet201PlayerInfo::processPacket(NetHandler &nethandler)
{
	nethandler.handlePlayerInfo(*this);
}

int_t Packet201PlayerInfo::getPacketSize()
{
	return static_cast<int_t>(String::toUtf16(playerName).size()) + 5;
}
