#pragma once

#include <string>

class FEmuBase;

// Base class for all viewers
class FViewerBase
{
public:
	FViewerBase(FEmuBase* pEmu) : pEmulator(pEmu) {}
	virtual bool	Init() = 0;
	virtual void	ResetForGame() {};
	virtual void	Shutdown() = 0;
	virtual void	DrawUI() = 0;

	virtual void	FixupAddressRefs() {}

	const char* GetName() const { return Name.c_str(); }
protected:
	FEmuBase* pEmulator = nullptr;
	std::string		Name;
public:
	bool		bOpen = true;
	bool		bCreateImGuiWindow = true;
};
