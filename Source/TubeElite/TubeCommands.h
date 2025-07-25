#pragma once

#include <vector>
#include <cstdint>

class FTubeElite;

class FTubeCommand
{
public:
	FTubeCommand(FTubeElite* pSys) : pTubeSys(pSys)
	{
		ParamBytes.reserve(16); // reserve space for parameters
	}
    
    virtual ~FTubeCommand(){}

	bool IsReady() const { return bIsReady; }
	bool IsComplete() const { return bIsComplete; }

	virtual bool ReceiveParamByte(uint8_t byte) = 0;	// returns if all params received
	virtual void Execute() = 0;	// execute the command

	std::vector<uint8_t> ParamBytes;	// received parameters
	FTubeElite* pTubeSys = nullptr;	// pointer to Tube Elite system
	bool bIsReady = false;
	bool bIsComplete = false;
};

FTubeCommand* CreateTubeCommand(FTubeElite* pSys, uint8_t commandId);

// Char Commands

bool ProcessTubeCharCommand(FTubeElite* pSys, uint8_t commandId);
