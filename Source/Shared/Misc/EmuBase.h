#pragma once

#include "CodeAnalyser/CodeAnalyser.h"
#include "GamesList.h"

// forward decs
class FEmuBase;
class FGraphicsViewer;
class FCharacterMapViewer;
class FFunctionViewer;
class FViewerBase;
class FGlobalsViewer;
class FMemoryAnalyser;
class FStaticAnalyser;

struct FProjectConfig;
struct FEmulatorFile;
struct FGlobalConfig;

struct FEmulatorLaunchConfig
{
	virtual void ParseCommandline(int argc, char** argv);

	std::string		SpecificGame;

	bool		bMultiWindow = true;
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

	virtual bool	LoadLua(){ return false;}

	virtual bool	LoadEmulatorFile(const FEmulatorFile* pSnapshot) = 0;

	virtual bool	NewProjectFromEmulatorFile(const FEmulatorFile& gameSnapshot) = 0;
	virtual bool	LoadProject(FProjectConfig* pConfig, bool bLoadGame) = 0;
	virtual bool	SaveProject(void) = 0;

	virtual void	OnEnterEditMode(void) {}
	virtual void	OnExitEditMode(void) {}

	bool			StartGameFromName(const char* pGameName, bool bLoadGame);

	void			GraphicsViewerSetView(FAddressRef address);
	void			CharacterMapViewerSetView(FAddressRef address);

	bool			DrawDockingView();
	void			DrawMainMenu();
	void			DrawUI();
	virtual void	DrawEmulatorUI() = 0;

	void			AddViewer(FViewerBase* pViewr);

	void			SetXHighlight(int x) { HighlightXPos = x; }
	void			SetYHighlight(int y) { HighlightYPos = y; }
	void			SetScanlineHighlight(int scanline, uint32_t colour = 0x50ffffff) 
	{
		HighlightScanline = scanline; 
		HighlightScanlineCol = colour;
	}

	int				GetHighlightX() const { return HighlightXPos; }
	int				GetHighlightY() const { return HighlightYPos; }
	int				GetHighlightScanline() const { return HighlightScanline; }
	int				GetHighlightScanlineColour() const { return HighlightScanlineCol; }

	FCodeAnalysisState&		GetCodeAnalysis() { return CodeAnalysis; }
	const FGlobalConfig*	GetGlobalConfig() const { return pGlobalConfig; }
	const FProjectConfig*		GetProjectConfig() const { return pCurrentProjectConfig; }

	std::string		GetGameWorkspaceRoot() const;
	
    void			SetLastError(const char* fmt, ...);
	void			DisplayErrorMessage(const char *fmt, ...);

	FGlobalsViewer* GetGlobalsViewer() { return pGlobalsViewer; }

	void		FixupAddressRefs();

	// Games List
	bool	AddGamesList(const char* pFileType, const char* pRootDir);
	
	void	LoadFont();

	bool	IsLabelStubbed(const char* pLabelName);
	bool	AddStubbedLabel(const char* pLabelName);
	bool	RemoveStubbedLabel(const char* pLabelName);

protected:
	void			FileMenu();
	void			SystemMenu();
	void			OptionsMenu();
	void			ActionsMenu();
	void			WindowsMenu();
	
	virtual void	FileMenuAdditions(void) {}	// system specific additions
	virtual void	SystemMenuAdditions(void) {}	// system specific additions
	virtual void	OptionsMenuAdditions(void) {}	// system specific additions
	virtual void	ActionMenuAdditions(void) {}	// system specific additions
	virtual void	WindowsMenuAdditions(void) {}	// system specific additions

	void			DrawEditGlobalConfigModalPopup(void);
	void			DrawExportAsmModalPopup(void);
	void			DrawReplaceGameModalPopup(void);
	void			DrawErrorMessageModalPopup(void);

	FGlobalConfig*		pGlobalConfig = nullptr;
	FProjectConfig*		pCurrentProjectConfig = nullptr;

	FCodeAnalysisState  CodeAnalysis;
	//FGamesList			GamesList;
	std::unordered_map<std::string, FGamesList>	GamesLists;
	FGraphicsViewer*	pGraphicsViewer = nullptr;
	FCharacterMapViewer* pCharacterMapViewer = nullptr;
	FFunctionViewer*	pFunctionViewer = nullptr;
	FGlobalsViewer*		pGlobalsViewer = nullptr;
	FMemoryAnalyser*	pMemoryAnalyser = nullptr;	
	FStaticAnalyser*	pStaticAnalysis = nullptr;

	// Highligthing
	int					HighlightXPos = -1;
	int					HighlightYPos = -1;
	int					HighlightScanline = -1;
	uint32_t			HighlightScanlineCol = 0x50ffffff;

	

	// Assembler Export
	uint16_t			ExportStartAddress = 0x0000;
	uint16_t			ExportEndAddress = 0xffff;
	
public:
	bool		bShowImGuiDemo = false;
	bool		bShowImPlotDemo = false;

	// Coord viewer
	FAddressRef	XCoordAddress;	// for debugging
	FAddressRef	YCoordAddress;
	bool		bInvertXCoord = false;
	bool		bInvertYCoord = false;
	bool		bXCoordChars = false;
	bool		bYCoordChars = false;
	bool		bShowCoordinates = false;
protected:
	bool		bShowDebugLog = false;
	bool		bReplaceGamePopup = false;
	bool		bExportAsm = false;
	bool		bExportBinary = false;
	bool 		bEditGlobalConfig = false;

	FEmulatorFile EmulatorFileToLoad;	// for 'are you sure?' popup

	std::string ErrorPopupText;
	std::string LastError;
	bool				bErrorMessagePopup = false;

	std::vector<FViewerBase*>	Viewers;
};
