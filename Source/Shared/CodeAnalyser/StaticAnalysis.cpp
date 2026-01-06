#include "StaticAnalysis.h"

#include <imgui.h>
#include "CodeAnalyser.h"
#include "UI/CodeAnalyserUI.h"
#include "Misc/EmuBase.h"

// check for multiplies using adds
class FMultByAddCheck : public FStaticAnalysisCheck
{
public:

	void Reset() override 
	{
		Start = FAddressRef();
		AddRunLength = 0;
	}

	FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) override
	{
		const EInstructionType instType = GetInstructionType(state, addrRef);

		if (instType == EInstructionType::AddToSelf)
		{
			if (AddRunLength == 0)
				Start = addrRef;
			AddRunLength++;
		}
		else
		{
			if (AddRunLength > 0)
			{
				// Register item
				char itemName[32];
				snprintf(itemName, 32, "Multiply by %d", 1 << AddRunLength);
				AddRunLength = 0;
				return new FStaticAnalysisItem(Start, itemName);
			}
		}

		return nullptr;
	}
private:
	FAddressRef		Start;
	int				AddRunLength = 0;
};

// check for multiple function calls
class FFunctionCallCheck : public FStaticAnalysisCheck
{
public:
	void Reset() override
	{
		Start = FAddressRef();
		CallCount = 0;
	}
	FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) override
	{
		const EInstructionType instType = GetInstructionType(state, addrRef);
		if (instType == EInstructionType::FunctionCall)
		{
			if (CallCount == 0)
				Start = addrRef;
			CallCount++;
		}
		else
		{
			if (CallCount >= kCallCountThreshold)
			{
				CallCount = 0;
				return new FStaticAnalysisItem(Start, "Multiple Calls");
			}
			CallCount = 0;
		}
		return nullptr;
	}
private:
	const int		kCallCountThreshold = 4;
	FAddressRef		Start;
	int				CallCount = 0;
};

class FROMLabelReferenceCheck : public FStaticAnalysisCheck
{
public:
	FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) override
	{
		const FCodeInfo* pCodeInfoItem = state.GetCodeInfoForAddress(addrRef);
		if (pCodeInfoItem && (pCodeInfoItem->OperandType == EOperandType::JumpAddress || pCodeInfoItem->OperandType == EOperandType::Pointer))
		{
			const FCodeAnalysisBank* pBank = state.GetBank(pCodeInfoItem->OperandAddress.BankId);
			if (pBank != nullptr && pBank->bMachineROM)
			{
				return new FStaticAnalysisItem(addrRef, "ROM Label Ref");
			}
		}
		return nullptr;
	}
};

// generic checks for simple Items
class FSimpleChecks : public FStaticAnalysisCheck
{
public:
	FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) override
	{
		const EInstructionType instType = GetInstructionType(state, addrRef);

		switch (instType)	// myabe switch to a map?
		{
			case EInstructionType::PortInput:
				return new FStaticAnalysisItem(addrRef, "Port Input");
			case EInstructionType::PortOutput:
				return new FStaticAnalysisItem(addrRef, "Port Output");
			case EInstructionType::ChangeInterruptMode:
				return new FStaticAnalysisItem(addrRef, "Interrupt Mode");
			case EInstructionType::JumpToPointer:
				return new FStaticAnalysisItem(addrRef, "Jump to Pointer");
			case EInstructionType::Halt:
				return new FStaticAnalysisItem(addrRef, "Halt");
			case EInstructionType::EnableInterrupts:
				return new FStaticAnalysisItem(addrRef, "Enable Interrupts");
			case EInstructionType::DisableInterrupts:
				return new FStaticAnalysisItem(addrRef, "Disable Interrupts");
			case EInstructionType::SetStackPointer:
				return new FStaticAnalysisItem(addrRef, "Set Stack Pointer");
			default:
				return nullptr;
		}
	}

};


bool FStaticAnalyser::Init(void)
{
	for (auto check : Checks)
		delete check;
	Checks.clear();
	Checks.push_back(new FMultByAddCheck);
	Checks.push_back(new FFunctionCallCheck);
	Checks.push_back(new FROMLabelReferenceCheck);
	Checks.push_back(new FSimpleChecks);
	return true;
}

void FStaticAnalyser::Shutdown(void)
{
	// delete checks
	for (auto check : Checks)
		delete check;
	Checks.clear();
	// delete items
	for (auto item : Items)
		delete item;
	Items.clear();
}

void FStaticAnalyser::ResetForGame(void)
{
	ResetAnalysis();
}

void FStaticAnalyser::ResetAnalysis()
{
	// reset item list
	for (auto item : Items)
	{
		delete item;
	}

	Items.clear();

	// reset checks
	for (auto check : Checks)
	{
		check->Reset();
	}

}

bool FStaticAnalyser::RunAnalysis(void)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	const auto& banks = state.GetBanks();

	ResetAnalysis();

	// iterate through all registered banks
	for (int bankNo = 0; bankNo < banks.size(); bankNo++)
	{
		const FCodeAnalysisBank& bank = banks[bankNo];

		if(bank.bMachineROM)	// TODO: this should be optional
			continue;

		int addToSelfRun = 0;
		FAddressRef addToSelfRunStart;

		// iterate through all address in bank
		int addr = bank.GetMappedAddress();
		while (addr < bank.GetMappedAddress() + bank.GetSizeBytes())
		{
			FAddressRef addrRef(bank.Id,addr);
			const FCodeInfo* pCodeInfoItem = state.GetCodeInfoForAddress(addrRef);
			if (pCodeInfoItem)
			{
				for (auto check : Checks)	// perform each check
				{
					FStaticAnalysisItem* pItem = check->RunCheck(state,addrRef);
					if(pItem)
						Items.push_back(pItem);
				}

				addr += pCodeInfoItem->ByteSize;
			}
			else
			{
				addr++;
			}

		}
	}

	return true;
}

void FStaticAnalyser::DrawUI(void)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	if(ImGui::Button("RunAnalysis"))
	{
		RunAnalysis();
	}

	if (ImGui::BeginChild("StaticAnalysisList", ImVec2(0, 0), false))
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;

		const ImVec2 outer_size = ImVec2(0.0f, 0.0f);
		if (ImGui::BeginTable("StaticAnalysis", 2, flags, outer_size))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Item");
			ImGui::TableSetupColumn("Address");
			ImGui::TableHeadersRow();
			ImGuiListClipper clipper;
			clipper.Begin((int)Items.size());

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					const FStaticAnalysisItem* item = Items[i];

					if (item)
						item->DrawUI(state,viewState);
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
}

void FStaticAnalysisItem::DrawUI(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState) const
{
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", Name.c_str());
	ImGui::TableNextColumn();
	DrawAddressLabel(state, viewState, AddressRef);
}

