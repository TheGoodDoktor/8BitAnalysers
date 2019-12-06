#include "CodeAnalyser.h"
#include <cstdint>
#include "SpeccyUI.h"

#include "ROMLabels.h"
#include <algorithm>

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

bool CheckCallInstruction(FSpeccy* pSpeccy, uint16_t pc)
{
	const uint8_t instrByte = ReadySpeccyByte(pSpeccy, pc);

	switch (instrByte)
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
		return true;
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

bool GenerateLabelsForAddress(FSpeccyUI *pUI, uint16_t pc, LabelType labelType)
{
	//uint16_t outAddr;
	//if (CheckJumpInstruction(pUI->pSpeccy, pc, &outAddr))
	{
		FLabelInfo* pLabel = pUI->Labels[pc];
		if (pLabel == nullptr)
		{
			pLabel = new FLabelInfo;
			pLabel->LabelType = labelType;
			pLabel->Address = pc;
			pLabel->ByteSize = 0;
			
			//if(GetROMLabelName(pc, pLabel->Name) == false)
			{
				char label[32];
				switch (labelType)
				{
				case LabelType::Function:
					sprintf(label, "function_0x%04X", pc);
					break;
				case LabelType::Code:
					sprintf(label, "label_0x%04X", pc);
					break;
				case LabelType::Data:
					sprintf(label, "label_0x%04X", pc);
					break;
				}

				pLabel->Name = label;
			}
			pUI->Labels[pc] = pLabel;
			return true;
		}

	}
	return false;
}


struct FAnalysisDasmState
{
	FSpeccy*		pSpeccy;
	uint16_t		CurrentAddress;
	std::string		Text;
};


/* disassembler callback to fetch the next instruction byte */
static uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	const uint8_t val = ReadySpeccyByte(pDasmState->pSpeccy, pDasmState->CurrentAddress++);

	// push into binary buffer
	//if (pDasm->bin_pos < DASM_MAX_BINLEN)
	//	pDasm->bin_buf[pDasm->bin_pos++] = val;

	return val;
}

/* disassembler callback to output a character */
static void AnalysisOutputCB(char c, void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	// add character to string
	pDasmState->Text += c;
}

void AnalyseFromPC(FSpeccyUI *pUI, uint16_t pc)
{
	if (pUI->CodeInfo[pc] != nullptr)	// already been analysed
		return;

	/*if(pc >= 0x4000 && pc <= 0x5AFF)	// screen memory
	{
		assert(0);
	}*/

	pUI->bCodeAnalysisDataDirty = true;

	/*if(pc == 0x7e38)
	{
		assert(0);

	}*/
	bool bStop = false;

	while (bStop == false)
	{
		uint16_t jumpAddr;
		
		FAnalysisDasmState dasmState;
		dasmState.pSpeccy = pUI->pSpeccy;
		dasmState.CurrentAddress = pc;
		const uint16_t newPC = z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
		FCodeInfo *pNewCodeInfo = new FCodeInfo;
		pNewCodeInfo->Text = dasmState.Text;
		pNewCodeInfo->Address = pc;
		pNewCodeInfo->ByteSize = newPC - pc;
		pUI->CodeInfo[pc] = pNewCodeInfo;	// just set to first instruction?

		//for (uint16_t i = pc; i < newPC; i++)
		//	state.CodeInfo[i] = pNewCodeInfo;

		// does this function branch?
		if (CheckJumpInstruction(pUI->pSpeccy, pc, &jumpAddr))
		{
			//fprintf(stderr,"Jump 0x%04X - > 0x%04X\n", pc, jumpAddr);
			const bool isCall = CheckCallInstruction(pUI->pSpeccy, pc);
			GenerateLabelsForAddress(pUI, jumpAddr, isCall ? LabelType::Function : LabelType::Code);

			FLabelInfo* pLabel = pUI->Labels[jumpAddr];
			if (pLabel != nullptr)
				pLabel->References[pc]++;	// add/increment reference


			pNewCodeInfo->JumpAddress = jumpAddr;
			AnalyseFromPC(pUI, jumpAddr);
			bStop = false;	// should just be call & rst really
		}


		
		// do we need to stop tracing ??
		if (CheckStopInstruction(pUI->pSpeccy, pc) || newPC < pc)
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
	
	AnalyseFromPC(pUI, pc);
	
	//if (bRead)
	//	pUI->MemStats.ReadCount[addr]++;
	//if (bWrite)
	//	pUI->MemStats.WriteCount[addr]++;
	//
}

FLabelInfo* AddLabel(FSpeccyUI *pUI, uint16_t address,const char *name,LabelType type)
{
	FLabelInfo *pLabel = new FLabelInfo;
	pLabel->Name = name;
	pLabel->LabelType = type;
	pUI->Labels[address] = pLabel;
	return pLabel;
}

void InsertROMLabels(FSpeccyUI *pUI)
{
	for (const auto &label : g_RomLabels)
	{
		AddLabel(pUI, label.Address, label.pLabelName, label.LabelType);

		// run static analysis on all code labels
		if(label.LabelType == LabelType::Code || label.LabelType == LabelType::Function)
			RunStaticCodeAnalysis(pUI, label.Address);
	}
}

void InsertSystemLabels(FSpeccyUI *pUI)
{
	// screen memory start
	AddLabel(pUI, 0x4000, "ScreenPixels", LabelType::Data);
	
	FDataInfo *pScreenPixData = new FDataInfo;
	pScreenPixData->Address = 0x4000;
	pScreenPixData->ByteSize = 0x1800;
	pUI->DataInfo[pScreenPixData->Address] = pScreenPixData;

	AddLabel(pUI, 0x5800, "ScreenAttributes", LabelType::Data);
	FDataInfo *pScreenAttrData = new FDataInfo;
	pScreenAttrData->Address = 0x5800;
	pScreenAttrData->ByteSize = 0x400;
	pUI->DataInfo[pScreenAttrData->Address] = pScreenAttrData;

	// system variables?
}

void InitialiseCodeAnalysis(FSpeccyUI *pUI)
{
	uint16_t initialPC = z80_pc(&pUI->pSpeccy->CurrentState.cpu);
	InsertROMLabels(pUI);
	InsertSystemLabels(pUI);
	RunStaticCodeAnalysis(pUI, initialPC);
}

void DrawCodeAnalysisData(FSpeccyUI *pUI)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	ImGui::BeginChild("##analysis", ImVec2(0, 0), true);
	{
		// build item list - not every frame please!
		static std::vector< const FItem *> itemList;

		if (pUI->bCodeAnalysisDataDirty)
		{
			itemList.clear();

			// loop across address range
			for (int addr = 0; addr < (1<<16); addr++)
			{
				const FLabelInfo *pLabelInfo = pUI->Labels[addr];
				if (pLabelInfo != nullptr)
				{
					itemList.push_back(pLabelInfo);

				}
				const FCodeInfo *pCodeInfo = pUI->CodeInfo[addr];
				if (pCodeInfo != nullptr)
				{
					itemList.push_back(pCodeInfo);
				}
				const FDataInfo *pDataInfo = pUI->DataInfo[addr];
				if (pDataInfo != nullptr)
				{
					itemList.push_back(pDataInfo);
				}
			}

			pUI->bCodeAnalysisDataDirty = false;
		}

		// draw clipped list
		ImGuiListClipper clipper((int)itemList.size());
		const FItem *pPrevItem = nullptr;
		static const FItem *pSelectedItem = nullptr;

		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FItem* pItem = itemList[i];

				ImGui::PushID(i);

				if (pPrevItem != nullptr && pItem->Address > pPrevItem->Address + pPrevItem->ByteSize)
					ImGui::Separator();

				// selectable
				if(ImGui::Selectable("##codeanalysisline",pItem == pSelectedItem))
				{
					pSelectedItem = pItem;
				}
				ImGui::SameLine();

				if (pItem->Type == ItemType::Label)
				{
					const FLabelInfo *pLabelInfo = static_cast<const FLabelInfo *>(pItem);
					ImGui::Text("%s: ", pLabelInfo->Name.c_str());
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("Callers:");
						for (const auto & caller : pLabelInfo->References)
						{
							ImGui::Text("0x%04X", caller.first);
						}
						ImGui::EndTooltip();
					}
				}
				else if (pItem->Type == ItemType::Code)
				{
					const FCodeInfo *pCodeInfo = static_cast<const FCodeInfo *>(pItem);

					const int frameSinceAccessed = pUI->CurrentFrameNo - pCodeInfo->FrameLastAccessed;
					const int brightVal = (255 - std::min(frameSinceAccessed, 255)) & 0xff;

					const ImU32 col = 0xff000000 | (brightVal << 16) | (brightVal << 8) | (brightVal << 0);
					ImGui::PushStyleColor(ImGuiCol_Text, col);
					ImGui::Text(">> ");
					ImGui::SameLine();
					ImGui::PopStyleColor();

					ImGui::Text("0x%04X", pCodeInfo->Address);
					const float line_start_x = ImGui::GetCursorPosX();
					ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
					ImGui::Text("%s", pCodeInfo->Text.c_str());
					if (pCodeInfo->JumpAddress != 0)
					{
						const FLabelInfo *pLabelInfo = pUI->Labels[pCodeInfo->JumpAddress];
						if (pLabelInfo != nullptr)
						{
							ImGui::SameLine();
							ImGui::PushStyleColor(ImGuiCol_Text, 0xff808080);
							ImGui::Text("[%s]", pLabelInfo->Name.c_str());
							ImGui::PopStyleColor();
						}
					}
				}
				else if (pItem->Type == ItemType::Data)
				{
					const FDataInfo *pDataInfo = static_cast<const FDataInfo *>(pItem);

					ImGui::Text("\t0x%04X", pDataInfo->Address);
					const float line_start_x = ImGui::GetCursorPosX();
					ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
					ImGui::Text("%d Bytes", pDataInfo->ByteSize);
				}

				pPrevItem = pItem;

				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
}