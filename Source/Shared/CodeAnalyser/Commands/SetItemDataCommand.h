#pragma once
#include "CommandProcessor.h"
#include "../CodeAnalysisPage.h"
#include "../CodeAnalyser.h"

struct FItem;
class FCodeAnalysisState;

class FSetItemDataCommand : public FCommand
{
public:
	FSetItemDataCommand(const FCodeAnalysisItem& item) :Item(item) {}

	virtual void Do(FCodeAnalysisState& state) override;
	virtual void Undo(FCodeAnalysisState& state) override;
	virtual void FixupAddressRefs(FCodeAnalysisState& state) override;

	FCodeAnalysisItem Item;

	EDataType	oldDataType = EDataType::None;
	uint16_t	oldDataSize = 0;
};

class FSetDataItemDisplayTypeCommand : public FCommand
{
public:
	FSetDataItemDisplayTypeCommand(const FCodeAnalysisItem& item, EDataItemDisplayType displayType) :Item(item), DisplayType(displayType){}

	virtual void Do(FCodeAnalysisState& state) override;
	virtual void Undo(FCodeAnalysisState& state) override;
	virtual void FixupAddressRefs(FCodeAnalysisState& state) override;

	FCodeAnalysisItem Item;
	EDataItemDisplayType	DisplayType = EDataItemDisplayType::Unknown;
	EDataItemDisplayType	OldDisplayType = EDataItemDisplayType::Unknown;
};

class FSetItemCodeCommand : public FCommand
{
public:
	FSetItemCodeCommand(FAddressRef addr) :Addr(addr) {}

	virtual void Do(FCodeAnalysisState& state) override;
	virtual void Undo(FCodeAnalysisState& state) override;
	virtual void FixupAddressRefs(FCodeAnalysisState& state) override;

	FAddressRef	Addr;
};

