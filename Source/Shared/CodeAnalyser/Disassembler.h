#pragma once

#include <cinttypes>
#include <string>
#include <cstring>
#include "Util/Misc.h"
#include <set>

#include "CodeAnalyserTypes.h"

class FCodeAnalysisState;
struct FCodeInfo;

/* the input callback type */
typedef uint8_t(*dasm_input_t)(void* user_data);

/* the output callback type */
typedef void (*dasm_output_t)(char c, void* user_data);

class IDasmNumberOutput
{
public:

	virtual void OutputU8(uint8_t val, dasm_output_t out_cb) = 0;
	virtual void OutputU16(uint16_t val, dasm_output_t out_cb) = 0;
	virtual void OutputD8(int8_t val, dasm_output_t out_cb) = 0;
};

class FDasmStateBase : public IDasmNumberOutput
{
public:
	void PushString(const char* pString, dasm_output_t outputCallback)
	{
		for (int i = 0; i < strlen(pString); i++)
			outputCallback(pString[i], this);
	}

	FCodeAnalysisState*		CodeAnalysisState = nullptr;
	std::string				Text;
	int						OperandOutputCount = 0;	// incremented by OutputU8/OutputU16, used for per-operand type lookup
};

class FAnalysisDasmState : public FDasmStateBase
{
public:
	void OutputU8(uint8_t val, dasm_output_t outputCallback) override;
	void OutputU16(uint16_t val, dasm_output_t outputCallback) override;
	void OutputD8(int8_t val, dasm_output_t outputCallback) override;

	FCodeInfo* pCodeInfoItem = nullptr;
	uint16_t CurrentAddress = 0;
};

uint8_t AnalysisDasmInputCB(void* pUserData);
void AnalysisOutputCB(char c, void* pUserData);

struct FExportRange
{
	bool operator<(const FExportRange& other) const { return Min < other.Min; }

	uint16_t Min;
	uint16_t Max;
};

class FExportDasmState : public FDasmStateBase
{
public:
	void OutputU8(uint8_t val, dasm_output_t outputCallback) override;
	void OutputU16(uint16_t val, dasm_output_t outputCallback) override;
	void OutputD8(int8_t val, dasm_output_t outputCallback) override;

	class FASMExporter* pExporter = nullptr;
	const FCodeInfo* pCodeInfoItem = nullptr;
	ENumberDisplayMode	HexDisplayMode = ENumberDisplayMode::HexDollar;
	struct FLabelInfo*	pCurrentScope = nullptr;
	FAddressRef CurrentAddress;

	uint16_t	ExportMin = 0;
	uint16_t	ExportMax = 0xffff;

	std::set<FAddressRef>	LabelsOutsideRange;

	// sam. How many raw 16 bit values did we output instead of a label.
	int NumRawValuesOutput = 0;
};

uint8_t ExportDasmInputCB(void* pUserData);
void ExportOutputCB(char c, void* pUserData);

void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj);
void DasmOutputU8(uint8_t val, dasm_output_t out_cb, void* user_data);
void DasmOutputU16(uint16_t val, dasm_output_t out_cb, void* user_data);
void DasmOutputD8(int8_t val, dasm_output_t out_cb, void* user_data);

//std::string GenerateDasmStringForAddress(FCodeAnalysisState& state, uint16_t pc, ENumberDisplayMode hexMode);

bool GenerateDasmExportString(FExportDasmState& exportState);