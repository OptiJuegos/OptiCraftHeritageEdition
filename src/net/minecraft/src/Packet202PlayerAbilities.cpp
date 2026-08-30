#include "Packet202PlayerAbilities.h"

#include "NetHandler.h"
#include "PlayerCapabilities.h"

Packet202PlayerAbilities::Packet202PlayerAbilities(const PlayerCapabilities *capabilities)
{
	if (capabilities == nullptr)
		return;
	disableDamage = capabilities->disableDamage;
	isFlying = capabilities->isFlying;
	allowFlying = capabilities->allowFlying;
	isCreativeMode = capabilities->isCreativeMode;
}

void Packet202PlayerAbilities::readPacketData(std::istream &is)
{
	disableDamage = IOUtil::readBoolean(is);
	isFlying = IOUtil::readBoolean(is);
	allowFlying = IOUtil::readBoolean(is);
	isCreativeMode = IOUtil::readBoolean(is);
}

void Packet202PlayerAbilities::writePacketData(std::ostream &os)
{
	IOUtil::writeBoolean(os, disableDamage);
	IOUtil::writeBoolean(os, isFlying);
	IOUtil::writeBoolean(os, allowFlying);
	IOUtil::writeBoolean(os, isCreativeMode);
}

void Packet202PlayerAbilities::processPacket(NetHandler &nethandler)
{
	nethandler.func_50100_a(*this);
}

int_t Packet202PlayerAbilities::getPacketSize()
{
	return 1;
}
