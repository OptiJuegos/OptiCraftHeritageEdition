#include "Packet103SetSlot.h"

#include "NetHandler.h"
#include "ItemStack.h"

Packet103SetSlot::~Packet103SetSlot()
{
	delete myItemStack;
}

ItemStack *Packet103SetSlot::releaseItemStack()
{
	ItemStack *stack = myItemStack;
	myItemStack = nullptr;
	return stack;
}

void Packet103SetSlot::readPacketData(std::istream &is)
{
	delete myItemStack;
	myItemStack = nullptr;
	windowId = IOUtil::readByte(is);
	itemSlot = IOUtil::readShort(is);
	myItemStack = readItemStack(is);
}

void Packet103SetSlot::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeShort(os, itemSlot);
	writeItemStack(myItemStack, os);
}

void Packet103SetSlot::processPacket(NetHandler &nethandler)
{
	nethandler.handleSetSlot(*this);
}

int_t Packet103SetSlot::getPacketSize()
{
	return 8;
}
