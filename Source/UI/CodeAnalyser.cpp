#include "CodeAnalyser.h"
#include <cstdint>
#include "SpeccyUI.h"

bool CheckJumpInstruction(FSpeccy* pSpeccy, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = ReadySpeccyByte(pSpeccy, pc);

	switch (instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* JP nnnn */
	case 0xC3:
		/* JP cc,nnnn */
	case 0xDA: case 0xFA: case 0xD2: case 0xC2:
	case 0xF2: case 0xEA: case 0xE2: case 0xCA:
		*out_addr = (ReadySpeccyByte(pSpeccy, pc + 2) << 8) | ReadySpeccyByte(pSpeccy, pc + 1);
		return true;

		/* DJNZ d */
	case 0x10:
		/* JR d */
	case 0x18:
		/* JR cc,d */
	case 0x38: case 0x30: case 0x20: case 0x28:
	{
		const int8_t relJump = (int8_t)ReadySpeccyByte(pSpeccy, pc + 1);
		*out_addr = pc + 2 + relJump;	// +2 because it's relative to the next instruction
	}
		return true;
		/* RST */
	case 0xC7:  *out_addr = 0x00; return true;
	case 0xCF:  *out_addr = 0x08; return true;
	case 0xD7:  *out_addr = 0x10; return true;
	case 0xDF:  *out_addr = 0x18; return true;
	case 0xE7:  *out_addr = 0x20; return true;
	case 0xEF:  *out_addr = 0x28; return true;
	case 0xF7:  *out_addr = 0x30; return true;
	case 0xFF:  *out_addr = 0x38; return true;
	}

	return false;
}

// check if function should stop static analysis
// this would be a function that unconditionally affects the PC
bool CheckStopInstruction(FSpeccy* pSpeccy, uint16_t pc)
{
	const uint8_t instrByte = ReadySpeccyByte(pSpeccy, pc);

	switch(instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* RST */
	case 0xC7:
	case 0xCF:
	case 0xD7:
	case 0xDF:
	case 0xE7:
	case 0xEF:
	case 0xF7:
	case 0xFF:
		// ret
	case 0xC8:
	case 0xC9:
	case 0xD8:
	case 0xE8:
	case 0xF8:
	
		
	case 0xc3:// jp
	case 0xe9:// jp(hl)
	case 0x18://jr
		return true;
	case 0xED:	// extended instructions
	{
		const uint8_t extInstrByte = ReadySpeccyByte(pSpeccy, pc+1);
		switch(extInstrByte)
		{
		case 0x4D:	// more RET functions
		case 0x5D:
		case 0x6D:
		case 0x7D:
		case 0x45:	
		case 0x55:
		case 0x65:
		case 0x75:
			return true;
		}
	}
	// index register instructions
	case 0xDD:	// IX
	case 0xFD:	// IY
	{
		const uint8_t extInstrByte = ReadySpeccyByte(pSpeccy, pc + 1);
		switch (extInstrByte)
		{
		case 0xE9:	// JP(IX)
			return true;
		}
	}
	default:
		return false;
	}
}

bool GenerateLabelsForAddress(FSpeccyUI *pUI, uint16_t pc)
{
	uint16_t outAddr;
	if (CheckJumpInstruction(pUI->pSpeccy, pc, &outAddr))
	{
		FLabelInfo* pLabel = pUI->Labels[outAddr];
		if (pLabel == nullptr)
		{
			pLabel = new FLabelInfo;
			char label[32];
			sprintf(label, "label_0x%x", outAddr);
			pLabel->Name = label;
			pUI->Labels[outAddr] = pLabel;
		}

		//pLabel->References[pc]++;	// add/increment reference
		return true;
	}
	return false;
}



/* disassembler callback to fetch the next instruction byte */
static uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FCodeAnalysisState* pAnalysis = (FCodeAnalysisState*)pUserData;

	const uint8_t val = ReadySpeccyByte(pAnalysis->pUI->pSpeccy, pAnalysis->CurrentAddress++);

	// push into binary buffer
	//if (pDasm->bin_pos < DASM_MAX_BINLEN)
	//	pDasm->bin_buf[pDasm->bin_pos++] = val;

	return val;
}

/* disassembler callback to output a character */
static void AnalysisOutputCB(char c, void* user_data)
{
	FCodeAnalysisState* pAnalysis = (FCodeAnalysisState*)user_data;

	// add character to string
	assert(pAnalysis->pInstructionInfo);
	pAnalysis->pInstructionInfo->Text += c;
}

void AnalyseFromPC(FCodeAnalysisState* pState, uint16_t pc)
{
	if (pState->CodeInfo[pc] != nullptr)	// already been analysed
		return;

	/*if(pc >= 0x4000 && pc <= 0x5AFF)	// screen memory
	{
		assert(0);
	}*/

	pState->bDirty = true;

	if(pc == 0x7e38)
	{
		assert(0);

	}
	bool bStop = false;

	while (bStop == false)
	{
		uint16_t jumpAddr;
		
		FCodeInfo *pNewCodeInfo = new FCodeInfo;
		pState->CurrentAddress = pc;
		pState->pInstructionInfo = pNewCodeInfo;
		const uint16_t newPC = z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, pState);
		pNewCodeInfo->Address = pc;
		pNewCodeInfo->ByteSize = newPC - pc;

		//for (uint16_t i = pc; i < newPC; i++)
		//	state.CodeInfo[i] = pNewCodeInfo;
		pState->CodeInfo[pc] = pNewCodeInfo;	// just set to first instruction?

		// does this function branch?
		if (CheckJumpInstruction(pState->pUI->pSpeccy, pc, &jumpAddr))
		{
			//fprintf(stderr,"Jump 0x%04X - > 0x%04X\n", pc, jumpAddr);
			GenerateLabelsForAddress(pState->pUI, jumpAddr);
			AnalyseFromPC(pState, jumpAddr);
			bStop = false;	// should just be call & rst really
		}
		
		// do we need to stop tracing ??
		if (CheckStopInstruction(pState->pUI->pSpeccy, pc) || newPC < pc)
		{
			bStop = true;
			//pNewCodeInfo->EndPoint = true;
		}
		else
			pc = newPC;
	}
	
}

void RunStaticCodeAnalysis(FSpeccyUI *pUI, uint16_t pc)
{
	//const uint8_t instrByte = ReadySpeccyByte(pUI->pSpeccy, pc);

	if (pUI->pCodeAnalysis == nullptr)
	{
		FCodeAnalysisState* pState = new FCodeAnalysisState;
		pState->pUI = pUI;
		memset(pState->CodeInfo, 0, sizeof(pState->CodeInfo));

		pUI->pCodeAnalysis = pState;
	}

	AnalyseFromPC(pUI->pCodeAnalysis, pc);
	
	//if (bRead)
	//	pUI->MemStats.ReadCount[addr]++;
	//if (bWrite)
	//	pUI->MemStats.WriteCount[addr]++;
	//
}


void DrawCodeAnalysisData(FSpeccyUI *pUI)
{
	FCodeAnalysisState* pState = pUI->pCodeAnalysis;
	if(pState == nullptr)
	{
		ImGui::Text("Code Analysis not run");
		return;
	}
	
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	ImGui::BeginChild("##analysis", ImVec2(0, 0), true);
	{
		// build item list - not every frame please!
		static std::vector< const FCodeInfo *> itemList;

		if (pState->bDirty)
		{
			itemList.clear();

			for (int addr = 0; addr < 0x10000; addr++)
			{
				const FCodeInfo *pNewCodeInfo = pState->CodeInfo[addr];
				if (pNewCodeInfo != nullptr)
				{
					itemList.push_back(pNewCodeInfo);
				}
			}

			pState->bDirty = false;
		}

		// draw clipped list
		ImGuiListClipper clipper((int)itemList.size());
		const FCodeInfo *pPrevCodeInfo = nullptr;
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FCodeInfo *pCodeInfo = itemList[i];

				if(pPrevCodeInfo!=nullptr && pCodeInfo->Address > pPrevCodeInfo->Address + pPrevCodeInfo->ByteSize)
					ImGui::Separator();

				// label
				/*if (pUI->Labels[pCodeInfo->Address] != nullptr)
				{
					ImGui::Text("%s: ", pUI->Labels[pCodeInfo->Address]->Name.c_str());
					//ImGui::SameLine();
				}*/
				ImGui::Text("\t0x%04X", pCodeInfo->Address);
				const float line_start_x = ImGui::GetCursorPosX();
				ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
				ImGui::Text("%s", pCodeInfo->Text.c_str());

				//if (pCodeInfo->EndPoint == true)
				//	ImGui::Separator();

				pPrevCodeInfo = pCodeInfo;
			}
		}
	}
	ImGui::EndChild();
}