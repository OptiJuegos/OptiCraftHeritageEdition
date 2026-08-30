#pragma once

#include "Packet30Entity.h"

// net.minecraft.src.Packet32EntityLook
class Packet32EntityLook : public Packet30Entity
{
public:
	Packet32EntityLook();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	int_t getPacketSize() override;
};
