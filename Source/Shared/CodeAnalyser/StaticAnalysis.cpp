#include "StaticAnalysis.h"

#include <imgui.h>
#include "CodeAnalyser.h"
#include "UI/CodeAnalyserUI.h"

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

// generic checks for simple Items
class FSimpleChecks : public FStaticAnalysisCheck
{
public:
	FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) override
	{
		const EInstructionType instType = GetInstructionType(state, addrRef);

		switch (instType)
		{
			case EInstructionType::PortInput:
				return new FStaticAnalysisItem(addrRef, "Port Input");
			case EInstructionType::PortOutput:
				return new FStaticAnalysisItem(addrRef, "Port Output");
			case EInstructionType::ChangeInterruptMode:
				return new FStaticAnalysisItem(addrRef, "Interrupt Mode");
			case EInstructionType::JumpToPointer:
				return new FStaticAnalysisItem(addrRef, "Jump to Pointer");
			default:
				return nullptr;
		}
	}

};


bool FStaticAnalyser::Init(FCodeAnalysisState* pState)
{
	pCodeAnalysis = pState;

	Checks.push_back(new FMultByAddCheck);
	Checks.push_back(new FSimpleChecks);
	return true;
}

void FStaticAnalyser::Reset()
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
	FCodeAnalysisState& state = *pCodeAnalysis;
	const auto& banks = pCodeAnalysis->GetBanks();

	Reset();

	// iterate through all registered banks
	for (int bankNo = 0; bankNo < banks.size(); bankNo++)
	{
		const FCodeAnalysisBank& bank = banks[bankNo];

		if(bank.bMachineROM)
			continue;

		int addToSelfRun = 0;
		FAddressRef addToSelfRunStart;

		// iterate through all address in bank
		int addr = bank.GetMappedAddress();
		while (addr < bank.GetMappedAddress() + bank.GetSizeBytes())
		{
			FAddressRef addrRef(bank.Id,addr);
			const FCodeInfo* pCodeInfoItem = pCodeAnalysis->GetCodeInfoForAddress(addrRef);
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
	for (auto item : Items)
	{
		item->DrawUi(*pCodeAnalysis,pCodeAnalysis->GetFocussedViewState());
	}
}

void FStaticAnalysisItem::DrawUi(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	ImGui::Text("%s : ",Name.c_str());
	DrawAddressLabel(state, viewState, AddressRef);
}
