#include "MCPTools.h"
#include "Misc/EmuBase.h"

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

void RegisterBaseTools(FMCPToolsRegistry& registry)
{
	registry.RegisterTool("read_memory", new FReadMemoryTool());
	registry.RegisterTool("go_to_address", new FGoToAddressTool());
	registry.RegisterTool("add_comment", new FAddCommentTool());
}