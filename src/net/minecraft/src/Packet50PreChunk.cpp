#include "Packet50PreChunk.h"

#include "NetHandler.h"

Packet50PreChunk::Packet50PreChunk() :
	xPosition(0),
	yPosition(0),
	mode(false)
{
	isChunkDataPacket = false;
}

void Packet50PreChunk::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	mode = IOUtil::readUnsignedByte(is) != 0;
}

void Packet50PreChunk::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	os.put(mode ? 1 : 0);
}

void Packet50PreChunk::processPacket(NetHandler &nethandler)
{
	nethandler.handlePreChunk(*this);
}

int_t Packet50PreChunk::getPacketSize()
{
	return 9;
}
