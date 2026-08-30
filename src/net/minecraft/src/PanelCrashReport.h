#pragma once

#include <string>

class UnexpectedThrowable;

// net.minecraft.src.PanelCrashReport
// Java: AWT Panel that displays crash info in a TextArea.
// C++: Collects the crash report text; rendering is done via the crash handler.
class PanelCrashReport
{
public:
    explicit PanelCrashReport(UnexpectedThrowable *unexpectedthrowable);

    const std::string &getText() const;

private:
    std::string reportText;
};
