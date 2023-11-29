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
		FCommand* pCommand = state.CommandStack.back();
		pCommand->Undo(state);
		state.CommandStack.pop_back();

		// Delete command or do we want 'redo'?
		
	}
}
