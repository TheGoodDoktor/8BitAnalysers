#include "MCPResources.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include <sstream>
#include <iomanip>
#include <set>
#include "CodeAnalyser/CodeAnalysisPage.h"
#include "CodeAnalyser/CodeAnalyserTypes.h"

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

		ExportAssembler(pEmulator, &outStr, 0x0000, 0x5FFF);	// just cover the code area of the ROM
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

class FFunctionIndexResource : public FMCPResource
{
public:
	FFunctionIndexResource()
	{
		URI = "arcadez80://function-index";
		Title = "Function Index";
		Description = "Compact listing of all named functions with addresses and descriptions. Read this first for an overview before diving into individual functions.";
		MimeType = "text/plain";
		Category = "code";
	}

	std::string Read(FEmuBase* pEmulator) override
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
		std::ostringstream out;

		out << "# Function Index\n";
		out << "# Format: address  name  [description]\n\n";

		for (const auto& funcIt : state.pFunctions->GetFunctions())
		{
			const FFunctionInfo& func = funcIt.second;
			FLabelInfo* pLabel = state.GetLabelForAddress(func.StartAddress);
			const std::string name = pLabel ? pLabel->GetName() : "<unnamed>";

			out << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
				<< func.StartAddress.Address << "  " << name;

			if (!func.Description.empty())
				out << "  ; " << func.Description;

			out << "\n";
		}

		return out.str();
	}
};

class FLabelsResource : public FMCPResource
{
public:
	FLabelsResource()
	{
		URI = "arcadez80://labels";
		Title = "Symbol Table";
		Description = "All named labels (data, code, function, text) with addresses and types. Use this to find named RAM variables, I/O ports, and data structures without reading the full disassembly.";
		MimeType = "text/plain";
		Category = "code";
	}

	std::string Read(FEmuBase* pEmulator) override
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
		std::ostringstream out;

		out << "# Symbol Table\n";
		out << "# Format: $address  type  name  [; comment]\n\n";

		for (const auto& bank : state.GetBanks())
		{
			if (bank.bHidden || !bank.IsMapped() || bank.PrimaryMappedPage < 0)
				continue;

			const uint16_t bankBaseAddr = bank.GetMappedAddress();

			for (int pageIdx = 0; pageIdx < bank.NoPages; pageIdx++)
			{
				const FCodeAnalysisPage& page = bank.Pages[pageIdx];
				for (int offset = 0; offset < FCodeAnalysisPage::kPageSize; offset++)
				{
					const FLabelInfo* pLabel = page.Labels[offset];
					if (!pLabel)
						continue;

					const uint16_t addr = bankBaseAddr + (uint16_t)(pageIdx * FCodeAnalysisPage::kPageSize + offset);

					const char* typeStr = "data";
					switch (pLabel->LabelType)
					{
					case ELabelType::Function:	typeStr = "function"; break;
					case ELabelType::Code:		typeStr = "code";     break;
					case ELabelType::Text:		typeStr = "text";     break;
					default: break;
					}

					out << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
						<< addr << "  " << typeStr << "  " << pLabel->GetName();

					if (!pLabel->Comment.empty())
						out << "  ; " << pLabel->Comment;

					out << "\n";
				}
			}
		}

		return out.str();
	}
};

class FCallGraphResource : public FMCPResource
{
public:
	FCallGraphResource()
	{
		URI = "arcadez80://call-graph";
		Title = "Function Call Graph";
		Description = "For each function, lists the functions it calls (outgoing edges). Use this to understand call relationships without repeated get_function_info calls.";
		MimeType = "text/plain";
		Category = "code";
	}

	std::string Read(FEmuBase* pEmulator) override
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
		std::ostringstream out;

		out << "# Function Call Graph\n";
		out << "# Format: $addr  CallerName -> Callee1, Callee2, ...\n\n";

		for (const auto& funcIt : state.pFunctions->GetFunctions())
		{
			const FFunctionInfo& func = funcIt.second;
			if (func.CallPoints.empty())
				continue;

			FLabelInfo* pCallerLabel = state.GetLabelForAddress(func.StartAddress);
			const std::string callerName = pCallerLabel ? pCallerLabel->GetName() : "<unnamed>";

			out << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
				<< func.StartAddress.Address << "  " << callerName << " ->";

			// Deduplicate callees by address
			std::set<uint16_t> seen;
			bool first = true;
			for (const auto& cp : func.CallPoints)
			{
				if (!seen.insert(cp.FunctionAddr.Address).second)
					continue;

				FLabelInfo* pCalleeLabel = state.GetLabelForAddress(cp.FunctionAddr);
				std::string calleeName;
				if (pCalleeLabel)
				{
					calleeName = pCalleeLabel->GetName();
				}
				else
				{
					std::ostringstream addrStr;
					addrStr << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << cp.FunctionAddr.Address;
					calleeName = addrStr.str();
				}

				out << (first ? " " : ", ") << calleeName;
				first = false;
			}

			out << "\n";
		}

		return out.str();
	}
};

void RegisterBaseResources(FMCPResourceRegistry& registry)
{
	registry.RegisterResource(new FFunctionIndexResource());
	registry.RegisterResource(new FLabelsResource());
	registry.RegisterResource(new FCallGraphResource());
	registry.RegisterResource(new FDisassemblyResource());
}