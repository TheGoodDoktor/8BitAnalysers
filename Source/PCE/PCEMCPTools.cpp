#include "PCEMCPTools.h"
// MCP headers must come before PCEEmu.h: winsock2.h must be included before
// windows.h (pulled in by geargfx/common.h), otherwise the winsock/winsock2 conflict fires.
#include "MCPServer/MCPManager.h"
#include "MCPServer/MCPTools.h"
#include "PCEEmu.h"
#include <geargrafx_core.h>
#include "huc6270.h"

// -----------------------------------------------------------------------
// read_vram
// Reads 16-bit words from HuC6270 VRAM.
// Addresses and length are in words (16-bit units).
// VRAM is 0x8000 words (0x0000–0x7FFF).
// -----------------------------------------------------------------------
class FReadVRAMTool : public FMCPTool
{
public:
	FReadVRAMTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description =
			"Reads 16-bit words from HuC6270 VRAM. "
			"Addresses and length are in 16-bit words. "
			"VRAM is 0x8000 words total (word addresses 0x0000–0x7FFF). "
			"Sprite pattern data layout: each 16x16 block is 64 words — "
			"words 0-15 plane 0, 16-31 plane 1, 32-47 plane 2, 48-63 plane 3. "
			"For a 32-wide sprite, the right 16x16 block starts at word offset 64 from the left block. "
			"For a 32-tall sprite, the next row of blocks starts at word offset 128.";

		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"address", {
					{"type", "integer"},
					{"description", "Starting VRAM word address (0x0000–0x7FFF)"}
				}},
				{"length", {
					{"type", "integer"},
					{"description", "Number of 16-bit words to read (each VRAM word is 2 bytes)"}
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
		const uint32_t length  = GetNumericalArgument("length",  arguments);

		if (address >= HUC6270_VRAM_SIZE)
			return { {"error", "address out of VRAM range (0x0000–0x7FFF)"} };

		const uint32_t clampedLength = (address + length > HUC6270_VRAM_SIZE)
		                               ? (HUC6270_VRAM_SIZE - address)
		                               : length;

		const u16* pVRAM = pPCEEmu->GetCore()->GetHuC6270_1()->GetVRAM();

		nlohmann::json words = nlohmann::json::array();
		for (uint32_t i = 0; i < clampedLength; i++)
			words.push_back(pVRAM[address + i]);

		nlohmann::json result;
		result["address"] = address;
		result["length"]  = clampedLength;
		result["words"]   = words;
		return result;
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------

void RegisterPCEMCPTools(FPCEEmu* pPCEEmu)
{
	AddMCPTool("read_vram", new FReadVRAMTool(pPCEEmu));
}
