#include "Packet2Handshake.h"
#include "java/String.h"

#include "NetHandler.h"

Packet2Handshake::Packet2Handshake(const jstring &s) :
	username(s)
{
}

void Packet2Handshake::readPacketData(std::istream &is)
{
	username = Packet::readString(is, 64);
}

void Packet2Handshake::writePacketData(std::ostream &os)
{
	Packet::writeString(username, os);
}

void Packet2Handshake::processPacket(NetHandler &nethandler)
{
	nethandler.handleHandshake(*this);
}

int_t Packet2Handshake::getPacketSize()
{
	return 4 + String::utf16Length(username) + 4;
}
