#pragma once

class FCodeAnalysisState;

class FMemoryAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	FrameTick(void);
	void	DrawUI(void);

private:
	FCodeAnalysisState* pCodeAnalysis = nullptr;
};