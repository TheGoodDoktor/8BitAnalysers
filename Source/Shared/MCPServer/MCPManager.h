#pragma once
#include "MCPTransport.h"
#include "MCPServer.h"
#include "MCPSuggestions.h"

class FEmuBase;
class FMCPTool;
class FMCPResource;

enum class EMCPTransportType
{
	Stdio,
	HTTP,
};

class FMCPManager
{
public:
	FMCPManager(FMCPToolsRegistry* pTools, FMCPResourceRegistry* pResources) : pToolsRegistry(pTools), pResourcesRegistry(pResources) {}

	~FMCPManager()
	{
	}

	void SetTransportType(EMCPTransportType type, int port = 7777)
	{
		TransportType = type;
		Port = port;
	}

	void Start();
	void Stop();
	void ProcessCommands();

	bool IsRunning() const { return pMCPServer && pMCPServer->IsRunning(); }
	int GetTransportType() const { return (int)TransportType; }

	FMCPSuggestionQueue& GetSuggestionQueue() { return SuggestionQueue; }

private:
	FMCPServer*			pMCPServer = nullptr;
	FMCPToolsRegistry*	pToolsRegistry = nullptr;
	FMCPResourceRegistry* pResourcesRegistry = nullptr;
	FMCPCommandQueue		CommandQueue;
	FMCPResponseQueue		ResponseQueue;
	FMCPSuggestionQueue		SuggestionQueue;
	EMCPTransportType	TransportType = EMCPTransportType::Stdio;

	int			Port = 7777;

};

void InitMCPServer(FEmuBase* pEmu);
void ShutdownMCPServer();
void UpdateMCPServer();
void DrawMCPServerUI(FEmuBase* pEmu);

bool AddMCPTool(const char* toolName, FMCPTool* pTool);
bool AddMCPResource(FMCPResource* pResource);	


extern FMCPManager* g_MCPManager;