#include "Packet9Respawn.h"

#include "NetHandler.h"
#include "WorldType.h"
#include "java/String.h"

Packet9Respawn::Packet9Respawn(int_t dimensionIn, byte_t difficultyIn, WorldType *terrainTypeIn,
                               int_t worldHeightIn, int_t creativeModeIn) :
	respawnDimension(dimensionIn),
	difficulty(difficultyIn),
	worldHeight(worldHeightIn),
	creativeMode(creativeModeIn),
	terrainType(terrainTypeIn)
{
	WorldType::initialize();
	if (terrainType == nullptr)
		terrainType = WorldType::DEFAULT;
}

void Packet9Respawn::readPacketData(std::istream &is)
{
	WorldType::initialize();
	respawnDimension = IOUtil::readInt(is);
	difficulty = IOUtil::readByte(is);
	creativeMode = IOUtil::readByte(is);
	worldHeight = IOUtil::readShort(is);
	terrainType = WorldType::parseWorldType(readString(is, 16));
	if (terrainType == nullptr)
		terrainType = WorldType::DEFAULT;
}

void Packet9Respawn::writePacketData(std::ostream &os)
{
	WorldType::initialize();
	WorldType *type = terrainType != nullptr ? terrainType : WorldType::DEFAULT;
	IOUtil::writeInt(os, respawnDimension);
	IOUtil::writeByte(os, difficulty);
	IOUtil::writeByte(os, creativeMode);
	IOUtil::writeShort(os, static_cast<short_t>(worldHeight));
	writeString(type->func_48628_a(), os);
}

void Packet9Respawn::processPacket(NetHandler &nethandler)
{
	nethandler.handleRespawn(*this);
}

int_t Packet9Respawn::getPacketSize()
{
	WorldType::initialize();
	WorldType *type = terrainType != nullptr ? terrainType : WorldType::DEFAULT;
	return 8 + static_cast<int_t>(String::utf16Length(type->func_48628_a()));
}
