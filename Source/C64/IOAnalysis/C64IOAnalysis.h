#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>

#include "VICAnalysis.h"
#include "SIDAnalysis.h"
#include "CIAAnalysis.h"

class FCodeAnalysisState;

class FC64IOAnalysis
{
public:
	void	Init(FC64Emulator* pEmulator);
	void	Reset();
	void	RegisterIORead(uint16_t addr, FAddressRef pc);
	void	RegisterIOWrite(uint16_t addr, uint8_t val, FAddressRef pc);

	void	DrawIOAnalysisUI(void);

	const FVICAnalysis&	GetVICAnalysis() const { return VICAnalysis;}
	const FSIDAnalysis& GetSIDAnalysis() const { return SIDAnalysis;}
	const FCIA1Analysis& GetCIA1Analysis() const { return CIA1Analysis; }
	const FCIA2Analysis& GetCIA2Analysis() const { return CIA2Analysis; }
private:

	FVICAnalysis	VICAnalysis;
	FSIDAnalysis	SIDAnalysis;
	FCIA1Analysis	CIA1Analysis;
	FCIA2Analysis	CIA2Analysis;
};