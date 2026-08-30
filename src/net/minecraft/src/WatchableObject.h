#pragma once

#include <any>
#include "java/Type.h"
#include "ItemStack.h"
#include "ChunkCoordinates.h"

// net.minecraft.src.WatchableObject
// Java Object replaced with std::any. Types 5 and 6 own their pointer payloads.
class WatchableObject
{
public:
	WatchableObject(int_t i, int_t j, std::any obj)
		: objectType(i), dataValueId(j), watchedObject(std::move(obj)), isWatching(true) {}

	~WatchableObject()
	{
		clearObject();
	}

	WatchableObject(const WatchableObject&) = delete;
	WatchableObject& operator=(const WatchableObject&) = delete;

	int_t getDataValueId() const { return dataValueId; }

	void setObject(std::any obj)
	{
		clearObject();
		watchedObject = std::move(obj);
	}

	const std::any& getObject() const { return watchedObject; }
	int_t getObjectType() const { return objectType; }
	void setWatching(bool flag) { isWatching = flag; }

private:
	void clearObject()
	{
		if (!watchedObject.has_value())
			return;

		try
		{
			if (objectType == 5 && watchedObject.type() == typeid(ItemStack*))
			{
				delete std::any_cast<ItemStack*>(watchedObject);
			}
			else if (objectType == 6 && watchedObject.type() == typeid(ChunkCoordinates*))
			{
				delete std::any_cast<ChunkCoordinates*>(watchedObject);
			}
		}
		catch (const std::bad_any_cast&)
		{
			// Keep destructor noexcept-like. A malformed watcher should not crash
			// while cleaning up a packet/entity during shutdown.
		}

		watchedObject.reset();
	}

	const int_t objectType;
	const int_t dataValueId;
	std::any watchedObject;
	bool isWatching;
};
