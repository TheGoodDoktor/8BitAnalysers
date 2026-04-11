#include "MCPTools.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include "CodeAnalyser/AssemblerExport.h"

class FGetFunctionListTool : public FMCPTool
{
public:
	FGetFunctionListTool()
	{
		Description = "Gets a list of all functions in the code analysis";
		InputSchema = {
			{"type", "object"},
			{"properties", {}},
			{"required", {}}
		};
	}
	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		nlohmann::json result;
		nlohmann::json functions = nlohmann::json::array();
		for (const auto& funcIt : codeAnalysis.pFunctions->GetFunctions())
		{
			const auto& func = funcIt.second;
			FAddressRef funcAddr = funcIt.first;
			FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(func.StartAddress);
			nlohmann::json funcJson;
			if(pLabel != nullptr)
			{
				funcJson["name"] = pLabel->GetName();
				funcJson["address"] = func.StartAddress.Address;
				functions.push_back(funcJson);
			}
		}
		result["functions"] = functions;
		return result;
	}
};

class FGetFunctionInfoTool : public FMCPTool
{
public:
	FGetFunctionInfoTool()
	{
		Description = "Gets information about a function, such as its start address, description of what it does, and its parameters";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"function_name", {
					{"type", "string"},
					{"description", "Name of the function to get info on"}
				}}
			}},
			{"required", {"function_name"}}
		};
	}
	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		std::string functionName = arguments["function_name"].get<std::string>();
		FAddressRef funcAddress;
		const FFunctionInfo* pFuncInfo = codeAnalysis.pFunctions->FindFunctionByName(functionName.c_str());
		if (pFuncInfo)
		{
			nlohmann::json result;
			result["name"] = functionName;
			result["description"] = pFuncInfo->Description; 
			result["address"] = pFuncInfo->StartAddress.Address;

			// parameters
			if (pFuncInfo->Params.empty() == false)
			{
				nlohmann::json params = nlohmann::json::array();
				for (const auto& param : pFuncInfo->Params)
				{
					nlohmann::json paramJson;
					paramJson["name"] = param.Name;
					paramJson["description"] = param.GenerateDescription(codeAnalysis);
					params.push_back(paramJson);
				}
				result["parameters"] = params;
			}

			// return values
			if(pFuncInfo->ReturnValues.empty() == false)
			{
				nlohmann::json returnValues = nlohmann::json::array();
				for (const auto& returnValue : pFuncInfo->ReturnValues)
				{
					nlohmann::json returnValueJson;
					returnValueJson["name"] = returnValue.Name;
					returnValueJson["description"] = returnValue.GenerateDescription(codeAnalysis);
					returnValues.push_back(returnValueJson);
				}
				result["return_values"] = returnValues;
			}

			// call points
			if (pFuncInfo->CallPoints.empty() == false)
			{
				nlohmann::json callPoints = nlohmann::json::array();
				for (const auto& callPoint : pFuncInfo->CallPoints)
				{
					nlohmann::json callPointJson;
					callPointJson["address"] = callPoint.CallAddr.Address;
					callPoints.push_back(callPointJson);
				}
				result["call_points"] = callPoints;
			}

			// exit points
			if (pFuncInfo->ExitPoints.empty() == false)
			{
				nlohmann::json exitPoints = nlohmann::json::array();
				for (const auto& exitPoint : pFuncInfo->ExitPoints)
				{
					nlohmann::json exitPointJson;
					exitPointJson["address"] = exitPoint.Address;
					exitPoints.push_back(exitPointJson);
				}
				result["exit_points"] = exitPoints;
			}

			return result;
		}
		return { {"success", false}, {"error", "Function not found"} };
	}
};

// This tool gets the disassembly of a function specified by the user
class FGetFunctionDisassemblyTool : public FMCPTool
{
public:

	FGetFunctionDisassemblyTool()
	{
		Description = "Gets the annotated disassembly of a function";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"function_name", {
					{"type", "string"},
					{"description", "Name of the function to get disassembly for"}
				}}
			}},
			{"required", {"function_name"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		std::string functionName = arguments["function_name"].get<std::string>();
		FAddressRef funcAddress;
		const FFunctionInfo* pFuncInfo = codeAnalysis.pFunctions->FindFunctionByName(functionName.c_str());
		if (pFuncInfo)
		{
			nlohmann::json result;
			result["name"] = functionName;

			std::string outStr;
			ExportAssembler(pEmu, &outStr, pFuncInfo->StartAddress.Address, pFuncInfo->EndAddress.Address);
			result["disassembly"] = outStr;

			return result;
		}
		else
		{
			return { {"success", false}, {"error", "Function not found"} };
		}
	}

};

// This tool gets the disassembly of a range of addresses specified by the user
class FDisassembleAddressRangeTool : public FMCPTool
{
public:
	FDisassembleAddressRangeTool()
	{
		Description = "Gets the annotated disassembly of a memory range within a 16-bit address space, the area cannot go beyond 0xFFFF";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"start_address", {
					{"type", "integer"},
					{"description", "Starting address of the range to disassemble within a 16-bit address space"}
				}},
				{"end_address", {
					{"type", "integer"},
					{"description", "Ending address of the range to disassemble within a 16-bit address space"}
				}}
			}},
			{"required", {"start_address", "end_address"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		const uint32_t startAddress = GetNumericalArgument("start_address", arguments);
		const uint32_t endAddress = GetNumericalArgument("end_address", arguments);
		nlohmann::json result;
		std::string outStr;
		ExportAssembler(pEmu, &outStr, startAddress, endAddress);
		result["disassembly"] = outStr;
		return result;
	}
};

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
		uint32_t address = GetNumericalArgument("address", arguments);
		uint32_t length = GetNumericalArgument("length", arguments);


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

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		uint32_t address = GetNumericalArgument("address", arguments);

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

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		uint32_t address = GetNumericalArgument("address", arguments);
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

// Get code info tool
class FGetCodeInfoTool : public FMCPTool
{
	public:
		FGetCodeInfoTool()
		{
			Description = "Get information about the instruction code at a specific address, such as which memory addresses it reads and writes to";
			InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Memory address of the instruction"}
				}}
			}},
			{"required", {"address"}}
			};
		}		

		nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
		{
			FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
			uint32_t address = GetNumericalArgument("address", arguments);//arguments["address"].get<uint32_t>();
			FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
			FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(addrRef);
			if(pCodeInfo)
			{
				// output code info as json
				nlohmann::json result;
				result["disassembly"] = pCodeInfo->Text;
				result["no_times_executed"] = pCodeInfo->ExecutionCount;

				// reads
				nlohmann::json reads = nlohmann::json::array();
				auto& readRefs = pCodeInfo->Reads.GetReferences();
				for (auto& read : readRefs)
				{
					reads.push_back(read.Address);
				}
				result["memory_reads"] = reads;

				// writes
				nlohmann::json writes = nlohmann::json::array();
				auto& writeRefs = pCodeInfo->Writes.GetReferences();
				for (auto& write : writeRefs)
				{
					writes.push_back(write.Address);
				}
				result["memory_writes"] = writes;

				return result;
			}
			else
			{
				return { {"success", false}, {"error", "No code at address"} };
			}
		}
};


// GetDataInfoTool
class FGetDataInfoTool : public FMCPTool
{
public:
	FGetDataInfoTool()
	{
		Description = "Get information about the data at a specific address, such as what code has read and written to it";
		InputSchema = {
		{"type", "object"},
		{"properties", {
			{"address", {
				{"type", "integer"},
				{"description", "Memory address of the data to get info on"}
			}}
		}},
		{"required", {"address"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		uint32_t address = GetNumericalArgument("address", arguments);
		FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
		FDataInfo* pDataInfo = codeAnalysis.GetDataInfoForAddress(addrRef);
		if (pDataInfo)
		{
			nlohmann::json result;

			// reads
			nlohmann::json reads = nlohmann::json::array();
			auto& readRefs = pDataInfo->Reads.GetReferences();
			for (auto& read : readRefs)
			{
				reads.push_back(read.Address);
			}
			result["read_instructions"] = reads;

			// writes
			nlohmann::json writes = nlohmann::json::array();
			auto& writeRefs = pDataInfo->Writes.GetReferences();
			for (auto& write : writeRefs)
			{
				writes.push_back(write.Address);
			}
			result["write_instructions"] = writes;
			result["last_writer"] = pDataInfo->LastWriter.Address;

			return result;
		}

		return { {"success", false}, {"error", "No data at address"} };

	}
};

// Tool to get address of label
class FGetLabelAddressTool : public FMCPTool
{
public:
	FGetLabelAddressTool()
	{
		Description = "Get the address of a label by name";
		InputSchema = {
		{"type", "object"},
		{"properties", {
			{"label_name", {
				{"type", "string"},
				{"description", "Name of the label to get address for"}
			}}
		}},
		{"required", {"label_name"}}
		};
	}
	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		if(arguments.contains("label_name") == false)
			return { {"success", false}, {"error", "No 'label_name' field supplied"} };

		std::string labelName = arguments["label_name"].get<std::string>();

		FAddressRef address;
		const FLabelInfo* pLabelInfo = codeAnalysis.FindLabel(labelName.c_str(), address);
		if (pLabelInfo)
		{
			nlohmann::json result;
			result["address"] = address.Address;
			return result;
		}
		return { {"success", false}, {"error", "Label not found"} };
	}
};


void RegisterBaseTools(FMCPToolsRegistry& registry)
{
	registry.RegisterTool("get_function_list", new FGetFunctionListTool());
	registry.RegisterTool("get_function_info", new FGetFunctionInfoTool());
	registry.RegisterTool("get_function_disassembly", new FGetFunctionDisassemblyTool());
	registry.RegisterTool("disassemble_address_range", new FDisassembleAddressRangeTool());
	registry.RegisterTool("read_memory", new FReadMemoryTool());
	registry.RegisterTool("go_to_address", new FGoToAddressTool());
	//registry.RegisterTool("add_comment", new FAddCommentTool());
	registry.RegisterTool("get_code_info", new FGetCodeInfoTool());
	registry.RegisterTool("get_data_info", new FGetDataInfoTool());
	registry.RegisterTool("get_label_address", new FGetLabelAddressTool());
}