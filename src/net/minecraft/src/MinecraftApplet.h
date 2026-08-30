#pragma once

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

class CanvasMinecraftApplet;
class MinecraftAppletImpl;

// net.minecraft.client.MinecraftApplet
// AWT is not part of the native runtime. This adapter preserves the Java
// applet lifecycle and parameter semantics on top of MinecraftAppletImpl.
class MinecraftApplet
{
public:
    MinecraftApplet(int width = 854, int height = 480);
    ~MinecraftApplet();

    void setParameter(const std::string &name, const std::string &value);
    void setDocumentHost(const std::string &host);

    void init();
    void startMainThread();
    void start();
    void stop();
    void destroy();
    void shutdown();
    void clearApplet();

    MinecraftAppletImpl *getMinecraft() const;

private:
    std::string getParameter(const std::string &name) const;

    int width;
    int height;
    std::string documentHost;
    std::unordered_map<std::string, std::string> parameters;
    std::unique_ptr<CanvasMinecraftApplet> mcCanvas;
    std::unique_ptr<MinecraftAppletImpl> mc;
    std::thread mcThread;
};
