#include "StepSound.h"

StepSound::StepSound(const jstring &s, float f, float f1) :
	motionZ(s),
	motionY(f),
	rotationYaw(f1)
{
}

float StepSound::getVolume()
{
	return motionY;
}

float StepSound::getPitch()
{
	return rotationYaw;
}

jstring StepSound::getStepSound()
{
	return "step." + motionZ;
}

jstring StepSound::getBreakSound()
{
	return "step." + motionZ;
}

jstring StepSound::stepSoundDir()
{
	return getStepSound();
}
