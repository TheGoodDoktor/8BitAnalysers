#pragma once

#include <cstdint>
#include "CodeAnalyser/CodeAnalyserTypes.h"

class FCodeAnalysisState;

// this is a surface on which to draw game graphics
class FGraphicsView
{
public:
	FGraphicsView(int width, int height);
	~FGraphicsView();

	void Clear(const uint32_t col = 0xff000000);
	void UpdateTexture(void);
	void Draw(float xSize, float ySize, bool bMagnifier = true);
	void Draw(bool bMagnifier = true);

	void DrawCharLine(uint8_t charLine, int xp, int yp, uint32_t inkCol, uint32_t paperCol);

	// Draw image from a bitmap
	// Size is given in (8x8) chars
	void DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint32_t inkCol, uint32_t paperCol);

	// Draw image from a bitmap
	// Size is given in (8x8) chars
	// image is arranged chat by char
	void DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint32_t inkCol, uint32_t paperCol);

	uint32_t* GetPixelBuffer() { return PixelBuffer; }
	const uint32_t* GetPixelBuffer() const { return PixelBuffer; }

	int GetWidth() const { return Width; }
	int GetHeight() const { return Height; }
	const void* GetTexture() const { return Texture; }

	bool SavePNG(const char* pFName);

private:
	int				Width = 0;
	int				Height = 0;
	uint32_t*		PixelBuffer = nullptr;
	void*			Texture = nullptr;
};

// Character set stuff

// order is important as it gets saved as an integer
enum class EMaskInfo
{
	None,
	InterleavedBytesPM,
	InterleavedBytesMP,

	Max
};

// order is important as it gets saved as an integer
// if this enum changes then you need to change g_ColourInfoTxt in CharacterMapViewer.cpp
enum class EColourInfo
{
	None,
	InterleavedPost,
	MemoryLUT,
	InterleavedPre,

	Max
};
struct FCharUVS
{
	FCharUVS(float u, float v, float size) :U0(u), V0(v), U1(u + size), V1(v + size) {}
	float U0, V0, U1, V1;
};

struct FCharSetCreateParams
{
	FAddressRef		Address;
	FAddressRef		AttribsAddress;
	EMaskInfo		MaskInfo = EMaskInfo::None;
	EColourInfo		ColourInfo = EColourInfo::None;
	const uint32_t* ColourLUT = nullptr;

	bool			bDynamic = false;
};

struct FCharacterSet
{
	~FCharacterSet() { delete Image; }
	FCharUVS GetCharacterUVS(uint8_t charNo) const
	{
		const int xp = (charNo & 15) * 8;
		const int yp = (charNo >> 4) * 8;

		return FCharUVS((float)xp * (1.0f / 128.0f), (float)yp * (1.0f / 128.0f), 8.0f / 128.0f);
	}

	FCharSetCreateParams	Params;

	FGraphicsView*	Image = nullptr;	
};

// Character Maps
struct FCharMapCreateParams
{
	FAddressRef	Address;
	int			Width = 0;
	int			Height = 0;
	FAddressRef	CharacterSet;
	uint8_t		IgnoreCharacter = 0;
};

struct FCharacterMap
{
	FCharMapCreateParams	Params;
};

// Palette
struct FPalette
{
	FPalette();
	void SetColourCount(int count);
	void SetColour(int colourIndex, uint32_t rgb);
	size_t GetColourCount() const;
	uint32_t GetColour(int colourIndex) const;
protected:
	std::vector<uint32_t> Colours;
};

FPalette& GetCurrentPalette();
const FPalette& GetCurrentPalette_Const();

// utils
uint32_t GetColFromAttr(uint8_t colBits, const uint32_t* colourLUT, bool bBright = true);

// Character sets
void InitCharacterSets();
void UpdateCharacterSets(FCodeAnalysisState& state);
int GetNoCharacterSets();
void DeleteCharacterSet(int index);
FCharacterSet* GetCharacterSetFromIndex(int index);
FCharacterSet* GetCharacterSetFromAddress(FAddressRef address);
void UpdateCharacterSet(FCodeAnalysisState& state, FCharacterSet& characterSet, const FCharSetCreateParams& params);
bool CreateCharacterSetAt(FCodeAnalysisState& state, const FCharSetCreateParams& params);

// Character Maps
int GetNoCharacterMaps();
void DeleteCharacterMap(int index);
FCharacterMap* GetCharacterMapFromIndex(int index);
FCharacterMap* GetCharacterMapFromAddress(FAddressRef address);
bool CreateCharacterMap(FCodeAnalysisState& state, const FCharMapCreateParams& params);

