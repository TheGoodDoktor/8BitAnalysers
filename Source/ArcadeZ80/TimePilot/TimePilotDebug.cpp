#include "TimePilotDebug.h"
#include "TimePilotMachine.h"
#include "Util/GraphicsView.h"

#include <imgui.h>
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "ImGuiSupport/ImGuiScaling.h"

// Addresses

// ROM
static const uint16_t kStringTableAddr	= 0x0C50;

// RAM
static const uint16_t kCommandQueueAddr		= 0xAC00;
static const uint16_t kReadIndexAddr		= 0xA9B3;
static const uint16_t kWriteIndexAddr		= 0xA9B2;

static const uint16_t kProgramPhase			= 0xA9AC;
static const uint16_t kPhaseExecution		= 0x0F1F;

static const uint16_t kEnemyData			= 0xA850;


class FTimePilotDebug : public FMachineDebug
{
public:
	FTimePilotDebug(FArcadeZ80Machine* pTPMachine);
	void DrawDebugUI() override;
	void DrawDebugOverlays(float x, float y) override;
	void DrawSpriteDebug(ImVec2 pos);

private:
	void	SpriteViewer();
	void	StringViewer();
	void	GameStateViewer();
	void	DebugDrawCommandQueue();
	void	DebugDrawString(uint16_t stringAddress);

	FTimePilotMachine* pMachine = nullptr;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
	FGraphicsView* pSpriteView = nullptr;
	FGraphicsView* pStringView = nullptr;
	FGraphicsView* pCharacterView = nullptr;

	// Sprite Viewer
	struct FSpriteDebugState
	{
		int		SpriteNo = 0;
		int		SpriteColour = 0;
		bool	bFlipx = false;
		bool	bFlipy = false;
		bool	bRot = false;
	} SpriteDebugState;

	bool	bSpriteDebug = false;
};

FMachineDebug* CreateTimePilotDebug(FArcadeZ80Machine *pMachine)
{
	return new FTimePilotDebug(pMachine);
}

FTimePilotDebug::FTimePilotDebug(FArcadeZ80Machine* pTPMachine)
	: pMachine((FTimePilotMachine*)pTPMachine)
	, pCodeAnalysis(pMachine->pCodeAnalysis)
{
	pSpriteView = new FGraphicsView(64, 64);
	pStringView = new FGraphicsView(256, 256);
	pCharacterView = new FGraphicsView(128, 128);
}

void FTimePilotDebug::DrawDebugUI()
{

	// Create an ImGui tab bar for the debug UI and add tabs for different viewers
	if (ImGui::BeginTabBar("DebugTabs"))
	{
		if (ImGui::BeginTabItem("Sprite Viewer"))
		{
			SpriteViewer();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Command Queue"))
		{
			DebugDrawCommandQueue();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("String Viewer"))
		{
			StringViewer();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Game State"))
		{
			GameStateViewer();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	
}

void FTimePilotDebug::DrawDebugOverlays(float x, float y)
{
	const ImVec2 pos(x, y);
	
	ImGui::Checkbox("Sprite Debug", &bSpriteDebug);

	if (bSpriteDebug)
	{
		DrawSpriteDebug(pos);
	}
}

void FTimePilotDebug::DrawSpriteDebug(ImVec2 pos)
{
	const float scale = ImGui_GetScaling();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	const float sprSize = 16.0f * scale;
	auto pSpriteRAM = pMachine->pSpriteRAM;

	for (int offs = 0x3e; offs >= 0x10; offs -= 2)
	{
		int const sx = pSpriteRAM[0][offs];
		int const sy = 241 - pSpriteRAM[1][offs + 1];

		int const code = pSpriteRAM[0][offs + 1];
		int const color = pSpriteRAM[1][offs] & 0x3f;
		int const flipx = ~pSpriteRAM[1][offs] & 0x40;
		int const flipy = pSpriteRAM[1][offs] & 0x80;

		int sprNo = (offs - 0x10) / 2;
		char sprNoText[8];
		snprintf(sprNoText, 8, "%d", sprNo);

		if (pMachine->bRotateScreen)
		{
			ImVec2 scrPos(pos.x + (256 - sy - 16) * scale, pos.y + sx * scale);

			pDrawList->AddRect(
				scrPos,
				ImVec2(scrPos.x + sprSize, scrPos.y + sprSize),
				IM_COL32(255, 0, 0, 255));
			pDrawList->AddText(scrPos, 0xffffffff, sprNoText);
		}
		else
		{
			ImVec2 scrPos(pos.x + sx * scale, pos.y + sy * scale);

			pDrawList->AddRect(
				scrPos,
				ImVec2(pos.x + (sx + 16) * scale, pos.y + (sy + 16) * scale),
				IM_COL32(255, 0, 0, 255));

			pDrawList->AddText(scrPos, 0xffffffff, sprNoText);
		}
	}
}

void FTimePilotDebug::SpriteViewer()
{
	static bool bFirstRun = true;
	bool bRedrawSprite = false;

	bRedrawSprite |= ImGui::InputInt("SpriteNo", &SpriteDebugState.SpriteNo);
	//ImGui::SameLine();
	bRedrawSprite |= ImGui::InputInt("SpriteColour", &SpriteDebugState.SpriteColour);
	bRedrawSprite |= ImGui::Checkbox("Flip X", &SpriteDebugState.bFlipx);
	ImGui::SameLine();
	bRedrawSprite |= ImGui::Checkbox("Flip Y", &SpriteDebugState.bFlipy);
	ImGui::SameLine();
	bRedrawSprite |= ImGui::Checkbox("Rotate 90", &SpriteDebugState.bRot);

	if (bRedrawSprite || bFirstRun)
	{
		pSpriteView->Clear(0xff000000);
		const uint32_t* pSpriteColours = pMachine->SpriteColours[SpriteDebugState.SpriteColour];
		const int spriteByteSize = (4 * 16);
		const uint8_t* pSprite = &pMachine->SpriteROM[SpriteDebugState.SpriteNo * spriteByteSize];
		DrawSprite(pSpriteView, pSprite, 0, 0, pSpriteColours, SpriteDebugState.bFlipx, SpriteDebugState.bFlipy, SpriteDebugState.bRot);

		bFirstRun = false;
	}

	pSpriteView->Draw(true);

	if (ImGui::Button("Export Sprites"))
	{
		pMachine->ExportZXNSprites();
	}
}

void FTimePilotDebug::StringViewer()
{
	static bool bFirstRun = true;
	ImGui::Text("Output Strings");
	static int stringNo = 0;
	if (ImGui::InputInt("String No", &stringNo) || bFirstRun)
	{
		const uint16_t stringAddress = pCodeAnalysis->ReadWord(kStringTableAddr + (stringNo * 2));
		DebugDrawString(stringAddress);
	}
	ImGui::Text("%d (%02Xh)", stringNo, stringNo);
	pStringView->Draw();

	// Character Set Viewer
	static bool bCharSet2 = false;
	static int colourSet = 0;
	bool bUpdate = bFirstRun;
	ImGui::Separator();
	ImGui::Text("Character Set Viewer");
	bUpdate |= ImGui::InputInt("Colour Set", &colourSet);
	bUpdate |= ImGui::Checkbox("Character Set 2", &bCharSet2);
	// Draw Character Set
	if (bUpdate)
	{
		pCharacterView->Clear(0xff000000);

		for(int characterNo = 0; characterNo < 256; characterNo++)
		{
			bool bFlipX = false;
			bool bFlipY = false;
			const int x = (characterNo % 16);
			const int y = (characterNo / 16);

			const uint8_t* pCharacter = &pMachine->TilesROM[(characterNo + (bCharSet2 ? 256 : 0)) * 16];
			const uint32_t* pColours = pMachine->TileColours[colourSet];	

			DrawCharacter8x8(pCharacterView, pCharacter, x * 8, y * 8, pColours, bFlipX, bFlipY, false, false);
		}

	}

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	pCharacterView->Draw();

	// determine which character no is being hovered over
	if (ImGui::IsItemHovered())
	{
		ImVec2 mousePos = ImGui::GetIO().MousePos;
		int localX = (int)(mousePos.x - pos.x);
		int localY = (int)(mousePos.y - pos.y);
		int charX = localX / 8;
		int charY = localY / 8;
		int characterNo = charX + (charY * 16);
		ImGui::BeginTooltip();
		ImGui::Text("Character No: %d (%02Xh)", characterNo, characterNo);
		ImGui::EndTooltip();
		// draw a rectangle round the character
		pDrawList->AddRect(ImVec2(pos.x + charX * 8, pos.y + charY * 8), ImVec2(pos.x + (charX + 1) * 8, pos.y + (charY + 1) * 8), IM_COL32(255, 255, 0, 255));
	}

	if (ImGui::Button("Export Characters"))
	{
		pMachine->ExportZXNChars();
	}

	if (ImGui::Button("Export Palettes"))
	{
		pMachine->ExportZXNPalettes();
	}

	bFirstRun = false;
}

static const char* g_pProgramPhases[] = 
{
	"Phase 0",
	"Phase 1",
	"Game - Radial Clear",
	"Phase 3",
	"Title - Deposit Coin",
	"Game - Era Intro",
	"Title - Show Hi Scores",	// Game does this too - could it be a pause?
	"Game - Gameplay",
	"Phase 8",
	"Phase 9",
	"Phase A",
	"Phase B",
	"Phase C",
	"Phase D",
	"Phase E",
	"Phase F"
};

void FTimePilotDebug::GameStateViewer()
{
	const uint8_t phase = pCodeAnalysis->ReadByte(kProgramPhase);
	ImGui::Text("Program Phase %02X: %s", phase, g_pProgramPhases[phase]);
	ImGui::Text("Variable Address: ");
	DrawAddressLabel(*pCodeAnalysis,pCodeAnalysis->GetFocussedViewState(),pCodeAnalysis->AddressRefFromPhysicalAddress(kProgramPhase));
	ImGui::Text("Phase execution Function: "); 
	DrawAddressLabel(*pCodeAnalysis, pCodeAnalysis->GetFocussedViewState(), pCodeAnalysis->AddressRefFromPhysicalAddress(kPhaseExecution));
}

// Debug Command Queue
// Queue at 0xAC00
// Read index at 0xA9B3
// Write index at 0xA9B2



// string for all 15 commands
static const char* kCommandNames[] =
{
	"Command 00",
	"Output String",
	"Command 02",
	"Command 03",
	"Command 04",
	"Command 05",
	"Command 06",
	"Command 07",
	"Command 08",
	"Command 09",
	"Command 0A",
	"Command 0B",
	"Command 0C",
	"Command 0D",
	"Command 0E",
	"Command 0F"
};

void FTimePilotDebug::DebugDrawCommandQueue()
{
	FCodeAnalysisState& analysis = *pCodeAnalysis;


	uint16_t readIndex = analysis.ReadByte(kReadIndexAddr);
	uint8_t command = analysis.ReadByte(kCommandQueueAddr + readIndex);

	ImGui::Text("Command Queue:");

	int bailout = 0;
	while (command != 0xff)
	{
		uint8_t param = analysis.ReadByte(kCommandQueueAddr + readIndex + 1);

		ImGui::Text("Command: %s, Param: %02x", kCommandNames[command], param);

		// advance read index
		readIndex = (readIndex + 2) & 0x3f;
		command = analysis.ReadByte(kCommandQueueAddr + readIndex);

		bailout++;
		if (bailout > 32)
			break;
	}

}

// Draw one of the out string data sets, which contain:
// VideoRAM address
// Attribute
// String characters terminated by 0xB9
const uint8_t kCharAttrib_ColourMask	= 0x1f;
const uint8_t kCharAttrib_Foreground	= 0x10;
const uint8_t kCharAttrib_CharSet2		= 0x20;
const uint8_t kCharAttrib_FlipX			= 0x40;
const uint8_t kCharAttrib_FlipY			= 0x80;

void FTimePilotDebug::DebugDrawString(uint16_t stringAddress)
{
	const uint8_t kStringTerminator = 0xB9;

	pStringView->Clear(0xff000000);

	uint16_t videoRAMAddress = pCodeAnalysis->ReadWord(stringAddress);
	uint16_t videoRAMOffset = videoRAMAddress - 0xA400;
	int x = videoRAMOffset & 31;
	int y = videoRAMOffset / 32;

	stringAddress += 2;
	const uint8_t attr = pCodeAnalysis->ReadByte(stringAddress++);
	const uint8_t colour = attr & kCharAttrib_ColourMask;
	const bool bFlipX = (attr & kCharAttrib_FlipX) != 0;
	const bool bFlipY = (attr & kCharAttrib_FlipY) != 0;
	const bool bForeground = (attr & kCharAttrib_Foreground) != 0;
	const uint32_t* pColours = pMachine->TileColours[colour];

	while (true)
	{
		int ch = pCodeAnalysis->ReadByte(stringAddress++);
		if (ch == kStringTerminator)
			break;

		if (attr & 0x20)	// character set 2
			ch += 256;

		const uint8_t* pCharacter = &pMachine->TilesROM[ch * 16];
		DrawCharacter8x8(pStringView, pCharacter, x * 8, y * 8, pColours, bFlipX, bFlipY, false, false);
		y--;
	}

}