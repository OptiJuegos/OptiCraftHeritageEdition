#include "Packet70Bed.h"

#include "NetHandler.h"

const jstring Packet70Bed::bedMessages[] = {
	"tile.bed.notValid",
	"",
	"",
	"gameMode.changed"
};
const int_t Packet70Bed::bedMessageCount = 4;

void Packet70Bed::readPacketData(std::istream &is)
{
	bedState = IOUtil::readByte(is);
	gameMode = IOUtil::readByte(is);
}

void Packet70Bed::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, bedState);
	IOUtil::writeByte(os, gameMode);
}

void Packet70Bed::processPacket(NetHandler &nethandler)
{
	nethandler.handleBed(*this);
}

int_t Packet70Bed::getPacketSize()
{
	return 2;
}
