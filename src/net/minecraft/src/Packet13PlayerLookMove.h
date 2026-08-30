#pragma once

#include "Packet10Flying.h"

// net.minecraft.src.Packet13PlayerLookMove
class Packet13PlayerLookMove : public Packet10Flying
{
public:
	Packet13PlayerLookMove();
	Packet13PlayerLookMove(double d, double d1, double d2, double d3, float f, float f1, bool flag);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	int_t getPacketSize() override;
};
