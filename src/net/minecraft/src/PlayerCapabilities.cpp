#include "PlayerCapabilities.h"

#include "NBTTagCompound.h"

PlayerCapabilities::PlayerCapabilities() :
	disableDamage(false), isFlying(false), allowFlying(false), isCreativeMode(false)
{
}

void PlayerCapabilities::writeCapabilitiesToNBT(NBTTagCompound *compound) const
{
	NBTTagCompound *abilities = new NBTTagCompound();
	abilities->setBoolean("invulnerable", disableDamage);
	abilities->setBoolean("flying", isFlying);
	abilities->setBoolean("mayfly", allowFlying);
	abilities->setBoolean("instabuild", isCreativeMode);
	compound->setTag("abilities", abilities);
}

void PlayerCapabilities::readCapabilitiesFromNBT(NBTTagCompound *compound)
{
	if (!compound->hasKey("abilities"))
		return;

	NBTTagCompound *abilities = compound->getCompoundTag("abilities");
	if (abilities == nullptr)
		return;
	disableDamage = abilities->getBoolean("invulnerable");
	isFlying = abilities->getBoolean("flying");
	allowFlying = abilities->getBoolean("mayfly");
	isCreativeMode = abilities->getBoolean("instabuild");
}
