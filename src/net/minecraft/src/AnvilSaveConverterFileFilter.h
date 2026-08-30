#pragma once

#include <string>

class AnvilSaveConverter;

// net.minecraft.src.AnvilSaveConverterFileFilter
class AnvilSaveConverterFileFilter
{
public:
    explicit AnvilSaveConverterFileFilter(AnvilSaveConverter *converter) : parent(converter) {}

    bool accept(const std::string &name) const;

private:
    AnvilSaveConverter *parent;
};
