#pragma once

#include <cstdint>
#include <vector>

struct FCodeAnalysisState;

// this is a surface on which to draw game graphics
class FGraphicsView
{
public:
	FGraphicsView(int width, int height);
	~FGraphicsView();

	void Clear(const uint32_t col = 0xff000000);
	void UpdateTexture(void);
	void Draw(float xSize, float ySize, bool bScale = false, bool bMagnifier = true);
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
private:
	int				Width = 0;
	int				Height = 0;
	uint32_t*		PixelBuffer = nullptr;
	void*			Texture = nullptr;
};

// Character set stuff

enum class EMaskInfo
{
	None,
	InterleavedBytesPM,
	InterleavedBytesMP,
};

enum class EColourInfo
{
	None,
	Interleaved,
	MemoryLUT
};
struct FCharUVS
{
	FCharUVS(float u, float v, float size) :U0(u), V0(v), U1(u + size), V1(v + size) {}
	float U0, V0, U1, V1;
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

	uint16_t		Address = 0;
	uint16_t		AttribAddress = 0;
	EMaskInfo		MaskInfo = EMaskInfo::None;
	EColourInfo		ColourInfo = EColourInfo::None;
	FGraphicsView*	Image = nullptr;
};

struct FCharSetCreateParams
{
	uint16_t	Address = 0;
	uint16_t	AttribsAddress = 0;
	EMaskInfo	MaskInfo = EMaskInfo::None;
	EColourInfo	ColourInfo = EColourInfo::None;
};

// utils
uint32_t GetColFromAttr(uint8_t colBits, bool bBright);

// Character sets
void InitCharacterSets();
const std::vector<FCharacterSet *>& GetCharacterSets();
FCharacterSet* GetCharacterSet(uint16_t address);
void UpdateCharacterSet(FCodeAnalysisState& state, FCharacterSet& characterSet, const FCharSetCreateParams& params);
void CreateCharacterSetAt(FCodeAnalysisState& state, const FCharSetCreateParams& params);
void DrawMaskInfoComboBox(EMaskInfo* pValue);
void DrawColourInfoComboBox(EColourInfo* pValue);
void DrawCharacterSetComboBox(FCodeAnalysisState& state, uint16_t* pAddr);