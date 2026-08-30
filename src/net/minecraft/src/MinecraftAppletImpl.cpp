#include "platform/Log.h"
#include "MinecraftAppletImpl.h"

#include <iostream>

#include "PanelCrashReport.h"
#include "UnexpectedThrowable.h"

MinecraftAppletImpl::MinecraftAppletImpl(void *minecraftAppletFrame,
                                         void *component, void *canvas,
                                         void *minecraftapplet,
                                         int_t width, int_t height, bool fullscreen)
    : Minecraft(width, height, fullscreen),
      mainFrame(minecraftAppletFrame)
{
}

void MinecraftAppletImpl::displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable)
{
    PanelCrashReport report(unexpectedthrowable);
    MC_LOG_ERROR("crash", "%s\n", report.getText().c_str());
    // In the AWT build, mainFrame.removeAll() + setLayout(BorderLayout) +
    // mainFrame.add(PanelCrashReport, "Center") + mainFrame.validate() would
    // inject the crash UI into the applet.  In SDL2 we write to stderr.
}
