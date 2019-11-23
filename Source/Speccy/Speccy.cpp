#include "Speccy.h"

#include "zx-roms.h"


#include "../imgui_impl_lucidextra.h"

#include "Util/FileUtil.h"

std::vector<std::string> g_GamesList;

bool EnumerateGames(void)
{
	FDirFileList listing;

	if (EnumerateDirectory(".", listing) == false)
		return false;

	for (const auto &file : listing)
	{
		const std::string &fn = file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "z80") || (fn.substr(fn.find_last_of(".") + 1) == "Z80"))
		{
			g_GamesList.push_back(fn);
		}
	}
	return true;
}

const std::vector<std::string>& GetGameList()
{
	return g_GamesList;
}

/* audio-streaming callback */
static void PushAudio(const float* samples, int num_samples, void* user_data) 
{
	//saudio_push(samples, num_samples);
}
 
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
	desc.type = type;
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

	zx_init(&pNewInstance->CurrentState, &desc);

	EnumerateGames();

	// create state buffer
	if (config.NoStateBuffers > 0)
	{
		const size_t bufferMemSize = config.NoStateBuffers * sizeof(zx_t);
		pNewInstance->pStateBuffers = (zx_t*)malloc(bufferMemSize);
		pNewInstance->NoStateBuffers = config.NoStateBuffers;
		pNewInstance->CurrentStateBuffer = 0;
	}

	return pNewInstance;
}

void TickSpeccy(FSpeccy &speccyInstance)
{
	zx_exec(&speccyInstance.CurrentState, static_cast<uint32_t>(1000000.0f / ImGui::GetIO().Framerate));
	ImGui_ImplDX11_UpdateTextureRGBA(speccyInstance.Texture, speccyInstance.FrameBuffer);

	// Copy state buffer over - could be more efficient if needed
	memcpy(&speccyInstance.pStateBuffers[speccyInstance.CurrentStateBuffer], &speccyInstance.CurrentState, sizeof(zx_t));

	speccyInstance.CurrentStateBuffer = (speccyInstance.CurrentStateBuffer + 1) % speccyInstance.NoStateBuffers;
}

void ShutdownSpeccy(FSpeccy*&pSpeccy)
{
	delete pSpeccy;
	pSpeccy = nullptr;
}


bool LoadZ80File(FSpeccy &speccyInstance, const char *fName)
{
	size_t byteCount = 0;
	void *pData = LoadBinaryFile(fName, byteCount);

	const bool bSuccess = zx_quickload(&speccyInstance.CurrentState, (const uint8_t *)pData, (int)byteCount);
	free(pData);
	return bSuccess;
}