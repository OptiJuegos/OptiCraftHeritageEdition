#pragma once

#include "java/String.h"
#include "java/Type.h"

// net.minecraft.src.IProgressUpdate
class IProgressUpdate
{
public:
	virtual ~IProgressUpdate() = default;
	virtual void displaySavingString(const jstring &s) = 0;
	virtual void displayLoadingString(const jstring &s) = 0;
	virtual void resetProgressAndMessage(const jstring &s) { displaySavingString(s); }
	virtual void setLoadingProgress(int_t i) = 0;
};
