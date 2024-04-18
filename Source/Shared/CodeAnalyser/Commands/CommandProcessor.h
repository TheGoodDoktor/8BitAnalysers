#pragma once

class FCodeAnalysisState;


// Commands
class FCommand
{
public:
	virtual void Do(FCodeAnalysisState& state) = 0;
	virtual void Undo(FCodeAnalysisState& state) = 0;
	virtual void FixupAddressRefs(FCodeAnalysisState& state) = 0;
};

void DoCommand(FCodeAnalysisState& state, FCommand* pCommand);
void UndoCommand(FCodeAnalysisState& state);

