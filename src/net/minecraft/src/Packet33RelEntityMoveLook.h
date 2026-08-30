#pragma once

#include "Packet30Entity.h"

// net.minecraft.src.Packet33RelEntityMoveLook
class Packet33RelEntityMoveLook : public Packet30Entity
{
public:
	Packet33RelEntityMoveLook();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	int_t getPacketSize() override;
};
