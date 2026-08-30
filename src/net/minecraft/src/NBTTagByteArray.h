#pragma once

#include <vector>

#include "NBTBase.h"

// net.minecraft.src.NBTTagByteArray
class NBTTagByteArray : public NBTBase
{
public:
	NBTTagByteArray();
	NBTTagByteArray(const std::vector<byte_t> &abyte0);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	std::vector<byte_t> byteArray;
};
