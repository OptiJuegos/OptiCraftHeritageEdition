#pragma once

#include "java/String.h"

// net.minecraft.src.ProfilerResult
class ProfilerResult
{
public:
	ProfilerResult(const jstring &name, double sectionPercentage, double globalPercentage);

	int compareProfilerResult(const ProfilerResult &other) const;
	int getDisplayColor() const;

	double sectionPercentage;
	double globalPercentage;
	jstring name;
};
