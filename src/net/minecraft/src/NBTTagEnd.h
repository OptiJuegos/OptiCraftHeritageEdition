#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagEnd
class NBTTagEnd : public NBTBase
{
public:
	NBTTagEnd();

	void readTagContents(std::istream &datainput) override;
	void writeTagContents(std::ostream &dataoutput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;
};
