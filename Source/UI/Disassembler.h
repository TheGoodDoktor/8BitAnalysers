#pragma once
#include <cstdint>
#include <string>

// Disassembler - based on Andre Weissflog's # ui_dasm.h

/* CPU types */
enum class DasmCPUType
{
	Z80 = 0,
	M6502 = 1,
};

#define DASM_MAX_LAYERS (16)
#define DASM_MAX_STRLEN (32)
#define DASM_MAX_BINLEN (16)
#define DASM_MAX_STACK (128)
#define DASM_NUM_LINES (512)


/* callback for reading a byte from memory */
typedef uint8_t(*DasmReadCallback)(int layer, uint16_t addr, void* user_data);

// Description structure for creating Disassembler
struct FDasmDesc
{
	std::string			Title;  /* window title */
	std::string			LayerNames[DASM_MAX_LAYERS];   /* memory system layer names */
	DasmCPUType			CPUType;     /* only needed when defining both UI_DASM_CPUTYPE_Z80 and _M6502 */
	uint16_t			StartAddress;
	DasmReadCallback	ReadCB;
	void*				pUserData;

	// items below might get removed
	int					x, y;           /* initial window pos */
	int					w, h;           /* initial window size or 0 for default size */
	bool				open;          /* initial open state */
};

// Disassembler runtime state
struct FDasmState
{
	std::string			Title;
	DasmReadCallback	ReadCB;
	DasmCPUType			CPUType;
	int					CurLayer;
	int					NumLayers;
	std::string			LayerNames[DASM_MAX_LAYERS];
	void*				pUserData;

	float init_x, init_y;
	float init_w, init_h;
	bool open;
	bool valid;
	
	uint16_t			StartAddress;
	uint16_t			CurrentAddress;
	
	int					str_pos;
	char				str_buf[DASM_MAX_STRLEN];
	int					bin_pos;
	uint8_t				bin_buf[DASM_MAX_BINLEN];
	
	int					StackNum;
	int					StackPos;
	uint16_t			Stack[DASM_MAX_STACK];
	uint16_t			HighlightAddr;
	uint32_t			HighlightColour;
};

void DasmInit(FDasmState* pDasmState, const FDasmDesc* pDasmDesc);
void DasmDiscard(FDasmState* pDasmState);
void DasmDraw(FDasmState* pDasmState);