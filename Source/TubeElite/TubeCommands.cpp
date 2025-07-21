#include "TubeCommands.h"

#include "TubeElite.h"
#include "Debug/DebugLog.h"

class FReadInputLineCommand : public FTubeCommand
{
public:
	FReadInputLineCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}

	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 5)
		{
			// Send ack 0x7f or escape 0xff
			pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, 0x7f); // acknowledge the command

			// Set up parameters
			MaxChar = ParamBytes[0];	// maximum character
			MinChar = ParamBytes[1];	// minimum character
			LineLength = ParamBytes[2];	// line length
			Address = (ParamBytes[3] << 8) | ParamBytes[4];	// address to read from
			bIsReady = true;
			return true;
		}

		return false;
	}

	void Execute()
	{
		uint8_t inputByte = 0;
		if (pTubeSys->PopInputByte(inputByte))
		{
			pTubeSys->GetDisplay().ProcessVDUChar(inputByte); // process the character for display

			if (inputByte >= MinChar && inputByte <= MaxChar)
			{
				pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, inputByte);
				
			}

			if (inputByte == 0x0D)	// <cr> pressed
			{
				pTubeSys->GetDisplay().ProcessVDUChar(0x0A); // process line feed
				pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, 0x0D); // send CR to acknowledge the end of input line
				bIsComplete = true; // command complete on CR
			}
		}
	}
private:

	uint8_t	MaxChar;
	uint8_t MinChar;
	uint8_t LineLength;
	uint16_t Address;
};

class FOSCLICommand : public FTubeCommand
{
public:
	FOSCLICommand(FTubeElite* pSys) :FTubeCommand(pSys) {}

	bool ReceiveParamByte(uint8_t byte) override
	{
		if (byte == 0x0D)
		{
			bIsReady = true;
		}
		else
		{
			CommandLine.push_back(byte);
		}

		return bIsReady; // return true when ready to execute
	}

	void Execute(void) override
	{
		LOGINFO("OSCLI: %s", CommandLine.c_str());
		const uint8_t returnCode = 0x00; // 0x80 make parasite run code - investigate
		pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, returnCode); // acknowledge the command
		bIsComplete = true;
	}
private:
	std::string		CommandLine;
};

FTubeCommand* CreateTubeCommand(FTubeElite* pSys, uint8_t commandId)
{
	FTubeCommand* pCommand = nullptr;

	switch (commandId)
	{
	case 0x02:
		pCommand = new FOSCLICommand(pSys);
		break;
	case 0x0A:
		pCommand = new FReadInputLineCommand(pSys);
		break;

	default:
		pCommand = nullptr;
		LOGWARNING("Unknown Tube command: 0x%02X", commandId);
		break;
	}

	return pCommand;
}