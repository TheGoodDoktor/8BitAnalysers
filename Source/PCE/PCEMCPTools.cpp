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
// Shared helper: read bytes from a FCodeAnalysisBank into a JSON result.
static nlohmann::json ReadBankMemory(const FCodeAnalysisBank* pBank, uint32_t offset, uint32_t length)
{
	const uint32_t bankSize = pBank->GetSizeBytes();
	if (offset >= bankSize)
		return { {"error", "offset out of range for this bank"} };
	const uint32_t clampedLength = (offset + length > bankSize) ? (bankSize - offset) : length;
	nlohmann::json bytes = nlohmann::json::array();
	for (uint32_t i = 0; i < clampedLength; i++)
		bytes.push_back(pBank->Memory[offset + i]);
	nlohmann::json result;
	result["bank"]   = pBank->Name;
	result["offset"] = offset;
	result["length"] = clampedLength;
	result["data"]   = bytes;
	return result;
}

// -----------------------------------------------------------------------
// read_bank_by_name
// Reads bytes from a named bank regardless of whether it is currently mapped.
// -----------------------------------------------------------------------
class FReadBankByNameTool : public FMCPTool
{
public:
	FReadBankByNameTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description = "Reads bytes from a named code analysis bank (e.g. 'ROM_43') regardless of whether it is currently mapped into the CPU address space.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"bank_name", {
					{"type", "string"},
					{"description", "Name of the bank as shown in the analyser (e.g. 'ROM_00', 'ROM_43', 'RAM')"}
				}},
				{"offset", {
					{"type", "integer"},
					{"description", "Byte offset within the bank"}
				}},
				{"length", {
					{"type", "integer"},
					{"description", "Number of bytes to read"}
				}}
			}},
			{"required", {"bank_name", "offset", "length"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("bank_name"))
			return { {"error", "Missing required argument: bank_name"} };
		if (!arguments.contains("offset"))
			return { {"error", "Missing required argument: offset"} };
		if (!arguments.contains("length"))
			return { {"error", "Missing required argument: length"} };

		const std::string bankName = arguments["bank_name"].get<std::string>();
		const uint32_t offset = GetNumericalArgument("offset", arguments);
		const uint32_t length = GetNumericalArgument("length", arguments);

		FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
		for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
		{
			const FCodeAnalysisBank* pBank = state.GetBank((int16_t)b);
			if (pBank && pBank->Memory && pBank->Name == bankName)
				return ReadBankMemory(pBank, offset, length);
		}
		return { {"error", "Bank not found: " + bankName} };
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------
// read_bank_by_mpr
// Reads bytes from a bank identified by its MPR bank number (0x00-0xFF).
// Works regardless of whether the bank is currently mapped.
// -----------------------------------------------------------------------
class FReadBankByMprTool : public FMCPTool
{
public:
	FReadBankByMprTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description = "Reads bytes from a bank identified by its MPR bank number (0x00-0xFF). Works regardless of whether the bank is currently mapped. ROM banks are 0x00-0x7F, work RAM is 0xF8, hardware page is 0xFF.";
		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"mpr_bank", {
					{"type", "integer"},
					{"description", "MPR bank number (0x00-0xFF). ROM banks are 0x00-0x7F, RAM is 0xF8."}
				}},
				{"offset", {
					{"type", "integer"},
					{"description", "Byte offset within the 8KB bank"}
				}},
				{"length", {
					{"type", "integer"},
					{"description", "Number of bytes to read"}
				}}
			}},
			{"required", {"mpr_bank", "offset", "length"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("mpr_bank"))
			return { {"error", "Missing required argument: mpr_bank"} };
		if (!arguments.contains("offset"))
			return { {"error", "Missing required argument: offset"} };
		if (!arguments.contains("length"))
			return { {"error", "Missing required argument: length"} };

		const uint32_t mprBank = GetNumericalArgument("mpr_bank", arguments);
		const uint32_t offset  = GetNumericalArgument("offset",   arguments);
		const uint32_t length  = GetNumericalArgument("length",   arguments);

		if (mprBank >= FPCEEmu::kNumBanks)
			return { {"error", "mpr_bank out of range (0x00-0xFF)"} };

		const FBankSet* pBankSet = pPCEEmu->Banks[mprBank];
		if (!pBankSet)
			return { {"error", "No bank set for this MPR bank number"} };

		const int16_t bankId = pBankSet->GetBankId(0);
		const FCodeAnalysisBank* pBank = pPCEEmu->GetCodeAnalysis().GetBank(bankId);
		if (!pBank || !pBank->Memory)
			return { {"error", "Bank has no memory"} };

		return ReadBankMemory(pBank, offset, length);
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------

void RegisterPCEMCPTools(FPCEEmu* pPCEEmu)
{
	AddMCPTool("read_vram",          new FReadVRAMTool(pPCEEmu));
	AddMCPTool("read_bank_by_name",  new FReadBankByNameTool(pPCEEmu));
	AddMCPTool("read_bank_by_mpr",   new FReadBankByMprTool(pPCEEmu));
}
