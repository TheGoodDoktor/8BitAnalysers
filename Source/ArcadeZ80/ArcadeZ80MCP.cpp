#include "ArcadeZ80MCP.h"

#define NOMINMAX

#include "MCPServer/MCPManager.h"
#include "MCPServer/MCPTools.h"
#include "MCPServer/MCPResources.h"
#include "ArcadeZ80.h"
#include "CodeAnalyser/AssemblerExport.h"


FMCPManager* g_MCPManager = nullptr;
FMCPToolsRegistry* g_MCPToolsRegistry = nullptr;
FMCPResourceRegistry* g_MCPResourceRegistry = nullptr;

void RegisterArcadeZ80Tools();
void RegisterArcadeZ80Resources();

void InitMCPServer(FArcadeZ80* pEmu)
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

	RegisterArcadeZ80Tools();
	RegisterArcadeZ80Resources();
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
		Description = "Reads memory from specified memory area within a 16-bit address space, the area cannot go beyond 0xFFFF";

		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Starting memory address to read from within a 16-bit address space"}
				}},
				{"length", {
					{"type", "integer"},
					{"description", "Number of bytes to read within a 16-bit address space"}
				}}
			}},
			{"required", {"address", "length"}}
		};
	}


	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments)
	{
		// For demonstration, return dummy data
		uint32_t address = arguments["address"].get<uint32_t>();
		uint32_t length = arguments["length"].get<uint32_t>();
		
		
		// In real implementation, read memory from the emulated system
		std::vector<uint8_t> data;
		for (uint32_t i = 0; i < length; ++i)
		{
			data.push_back(pEmu->ReadByte(address + i));
		}

		/*
		std::ostringstream hex_ss;
		for (size_t i = 0; i < data.size(); i++)
		{
			hex_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)data[i];
			if (i < data.size() - 1)
				hex_ss << " ";
		}*/

		nlohmann::json result;
		result["address"] = address;
		result["length"] = length;
		result["data"] = data;//hex_ss.str();
		return result;
	}

};

// Go to address command
// GetFocussedViewState().GoToAddress

class FGoToAddressTool : public FMCPTool
{
public:
	FGoToAddressTool()
	{
		Description = "Moves the code analysis view to the specified address within a 16-bit address space";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Memory address to go to within a 16-bit address space"}
				}}
			}},
			{"required", {"address"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments)
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		uint32_t address = arguments["address"].get<uint32_t>();

		FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
		codeAnalysis.GetFocussedViewState().GoToAddress(addrRef);
		return { {"success", true} };
	}
};

// Add comment tool
class FAddCommentTool : public FMCPTool
{
public:
	FAddCommentTool()
	{
		Description = "Adds a comment to the specified address within a 16-bit address space";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Memory address to add comment to within a 16-bit address space"}
				}},
				{"comment", {
					{"type", "string"},
					{"description", "The comment text to add"}
				}}
			}},
			{"required", {"address", "comment"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments)
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		uint32_t address = arguments["address"].get<uint32_t>();
		std::string comment = arguments["comment"].get<std::string>();
		FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
		FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(addrRef);
		if (pCodeInfo)
		{
			pCodeInfo->Comment = comment;
			return { {"success", true} };
		}
		return { {"success", false}, {"error", "Invalid address"} };
	}
};


void RegisterArcadeZ80Tools()
{
	if (g_MCPToolsRegistry)
	{
		// Register tools here
		g_MCPToolsRegistry->RegisterTool("read_memory", new FReadMemoryTool());
		g_MCPToolsRegistry->RegisterTool("go_to_address", new FGoToAddressTool());
		//g_MCPToolsRegistry->RegisterTool("add_comment", new FAddCommentTool());
	}
	
}

// Resources can be registered here too

class FDisassemblyResource : public FMCPResource
{
public:
	FDisassemblyResource()
	{
		URI = "arcadez80://disassembly";
		Title = "Z80 Disassembly";
		Description = "Z80 Disassembly of the currently loaded program";
		MimeType = "text/plain";
		Category = "code";
	}

	std::string Read(FEmuBase* pEmulator) override
	{
		// Generate disassembly string
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
		std::string outStr;
		//bool ExportAssembler(FEmuBase * pEmu, std::string * pOutStr, uint16_t startAddr, uint16_t endAddr)

		ExportAssembler(pEmulator, &outStr, 0x0000, 0x5FFF);
		return outStr;

// 		std::ostringstream disasmStream;
// 		// Simple disassembly loop for demonstration
// 		for (uint16_t addr = 0x0000; addr <= 0xFFFF; ++addr)
// 		{
// 			FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(addr);
// 			if (pCodeInfo && pCodeInfo->bIsInstruction)
// 			{
// 				disasmStream << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addr << ": ";
// 				disasmStream << " ; "; // Placeholder for actual disassembly
// 				disasmStream << "\n";
// 			}
// 		}
// 		return disasmStream.str();
	}
};

void RegisterArcadeZ80Resources()
{
	if (g_MCPResourceRegistry)
	{
		// Register resources here
		g_MCPResourceRegistry->RegisterResource(new FDisassemblyResource());
	}
}