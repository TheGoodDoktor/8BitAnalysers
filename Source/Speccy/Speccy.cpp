#include "Speccy.h"

#include "zx-roms.h"


#include "../imgui_impl_lucidextra.h"

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

int MyTrapCallback(uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	FSpeccy *pSpeccy = (FSpeccy *)user_data;

	const uint16_t addr = Z80_GET_ADDR(pins);
	if ((pins & Z80_CTRL_MASK) == (Z80_MREQ | Z80_RD)) 
	{
		const uint16_t addr = Z80_GET_ADDR(pins);
		//win->heatmap.items[addr].read_count++;
	}
	else if ((pins & Z80_CTRL_MASK) == (Z80_MREQ | Z80_WR)) 
	{
		const uint16_t addr = Z80_GET_ADDR(pins);
		//win->heatmap.items[addr].write_count++;
	}
	return 0;
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
	const float frameTime = std::min(1000000.0f / ImGui::GetIO().Framerate,32000.0f) * speccyInstance.ExecSpeedScale;
	zx_exec(&speccyInstance.CurrentState, std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));
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
// Memory access functions

uint8_t* MemGetPtr(zx_t* zx, int layer, uint16_t addr)
{
	if (layer == 0)
	{
		/* ZX128 ROM, RAM 5, RAM 2, RAM 0 */
		if (addr < 0x4000)
			return &zx->rom[0][addr];
		else if (addr < 0x8000)
			return &zx->ram[5][addr - 0x4000];
		else if (addr < 0xC000)
			return &zx->ram[2][addr - 0x8000];
		else
			return &zx->ram[0][addr - 0xC000];
	}
	else if (layer == 1)
	{
		/* 48K ROM, RAM 1 */
		if (addr < 0x4000)
			return &zx->rom[1][addr];
		else if (addr >= 0xC000)
			return &zx->ram[1][addr - 0xC000];
	}
	else if (layer < 8)
	{
		if (addr >= 0xC000)
			return &zx->ram[layer][addr - 0xC000];
	}
	/* fallthrough: unmapped memory */
	return 0;
}

uint8_t MemReadFunc(int layer, uint16_t addr, void* user_data)
{
	assert(user_data);
	zx_t* zx = (zx_t*)user_data;
	if ((layer == 0) || (ZX_TYPE_48K == zx->type))
	{
		/* CPU visible layer */
		return mem_rd(&zx->mem, addr);
	}
	else
	{
		uint8_t* ptr = MemGetPtr(zx, layer - 1, addr);
		if (ptr)
			return *ptr;
		else
			return 0xFF;
	}
}

void MemWriteFunc(int layer, uint16_t addr, uint8_t data, void* user_data)
{
	assert(user_data);
	zx_t* zx = (zx_t*)user_data;
	if ((layer == 0) || (ZX_TYPE_48K == zx->type)) {
		mem_wr(&zx->mem, addr, data);
	}
	else {
		uint8_t* ptr = MemGetPtr(zx, layer - 1, addr);
		if (ptr) {
			*ptr = data;
		}
	}
}


uint16_t GetScreenPixMemoryAddress(int x, int y)
{
	if (x < 0 || x>255 || y < 0 || y> 191)
		return 0;

	const int char_x = x / 8;
	const uint16_t addr = 0x4000 | ((y & 7) << 8) | (((y >> 3) & 7) << 5) | (((y >> 6) & 3) << 11) | (char_x & 31);

	return addr;
}

uint16_t GetScreenAttrMemoryAddress(int x, int y)
{
	if (x < 0 || x>255 || y < 0 || y> 191)
		return 0;

	const int char_x = x / 8;
	const int char_y = y / 8;

	return 0x5800 + (char_y * 32) + char_x;
}