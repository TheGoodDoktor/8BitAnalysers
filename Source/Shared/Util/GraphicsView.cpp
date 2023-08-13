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

void DisplayTextureInspector(const ImTextureID texture, float width, float height, bool bMagnifier = true);

// speccy colour CLUT
static const uint32_t g_SpeccyColourLUT[8] =
{
	0xFF000000,     // 0 - black
	0xFFFF0000,     // 1 - blue
	0xFF0000FF,     // 2 - red
	0xFFFF00FF,     // 3 - magenta
	0xFF00FF00,     // 4 - green
	0xFFFFFF00,     // 5 - cyan
	0xFF00FFFF,     // 6 - yellow
	0xFFFFFFFF,     // 7 - white
};

static const uint32_t* g_ColourLUT = g_SpeccyColourLUT;


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

void FGraphicsView::DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars,  uint32_t inkCol, uint32_t paperCol)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));

	for (int y = 0; y < heightChars * 8; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? inkCol : paperCol;
				if (col != 0xFF000000)
					*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += Width;
	}
}

void FGraphicsView::DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint32_t inkCol, uint32_t paperCol)
{
	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			DrawBitImage(pSrc, xp + (x * 8), yp + (y * 8), 1, 1, inkCol, paperCol);
			pSrc+=8;
		}
	}
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

// This function assumes the data is mapped in memory
void UpdateCharacterSetImage(FCodeAnalysisState& state, FCharacterSet& characterSet)
{
	uint16_t addr = characterSet.Params.Address.Address;

	// TODO: these are speccy specific, put in config
	const uint8_t brightMask = 1 << 6;
	const uint8_t inkMask = 7;
	const uint8_t inkShift = 0;
	const uint8_t paperMask = 7;
	const uint8_t paperShift = 3;

	characterSet.Image->Clear(0);	// clear first

	for (int charNo = 0; charNo < 256; charNo++)
	{
		const uint8_t* pCharData = state.CPUInterface->GetMemPtr(addr);
		const int xp = (charNo & 15) * 8;
		const int yp = (charNo >> 4) * 8;
		uint32_t inkCol = 0xffffffff;
		uint32_t paperCol = 0;
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
			inkCol = GetColFromAttr((colAttr >> inkShift) & inkMask, characterSet.Params.ColourLUT, bBright);
			paperCol = GetColFromAttr((colAttr >> paperShift) & paperMask, characterSet.Params.ColourLUT, bBright);
		}

		characterSet.Image->DrawBitImage(charPix, xp, yp, 1, 1, inkCol, paperCol);
	}

	characterSet.Image->UpdateTexture();
}

void UpdateCharacterSet(FCodeAnalysisState& state, FCharacterSet& characterSet, const FCharSetCreateParams& params)
{
	characterSet.Params = params;
	//characterSet.Params.Address = params.Address;
	//characterSet.Params.AttribsAddress = params.AttribsAddress;
	//characterSet.Params.MaskInfo = params.MaskInfo;
	//characterSet.Params.ColourInfo = params.ColourInfo;
	//characterSet.Params.bDynamic = params.bDynamic;

	UpdateCharacterSetImage(state, characterSet);
}

bool CreateCharacterSetAt(FCodeAnalysisState& state, const FCharSetCreateParams& params)
{
	if (params.Address.IsValid() == false || GetCharacterSetFromAddress(params.Address) != nullptr)
		return false;

	FCharacterSet* pNewCharSet = new FCharacterSet;
	pNewCharSet->Image = new FGraphicsView(128, 128);
	UpdateCharacterSet(state, *pNewCharSet, params);

	g_CharacterSets.push_back(pNewCharSet);
	return true;
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

	FLabelInfo* pLabel = state.GetLabelForAddress(params.Address);
	if (pLabel == nullptr)
		AddLabelAtAddress(state, params.Address);	

	FCharacterMap* pNewCharMap = new FCharacterMap;
	pNewCharMap->Params = params;

	g_CharacterMaps.push_back(pNewCharMap);
	return true;
}