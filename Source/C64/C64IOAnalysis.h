#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>

struct FCodeAnalysisState;

struct FC64IORegisterAccessInfo
{
	std::set<uint8_t>	WriteVals;
};

struct FC64IORegisterInfo
{
	std::map<uint16_t, FC64IORegisterAccessInfo>	Accesses;
	uint8_t			LastVal;
};

class FC64IOAnalysis
{
public:
	void	Init(FCodeAnalysisState *pAnalysis);
	void	RegisterIORead(uint16_t addr, uint16_t pc);
	void	RegisterIOWrite(uint16_t addr, uint8_t val, uint16_t pc);

	void	DrawIOAnalysisUI(void);
private:
	void	RegisterVICRead(uint8_t reg, uint16_t pc);
	void	RegisterVICWrite(uint8_t reg, uint8_t val, uint16_t pc);

	void	RegisterSIDRead(uint8_t reg, uint16_t pc);
	void	RegisterSIDWrite(uint8_t reg, uint8_t val, uint16_t pc);

	FC64IORegisterInfo	VICRegisters[64];	// buffer of VIC registers for comparisons etc	
	FC64IORegisterInfo SIDRegisters[32];	// buffer of SID registers for comparisons etc	

	int		UIVICRegister = -1;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};