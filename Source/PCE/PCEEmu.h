#pragma once

#include "Misc/EmuBase.h"

class GeargrafxCore;
struct FPCEConfig;
struct FPCEGameConfig;
class FPCECPUEmulator6502;

struct FPCELaunchConfig : public FEmulatorLaunchConfig
{
	void ParseCommandline(int argc, char** argv) override;
};

class FPCEEmu : public FEmuBase
{
public:
	FPCEEmu()
	{
	}

	// FEmuBase Begin
	bool	Init(const FEmulatorLaunchConfig& config) override;
	void	Shutdown() override;
	void	Tick() override;
	void	Reset() override;
	void	OnEnterEditMode(void) override;
	void	OnExitEditMode(void) override;
	bool	LoadEmulatorFile(const FEmulatorFile* pSnapshot) override;
	bool	NewProjectFromEmulatorFile(const FEmulatorFile& snapshot) override;
	bool	LoadProject(FProjectConfig* pGameConfig, bool bLoadGame) override;
	bool	SaveProject() override;

	void	FileMenuAdditions(void) override;
	void	SystemMenuAdditions(void)  override;
	void	OptionsMenuAdditions(void) override;
	void	ActionMenuAdditions(void) override;
	void	WindowsMenuAdditions(void)  override;
	void	AppFocusCallback(int focused) override;
	void	DrawEmulatorUI() override;
	// FEmuBase End

	// disable copy & assign because this class is big!
	FPCEEmu(const FPCEEmu&) = delete;
	FPCEEmu& operator= (const FPCEEmu&) = delete;

	//ICPUInterface Begin
	uint8_t		ReadByte(uint16_t address) const override;
	uint16_t	ReadWord(uint16_t address) const override;
	const uint8_t*	GetMemPtr(uint16_t address) const override;
	void		WriteByte(uint16_t address, uint8_t value) override;

	FAddressRef	GetPC(void) override;
	uint16_t	GetSP(void) override;
	ICPUEmulator*		GetCPUEmulator(void) const override;
	//ICPUInterface End

	const FPCEConfig* GetPCEGlobalConfig() { return (const FPCEConfig*)pGlobalConfig; }

	GeargrafxCore* GetCore() const { return pCore; }
	uint8_t* GetFrameBuffer() const { return pFrameBuffer; }

	static const int kNumBanks = 256;
	int16_t Banks[kNumBanks];
	int16_t MprBankId[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
protected:
	bool LoadMachineState(const char* fname);
	bool SaveMachineState(const char* fname);
	
protected:
	GeargrafxCore* pCore = nullptr;
	uint8_t* pFrameBuffer = nullptr;
	int16_t* pAudioBuf = nullptr;
	FPCECPUEmulator6502* pPCE6502CPU;
};
