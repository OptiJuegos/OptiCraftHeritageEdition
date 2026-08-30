#include "NaturalProperties.h"

NaturalProperties::NaturalProperties(const std::string &type)
	: rotation(1), flip(false)
{
	if (type == "4" || type == "4F") rotation = 4;
	else if (type == "2" || type == "2F") rotation = 2;
	if (type == "F" || type == "4F" || type == "2F") flip = true;
}

bool NaturalProperties::isValid() const
{
	return rotation == 2 || rotation == 4 || flip;
}
