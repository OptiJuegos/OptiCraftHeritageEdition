#include "ProfilerResult.h"

#include "java/String.h"

ProfilerResult::ProfilerResult(const jstring &sectionName, double section, double global) :
	sectionPercentage(section), globalPercentage(global), name(sectionName)
{
}

int ProfilerResult::compareProfilerResult(const ProfilerResult &other) const
{
	if (other.sectionPercentage < sectionPercentage)
		return -1;
	if (other.sectionPercentage > sectionPercentage)
		return 1;
	return other.name.compare(name);
}

int ProfilerResult::getDisplayColor() const
{
	return (String::hashCode(name) & 11184810) + 4473924;
}
