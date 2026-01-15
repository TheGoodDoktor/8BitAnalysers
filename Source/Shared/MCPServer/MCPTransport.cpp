#include "MCPTransport.h"

#define MCP_HTTP_ENDPOINT_PATH "/mcp"


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

// FHttpTransport
FHttpTransport::FHttpTransport(uint16_t InPort)
{
	bIsClosed = false;
	ServerSocket = INVALID_SOCKET_VALUE;
	CurrentClientSocket = INVALID_SOCKET_VALUE;
#ifdef _WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LOGERROR("WSAStartup failed with error: %d", iResult);
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

	if (listen(ServerSocket, 5) < 0)
	{
		LOGERROR("FHttpTransport: Failed to listen on socket");
		SOCKET_CLOSE(ServerSocket);
		ServerSocket = INVALID_SOCKET_VALUE;
		return;
	}

	LOGINFO("FHttpTransport: Listening on port %d", InPort);
}

FHttpTransport::~FHttpTransport()
{
	Close();
#ifdef _WIN32
	WSACleanup();
#endif
}

bool FHttpTransport::SendData(const std::string& jsonLine)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (bIsClosed || CurrentClientSocket == INVALID_SOCKET_VALUE)
	{
		LOGWARNING("FHttpTransport::SendData failed: %s (closed=%d, client=%d)",
			bIsClosed ? "Server Closed" : "No Client Connection",
			bIsClosed, (int)CurrentClientSocket);
		return false;
	}

	// Build HTTP response (close connection after each response for simplicity)
	std::string http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: " + std::to_string(jsonLine.length()) + "\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Connection: close\r\n"
		"\r\n" +
		jsonLine;

	int sent = send(CurrentClientSocket, http_response.c_str(), (int)http_response.length(), 0);

	if (sent < 0)
	{
		LOGWARNING("FHttpTransport::SendData failed to send data to client");
		CurrentClientSocket = INVALID_SOCKET_VALUE;
		ClientCV.notify_all();
		return false;
	}

	std::string logOutput = jsonLine.length() > 500
		? jsonLine.substr(0, 500) + "..."
		: jsonLine;
	LOGINFO("FHttpTransport : HTTP response sent (%d bytes): %s",
		sent, logOutput.c_str());

	SOCKET_CLOSE(CurrentClientSocket);
	CurrentClientSocket = INVALID_SOCKET_VALUE;
	ClientCV.notify_all();
	return true;
}

// Helper function to extract path from HTTP request line
std::string ExtractHttpPath(const std::string& request)
{
	// Request format: "METHOD /path HTTP/1.1\r\n..."
	size_t method_end = request.find(' ');
	if (method_end == std::string::npos)
		return "";

	size_t path_start = method_end + 1;
	size_t path_end = request.find(' ', path_start);
	if (path_end == std::string::npos)
		return "";

	return request.substr(path_start, path_end - path_start);
}

// Helper function to validate path and send 404 if invalid
// Returns true if path is valid, false if rejected (and response already sent)
bool FHttpTransport::ValidateAndRejectInvalidPath(const std::string& request, socket_t client)
{
	std::string path = ExtractHttpPath(request);
	if (path == MCP_HTTP_ENDPOINT_PATH)
		return true;

	LOGWARNING("[MCP] Rejecting request to invalid path: %s", path.c_str());

	const char* not_found_response =
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Connection: close\r\n"
		"\r\n"
		"404 Not Found";
	::send(client, not_found_response, (int)strlen(not_found_response), 0);
	SOCKET_CLOSE(client);
	{
		std::lock_guard<std::mutex> lock(Mutex);
		CurrentClientSocket = INVALID_SOCKET_VALUE;
		ClientCV.notify_all();
	}
	return false;
}

bool FHttpTransport::ReceiveData(std::string& jsonLine)
{
	while(!bIsClosed && ServerSocket != INVALID_SOCKET_VALUE)
	{
		// wait for previous client
		{
			std::unique_lock<std::mutex> lock(Mutex);
			ClientCV.wait(lock, [this]() 
			{
				return bIsClosed || CurrentClientSocket == INVALID_SOCKET_VALUE;
			});

			if (bIsClosed)
			{
				LOGINFO("FHttpTransport::ReceiveData exiting due to closed transport");
				return false;
			}
		}

		// accept new client
		struct sockaddr_in clientAddr;
		socklen_t clientLen = sizeof(clientAddr);
		socket_t clientSocket = accept(ServerSocket, (struct sockaddr*)&clientAddr, &clientLen);

		if (clientSocket == INVALID_SOCKET_VALUE)
		{
			if(bIsClosed)
			{
				LOGINFO("FHttpTransport::ReceiveData exiting due to closed transport");
				return false;
			}
			LOGWARNING("FHttpTransport::ReceiveData failed to accept client, retrying");
			continue;
		}

		LOGINFO("FHttpTransport: Accepted connection from %s:%d",
			inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		{
			std::lock_guard<std::mutex> lock(Mutex);
			CurrentClientSocket = clientSocket;
		}

		// Read HTTP request
		const int kBufferSize = 4096;
		char buffer[kBufferSize];
		int headerEnd = -1;
		int contentLength = -1;
		bool bReadError = false;
		bool bIsOptions = false;
		bool bConnectionClosed = false;
		std::string request;

		while (true)
		{
			int received = recv(clientSocket, buffer, kBufferSize - 1, 0);	
			if (received <= 0)
			{
				bReadError = true;
				bConnectionClosed = (received == 0);
				LOGWARNING("FHttpTransport::ReceiveData %s (received=%d)", bConnectionClosed ? "Connection closed by peer" : "Read error", received);
				break;
			}

			request.append(buffer, received);

			// Check for end of headers
			if (headerEnd == -1)
			{
				size_t pos = request.find("\r\n\r\n");
				if (pos != std::string::npos)
				{
					headerEnd = (int)(pos + 4);

					LOGINFO("FHttpTransport: HTTP headers: \n%s",
						request.substr(0, headerEnd).c_str());

					// check for OPTIONS request
					if (request.substr(0, 7) == "OPTIONS")
					{
						if (!ValidateAndRejectInvalidPath(request, clientSocket))
							continue;

						bIsOptions = true;
						contentLength = 0;
						break;
					}

					// Parse Content-Length
					size_t clPos = request.find("Content-Length:");

					if (clPos == std::string::npos)
						clPos = request.find("content-length:");

					if (clPos != std::string::npos)
					{
						const char* clStart = request.c_str() + clPos;
						while (*clStart && *clStart != ':') clStart++;
						if (*clStart == ':') clStart++;
						while (*clStart && (*clStart == ' ' || *clStart == '\t')) clStart++;
						contentLength = atoi(clStart);
					}
					else
					{
						// assume no body
						contentLength = 0;
					}
				}
			}

			// check if we have the complete body
			if (headerEnd > 0 && contentLength >= 0)
			{
				if ((int)(request.length() - headerEnd) >= contentLength)
				{
					break; // complete request received
				}
			}
		}

		if (bConnectionClosed)
		{
			LOGINFO("FHttpTransport: Client closed connection");
			std::lock_guard<std::mutex> lock(Mutex);
			SOCKET_CLOSE(CurrentClientSocket);
			CurrentClientSocket = INVALID_SOCKET_VALUE;
			ClientCV.notify_all();
			continue;
		}

		// Handle options request
		if (bIsOptions)
		{
			LOGINFO("FHttpTransport: Handling OPTIONS request");

			std::string optionsResponse =
				"HTTP/1.1 204 No Content\r\n"
				"Access-Control-Allow-Origin: *\r\n"
				"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
				"Access-Control-Allow-Headers: Content-Type, Accept, MCP-Session-Id, MCP-Protocol-Version\r\n"
				"Connection: close\r\n"
				"\r\n";

			::send(clientSocket, optionsResponse.c_str(), (int)optionsResponse.length(), 0);

			std::lock_guard<std::mutex> lock(Mutex);
			SOCKET_CLOSE(CurrentClientSocket);
			CurrentClientSocket = INVALID_SOCKET_VALUE;
			ClientCV.notify_all();
			continue;	// wait for next client
		}

		// Handle POST with body (normal MCP request)
		if (!bReadError && headerEnd > 0 && contentLength > 0)
		{
			if(!ValidateAndRejectInvalidPath(request, clientSocket))
				continue; // invalid path, already handled
			
			jsonLine = request.substr(headerEnd, contentLength);
			LOGINFO("FHttpTransport: Received MCP request (%d bytes): %s",
				(int)jsonLine.length(),
				jsonLine.substr(0, 100).c_str());
			return true;
		}

		// Handle GET - client wants to open SSE stream for async notifications
		// We don't support SSE, so respond with 405 Method Not Allowed
		if (!bReadError && headerEnd > 0 && request.find("GET ") == 0)
		{
			if(!ValidateAndRejectInvalidPath(request, clientSocket))
				continue; // invalid path, already handled
			
			LOGINFO("FHttpTransport: Rejecting unsupported GET request for SSE");

			std::string methodNotAllowed =
				"HTTP/1.1 405 Method Not Allowed\r\n"
				"Content-Type: application/json\r\n"
				"Content-Length: 56\r\n"
				"Access-Control-Allow-Origin: *\r\n"
				"Connection: close\r\n"
				"\r\n"
				"{\"error\":\"SSE streaming not supported by this server\"}";

			::send(clientSocket, methodNotAllowed.c_str(), (int)methodNotAllowed.length(), 0);

			std::lock_guard<std::mutex> lock(Mutex);
			SOCKET_CLOSE(CurrentClientSocket);
			CurrentClientSocket = INVALID_SOCKET_VALUE;
			ClientCV.notify_all();
			continue;	// wait for next client
		}

		// Handle POST without body 
		if (!bReadError && headerEnd > 0 && contentLength == 0 && request.find("POST ") == 0)
		{
			if(!ValidateAndRejectInvalidPath(request, clientSocket))
				continue; // invalid path, already handled
			
			LOGINFO("FHttpTransport: Received POST with no body");
			jsonLine = "";

			std::lock_guard<std::mutex> lock(Mutex);
			SOCKET_CLOSE(CurrentClientSocket);
			CurrentClientSocket = INVALID_SOCKET_VALUE;
			ClientCV.notify_all();
			continue;	// wait for next client
		}

		// Unknown request type or error
		if (bReadError && !bConnectionClosed)
		{
			LOGWARNING("FHttpTransport: Read error on client socket (headerEnd=%d, contentLength=%d)", headerEnd, contentLength);

			size_t firstLineEnd = request.find("\r\n");
			if(firstLineEnd != std::string::npos)
			{
				LOGWARNING("FHttpTransport: First line of request: %s", request.substr(0, firstLineEnd).c_str());
			}

			std::lock_guard<std::mutex> lock(Mutex);
			SOCKET_CLOSE(CurrentClientSocket);
			CurrentClientSocket = INVALID_SOCKET_VALUE;
			ClientCV.notify_all();
			continue;	// wait for next client
		}
	}

	return false;	// Server closed
}

void FHttpTransport::Close()
{
	bIsClosed = true;
	ClientCV.notify_all();

	if (CurrentClientSocket != INVALID_SOCKET_VALUE)
	{
		SOCKET_CLOSE(CurrentClientSocket);
		CurrentClientSocket = INVALID_SOCKET_VALUE;
	}
	if (ServerSocket != INVALID_SOCKET_VALUE)
	{
		SOCKET_CLOSE(ServerSocket);
		ServerSocket = INVALID_SOCKET_VALUE;
	}
}
