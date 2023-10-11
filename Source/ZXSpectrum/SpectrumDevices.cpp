#include "SpectrumDevices.h"

#include <imgui.h>
#include <ui/ui_kbd.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include "CodeAnalyser/CodeAnalyser.h"


FSpectrumKeyboard::FSpectrumKeyboard()
{
    Name = "Keyboard";
}


bool	FSpectrumKeyboard::Init(kbd_t* kbd)
{
	pKeyboard = kbd;

   

    return true;
}

void FSpectrumKeyboard::RegisterKeyboardRead(FAddressRef pc, uint16_t ioAddress, uint8_t value)
{

}

void FSpectrumKeyboard::DrawDetailsUI()
{
  

}


// Beeper
FSpectrumBeeper::FSpectrumBeeper()
{
    Name = "Beeper";
}

bool FSpectrumBeeper::Init(beeper_t* beep)
{
    pBeeper = beep;
    return true;
}

void FSpectrumBeeper::DrawDetailsUI()
{
    ImGui::Text("Access Locations");
    for (const auto& location : AccessLocations)
    {
		ShowCodeAccessorActivity(*pCodeAnalyser, location);
        
        ImGui::Text("");
        ImGui::SameLine(30);
        DrawCodeAddress(*pCodeAnalyser,pCodeAnalyser->GetFocussedViewState(),location);
    }
}

void FSpectrumBeeper::RegisterBeeperWrite(FAddressRef pc, uint8_t value)
{
	AccessLocations.insert(pc);
}


// 128k Memory control
FSpectrum128MemoryCtrl::FSpectrum128MemoryCtrl()
{
    Name = "Memory Control";

}

void FSpectrum128MemoryCtrl::DrawDetailsUI()
{
	ImGui::Text("Access Locations");
	for (const auto& location : AccessLocations)
	{
		ShowCodeAccessorActivity(*pCodeAnalyser, location);

		ImGui::Text("");
		ImGui::SameLine(30);
		DrawCodeAddress(*pCodeAnalyser, pCodeAnalyser->GetFocussedViewState(), location);
	}
}

void FSpectrum128MemoryCtrl::RegisterMemoryConfigWrite(FAddressRef pc, uint8_t value)
{
    const int ramBank = value & 0x7;
    const int romBank = (value & (1 << 4)) ? 1 : 0;
    const int displayRamBank = (value & (1 << 3)) ? 7 : 5;

    CurrentConfig = value;

    AccessLocations.insert(pc);
}