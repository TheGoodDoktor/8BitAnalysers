#include "SNALoader.h"
#include "../SpectrumEmu.h"

#include <cstdint>
#include <Shared/Util/FileUtil.h>
#include <cassert>
#include <Vendor/chips/systems/zx.h>


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


bool LoadSNAFile(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	const bool bSuccess = LoadSNAFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadSNAFromMemory(FSpectrumEmu * pEmu, const uint8_t * pData, size_t dataSize)
{
	const FSNAHeader* pHdr = (const FSNAHeader*)pData;
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

	zx_t* pSys = &pEmu->ZXEmuState;

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
	for (int address = 0x4000; address < (1 << 16); address++)
	{
		pEmu->WriteByte( address, *pRAMData);
		pRAMData++;
	}


	return false;	// NOT implemented
}

