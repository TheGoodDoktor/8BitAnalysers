#include "MCPServer.h"
#include "MCPTools.h"

#include <sstream>
#include <fstream>

static const std::string kServerName = "arcadez80-mcp-server";
static const std::string kServerVersion = "1.0";
static const std::string kServerHttpPrefix = "ArcadeZ80://";


FMCPServer::FMCPServer(FMCPTransport* pTransport, FMCPToolsRegistry* toolsRegistry, FMCPCommandQueue& commandQueue, FMCPResponseQueue& responseQueue)
	: pTransport(pTransport)
	, pToolsRegistry(toolsRegistry)
	, CommandQueue(commandQueue)
	, ResponseQueue(responseQueue)
{
	bRunning = false;
	bInitialized = false;
}

FMCPServer::~FMCPServer()
{
	Stop();
	delete pTransport;
}

void FMCPServer::Start()
{
	if (bRunning.load())
		return;

	LoadResources();
	bRunning.store(true);
	Thread = std::thread(&FMCPServer::Run, this);
}

void FMCPServer::Stop()
{
	bRunning.store(false);
	ResponseQueue.Stop();

	// Detach the thread instead of joining to avoid blocking on stdin read
	// The thread will exit naturally when ReceiveData() returns or on next iteration
	if (Thread.joinable())
		Thread.detach();
}

static void* ReaderThreadFunc(void* arg)
{
	FMCPServer* server = (FMCPServer*)arg;
	server->ReaderLoop();
	return NULL;
}

void FMCPServer::ReaderLoop()
{
	while (bRunning.load())
	{
		std::string line;
		if(pTransport->ReceiveData(line))
		{
			if(!line.empty())
			{
				HandleLine(line);
			}
		}
		else
		{
			Stop();
			break;
		}
	}
}

void FMCPServer::Run()
{
	std::thread readerThread(ReaderThreadFunc, this);
	readerThread.detach();

	while (bRunning.load())
	{
		FMCPResponse* pResponse = ResponseQueue.WaitAndPop();
		if(pResponse == nullptr)
			break;

		if(pResponse->bIsError)
		{
			SendError(pResponse->RequestId, pResponse->ErrorCode, pResponse->ErrorMessage);
		}
		else
		{
			nlohmann::json mcpResult;
			mcpResult["content"] = nlohmann::json::array();

			// Check is this is image data
			if(pResponse->Result.contains("__mcp_image") && pResponse->Result["__mcp_image"] == true)
			{
				// Image content type
				mcpResult["content"].push_back({
					{"type", "image"},
					{"data", pResponse->Result["data"]},
					{"mimeType", pResponse->Result["mimeType"]}
					});
			}
			else
			{
				// Text content type
				std::ostringstream resultStrStream;
				resultStrStream << pResponse->Result.dump(2);

				mcpResult["content"].push_back({
					{"type", "text"},
					{"text", resultStrStream.str()}
					});
			}

			nlohmann::json response;
			response["jsonrpc"] = "2.0";
			response["id"] = pResponse->RequestId;
			response["result"] = mcpResult;

			SendResponse(response);
		}

		delete pResponse;
	}
}

void FMCPServer::HandleLine(const std::string& line)
{
	// check if we can parse JSON
	if (!nlohmann::json::accept(line))
	{
		SendError(0, -32700, "Parse error: Invalid JSON");
		return;
	}

	nlohmann::json request = nlohmann::json::parse(line);

	// Validate JSON-RPC structure
	if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0")
	{
		SendError(0, -32600, "Invalid Request: missing or invalid jsonrpc version");
		return;
	}

	if (!request.contains("method") || !request["method"].is_string())
	{
		SendError(0, -32600, "Invalid Request: missing method");
		return;
	}

	std::string method = request["method"];

	// Handle different methods
	if (method == "initialize")
	{
		HandleInitialise(request);
	}
	else if (method == "notifications/initialized")
	{
		// Notifications don't need JSON-RPC response, but HTTP needs HTTP response
		if (dynamic_cast<FHttpTransport*>(pTransport))
		{
			pTransport->SendData("{}");
		}
	}
	else if (method == "tools/list")
	{
		HandleToolsList(request);
	}
	else if (method == "tools/call")
	{
		HandleToolsCall(request);
	}
	else if (method == "resources/list")
	{
		HandleResourcesList(request);
	}
	else if (method == "resources/read")
	{
		HandleResourcesRead(request);
	}
	else
	{
		int64_t id = request.contains("id") ? request["id"].get<int64_t>() : 0;
		SendError(id, -32601, "Method not found: " + method);
	}

}

void FMCPServer::HandleInitialise(const nlohmann::json& request)
{
	if (!request.contains("id"))
	{
		SendError(0, -32600, "Invalid Request: missing id");
		return;
	}

	const int64_t id = request["id"].get<int64_t>();

	// Get protocol version
	std::string protocolVersion = "1.0";
	if (request.contains("params") && request["params"].contains("protocolVersion"))
	{
		protocolVersion = request["params"]["protocolVersion"].get<std::string>();
	}

	// Prepare response
	nlohmann::json response;
	response["jsonrpc"] = "2.0";
	response["id"] = id;
	response["result"] = {
		{"protocolVersion", protocolVersion},
		{"capabilities", {
			{"tools", nlohmann::json::object()},
			{"resources", nlohmann::json::object()}
		}},
		{"serverInfo", {
			{"name", kServerName},
			{"version", kServerVersion}
		}}
	};

	bInitialized = true;
	SendResponse(response);
}

void FMCPServer::HandleToolsList(const nlohmann::json& request)
{
	if (!request.contains("id"))
	{
		SendError(0, -32600, "Invalid Request: missing id");
		return;
	}

	const int64_t id = request["id"].get<int64_t>();
	nlohmann::json toolsArray = nlohmann::json::array();

	pToolsRegistry->GenerateToolsList(toolsArray);

	nlohmann::json response;
	response["jsonrpc"] = "2.0";
	response["id"] = id;
	response["result"] = {
		{"tools", toolsArray}
	};

	SendResponse(response);
}

void FMCPServer::HandleToolsCall(const nlohmann::json& request)
{
	if (!request.contains("id"))
	{
		SendError(0, -32600, "Invalid Request: missing id");
		return;
	}

	int64_t id = request["id"];

	if (!request.contains("params") || !request["params"].contains("name"))
	{
		SendError(id, -32602, "Invalid params: missing tool name");
		return;
	}

	std::string toolName = request["params"]["name"];

	nlohmann::json arguments = request["params"].contains("arguments") ? request["params"]["arguments"] : nlohmann::json::object();
	// Enqueue command for main thread to execute
	FMCPCommand* cmd = new FMCPCommand();
	cmd->RequestId = id;
	cmd->ToolName = toolName;
	cmd->Arguments = arguments;
	CommandQueue.Push(cmd);

}

nlohmann::json FMCPServer::ExecuteCommand(const std::string& toolName, const nlohmann::json& arguments)
{
	// Normalize tool name: VS Code converts underscores to dots
	std::string normalizedTool = toolName;
	size_t pos = 0;
	while ((pos = normalizedTool.find('.', pos)) != std::string::npos) 
	{
		normalizedTool[pos] = '_';
		pos++;
	}

	// Find tool and execute it
	nlohmann::json result;
	if (pToolsRegistry->ExecuteTool(normalizedTool, arguments, result))
	{
		return result;
	}

	// Report unknown tool for now
	return { {"error", "Unknown tool: " + toolName} };
}

void FMCPServer::SendResponse(const nlohmann::json& response)
{
	std::string responseStr = response.dump();
	pTransport->SendData(responseStr);
}

void FMCPServer::SendError(int64_t id, int code, const std::string& message, const nlohmann::json& data)
{
	nlohmann::json errorResponse;
	errorResponse["jsonrpc"] = "2.0";
	errorResponse["id"] = id;
	errorResponse["error"] = {
		{"code", code},
		{"message", message}
	};

	if (!data.is_null() && !data.empty())
	{
		errorResponse["error"]["data"] = data;
	}

	LOGERROR("MCP Error %d: %s", code, message.c_str());

	std::string responseStr = errorResponse.dump();
	pTransport->SendData(responseStr);
}

// Resources
void FMCPServer::LoadResources()
{
	// TODO: this might need some wrangling to find the correct path in different environments
	std::string resourcesPath = "/mcp/resources";

	LoadResourcesFromCategory("hardware", resourcesPath + "/hardware/toc.json");
}

void FMCPServer::LoadResourcesFromCategory(const std::string& category, const std::string& tocPath)
{
	// Read toc.json
	std::ifstream tocFile(tocPath);
	if (!tocFile.is_open())
	{
		LOGERROR("Failed to open resource TOC: %s", tocPath.c_str());
		return;
	}

	std::stringstream tocBuffer;
	tocBuffer << tocFile.rdbuf();
	tocFile.close();

	std::string tocContent = tocBuffer.str();

	if(!nlohmann::json::accept(tocContent))
	{
		LOGERROR("Invalid JSON in resource TOC: %s", tocPath.c_str());
		return;
	}

	nlohmann::json tocJson = nlohmann::json::parse(tocContent);
	if(!tocJson.contains("toc") || !tocJson["toc"].is_array())
	{
		LOGERROR("Invalid TOC format in: %s", tocPath.c_str());
		return;
	}

	// get the directory containing the TOC file
	std::string tocDirectory = tocPath.substr(0, tocPath.find_last_of("/\\"));

	for(const nlohmann::json& entry : tocJson["toc"])
	{
		if(!entry.contains("uri") || !entry.contains("title") || !entry.contains("filePath"))
		{
			LOGERROR("Invalid TOC entry in: %s", tocPath.c_str());
			continue;
		}
		FResourceInfo resource;
		resource.Uri = kServerHttpPrefix + category + "/" + entry["uri"].get<std::string>();
		resource.Title = entry["title"].get<std::string>();
		resource.Description = entry.contains("description") ? entry["description"].get<std::string>() : "";
		resource.MimeType = entry.contains("mimeType") ? entry["mimeType"].get<std::string>() : "text/plain";
		resource.Category = category;		
		resource.FilePath = tocDirectory + "/" + entry["uri"].get<std::string>() + ".md";
		Resources.push_back(resource);

		ResourceMap[resource.Uri] = resource;
	}
}

std::string FMCPServer::ReadFileContents(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		LOGERROR("Failed to open resource file: %s", filePath.c_str());
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

void FMCPServer::HandleResourcesList(const nlohmann::json& request)
{
	if (!request.contains("id"))
	{
		SendError(0, -32600, "Invalid Request: missing id");
		return;
	}
	const int64_t id = request["id"].get<int64_t>();
	nlohmann::json resourcesJson = nlohmann::json::array();
	for (const FResourceInfo& resource : Resources)
	{
		nlohmann::json resourceJson;
		resourceJson["uri"] = resource.Uri;
		resourceJson["name"] = resource.Title;
		resourceJson["description"] = resource.Description;
		resourceJson["mimeType"] = resource.MimeType;

		resourcesJson.push_back(resourceJson);
	}

	nlohmann::json response;
	response["jsonrpc"] = "2.0";
	response["id"] = id;
	response["result"] = {
		{"resources", resourcesJson}
	};

	SendResponse(response);
}

void FMCPServer::HandleResourcesRead(const nlohmann::json& request)
{
	if (!request.contains("id"))
	{
		SendError(0, -32600, "Invalid Request: missing id");
		return;
	}
	
	const int64_t id = request["id"].get<int64_t>();
	if (!request.contains("params") || !request["params"].contains("uri"))
	{
		SendError(id, -32602, "Invalid Request: missing uri parameter");
		return;
	}

	std::string uri = request["params"]["uri"].get<std::string>();

	// Find resource
	auto resourceIt = ResourceMap.find(uri);

	if (resourceIt == ResourceMap.end())
	{
		SendError(id, -32602, "Resource not found: " + uri);
		return;
	}

	const FResourceInfo& resource = resourceIt->second;
	std::string content = ReadFileContents(resource.FilePath);

	if(content.empty())
	{
		SendError(id, -32602, "Failed to read resource content: " + resource.FilePath);
		return;
	}

	nlohmann::json response;
	response["jsonrpc"] = "2.0";
	response["id"] = id;
	response["result"] = {
		{"contents", nlohmann::json::array({
			{
				{"uri", resource.Uri},
				{"mimeType", resource.MimeType},
				{"data", content}
			}
		})}
	};

	SendResponse(response);
}