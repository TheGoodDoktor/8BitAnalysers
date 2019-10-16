#include "Speccy.h"

#define CHIPS_IMPL
#define UI_DBG_USE_Z80
#define UI_DASM_USE_Z80
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "util/z80dasm.h"
#include "chips/mem.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "systems/zx.h"
#include "chips/mem.h"
#include "ui/ui_util.h"
#include "ui/ui_chip.h"
#include "ui/ui_z80.h"
#include "ui/ui_ay38910.h"
#include "ui/ui_audio.h"
#include "ui/ui_kbd.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_zx.h"
#include "zx-roms.h"
#include "../imgui_impl_lucidextra.h"


/* audio-streaming callback */
static void PushAudio(const float* samples, int num_samples, void* user_data) 
{
	//saudio_push(samples, num_samples);
}



static zx_t state;
FSpeccy* InitSpeccy(const FSpeccyConfig& config)
{
	FSpeccy *pNewInstance = new FSpeccy();
	//keybuf_init(6);
	//clock_init();
	//saudio_setup(&(saudio_desc) { 0 });
	//fs_init();

	// setup pixel buffer
	const size_t pixelBufferSize = 320 * 256 * 4;
	pNewInstance->FrameBuffer = new unsigned char[pixelBufferSize * 2];

	// setup texture
	pNewInstance->Texture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(pNewInstance->FrameBuffer), 320, 256);
	// setup emu
	zx_type_t type = config.Model == SpeccyModel::Spectrum128K ? ZX_TYPE_128 : ZX_TYPE_48K;
	zx_joystick_type_t joy_type = ZX_JOYSTICKTYPE_NONE;
	
	zx_desc_t desc;
	desc.joystick_type = joy_type;
	desc.pixel_buffer = pNewInstance->FrameBuffer;
	desc.pixel_buffer_size = pixelBufferSize;
	desc.audio_cb = PushAudio;	// our audio callback
	desc.audio_num_samples = ZX_DEFAULT_AUDIO_SAMPLES;
	desc.audio_sample_rate = 44100;// saudio_sample_rate();
	desc.rom_zx48k = dump_amstrad_zx48k_bin;
	desc.rom_zx48k_size = sizeof(dump_amstrad_zx48k_bin);
	desc.rom_zx128_0 = dump_amstrad_zx128k_0_bin;
	desc.rom_zx128_0_size = sizeof(dump_amstrad_zx128k_0_bin);
	desc.rom_zx128_1 = dump_amstrad_zx128k_1_bin;
	desc.rom_zx128_1_size = sizeof(dump_amstrad_zx128k_1_bin);

	pNewInstance->EmuState = &state;// new zx_t;
	zx_init((zx_t*)pNewInstance->EmuState, &desc);

	return pNewInstance;
}

void TickSpeccy(FSpeccy &speccyInstance)
{
	zx_exec((zx_t*)speccyInstance.EmuState, 1000000.0f / ImGui::GetIO().Framerate);

	ImGui_ImplDX11_UpdateTextureRGBA(speccyInstance.Texture, speccyInstance.FrameBuffer);
}

void ShutdownSpeccy(FSpeccy*&pSpeccy)
{
	delete pSpeccy;
	pSpeccy = nullptr;
}
