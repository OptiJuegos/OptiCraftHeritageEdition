#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagString
class NBTTagString : public NBTBase
{
public:
	NBTTagString();
	NBTTagString(const jstring &s);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	jstring stringValue;
};
