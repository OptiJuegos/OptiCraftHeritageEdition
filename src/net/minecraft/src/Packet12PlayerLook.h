#pragma once

#include "Packet10Flying.h"

// net.minecraft.src.Packet12PlayerLook
class Packet12PlayerLook : public Packet10Flying
{
public:
	Packet12PlayerLook();
	Packet12PlayerLook(float f, float f1, bool flag);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	int_t getPacketSize() override;
};
