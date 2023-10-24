#pragma once

#include <string>

class FCpcEmu;

class FViewerBase
{
	friend class FCpcEmu;
public:
					FViewerBase(FCpcEmu* pEmu) : pCpcEmu(pEmu) {}
	virtual bool	Init() = 0;
	virtual void	DrawUI() = 0;
	const char*		GetName() const { return Name.c_str(); }
protected:
	FCpcEmu*	pCpcEmu;
	std::string		Name;
private:
	bool		bOpen = true;
};