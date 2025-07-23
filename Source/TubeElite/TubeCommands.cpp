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

class FOSBYTELowCommand : public FTubeCommand
{
public:
	FOSBYTELowCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}
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

	void Execute() override
	{
		LOGINFO("OSBYTE LO: A=%d, X=%d", ParamA, ParamX);
		bIsComplete = true;
	}
private:
	uint8_t		ParamA;
	uint8_t		ParamX;
};

class FOSBYTEHiCommand : public FTubeCommand
{
public:
	FOSBYTEHiCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}
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

	void Execute() override
	{
		LOGINFO("OSBYTE HI: A=%d, X=%d, Y=%d", ParamA, ParamX, ParamY);
		bIsComplete = true;
	}
private:
	uint8_t		ParamA;
	uint8_t		ParamX;
	uint8_t		ParamY;
};

class FOSWORDCommand : public FTubeCommand
{
public:
	FOSWORDCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}
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
	void Execute() override
	{
		LOGINFO("OSWORD: %d, inBytes: %d, outBytes: %d", Action, NumInputBytes, NumOutputBytes);
		pTubeSys->OSWORD(Action,ParamBytes.data() + 2, OutBytes);
		bIsComplete = true;
	}
private:
	uint8_t		Action = 0; 
	uint8_t		NumInputBytes = 0; 
	uint8_t		NumOutputBytes = 0; // number of bytes to return
	std::vector<uint8_t>	OutBytes;
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

// Char commands
// https://elite.bbcelite.com/deep_dives/6502sp_tube_communication.html

const uint8_t kCharCommand_DrawLines			= 129;
const uint8_t kCharCommand_SetCursorX			= 133;
const uint8_t kCharCommand_SetCursorY			= 134;
const uint8_t kCharCommand_ClearScreenBottom	= 135;
const uint8_t kCharCommand_RDPARAMS				= 136;

class FRDPARAMSCommand : public FTubeCommand
{
public:
	FRDPARAMSCommand(FTubeElite* pSys):FTubeCommand(pSys){}

	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 16)
		{
			// TODO: Unpack parameters and send to TubeSys
			return true;
		}

		return false;
	}

	void Execute(void) override
	{
	}

};

FTubeCommand* CreateTubeCharCommand(FTubeElite* pSys, uint8_t commandId)
{
	FTubeCommand* pCommand = nullptr;
	switch (commandId)
	{
	case kCharCommand_DrawLines:
		LOGINFO("Tube char command: Draw Lines - not implemented");
		break;
	case kCharCommand_RDPARAMS:
		pCommand = new FRDPARAMSCommand(pSys);
		break;
	default:
		break;
	}
	return pCommand;
}