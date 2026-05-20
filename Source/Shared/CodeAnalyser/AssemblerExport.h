#pragma once

#include <cinttypes>
#include <string>
#include <map>
#include <vector>

#include "CodeAnalyserTypes.h"
#include "Disassembler.h"
#include "Util/Misc.h"

class FCodeAnalysisState;
struct FCodeAnalysisBank;
struct FFunctionInfo;

struct FAssemblerConfig
{
	const char* DataBytePrefix = nullptr;
	const char* DataWordPrefix = nullptr;
	const char* DataTextPrefix = nullptr;
	const char* ORGText = nullptr;
	const char* EQUText = nullptr;
	const char* LocalLabelPrefix = nullptr;
	bool bUseLocalLabelPrefix = true;
};

// Class to encapsulate ASM exporting
class FASMExporter
{
public:
	bool		Init(const char* pFilename, class FEmuBase* pEmu);
	bool		Init(std::string* pOutStr, class FEmuBase* pEmu);
	bool		Finish();
	void		SetOutputToHeader(){OutputString = &HeaderText;}
	void		SetOutputToBody(){OutputString = &BodyText;}
	void		Output(const char* pFormat, ...);

	virtual void	ExportDidBegin(){}
	virtual void	ExportDidEnd(){}
	virtual void	AddHeader(void) {}
	virtual void	AddBankSection(const FCodeAnalysisBank* pBank);
	virtual void	ProcessLabelsOutsideExportedRange(void){}
	//void			OutputFunctionDescription(const FFunctionInfo* pFunctionInfo);

	// sam. Output operand label at labelAddress. Returns the label if was successful or nullptr if not.
	// The labelAddress can be modified by the function.
	virtual FLabelInfo* OutputOperandLabelAtAddress(FAddressRef& labelAddress, uint16_t disassemblyValue, dasm_output_t outputCallback); // sam

	bool		ExportAddressRange(const std::vector<FCodeAnalysisItem>& itemList, uint16_t startAddr, uint16_t endAddr, bool bIsPhysicalMem	);
	void		QueueWarning(const char* pFormat, ...);

	//std::string		GenerateAddressLabelString(FAddressRef addr);
	void			ExportDataInfoASM(FAddressRef addr);
	bool			TryExportByteRun(FCodeAnalysisState& state, const std::vector<FCodeAnalysisItem>& itemList, int& itemIdx, uint16_t endAddr, uint16_t& nextAddr);
	const FAssemblerConfig&	GetConfig() const { return	Config;}

	bool			IsLabelStubbed(const char* pLabelName) const;

	// todo tidy up this
	std::vector<const FCodeAnalysisBank*> ExportBanks;

protected:
	// sam. Search backwards from addr for the nearest label in the same bank.
	// On success, sets labelAddress to the label's address, sets offset to the
	// distance from that label to addr, and returns the label. Returns nullptr if none found.
	FLabelInfo*		FindNearestLabel(FAddressRef addr, FAddressRef& labelAddress, int& offset) const;
	
	// sam. Output labelName[+bankMappingOffset][+searchOffset]. bankMappingOffset is non-zero when
	// the label's bank was mapped to a different address at encode time than at export time.
	void			OutputLabelWithOffsets(const FLabelInfo* pLabel, const FAddressRef& labelAddress, int searchOffset, uint16_t disassemblyValue, dasm_output_t outputCallback);

	void				OutputDataItemBytes(FAddressRef addr, const FDataInfo* pDataInfo);
	int				OutputOpcodeBytes(FAddressRef addr, const FCodeInfo* pCodeInfo);
	ENumberDisplayMode	GetNumberDisplayModeForDataItem(const FDataInfo* pDataInfo);

	// sam. track line numbers
	struct FDeferredWarning
	{
		int			BodyLineNumber;
		std::string	Message;
	};

	int				BodyLineNumber = 1;
	int				HeaderLineNumber = 1;
	std::vector<FDeferredWarning> DeferredWarnings;
	// sam. track line numbers

	bool			bInitialised = false;
	bool			bLogImmediately = false;
	ENumberDisplayMode HexMode = ENumberDisplayMode::HexDollar;
	ENumberDisplayMode OldNumberMode;

	std::string		Filename;
	FILE* FilePtr = nullptr;
	FEmuBase* pEmulator = nullptr;

	std::set<FExportRange> ExportRanges;

	FExportDasmState	DasmState;
	std::string		HeaderText;
	std::string		BodyText;
	std::string*	OutputString = nullptr;

	std::string*	ExportOutputString = nullptr;

	FAssemblerConfig	Config;
};


const std::map<std::string, FASMExporter*>&	GetAssemblerExporters();

bool AddAssemblerExporter(const char* pName, FASMExporter* pExporter);

// Export physical address range to a file.
bool ExportAssembler(class FEmuBase* pEmu, const char* pTextFileName, uint16_t startAddr, uint16_t endAddr);

// Export selected banks to a file. Passing in an empty bankList will export all banks. 
bool ExportAssemblerForBanks(class FEmuBase* pEmu, const char* pTextFileName, const std::vector<int16_t>& bankList = std::vector<int16_t>());

// Export physical address range to a string.
bool ExportAssembler(FEmuBase* pEmu, std::string* pOutStr, uint16_t startAddr, uint16_t endAddr);

// needed?
//bool ExportFunctionStubs(FEmuBase* pEmu, const char* pTextFileName);
