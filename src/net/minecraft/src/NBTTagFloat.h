#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagFloat
class NBTTagFloat : public NBTBase
{
public:
	NBTTagFloat();
	NBTTagFloat(float f);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	float floatValue = 0.0f;
};
