#include "Packet107CreativeSetSlot.h"

#include "ItemStack.h"
#include "NetHandler.h"

Packet107CreativeSetSlot::Packet107CreativeSetSlot(int_t slotIn, ItemStack *stack) :
	slot(slotIn),
	itemStack(stack != nullptr ? stack->copy() : nullptr)
{
}

Packet107CreativeSetSlot::~Packet107CreativeSetSlot()
{
	delete itemStack;
}

void Packet107CreativeSetSlot::readPacketData(std::istream &is)
{
	delete itemStack;
	itemStack = nullptr;
	slot = IOUtil::readShort(is);
	itemStack = readItemStack(is);
}

void Packet107CreativeSetSlot::writePacketData(std::ostream &os)
{
	IOUtil::writeShort(os, slot);
	writeItemStack(itemStack, os);
}

void Packet107CreativeSetSlot::processPacket(NetHandler &nethandler)
{
	nethandler.handleCreativeSetSlot(*this);
}

int_t Packet107CreativeSetSlot::getPacketSize()
{
	return 8;
}
