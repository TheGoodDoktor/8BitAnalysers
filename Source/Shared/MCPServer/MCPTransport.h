#pragma once

#include <string>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <cerrno>
#include "Debug/DebugLog.h"

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
	std::mutex			Mutex;
	std::atomic<bool>	bIsClosed{false};
};

// Transport that uses stdin/stdout for communication
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

	// Push an SSE event to all connected SSE clients
	void SendSseEvent(const std::string& eventType, const std::string& data);

	bool ValidateAndRejectInvalidPath(const std::string& request, socket_t client);

private:
	// Parsed HTTP request
	struct FHttpRequest
	{
		std::string Method;
		std::string Path;
		std::string Body;
		bool bReadError = false;
		bool bConnectionClosed = false;
	};

	// A queued POST request waiting to be processed
	struct FPendingPost
	{
		socket_t	Socket;
		std::string Body;
	};

	bool ReadHttpRequest(socket_t client, FHttpRequest& outRequest);
	void AcceptLoop();
	void SseKeepaliveLoop();
	bool HandleGetRequest(socket_t client);
	void HandleOptionsRequest(socket_t client);

	// Server listen socket
	socket_t ServerSocket;

	// Current POST client being served (protected by Mutex / ClientCV)
	socket_t CurrentClientSocket;
	std::condition_variable ClientCV;

	// Queue of incoming POST requests (AcceptLoop produces, ReceiveData consumes)
	std::queue<FPendingPost>	PostQueue;
	std::mutex					PostQueueMutex;
	std::condition_variable		PostQueueCV;

	// Active SSE client sockets (protected by SseMutex)
	std::vector<socket_t>	SseClients;
	std::mutex				SseMutex;

	// Background threads
	std::thread AcceptThread;
	std::thread SseThread;
};
