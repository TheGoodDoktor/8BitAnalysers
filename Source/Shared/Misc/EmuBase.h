#pragma once

#include "CodeAnalyser/CodeAnalyser.h"

class FEmuBase;
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
	virtual void	AppFocusCallback(int focused){}

	bool			DrawDockingView();
	void			DrawMainMenu();
	void			DrawUI();
	virtual void	DrawEmulatorUI() = 0;

	void			AddViewer(FViewerBase* pViewr);
	void			InitViewers();

	void			SetXHighlight(int x) { HighlightXPos = x; }
	void			SetYHighlight(int y) { HighlightYPos = y; }
	void			SetScanlineHighlight(int scanline) { HighlightScanline = scanline;}

	int				GetHighlightX() const { return HighlightXPos; }
	int				GetHighlightY() const { return HighlightYPos; }
	int				GetHighlightScanline() const { return HighlightScanline;}

	FCodeAnalysisState&		GetCodeAnalysis() { return CodeAnalysis; }
	const FGlobalConfig*	GetGlobalConfig() const { return pGlobalConfig; }

protected:
	void	FileMenu();
	void	OptionsMenu();
	void	WindowsMenu();

	FGlobalConfig*		pGlobalConfig = nullptr;
	FCodeAnalysisState  CodeAnalysis;

	// Highligthing
	int					HighlightXPos = -1;
	int					HighlightYPos = -1;
	int					HighlightScanline = -1;

	std::vector<FViewerBase*>	Viewers;
public:
	bool		bShowImGuiDemo = false;
	bool		bShowImPlotDemo = false;

};