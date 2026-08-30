#include "Packet52MultiBlockChange.h"

#include "NetHandler.h"

#include <stdexcept>

Packet52MultiBlockChange::Packet52MultiBlockChange() :
	xPosition(0),
	zPosition(0),
	size(0)
{
	isChunkDataPacket = true;
}

void Packet52MultiBlockChange::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	size = (int_t)(ushort_t)IOUtil::readShort(is);

	const int_t payloadLength = IOUtil::readInt(is);
	if (payloadLength < 0 || payloadLength > 4 * 65535)
		throw std::runtime_error("Invalid multi-block payload length: " + std::to_string(payloadLength));
	if (payloadLength < size * 4)
		throw std::runtime_error("Truncated multi-block payload");

	metadataArray.resize((size_t)payloadLength);
	if (payloadLength > 0)
	{
		is.read(reinterpret_cast<char *>(metadataArray.data()), payloadLength);
		if (!is)
			throw std::runtime_error("Truncated multi-block change data");
	}
}

void Packet52MultiBlockChange::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeShort(os, (short_t)size);
	IOUtil::writeInt(os, (int_t)metadataArray.size());
	if (!metadataArray.empty())
		os.write(reinterpret_cast<const char *>(metadataArray.data()), (std::streamsize)metadataArray.size());
}

void Packet52MultiBlockChange::processPacket(NetHandler &nethandler)
{
	nethandler.handleMultiBlockChange(*this);
}

int_t Packet52MultiBlockChange::getPacketSize()
{
	return 10 + size * 4;
}
