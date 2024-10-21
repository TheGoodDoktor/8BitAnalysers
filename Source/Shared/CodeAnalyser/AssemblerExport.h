#pragma once

#include <cinttypes>
#include <string>
#include <map>

#include "CodeAnalyserTypes.h"
#include "Disassembler.h"
#include "Util/Misc.h"

class FCodeAnalysisState;

struct FAssemblerConfig
{
	const char* DataBytePrefix = nullptr;
	const char* DataWordPrefix = nullptr;
	const char* DataTextPrefix = nullptr;
	const char* ORGText = nullptr;
	const char* EQUText = nullptr;
	const char* LocalLabelPrefix = nullptr;
};

// Class to encapsulate ASM exporting
class FASMExporter
{
public:
	bool		Init(const char* pFilename, class FEmuBase* pEmu);
	bool		Finish();
	void		SetOutputToHeader(){OutputString = &HeaderText;}
	void		SetOutputToBody(){OutputString = &BodyText;}
	void		Output(const char* pFormat, ...);
	virtual void	AddHeader(void){}
	virtual void	ProcessLabelsOutsideExportedRange(void){}
	bool		ExportAddressRange(uint16_t startAddr, uint16_t endAddr);

	//std::string		GenerateAddressLabelString(FAddressRef addr);
	void			ExportDataInfoASM(FAddressRef addr);
	const FAssemblerConfig&	GetConfig() const { return	Config;}

	bool			IsLabelStubbed(const char* pLabelName) const;

protected:
	void				OutputDataItemBytes(FAddressRef addr, const FDataInfo* pDataInfo);
	ENumberDisplayMode	GetNumberDisplayModeForDataItem(const FDataInfo* pDataInfo);

	bool			bInitialised = false;
	ENumberDisplayMode HexMode = ENumberDisplayMode::HexDollar;
	ENumberDisplayMode OldNumberMode;

	std::string		Filename;
	FILE* FilePtr = nullptr;
	FEmuBase* pEmulator = nullptr;

	FExportDasmState	DasmState;
	std::string		HeaderText;
	std::string		BodyText;
	std::string*	OutputString = nullptr;

	FAssemblerConfig	Config;
};


const std::map<std::string, FASMExporter*>&	GetAssemblerExporters();

bool AddAssemblerExporter(const char* pName, FASMExporter* pExporter);

// TODO: we should have a bank based approach?
bool ExportAssembler(class FEmuBase* pEmu, const char* pTextFileName, uint16_t startAddr, uint16_t endAddr);
