#pragma once

#include "IORegisterAnalysis.h"

#include <vector>

class FCodeAnalysisState;
struct FCodeAnalysisPage;
class FC64Emulator;
class FGraphicsView;

enum class EC64Event;

enum class EVicRegister
{
	Sprite0_X = 0,
	Sprite0_Y,
	Sprite1_X,
	Sprite1_Y,
	Sprite2_X,
	Sprite2_Y,
	Sprite3_X,
	Sprite3_Y,
	Sprite4_X,
	Sprite4_Y,
	Sprite5_X,
	Sprite5_Y,
	Sprite6_X,
	Sprite6_Y,
	Sprite7_X,
	Sprite7_Y,
	SpritesXMSB,
	ScreenControl1,
	RasterLine,
	LightPenX,
	LightPenY,
	SpriteEnable,
	ScreenControl2,
	SpriteDoubleHeight,
	MemorySetup,
	InterruptStatus,
	InterruptControl,
	SpritePriority,
	SpriteMultiColour,
	SpriteDoubleWidth,
	SpriteSpriteCollision,
	SpriteBackCollision,
	BorderColour,
	BackgroundColour,
	ExtraBackColour1,
	ExtraBackColour2,
	ExtraBackColour3,
	SpriteExtraColour1,
	SpriteExtraColour2,
	Sprite0_Colour,
	Sprite1_Colour,
	Sprite2_Colour,
	Sprite3_Colour,
	Sprite4_Colour,
	Sprite5_Colour,
	Sprite6_Colour,
	Sprite7_Colour,
};

struct FSpriteDef
{
	FAddressRef	Address;
	int			PaletteNo = -1;
	FGraphicsView*	SpriteImage;
	bool		bMultiColour = false;
};

struct FCharSetDef
{
	FAddressRef	Address;
	int			PaletteNo = -1;
	bool		bMultiColour = false;
};

struct FSpriteInfo
{
	int		ScanlineNo = -1;
	int		SpriteNo = -1;
	int		ImageNo = -1;
	int		XPosition = -1;
	int		YPosition = -1;
	FAddressRef	CodeAddress;
};

class FVICAnalysis : public FC64IODevice
{
public:
	void	Init(FC64Emulator* pEmulator);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void) override;
	void	OnMachineFrameStart(void) override;
	void	OnMachineFrameEnd(void) override;

	void	DrawScreenOverlay(float x,float y) const;

	const std::vector<FSpriteDef>& GetFoundSprites() const { return SpriteDefs; }
	const std::vector<FCharSetDef>&	GetFoundCharSets() const { return CharSets; }

private:
	EC64Event	GetVICEvent(uint8_t reg, uint8_t val, FAddressRef pc);
	void	DrawVICRegisterInfo(void);
	void	DrawLastFrameSpriteInfo(void);
	int		GetFrameSprite(int scanLine, int spriteNo);
private:
	static const int kScanlineMax = 320;
	static const int kNoRegisters = 64;
	FC64IORegisterInfo	VICRegisters[kNoRegisters];

	int		SelectedRegister = -1;

	std::vector<FSpriteDef>		SpriteDefs;
	std::vector<FCharSetDef>	CharSets;

	std::vector<FSpriteInfo>	FrameSprites;
	std::vector<FSpriteInfo>	LastFrameSprites;

};

void AddVICRegisterLabels(FCodeAnalysisPage& IOPage);