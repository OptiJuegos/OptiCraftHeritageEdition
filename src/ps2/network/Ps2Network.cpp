#ifdef PS2_PLATFORM

#include "ps2/network/Ps2Network.h"

#include "platform/Log.h"
#include "platform/Mutex.h"
#include "ps2/system/Ps2IrxLoader.h"

#include <delaythread.h>
#include <kernel.h>

extern "C" {
#include <netman.h>
#include <ps2ip.h>
}

#include <atomic>
#include <mutex>
#include <string>

namespace
{
PlatformMutex s_initMutex;
std::atomic_bool s_ready{false};
bool s_stackInitialized = false;
unsigned long s_provisionalAddress = 0;
std::string s_localAddress;

void loadNetworkModules()
{
    // Modern EE ps2ip keeps lwIP on the EE. Only the Ethernet driver and
    // NETMAN bridge are required on the IOP; ps2ip-nm.irx is intentionally not
    // loaded because that is the alternative IOP-side TCP/IP stack.
    const int dev9 = Ps2IrxLoader::load("irx/ps2dev9.irx", "host:ps2dev9.irx");
    const int netman = Ps2IrxLoader::load("irx/netman.irx", "host:netman.irx");
    const int smap = Ps2IrxLoader::load("irx/smap.irx", "host:smap.irx");

    // A loader such as ps2link/OPL may already have one or more of these IRXs
    // resident, in which case a duplicate load can return an error. Do not fail
    // solely on the loader return values; the sm0 interface check below is the
    // definitive test that the Ethernet path came up.
    MC_LOG_INFO("network", "[PS2] IRX network load: dev9=%d netman=%d smap=%d\n",
                dev9, netman, smap);
}

std::string addressToString(unsigned long raw)
{
    // PS2 is little-endian while IPv4 is stored in network byte order; reading
    // the low byte first produces the familiar dotted-quad representation.
    return std::to_string((raw      ) & 0xff) + "." +
           std::to_string((raw >>  8) & 0xff) + "." +
           std::to_string((raw >> 16) & 0xff) + "." +
           std::to_string((raw >> 24) & 0xff);
}

bool startStackAndDhcp()
{
    if (s_stackInitialized)
        return true;

    loadNetworkModules();

    // ps2ipInit initializes NETMAN itself, then registers the EE-side lwIP
    // stack as sm0. Calling NetManInit a second time here is unnecessary and
    // can make ownership/deinit semantics ambiguous.
    ip4_addr ip{};
    ip4_addr mask{};
    ip4_addr gateway{};
    IP4_ADDR(&ip, 169, 254, 0, 1);
    IP4_ADDR(&mask, 255, 255, 0, 0);
    IP4_ADDR(&gateway, 0, 0, 0, 0);
    s_provisionalAddress = ip.addr;

    MC_LOG_INFO("network", "[PS2] calling ps2ipInit\n");
    McLog::flush();
    if (ps2ipInit(&ip, &mask, &gateway) < 0)
    {
        MC_LOG_ERROR("network", "[PS2] ps2ipInit failed\n");
        return false;
    }

    MC_LOG_INFO("network", "[PS2] ps2ipInit OK\n");
    McLog::flush();
    const int linkModeResult = NetManSetLinkMode(NETMAN_NETIF_ETH_LINK_MODE_AUTO);
    MC_LOG_INFO("network", "[PS2] NetManSetLinkMode(AUTO) -> %d\n", linkModeResult);
    McLog::flush();

    t_ip_info info{};
    if (!ps2ip_getconfig(const_cast<char *>("sm0"), &info))
    {
        MC_LOG_ERROR("network", "[PS2] network interface sm0 was not registered\n");
        ps2ipDeinit();
        return false;
    }

    MC_LOG_INFO("network", "[PS2] sm0 registered, enabling DHCP\n");
    McLog::flush();
    info.dhcp_enabled = 1;
    if (!ps2ip_setconfig(&info))
    {
        MC_LOG_ERROR("network", "[PS2] failed to enable DHCP on sm0\n");
        ps2ipDeinit();
        return false;
    }

    // Keep the stack resident even if the first DHCP wait times out. DHCP can
    // finish later (for example after a cable is connected), and the next
    // connection attempt can then succeed without reinitializing lwIP.
    s_stackInitialized = true;
    MC_LOG_INFO("network", "[PS2] DHCP client started\n");
    McLog::flush();
    return true;
}
}

namespace Ps2Network
{
bool initialize()
{
#ifdef NO_NETWORK
    return false;
#else
    if (s_ready.load(std::memory_order_acquire))
        return true;

    MC_LOG_INFO("network", "[PS2] network initialize entered\n");
    McLog::flush();
    std::lock_guard<PlatformMutex> guard(s_initMutex);
    MC_LOG_INFO("network", "[PS2] network init lock acquired\n");
    McLog::flush();
    if (s_ready.load(std::memory_order_relaxed))
        return true;
    if (!startStackAndDhcp())
        return false;

    // Link negotiation + DHCP normally completes quickly on a LAN. This runs
    // from the connection/ping worker, not the render thread, and is still
    // bounded so an unplugged cable cannot leave a worker stuck forever.
    // DHCP on the PS2 can take substantially longer than 10 seconds. In
    // particular, lwIP enters DHCP_STATE_CHECKING after receiving the ACK and
    // performs duplicate-address detection before committing the lease. The
    // PCSX2 sockets backend can also take a while to bring SMAP/link up.
    constexpr int kPollCount = 900;
    constexpr int kPollDelayUs = 100000;
    for (int i = 0; i < kPollCount; ++i)
    {
        const bool linkUp = NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS,
                                        nullptr, 0, nullptr, 0) ==
                            NETMAN_NETIF_ETH_LINK_STATE_UP;
        t_ip_info current{};
        const bool haveConfig = ps2ip_getconfig(const_cast<char *>("sm0"), &current) != 0;
        const bool dhcpBound = haveConfig && current.dhcp_enabled &&
                               current.dhcp_status == DHCP_STATE_BOUND;
        const bool haveLease = dhcpBound && current.ipaddr.s_addr != 0 &&
                               current.ipaddr.s_addr != s_provisionalAddress;
        if ((i % 10) == 0)
        {
            MC_LOG_INFO("network", "[PS2] net poll %d link=%d config=%d ip=%s dhcp=%d state=%d\n",
                        i, linkUp ? 1 : 0, haveConfig ? 1 : 0,
                        haveConfig ? addressToString(current.ipaddr.s_addr).c_str() : "n/a",
                        haveConfig ? current.dhcp_enabled : -1,
                        haveConfig ? current.dhcp_status : -1);
            McLog::flush();
        }
        if (linkUp && haveLease)
        {
            s_localAddress = addressToString(current.ipaddr.s_addr);
            s_ready.store(true, std::memory_order_release);
            MC_LOG_INFO("network", "[PS2] network ready, DHCP address %s\n",
                        s_localAddress.c_str());
            return true;
        }
        DelayThread(kPollDelayUs);
    }

    MC_LOG_WARN("network", "[PS2] DHCP/link timeout after 90s; retry is allowed on the next connection\n");
    return false;
#endif
}

bool isReady()
{
    return s_ready.load(std::memory_order_acquire);
}

std::string localAddress()
{
    std::lock_guard<PlatformMutex> guard(s_initMutex);
    return s_localAddress;
}
}

#endif
