#pragma once

#include <string>

class FSpectrumEmu;

class FViewerBase
{
	friend class FSpectrumEmu;
public:
					FViewerBase(FSpectrumEmu* pEmu) : pSpectrumEmu(pEmu) {}
	virtual bool	Init() = 0;
	virtual void	DrawUI() = 0;
	const char*		GetName() const { return Name.c_str(); }
protected:
	FSpectrumEmu*	pSpectrumEmu;
	std::string		Name;
private:
	bool		bOpen = true;
};