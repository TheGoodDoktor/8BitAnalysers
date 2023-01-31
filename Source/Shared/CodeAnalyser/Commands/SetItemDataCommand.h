#pragma once
#include "CommandProcessor.h"
#include "../CodeAnaysisPage.h"

struct FItem;
struct FCodeAnalysisState;

class FSetItemDataCommand : public FCommand
{
public:
	FSetItemDataCommand(FItem* _pItem) :pItem(_pItem) {}

	virtual void Do(FCodeAnalysisState& state) override;
	virtual void Undo(FCodeAnalysisState& state) override;

	FItem* pItem;

	EDataType	oldDataType;
	uint16_t	oldDataSize;
};

class FSetItemCodeCommand : public FCommand
{
public:
	FSetItemCodeCommand(uint16_t addr) :Addr(addr) {}

	virtual void Do(FCodeAnalysisState& state) override;
	virtual void Undo(FCodeAnalysisState& state) override;

	uint16_t	Addr;
};

