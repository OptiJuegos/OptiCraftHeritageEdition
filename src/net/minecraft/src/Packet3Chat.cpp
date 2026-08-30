#include "Packet3Chat.h"

#include "NetHandler.h"
#include "java/String.h"

Packet3Chat::Packet3Chat(const jstring &s)
{
	if (String::utf16Length(s) > 119)
		message = String::truncateUtf16(s, 119);
	else
		message = s;
}

void Packet3Chat::readPacketData(std::istream &is)
{
	message = Packet::readString(is, 119);
}

void Packet3Chat::writePacketData(std::ostream &os)
{
	Packet::writeString(message, os);
}

void Packet3Chat::processPacket(NetHandler &nethandler)
{
	nethandler.handleChat(*this);
}

int_t Packet3Chat::getPacketSize()
{
	return 2 + String::utf16Length(message) * 2;
}
