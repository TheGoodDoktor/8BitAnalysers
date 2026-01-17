#include "MCPResources.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/AssemblerExport.h"

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

void RegisterBaseResources(FMCPResourceRegistry& registry)
{
	registry.RegisterResource(new FDisassemblyResource());
}