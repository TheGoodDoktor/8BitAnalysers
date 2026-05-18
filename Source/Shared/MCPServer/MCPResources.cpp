#include "MCPResources.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include <sstream>
#include <iomanip>
#include <set>
#include <vector>
#include "CodeAnalyser/CodeAnalysisPage.h"
#include "CodeAnalyser/CodeAnalyserTypes.h"
#include "MCPServer.h"
#include "Util/GraphicsView.h"
#include "stb/stb_image_write.h"

class FDisassemblyResource : public FMCPResource
{
public:
	FDisassemblyResource()
	{
		URI = std::string(GetMCPServerName()) + "://disassembly";
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

		outStr = "// Assembler export not yet wired up for this platform";
		return outStr;
	}
};

class FFunctionIndexResource : public FMCPResource
{
public:
	FFunctionIndexResource()
	{
		URI = std::string(GetMCPServerName()) + "://function-index";
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
				<< func.StartAddress.GetAddress() << "  " << name;

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
		URI = std::string(GetMCPServerName()) + "://labels";
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

		for (int bi = 0; bi < FCodeAnalysisState::BankCount; bi++)
		{
			const FCodeAnalysisBank& bank = state.GetBanks()[bi];
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
		URI = std::string(GetMCPServerName()) + "://call-graph";
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
				<< func.StartAddress.GetAddress() << "  " << callerName << " ->";

			// Deduplicate callees by address
			std::set<uint16_t> seen;
			bool first = true;
			for (const auto& cp : func.CallPoints)
			{
				if (!seen.insert(cp.FunctionAddr.GetAddress()).second)
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
					addrStr << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << cp.FunctionAddr.GetAddress();
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


static std::string Base64Encode(const uint8_t* data, size_t length)
{
	static const char kTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string out;
	out.reserve(((length + 2) / 3) * 4);
	for (size_t i = 0; i < length; i += 3)
	{
		const uint32_t b0 = data[i];
		const uint32_t b1 = (i + 1 < length) ? data[i + 1] : 0;
		const uint32_t b2 = (i + 2 < length) ? data[i + 2] : 0;
		out += kTable[(b0 >> 2) & 0x3F];
		out += kTable[((b0 << 4) | (b1 >> 4)) & 0x3F];
		out += (i + 1 < length) ? kTable[((b1 << 2) | (b2 >> 6)) & 0x3F] : '=';
		out += (i + 2 < length) ? kTable[b2 & 0x3F] : '=';
	}
	return out;
}

class FScreenshotResource : public FMCPResource
{
public:
	FScreenshotResource()
	{
		URI = std::string(GetMCPServerName()) + "://screenshot";
		Title = "Screenshot";
		Description = "Current frame buffer as a PNG image. Use this to visually confirm the game state before diving into analysis — which era is active, what enemies are on screen, etc.";
		MimeType = "image/png";
		Category = "display";
	}

	std::string Read(FEmuBase* pEmulator) override
	{
		FGraphicsView* pScreen = pEmulator->GetScreen();
		if (!pScreen || !pScreen->GetPixelBuffer())
			return "";

		std::vector<uint8_t> pngData;
		stbi_write_png_to_func(
			[](void* context, void* data, int size)
			{
				auto* vec = static_cast<std::vector<uint8_t>*>(context);
				const uint8_t* bytes = static_cast<const uint8_t*>(data);
				vec->insert(vec->end(), bytes, bytes + size);
			},
			&pngData,
			pScreen->GetWidth(),
			pScreen->GetHeight(),
			4,
			pScreen->GetPixelBuffer(),
			pScreen->GetWidth() * (int)sizeof(uint32_t)
		);

		if (pngData.empty())
			return "";

		return Base64Encode(pngData.data(), pngData.size());
	}
};

void RegisterBaseResources(FMCPResourceRegistry& registry)
{
	registry.RegisterResource(new FFunctionIndexResource());
	registry.RegisterResource(new FLabelsResource());
	registry.RegisterResource(new FCallGraphResource());
	registry.RegisterResource(new FDisassemblyResource());
	registry.RegisterResource(new FScreenshotResource());
}