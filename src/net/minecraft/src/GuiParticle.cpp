#include "GuiParticle.h"
#include "Particle.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"

GuiParticle::GuiParticle(Minecraft *minecraft)
	: mc(minecraft)
{
}

void GuiParticle::updateParticles()
{
	for (int_t i = 0; i < (int_t)particles.size(); i++)
	{
		Particle *p = particles[i];
		p->updateParticle();
		p->renderIntoGui(this);
		if (p->isDead)
		{
			particles.erase(particles.begin() + i);
			i--;
		}
	}
}

void GuiParticle::updateAndRender(float_t partialTick)
{
	updateParticles();
	renderParticles(partialTick);
}

void GuiParticle::renderParticles(float_t partialTick)
{
	mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/gui/particles.png"));
	for (int_t i = 0; i < (int_t)particles.size(); i++)
	{
		Particle *p = particles[i];
		int_t x = (int_t)((p->prevPosX + (p->posX - p->prevPosX) * partialTick) - 4.0f);
		int_t y = (int_t)((p->prevPosY + (p->posY - p->prevPosY) * partialTick) - 4.0f);
		float_t alpha  = p->prevAlpha + (p->alpha - p->prevAlpha) * partialTick;
		float_t red    = p->prevRed + (p->red - p->prevRed) * partialTick;
		float_t green  = p->prevGreen + (p->green - p->prevGreen) * partialTick;
		float_t blue   = p->prevBlue + (p->blue - p->prevBlue) * partialTick;
		renderColor4f(red, green, blue, alpha);
		drawTexturedModalRect(x, y, 40, 0, 8, 8);
	}
}
