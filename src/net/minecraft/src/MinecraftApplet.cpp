#include "MinecraftApplet.h"

#include "CanvasMinecraftApplet.h"
#include "MinecraftAppletImpl.h"
#include "Session.h"
#include "java/String.h"

#include <cstdlib>
#include <stdexcept>

MinecraftApplet::MinecraftApplet(int widthValue, int heightValue)
    : width(widthValue),
      height(heightValue)
{
}

MinecraftApplet::~MinecraftApplet()
{
    shutdown();
}

void MinecraftApplet::setParameter(const std::string &name, const std::string &value)
{
    parameters[name] = value;
}

void MinecraftApplet::setDocumentHost(const std::string &host)
{
    documentHost = host;
}

std::string MinecraftApplet::getParameter(const std::string &name) const
{
    const auto it = parameters.find(name);
    return it != parameters.end() ? it->second : std::string();
}

void MinecraftApplet::init()
{
    if (mc != nullptr)
        return;

    const std::string fullscreenValue = getParameter("fullscreen");
    const bool fullscreen = fullscreenValue == "true" || fullscreenValue == "TRUE" || fullscreenValue == "True";

    mc = std::make_unique<MinecraftAppletImpl>(this, this, nullptr, this, width, height, fullscreen);
    mcCanvas = std::make_unique<CanvasMinecraftApplet>(mc.get());
    mc->minecraftUri = documentHost;

    const std::string username = getParameter("username");
    const std::string sessionId = getParameter("sessionid");
    delete mc->session;
    if (!username.empty() && !sessionId.empty())
    {
        mc->session = new Session(username, sessionId);
        mc->session->mpPassParameter = getParameter("mppass");
    }
    else
    {
        mc->session = new Session("Player", "");
    }

    const std::string server = getParameter("server");
    const std::string port = getParameter("port");
    if (!server.empty() && !port.empty())
    {
        int_t serverPort = 0;
        if (!String::tryParseInt(port, serverPort))
            throw std::invalid_argument("Invalid applet server port: " + port);
        mc->setServer(server, serverPort);
    }

    mc->hideQuitButton = getParameter("stand-alone") != "true";
}

void MinecraftApplet::startMainThread()
{
    if (mc == nullptr)
        init();
    if (mc == nullptr || mcThread.joinable())
        return;

    mcThread = std::thread([this]()
    {
        if (mc != nullptr)
            mc->run();
    });
}

void MinecraftApplet::start()
{
    if (mc != nullptr)
        mc->isGamePaused = false;
}

void MinecraftApplet::stop()
{
    if (mc != nullptr)
        mc->isGamePaused = true;
}

void MinecraftApplet::destroy()
{
    shutdown();
}

void MinecraftApplet::shutdown()
{
    if (mc != nullptr)
        mc->shutdown();

    if (mcThread.joinable())
        mcThread.join();
}

void MinecraftApplet::clearApplet()
{
    shutdown();
    mcCanvas.reset();
    mc.reset();
}

MinecraftAppletImpl *MinecraftApplet::getMinecraft() const
{
    return mc.get();
}
