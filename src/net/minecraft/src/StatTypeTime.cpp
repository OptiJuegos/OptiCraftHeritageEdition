#include "StatTypeTime.h"

#include <sstream>
#include <iomanip>

StatTypeTime::StatTypeTime()
{
}

std::string StatTypeTime::format(int_t i)
{
	double d = (double)i / 20.0;
	double d1 = d / 60.0;
	double d2 = d1 / 60.0;
	double d3 = d2 / 24.0;
	double d4 = d3 / 365.0;

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2);

	if (d4 > 0.5)
	{
		oss << d4 << " y";
		return oss.str();
	}
	if (d3 > 0.5)
	{
		oss << d3 << " d";
		return oss.str();
	}
	if (d2 > 0.5)
	{
		oss << d2 << " h";
		return oss.str();
	}
	if (d1 > 0.5)
	{
		oss << d1 << " m";
		return oss.str();
	}
	return std::to_string((int)d) + " s";
}
