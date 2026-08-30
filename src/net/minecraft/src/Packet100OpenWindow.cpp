#include "Packet100OpenWindow.h"
#include "java/String.h"

#include "NetHandler.h"

void Packet100OpenWindow::readPacketData(std::istream &is)
{
	windowId = IOUtil::readUnsignedByte(is);
	inventoryType = IOUtil::readUnsignedByte(is);
	windowTitle = readString(is, 32);
	slotsCount = IOUtil::readUnsignedByte(is);
}

void Packet100OpenWindow::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId & 0xff);
	IOUtil::writeByte(os, inventoryType & 0xff);
	writeString(windowTitle, os);
	IOUtil::writeByte(os, slotsCount & 0xff);
}

void Packet100OpenWindow::processPacket(NetHandler &nethandler)
{
	nethandler.handleOpenWindow(*this);
}

int_t Packet100OpenWindow::getPacketSize()
{
	return 3 + String::utf16Length(windowTitle);
}
