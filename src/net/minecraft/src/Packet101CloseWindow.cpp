#include "Packet101CloseWindow.h"

#include "NetHandler.h"

Packet101CloseWindow::Packet101CloseWindow(int_t windowId) :
	windowId(windowId)
{
}

void Packet101CloseWindow::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
}

void Packet101CloseWindow::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
}

void Packet101CloseWindow::processPacket(NetHandler &nethandler)
{
	nethandler.handleCloseWindow(*this);
}

int_t Packet101CloseWindow::getPacketSize()
{
	return 1;
}
