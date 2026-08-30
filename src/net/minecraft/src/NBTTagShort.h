#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagShort
class NBTTagShort : public NBTBase
{
public:
	NBTTagShort();
	NBTTagShort(short_t word0);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	short_t shortValue = 0;
};
