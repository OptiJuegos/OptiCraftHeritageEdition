#include "Packet105UpdateProgressbar.h"

#include "NetHandler.h"

void Packet105UpdateProgressbar::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
	progressBar = IOUtil::readShort(is);
	progressBarValue = IOUtil::readShort(is);
}

void Packet105UpdateProgressbar::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeShort(os, progressBar);
	IOUtil::writeShort(os, progressBarValue);
}

void Packet105UpdateProgressbar::processPacket(NetHandler &nethandler)
{
	nethandler.handleUpdateProgressbar(*this);
}

int_t Packet105UpdateProgressbar::getPacketSize()
{
	return 5;
}
