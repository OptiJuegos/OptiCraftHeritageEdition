#include "Packet255KickDisconnect.h"
#include "java/String.h"

#include "NetHandler.h"

Packet255KickDisconnect::Packet255KickDisconnect(const jstring &r) :
	reason(r)
{
}

void Packet255KickDisconnect::readPacketData(std::istream &is)
{
	reason = Packet::readString(is, 256);
}

void Packet255KickDisconnect::writePacketData(std::ostream &os)
{
	Packet::writeString(reason, os);
}

void Packet255KickDisconnect::processPacket(NetHandler &nethandler)
{
	nethandler.handleKickDisconnect(*this);
}

int_t Packet255KickDisconnect::getPacketSize()
{
	return String::utf16Length(reason);
}
