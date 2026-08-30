#include "StatTypeDistance.h"

#include <sstream>
#include <iomanip>

StatTypeDistance::StatTypeDistance()
{
}

std::string StatTypeDistance::format(int_t i)
{
	int_t j = i;
	double d = (double)j / 100.0;
	double d1 = d / 1000.0;
	if (d1 > 0.5)
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << d1 << " km";
		return oss.str();
	}
	if (d > 0.5)
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << d << " m";
		return oss.str();
	}
	return std::to_string(i) + " cm";
}
