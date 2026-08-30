#pragma once

#include "Gui.h"
#include <vector>

class Minecraft;
class Particle;

// net.minecraft.src.GuiParticle
class GuiParticle : public Gui
{
public:
	GuiParticle(Minecraft *minecraft);

	void updateParticles();
	void renderParticles(float_t partialTick);
	void updateAndRender(float_t partialTick);

private:
	std::vector<Particle *> particles;
	Minecraft *mc;
};
