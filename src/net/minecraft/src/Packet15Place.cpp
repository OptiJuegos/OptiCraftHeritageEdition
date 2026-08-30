#include "Packet15Place.h"

#include "NetHandler.h"
#include "ItemStack.h"

Packet15Place::Packet15Place(int_t x, int_t y, int_t z, int_t direction, ItemStack *itemstack) :
	xPosition(x),
	yPosition(y),
	zPosition(z),
	direction(direction),
	itemStack(itemstack != nullptr ? itemstack->copy() : nullptr),
	ownsItemStack(itemstack != nullptr)
{
}

Packet15Place::~Packet15Place()
{
	if (ownsItemStack) delete itemStack;
}

void Packet15Place::readPacketData(std::istream &is)
{
	if (ownsItemStack) delete itemStack;
	itemStack = nullptr;
	ownsItemStack = false;
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readUnsignedByte(is);
	zPosition = IOUtil::readInt(is);
	direction = IOUtil::readUnsignedByte(is);
	itemStack = readItemStack(is);
	ownsItemStack = itemStack != nullptr;
}

void Packet15Place::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeByte(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeByte(os, direction);
	writeItemStack(itemStack, os);
}

void Packet15Place::processPacket(NetHandler &nethandler)
{
	nethandler.handlePlace(*this);
}

int_t Packet15Place::getPacketSize()
{
	return 15;
}
