#pragma once

class NBTTagCompound;

// net.minecraft.src.PlayerCapabilities
class PlayerCapabilities
{
public:
	PlayerCapabilities();

	void writeCapabilitiesToNBT(NBTTagCompound *compound) const;
	void readCapabilitiesFromNBT(NBTTagCompound *compound);

	bool disableDamage;
	bool isFlying;
	bool allowFlying;
	bool isCreativeMode;
};
