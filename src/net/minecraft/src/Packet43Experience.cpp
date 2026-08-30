#include "Packet43Experience.h"

#include "NetHandler.h"

void Packet43Experience::readPacketData(std::istream &is)
{
	experience = IOUtil::readFloat(is);
	experienceLevel = IOUtil::readShort(is);
	experienceTotal = IOUtil::readShort(is);
}

void Packet43Experience::writePacketData(std::ostream &os)
{
	IOUtil::writeFloat(os, experience);
	IOUtil::writeShort(os, experienceLevel);
	IOUtil::writeShort(os, experienceTotal);
}

void Packet43Experience::processPacket(NetHandler &nethandler)
{
	nethandler.handleExperience(*this);
}

int_t Packet43Experience::getPacketSize()
{
	return 4;
}
