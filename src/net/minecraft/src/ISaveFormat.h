#pragma once

#include <vector>
#include "java/String.h"

class ISaveHandler;
class WorldInfo;
class IProgressUpdate;
class SaveFormatComparator;

// net.minecraft.src.ISaveFormat
class ISaveFormat
{
public:
	virtual ~ISaveFormat() = default;
	virtual jstring getSaveFormatName() = 0;                                    // returns save format name
	virtual ISaveHandler *getSaveLoader(const jstring &s, bool flag) = 0;
	virtual std::vector<SaveFormatComparator *> getSaveList() = 0;         // func_22176_b
	virtual void flushCache() = 0;
	virtual WorldInfo *getWorldInfo(const jstring &s) = 0;                 // func_22173_b
	virtual void deleteWorldDirectory(const jstring &s) = 0;               // func_22172_c
	virtual void renameWorld(const jstring &s, const jstring &s1) = 0;     // func_22170_a
	virtual bool isOldMapFormat(const jstring &s) = 0;
	virtual bool convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate) = 0;
};
