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
		LOGINFO("Update Dashboard");
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
			NoBytes = byte; // first byte is the number of points to draw
			if (NoBytes > 0)
			{
				ParamBytes.reserve(NoBytes); // reserve space for points
			}
		}
		else if (ParamBytes.size() == NoBytes)
		{
			bIsReady = true; // ready to execute
			return true;
		}
		return false;
	}
	void Execute(void) override
	{
		const int noOfPoints = ((int)ParamBytes.size() - 1) / 2; // number of points is (total bytes - 1) / 2
		const uint8_t* pPoints = ParamBytes.data() + 1; // skip the first byte which is the number of bytes
		for (int i = 0; i < noOfPoints / 2; i++)
		{
			uint8_t x1 = pPoints[i * 4];     // X1 coordinate
			uint8_t y1 = pPoints[i * 4 + 1]; // Y1 coordinate
			uint8_t x2 = pPoints[i * 4 + 2]; // X2 coordinate
			uint8_t y2 = pPoints[i * 4 + 3]; // Y2 coordinate
			pTubeSys->GetDisplay().AddLine(x1, y1, x2, y2); // add the line to the display
		}
		//LOGINFO("Draw Lines command with %d points", noOfPoints);
		//pTubeSys->GetDisplay().DrawLines(ParamBytes.data(), ParamBytes.size());

		bIsComplete = true;
	}
private:
	uint8_t		NoBytes = 0;	// number of bytes in the line heap

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

class FShowEnergyBombEffectCommand : public FSingleParamCommand
{
public:
	FShowEnergyBombEffectCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Show Energy Bomb Effect with parameter %d", Param);
		//pTubeSys->GetDisplay().ShowEnergyBombEffect(Param);
		bIsComplete = true;
	}
};

class FShowHyperspaceColoursCommand : public FSingleParamCommand
{
public:
	FShowHyperspaceColoursCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Show Hyperspace Colours with parameter %d", Param);
		//pTubeSys->GetDisplay().ShowHyperspaceColours(Param);
		bIsComplete = true;
	}
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

class FChangeColourPaletteCommand : public FSingleParamCommand
{
public:
	FChangeColourPaletteCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Change colour palette to %d", Param);
		//pTubeSys->GetDisplay().ChangeColourPalette(Param);
		bIsComplete = true;
	}
};

class FToggleDashboardBulbCommand : public FSingleParamCommand
{
public:
	FToggleDashboardBulbCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Toggle dashboard bulb with parameter %d", Param);
		//pTubeSys->GetDisplay().ToggleDashboardBulb(Param);
		bIsComplete = true;
	}
};

class FSetCurrentColourCommand : public FSingleParamCommand
{
public:
	FSetCurrentColourCommand(FTubeElite* pSys) :FSingleParamCommand(pSys) {}
	void Execute(void) override
	{
		LOGINFO("Set current colour to %d", Param);
		//pTubeSys->GetDisplay().SetCurrentColour(Param);
		bIsComplete = true;
	}
};

bool ProcessTubeCharCommand(FTubeElite* pSys, uint8_t commandId)
{
	switch (commandId)
	{
	case kCharCommand_DrawLines:
		pSys->SetTubeCharCommandHandler(new FDrawLinesCommand(pSys));
		//pSys->DebugBreak(); // break the execution
		return true; // command processed
	case kCharCommand_ShowEnergyBombEffect:
		pSys->SetTubeCharCommandHandler(new FShowEnergyBombEffectCommand(pSys));
		return true; // command processed
	case kCharCommand_ShowHyperspaceColours:
		pSys->SetTubeCharCommandHandler(new FShowHyperspaceColoursCommand(pSys));
		return true; // command processed

	case kCharCommand_SetCursorX:
		pSys->SetTubeCharCommandHandler(new FSetCursorXCommand(pSys));
		//pSys->DebugBreak(); // break the execution
		return true; // command processed

	case kCharCommand_SetCursorY:
		pSys->SetTubeCharCommandHandler(new FSetCursorYCommand(pSys));
		//pSys->DebugBreak(); // break the execution
		return true; // command processed

	case kCharCommand_ClearScreenBottom:
		LOGINFO("Tube char command: ClearScreenBottom");
		pSys->GetDisplay().ClearTextScreenFromRow(20, 0);
		pSys->GetDisplay().SetCursorY(20);
		return true; // command processed

	case kCharCommand_UpdateDashboard:
		pSys->SetTubeCharCommandHandler(new FUpdateDashboardCommand(pSys));
		return true; // command processed

	case kCharCommand_ShowHideDashboard:
		LOGINFO("Tube char command: Show/Hide Dashboard - not implemented");
		pSys->DebugBreak(); // break the execution
		break;
	case kCharCommand_Set6522SystemVIAIER:
		LOGINFO("Tube char command: Set 6522 System VIA IER - not implemented");
		pSys->DebugBreak(); // break the execution
		break;
	case kCharCommand_ToggleDashboardBulb:
		pSys->SetTubeCharCommandHandler(new FToggleDashboardBulbCommand(pSys));
		return true; // command processed
	case kCharCommand_SetDiscCatalogueFlag:
		LOGINFO("Tube char command: Set Disc Catalogue Flag - not implemented");
		pSys->DebugBreak(); // break the execution
		break;
	case kCharCommand_SetCurrentColour:
		pSys->SetTubeCharCommandHandler(new FSetCurrentColourCommand(pSys));
		return true; // command processed
	case kCharCommand_ChangeColourPalette:
		pSys->SetTubeCharCommandHandler(new FChangeColourPaletteCommand(pSys));
		return true; // command processed
	case kCharCommand_SetFileSavingFlag:
		LOGINFO("Tube char command: Set File Saving Flag - not implemented");
		pSys->DebugBreak(); // break the execution
		break;
	case kCharCommand_ExecuteBRKInstruction:
		LOGINFO("Tube char command: Execute BRK Instruction - not implemented");
		pSys->DebugBreak(); // break the execution
		break;
	case kCharCommand_WriteToPrinterScreen:
		LOGINFO("Tube char command: Write to Printer Screen - not implemented");
		pSys->DebugBreak(); // break the execution
		break;
	case kCharCommand_BlankLineOnPrinter:
		LOGINFO("Tube char command: Blank Line on Printer - not implemented");
		pSys->DebugBreak(); // break the execution
		break;

	default:
		break;
	}

	
	return false;
}