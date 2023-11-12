#pragma once

#include "CodeAnalyser/CodeAnalyser.h"
#include "GamesList.h"

class FEmuBase;
class FGraphicsViewer;

struct FGameConfig;
struct FGameSnapshot;
struct FGlobalConfig;

struct FEmulatorLaunchConfig
{
	virtual void ParseCommandline(int argc, char** argv);

	std::string		SpecificGame;
};

class FViewerBase
{
public:
	FViewerBase(FEmuBase* pEmu) : pEmulator(pEmu) {}
	virtual bool	Init() = 0;
	virtual void	Shutdown() = 0;
	virtual void	DrawUI() = 0;
	const char* GetName() const { return Name.c_str(); }
protected:
	FEmuBase*		pEmulator = nullptr;
	std::string		Name;
public:
	bool		bOpen = true;
};

// Base class for emulators
class FEmuBase : public ICPUInterface
{
public:
	virtual bool	Init(const FEmulatorLaunchConfig& launchConfig);
	virtual void    Shutdown();
	virtual void    Tick();
	virtual void    Reset();
	virtual void	AppFocusCallback(int focused){}


	virtual bool	NewGameFromSnapshot(const FGameSnapshot& gameConfig) = 0;
	virtual bool	StartGame(FGameConfig* pConfig, bool bLoadGame) = 0;
	virtual bool	SaveCurrentGameData(void) = 0;

	bool			StartGameFromName(const char* pGameName, bool bLoadGame);

	void			GraphicsViewerSetView(FAddressRef address) override;

	bool			DrawDockingView();
	void			DrawMainMenu();
	void			DrawUI();
	virtual void	DrawEmulatorUI() = 0;

	void			AddViewer(FViewerBase* pViewr);

	void			SetXHighlight(int x) { HighlightXPos = x; }
	void			SetYHighlight(int y) { HighlightYPos = y; }
	void			SetScanlineHighlight(int scanline) { HighlightScanline = scanline;}

	int				GetHighlightX() const { return HighlightXPos; }
	int				GetHighlightY() const { return HighlightYPos; }
	int				GetHighlightScanline() const { return HighlightScanline;}

	FCodeAnalysisState&		GetCodeAnalysis() { return CodeAnalysis; }
	const FGlobalConfig*	GetGlobalConfig() const { return pGlobalConfig; }

protected:
	void			FileMenu();
	void			SystemMenu();
	void			OptionsMenu();
	void			WindowsMenu();
	
	virtual void	FileMenuAdditions(void) {}	// system specific additions
	virtual void	SystemMenuAdditions(void) {}	// system specific additions
	virtual void	OptionsMenuAdditions(void) {}	// system specific additions
	virtual void	WindowsMenuAdditions(void) {}	// system specific additions


	void			DrawExportAsmModalPopup(void);
	void			DrawReplaceGameModalPopup(void);

	FGlobalConfig*		pGlobalConfig = nullptr;
	FGameConfig*		pCurrentGameConfig = nullptr;

	FCodeAnalysisState  CodeAnalysis;
	FGamesList			GamesList;
	FGraphicsViewer*	pGraphicsViewer = nullptr;

	// Highligthing
	int					HighlightXPos = -1;
	int					HighlightYPos = -1;
	int					HighlightScanline = -1;


	// Assembler Export
	uint16_t			AssemblerExportStartAddress = 0x0000;
	uint16_t			AssemblerExportEndAddress = 0xffff;
public:
	bool		bShowImGuiDemo = false;
	bool		bShowImPlotDemo = false;
private:
	bool		bShowDebugLog = false;
	bool		bReplaceGamePopup = false;
	bool		bExportAsm = false;

	int		ReplaceGameSnapshotIndex = 0;

	std::vector<FViewerBase*>	Viewers;
};