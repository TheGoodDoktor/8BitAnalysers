#if 0
#include "Speccy.h"

#include "zx-roms.h"


#include <ImGuiSupport/imgui_impl_lucidextra.h>

#include "Util/FileUtil.h"
#include <algorithm>


std::vector<std::string> g_GamesList;

bool EnumerateGames(void)
{
	FDirFileList listing;

	if (EnumerateDirectory("./Games", listing) == false)
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

	// setup pixel buffer
	const size_t pixelBufferSize = 320 * 256 * 4;
	pNewInstance->FrameBuffer = new unsigned char[pixelBufferSize * 2];

	// setup texture
	pNewInstance->Texture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(pNewInstance->FrameBuffer), 320, 256);
	// setup emu
	zx_type_t type = config.Model == SpeccyModel::Spectrum128K ? ZX_TYPE_128 : ZX_TYPE_48K;
	zx_joystick_type_t joy_type = ZX_JOYSTICKTYPE_NONE;
	
	zx_desc_t desc;
	memset(&desc, 0, sizeof(zx_desc_t));
	desc.type = type;
	desc.joystick_type = joy_type;
	desc.pixel_buffer = pNewInstance->FrameBuffer;
	desc.pixel_buffer_size = pixelBufferSize;
	desc.audio_cb = PushAudio;	// our audio callback
	desc.audio_sample_rate = saudio_sample_rate();
	desc.rom_zx48k = dump_amstrad_zx48k_bin;
	desc.rom_zx48k_size = sizeof(dump_amstrad_zx48k_bin);
	desc.rom_zx128_0 = dump_amstrad_zx128k_0_bin;
	desc.rom_zx128_0_size = sizeof(dump_amstrad_zx128k_0_bin);
	desc.rom_zx128_1 = dump_amstrad_zx128k_1_bin;
	desc.rom_zx128_1_size = sizeof(dump_amstrad_zx128k_1_bin);

	zx_init(&pNewInstance->CurrentState, &desc);
	//static int _ui_dbg_bp_eval(uint16_t pc, int ticks, uint64_t pins, void* user_data) {
	//z80_trap_cb(&pNewInstance->CurrentState.cpu, MyTrapCallback, pNewInstance);

	
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
	const float frameTime = min(1000000.0f / ImGui::GetIO().Framerate,32000.0f) * speccyInstance.ExecSpeedScale;
	zx_exec(&speccyInstance.CurrentState, max(static_cast<uint32_t>(frameTime), uint32_t(1)));
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

struct FSNAHeader
{
	uint8_t		I;
	uint8_t		H_;
	uint8_t		L_;
	uint8_t		D_;
	uint8_t		E_;
	uint8_t		B_;
	uint8_t		C_;
	uint8_t		A_;
	uint8_t		F_;
	uint8_t		H;
	uint8_t		L;
	uint8_t		D;
	uint8_t		E;
	uint8_t		B;
	uint8_t		C;
	uint8_t		IY_h;
	uint8_t		IY_l;
	uint8_t		IX_h;
	uint8_t		IX_l;
	uint8_t		IFF2;
	uint8_t		R;
	uint8_t		IX;
	uint8_t		A;
	uint8_t		F;
	uint8_t		SP_h;
	uint8_t		SP_l;
	uint8_t		IM;
	uint8_t		Border;
};

bool LoadSNAFile(FSpeccy &speccyInstance, const char *fName)
{
	size_t byteCount = 0;
	uint8_t *pData = (uint8_t *)LoadBinaryFile(fName, byteCount);

	const FSNAHeader *pHdr = (const FSNAHeader *)pData;
	const uint8_t* pRAMData = pData + sizeof(FSNAHeader);

	assert(sizeof(FSNAHeader) == 27);

/* $00  I
$01  HL'    
$03  DE'
$05  BC'
$07  AF'
$09  HL
$0B  DE
$0D  BC
$0F  IY
$11  IX
$13  IFF2    [Only bit 2 is defined: 1 for EI, 0 for DI]
$14  R
$15  AF
$17  SP
$19  Interrupt mode: 0, 1 or 2
$1A  Border colour

Then 48K of RAM
*/

	zx_t *pSys = &speccyInstance.CurrentState;

	z80_reset(&pSys->cpu);
	z80_set_a(&pSys->cpu, pHdr->A); z80_set_f(&pSys->cpu, pHdr->F);
	z80_set_b(&pSys->cpu, pHdr->B); z80_set_c(&pSys->cpu, pHdr->C);
	z80_set_d(&pSys->cpu, pHdr->D); z80_set_e(&pSys->cpu, pHdr->E);
	z80_set_h(&pSys->cpu, pHdr->H); z80_set_l(&pSys->cpu, pHdr->L);
	z80_set_ix(&pSys->cpu, pHdr->IX_h << 8 | pHdr->IX_l);
	z80_set_iy(&pSys->cpu, pHdr->IY_h << 8 | pHdr->IY_l);
	z80_set_af_(&pSys->cpu, pHdr->A_ << 8 | pHdr->F_);
	z80_set_bc_(&pSys->cpu, pHdr->B_ << 8 | pHdr->C_);
	z80_set_de_(&pSys->cpu, pHdr->D_ << 8 | pHdr->E_);
	z80_set_hl_(&pSys->cpu, pHdr->H_ << 8 | pHdr->L_);
	z80_set_sp(&pSys->cpu, pHdr->SP_h << 8 | pHdr->SP_l);
	z80_set_i(&pSys->cpu, pHdr->I);
	z80_set_r(&pSys->cpu, pHdr->R);
	z80_set_iff2(&pSys->cpu, pHdr->IFF2);
	//z80_set_ei_pending(&sys->cpu, hdr->EI != 0);
	z80_set_im(&pSys->cpu, pHdr->IM & 3);

#	// copy RAM across
	for(int address=0x4000; address < (1<<16);address++)
	{
		WriteSpeccyByte(&speccyInstance, address, *pRAMData);
		pRAMData++;
	}

	free(pData);

	return false;	// NOT implemented
}

bool LoadGameSnapshot(FSpeccy &speccyInstance, const char *fName)
{
	const std::string fn(fName);
	if ((fn.substr(fn.find_last_of(".") + 1) == "z80") || (fn.substr(fn.find_last_of(".") + 1) == "Z80"))
		return LoadZ80File(speccyInstance, fName);
	else if ((fn.substr(fn.find_last_of(".") + 1) == "sna") || (fn.substr(fn.find_last_of(".") + 1) == "SNA"))
		return LoadSNAFile(speccyInstance, fName);
	else
		return false;
}
#endif


