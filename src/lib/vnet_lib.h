#pragma once
#include <string>
#include <vector>

// ============================================================
// LOW-LEVEL VNET NETWORKING LIBRARY (WRITTEN FROM SCRATCH)
// ============================================================

// Socket handle type (platform independent)
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketHandle;
    #define INVALID_SOCKET_HANDLE INVALID_SOCKET
    #define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int SocketHandle;
    #define INVALID_SOCKET_HANDLE (-1)
    #define IS_INVALID_SOCKET(s) ((s) < 0)
    #define closesocket close
#endif

namespace VNetLib {

// Initialize networking (Winsock on Windows)
bool InitNetworking();

// Cleanup networking
void ShutdownNetworking();

// Create a UDP socket, bind to a port (0 = any available port)
SocketHandle CreateUDPSocket(int port = 0);

// Send a UDP packet
bool SendTo(SocketHandle sock, const std::string& ip, int port, const std::string& data);

// Receive a UDP packet (non-blocking)
// Returns: {data, sender_ip, sender_port} or empty vector if nothing received
std::vector<std::string> RecvFrom(SocketHandle sock, int bufferSize = 2048);

// Close a socket
void CloseSocket(SocketHandle sock);

// Get the local port of a socket
int GetLocalPort(SocketHandle sock);

// Set socket to non-blocking mode
bool SetNonBlocking(SocketHandle sock);

void Sleep(int ms); 

} // namespace VNetLib