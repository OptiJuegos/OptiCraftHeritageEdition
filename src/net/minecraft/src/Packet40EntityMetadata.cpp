#include "Packet40EntityMetadata.h"

#include "NetHandler.h"
#include "DataWatcher.h"
#include "WatchableObject.h"

static void clearWatchableList(std::vector<WatchableObject*> &list)
{
	for (WatchableObject *obj : list)
		delete obj;
	list.clear();
}

Packet40EntityMetadata::~Packet40EntityMetadata()
{
	clearWatchableList(metadataList);
}


void Packet40EntityMetadata::readPacketData(std::istream &is)
{
	entityId     = IOUtil::readInt(is);
	clearWatchableList(metadataList);
	metadataList = DataWatcher::readWatchableObjects(is);
}

void Packet40EntityMetadata::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	DataWatcher::writeObjectsInListToStream(metadataList, os);
}

void Packet40EntityMetadata::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityMetadata(*this);
}

int_t Packet40EntityMetadata::getPacketSize()
{
	return 5;
}

std::vector<WatchableObject *> Packet40EntityMetadata::getMetadata()
{
	return metadataList;
}
