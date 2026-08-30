#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagInt
class NBTTagInt : public NBTBase
{
public:
	NBTTagInt();
	NBTTagInt(int_t i);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	int_t intValue = 0;
};
