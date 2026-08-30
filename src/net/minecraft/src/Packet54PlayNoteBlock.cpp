#include "Packet54PlayNoteBlock.h"

#include "NetHandler.h"

void Packet54PlayNoteBlock::readPacketData(std::istream &is)
{
	xLocation = IOUtil::readInt(is);
	yLocation = IOUtil::readShort(is);
	zLocation = IOUtil::readInt(is);
	instrumentType = IOUtil::readUnsignedByte(is);
	pitch = IOUtil::readUnsignedByte(is);
}

void Packet54PlayNoteBlock::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xLocation);
	IOUtil::writeShort(os, yLocation);
	IOUtil::writeInt(os, zLocation);
	os.put(instrumentType);
	os.put(pitch);
}

void Packet54PlayNoteBlock::processPacket(NetHandler &nethandler)
{
	nethandler.handlePlayNoteBlock(*this);
}

int_t Packet54PlayNoteBlock::getPacketSize()
{
	return 12;
}
