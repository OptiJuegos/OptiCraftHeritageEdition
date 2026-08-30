#include "Packet104WindowItems.h"

#include "NetHandler.h"
#include "ItemStack.h"
#include <stdexcept>

namespace
{
constexpr int_t MAX_WINDOW_ITEMS = 256;
}

Packet104WindowItems::~Packet104WindowItems()
{
	for (ItemStack *stack : itemStack)
		delete stack;
}

ItemStack *Packet104WindowItems::releaseItemStack(size_t index)
{
	if (index >= itemStack.size())
		return nullptr;
	ItemStack *stack = itemStack[index];
	itemStack[index] = nullptr;
	return stack;
}

void Packet104WindowItems::readPacketData(std::istream &is)
{
	for (ItemStack *stack : itemStack)
		delete stack;
	itemStack.clear();
	windowId = IOUtil::readByte(is);
	const short_t count = IOUtil::readShort(is);
	if (count < 0 || count > MAX_WINDOW_ITEMS)
		throw std::runtime_error("Invalid window item count: " + std::to_string(count));
	itemStack.resize(static_cast<size_t>(count), nullptr);
	for (int_t i = 0; i < count; ++i)
		itemStack[static_cast<size_t>(i)] = readItemStack(is);
}

void Packet104WindowItems::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeShort(os, static_cast<int_t>(itemStack.size()));
	for (ItemStack *stack : itemStack)
		writeItemStack(stack, os);
}

void Packet104WindowItems::processPacket(NetHandler &nethandler)
{
	nethandler.handleWindowItems(*this);
}

int_t Packet104WindowItems::getPacketSize()
{
	return 3 + (int_t)itemStack.size() * 5;
}
