#pragma once

#include "StepSound.h"

// net.minecraft.src.StepSoundSand
class StepSoundSand final : public StepSound
{
public:
	StepSoundSand(const jstring &s, float f, float f1);

	jstring getBreakSound() override;
};
