#pragma once

#include <cinttypes>
#include <string>
#include "Util/Misc.h"

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
	uint16_t				CurrentAddress = 0;
	std::string				Text;
};

class FAnalysisDasmState : public FDasmStateBase
{
public:
	void OutputU8(uint8_t val, dasm_output_t outputCallback) override;
	void OutputU16(uint16_t val, dasm_output_t outputCallback) override;
	void OutputD8(int8_t val, dasm_output_t outputCallback) override;

	FCodeInfo* pCodeInfoItem = nullptr;
};

uint8_t AnalysisDasmInputCB(void* pUserData);
void AnalysisOutputCB(char c, void* pUserData);

class FExportDasmState : public FDasmStateBase
{
public:
	void OutputU8(uint8_t val, dasm_output_t outputCallback) override;
	void OutputU16(uint16_t val, dasm_output_t outputCallback) override;
	void OutputD8(int8_t val, dasm_output_t outputCallback) override;

	FCodeInfo* pCodeInfoItem = nullptr;
	ENumberDisplayMode	HexDisplayMode = ENumberDisplayMode::HexDollar;
};

uint8_t ExportDasmInputCB(void* pUserData);
void ExportOutputCB(char c, void* pUserData);

void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj);
void DasmOutputU8(uint8_t val, dasm_output_t out_cb, void* user_data);
void DasmOutputU16(uint16_t val, dasm_output_t out_cb, void* user_data);
void DasmOutputD8(int8_t val, dasm_output_t out_cb, void* user_data);
