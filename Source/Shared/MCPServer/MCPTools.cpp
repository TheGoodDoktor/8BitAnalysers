#include "MCPTools.h"
#include "MCPManager.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CodeAnalyser/Z80/Z80Disassembler.h"
#include "CodeAnalyser/6502/M6502Disassembler.h"
#include "CodeAnalyser/6502/M65C02Disassembler.h"
#include <sstream>
#include <algorithm>

// Global storage for memory snapshots (shared between snapshot_memory and compare_memory tools)
namespace {
	int g_SnapshotCounter = 0;
	std::map<int, std::vector<uint8_t>> g_Snapshots;
	std::map<int, uint32_t> g_SnapshotStarts;
	std::map<int, std::string> g_SnapshotNames;
}

static void SaveSuggestions(FEmuBase* pEmu)
{
	if (pEmu->GetProjectConfig() != nullptr)
		g_MCPManager->GetSuggestionQueue().Save(pEmu->GetGameWorkspaceRoot() + "Suggestions.json");
}

class FGetFunctionListTool : public FMCPTool
{
public:
	FGetFunctionListTool()
	{
		Description = "Gets a list of all functions in the code analysis";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
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
		if (!arguments.contains("function_name"))
			return { {"error", "Missing required argument: function_name"} };

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
		if (!arguments.contains("function_name"))
			return { {"error", "Missing required argument: function_name"} };

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
		if (!arguments.contains("start_address"))
			return { {"error", "Missing required argument: start_address"} };
		if (!arguments.contains("end_address"))
			return { {"error", "Missing required argument: end_address"} };

		const uint32_t startAddress = GetNumericalArgument("start_address", arguments);
		const uint32_t endAddress = GetNumericalArgument("end_address", arguments);

		if (startAddress > endAddress)
			return { {"error", "start_address must be <= end_address"} };

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


	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("address"))
			return { {"error", "Missing required argument: address"} };
		if (!arguments.contains("length"))
			return { {"error", "Missing required argument: length"} };

		const uint32_t address = GetNumericalArgument("address", arguments);
		const uint32_t length = GetNumericalArgument("length", arguments);

		// Clamp to valid 16-bit address space
		const uint32_t clampedLength = (address + length > 0x10000) ? (0x10000 - address) : length;

		std::vector<uint8_t> data;
		data.reserve(clampedLength);
		for (uint32_t i = 0; i < clampedLength; ++i)
			data.push_back(pEmu->ReadByte(address + i));

		nlohmann::json result;
		result["address"] = address;
		result["length"] = clampedLength;
		result["data"] = data;
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
		if (!arguments.contains("address"))
			return { {"error", "Missing required argument: address"} };

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		const uint32_t address = GetNumericalArgument("address", arguments);

		FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
		codeAnalysis.GetFocussedViewState().GoToAddress(addrRef);
		return { {"success", true} };
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
				// Re-disassemble on the fly if Text wasn't populated during analysis
				if (pCodeInfo->Text.empty())
				{
					switch (codeAnalysis.CPUInterface->CPUType)
					{
					case ECPUType::Z80:    Z80DisassembleCodeInfoItem(address, codeAnalysis, pCodeInfo);    break;
					case ECPUType::M6502:  M6502DisassembleCodeInfoItem(address, codeAnalysis, pCodeInfo);  break;
					case ECPUType::M65C02: M65C02DisassembleCodeInfoItem(address, codeAnalysis, pCodeInfo); break;
					default: break;
					}
				}

				// output code info as json
				nlohmann::json result;
				Markup::SetCodeInfo(pCodeInfo);
				result["disassembly"] = Markup::ExpandString(codeAnalysis, pCodeInfo->Text.c_str());
				Markup::SetCodeInfo(nullptr);
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


// ---- Write-back tools: queue suggestions for user review ----

class FRenameFunctionTool : public FMCPTool
{
public:
	FRenameFunctionTool()
	{
		Description = "Suggest renaming a function. The rename will be queued for user review before being applied.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"function_name", {
					{"type", "string"},
					{"description", "Current name of the function to rename"}
				}},
				{"new_name", {
					{"type", "string"},
					{"description", "Proposed new name for the function"}
				}},
				{"rationale", {
					{"type", "string"},
					{"description", "Reasoning for the rename suggestion"}
				}}
			}},
			{"required", {"function_name", "new_name"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("function_name"))
			return { {"error", "Missing required argument: function_name"} };
		if (!arguments.contains("new_name"))
			return { {"error", "Missing required argument: new_name"} };

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		const std::string functionName = arguments["function_name"].get<std::string>();
		const std::string newName = arguments["new_name"].get<std::string>();

		const FFunctionInfo* pFuncInfo = codeAnalysis.pFunctions->FindFunctionByName(functionName.c_str());
		if (!pFuncInfo)
			return { {"error", "Function not found: " + functionName} };

		FMCPSuggestion suggestion;
		suggestion.Type = EMCPSuggestionType::RenameFunction;
		suggestion.AddressRef = pFuncInfo->StartAddress;
		suggestion.TargetName = functionName;
		suggestion.OldValue = functionName;
		suggestion.NewValue = newName;
		suggestion.Rationale = arguments.contains("rationale") ? arguments["rationale"].get<std::string>() : "";

		g_MCPManager->GetSuggestionQueue().Add(suggestion);
		SaveSuggestions(pEmu);
		return { {"status", "queued"}, {"message", "Rename suggestion queued for user review"} };
	}
};

class FSetFunctionDescriptionTool : public FMCPTool
{
public:
	FSetFunctionDescriptionTool()
	{
		Description = "Suggest a description for a function. The change will be queued for user review before being applied.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"function_name", {
					{"type", "string"},
					{"description", "Name of the function to describe"}
				}},
				{"description", {
					{"type", "string"},
					{"description", "Proposed description of what the function does"}
				}},
				{"rationale", {
					{"type", "string"},
					{"description", "Reasoning behind the description"}
				}}
			}},
			{"required", {"function_name", "description"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("function_name"))
			return { {"error", "Missing required argument: function_name"} };
		if (!arguments.contains("description"))
			return { {"error", "Missing required argument: description"} };

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		const std::string functionName = arguments["function_name"].get<std::string>();
		const std::string newDescription = arguments["description"].get<std::string>();

		const FFunctionInfo* pFuncInfo = codeAnalysis.pFunctions->FindFunctionByName(functionName.c_str());
		if (!pFuncInfo)
			return { {"error", "Function not found: " + functionName} };

		FMCPSuggestion suggestion;
		suggestion.Type = EMCPSuggestionType::SetFunctionDescription;
		suggestion.AddressRef = pFuncInfo->StartAddress;
		suggestion.TargetName = functionName;
		suggestion.OldValue = pFuncInfo->Description;
		suggestion.NewValue = newDescription;
		suggestion.Rationale = arguments.contains("rationale") ? arguments["rationale"].get<std::string>() : "";

		g_MCPManager->GetSuggestionQueue().Add(suggestion);
		SaveSuggestions(pEmu);
		return { {"status", "queued"}, {"message", "Description suggestion queued for user review"} };
	}
};

class FSetLabelTool : public FMCPTool
{
public:
	FSetLabelTool()
	{
		Description = "Suggest setting or renaming the label at a specific address. The change will be queued for user review before being applied.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Memory address to set the label at"}
				}},
				{"label_name", {
					{"type", "string"},
					{"description", "Proposed label name"}
				}},
				{"rationale", {
					{"type", "string"},
					{"description", "Reasoning for the label name"}
				}}
			}},
			{"required", {"address", "label_name"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("address"))
			return { {"error", "Missing required argument: address"} };
		if (!arguments.contains("label_name"))
			return { {"error", "Missing required argument: label_name"} };

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		const uint32_t address = GetNumericalArgument("address", arguments);
		const std::string labelName = arguments["label_name"].get<std::string>();

		const FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
		const FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(addrRef);

		FMCPSuggestion suggestion;
		suggestion.Type = EMCPSuggestionType::SetLabel;
		suggestion.AddressRef = addrRef;
		suggestion.TargetName = pLabel ? pLabel->GetName() : "(none)";
		suggestion.OldValue = pLabel ? pLabel->GetName() : "";
		suggestion.NewValue = labelName;
		suggestion.Rationale = arguments.contains("rationale") ? arguments["rationale"].get<std::string>() : "";

		g_MCPManager->GetSuggestionQueue().Add(suggestion);
		SaveSuggestions(pEmu);
		return { {"status", "queued"}, {"message", "Label suggestion queued for user review"} };
	}
};

class FAddCommentTool : public FMCPTool
{
public:
	FAddCommentTool()
	{
		Description = "Suggest adding a comment to the instruction at a specific address. The change will be queued for user review before being applied.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Memory address of the instruction to comment"}
				}},
				{"comment", {
					{"type", "string"},
					{"description", "The comment text to add"}
				}},
				{"rationale", {
					{"type", "string"},
					{"description", "Reasoning for the comment"}
				}}
			}},
			{"required", {"address", "comment"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("address"))
			return { {"error", "Missing required argument: address"} };
		if (!arguments.contains("comment"))
			return { {"error", "Missing required argument: comment"} };

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		const uint32_t address = GetNumericalArgument("address", arguments);
		const std::string comment = arguments["comment"].get<std::string>();

		const FAddressRef addrRef = codeAnalysis.AddressRefFromPhysicalAddress(address);
		const FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(addrRef);

		FMCPSuggestion suggestion;
		suggestion.Type = EMCPSuggestionType::AddComment;
		suggestion.AddressRef = addrRef;
		suggestion.TargetName = pCodeInfo ? pCodeInfo->Text : "";
		suggestion.OldValue = pCodeInfo ? pCodeInfo->Comment : "";
		suggestion.NewValue = comment;
		suggestion.Rationale = arguments.contains("rationale") ? arguments["rationale"].get<std::string>() : "";

		g_MCPManager->GetSuggestionQueue().Add(suggestion);
		SaveSuggestions(pEmu);
		return { {"status", "queued"}, {"message", "Comment suggestion queued for user review"} };
	}
};

// ============================================================================
// Execution Control Tools
// ============================================================================

class FPauseEmulatorTool : public FMCPTool
{
public:
	FPauseEmulatorTool()
	{
		Description = "Pause the emulator so its state can be inspected. Returns the current PC and stopped status. Must be called before stepping or reading registers for accurate results.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FDebugger& debugger = pEmu->GetCodeAnalysis().Debugger;
		debugger.Break();

		uint16_t pcVal = 0;
		debugger.GetRegisterWordValue("PC", pcVal);
		char pcStr[8];
		snprintf(pcStr, sizeof(pcStr), "$%04X", pcVal);

		return {
			{"stopped", debugger.IsStopped()},
			{"pc", pcStr}
		};
	}
};

class FResumeEmulatorTool : public FMCPTool
{
public:
	FResumeEmulatorTool()
	{
		Description = "Resume emulator execution from its current paused state.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FDebugger& debugger = pEmu->GetCodeAnalysis().Debugger;
		debugger.Continue();
		return { {"stopped", debugger.IsStopped()} };
	}
};

class FStepIntoTool : public FMCPTool
{
public:
	FStepIntoTool()
	{
		Description = "Step the emulator into the next single instruction (follows CALL into subroutines). The emulator must be paused first via pause_emulator. After this returns, the step executes on the next emulator tick — call get_registers to read the updated state.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FDebugger& debugger = pEmu->GetCodeAnalysis().Debugger;
		if (!debugger.IsStopped())
			return { {"error", "Emulator must be paused before stepping. Call pause_emulator first."} };

		debugger.StepInto();
		return { {"status", "stepping"} };
	}
};

class FStepOverTool : public FMCPTool
{
public:
	FStepOverTool()
	{
		Description = "Step the emulator over the next instruction, treating CALL as a single step (runs called subroutine to completion). The emulator must be paused first. Call get_registers to read the updated state after it pauses again.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FDebugger& debugger = pEmu->GetCodeAnalysis().Debugger;
		if (!debugger.IsStopped())
			return { {"error", "Emulator must be paused before stepping. Call pause_emulator first."} };

		debugger.StepOver();
		return { {"status", "stepping"} };
	}
};

class FStepFrameTool : public FMCPTool
{
public:
	FStepFrameTool()
	{
		Description = "Run the emulator for one full machine frame then pause. Useful for advancing game state by one frame at a time. Call get_registers to read state once the emulator pauses again.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		pEmu->GetCodeAnalysis().Debugger.StepFrame();
		return { {"status", "stepping"} };
	}
};

// ============================================================================
// Inspection Tools
// ============================================================================

class FGetRegistersTool : public FMCPTool
{
public:
	FGetRegistersTool()
	{
		Description = "Get the current CPU register values and emulator stopped/running status. Pause the emulator first for a stable snapshot.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		FDebugger& debugger = codeAnalysis.Debugger;

		static const char* kByteRegs[] = { "A", "F", "B", "C", "D", "E", "H", "L", "R", "I" };
		static const char* kWordRegs[] = { "AF", "BC", "DE", "HL", "IX", "IY", "SP", "PC" };

		nlohmann::json regs;
		char valStr[8];
		uint8_t byteVal;
		uint16_t wordVal;

		for (const char* reg : kByteRegs)
		{
			if (debugger.GetRegisterByteValue(reg, byteVal))
			{
				snprintf(valStr, sizeof(valStr), "$%02X", byteVal);
				regs[reg] = valStr;
			}
		}
		for (const char* reg : kWordRegs)
		{
			if (debugger.GetRegisterWordValue(reg, wordVal))
			{
				snprintf(valStr, sizeof(valStr), "$%04X", wordVal);
				regs[reg] = valStr;
			}
		}

		return {
			{"stopped",   debugger.IsStopped()},
			{"frame",     codeAnalysis.CurrentFrameNo},
			{"registers", regs}
		};
	}
};

static const char* GetLabelTypeStr(ELabelType type)
{
	switch (type)
	{
	case ELabelType::Function: return "function";
	case ELabelType::Code:     return "code";
	case ELabelType::Text:     return "text";
	default:                   return "data";
	}
}

class FReadMemoryAnnotatedTool : public FMCPTool
{
public:
	FReadMemoryAnnotatedTool()
	{
		Description =
			"Read a range of memory and return an annotated hex dump. "
			"Any known labels, types, or comments at addresses in the range are shown inline. "
			"Use this instead of read_memory when you want to understand what the data means, not just its raw bytes.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Starting address (decimal or 0x hex)"}
				}},
				{"length", {
					{"type", "integer"},
					{"description", "Number of bytes to read (max 512)"}
				}}
			}},
			{"required", {"address", "length"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("address"))
			return { {"error", "Missing required argument: address"} };
		if (!arguments.contains("length"))
			return { {"error", "Missing required argument: length"} };

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();

		const uint32_t startAddr  = GetNumericalArgument("address", arguments);
		const uint32_t rawLength  = GetNumericalArgument("length",  arguments);
		const uint32_t length     = (std::min)(rawLength, 512u);
		const uint32_t endAddr    = (std::min)(startAddr + length, 0x10000u);
		const uint32_t actualLen  = endAddr - startAddr;

		static const int kRowBytes = 16;
		std::ostringstream out;

		for (uint32_t rowBase = startAddr; rowBase < endAddr; rowBase += kRowBytes)
		{
			const uint32_t rowEnd = (std::min)(rowBase + (uint32_t)kRowBytes, endAddr);

			// Emit any labels that fall within this row
			for (uint32_t a = rowBase; a < rowEnd; a++)
			{
				const FAddressRef ref = codeAnalysis.AddressRefFromPhysicalAddress((uint16_t)a);
				const FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(ref);
				if (pLabel)
				{
					out << "; " << pLabel->GetName()
					    << "  [" << GetLabelTypeStr(pLabel->LabelType) << "]";
					if (!pLabel->Comment.empty())
						out << "  ; " << pLabel->Comment;
					out << "\n";
				}
			}

			// Hex bytes
			char buf[8];
			snprintf(buf, sizeof(buf), "$%04X: ", (uint16_t)rowBase);
			out << buf;

			for (uint32_t a = rowBase; a < rowBase + kRowBytes; a++)
			{
				if (a < rowEnd)
				{
					snprintf(buf, sizeof(buf), "%02X ", pEmu->ReadByte((uint16_t)a));
					out << buf;
				}
				else
				{
					out << "   ";
				}
			}

			// ASCII
			out << " ";
			for (uint32_t a = rowBase; a < rowEnd; a++)
			{
				const uint8_t byte = pEmu->ReadByte((uint16_t)a);
				out << (char)(byte >= 0x20 && byte < 0x7F ? byte : '.');
			}
			out << "\n";
		}

		return {
			{"address",  startAddr},
			{"length",   actualLen},
			{"hex_dump", out.str()}
		};
	}
};

class FGetFrameTraceTool : public FMCPTool
{
public:
	FGetFrameTraceTool()
	{
		Description =
			"Returns a summary of every function executed during the last emulated frame, "
			"in the order they were first entered, with per-function instruction hit counts. "
			"Useful for building a dynamic call graph, identifying hot functions, and finding "
			"indirect calls (JP HL, RST dispatch) that static analysis misses. "
			"Run step_frame first to capture a representative frame.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		const std::vector<FAddressRef>& trace = codeAnalysis.Debugger.GetFrameTrace();

		if (trace.empty())
			return { {"error", "Frame trace is empty. Call step_frame first to capture a frame."} };

		// Walk the trace, mapping each PC to its containing function.
		// Track: insertion order, total hits per function start address.
		std::vector<FAddressRef>          order;      // function start addresses, first-seen order
		std::map<FAddressRef, int>         hitCount;   // start address → hit count
		std::map<FAddressRef, int>         firstIndex; // start address → order index

		for (const FAddressRef& pc : trace)
		{
			FFunctionInfo* pFunc = codeAnalysis.pFunctions->FindFunction(pc);
			FAddressRef key = pFunc ? pFunc->StartAddress : pc;

			auto it = hitCount.find(key);
			if (it == hitCount.end())
			{
				firstIndex[key] = (int)order.size();
				order.push_back(key);
				hitCount[key] = 1;
			}
			else
			{
				it->second++;
			}
		}

		// Build output array in first-seen order
		nlohmann::json functions = nlohmann::json::array();
		char addrStr[8];
		for (const FAddressRef& startAddr : order)
		{
			FFunctionInfo* pFunc = codeAnalysis.pFunctions->FindFunction(startAddr);
			const FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(startAddr);
			const std::string name = pLabel ? pLabel->GetName() : "";

			snprintf(addrStr, sizeof(addrStr), "$%04X", startAddr.Address);

			nlohmann::json entry;
			entry["address"]   = addrStr;
			entry["hit_count"] = hitCount[startAddr];
			if (!name.empty())
				entry["name"] = name;
			if (pFunc && !pFunc->Description.empty())
				entry["description"] = pFunc->Description;

			functions.push_back(entry);
		}

		return {
			{"total_instructions", (int)trace.size()},
			{"unique_addresses",   (int)order.size()},
			{"functions",          functions}
		};
	}
};

// Parse a pattern from a JSON value: integer → single byte; string → space- or run-separated hex bytes
static std::vector<uint8_t> ParseSearchPattern(const nlohmann::json& arg)
{
	if (arg.is_number())
		return { (uint8_t)(arg.get<uint32_t>() & 0xFF) };

	std::string str = arg.get<std::string>();

	// Remove "0x"/"0X" prefix if present on the whole string
	if (str.size() >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		str = str.substr(2);

	// Split on spaces; if no spaces treat as run of hex digits
	std::vector<uint8_t> pattern;
	std::istringstream ss(str);
	std::string token;
	while (ss >> token)
	{
		try { pattern.push_back((uint8_t)std::stoul(token, nullptr, 16)); }
		catch (...) {}
	}

	// If still empty try parsing as a run of hex digit pairs
	if (pattern.empty() && str.size() >= 2 && str.size() % 2 == 0)
	{
		for (size_t i = 0; i + 1 < str.size(); i += 2)
		{
			try { pattern.push_back((uint8_t)std::stoul(str.substr(i, 2), nullptr, 16)); }
			catch (...) { pattern.clear(); break; }
		}
	}

	return pattern;
}

class FSearchMemoryTool : public FMCPTool
{
public:
	FSearchMemoryTool()
	{
		Description =
			"Search for a byte value or multi-byte pattern anywhere in the 16-bit address space. "
			"'pattern' can be a single integer (e.g. 255 or 0xFF) for a single byte, "
			"or a hex string for a sequence (e.g. \"FF A8 00\" or \"FFA800\"). "
			"Matches are annotated with any label at that address. "
			"Typical use: find where a known value is stored in RAM, or locate a data signature.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"pattern", {
					{"description", "Byte or byte sequence to find. Integer for single byte (e.g. 42), or hex string for multi-byte (e.g. \"FF A8 00\")"}
				}},
				{"start_address", {
					{"type", "integer"},
					{"description", "Start of search range (default 0)"}
				}},
				{"end_address", {
					{"type", "integer"},
					{"description", "End of search range inclusive (default 0xFFFF)"}
				}},
				{"max_results", {
					{"type", "integer"},
					{"description", "Maximum number of matches to return (default 32)"}
				}}
			}},
			{"required", {"pattern"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("pattern"))
			return { {"error", "Missing required argument: pattern"} };

		const std::vector<uint8_t> pattern = ParseSearchPattern(arguments["pattern"]);
		if (pattern.empty())
			return { {"error", "Could not parse pattern. Use an integer or a hex string like \"FF A8 00\"."} };

		const uint32_t startAddr  = arguments.contains("start_address") ? GetNumericalArgument("start_address", arguments) : 0u;
		const uint32_t endAddr    = arguments.contains("end_address")   ? GetNumericalArgument("end_address",   arguments) : 0xFFFFu;
		const uint32_t maxResults = arguments.contains("max_results")   ? GetNumericalArgument("max_results",   arguments) : 32u;

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();

		nlohmann::json matches = nlohmann::json::array();
		const size_t patLen = pattern.size();
		const uint32_t searchEnd = (endAddr + 1 >= patLen) ? (endAddr + 1 - (uint32_t)patLen) : 0;

		for (uint32_t addr = startAddr; addr <= searchEnd && (uint32_t)matches.size() < maxResults; addr++)
		{
			bool found = true;
			for (size_t i = 0; i < patLen && found; i++)
				found = (pEmu->ReadByte((uint16_t)(addr + i)) == pattern[i]);

			if (!found)
				continue;

			char addrStr[8];
			snprintf(addrStr, sizeof(addrStr), "$%04X", (uint16_t)addr);

			nlohmann::json match;
			match["address"] = addrStr;

			// Annotate with label if present
			const FAddressRef ref = codeAnalysis.AddressRefFromPhysicalAddress((uint16_t)addr);
			const FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(ref);
			if (pLabel)
				match["label"] = pLabel->GetName();

			// Show a few bytes of context around the match
			std::ostringstream ctx;
			const uint32_t ctxStart = (addr >= 2) ? addr - 2 : 0;
			const uint32_t ctxEnd   = (std::min)(addr + (uint32_t)patLen + 2u, 0x10000u);
			for (uint32_t c = ctxStart; c < ctxEnd; c++)
			{
				if (c == addr) ctx << "[";
				char byteStr[4];
				snprintf(byteStr, sizeof(byteStr), "%02X", pEmu->ReadByte((uint16_t)c));
				ctx << byteStr;
				if (c == addr + patLen - 1) ctx << "]";
				if (c + 1 < ctxEnd) ctx << " ";
			}
			match["context"] = ctx.str();

			matches.push_back(match);
		}

		// Format pattern for display
		std::ostringstream patStr;
		for (size_t i = 0; i < pattern.size(); i++)
		{
			if (i) patStr << " ";
			char b[4];
			snprintf(b, sizeof(b), "%02X", pattern[i]);
			patStr << b;
		}

		return {
			{"pattern",     patStr.str()},
			{"match_count", (int)matches.size()},
			{"matches",     matches}
		};
	}
};

class FSnapshotMemoryTool : public FMCPTool
{
public:
	FSnapshotMemoryTool()
	{
		Description = "Takes a snapshot of a memory range for later comparison. Returns a snapshot ID that can be used with compare_memory.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"start_address", {
					{"type", "integer"},
					{"description", "Starting address to snapshot (default: 0)"}
				}},
				{"end_address", {
					{"type", "integer"},
					{"description", "Ending address to snapshot inclusive (default: 0xFFFF)"}
				}},
				{"name", {
					{"type", "string"},
					{"description", "Optional name for this snapshot"}
				}}
			}},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		const uint32_t startAddr = GetNumericalArgument("start_address", arguments, 0);
		const uint32_t endAddr = GetNumericalArgument("end_address", arguments, 0xFFFF);
		const std::string name = arguments.contains("name") ? arguments["name"].get<std::string>() : "";

		if (startAddr > 0xFFFF || endAddr > 0xFFFF)
			return { {"error", "Address out of range (must be 0x0000-0xFFFF)"} };
		if (startAddr > endAddr)
			return { {"error", "start_address must be <= end_address"} };

		// Generate snapshot ID
		int snapshotId = ++g_SnapshotCounter;

		const uint32_t length = endAddr - startAddr + 1;
		std::vector<uint8_t> snapshot;
		snapshot.reserve(length);

		for (uint32_t addr = startAddr; addr <= endAddr; ++addr)
			snapshot.push_back(pEmu->ReadByte(static_cast<uint16_t>(addr)));

		g_Snapshots[snapshotId] = std::move(snapshot);
		g_SnapshotStarts[snapshotId] = startAddr;
		g_SnapshotNames[snapshotId] = name;

		char startStr[8], endStr[8];
		snprintf(startStr, sizeof(startStr), "$%04X", static_cast<uint16_t>(startAddr));
		snprintf(endStr, sizeof(endStr), "$%04X", static_cast<uint16_t>(endAddr));

		return {
			{"snapshot_id", snapshotId},
			{"name", name},
			{"start_address", startStr},
			{"end_address", endStr},
			{"length", (int)length}
		};
	}
};

class FCompareMemoryTool : public FMCPTool
{
public:
	FCompareMemoryTool()
	{
		Description = "Compares current memory state to a previously taken snapshot and returns all changed addresses.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"snapshot_id", {
					{"type", "integer"},
					{"description", "Snapshot ID returned from snapshot_memory"}
				}},
				{"max_changes", {
					{"type", "integer"},
					{"description", "Maximum number of changes to return (default: 100)"}
				}},
				{"show_annotations", {
					{"type", "boolean"},
					{"description", "Include labels and comments for changed addresses (default: true)"}
				}}
			}},
			{"required", {"snapshot_id"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("snapshot_id"))
			return { {"error", "Missing required argument: snapshot_id"} };

		const int snapshotId = arguments["snapshot_id"].get<int>();
		const uint32_t maxChanges = GetNumericalArgument("max_changes", arguments, 100);
		const bool showAnnotations = arguments.contains("show_annotations") ? arguments["show_annotations"].get<bool>() : true;

		if (g_Snapshots.find(snapshotId) == g_Snapshots.end())
			return { {"error", "Invalid snapshot_id. Use snapshot_memory to create a snapshot first."} };

		const std::vector<uint8_t>& snapshot = g_Snapshots[snapshotId];
		const uint32_t startAddr = g_SnapshotStarts[snapshotId];
		const std::string& name = g_SnapshotNames[snapshotId];

		FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
		nlohmann::json changes = nlohmann::json::array();
		uint32_t totalChanges = 0;

		for (size_t i = 0; i < snapshot.size() && (uint32_t)changes.size() < maxChanges; ++i)
		{
			const uint16_t addr = static_cast<uint16_t>(startAddr + i);
			const uint8_t oldVal = snapshot[i];
			const uint8_t newVal = pEmu->ReadByte(addr);

			if (oldVal != newVal)
			{
				totalChanges++;

				char addrStr[8], oldStr[8], newStr[8];
				snprintf(addrStr, sizeof(addrStr), "$%04X", addr);
				snprintf(oldStr, sizeof(oldStr), "$%02X", oldVal);
				snprintf(newStr, sizeof(newStr), "$%02X", newVal);

				nlohmann::json change;
				change["address"] = addrStr;
				change["old_value"] = oldStr;
				change["new_value"] = newStr;
				change["decimal_old"] = oldVal;
				change["decimal_new"] = newVal;

				if (showAnnotations)
				{
					const FAddressRef ref = codeAnalysis.AddressRefFromPhysicalAddress(addr);
					const FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(ref);
					if (pLabel)
						change["label"] = pLabel->GetName();

					const FCommentBlock* pComment = codeAnalysis.GetCommentBlockForAddress(ref);
					if (pComment && !pComment->Comment.empty())
						change["comment"] = pComment->Comment;
				}

				changes.push_back(change);
			}
		}

		char startStr[8], endStr[8];
		snprintf(startStr, sizeof(startStr), "$%04X", static_cast<uint16_t>(startAddr));
		snprintf(endStr, sizeof(endStr), "$%04X", static_cast<uint16_t>(startAddr + snapshot.size() - 1));

		return {
			{"snapshot_id", snapshotId},
			{"snapshot_name", name},
			{"snapshot_range", std::string(startStr) + " - " + std::string(endStr)},
			{"total_changes", (int)totalChanges},
			{"changes_returned", (int)changes.size()},
			{"changes", changes}
		};
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
	registry.RegisterTool("get_code_info", new FGetCodeInfoTool());
	registry.RegisterTool("get_data_info", new FGetDataInfoTool());
	registry.RegisterTool("get_label_address", new FGetLabelAddressTool());

	// Write-back tools — queue suggestions for user review in the GUI
	registry.RegisterTool("rename_function", new FRenameFunctionTool());
	registry.RegisterTool("set_function_description", new FSetFunctionDescriptionTool());
	registry.RegisterTool("set_label", new FSetLabelTool());
	registry.RegisterTool("add_comment", new FAddCommentTool());

	// Execution control
	registry.RegisterTool("pause_emulator",  new FPauseEmulatorTool());
	registry.RegisterTool("resume_emulator", new FResumeEmulatorTool());
	registry.RegisterTool("step_into",       new FStepIntoTool());
	registry.RegisterTool("step_over",       new FStepOverTool());
	registry.RegisterTool("step_frame",      new FStepFrameTool());

	// Runtime inspection
	registry.RegisterTool("get_registers",          new FGetRegistersTool());
	registry.RegisterTool("read_memory_annotated",  new FReadMemoryAnnotatedTool());
	registry.RegisterTool("get_frame_trace",        new FGetFrameTraceTool());
	registry.RegisterTool("search_memory",          new FSearchMemoryTool());
	
	// Memory analysis
	registry.RegisterTool("snapshot_memory",        new FSnapshotMemoryTool());
	registry.RegisterTool("compare_memory",         new FCompareMemoryTool());
}

