#include "Packet250CustomPayload.h"

#include <stdexcept>

#include "NetHandler.h"

void Packet250CustomPayload::readPacketData(std::istream &is)
{
	channel = readString(is, 16);
	length = IOUtil::readShort(is);
	if (length <= 0)
	{
		data.clear();
		return;
	}
	if (length >= 32767)
		throw std::runtime_error("Invalid custom payload length: " + std::to_string(length));

	data.resize(static_cast<size_t>(length));
	is.read(reinterpret_cast<char *>(data.data()), length);
	if (!is)
		throw std::runtime_error("Unexpected EOF while reading custom payload");
}

void Packet250CustomPayload::writePacketData(std::ostream &os)
{
	if (length < 0 || length >= 32767)
		throw std::runtime_error("Invalid custom payload length: " + std::to_string(length));
	if (static_cast<size_t>(length) > data.size())
		throw std::runtime_error("Custom payload length exceeds data size");
	writeString(channel, os);
	IOUtil::writeShort(os, length);
	if (length > 0)
	{
		os.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(length));
		if (!os)
			throw std::runtime_error("Failed to write custom payload");
	}
}

void Packet250CustomPayload::processPacket(NetHandler &nethandler)
{
	nethandler.handleCustomPayload(*this);
}

int_t Packet250CustomPayload::getPacketSize()
{
	return 4 + static_cast<int_t>(String::toUtf16(channel).size()) * 2 + length;
}
