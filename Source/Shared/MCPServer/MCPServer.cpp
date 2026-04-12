#include "MCPServer.h"
#include "MCPTools.h"
#include "MCPResources.h"

#include <sstream>
#include <fstream>

static const std::string kServerVersion = "1.0";

std::string g_ServerName = "analyser";

void SetMCPServerName(const std::string& name)
{
	g_ServerName = name;
}

const char* GetMCPServerName()
{
	return g_ServerName.c_str();
}


nlohmann::json	FMCPToolCommand::Execute(FMCPServer* pServer)
{
	FMCPToolsRegistry* pToolsRegistry = pServer->GetToolsRegistry();

	// Normalize tool name: VS Code converts underscores to dots
	std::string normalizedTool = ToolName;
	size_t pos = 0;
	while ((pos = normalizedTool.find('.', pos)) != std::string::npos)
	{
		normalizedTool[pos] = '_';
		pos++;
	}

	// Find tool and execute it
	nlohmann::json result;
	if (pToolsRegistry->ExecuteTool(normalizedTool, Arguments, result))
	{
		return result;
	}

	// Report unknown tool for now
	return { {"error", "Unknown tool: " + ToolName} };
}

nlohmann::json FMCPResourceCommand::Execute(FMCPServer* pServer)
{
	FMCPResourceRegistry* pResourcesRegistry = pServer->GetResourcesRegistry();
	FMCPResource* pResource = pResourcesRegistry->GetResource(ResourceUri);
	if (pResource)
	{
		std::string data = pResourcesRegistry->ReadResource(pResource);

		nlohmann::json result;
		result["data"] = data;
		result["mimeType"] = pResource->MimeType;
		result["__mcp_resource_uri"] = ResourceUri;	// signals Run() to use resources/read response format
		result["__mcp_image"] = (pResource->MimeType.find("image/") == 0);
		return result;
	}
	return { {"error", "Unknown resource: " + ResourceUri} };
}


FMCPServer::FMCPServer(FMCPTransport* pTransport, FMCPToolsRegistry* toolsRegistry, FMCPResourceRegistry* resourcesRegistry, FMCPCommandQueue& commandQueue, FMCPResponseQueue& responseQueue)
	: pTransport(pTransport)
	, pToolsRegistry(toolsRegistry)
	, pResourcesRegistry(resourcesRegistry)
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

			// Resource read responses use a different format to tool call responses
			if(pResponse->Result.contains("__mcp_resource_uri"))
			{
				// MCP resources/read format: result.contents = [{uri, mimeType, text|blob}]
				nlohmann::json contentItem;
				contentItem["uri"] = pResponse->Result["__mcp_resource_uri"];
				contentItem["mimeType"] = pResponse->Result["mimeType"];
				if(pResponse->Result.contains("__mcp_image") && pResponse->Result["__mcp_image"] == true)
					contentItem["blob"] = pResponse->Result["data"]; // base64 for binary/image
				else
					contentItem["text"] = pResponse->Result["data"];
				mcpResult["contents"] = nlohmann::json::array({contentItem});
			}
			else
			{
				// Tool call format: result.content = [{type, text|image}]
				mcpResult["content"] = nlohmann::json::array();

				// Tool errors: return as successful JSON-RPC response with isError:true in content
				if(pResponse->Result.contains("error"))
				{
					mcpResult["isError"] = true;
					mcpResult["content"].push_back({
						{"type", "text"},
						{"text", pResponse->Result["error"].get<std::string>()}
						});
				}
				// Check if this is image data
				else if(pResponse->Result.contains("__mcp_image") && pResponse->Result["__mcp_image"] == true)
				{
					mcpResult["content"].push_back({
						{"type", "image"},
						{"data", pResponse->Result["data"]},
						{"mimeType", pResponse->Result["mimeType"]}
						});
				}
				else
				{
					std::ostringstream resultStrStream;
					resultStrStream << pResponse->Result.dump(2);
					mcpResult["content"].push_back({
						{"type", "text"},
						{"text", resultStrStream.str()}
						});
				}
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
		// Notification — no JSON-RPC response needed, but HTTP requires an HTTP response
		pTransport->SendData("");
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




std::string FMCPServer::GetInstructions() const
{
	const std::string p = std::string(GetMCPServerName()) + "://";

	return
		"You are connected to a tool which is used to analyse 8-bit games. "
		"As well as a disassembler, it provides various tools to query the game code and data, recording memory accesses for code and data locations. "
		"\n"
		"## Recommended workflow\n"
		"1. Read '" + p + "function-index' for a compact overview of all known functions and their descriptions.\n"
		"2. Read '" + p + "labels' for the full symbol table - named RAM variables, I/O ports, data structures, etc.\n"
		"3. Read '" + p + "call-graph' to understand which functions call which, without repeated tool calls.\n"
		"4. Use get_function_disassembly to inspect individual functions in detail.\n"
		"5. Use disassemble_address_range for ad-hoc address ranges not covered by a named function.\n"
		"6. The '" + p + "disassembly' resource provides the FULL annotated assembly listing (~280KB). "
		"Only request it when you need a broad search across the entire codebase - it is too large to read routinely.\n"
		"\n"
		"## Write-back tools\n"
		"When you understand what a function, label, or instruction does, record your findings using: "
		"rename_function, set_function_description, set_label, and add_comment. "
		"These suggestions are queued for the user to review and accept or reject in the GUI - they are NOT applied immediately. "
		"Always provide a rationale argument explaining your reasoning so the user can make an informed decision.\n";
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
			{"name", g_ServerName},
			{"version", kServerVersion}
		}},
		{"instructions", GetInstructions()}
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
	FMCPToolCommand* cmd = new FMCPToolCommand();
	cmd->RequestId = id;
	cmd->ToolName = toolName;
	cmd->Arguments = arguments;
	CommandQueue.Push(cmd);
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

	pResourcesRegistry->GenerateResourcesList(resourcesJson);
	/*
	for (const FResourceInfo& resource : Resources)
	{
		nlohmann::json resourceJson;
		resourceJson["uri"] = resource.Uri;
		resourceJson["name"] = resource.Title;
		resourceJson["description"] = resource.Description;
		resourceJson["mimeType"] = resource.MimeType;

		resourcesJson.push_back(resourceJson);
	}
	*/
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

	// queue resource read
	FMCPResourceCommand* pResourceCmd = new FMCPResourceCommand();
	pResourceCmd->RequestId = id;
	pResourceCmd->ResourceUri = uri;
	CommandQueue.Push(pResourceCmd);
#if 0
	nlohmann::json response;

	FMCPResource* pResource = pResourcesRegistry->GetResource(uri);
	if (!pResource)
	{
		SendError(id, -32602, "Resource not found: " + uri);
		return;
	}

	std::string content = pResourcesRegistry->ReadResource(pResource);

	if (content.empty())
	{
		SendError(id, -32602, "Failed to read resource content: " + uri);
		return;
	}

	response["jsonrpc"] = "2.0";
	response["id"] = id;
	response["result"] = {
		{"contents", nlohmann::json::array({
			{
				{"uri", uri},
				{"mimeType", pResource->MimeType},
				{"data", content}
			}
		})}
	};	

	SendResponse(response);
#endif
}