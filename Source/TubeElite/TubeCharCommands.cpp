#include "TubeCharCommands.h"

#include "TubeCommands.h"
#include "TubeElite.h"

#include "Debug/DebugLog.h"

// Char commands
// https://elite.bbcelite.com/deep_dives/6502sp_tube_communication.html

const uint8_t kCharCommand_DrawLines = 129;
const uint8_t kCharCommand_ShowEnergyBombEffect = 131;
const uint8_t kCharCommand_ShowHyperspaceColours = 132;
const uint8_t kCharCommand_SetCursorX = 133;
const uint8_t kCharCommand_SetCursorY = 134;
const uint8_t kCharCommand_ClearScreenBottom = 135;
const uint8_t kCharCommand_UpdateDashboard = 136;
const uint8_t kCharCommand_ShowHideDashboard = 138;	
const uint8_t kCharCommand_Set6522SystemVIAIER = 139;
const uint8_t kCharCommand_ToggleDashboardBulb = 140;
const uint8_t kCharCommand_SetDiscCatalogueFlag = 141;
const uint8_t kCharCommand_SetCurrentColour = 142;	
const uint8_t kCharCommand_ChangeColourPalette = 143;
const uint8_t kCharCommand_SetFileSavingFlag = 144;
const uint8_t kCharCommand_ExecuteBRKInstruction = 145;
const uint8_t kCharCommand_WriteToPrinterScreen = 146;
const uint8_t kCharCommand_BlankLineOnPrinter = 147;

class FUpdateDashboardCommand : public FTubeCommand
{
public:
	FUpdateDashboardCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}

	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 16)
		{
			// TODO: Unpack parameters and send to TubeSys
			bIsReady = true; // ready to execute
			return true;
		}

		return false;
	}

	void Execute(void) override
	{
	}

};

class FDrawLinesCommand : public FTubeCommand
{
public:
	FDrawLinesCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}
	bool ReceiveParamByte(uint8_t byte) override
	{
		ParamBytes.push_back(byte);
		if (ParamBytes.size() == 1)	// First byte
		{
			NoPoints = byte; // first byte is the number of points to draw
			if (NoPoints > 0)
			{
				ParamBytes.reserve(NoPoints * 2 + 1); // reserve space for points
			}
		}
		else if (ParamBytes.size() == NoPoints * 2 + 1)
		{
			bIsReady = true; // ready to execute
			return true;
		}
		return false;
	}
	void Execute(void) override
	{
		LOGINFO("Draw Lines command with %d parameters", ParamBytes.size());
		//pTubeSys->GetDisplay().DrawLines(ParamBytes.data(), ParamBytes.size());
		bIsComplete = true;
	}
private:
	uint8_t		NoPoints = 0;	// number of points to draw
};

class FSingleParamCommand : public FTubeCommand
{
public:
	FSingleParamCommand(FTubeElite* pSys) :FTubeCommand(pSys) {}

	bool ReceiveParamByte(uint8_t byte) override
	{
		Param = byte; // store the single parameter
		bIsReady = true; // ready to execute
		return true;
	}

protected:
	uint8_t Param = 0;	// single parameter
};

class FSetCursorXCommand : public FSingleParamCommand
{
public:
	FSetCursorXCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Set cursor X to %d", Param);
		pTubeSys->GetDisplay().SetCursorX(Param);
		bIsComplete = true;
	}
};

class FSetCursorYCommand : public FSingleParamCommand
{
public:
	FSetCursorYCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Set cursor Y to %d", Param);
		pTubeSys->GetDisplay().SetCursorY(Param);
		bIsComplete = true;
	}
};

FTubeCommand* CreateTubeCharCommand(FTubeElite* pSys, uint8_t commandId)
{
	FTubeCommand* pCommand = nullptr;
	switch (commandId)
	{
	case kCharCommand_DrawLines:
		pCommand = new FDrawLinesCommand(pSys);
		break;
	case kCharCommand_ShowEnergyBombEffect:
		LOGINFO("Tube char command: Show Energy Bomb Effect - not implemented");
		break;
	case kCharCommand_ShowHyperspaceColours:
		LOGINFO("Tube char command: Show Hyperspace Colours - not implemented");
		break;

	case kCharCommand_SetCursorX:
		pCommand = new FSetCursorXCommand(pSys);
		break;

	case kCharCommand_SetCursorY:
		pCommand = new FSetCursorYCommand(pSys);
		break;

	case kCharCommand_ClearScreenBottom:
		LOGINFO("Tube char command: Clear Screen Bottom - not implemented");
		break;

	case kCharCommand_UpdateDashboard:
		pCommand = new FUpdateDashboardCommand(pSys);
		break;

	case kCharCommand_ShowHideDashboard:
		LOGINFO("Tube char command: Show/Hide Dashboard - not implemented");
		break;
	case kCharCommand_Set6522SystemVIAIER:
		LOGINFO("Tube char command: Set 6522 System VIA IER - not implemented");
		break;
	case kCharCommand_ToggleDashboardBulb:
		LOGINFO("Tube char command: Toggle Dashboard Bulb - not implemented");
		break;
	case kCharCommand_SetDiscCatalogueFlag:
		LOGINFO("Tube char command: Set Disc Catalogue Flag - not implemented");
		break;
	case kCharCommand_SetCurrentColour:
		LOGINFO("Tube char command: Set Current Colour - not implemented");
		break;
	case kCharCommand_ChangeColourPalette:
		LOGINFO("Tube char command: Change Colour Palette - not implemented");
		break;
	case kCharCommand_SetFileSavingFlag:
		LOGINFO("Tube char command: Set File Saving Flag - not implemented");
		break;
	case kCharCommand_ExecuteBRKInstruction:
		LOGINFO("Tube char command: Execute BRK Instruction - not implemented");
		break;
	case kCharCommand_WriteToPrinterScreen:
		LOGINFO("Tube char command: Write to Printer Screen - not implemented");
		break;
	case kCharCommand_BlankLineOnPrinter:
		LOGINFO("Tube char command: Blank Line on Printer - not implemented");
		break;

	default:
		break;
	}
	return pCommand;
}