#pragma once

// net.minecraft.src.IUpdateListener (OptiFine)
// Callback que el UpdateThread invoca periodicamente mientras construye un chunk,
// para ceder/intercambiar el Tessellator del hilo (ver UpdateThread / F7 en OPTIFINE_PORT.md).
class IUpdateListener
{
public:
	virtual ~IUpdateListener() = default;
	virtual void updating() = 0;
};
