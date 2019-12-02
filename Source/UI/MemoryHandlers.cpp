#include "MemoryHandlers.h"
#include <cstdint>
#include "SpeccyUI.h"

// Disassembly handlers
static uint8_t DasmCB(void* user_data)
{
	FSpeccyUI *pUI = (FSpeccyUI *)user_data;
	return ReadySpeccyByte(*pUI->pSpeccy, pUI->dasmCurr++);
}

static uint16_t DisasmLen(FSpeccyUI *pUI, uint16_t pc)
{
	pUI->dasmCurr = pc;
	uint16_t next_pc = z80dasm_op(pc, DasmCB, 0, pUI);
	return next_pc - pc;
}

int MemoryHandlerTrapFunction(uint16_t pc, int ticks, uint64_t pins, FSpeccyUI *pUI)
{
	const uint16_t addr = Z80_GET_ADDR(pins);
	const bool bRead = (pins & Z80_CTRL_MASK) == (Z80_MREQ | Z80_RD);
	const bool bWrite = (pins & Z80_CTRL_MASK) == (Z80_MREQ | Z80_WR);


	// increment counters
	pUI->MemStats.ExecCount[pc]++;
	const int op_len = DisasmLen(pUI, pc);
	for (int i = 1; i < op_len; i++) {
		pUI->MemStats.ExecCount[(pc + i) & 0xFFFF]++;
	}

	if (bRead)
		pUI->MemStats.ReadCount[addr]++;
	if (bWrite)
		pUI->MemStats.WriteCount[addr]++;

	// See if we can find a handler
	for (auto& handler : pUI->MemoryAccessHandlers)
	{
		if (handler.bEnabled == false)
			continue;

		bool bCallHandler = false;

		if (handler.Type == MemoryAccessType::Execute)	// Execution
		{
			if (pc >= handler.MemStart && pc <= handler.MemEnd)
				bCallHandler = true;
		}
		else // Memory access
		{
			if (addr >= handler.MemStart && addr <= handler.MemEnd)
			{
				bool bExecute = false;

				if (handler.Type == MemoryAccessType::Read && bRead)
					bCallHandler = true;
				else if (handler.Type == MemoryAccessType::Write && bWrite)
					bCallHandler = true;
			}
		}

		if (bCallHandler)
		{
			// update handler stats
			handler.TotalCount++;
			handler.CallerCounts[pc]++;
			handler.AddressCounts[addr]++;
			if (handler.pHandlerFunction != nullptr)
				handler.pHandlerFunction(handler, pUI->pActiveGame, pc, pins);

			if (handler.bBreak)
				return UI_DBG_STEP_TRAPID;
		}
	}
	
	assert(!(bRead == true && bWrite == true));

	return 0;
}


void AddMemoryHandler(FSpeccyUI *pUI, const FMemoryAccessHandler &handler)
{
	pUI->MemoryAccessHandlers.push_back(handler);
}

MemoryUse DetermineAddressMemoryUse(const FMemoryStats &memStats, uint16_t addr, bool &smc)
{
	const bool bCode = memStats.ExecCount[addr] > 0;
	const bool bData = memStats.ReadCount[addr] > 0 || memStats.WriteCount[addr] > 0;

	if (bCode && memStats.WriteCount[addr] > 0)
	{
		smc = true;
	}

	if (bCode)
		return MemoryUse::Code;
	if (bData)
		return MemoryUse::Data;

	return MemoryUse::Unknown;
}

void AnalyseMemory(FMemoryStats &memStats)
{
	FMemoryBlock currentBlock;
	bool bSelfModifiedCode = false;

	memStats.MemoryBlockInfo.clear();	// Clear old list
	memStats.CodeAndDataList.clear();

	currentBlock.StartAddress = 0;
	currentBlock.Use = DetermineAddressMemoryUse(memStats, 0, bSelfModifiedCode);
	if (bSelfModifiedCode)
		memStats.CodeAndDataList.push_back(0);

	for (int addr = 1; addr < 65536; addr++)
	{
		bSelfModifiedCode = false;
		const MemoryUse addrUse = DetermineAddressMemoryUse(memStats, addr, bSelfModifiedCode);
		if (bSelfModifiedCode)
			memStats.CodeAndDataList.push_back(addr);
		if (addrUse != currentBlock.Use)
		{
			currentBlock.EndAddress = addr - 1;
			memStats.MemoryBlockInfo.push_back(currentBlock);

			// start new block
			currentBlock.StartAddress = addr;
			currentBlock.Use = addrUse;
		}
	}

	// finish off last block
	currentBlock.EndAddress = 0xffff;
	memStats.MemoryBlockInfo.push_back(currentBlock);
}

void ResetMemoryStats(FMemoryStats &memStats)
{
	memStats.MemoryBlockInfo.clear();	// Clear list
	// 
	// reset counters
	memset(memStats.ExecCount, 0, sizeof(memStats.ExecCount));
	memset(memStats.ReadCount, 0, sizeof(memStats.ReadCount));
	memset(memStats.WriteCount, 0, sizeof(memStats.WriteCount));
}


// UI
void DrawMemoryHandlers(FSpeccyUI* pUI)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("DrawMemoryHandlersGUIChild1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, window_flags);
	FMemoryAccessHandler *pSelectedHandler = nullptr;

	for (auto &handler : pUI->MemoryAccessHandlers)
	{
		const bool bSelected = pUI->SelectedMemoryHandler == handler.Name;
		if (bSelected)
		{
			pSelectedHandler = &handler;
		}

		if (ImGui::Selectable(handler.Name.c_str(), bSelected))
		{
			pUI->SelectedMemoryHandler = handler.Name;
		}

	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Handler details
	ImGui::BeginChild("DrawMemoryHandlersGUIChild2", ImVec2(0, 0), false, window_flags);
	if (pSelectedHandler != nullptr)
	{
		ImGui::Checkbox("Enabled", &pSelectedHandler->bEnabled);
		ImGui::Checkbox("Break", &pSelectedHandler->bBreak);
		ImGui::Text(pSelectedHandler->Name.c_str());
		ImGui::Text("0x%x - 0x%x", pSelectedHandler->MemStart, pSelectedHandler->MemEnd);
		ImGui::Text("Total Accesses %d", pSelectedHandler->TotalCount);

		ImGui::Text("Callers");
		for (const auto &accessPC : pSelectedHandler->CallerCounts)
		{
			ImGui::PushID(accessPC.first);
			ImGui::Text("0x%x - %d accesses", accessPC.first, accessPC.second);
			ImGui::SameLine();
			if (ImGui::ArrowButton("GotoAddr", ImGuiDir_Right))
			{
				pUI->UIZX.dasm[0].start_addr = accessPC.first;
			}
			ImGui::PopID();
		}
	}

	ImGui::EndChild();
}

void DrawMemoryAnalysis(FSpeccyUI* pUI)
{

	ImGui::Text("Memory Analysis");
	if (ImGui::Button("Analyse"))
	{
		AnalyseMemory(pUI->MemStats);	// non-const on purpose
	}
	const FMemoryStats& memStats = pUI->MemStats;
	ImGui::Text("%d self modified code points", (int)memStats.CodeAndDataList.size());
	ImGui::Text("%d blocks", (int)memStats.MemoryBlockInfo.size());
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("DrawMemoryAnalysisChild1", ImVec2(0, 0), false, window_flags);
	for (const auto &memblock : memStats.MemoryBlockInfo)
	{
		const char *pTypeStr = "Unknown";
		if (memblock.Use == MemoryUse::Code)
			pTypeStr = "Code";
		else if (memblock.Use == MemoryUse::Data)
			pTypeStr = "Data";
		ImGui::Text("%s", pTypeStr);
		ImGui::SameLine(100);
		ImGui::Text("0x%x - 0x%x", memblock.StartAddress, memblock.EndAddress);
	}
	ImGui::EndChild();

}

