#pragma once

#include "Packet10Flying.h"

// net.minecraft.src.Packet11PlayerPosition
class Packet11PlayerPosition : public Packet10Flying
{
public:
	Packet11PlayerPosition();
	Packet11PlayerPosition(double d, double d1, double d2, double d3, bool flag);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	int_t getPacketSize() override;
};
