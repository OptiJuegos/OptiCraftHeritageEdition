#pragma once

#include <vector>

#include "NBTBase.h"

// net.minecraft.src.NBTTagIntArray
class NBTTagIntArray : public NBTBase
{
public:
    NBTTagIntArray();
    explicit NBTTagIntArray(const std::vector<int_t> &values);

    void writeTagContents(std::ostream &dataoutput) override;
    void readTagContents(std::istream &datainput) override;
    byte_t getType() override;
    NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
    jstring toString() override;

    std::vector<int_t> intArray;
};
