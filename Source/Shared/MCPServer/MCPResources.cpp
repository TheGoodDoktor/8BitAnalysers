#include "MCPResources.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include <sstream>
#include <iomanip>

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

void RegisterBaseResources(FMCPResourceRegistry& registry)
{
	registry.RegisterResource(new FFunctionIndexResource());
	registry.RegisterResource(new FDisassemblyResource());
}