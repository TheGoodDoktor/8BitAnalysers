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

void RegisterArcadeZ80Tools();

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

	RegisterArcadeZ80Tools();
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

// TODO: Create some tools for ArcadeZ80 here
// Start with one which reads memory

class FReadMemoryTool : public FMCPTool
{
public:
	FReadMemoryTool()
	{
		Description = "Reads memory from specified memory area";

		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Starting memory address to read from"}
				}},
				{"length", {
					{"type", "integer"},
					{"description", "Number of bytes to read"}
				}}
			}},
			{"required", {"address", "length"}}
		};
	}


	nlohmann::json Execute(FProgramInterface* pProgramIF, const nlohmann::json& arguments)
	{
		// For demonstration, return dummy data
		uint32_t address = arguments["address"].get<uint32_t>();
		uint32_t length = arguments["length"].get<uint32_t>();
		// In real implementation, read memory from the emulated system
		std::vector<uint8_t> data;
		for (uint32_t i = 0; i < length; ++i)
		{
			data.push_back((address + i) & 0xFF); // Dummy data
		}
		nlohmann::json result;
		result["address"] = address;
		result["length"] = length;
		result["data"] = data;
		return result;
	}

};


void RegisterArcadeZ80Tools()
{
	if (g_MCPToolsRegistry)
	{
		// Register tools here
		g_MCPToolsRegistry->RegisterTool("read_memory", new FReadMemoryTool());
	}
}