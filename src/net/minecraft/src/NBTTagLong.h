#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagLong
class NBTTagLong : public NBTBase
{
public:
	NBTTagLong();
	NBTTagLong(long_t l);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	long_t longValue = 0;
};
