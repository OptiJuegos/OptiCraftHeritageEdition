#pragma once

#include "StepSound.h"

// net.minecraft.src.StepSoundStone
class StepSoundStone final : public StepSound
{
public:
	StepSoundStone(const jstring &s, float f, float f1);

	jstring getBreakSound() override;
};
