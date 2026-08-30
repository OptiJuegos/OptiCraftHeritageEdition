#include "Packet60Explosion.h"

#include "NetHandler.h"
#include "ChunkPosition.h"
#include "java/Arithmetic.h"
#include <stdexcept>

namespace
{
constexpr int_t MAX_EXPLOSION_BLOCKS = 8192;
}

void Packet60Explosion::readPacketData(std::istream &is)
{
	explosionX = IOUtil::readDouble(is);
	explosionY = IOUtil::readDouble(is);
	explosionZ = IOUtil::readDouble(is);
	explosionSize = IOUtil::readFloat(is);
	int_t count = IOUtil::readInt(is);
	if (count < 0 || count > MAX_EXPLOSION_BLOCKS)
		throw std::runtime_error("Invalid explosion block count: " + std::to_string(count));
	destroyedBlockPositions.clear();
	int_t j = JavaArithmetic::doubleToInt(explosionX);
	int_t k = JavaArithmetic::doubleToInt(explosionY);
	int_t l = JavaArithmetic::doubleToInt(explosionZ);
	for (int_t i = 0; i < count; i++)
	{
		int_t j1 = IOUtil::readByte(is) + j;
		int_t k1 = IOUtil::readByte(is) + k;
		int_t l1 = IOUtil::readByte(is) + l;
		destroyedBlockPositions.add(ChunkPosition(j1, k1, l1));
	}
}

void Packet60Explosion::writePacketData(std::ostream &os)
{
	IOUtil::writeDouble(os, explosionX);
	IOUtil::writeDouble(os, explosionY);
	IOUtil::writeDouble(os, explosionZ);
	IOUtil::writeFloat(os, explosionSize);
	IOUtil::writeInt(os, (int_t)destroyedBlockPositions.size());
	int_t i = JavaArithmetic::doubleToInt(explosionX);
	int_t j = JavaArithmetic::doubleToInt(explosionY);
	int_t k = JavaArithmetic::doubleToInt(explosionZ);
	for (const ChunkPosition &pos : destroyedBlockPositions.valuesInIterationOrder())
	{
		int_t l = pos.x - i;
		int_t i1 = pos.y - j;
		int_t j1 = pos.z - k;
		IOUtil::writeByte(os, l);
		IOUtil::writeByte(os, i1);
		IOUtil::writeByte(os, j1);
	}
}

void Packet60Explosion::processPacket(NetHandler &nethandler)
{
	nethandler.handleExplosion(*this);
}

int_t Packet60Explosion::getPacketSize()
{
	return 32 + (int_t)destroyedBlockPositions.size() * 3;
}
