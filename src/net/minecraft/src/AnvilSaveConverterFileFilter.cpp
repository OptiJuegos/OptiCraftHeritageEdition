#include "AnvilSaveConverterFileFilter.h"

bool AnvilSaveConverterFileFilter::accept(const std::string &name) const
{
    return name.size() >= 4 && name.compare(name.size() - 4, 4, ".mcr") == 0;
}
