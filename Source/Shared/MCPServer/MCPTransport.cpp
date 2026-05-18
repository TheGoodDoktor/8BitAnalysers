#include "MCPTransport.h"
#include <chrono>

#define MCP_HTTP_ENDPOINT_PATH "/mcp"

// ─────────────────────────────────────────────────────────────────────────────
// FStdioTransport  (unchanged)
// ─────────────────────────────────────────────────────────────────────────────

bool FStdioTransport::SendData(const std::string& line)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (bIsClosed)
	{
		LOGWARNING("FStdioTransport::SendData called on closed transport");
		return false;
	}
	std::cout << line << std::endl;
	std::cout.flush();
	if (std::cout.fail())
	{
		LOGWARNING("FStdioTransport::SendData failed to write to stdout");
		return false;
	}
	return true;
}

bool FStdioTransport::ReceiveData(std::string& line)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (bIsClosed)
	{
		LOGWARNING("FStdioTransport::ReceiveData called on closed transport");
		return false;
	}
	if (!std::getline(std::cin, line))
	{
		LOGWARNING("FStdioTransport::ReceiveData failed to read from stdin");
		return false;
	}
	return true;
}

void FStdioTransport::Close()
{
	std::lock_guard<std::mutex> lock(Mutex);
	bIsClosed = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// FHttpTransport
// ─────────────────────────────────────────────────────────────────────────────

FHttpTransport::FHttpTransport(uint16_t InPort)
	: ServerSocket(INVALID_SOCKET_VALUE)
	, CurrentClientSocket(INVALID_SOCKET_VALUE)
{
	bIsClosed = false;

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		LOGERROR("WSAStartup failed");
		return;
	}
#endif

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET_VALUE)
	{
		LOGERROR("FHttpTransport: Failed to create socket");
		return;
	}

	int opt = 1;
#ifdef _WIN32
	setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
	setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(InPort);

	if (bind(ServerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		LOGERROR("FHttpTransport: Failed to bind socket");
		SOCKET_CLOSE(ServerSocket);
		ServerSocket = INVALID_SOCKET_VALUE;
		return;
	}

	if (listen(ServerSocket, 10) < 0)
	{
		LOGERROR("FHttpTransport: Failed to listen on socket");
		SOCKET_CLOSE(ServerSocket);
		ServerSocket = INVALID_SOCKET_VALUE;
		return;
	}

	LOGINFO("FHttpTransport: Listening on port %d", InPort);

	// Start background threads
	AcceptThread = std::thread(&FHttpTransport::AcceptLoop, this);
	SseThread    = std::thread(&FHttpTransport::SseKeepaliveLoop, this);
}

FHttpTransport::~FHttpTransport()
{
	Close();
#ifdef _WIN32
	WSACleanup();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// ReadHttpRequest  — reads headers + body from a connected client socket
// ─────────────────────────────────────────────────────────────────────────────

bool FHttpTransport::ReadHttpRequest(socket_t client, FHttpRequest& out)
{
	const int kBufSize = 4096;
	char buffer[kBufSize];
	std::string raw;
	int headerEnd    = -1;
	int contentLength = -1;

	while (true)
	{
		int received = recv(client, buffer, kBufSize - 1, 0);
		if (received <= 0)
		{
			out.bConnectionClosed = (received == 0);
			out.bReadError        = (received < 0);
			return false;
		}
		raw.append(buffer, received);

		// Locate end of headers on first pass
		if (headerEnd == -1)
		{
			size_t pos = raw.find("\r\n\r\n");
			if (pos != std::string::npos)
			{
				headerEnd = (int)(pos + 4);

				LOGINFO("FHttpTransport: HTTP headers:\n%s", raw.substr(0, headerEnd).c_str());

				// Parse method + path from request line
				size_t methodEnd = raw.find(' ');
				size_t pathEnd   = (methodEnd != std::string::npos) ? raw.find(' ', methodEnd + 1) : std::string::npos;
				if (methodEnd == std::string::npos || pathEnd == std::string::npos)
				{
					out.bReadError = true;
					return false;
				}
				out.Method = raw.substr(0, methodEnd);
				out.Path   = raw.substr(methodEnd + 1, pathEnd - methodEnd - 1);

				// Parse Content-Length
				size_t clPos = raw.find("Content-Length:");
				if (clPos == std::string::npos)
					clPos = raw.find("content-length:");
				if (clPos != std::string::npos)
				{
					const char* p = raw.c_str() + clPos;
					while (*p && *p != ':') p++;
					if (*p == ':') p++;
					while (*p == ' ' || *p == '\t') p++;
					contentLength = atoi(p);
				}
				else
				{
					contentLength = 0;
				}
			}
		}

		// Stop once we have the full body
		if (headerEnd >= 0 && contentLength >= 0)
		{
			if ((int)(raw.length() - headerEnd) >= contentLength)
				break;
		}
	}

	out.Body = raw.substr(headerEnd, contentLength);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// AcceptLoop  — runs on AcceptThread; routes connections to SSE list or POST queue
// ─────────────────────────────────────────────────────────────────────────────

void FHttpTransport::AcceptLoop()
{
	while (!bIsClosed && ServerSocket != INVALID_SOCKET_VALUE)
	{
		struct sockaddr_in clientAddr;
		socklen_t clientLen = sizeof(clientAddr);
		socket_t client = accept(ServerSocket, (struct sockaddr*)&clientAddr, &clientLen);

		if (client == INVALID_SOCKET_VALUE)
		{
			if (bIsClosed) break;
			LOGWARNING("FHttpTransport::AcceptLoop: accept() failed, retrying");
			continue;
		}

		LOGINFO("FHttpTransport: Accepted connection from %s:%d",
			inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		FHttpRequest req;
		if (!ReadHttpRequest(client, req))
		{
			if (!req.bConnectionClosed)
				LOGWARNING("FHttpTransport::AcceptLoop: Failed to read request");
			SOCKET_CLOSE(client);
			continue;
		}

		LOGINFO("FHttpTransport: %s %s", req.Method.c_str(), req.Path.c_str());

		// Reject unknown paths
		if (req.Path != MCP_HTTP_ENDPOINT_PATH)
		{
			LOGWARNING("FHttpTransport: Rejecting request to invalid path: %s", req.Path.c_str());
			const char* notFound =
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Length: 0\r\n"
				"Access-Control-Allow-Origin: *\r\n"
				"Connection: close\r\n\r\n";
			::send(client, notFound, (int)strlen(notFound), 0);
			SOCKET_CLOSE(client);
			continue;
		}

		if (req.Method == "OPTIONS")
		{
			HandleOptionsRequest(client);
			continue;
		}

		if (req.Method == "GET")
		{
			// Open SSE stream — socket stays open
			HandleGetRequest(client);
			continue;
		}

		if (req.Method == "POST")
		{
			if (req.Body.empty())
			{
				LOGINFO("FHttpTransport: POST with no body, ignoring");
				const char* ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
				::send(client, ok, (int)strlen(ok), 0);
				SOCKET_CLOSE(client);
				continue;
			}

			LOGINFO("FHttpTransport: Queuing POST request (%d bytes): %.100s",
				(int)req.Body.length(), req.Body.c_str());

			{
				std::lock_guard<std::mutex> lock(PostQueueMutex);
				PostQueue.push({client, std::move(req.Body)});
			}
			PostQueueCV.notify_one();
			continue;
		}

		// Unknown method
		const char* methodNotAllowed =
			"HTTP/1.1 405 Method Not Allowed\r\n"
			"Content-Length: 0\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Connection: close\r\n\r\n";
		::send(client, methodNotAllowed, (int)strlen(methodNotAllowed), 0);
		SOCKET_CLOSE(client);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// HandleGetRequest  — sends SSE headers and registers the client
// ─────────────────────────────────────────────────────────────────────────────

bool FHttpTransport::HandleGetRequest(socket_t client)
{
	const std::string sseHeaders =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/event-stream\r\n"
		"Cache-Control: no-cache\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Connection: keep-alive\r\n"
		"\r\n";

	if (::send(client, sseHeaders.c_str(), (int)sseHeaders.length(), 0) < 0)
	{
		LOGWARNING("FHttpTransport: Failed to send SSE headers");
		SOCKET_CLOSE(client);
		return false;
	}

	LOGINFO("FHttpTransport: SSE client connected (socket=%d)", (int)client);
	{
		std::lock_guard<std::mutex> lock(SseMutex);
		SseClients.push_back(client);
	}
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// HandleOptionsRequest  — CORS preflight
// ─────────────────────────────────────────────────────────────────────────────

void FHttpTransport::HandleOptionsRequest(socket_t client)
{
	const std::string response =
		"HTTP/1.1 204 No Content\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
		"Access-Control-Allow-Headers: Content-Type, Accept, MCP-Session-Id, MCP-Protocol-Version\r\n"
		"Connection: close\r\n"
		"\r\n";

	::send(client, response.c_str(), (int)response.length(), 0);
	SOCKET_CLOSE(client);
}

// ─────────────────────────────────────────────────────────────────────────────
// SseKeepaliveLoop  — runs on SseThread; keeps SSE connections alive
// ─────────────────────────────────────────────────────────────────────────────

void FHttpTransport::SseKeepaliveLoop()
{
	const std::string kKeepalive = ": keepalive\r\n\r\n";

	while (!bIsClosed)
	{
		// Sleep 15 seconds in short slices so Close() wakes us quickly
		for (int i = 0; i < 150 && !bIsClosed; ++i)
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (bIsClosed) break;

		std::lock_guard<std::mutex> lock(SseMutex);
		for (auto it = SseClients.begin(); it != SseClients.end(); )
		{
			if (::send(*it, kKeepalive.c_str(), (int)kKeepalive.length(), 0) < 0)
			{
				LOGINFO("FHttpTransport: SSE client disconnected (socket=%d)", (int)*it);
				SOCKET_CLOSE(*it);
				it = SseClients.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// SendSseEvent  — push an event to all connected SSE clients
// ─────────────────────────────────────────────────────────────────────────────

void FHttpTransport::SendSseEvent(const std::string& eventType, const std::string& data)
{
	const std::string event = "event: " + eventType + "\r\ndata: " + data + "\r\n\r\n";

	std::lock_guard<std::mutex> lock(SseMutex);
	for (auto it = SseClients.begin(); it != SseClients.end(); )
	{
		if (::send(*it, event.c_str(), (int)event.length(), 0) < 0)
		{
			LOGINFO("FHttpTransport: SSE client disconnected while sending event (socket=%d)", (int)*it);
			SOCKET_CLOSE(*it);
			it = SseClients.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// ReceiveData  — blocks until a POST request is available
// ─────────────────────────────────────────────────────────────────────────────

bool FHttpTransport::ReceiveData(std::string& jsonLine)
{
	// Wait for the previous POST response to be sent before taking the next request
	{
		std::unique_lock<std::mutex> lock(Mutex);
		ClientCV.wait(lock, [this]() {
			return bIsClosed.load() || CurrentClientSocket == INVALID_SOCKET_VALUE;
		});
		if (bIsClosed) return false;
	}

	// Wait for the next POST request from AcceptLoop
	FPendingPost post;
	{
		std::unique_lock<std::mutex> lock(PostQueueMutex);
		PostQueueCV.wait(lock, [this]() {
			return bIsClosed.load() || !PostQueue.empty();
		});
		if (bIsClosed) return false;

		post = std::move(PostQueue.front());
		PostQueue.pop();
	}

	// Register the socket so SendData knows where to reply
	{
		std::lock_guard<std::mutex> lock(Mutex);
		CurrentClientSocket = post.Socket;
	}

	jsonLine = std::move(post.Body);
	LOGINFO("FHttpTransport: Dequeued POST request (%d bytes)", (int)jsonLine.length());
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// SendData  — sends the HTTP response to the current POST client
// ─────────────────────────────────────────────────────────────────────────────

bool FHttpTransport::SendData(const std::string& jsonLine)
{
	std::lock_guard<std::mutex> lock(Mutex);

	if (bIsClosed || CurrentClientSocket == INVALID_SOCKET_VALUE)
	{
		LOGWARNING("FHttpTransport::SendData failed: %s",
			bIsClosed ? "transport closed" : "no active client");
		return false;
	}

	std::string http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: " + std::to_string(jsonLine.length()) + "\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Connection: close\r\n"
		"\r\n" +
		jsonLine;

	int sent = ::send(CurrentClientSocket, http_response.c_str(), (int)http_response.length(), 0);

	if (sent < 0)
		LOGWARNING("FHttpTransport::SendData: send() failed");
	else
	{
		std::string preview = jsonLine.length() > 200 ? jsonLine.substr(0, 200) + "..." : jsonLine;
		LOGINFO("FHttpTransport: Sent HTTP response (%d bytes): %s", sent, preview.c_str());
	}

	SOCKET_CLOSE(CurrentClientSocket);
	CurrentClientSocket = INVALID_SOCKET_VALUE;
	ClientCV.notify_all();

	return sent >= 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Close
// ─────────────────────────────────────────────────────────────────────────────

void FHttpTransport::Close()
{
	bIsClosed = true;

	// Wake all waiting threads
	ClientCV.notify_all();
	PostQueueCV.notify_all();

	// Close all SSE clients
	{
		std::lock_guard<std::mutex> lock(SseMutex);
		for (socket_t s : SseClients)
			SOCKET_CLOSE(s);
		SseClients.clear();
	}

	// Close the current POST client if any
	{
		std::lock_guard<std::mutex> lock(Mutex);
		if (CurrentClientSocket != INVALID_SOCKET_VALUE)
		{
			SOCKET_CLOSE(CurrentClientSocket);
			CurrentClientSocket = INVALID_SOCKET_VALUE;
		}
	}

	// Close server socket — causes AcceptLoop's accept() to return with error
	if (ServerSocket != INVALID_SOCKET_VALUE)
	{
		SOCKET_CLOSE(ServerSocket);
		ServerSocket = INVALID_SOCKET_VALUE;
	}

	// Detach threads (they will exit on the next bIsClosed check)
	if (AcceptThread.joinable())
		AcceptThread.detach();
	if (SseThread.joinable())
		SseThread.detach();
}

// ─────────────────────────────────────────────────────────────────────────────
// ValidateAndRejectInvalidPath  — kept for any external callers
// ─────────────────────────────────────────────────────────────────────────────

bool FHttpTransport::ValidateAndRejectInvalidPath(const std::string& request, socket_t client)
{
	// Extract path from request line
	size_t methodEnd = request.find(' ');
	size_t pathEnd   = (methodEnd != std::string::npos) ? request.find(' ', methodEnd + 1) : std::string::npos;
	std::string path;
	if (methodEnd != std::string::npos && pathEnd != std::string::npos)
		path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);

	if (path == MCP_HTTP_ENDPOINT_PATH)
		return true;

	LOGWARNING("FHttpTransport: Rejecting request to invalid path: %s", path.c_str());
	const char* notFound =
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Connection: close\r\n"
		"\r\n"
		"404 Not Found";
	::send(client, notFound, (int)strlen(notFound), 0);
	SOCKET_CLOSE(client);
	{
		std::lock_guard<std::mutex> lock(Mutex);
		CurrentClientSocket = INVALID_SOCKET_VALUE;
		ClientCV.notify_all();
	}
	return false;
}
