#include "StatTypeSimple.h"

#include "StatBase.h"

StatTypeSimple::StatTypeSimple()
{
}

std::string StatTypeSimple::format(int_t i)
{
	return StatBase::numberFormat(i);
}
