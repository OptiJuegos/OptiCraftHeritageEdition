#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagDouble
class NBTTagDouble : public NBTBase
{
public:
	NBTTagDouble();
	NBTTagDouble(double d);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	double doubleValue = 0.0;
};
