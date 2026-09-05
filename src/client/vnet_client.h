#pragma once
#include <string>
#include <vector>

bool InitVNetClient(const char* serverIP, int serverPort = 8000);
void ShutdownVNetClient();

bool VNetSend(const std::string& cmd, const std::string& payload = "");
bool VNetSendRaw(const std::string& data);
std::vector<std::string> VNetReceive();

int GetClientPort();
bool IsVNetConnected();