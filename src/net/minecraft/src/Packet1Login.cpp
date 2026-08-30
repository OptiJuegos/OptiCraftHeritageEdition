#include "Packet1Login.h"

#include "NetHandler.h"
#include "WorldType.h"
#include "java/String.h"

Packet1Login::Packet1Login(const jstring &usernameIn, int_t protocolVersionIn) :
	protocolVersion(protocolVersionIn),
	username(usernameIn)
{
}

void Packet1Login::readPacketData(std::istream &is)
{
	WorldType::initialize();
	protocolVersion = IOUtil::readInt(is);
	username = readString(is, 16);
	const jstring terrainName = readString(is, 16);
	terrainType = WorldType::parseWorldType(terrainName);
	if (terrainType == nullptr)
		terrainType = WorldType::DEFAULT;
	serverMode = IOUtil::readInt(is);
	dimension = IOUtil::readInt(is);
	difficultySetting = IOUtil::readByte(is);
	worldHeight = IOUtil::readByte(is);
	maxPlayers = IOUtil::readByte(is);
}

void Packet1Login::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, protocolVersion);
	writeString(username, os);
	writeString(terrainType != nullptr ? terrainType->func_48628_a() : jstring(), os);
	IOUtil::writeInt(os, serverMode);
	IOUtil::writeInt(os, dimension);
	IOUtil::writeByte(os, difficultySetting);
	IOUtil::writeByte(os, worldHeight);
	IOUtil::writeByte(os, maxPlayers);
}

void Packet1Login::processPacket(NetHandler &nethandler)
{
	nethandler.handleLogin(*this);
}

int_t Packet1Login::getPacketSize()
{
	const int_t terrainLength = terrainType != nullptr ? static_cast<int_t>(String::utf16Length(terrainType->func_48628_a())) : 0;
	return 4 + static_cast<int_t>(String::utf16Length(username)) + 4 + 7 + 7 + terrainLength;
}
