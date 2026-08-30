#pragma once

#include <random>
#include "java/Type.h"

class GuiParticle;

// net.minecraft.src.Particle  (GUI particle for title screen / menu animations)
class Particle
{
public:
    Particle();

    void updateParticle();                   // func_25125_a – tick; saves prev state then advances
    void renderIntoGui(GuiParticle *gui);    // per-particle render hook (no-op; rendering done in GuiParticle::renderParticles)
    void kill();                             // func_25126_b

    float_t posX;      // field_25146_a
    float_t posY;      // field_25145_b
    float_t prevPosX;  // field_25144_c
    float_t prevPosY;  // field_25143_d
    float_t velX;      // field_25142_e
    float_t velY;      // field_25141_f
    float_t friction;  // field_25140_g
    bool   isDead;    // field_25139_h
    int_t  age;       // field_25138_i
    int_t  lifetime;  // field_25137_j
    float_t red;       // field_25136_k
    float_t green;     // field_25135_l
    float_t blue;      // field_25134_m
    float_t alpha;     // field_25133_n
    float_t prevRed;   // field_25132_o
    float_t prevGreen; // field_25131_p
    float_t prevBlue;  // field_25130_q
    float_t prevAlpha; // field_25129_r

private:
    static std::mt19937 rng;
};
