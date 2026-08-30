#pragma once

#include "Packet.h"

class NetHandler;
class PlayerCapabilities;

// net.minecraft.src.Packet202PlayerAbilities
class Packet202PlayerAbilities : public Packet
{
public:
	Packet202PlayerAbilities() = default;
	explicit Packet202PlayerAbilities(const PlayerCapabilities *capabilities);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	bool disableDamage = false;
	bool isFlying = false;
	bool allowFlying = false;
	bool isCreativeMode = false;
};
