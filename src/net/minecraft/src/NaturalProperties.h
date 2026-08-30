#pragma once

#include <string>

class NaturalProperties
{
public:
	explicit NaturalProperties(const std::string &type = std::string());
	bool isValid() const;

	int rotation;
	bool flip;
};
