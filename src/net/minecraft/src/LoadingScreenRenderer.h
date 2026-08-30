#pragma once

#include <string>

#include "IProgressUpdate.h"
#include "java/Type.h"

class Minecraft;

// net.minecraft.src.LoadingScreenRenderer
class LoadingScreenRenderer : public IProgressUpdate
{
public:
	LoadingScreenRenderer(Minecraft *minecraft);

	void printText(const jstring &s);
	void displaySavingString(const jstring &s) override;
	void func_597_c(const jstring &s);
	void displayLoadingString(const jstring &s) override;
	void resetProgressAndMessage(const jstring &s) override;
	void setLoadingProgress(int_t i) override;

private:
	void setProgressMessage(const std::string &s);
	void resetProgressAndMessageUTF8(const std::string &s);
	void setLoadingMessage(const std::string &s);
	void prepareProgressDisplay(const std::string &s);

	std::string field_1004_a;
	Minecraft *mc;
	std::string field_1007_c;
	long_t field_1006_d;
	bool field_1005_e;
};
