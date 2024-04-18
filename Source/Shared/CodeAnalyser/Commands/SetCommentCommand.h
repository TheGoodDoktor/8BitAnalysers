#pragma once
#include "CommandProcessor.h"
#include "../CodeAnalysisPage.h"
#include "../CodeAnalyser.h"

struct FItem;
class FCodeAnalysisState;

class FSetItemCommentCommand : public FCommand
{
public:
	FSetItemCommentCommand(const FCodeAnalysisItem& item, const char *pText) :Item(item), CommentText(pText) {}

	virtual void Do(FCodeAnalysisState& state) override;
	virtual void Undo(FCodeAnalysisState& state) override;
	virtual void FixupAddressRefs(FCodeAnalysisState& state) override;

	FCodeAnalysisItem		Item;
	std::string		CommentText;
	std::string		OldCommentText;
};