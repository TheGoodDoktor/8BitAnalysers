#include "SetCommentCommand.h"

#include "CodeAnalyser/CodeAnalyser.h"

void FSetItemCommentCommand::Do(FCodeAnalysisState& state)
{
	OldCommentText = Item.Item->Comment;
	Item.Item->Comment = CommentText;
}
 
void FSetItemCommentCommand::Undo(FCodeAnalysisState& state)
{
	Item.Item->Comment = OldCommentText;
}

void FSetItemCommentCommand::FixupAddressRefs(FCodeAnalysisState& state)
{
	FixupAddressRef(state, Item.AddressRef);
}
