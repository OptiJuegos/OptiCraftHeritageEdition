#include "StatCrafting.h"

StatCrafting::StatCrafting(int_t i, const std::string &s, int_t j) :
	StatBase(i, s),
	itemID(j)
{
}

int_t StatCrafting::getItemID()
{
	return itemID;
}
