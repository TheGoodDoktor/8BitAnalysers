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

	void Execute() override
	{
		uint8_t inputByte = 0;
		if (pTubeSys->GetInputByte(inputByte))
		{
			if ((inputByte >= MinChar && inputByte <= MaxChar) || inputByte == 0x0D)
			{
				if (pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, inputByte))
				{
					if (inputByte == 0x0D)	// <cr> pressed
					{
						pTubeSys->GetDisplay().ProcessVDUChar(0x0A); // process line feed
						bIsComplete = true; // command complete on CR
					}

					pTubeSys->GetDisplay().ProcessVDUChar(inputByte); // process the character for display
					pTubeSys->PopInputByte(); // remove the byte from the input buffer
				}
			}
		}
	}
private:

	uint8_t	MaxChar;
	uint8_t MinChar;
	uint8_t LineLength;
	uint16_t Address;
};

class FR2Command : public FTubeCommand
{
public:
	FR2Command(FTubeElite* pSys) :FTubeCommand(pSys) {}

	virtual void RunCommand(std::deque<uint8_t>& returnBytes) = 0;

	void Execute() override
	{
		if (bReturningData == false)
		{
			RunCommand(ReturnBytes);
			if (ReturnBytes.empty())
			{
				bIsComplete = true; // command complete if no return bytes
			}
			else
			{
				bReturningData = true; // we have data to return
			}
		}
		else
		{
			uint8_t returnByte = ReturnBytes.front(); // get the first byte from the return bytes
			if (pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, returnByte))
			{
				ReturnBytes.pop_front(); // remove the byte from the return bytes
			}

			if (ReturnBytes.empty())
				bIsComplete = true;
		}
	}
private:
	bool bReturningData = false; // true if returning data
	std::deque<uint8_t>	ReturnBytes;
};

class FOSBYTELowCommand : public FR2Command
{
public:
	FOSBYTELowCommand(FTubeElite* pSys) :FR2Command(pSys) {}
	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 2)
		{
			ParamX = ParamBytes[0];	// OSBYTE parameter X
			ParamA = ParamBytes[1];	// OSBYTE parameter A
			bIsReady = true;
			return true; // ready to execute
		}
		return false;
	}

	void RunCommand(std::deque<uint8_t>& returnBytes) override
	{
		LOGINFO("OSBYTE LO: A=0x%02X(%d), X=%d", ParamA,ParamA, ParamX);
		returnBytes.push_back(pTubeSys->OSBYTE(ParamA, ParamX)); // call OSBYTE with parameters
	}
private:
	uint8_t		ParamA;
	uint8_t		ParamX;
};

class FOSBYTEHiCommand : public FR2Command
{
public:
	FOSBYTEHiCommand(FTubeElite* pSys) :FR2Command(pSys) {}
	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 3)
		{
			ParamX = ParamBytes[0];	// OSBYTE parameter X
			ParamY = ParamBytes[1];	// OSBYTE parameter Y
			ParamA = ParamBytes[2];	// OSBYTE parameter A
			bIsReady = true;
			return true; // ready to execute
		}
		return false;
	}

	void RunCommand(std::deque<uint8_t>& returnBytes) override
	{
		LOGINFO("OSBYTE HI: A=0x%02X(%d), X=%d, Y=%d", ParamA, ParamA, ParamX, ParamY);
		uint8_t	retBytes[3] = {0,0,0};
		pTubeSys->OSBYTE(ParamA, ParamX, ParamY, retBytes); // call OSBYTE with parameters

		// push the return bytes to the deque
		returnBytes.push_back(retBytes[0]);
		returnBytes.push_back(retBytes[1]);
		returnBytes.push_back(retBytes[2]);
	}
private:
	uint8_t		ParamA;
	uint8_t		ParamX;
	uint8_t		ParamY;
};

class FOSWORDCommand : public FR2Command
{
public:
	FOSWORDCommand(FTubeElite* pSys) :FR2Command(pSys) {}
	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 2)
		{
			Action = ParamBytes[0];	// OSWORD parameter Y
			NumInputBytes = ParamBytes[1];	// OSWORD parameter X

			//bIsReady = true;
			//return true; // ready to execute
		}
		else if (ParamBytes.size() == 2 + NumInputBytes + 1)
		{
			NumOutputBytes = ParamBytes.back(); // last byte is the number of output bytes
			bIsReady = true; // ready to execute
			return true;
		}
		return false;
	}
	void RunCommand(std::deque<uint8_t>& returnBytes) override
	{
		LOGINFO("OSWORD: %d, inBytes: %d, outBytes: %d", Action, NumInputBytes, NumOutputBytes);
		pTubeSys->OSWORD(Action,ParamBytes.data() + 2, returnBytes);
	}
private:
	uint8_t		Action = 0; 
	uint8_t		NumInputBytes = 0; 
	uint8_t		NumOutputBytes = 0; // number of bytes to return
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

// https://elite.bbcelite.com/deep_dives/6502sp_tube_communication.html

FTubeCommand* CreateTubeCommand(FTubeElite* pSys, uint8_t commandId)
{
	FTubeCommand* pCommand = nullptr;

	switch (commandId)
	{
	case 0x00:	//RDCH
		LOGINFO("Tube command: RDCH (Read Character) - not implemented");
		break;
	case 0x02: //CLI
		pCommand = new FOSCLICommand(pSys);
		break;
	case 0x04: // OSBYTE LO - &00-&7F
		pCommand = new FOSBYTELowCommand(pSys);
		break;
	case 0x06: // OSBYTE HI - &80-&FF
		pCommand = new FOSBYTEHiCommand(pSys);
		break;
	case 0x08:	// OSWORD
		pCommand = new FOSWORDCommand(pSys);
		break;
	case 0x0A:	// OSWORD 0
		pCommand = new FReadInputLineCommand(pSys);
		break;
	case 0x0C:	// OSARGS
		LOGINFO("Tube command: OSARGS - not implemented");
		break;
	case 0x0E:	// BGET
		LOGINFO("Tube command: BGET (Byte Get) - not implemented");
		break;
	case 0x10:	// BPUT
		LOGINFO("Tube command: BPUT (Byte Put) - not implemented");
		break;
	case 0x12:	// OSFIND
		LOGINFO("Tube command: OSFIND - not implemented");
		break;
	case 0x014:	// OSFILE
		LOGINFO("Tube command: OSFILE - not implemented");
		break;
	case 0x16:	// OSGBPB
		LOGINFO("Tube command: OSGBPB - not implemented");
		break;
	default:
		LOGWARNING("Unknown Tube command: 0x%02X", commandId);
		break;
	}

	return pCommand;
}

