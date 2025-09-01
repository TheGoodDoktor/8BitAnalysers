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
		//LOGINFO("OSBYTE LO: A=0x%02X(%d), X=%d", ParamA,ParamA, ParamX);
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
		//LOGINFO("OSBYTE HI: A=0x%02X(%d), X=%d, Y=%d", ParamA, ParamA, ParamX, ParamY);
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

class FOSWORDCommand : public FTubeCommand
{
public:
	enum class EState
	{
		WaitingForAction,			// waiting for the OSWORD action byte
		WaitingForNumInputBytes,	// waiting for the number of input bytes
		ReceivingInputBytes,		// receiving input bytes
		WaitingForNumOutputBytes,	// waiting for the number of output bytes
		RunOSWORD,					// running the OSWORD command
		WritingOutputBytes			// writing output bytes
	};
	FOSWORDCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}
	~FOSWORDCommand()
	{
		// clean up input and output byte pointers if allocated
		delete[] ControlBlock.pInputBytes;
		delete[] ControlBlock.pOutputBytes;
	}

	bool ReceiveParamByte(uint8_t byte) override
	{
		switch (State)
		{
		case EState::WaitingForAction:
			ControlBlock.Action = byte; // OSWORD action
			State = EState::WaitingForNumInputBytes;
			break;
		case EState::WaitingForNumInputBytes:
			ControlBlock.NumInputBytes = byte; // number of input bytes
			if (ControlBlock.NumInputBytes == 0) // no input bytes
			{
				State = EState::WaitingForNumOutputBytes; // skip to output bytes
				return false; // not ready yet
			}
			ControlBlock.pInputBytes = new uint8_t[ControlBlock.NumInputBytes]; // allocate memory for input bytes
			InputByteIndex = ControlBlock.NumInputBytes - 1;	// write backwards
			State = EState::ReceivingInputBytes;			
			break;
		case EState::ReceivingInputBytes:
			if (InputByteIndex >= 0)
			{
				ControlBlock.pInputBytes[InputByteIndex--] = byte; // store the input byte
				if (InputByteIndex < 0) // all input bytes received
				{
					State = EState::WaitingForNumOutputBytes;
				}
			}
			break;
		case EState::WaitingForNumOutputBytes:
			ControlBlock.NumOutputBytes = byte; // number of output bytes
			if (ControlBlock.NumOutputBytes > 0)
			{
				ControlBlock.pOutputBytes = new uint8_t[ControlBlock.NumOutputBytes]; // allocate memory for output bytes
				// clear the output bytes
				for (int i = 0; i < ControlBlock.NumOutputBytes; ++i)
					ControlBlock.pOutputBytes[i] = 0;
				for(int i = 0;i<ControlBlock.NumInputBytes;i++)	// start of output block is input block
					ControlBlock.pOutputBytes[i] = ControlBlock.pInputBytes[i];
			}
			State = EState::RunOSWORD;
			bIsReady = true; // ready to execute
			return true;
			break;
		default:
			LOGERROR("Unhandled OSWORD state: %d", static_cast<int>(State));
			pTubeSys->DebugBreak(); // break the execution if we hit an unhandled state
			break;
		}
		
		return false;
	}

	void Execute() override
	{
		switch (State)
		{
		case EState::RunOSWORD:
		{
			// run the OSWORD command
			if(pTubeSys->GetDebug().bOSWORDDebug)
				LOGINFO("OSWORD: %d, inBytes: %d, outBytes: %d", ControlBlock.Action, ControlBlock.NumInputBytes, ControlBlock.NumOutputBytes);
			pTubeSys->OSWORD(ControlBlock);
			if (ControlBlock.NumOutputBytes == 0) // no output bytes
			{
				bIsComplete = true; // command complete
			}
			else
			{
				OutputByteIndex = ControlBlock.NumOutputBytes - 1;	// write backwards
				State = EState::WritingOutputBytes; // move to writing output bytes
			}
		}
		break;
		
		case EState::WritingOutputBytes:
		{
			// write the output bytes
			const uint8_t returnByte = ControlBlock.pOutputBytes[OutputByteIndex]; // get the output byte
			if (pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, returnByte))
			{
				if (--OutputByteIndex < 0) // all output bytes written
				{
					bIsComplete = true; // command complete
				}
			}
		}
			break;
		default:
			LOGERROR("Unhandled OSWORD state: %d", static_cast<int>(State));
			break;
		}
	}
private:
	EState		State = EState::WaitingForAction; // current state of the command

	FOSWORDControlBlock ControlBlock; // control block for OSWORD command

	int			InputByteIndex = 0; // index of the current input byte
	int			OutputByteIndex = 0; // index of the current output byte
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
		// ".1" gets sent to catalogue drive 1, ".2" for drive 2 etc.
		const uint8_t returnCode = pTubeSys->OSCLI(CommandLine.c_str()); // 0x80 make parasite run code - investigate
		pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, returnCode); // acknowledge the command
		bIsComplete = true;
	}
private:
	std::string		CommandLine;
};



class FOSFILECommand : public FTubeCommand
{
public:
	enum class EState
	{
		ReceivingControlBlock,
		ReceivingFilename,
		ReceivingTransferType,
		ExecuteCommand,
		ReturningStatus,
		ReturningControlBlock
	};
	FOSFILECommand(FTubeElite* pSys) :FTubeCommand(pSys) 
	{
		State = EState::ReceivingControlBlock;
	}

	bool ReceiveParamByte(uint8_t byte) override
	{
		switch (State)
		{
		case EState::ReceivingControlBlock:
			ControlBlockIndex--;
			ControlBlock.Bytes[ControlBlockIndex] = byte;
			if (ControlBlockIndex == 0)
			{
				// Control block received, now receive filename
				State = EState::ReceivingFilename;
			}
			break;
		case EState::ReceivingFilename:
			if (byte == 0x0D) // null terminator
			{
				State = EState::ReceivingTransferType;
			}
			else
			{
				Filename.push_back(static_cast<char>(byte));
			}
			break;
		case EState::ReceivingTransferType:
			TransferType = byte; // 0=load, 1=save, 2=verify
			State = EState::ExecuteCommand;
			bIsReady = true; // ready to execute
			return true;
			break;
		}

		return false;
	}

	void Execute(void) override
	{
		switch (State)
		{
			case EState::ExecuteCommand:
			{
				// Execute the OSFILE command
				ReturnStatus = pTubeSys->OSFILE(Filename.c_str(), ControlBlock, TransferType);
				State = EState::ReturningStatus;
			}
			break;
			case EState::ReturningStatus:
			{
				if (pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, ReturnStatus))
				{
					ControlBlockIndex = kOSFILEControlBlockSize - 1;
					State = EState::ReturningControlBlock;
				}
			}
			break;
			case EState::ReturningControlBlock:
			{
				const uint8_t returnByte = ControlBlock.Bytes[ControlBlockIndex]; // get the output byte
				if (pTubeSys->GetMachine().Tube.HostWriteRegister(ETubeRegister::R2, returnByte))
				{
					ControlBlockIndex--;
					if (ControlBlockIndex < 0) // all output bytes written
					{
						bIsComplete = true; // command complete
					}
				}
			}
			break;
		}
	}
private:
	EState				State;
	const static int	kOSFILEControlBlockSize = 16;
	FOSFILEControlBlock	ControlBlock;
	int					ControlBlockIndex = kOSFILEControlBlockSize;
	std::string			Filename;
	uint8_t				TransferType = 0; // 0=load, 1=save, 2=verify
	uint8_t 			ReturnStatus = 0;
};

// https://elite.bbcelite.com/deep_dives/6502sp_tube_communication.html

FTubeCommand* CreateTubeCommand(FTubeElite* pSys, uint8_t commandId)
{
	FTubeCommand* pCommand = nullptr;

	switch (commandId)
	{
	case 0x00:	//RDCH
		// Elite seems to use this for saving a game
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
	case 0x14:	// OSFILE
		// Elite uses OSFILE a lot for loading and saving
		pCommand = new FOSFILECommand(pSys);
		//pSys->DebugBreak(); // break here to investigate OSFILE usage
		//LOGINFO("Tube command: OSFILE - not implemented");
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

