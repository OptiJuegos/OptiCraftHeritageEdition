#pragma once

#include "java/String.h"
#include "java/Type.h"
#include <mutex>

class NBTTagCompound;

// net.minecraft.src.ServerNBTStorage
class ServerNBTStorage
{
public:
	ServerNBTStorage(const jstring &name, const jstring &host);

	NBTTagCompound *getCompoundTag() const;
	static ServerNBTStorage *createServerNBTStorage(NBTTagCompound *tag);

	jstring name;
	jstring host;
	jstring playerCount;
	jstring motd;
	long_t lag;
	bool polled;
	mutable std::mutex stateMutex;
};
