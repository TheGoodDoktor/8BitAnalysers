#pragma once

#include "ViewerBase.h"

class FDebugger;

// Set to 1 to draw all debugger panels in tabs within a single "Debugger" window 
#define TABBED_DEBUGGER 0

class FDebuggerViewBase : public FViewerBase
{
public:
	FDebuggerViewBase(FEmuBase* pEmu) : FViewerBase(pEmu) {}
	bool	Init(void) override;
	void	Shutdown(void) override {}
protected:
	FDebugger*	pDebugger = nullptr;
};

#if TABBED_DEBUGGER

// All debugger panels combined into tabs within a single window.
class FDebuggerTabsViewer : public FDebuggerViewBase
{
public:
	FDebuggerTabsViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Debugger"; }
	void	DrawUI(void) override;
};

#else

class FBreakpointsViewer : public FDebuggerViewBase
{
public:
	FBreakpointsViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Breakpoints"; }
	void	DrawUI(void) override;
};

class FWatchesViewer : public FDebuggerViewBase
{
public:
	FWatchesViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Watches"; }
	void	DrawUI(void) override;
};

class FRegistersViewer : public FDebuggerViewBase
{
public:
	FRegistersViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Registers"; }
	void	DrawUI(void) override;
};

class FStackViewer : public FDebuggerViewBase
{
public:
	FStackViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Stack"; }
	void	DrawUI(void) override;
};

class FCallStackViewer : public FDebuggerViewBase
{
public:
	FCallStackViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Call Stack"; }
	void	DrawUI(void) override;
};

class FTraceViewer : public FDebuggerViewBase
{
public:
	FTraceViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Trace"; }
	void	DrawUI(void) override;
};

class FEventsViewer : public FDebuggerViewBase
{
public:
	FEventsViewer(FEmuBase* pEmu) : FDebuggerViewBase(pEmu) { Name = "Events"; }
	void	DrawUI(void) override;
};

#endif // TABBED_DEBUGGER
