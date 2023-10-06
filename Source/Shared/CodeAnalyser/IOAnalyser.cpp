#include "IOAnalyser.h"


#include "Util/Misc.h"

#include <imgui.h>

#include "CodeAnalyser.h"
#include "UI/CodeAnalyserUI.h"



// IO Analyser

void FIOAnalyser::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;
}

void FIOAnalyser::Shutdown()
{

}

void FIOAnalyser::RegisterIORead(FAddressRef pc, uint16_t IOAddress, uint8_t value)
{
	//FIOAccessInfo& access = IOAccesses[IOAddress];
	//access.IOAddress = IOAddress;
	//access.ReadLocations.insert(pc);
}

void FIOAnalyser::RegisterIOWrite(FAddressRef pc, uint16_t IOAddress, uint8_t value)
{
	//FIOAccessInfo& access = IOAccesses[IOAddress];
	//access.IOAddress = IOAddress;
	//access.WriteLocations.insert(pc);
}

void FIOAnalyser::FrameTick(void)
{

}

void FIOAnalyser::DrawUI(void)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;

	// List
	ImGui::BeginChild("IOAnalyserList", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, windowFlags);

	for (int i=0; i< Devices.size();i++)
	{
		FIODevice* pDevice = Devices[i];
		if (ImGui::Selectable(pDevice->GetName(),SelectedDeviceIndex == i))
		{
			SelectedDeviceIndex = i;
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	// Details
	ImGui::BeginChild("IOAnalyserDetails", ImVec2(0, 0), false, windowFlags);

	//const auto ioDetailsIt = IOAccesses.find(SelectedIOAddress);

	if(SelectedDeviceIndex != -1)
	{
		FIODevice* pDevice = Devices[SelectedDeviceIndex];
		
	}
	ImGui::EndChild();

}

// AY-3-8910 Audio Chip - move


const char* g_AYRegNames[] =
{
	"CH A Period Fine",		// 0
	"CH A Period Coarse",	// 1
	"CH B Period Fine",		// 2
	"CH B Period Coarse",	// 3
	"CH C Period Fine",		// 4
	"CH C Period Coarse",	// 5
	"Noise Pitch",			// 6
	"Mixer",				// 7
	"CH A Volume",			// 8
	"CH B Volume",			// 9
	"CH C Volume",			// 10 (A)
	"Env Dur fine",			// 11 (B)
	"Env Dur coarse",		// 12 (C)
	"Env Shape",			// 13 (D)
	"I/O Port A",			// 14 (E)
	"I/O Port B",			// 15 (F)
};

FAYAudioDevice::FAYAudioDevice()
{
	Name = "AYAudio";
}


void	FAYAudioDevice::WriteAYRegister(FAddressRef pc, uint8_t value)
{
	if (SelectedAYRegister == 255)
		return;

	AYRegisters[SelectedAYRegister] = value;
}


void DrawAYStateUI(const ay38910_t* ay)
{
	ImGui::Columns(4, "##ay_channels", false);
	ImGui::SetColumnWidth(0, 96);
	ImGui::SetColumnWidth(1, 40);
	ImGui::SetColumnWidth(2, 40);
	ImGui::SetColumnWidth(3, 40);
	ImGui::NextColumn();
	ImGui::Text("ChnA"); ImGui::NextColumn();
	ImGui::Text("ChnB"); ImGui::NextColumn();
	ImGui::Text("ChnC"); ImGui::NextColumn();
	ImGui::Separator();
	ImGui::Text("Tone Period"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		ImGui::Text("%04X", ay->tone[i].period); ImGui::NextColumn();
	}
	ImGui::Text("Tone Count"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		ImGui::Text("%04X", ay->tone[i].counter); ImGui::NextColumn();
	}
	ImGui::Text("Tone Bit"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		ImGui::Text("%s", ay->tone[i].bit ? "ON" : "OFF"); ImGui::NextColumn();
	}
	ImGui::Text("Volume"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		ImGui::Text("%X", ay->reg[AY38910_REG_AMP_A + i] & 0x0F); ImGui::NextColumn();
	}
	ImGui::Text("Ampl Ctrl"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		const uint8_t v = ay->reg[AY38910_REG_AMP_A + i] & 0x10;
		ImGui::Text("%s", v ? "ENV" : "VOL"); ImGui::NextColumn();
	}
	ImGui::Text("Mix Tone"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		const uint8_t v = (ay->enable >> i) & 1;
		ImGui::Text("%s", v ? "OFF" : "ON"); ImGui::NextColumn();
	}
	ImGui::Text("Mix Noise"); ImGui::NextColumn();
	for (int i = 0; i < 3; i++) {
		const uint8_t v = (ay->enable >> (i + 3)) & 1;
		ImGui::Text("%s", v ? "OFF" : "ON"); ImGui::NextColumn();
	}
	ImGui::Columns();
	ImGui::Separator();
	ImGui::Text("Noise Period  %02X (reg:%02X)", ay->noise.period, ay->reg[AY38910_REG_PERIOD_NOISE]);
	ImGui::Text("Noise Count   %02X", ay->noise.counter);
	ImGui::Text("Noise Rand    %05X", ay->noise.rng);
	ImGui::Text("Noise Bit     %s", ay->noise.bit ? "ON" : "OFF");
	ImGui::Separator();
	ImGui::Text("Env Period    %04X (reg:%04X)", ay->env.period, (ay->reg[AY38910_REG_ENV_PERIOD_COARSE] << 8) | ay->reg[AY38910_REG_ENV_PERIOD_FINE]);
	ImGui::Text("Env Count     %04X", ay->env.counter);
	ImGui::Text("Env Ampl      %02X", ay->env.shape_state);
	ImGui::Separator();
	const int num_ports = (ay->type == AY38910_TYPE_8910) ? 2 : ((ay->type == AY38910_TYPE_8912) ? 1 : 0);
	const int max_ports = 2;
	ImGui::Columns(max_ports + 1, "##ay_ports", false);
	ImGui::SetColumnWidth(0, 96);
	ImGui::SetColumnWidth(1, 60);
	ImGui::SetColumnWidth(2, 60);
	ImGui::NextColumn();
	ImGui::Text("PortA"); ImGui::NextColumn();
	ImGui::Text("PortB"); ImGui::NextColumn();
	ImGui::Text("In/Out Dir"); ImGui::NextColumn();
	int i;
	for (i = 0; i < num_ports; i++) {
		const uint8_t v = (ay->enable >> (i + 6)) & 1;
		ImGui::Text("%s", v ? "OUT" : "IN"); ImGui::NextColumn();
	}
	for (; i < max_ports; i++) {
		ImGui::Text("-"); ImGui::NextColumn();
	}
	ImGui::Text("Data Store"); ImGui::NextColumn();
	for (i = 0; i < num_ports; i++) {
		ImGui::Text("%02X", (i == 0) ? ay->port_a : ay->port_b); ImGui::NextColumn();
	}
	for (; i < max_ports; i++) {
		ImGui::Text("-"); ImGui::NextColumn();
	}
}
void FAYAudioDevice::DrawDetailsUI()
{
	// TODO: Tabbed view - log, current state
	if(pAYEmulator)
		DrawAYStateUI(pAYEmulator);
}
