#include "vnet_lib.h"
#include <cstring>
#include <cstdio>

#ifdef _WIN32
    static bool g_wsaInitialized = false;
#endif

namespace VNetLib {

bool InitNetworking() {
#ifdef _WIN32
    if (g_wsaInitialized) return true;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return false;
    }
    g_wsaInitialized = true;
#endif
    return true;
}

void ShutdownNetworking() {
#ifdef _WIN32
    if (g_wsaInitialized) {
        WSACleanup();
        g_wsaInitialized = false;
    }
#endif
}

SocketHandle CreateUDPSocket(int port) {
    if (!InitNetworking()) return INVALID_SOCKET_HANDLE;

    SocketHandle sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (IS_INVALID_SOCKET(sock)) {
        return INVALID_SOCKET_HANDLE;
    }

    // Set non-blocking
    SetNonBlocking(sock);

    // Bind to port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(sock);
        return INVALID_SOCKET_HANDLE;
    }

    return sock;
}

bool SendTo(SocketHandle sock, const std::string& ip, int port, const std::string& data) {
    if (IS_INVALID_SOCKET(sock)) return false;

    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &destAddr.sin_addr);

    int sent = sendto(sock, data.c_str(), data.length(), 0,
                      (sockaddr*)&destAddr, sizeof(destAddr));
    return sent > 0;
}

std::vector<std::string> RecvFrom(SocketHandle sock, int bufferSize) {
    std::vector<std::string> result;
    if (IS_INVALID_SOCKET(sock)) return result;

    char* buffer = new char[bufferSize];
    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);

    int bytes = recvfrom(sock, buffer, bufferSize - 1, 0,
                         (sockaddr*)&senderAddr, &addrLen);

    if (bytes > 0) {
        buffer[bytes] = '\0';
        result.push_back(std::string(buffer));

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &senderAddr.sin_addr, ip, INET_ADDRSTRLEN);
        result.push_back(std::string(ip));
        result.push_back(std::to_string(ntohs(senderAddr.sin_port)));
    }

    delete[] buffer;
    return result;
}

void CloseSocket(SocketHandle sock) {
    if (!IS_INVALID_SOCKET(sock)) {
        closesocket(sock);
    }
}

int GetLocalPort(SocketHandle sock) {
    if (IS_INVALID_SOCKET(sock)) return 0;

    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getsockname(sock, (sockaddr*)&addr, &len) == 0) {
        return ntohs(addr.sin_port);
    }
    return 0;
}

bool SetNonBlocking(SocketHandle sock) {
    if (IS_INVALID_SOCKET(sock)) return false;

#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

void Sleep(int ms) {
#ifdef _WIN32
    ::Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

} // namespace VNetLib