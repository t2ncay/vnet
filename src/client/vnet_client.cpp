#include "../shared/vnet.h"
#include "../shared/vnet_protocol.h"
#include "../lib/vnet_lib.h"
#include <cstring>
#include <cstdio>

static SocketHandle g_clientSocket = INVALID_SOCKET_HANDLE;
static std::string g_serverIP = "127.0.0.1";
static int g_serverPort = 8000;
static int g_clientPort = 0;
static bool g_connected = false;

bool InitVNetClient(const char* serverIP, int serverPort) {
    g_serverIP = serverIP;
    g_serverPort = serverPort;

    g_clientSocket = VNetLib::CreateUDPSocket(0);
    if (IS_INVALID_SOCKET(g_clientSocket)) {
        return false;
    }

    g_clientPort = VNetLib::GetLocalPort(g_clientSocket);
    g_connected = true;

    // Update player port
    g_player.port = g_clientPort;

    printf("[VNET] Client bound to port %d\n", g_clientPort);
    return true;
}

void ShutdownVNetClient() {
    if (!IS_INVALID_SOCKET(g_clientSocket)) {
        VNetLib::CloseSocket(g_clientSocket);
        g_clientSocket = INVALID_SOCKET_HANDLE;
    }
    g_connected = false;
}

bool VNetSend(const std::string& cmd, const std::string& payload) {
    if (!g_connected || IS_INVALID_SOCKET(g_clientSocket)) return false;

    std::string packet = BuildPacket(cmd, payload);
    bool result = VNetLib::SendTo(g_clientSocket, g_serverIP, g_serverPort, packet);
    
    if (!result) {
        printf("[VNET] Failed to send packet: %s\n", packet.c_str());
    }
    return result;
}

std::vector<std::string> VNetReceive() {
    if (!g_connected || IS_INVALID_SOCKET(g_clientSocket)) {
        return std::vector<std::string>();
    }
    return VNetLib::RecvFrom(g_clientSocket);
}

bool VNetSendRaw(const std::string& data) {
    if (!g_connected || IS_INVALID_SOCKET(g_clientSocket)) return false;
    return VNetLib::SendTo(g_clientSocket, g_serverIP, g_serverPort, data);
}

int GetClientPort() { return g_clientPort; }
bool IsVNetConnected() { return g_connected; }