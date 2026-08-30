#include "StepSoundStone.h"

StepSoundStone::StepSoundStone(const jstring &s, float f, float f1) :
	StepSound(s, f, f1)
{
}

jstring StepSoundStone::getBreakSound()
{
	return "random.glass";
}
