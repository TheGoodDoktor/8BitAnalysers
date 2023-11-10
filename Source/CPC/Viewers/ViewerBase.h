#pragma once
#if 0

#include <string>

class FCPCEmu;

class FViewerBase
{
	friend class FCPCEmu;
public:
					FViewerBase(FCPCEmu* pEmu) : pCPCEmu(pEmu) {}
	virtual bool	Init() = 0;
	virtual void	DrawUI() = 0;
	const char*		GetName() const { return Name.c_str(); }
protected:
	FCPCEmu*	pCPCEmu;
	std::string		Name;
private:
	bool		bOpen = true;
};

#endif