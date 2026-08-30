#include "StepSoundSand.h"

StepSoundSand::StepSoundSand(const jstring &s, float f, float f1) :
	StepSound(s, f, f1)
{
}

jstring StepSoundSand::getBreakSound()
{
	return "step.gravel";
}
