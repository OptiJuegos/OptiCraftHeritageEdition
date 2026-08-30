#include "StatCollector.h"

#include "StringTranslate.h"

StringTranslate *StatCollector::localizedName = StringTranslate::getInstance();

std::string StatCollector::translateToLocal(const std::string &s)
{
	if (localizedName == nullptr) return s;
	return localizedName->translateKey(s);
}

std::string StatCollector::translateToLocalFormatted(const std::string &s, const std::string &arg)
{
	if (localizedName == nullptr) return s;
	return localizedName->translateKeyFormat(s, arg);
}
