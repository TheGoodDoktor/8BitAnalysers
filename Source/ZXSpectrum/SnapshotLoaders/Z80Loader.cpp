#include "Z80Loader.h"
#include "../SpectrumEmu.h"
#include <Util/FileUtil.h>

#include "Debug/DebugLog.h"

// This Z80 loader was based on the one in Chips
// Contains attempts to improve compatability
/*=== FILE LOADING ===========================================================*/

// ZX Z80 file format header (http://www.worldofspectrum.org/faq/reference/z80format.htm)
struct FZ80Header
{
    uint8_t A, F;
    uint8_t C, B;
    uint8_t L, H;
    uint8_t PC_l, PC_h;
    uint8_t SP_l, SP_h;
    uint8_t I, R;
    uint8_t flags0;
    uint8_t E, D;
    uint8_t C_, B_;
    uint8_t E_, D_;
    uint8_t L_, H_;
    uint8_t A_, F_;
    uint8_t IY_l, IY_h;
    uint8_t IX_l, IX_h;
    uint8_t EI;
    uint8_t IFF2;
    uint8_t flags1;
} ;

struct FZ80ExtHeader
{
    uint8_t len_l;
    uint8_t len_h;
    uint8_t PC_l, PC_h;
    uint8_t hw_mode;
    uint8_t out_7ffd;
    uint8_t rom1;
    uint8_t flags;
    uint8_t out_fffd;
    uint8_t audio[16];
    uint8_t tlow_l;
    uint8_t tlow_h;
    uint8_t spectator_flags;
    uint8_t mgt_rom_paged;
    uint8_t multiface_rom_paged;
    uint8_t rom_0000_1fff;
    uint8_t rom_2000_3fff;
    uint8_t joy_mapping[10];
    uint8_t kbd_mapping[10];
    uint8_t mgt_type;
    uint8_t disciple_button_state;
    uint8_t disciple_flags;
    uint8_t out_1ffd;
};

struct FZ80PageHeader
{
    uint8_t len_l;
    uint8_t len_h;
    uint8_t page_nr;
};

static bool ZXOverflow(const uint8_t* ptr, intptr_t num_bytes, const uint8_t* end_ptr)
{
    return (ptr + num_bytes) > end_ptr;
}

static void Update128KMemoryMap(zx_t* sys, uint8_t data)
{
    if (!sys->memory_paging_disabled)
    {
        sys->last_mem_config = data;
        // bit 3 defines the video scanout memory bank (5 or 7)
        sys->display_ram_bank = (data & (1<<3)) ? 7 : 5;
        // only last memory bank is mappable
        mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[data & 0x7]);

        // ROM0 or ROM1
        if (data & (1<<4)) 
        {
            // bit 4 set: ROM1
            mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[1]);
        }
        else 
        {
            // bit 4 clear: ROM0
            mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[0]);
        }
    }
    
    if (data & (1<<5))
    {
        /* bit 5 prevents further changes to memory pages
            until computer is reset, this is used when switching
            to the 48k ROM
        */
        sys->memory_paging_disabled = true;
    }
}

bool LoadZ80(FSpectrumEmu* pSpectrumEmu, chips_range_t data)
{
    assert(data.ptr && (data.size > 0));
    
    zx_t* sys = &pSpectrumEmu->ZXEmuState;
    
    uint8_t* ptr = (uint8_t *)data.ptr;
    const uint8_t* end_ptr = ptr + data.size;
    if (ZXOverflow(ptr, sizeof(FZ80Header), end_ptr))
    {
        return false;
    }
    
    const FZ80Header* pHeader = (const FZ80Header*) ptr;
    ptr += sizeof(FZ80Header);
    const FZ80ExtHeader* pExtHeader = nullptr;
    uint16_t pc = (pHeader->PC_h<<8 | pHeader->PC_l) & 0xFFFF;
    
    // TODO: I need to mode switch the emulator based on the snapshot
    const bool bIsVersion1 = 0 != pc;
    if (!bIsVersion1)
    {
        if (ZXOverflow(ptr, sizeof(pExtHeader), end_ptr))
        {
            return false;
        }
        pExtHeader = (FZ80ExtHeader*) ptr;
        const int extHeaderLength = (pExtHeader->len_h<<8) | pExtHeader->len_l;
        ptr += 2 + extHeaderLength;
        
        if (pExtHeader->hw_mode < 3)
        {
            if (sys->type != ZX_TYPE_48K) 
            {
                pSpectrumEmu->InitForModel(ESpectrumModel::Spectrum48K);
                //return false;
            }
        }
        else 
        {
            if (sys->type != ZX_TYPE_128) 
            {
                pSpectrumEmu->InitForModel(ESpectrumModel::Spectrum128K);
                //return false;
            }
        }
    }
    else 
    {
        if (sys->type != ZX_TYPE_48K) 
        {
            pSpectrumEmu->InitForModel(ESpectrumModel::Spectrum48K);
            //return false;
        }
    }
    
    const bool v1_compr = 0 != (pHeader->flags0 & (1<<5));  // TODO: make clearer what this means
    
    while (ptr < end_ptr)
    {
        int page_index = 0;
        size_t src_len = 0;
        if (bIsVersion1)
        {
            src_len = data.size - sizeof(FZ80Header);
        }
        else 
        {
            FZ80PageHeader* pPageHeader = (FZ80PageHeader*) ptr;
            
            if (ZXOverflow(ptr, sizeof(FZ80PageHeader), end_ptr))
                return false;
            
            ptr += sizeof(FZ80PageHeader);
            src_len = (pPageHeader->len_h<<8 | pPageHeader->len_l) & 0xFFFF;
            page_index = pPageHeader->page_nr - 3;
            
            if ((sys->type == ZX_TYPE_48K) && (page_index == 5))
                page_index = 0;
            
            if ((page_index < 0) || (page_index > 7))
                page_index = -1;
        }
        
        uint8_t* dst_ptr;
        if (-1 == page_index)
            dst_ptr = sys->junk;
        else
            dst_ptr = sys->ram[page_index];
    
        if (0xFFFF == src_len) 
        {
            // we just copy 16k from ptr to dst_ptr!
            memcpy(dst_ptr,ptr,0x4000);
        }
        else 
        {
            // compressed
            int src_pos = 0;
            bool v1_done = false;
            uint8_t val[4];
            while ((src_pos < src_len) && !v1_done) 
            {
                val[0] = ptr[src_pos];
                val[1] = ptr[src_pos+1];
                val[2] = ptr[src_pos+2];
                val[3] = ptr[src_pos+3];
                // check for version 1 end marker
                if (v1_compr && (0==val[0]) && (0xED==val[1]) && (0xED==val[2]) && (0==val[3])) 
                {
                    v1_done = true;
                    src_pos += 4;
                }
                else if (0xED == val[0]) 
                {
                    if (0xED == val[1]) 
                    {
                        uint8_t count = val[2];
                        assert(0 != count);
                        uint8_t data = val[3];
                        src_pos += 4;
                        for (int i = 0; i < count; i++)
                            *dst_ptr++ = data;
                    }
                    else 
                    {
                        // single ED
                        *dst_ptr++ = val[0];
                        src_pos++;
                    }
                }
                else 
                {
                    // any value
                    *dst_ptr++ = val[0];
                    src_pos++;
                }
            }
            assert(src_pos == src_len);
        }
        
        if (0xFFFF == src_len)
            ptr += 0x4000;
        else
            ptr += src_len;
    }

    // start loaded image
    z80_reset(&sys->cpu);
    sys->cpu.a = pHeader->A; sys->cpu.f = pHeader->F;
    sys->cpu.b = pHeader->B; sys->cpu.c = pHeader->C;
    sys->cpu.d = pHeader->D; sys->cpu.e = pHeader->E;
    sys->cpu.h = pHeader->H; sys->cpu.l = pHeader->L;
    sys->cpu.ix = (pHeader->IX_h<<8)|pHeader->IX_l;
    sys->cpu.iy = (pHeader->IY_h<<8)|pHeader->IY_l;
    sys->cpu.af2 = (pHeader->A_<<8)|pHeader->F_;
    sys->cpu.bc2 = (pHeader->B_<<8)|pHeader->C_;
    sys->cpu.de2 = (pHeader->D_<<8)|pHeader->E_;
    sys->cpu.hl2 = (pHeader->H_<<8)|pHeader->L_;
    sys->cpu.sp = (pHeader->SP_h<<8)|pHeader->SP_l;
    sys->cpu.i = pHeader->I;
    sys->cpu.r = (pHeader->R & 0x7F) | ((pHeader->flags0 & 1)<<7);
    sys->cpu.iff2 = (pHeader->IFF2 != 0);
    sys->cpu.iff1 = (pHeader->EI != 0);
    
    if (pHeader->flags1 != 0xFF)
        sys->cpu.im = pHeader->flags1 & 3;
    else
        sys->cpu.im = 1;
    
    if (pExtHeader != nullptr)
    {
        sys->pins = z80_prefetch(&sys->cpu, (pExtHeader->PC_h<<8) | pExtHeader->PC_l);
        
        if (sys->type == ZX_TYPE_128) 
        {
            ay38910_reset(&sys->ay);
            for (uint8_t i = 0; i < AY38910_NUM_REGISTERS; i++)
                ay38910_set_register(&sys->ay, i, pExtHeader->audio[i]);
            
            ay38910_set_addr_latch(&sys->ay, pExtHeader->out_fffd);
            Update128KMemoryMap(sys, pExtHeader->out_7ffd);
        }
    }
    else 
    {
        sys->pins = z80_prefetch(&sys->cpu, (pHeader->PC_h<<8) | pHeader->PC_l);
    }
    
    sys->border_color = (pHeader->flags0>>1) & 7;
    
    return true;
}


bool LoadZ80File(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	void* pData = LoadBinaryFile(fName, byteCount);

	if (pData == nullptr)
		return false;

	const bool bSuccess = LoadZ80FromMemory(pEmu, (const uint8_t*)pData, byteCount);
	free(pData);
	return bSuccess;
}

bool LoadZ80FromMemory(FSpectrumEmu* pSpectrumEmu, const uint8_t* pData, size_t dataSize)
{
	chips_range_t dataInfo;
	dataInfo.ptr = (void*)pData;
	dataInfo.size = dataSize;
	//if (zx_quickload(&pSpectrumEmu->ZXEmuState, dataInfo) == false)
	//	return false;
    if (LoadZ80(pSpectrumEmu, dataInfo) == false)
        return false;
    

	if (pSpectrumEmu->ZXEmuState.type == ZX_TYPE_128)
	{
		uint8_t memConfig = pSpectrumEmu->ZXEmuState.last_mem_config;

		// Set code analysis banks
		pSpectrumEmu->SetROMBank(memConfig & (1 << 4) ? 1 : 0);
		pSpectrumEmu->SetRAMBank(3, memConfig & 0x7);
	}
	return true;
}
