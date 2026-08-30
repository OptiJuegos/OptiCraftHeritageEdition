#pragma once

#include "Packet30Entity.h"

// net.minecraft.src.Packet31RelEntityMove
class Packet31RelEntityMove : public Packet30Entity
{
public:
	Packet31RelEntityMove() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	int_t getPacketSize() override;
};
