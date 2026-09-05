#include "../shared/vnet.h"
#include "../shared/vnet_protocol.h"
#include "../lib/vnet_lib.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <atomic>

// Forward declaration
void RunVNTServer(int port);

static std::atomic<bool> g_running{true};

void SignalHandler(int signal) {
    (void)signal;
    std::cout << "\n[Server] Shutdown signal received. Stopping..." << std::endl;
    g_running = false;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  VNET SERVER v9.5 - CYBERWARFARE ENGINE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int port = 8000;
    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "[ERROR] Invalid port. Using default 8000." << std::endl;
            port = 8000;
        }
    }
    
    std::cout << "[Server] Starting on port " << port << std::endl;
    std::cout << "[Server] Press Ctrl+C to stop." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Setup signal handlers for graceful shutdown
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
    
    // Initialize networking
    if (!VNetLib::InitNetworking()) {
        std::cerr << "[ERROR] Failed to initialize networking!" << std::endl;
        return 1;
    }
    
    // Run the server
    RunVNTServer(port);
    
    // Cleanup
    VNetLib::ShutdownNetworking();
    
    std::cout << "[Server] Stopped." << std::endl;
    return 0;
}