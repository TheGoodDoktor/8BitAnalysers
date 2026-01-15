#pragma once

#include <string>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <cerrno>
#include "Debug/DebugLog.h"

//#define MCP_HTTP_ENDPOINT_PATH "/mcp"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	typedef SOCKET socket_t;
	#define INVALID_SOCKET_VALUE INVALID_SOCKET
	#define SOCKET_CLOSE(s) closesocket(s)
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	typedef int socket_t;
	#define INVALID_SOCKET_VALUE -1
	#define SOCKET_CLOSE(s) ::close(s)
#endif

class FMCPTransport
{
public:
	virtual ~FMCPTransport() {}

	virtual bool SendData(const std::string& jsonLine) = 0;
	virtual bool ReceiveData(std::string& jsonLine) = 0;
	virtual void Close() = 0;
protected:
	std::mutex Mutex;
	bool bIsClosed = false;
};

// transport that just uses stdin/stdout for communication
class FStdioTransport : public FMCPTransport
{
public:
	FStdioTransport(){}
	~FStdioTransport() override {}

	bool SendData(const std::string& jsonLine) override;
	bool ReceiveData(std::string& jsonLine) override;
	void Close() override;

};

class FHttpTransport : public FMCPTransport
{
public:
	FHttpTransport(uint16_t InPort);
	~FHttpTransport() override;
	bool SendData(const std::string& jsonLine) override;
	bool ReceiveData(std::string& jsonLine) override;
	void Close() override;

	bool ValidateAndRejectInvalidPath(const std::string& request, socket_t client);
private:
	std::condition_variable ClientCV;
	socket_t				ServerSocket;
	socket_t				CurrentClientSocket;
};