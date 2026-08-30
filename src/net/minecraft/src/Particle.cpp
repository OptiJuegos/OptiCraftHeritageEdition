#include "Particle.h"

#include "GuiParticle.h"

std::mt19937 Particle::rng;

Particle::Particle() :
    posX(0.0f), posY(0.0f),
    prevPosX(0.0f), prevPosY(0.0f),
    velX(0.0f), velY(0.0f),
    friction(1.0f),
    isDead(false),
    age(0),
    lifetime(0),
    red(0.0f), green(0.0f), blue(0.0f), alpha(0.0f),
    prevRed(0.0f), prevGreen(0.0f), prevBlue(0.0f), prevAlpha(0.0f)
{
}

void Particle::updateParticle()
{
    prevRed   = red;
    prevGreen = green;
    prevBlue  = blue;
    prevAlpha = alpha;
    prevPosX  = posX;
    prevPosY  = posY;

    posX += velX;
    posY += velY;
    velX *= friction;
    velY *= friction;
    velY += 0.1f;

    if (++age > lifetime)
        kill();

    alpha = 2.0f - (static_cast<float_t>(age) / static_cast<float_t>(lifetime)) * 2.0f;
    if (alpha > 1.0)
        alpha = 1.0f;
    alpha = alpha * alpha;
    alpha *= 0.5f;
}

void Particle::renderIntoGui(GuiParticle *)
{
}

void Particle::kill()
{
    isDead = true;
}
