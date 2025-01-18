#include "BBCIOAnalysis.h"
#include "../BBCEmulator.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include <imgui.h>

void	FBBCIOAnalysis::Init(FBBCEmulator* pEmulator)
{
	FredAnalysis.Init(pEmulator);
	JimAnalysis.Init(pEmulator);
	SheilaAnalysis.Init(pEmulator);

	SystemVIAIODevice.Init("System VIA", pEmulator, &pEmulator->GetBBC().via_system);
	UserVIAIODevice.Init("User VIA",pEmulator, &pEmulator->GetBBC().via_user);
	CRTCDevice.Init("CRTC", pEmulator, &pEmulator->GetBBC().crtc);
}

void	FBBCIOAnalysis::Reset()
{
	FredAnalysis.Reset();
	JimAnalysis.Reset();
	SheilaAnalysis.Reset();
}


void	FBBCIOAnalysis::RegisterIORead(uint16_t addr, FAddressRef pc)
{
	const uint8_t page = addr >> 8;
	const uint8_t reg = addr & 0xff;

	if (page == kFredPage)
	{
		FredAnalysis.OnRegisterRead(reg, pc);
	}
	else if (page == kJimPage)
	{
		JimAnalysis.OnRegisterRead(reg, pc);
	}
	else if (page == kSheilaPage)
	{
		SheilaAnalysis.OnRegisterRead(reg, pc);
	}
}

void	FBBCIOAnalysis::RegisterIOWrite(uint16_t addr, uint8_t val, FAddressRef pc)
{
	const uint8_t page = addr >> 8;
	const uint8_t reg = addr & 0xff;

	if (page == kFredPage)
	{
		FredAnalysis.OnRegisterWrite(reg, val, pc);
	}
	else if (page == kJimPage)
	{
		JimAnalysis.OnRegisterWrite(reg, val, pc);
	}
	else if (page == kSheilaPage)
	{
		SheilaAnalysis.OnRegisterWrite(reg, val, pc);
	}
}
