#include "Packet102WindowClick.h"

#include "NetHandler.h"
#include "ItemStack.h"

Packet102WindowClick::Packet102WindowClick(int_t windowId, int_t slot, int_t mouseClick, bool shift, ItemStack *itemstack, short_t action) :
	window_Id(windowId),
	inventorySlot(slot),
	mouseClick(mouseClick),
	action(action),
	itemStack(itemstack != nullptr ? itemstack->copy() : nullptr),
	ownsItemStack(itemstack != nullptr),
	shiftPressed(shift)
{
}

Packet102WindowClick::~Packet102WindowClick()
{
	if (ownsItemStack) delete itemStack;
}

void Packet102WindowClick::readPacketData(std::istream &is)
{
	if (ownsItemStack) delete itemStack;
	itemStack = nullptr;
	ownsItemStack = false;
	window_Id = IOUtil::readByte(is);
	inventorySlot = IOUtil::readShort(is);
	mouseClick = IOUtil::readByte(is);
	action = IOUtil::readShort(is);
	shiftPressed = IOUtil::readBoolean(is);
	itemStack = readItemStack(is);
	ownsItemStack = itemStack != nullptr;
}

void Packet102WindowClick::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, window_Id);
	IOUtil::writeShort(os, inventorySlot);
	IOUtil::writeByte(os, mouseClick);
	IOUtil::writeShort(os, action);
	IOUtil::writeBoolean(os, shiftPressed);
	writeItemStack(itemStack, os);
}

void Packet102WindowClick::processPacket(NetHandler &nethandler)
{
	nethandler.handleWindowClick(*this);
}

int_t Packet102WindowClick::getPacketSize()
{
	return 11;
}
