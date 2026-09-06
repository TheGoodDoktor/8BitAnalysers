#include "PCEMCPTools.h"
// MCP headers must come before PCEEmu.h: winsock2.h must be included before
// windows.h (pulled in by geargfx/common.h), otherwise the winsock/winsock2 conflict fires.
#include "MCPServer/MCPManager.h"
#include "MCPServer/MCPResources.h"
#include "MCPServer/MCPTools.h"
#include "PCEEmu.h"
#include <geargrafx_core.h>
#include "huc6270.h"
#include "Util/FileUtil.h"
#include "Util/GraphicsView.h"

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
		// todo improve description
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

		if (mprBank >= FPCEEmu::kNumHwBanks)
			return { {"error", "mpr_bank out of range (0x00-0xFF)"} };

		const FBankSet* pBankSet = pPCEEmu->GetBankSetPtr(mprBank);
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

class FResetEmulatorTool : public FMCPTool
{
public:
	FResetEmulatorTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description = "Resets the emulated PC Engine to its startup state, equivalent to pressing the Reset button. Useful to return to the title screen or to re-run game startup code from the entry point. Does not reset the code analysis state. Preserves labels, functions, comments etc.";
		InputSchema = {
			{"type", "object"},
			{"properties", nlohmann::json::object()},
			{"required", nlohmann::json::array()}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		pPCEEmu->SoftResetMachine();
		
		return { {"success", "true"} };
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------
// write_vram_to_binary_file
// Dumps the full HuC6270 VRAM (0x8000 16-bit words = 64KB) to a raw binary
// file on disk. Words are written little-endian.
// -----------------------------------------------------------------------
class FDumpVRAMTool : public FMCPTool
{
public:
	FDumpVRAMTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description =
			"Dumps the full HuC6270 VRAM (0x8000 16-bit words = 64KB) to a raw binary file on disk. "
			"Words are written little-endian. "
			"Useful for offline analysis: the output can be examined with a hex editor or tile viewer "
			"to inspect CHR tile graphics, the BAT tile map, and sprite attribute tables without "
			"issuing many individual read_vram calls.";

		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"file_path", {
					{"type", "string"},
					{"description", "Absolute path of the output file (e.g. C:/temp/vram.bin)"}
				}}
			}},
			{"required", {"file_path"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("file_path"))
			return { {"error", "Missing required argument: file_path"} };

		const std::string filePath = arguments["file_path"].get<std::string>();

		const u16* pVRAM = pPCEEmu->GetCore()->GetHuC6270_1()->GetVRAM();

		if (!SaveBinaryFile(filePath.c_str(), pVRAM, HUC6270_VRAM_SIZE * sizeof(u16)))
			return { {"error", "Failed to write VRAM to: " + filePath} };

		nlohmann::json result;
		result["file_path"]    = filePath;
		result["bytes_written"] = HUC6270_VRAM_SIZE * sizeof(u16);
		result["words_written"] = HUC6270_VRAM_SIZE;
		return result;
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------
// pce-analyser://vram
// Full HuC6270 VRAM (64KB) as a base64-encoded binary resource.
// -----------------------------------------------------------------------
class FVRAMResource : public FMCPResource
{
public:
	FVRAMResource(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		URI         = "pce-analyser://vram";
		Title       = "VRAM";
		Description = "Full HuC6270 VRAM contents (0x8000 16-bit words = 64KB) as raw binary. "
		              "Words are in little-endian order. "
		              "VRAM $0000–$0FFF typically contains the BAT (tile map). "
		              "Tile CHR data follows the BAT; tile N starts at word address N*16. "
		              "Use this instead of many read_vram calls when you need a broad view of VRAM layout.";
		MimeType    = "application/octet-stream";
		Category    = "graphics";
	}

	std::string Read(FEmuBase* pEmulator) override
	{
		const u16* pVRAM = pPCEEmu->GetCore()->GetHuC6270_1()->GetVRAM();
		return Base64Encode(reinterpret_cast<const uint8_t*>(pVRAM), HUC6270_VRAM_SIZE * sizeof(u16));
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------
// render_bg_tiles_to_png
// Renders PCE BG tiles (8x8, 4bpp planar) from VRAM to a PNG file.
// -----------------------------------------------------------------------
class FRenderBGTilesToPNGTool : public FMCPTool
{
public:
	FRenderBGTilesToPNGTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description =
			"Renders PCE background tiles from VRAM to a PNG file. "
			"Each BG tile is 8x8 pixels, 4bpp planar (32 bytes / 16 VRAM words). "
			"Tiles are laid out left-to-right, top-to-bottom in the output image. "
			"vram_address and tile_count are in VRAM words. "
			"palette_index selects one of the 32 VCE palettes (0-15 = BG palettes).";

		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"vram_address", {
					{"type", "integer"},
					{"description", "Starting VRAM word address of the first tile (must be 16-word aligned)"}
				}},
				{"tile_count", {
					{"type", "integer"},
					{"description", "Number of tiles to render"}
				}},
				{"width_in_tiles", {
					{"type", "integer"},
					{"description", "Number of tile columns in the output image"}
				}},
				{"palette_index", {
					{"type", "integer"},
					{"description", "VCE palette index 0-31 (0-15 = BG, 16-31 = sprite). Default 0."}
				}},
				{"file_path", {
					{"type", "string"},
					{"description", "Absolute path for the output PNG file"}
				}}
			}},
			{"required", {"vram_address", "tile_count", "width_in_tiles", "file_path"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("vram_address"))
			return { {"error", "Missing required argument: vram_address"} };
		if (!arguments.contains("tile_count"))
			return { {"error", "Missing required argument: tile_count"} };
		if (!arguments.contains("width_in_tiles"))
			return { {"error", "Missing required argument: width_in_tiles"} };
		if (!arguments.contains("file_path"))
			return { {"error", "Missing required argument: file_path"} };

		const uint32_t    vramAddress  = GetNumericalArgument("vram_address",   arguments);
		const uint32_t    tileCount    = GetNumericalArgument("tile_count",      arguments);
		const uint32_t    widthTiles   = GetNumericalArgument("width_in_tiles",  arguments);
		const uint32_t    paletteIndex = arguments.contains("palette_index") ? GetNumericalArgument("palette_index", arguments) : 0;
		const std::string filePath     = arguments["file_path"].get<std::string>();

		if (widthTiles == 0)
			return { {"error", "width_in_tiles must be greater than zero"} };
		if (tileCount == 0)
			return { {"error", "tile_count must be greater than zero"} };

		const uint32_t wordsPerTile = 16;
		if (vramAddress + tileCount * wordsPerTile > HUC6270_VRAM_SIZE)
			return { {"error", "vram_address + tile_count * 16 exceeds VRAM bounds"} };

		const uint32_t heightTiles = (tileCount + widthTiles - 1) / widthTiles;

		const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex);

		const u16* pVRAM = pPCEEmu->GetCore()->GetHuC6270_1()->GetVRAM();

		FGraphicsView view(widthTiles * 8, heightTiles * 8);
		view.Clear(0xff000000);
		view.Draw4bpp8x8PlanarBGTileImage(
			reinterpret_cast<const uint8_t*>(pVRAM + vramAddress),
			0, 0,
			(int)widthTiles, (int)heightTiles,
			pPalette);

		if (!view.SavePNG(filePath.c_str()))
			return { {"error", "Failed to write PNG to: " + filePath} };

		nlohmann::json result;
		result["file_path"]      = filePath;
		result["tile_count"]     = tileCount;
		result["width_in_tiles"] = widthTiles;
		result["height_in_tiles"] = heightTiles;
		result["image_width"]    = widthTiles * 8;
		result["image_height"]   = heightTiles * 8;
		return result;
	}

private:
	FPCEEmu* pPCEEmu;
};

// -----------------------------------------------------------------------
// render_sprite_tiles_to_png
// Renders PCE sprite tiles (16x16, 4bpp planar) from VRAM to a PNG file.
// -----------------------------------------------------------------------
class FRenderSpriteTilesToPNGTool : public FMCPTool
{
public:
	FRenderSpriteTilesToPNGTool(FPCEEmu* pEmu) : pPCEEmu(pEmu)
	{
		Description =
			"Renders PCE sprite tiles from VRAM to a PNG file. "
			"Each sprite tile block is 16x16 pixels, 4bpp planar (64 VRAM words): "
			"words 0-15 = plane 0, 16-31 = plane 1, 32-47 = plane 2, 48-63 = plane 3. "
			"The output image contains width_in_sprites * height_in_sprites blocks. "
			"palette_index selects one of the 32 VCE palettes (16-31 = sprite palettes).";

		InputSchema = {
			{"type", "object"},
			{"properties", {
				{"vram_address", {
					{"type", "integer"},
					{"description", "Starting VRAM word address of the first sprite tile block (must be 64-word aligned)"}
				}},
				{"width_in_sprites", {
					{"type", "integer"},
					{"description", "Number of sprite columns in the output image"}
				}},
				{"height_in_sprites", {
					{"type", "integer"},
					{"description", "Number of sprite rows in the output image"}
				}},
				{"palette_index", {
					{"type", "integer"},
					{"description", "VCE palette index 0-31 (16-31 = sprite palettes). Default 16."}
				}},
				{"file_path", {
					{"type", "string"},
					{"description", "Absolute path for the output PNG file"}
				}}
			}},
			{"required", {"vram_address", "width_in_sprites", "height_in_sprites", "file_path"}}
		};
	}

	nlohmann::json Execute(FEmuBase* pEmu, const nlohmann::json& arguments) override
	{
		if (!arguments.contains("vram_address"))
			return { {"error", "Missing required argument: vram_address"} };
		if (!arguments.contains("width_in_sprites"))
			return { {"error", "Missing required argument: width_in_sprites"} };
		if (!arguments.contains("height_in_sprites"))
			return { {"error", "Missing required argument: height_in_sprites"} };
		if (!arguments.contains("file_path"))
			return { {"error", "Missing required argument: file_path"} };

		const uint32_t    vramAddress     = GetNumericalArgument("vram_address",      arguments);
		const uint32_t    widthSprites    = GetNumericalArgument("width_in_sprites",  arguments);
		const uint32_t    heightSprites   = GetNumericalArgument("height_in_sprites", arguments);
		const uint32_t    paletteIndex    = arguments.contains("palette_index") ? GetNumericalArgument("palette_index", arguments) : 16;
		const std::string filePath        = arguments["file_path"].get<std::string>();

		if (widthSprites == 0 || heightSprites == 0)
			return { {"error", "width_in_sprites and height_in_sprites must be greater than zero"} };

		const uint32_t wordsPerSprite = 64;
		if (vramAddress + widthSprites * heightSprites * wordsPerSprite > HUC6270_VRAM_SIZE)
			return { {"error", "Requested sprite range exceeds VRAM bounds"} };

		const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex);

		const u16* pVRAM = pPCEEmu->GetCore()->GetHuC6270_1()->GetVRAM();

		FGraphicsView view(widthSprites * 16, heightSprites * 16);
		view.Clear(0xff000000);
		view.Draw4bpp16x16PlanarSpriteImage(
			reinterpret_cast<const uint8_t*>(pVRAM + vramAddress),
			0, 0,
			(int)widthSprites, (int)heightSprites,
			pPalette);

		if (!view.SavePNG(filePath.c_str()))
			return { {"error", "Failed to write PNG to: " + filePath} };

		nlohmann::json result;
		result["file_path"]         = filePath;
		result["width_in_sprites"]  = widthSprites;
		result["height_in_sprites"] = heightSprites;
		result["image_width"]       = widthSprites * 16;
		result["image_height"]      = heightSprites * 16;
		return result;
	}

private:
	FPCEEmu* pPCEEmu;
};

void RegisterPCEMCPTools(FPCEEmu* pPCEEmu)
{
	// Memory tools
	AddMCPTool("read_vram",                 new FReadVRAMTool(pPCEEmu));
	AddMCPTool("read_bank_by_name",         new FReadBankByNameTool(pPCEEmu));
	AddMCPTool("read_bank_by_mpr",          new FReadBankByMprTool(pPCEEmu));
	AddMCPTool("write_vram_to_binary_file", new FDumpVRAMTool(pPCEEmu));

	// Graphics tools
	AddMCPTool("render_bg_tiles_to_png",     new FRenderBGTilesToPNGTool(pPCEEmu));
	AddMCPTool("render_sprite_tiles_to_png", new FRenderSpriteTilesToPNGTool(pPCEEmu));

	// Resources
	AddMCPResource(new FVRAMResource(pPCEEmu));

	// Emulator control
	AddMCPTool("reset_emulator", new FResetEmulatorTool(pPCEEmu));
}
