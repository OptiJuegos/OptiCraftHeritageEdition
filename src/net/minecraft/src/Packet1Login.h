#pragma once

#include "Packet.h"

class NetHandler;
class WorldType;

// net.minecraft.src.Packet1Login
class Packet1Login : public Packet
{
public:
	Packet1Login() = default;
	Packet1Login(const jstring &username, int_t protocolVersion);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t protocolVersion = 0;
	jstring username;
	WorldType *terrainType = nullptr;
	int_t serverMode = 0;
	int_t dimension = 0;
	byte_t difficultySetting = 0;
	byte_t worldHeight = 0;
	byte_t maxPlayers = 0;
};
