#include "CommandProcessor.h"
#include "../CodeAnalyser.h"


// Command Processing
void DoCommand(FCodeAnalysisState& state, FCommand* pCommand)
{
	state.CommandStack.push_back(pCommand);
	pCommand->Do(state);
}

void UndoCommand(FCodeAnalysisState& state)
{
	if (state.CommandStack.empty() == false)
	{
		state.CommandStack.back()->Undo(state);
		state.CommandStack.pop_back();
	}
}
