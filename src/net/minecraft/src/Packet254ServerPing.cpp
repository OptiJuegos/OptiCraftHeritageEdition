#include "Packet254ServerPing.h"

#include "NetHandler.h"

void Packet254ServerPing::processPacket(NetHandler &nethandler)
{
	nethandler.handleServerPing(*this);
}
