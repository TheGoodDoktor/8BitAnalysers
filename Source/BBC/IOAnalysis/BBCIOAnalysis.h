#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>

#include "FredAnalysis.h"
#include "JIMAnalysis.h"
#include "SheilaAnalysis.h"
#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/IODevices/VIAIODevice.h>
#include <CodeAnalyser/IODevices/MC6845Device.h>

const uint8_t	kFredPage	= 0xFC;
const uint8_t	kJimPage	= 0xFD;
const uint8_t	kSheilaPage = 0xFE;

class FCodeAnalysisState;
class FBBCEmulator;

class FBBCIOAnalysis
{
public:
	void	Init(FBBCEmulator* pEmulator);
	void	Reset();
	void	RegisterIORead(uint16_t addr, FAddressRef pc);
	void	RegisterIOWrite(uint16_t addr, uint8_t val, FAddressRef pc);

	const FFredAnalysis&	GetFredAnalysis() const { return FredAnalysis;}
	const FJimAnalysis&		GetJimAnalysis() const { return JimAnalysis; }
	const FSheilaAnalysis&	GetSheilaAnalysis() const { return SheilaAnalysis; }
private:

	FFredAnalysis	FredAnalysis;
	FJimAnalysis	JimAnalysis;
	FSheilaAnalysis	SheilaAnalysis;
	FVIAIODevice	SystemVIAIODevice;
	FVIAIODevice	UserVIAIODevice;
	FMC6845Device	CRTCDevice;
};