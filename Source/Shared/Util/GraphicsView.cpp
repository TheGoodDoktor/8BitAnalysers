#include "GraphicsView.h"
#include "../CodeAnalyser/CodeAnalyser.h"
#include <imgui.h>
#include <ImGuiSupport/ImGuiTexture.h>
#include <ImGuiSupport/ImGuiScaling.h>
#include <cstdint>
#include <vector>

// TODO: should probably have a separate file with all the STB impls in
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize2.h"

void DisplayTextureInspector(const ImTextureID texture, float width, float height, bool bMagnifier = true);

uint32_t GetColFromAttr(uint8_t colBits, const uint32_t* colourLUT, bool bBright )
{
	const uint32_t outCol = colourLUT[colBits];
	if (bBright == false)
		return outCol & 0xFFD7D7D7;
	else
		return outCol;
}

FGraphicsView::FGraphicsView(int width, int height)
	: Width(width)
	, Height(height)
{
	Width = width;
	Height = height;
	PixelBuffer = new uint32_t[width * height];
	Texture = ImGui_CreateTextureRGBA((uint8_t*)PixelBuffer, width, height);
}

FGraphicsView::~FGraphicsView()
{
	delete PixelBuffer;
	if(Texture != nullptr)
		ImGui_FreeTexture(Texture);
}

void FGraphicsView::Clear(const uint32_t col)
{
	for (int i = 0; i < Width * Height; i++)
		PixelBuffer[i] = col;
}

void FGraphicsView::Draw(float xSize, float ySize, bool bMagnifier)
{
	UpdateTexture();
	DisplayTextureInspector(Texture, xSize, ySize, bMagnifier);
}

void FGraphicsView::UpdateTexture(void)
{
	ImGui_UpdateTextureRGBA(Texture, (uint8_t*)PixelBuffer);
}

void FGraphicsView::Draw(bool bMagnifier)
{
	Draw((float)Width, (float)Height, bMagnifier);
}

void FGraphicsView::DrawCharLine(uint8_t charLine, int xp, int yp, uint32_t inkCol, uint32_t paperCol)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));

	for (int xpix = 0; xpix < 8; xpix++)
	{
		const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
		const uint32_t col = bSet ? inkCol : paperCol;
		if (col != 0xFF000000)
			*(pBase + xpix) = col;
	}
}

void FGraphicsView::Draw1BppImageAt(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, const uint32_t* cols, int stride)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));
	int widthChars = widthPixels / 8;
	assert((widthPixels & 7) == 0);	// we don't currently support sub character widths - maybe you should implement it?

	for (int y = 0; y < heightPixels; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint8_t charLine = *pSrc;
			pSrc+=stride;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? cols[1] : cols[0];
				if (col != 0xFF000000)
					*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += Width;
	}
}

void FGraphicsView::Draw1BppImageAtMask(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, const uint32_t* cols, int stride)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));
	int widthChars = widthPixels / 8;
	assert((widthPixels & 7) == 0);	// we don't currently support sub character widths - maybe you should implement it?

	for (int y = 0; y < heightPixels; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint8_t mask = *pSrc++;
			const uint8_t pixels = *pSrc;
			pSrc += stride;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bMasked = (mask & (1 << (7 - xpix))) != 0;
				const bool bSet = (pixels & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? cols[1] : cols[0];
				if (bMasked == false)
					*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += Width;
	}
}

void FGraphicsView::Draw2BppImageAt(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, const uint32_t* cols)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));
	const int bytesPerLine = widthPixels / 4;
	assert((widthPixels & 7) == 0);	// we don't currently support sub character widths - maybe you should implement it?
	
	for (int y = 0; y < heightPixels; y++)
	{
		for (int x = 0; x < bytesPerLine; x++)
		{
			const uint8_t val = *pSrc++;

			for (int xpix = 0; xpix < 4; xpix++)
			{
				uint8_t colNo = 0;

				switch (xpix)
				{
				case 0:
					colNo = (val & 0x8 ? 2 : 0) | (val & 0x80 ? 1 : 0);
					break;
				case 1:
					colNo = (val & 0x4 ? 2 : 0) | (val & 0x40 ? 1 : 0);
					break;
				case 2:
					colNo = (val & 0x2 ? 2 : 0) | (val & 0x20 ? 1 : 0);
					break;
				case 3:
					colNo = (val & 0x1 ? 2 : 0) | (val & 0x10 ? 1 : 0);
					break;
				}
				const uint32_t pixelCol = cols ? cols[colNo] : colNo == 0 ? 0 : 0xffffffff;
				*(pBase + xpix + (x * 4)) = pixelCol;
			}
		}
		pBase += Width;
	}
}

void FGraphicsView::Draw2BppWideImageAt(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, const uint32_t* cols)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));
	int widthChars = widthPixels / 8;
	assert((widthPixels & 7) == 0);	// we don't currently support sub character widths - maybe you should implement it?

	//*pBase = 0;
	for (int y = 0; y < heightPixels; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 4; xpix++)
			{
				const uint8_t colNo = (charLine >> (6 - (xpix * 2))) & 3;

				// 0 check for sprites?
				*(pBase + (xpix * 2) + (x * 8)) = cols[colNo];
				*(pBase + (xpix * 2) + 1 + (x * 8)) = cols[colNo];
			}
		}

		pBase += Width;
	}
}

void FGraphicsView::Draw4BppWideImageAt(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, const uint32_t* cols)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));
	const int bytesPerLine = widthPixels / 2;
	assert((widthPixels & 1) == 0);	// we don't currently support sub character widths - maybe you should implement it?

	for (int y = 0; y < heightPixels; y++)
	{
		for (int x = 0; x < bytesPerLine; x++)
		{
			const uint8_t val = *pSrc++;

			for (int xpix = 0; xpix < 2; xpix++)
			{
				uint8_t colNo = 0;

				if (xpix == 0)
					colNo = (val & 0x80 ? 1 : 0) | (val & 0x8 ? 2 : 0) | (val & 0x20 ? 4 : 0) | (val & 0x2 ? 8 : 0);
				else 
					colNo = (val & 0x40 ? 1 : 0) | (val & 0x4 ? 2 : 0) | (val & 0x10 ? 4 : 0) | (val & 0x1 ? 8 : 0);

				*(pBase + (xpix*2) + (x * 4)) = cols[colNo];
				*(pBase + (xpix*2) + 1 + (x * 4)) = cols[colNo];
			}
		}
		pBase += Width;
	}
}

void FGraphicsView::Draw1BppImageFromCharsAt(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, const uint32_t* cols)
{
	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			Draw1BppImageAt(pSrc, xp + (x * 8), yp + (y * 8), 8, 8, cols);
			pSrc+=8;
		}
	}
}

void FGraphicsView::DrawOtherGraphicsViewScaled(const FGraphicsView* pView, int xp, int yp, int xsize, int ysize)
{
    if(xp < 0 || yp < 0 || xp + xsize >= Width || yp + ysize >= Height) // no clipping - just reject
        return;
    
    uint8_t* destAddress = (uint8_t *)(PixelBuffer + xp + (yp * Width * 4));
	stbir_resize_uint8_linear((uint8_t *)pView->PixelBuffer , pView->Width , pView->Height , pView->Width * 4,
                       destAddress, xsize, ysize, Width * 4, (stbir_pixel_layout)4);
}

bool FGraphicsView::SavePNG(const char* pFName)
{
	// TODO: we might need to flip the bytes

	stbi_write_png(pFName, Width, Height, 4, PixelBuffer, Width * sizeof(uint32_t));

	return true;
}


void DisplayTextureInspector(const ImTextureID texture, float width, float height, bool bMagnifier)
{
	const float imgScale = ImGui_GetScaling();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size(width * imgScale, height * imgScale);

	ImGui::Image(texture, size);

	if (bMagnifier && ImGui::IsItemHovered())
	{
		ImGuiIO& io = ImGui::GetIO();
		const float my_tex_w = size.x;
		const float my_tex_h = size.y;

		ImGui::BeginTooltip();
		const float region_sz = 64.0f;
		float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
		if (region_x < 0.0f)
			region_x = 0.0f;
		else if (region_x > my_tex_w - region_sz)
			region_x = my_tex_w - region_sz;

		float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
		if (region_y < 0.0f)
			region_y = 0.0f;
		else if (region_y > my_tex_h - region_sz)
			region_y = my_tex_h - region_sz;

		const float zoom = 4.0f;

		//ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
		//ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
		ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
		ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
		ImGui::Image(texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));


		ImGui::EndTooltip();
	}
}

// Character sets

static std::vector<FCharacterSet*>	g_CharacterSets;
static std::vector<FCharacterMap*>	g_CharacterMaps;

void UpdateCharacterSetImage(FCodeAnalysisState& state, FCharacterSet& characterSet);


void InitCharacterSets()
{
	// char sets
	for (auto& it : g_CharacterSets)
		delete it;

	g_CharacterSets.clear();

	// char maps
	for (auto& it : g_CharacterMaps)
		delete it;

	g_CharacterMaps.clear();
}

void UpdateCharacterSets(FCodeAnalysisState& state)
{
	for (auto& it : g_CharacterSets)
	{
		if(it->Params.bDynamic)
			UpdateCharacterSetImage(state, *it);
	}
}

int GetNoCharacterSets()
{
	return (int)g_CharacterSets.size();
}

void DeleteCharacterSet(int index)
{
	g_CharacterSets.erase(g_CharacterSets.begin() + index);
}

FCharacterSet* GetCharacterSetFromIndex(int index)
{
	if (index >= 0 && index < GetNoCharacterSets())
		return g_CharacterSets[index];
	else
		return nullptr;
}

FCharacterSet* GetCharacterSetFromAddress(FAddressRef address)
{
	for (auto& it : g_CharacterSets)
	{
		if (it->Params.Address == address)
			return it;
	}

	return nullptr;
}

void DrawCharacterSetImage2BppCPC(FCodeAnalysisState& state, FCharacterSet& characterSet, uint16_t addr)
{
	for (int charNo = 0; charNo < 256; charNo++)
	{
		const uint8_t* pCharData = state.CPUInterface->GetMemPtr(addr);
		const int xp = (charNo & 15) * 8;
		const int yp = (charNo >> 4) * 8;

		const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(characterSet.Params.PaletteNo);
		characterSet.Image->Draw2BppImageAt(pCharData, xp, yp, 8, 8, pPaletteColours);
		addr += 16; // 2bpp * 8
	}
}

void DrawCharacterSetImageMultiColourC64(FCodeAnalysisState& state, FCharacterSet& characterSet, uint16_t addr)
{
	for (int charNo = 0; charNo < 256; charNo++)
	{
		const uint8_t* pCharData = state.CPUInterface->GetMemPtr(addr);
		const int xp = (charNo & 15) * 8;
		const int yp = (charNo >> 4) * 8;

		const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(characterSet.Params.PaletteNo);
		characterSet.Image->Draw2BppWideImageAt(pCharData, xp, yp, 8, 8, pPaletteColours);
		addr += 8; // half res pixels
	}
}

void DrawCharacterSetImage1Bpp(FCodeAnalysisState& state, FCharacterSet& characterSet, uint16_t addr)
{
	// TODO: these are speccy specific, put in config
	const uint8_t brightMask = 1 << 6;
	const uint8_t inkMask = 7;
	const uint8_t inkShift = 0;
	const uint8_t paperMask = 7;
	const uint8_t paperShift = 3;

	for (int charNo = 0; charNo < 256; charNo++)
	{
		const uint8_t* pCharData = state.CPUInterface->GetMemPtr(addr);
		const int xp = (charNo & 15) * 8;
		const int yp = (charNo >> 4) * 8;
		uint32_t cols[2] = { 0,0xffffffff };
		uint8_t colAttr = 0xff;
		uint8_t charPix[8];
		uint8_t charMask[8];

		if (characterSet.Params.ColourInfo == EColourInfo::InterleavedPre)
			colAttr = state.ReadByte(addr++);

		for (int i = 0; i < 8; i++)
		{
			if (characterSet.Params.MaskInfo == EMaskInfo::InterleavedBytesMP)
				charMask[i] = state.ReadByte(addr++);
			charPix[i] = state.ReadByte(addr++);
			if (characterSet.Params.MaskInfo == EMaskInfo::InterleavedBytesPM)
				charMask[i] = state.ReadByte(addr++);
		}

		// Get colour from colour info
		switch (characterSet.Params.ColourInfo)
		{
            case EColourInfo::MemoryLUT:
                colAttr = state.ReadByte(characterSet.Params.AttribsAddress.Address + charNo);
                break;
            case EColourInfo::InterleavedPost:
                colAttr = state.ReadByte(addr++);
                break;
            default:
                break;
		}

		if (colAttr != 0xff)
		{
			// get ink & paper
			const bool bBright = !!(colAttr & brightMask);
			cols[0] = GetColFromAttr((colAttr >> paperShift) & paperMask, characterSet.Params.ColourLUT, bBright);
			cols[1] = GetColFromAttr((colAttr >> inkShift) & inkMask, characterSet.Params.ColourLUT, bBright);
		}

		characterSet.Image->Draw1BppImageAt(charPix, xp, yp, 8, 8, cols);
	}
}

// This function assumes the data is mapped in memory
void UpdateCharacterSetImage(FCodeAnalysisState& state, FCharacterSet& characterSet)
{
	const uint16_t addr = characterSet.Params.Address.Address;

	characterSet.Image->Clear(0);	// clear first

	switch (characterSet.Params.BitmapFormat)
	{
	case EBitmapFormat::Bitmap_1Bpp:
		DrawCharacterSetImage1Bpp(state, characterSet, addr);
		break;
	case EBitmapFormat::ColMap2Bpp_CPC:
		DrawCharacterSetImage2BppCPC(state, characterSet, addr);
		break;
	case EBitmapFormat::ColMapMulticolour_C64:
		DrawCharacterSetImageMultiColourC64(state, characterSet, addr);
		break;
    default:
        break;
	}

	characterSet.Image->UpdateTexture();
}

void UpdateCharacterSet(FCodeAnalysisState& state, FCharacterSet& characterSet, const FCharSetCreateParams& params)
{
	characterSet.Params = params;

	UpdateCharacterSetImage(state, characterSet);
}

bool CreateCharacterSetAt(FCodeAnalysisState& state, const FCharSetCreateParams& params)
{
	if (params.Address.IsValid() == false || GetCharacterSetFromAddress(params.Address) != nullptr)
		return false;

	FCharacterSet* pNewCharSet = new FCharacterSet;
	pNewCharSet->Image = new FGraphicsView(128, 128);
	UpdateCharacterSet(state, *pNewCharSet, params);

	// Char set needs to have a label
	if (state.GetLabelForAddress(params.Address) == nullptr)
	{
		FLabelInfo* pLabel = AddLabelAtAddress(state,params.Address);
		const int kLabelSize = 32;
		char label[kLabelSize] = { 0 };
		snprintf(label, kLabelSize, "charset_%04X", params.Address.Address);
		pLabel->ChangeName(label);
	}

	g_CharacterSets.push_back(pNewCharSet);
	return true;
}

void FixupCharacterSetAddressRefs(FCodeAnalysisState& state)
{
	for (int i = 0; i < GetNoCharacterSets(); i++)
	{
		FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
		FixupAddressRef(state, pCharSet->Params.Address);
	}
}

// Character Maps



int GetNoCharacterMaps()
{
	return (int)g_CharacterMaps.size();
}

void DeleteCharacterMap(int index)
{
	g_CharacterMaps.erase(g_CharacterMaps.begin() + index);
}

bool DeleteCharacterMap(FAddressRef address)
{
	for (auto it = g_CharacterMaps.begin(); it != g_CharacterMaps.end(); ++it)
	{
		if ((*it)->Params.Address == address)
		{
			g_CharacterMaps.erase(it);
			return true;
		}
	}

	return false;
}

FCharacterMap* GetCharacterMapFromIndex(int index)
{
	if (index >= 0 && index < GetNoCharacterMaps())
		return g_CharacterMaps[index];
	else
		return nullptr;
}

FCharacterMap* GetCharacterMapFromAddress(FAddressRef address)
{
	for (auto& it : g_CharacterMaps)
	{
		if (it->Params.Address == address)
			return it;
	}

	return nullptr;
}

bool CreateCharacterMap(FCodeAnalysisState& state, const FCharMapCreateParams& params)
{
	if (params.Address.IsValid() == false || GetCharacterMapFromAddress(params.Address) != nullptr)
		return false;

	if(params.bAddLabel)
	{ 
		FLabelInfo* pLabel = state.GetLabelForAddress(params.Address);
		if (pLabel == nullptr)
			AddLabelAtAddress(state, params.Address);	
	}
	FCharacterMap* pNewCharMap = new FCharacterMap;
	pNewCharMap->Params = params;

	g_CharacterMaps.push_back(pNewCharMap);
	return true;
}

void FixupCharacterMapAddressRefs(FCodeAnalysisState& state)
{
	for (int i = 0; i < GetNoCharacterMaps(); i++)
	{
		FCharacterMap* pCharMap = GetCharacterMapFromIndex(i);
		FixupAddressRef(state, pCharMap->Params.Address);
		FixupAddressRef(state, pCharMap->Params.CharacterSet);
	}
}

// New Palette stuff - just to annoy Sam!
// Palette Store - move?

std::vector<uint32_t>	g_PaletteColours;
std::vector<FPaletteEntry>	g_Palettes;

int	GetPaletteNo(const uint32_t* palette, int noCols)
{
	auto& paletteStore = g_PaletteColours;
	auto& paletteEntries = g_Palettes;

	// find out if palette is already there
	for (int paletteNo = 0; paletteNo < paletteEntries.size(); paletteNo++)
	{
		FPaletteEntry& paletteEntry = paletteEntries[paletteNo];
		if (paletteEntry.NoColours == noCols)
		{
			bool bFound = true;

			for (int colNo = 0; colNo < noCols; colNo++)
			{
				if (paletteStore[paletteEntry.FirstColourIndex + colNo] != palette[colNo])
				{
					bFound = false;
					break;
				}
			}

			if(bFound == true)
				return paletteNo;
		}
	}

	// add new palette
	FPaletteEntry newEntry;
	newEntry.FirstColourIndex = (int)paletteStore.size();
	for (int colNo = 0; colNo < noCols; colNo++)
		paletteStore.push_back(palette[colNo]);
	newEntry.NoColours = noCols;
	const int newPaletteNo = (int)g_Palettes.size();
	g_Palettes.push_back(newEntry);

	return newPaletteNo;
}

uint32_t* GetPaletteFromPaletteNo(int index)
{
	if (index == -1)
		return nullptr;

	auto& paletteStore = g_PaletteColours;
	auto& paletteEntries = g_Palettes;

	if(index < paletteEntries.size())
		return paletteStore.data() + paletteEntries[index].FirstColourIndex;

	return nullptr;
}

int GetNoPaletteEntries(void)
{
	return (int)g_Palettes.size();
}

const FPaletteEntry* GetPaletteEntry(int paletteNo)
{
	if(paletteNo >= 0 && paletteNo < g_Palettes.size())
		return &g_Palettes[paletteNo];

	return nullptr;
}

#include <json.hpp>

const char* kPaletteColours = "PaletteColours";
const char* kPaletteEntries = "PaletteEntries";

void SavePalettesToJson(nlohmann::json& jsonDoc)
{
	for (const auto& paletteEntry : g_PaletteColours)
	{
		jsonDoc[kPaletteColours].push_back(paletteEntry);
	}

	for (const auto& palette : g_Palettes)
	{
		nlohmann::json jsonPalette;
		jsonPalette["FirstColourIndex"] = palette.FirstColourIndex;
		jsonPalette["NoColours"] = palette.NoColours;
		jsonDoc[kPaletteEntries].push_back(jsonPalette);
	}
}

void LoadPalettesFromJson(const nlohmann::json& jsonDoc)
{
	g_PaletteColours.clear();
	g_Palettes.clear();

	if (jsonDoc.contains(kPaletteColours))	// new
	{
		for (const auto& paletteColour : jsonDoc[kPaletteColours])
			g_PaletteColours.push_back(paletteColour);
	}

	if (jsonDoc.contains(kPaletteEntries))	// new
	{
		for (const auto& jsonPalette : jsonDoc[kPaletteEntries])
		{
			FPaletteEntry palette;
			palette.FirstColourIndex = jsonPalette["FirstColourIndex"];
			palette.NoColours = jsonPalette["NoColours"];
			g_Palettes.push_back(palette);
		}
	}
}
