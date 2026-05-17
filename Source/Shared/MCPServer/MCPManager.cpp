#include "MCPManager.h"
#include "MCPTools.h"
#include "MCPResources.h"
#include "MCPSuggestionsUI.h"

FMCPManager* g_MCPManager = nullptr;
FMCPToolsRegistry* g_MCPToolsRegistry = nullptr;
FMCPResourceRegistry* g_MCPResourceRegistry = nullptr;

void FMCPManager::Start()
{
	if (pMCPServer && pMCPServer->IsRunning())
		return;

	CommandQueue.Clear();
	ResponseQueue.Reset();

	// Create transport
	FMCPTransport* pTransport = nullptr;
	if (TransportType == EMCPTransportType::Stdio)
	{
		pTransport = new FStdioTransport();
	}
	else if (TransportType == EMCPTransportType::HTTP)
	{
		pTransport = new FHttpTransport(Port);
	}

	pMCPServer = new FMCPServer(pTransport, pToolsRegistry, pResourcesRegistry, CommandQueue, ResponseQueue);
	pMCPServer->Start();

	if (TransportType == EMCPTransportType::HTTP)
		LOGINFO("MCP Server started. Connect at: http://localhost:%d/mcp", Port);
	else
		LOGINFO("MCP Server started on stdio");
}

void FMCPManager::Stop()
{
	if (pMCPServer)
	{
		pMCPServer->Stop();

		if (pMCPServer->GetTransport() != nullptr)
			pMCPServer->GetTransport()->Close();

		delete pMCPServer;
		pMCPServer = nullptr;
	}
}

void FMCPManager::ProcessCommands()
{
	FMCPCommand* pCmd = nullptr;
	while ((pCmd = CommandQueue.Pop()) != nullptr)
	{
		FMCPResponse* pResponse = new FMCPResponse();
		pResponse->RequestId = pCmd->RequestId;
		pResponse->bIsError = false;

		pResponse->Result = pCmd->Execute(pMCPServer);

		ResponseQueue.Push(pResponse);
		delete pCmd;
		pCmd = nullptr;
	}
}


void InitMCPServer(FEmuBase* pEmu)
{
	EMCPTransportType transportType = EMCPTransportType::HTTP;
	int port = 7777;

	if (g_MCPManager == nullptr)
	{
		g_MCPToolsRegistry = new FMCPToolsRegistry(pEmu);
		g_MCPResourceRegistry = new FMCPResourceRegistry(pEmu);
		g_MCPManager = new FMCPManager(g_MCPToolsRegistry, g_MCPResourceRegistry);
	}
	g_MCPManager->SetTransportType(transportType, port);
	g_MCPManager->Start();

	RegisterBaseTools(*g_MCPToolsRegistry);
	RegisterBaseResources(*g_MCPResourceRegistry);
}

void ShutdownMCPServer()
{
	if (g_MCPManager)
	{
		g_MCPManager->Stop();
		delete g_MCPManager;
		g_MCPManager = nullptr;
	}
	if (g_MCPToolsRegistry)
	{
		delete g_MCPToolsRegistry;
		g_MCPToolsRegistry = nullptr;
	}
	if (g_MCPResourceRegistry)
	{
		delete g_MCPResourceRegistry;
		g_MCPResourceRegistry = nullptr;
	}
}

bool AddMCPTool(const char* toolName, FMCPTool* pTool)
{
	if (g_MCPToolsRegistry)
	{
		g_MCPToolsRegistry->RegisterTool(toolName, pTool);
		return true;
	}
	return false;
}

bool AddMCPResource(FMCPResource* pResource)
{
	if (g_MCPResourceRegistry)
	{
		g_MCPResourceRegistry->RegisterResource(pResource);
		return true;
	}
	return false;
}


void UpdateMCPServer()
{
	if (g_MCPManager)
	{
		g_MCPManager->ProcessCommands();
	}
}

void DrawMCPServerUI(FEmuBase* pEmu)
{
	if (g_MCPManager)
	{
		DrawMCPSuggestionsUI(pEmu, g_MCPManager->GetSuggestionQueue());
	}
}