#pragma once

#include <cinttypes>

class FCodeAnalysisState;

class FIOAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	Shutdown();

	void	RegisterIORead(uint16_t IOAddress, uint8_t value);
	void	RegisterIOWrite(uint16_t IOAddress, uint8_t value);

	void	FrameTick(void);
	void	DrawUI(void);
private:
	FCodeAnalysisState* pCodeAnalysis = nullptr;

};