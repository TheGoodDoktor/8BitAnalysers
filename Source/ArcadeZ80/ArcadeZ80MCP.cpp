#include "ArcadeZ80MCP.h"

#define NOMINMAX

#include "MCPServer/MCPManager.h"
#include "MCPServer/MCPTools.h"

class FArcadeZ80Interface : public FProgramInterface
{
public:
	FArcadeZ80Interface()
	{
	}
};


FMCPManager* g_MCPManager = nullptr;
FMCPToolsRegistry* g_MCPToolsRegistry = nullptr;
FArcadeZ80Interface* g_ArcadeZ80Interface = nullptr;


void InitMCPServer(void)
{
	EMCPTransportType transportType = EMCPTransportType::HTTP;
	int port = 7777;

	if (g_MCPManager == nullptr)
	{
		g_ArcadeZ80Interface = new FArcadeZ80Interface();
		g_MCPToolsRegistry = new FMCPToolsRegistry(g_ArcadeZ80Interface);
		g_MCPManager = new FMCPManager(g_MCPToolsRegistry);
	}
	g_MCPManager->SetTransportType(transportType, port);
	g_MCPManager->Start();
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
}

void UpdateMCPServer()
{
	if (g_MCPManager)
	{
		g_MCPManager->ProcessCommands();
	}
}