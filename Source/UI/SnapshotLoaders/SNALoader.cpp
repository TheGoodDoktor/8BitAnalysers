#include "SNALoader.h"
#include "../SpectrumEmu.h"

#include <cstdint>
#include <Shared/Util/FileUtil.h>
#include <cassert>
#include <Vendor/chips/systems/zx.h>

#pragma pack(1)
struct FSNAHeader
{
	uint8_t		I;		// 0
	
	uint16_t	HL_;	// 1
	uint16_t	DE_;
	uint16_t	BC_;
	uint16_t	AF_;

	uint16_t	HL;		//9
	uint16_t	DE;
	uint16_t	BC;
	uint16_t	IX;
	uint16_t	IY;

	uint8_t		Interrupt;	// 19
	uint8_t		R;			// 20
	
	uint16_t	AF;			// 21
	uint16_t	SP;

	uint8_t		IM;			// 25
	uint8_t		Border;
};
#pragma pack()

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
	z80_set_af(&pSys->cpu, pHdr->AF);
	z80_set_bc(&pSys->cpu, pHdr->BC); 
	z80_set_de(&pSys->cpu, pHdr->DE); 
	z80_set_hl(&pSys->cpu, pHdr->HL); 
	z80_set_ix(&pSys->cpu, pHdr->IX);
	z80_set_iy(&pSys->cpu, pHdr->IY);
	z80_set_af_(&pSys->cpu, pHdr->AF_);
	z80_set_bc_(&pSys->cpu, pHdr->BC_);
	z80_set_de_(&pSys->cpu, pHdr->DE_);
	z80_set_hl_(&pSys->cpu, pHdr->HL_);
	z80_set_i(&pSys->cpu, pHdr->I);
	z80_set_r(&pSys->cpu, pHdr->R);
	z80_set_iff2(&pSys->cpu, pHdr->Interrupt & (1 << 2));
	//z80_set_ei_pending(&pSys->cpu, pHdr->Interrupt != 0);
	z80_set_im(&pSys->cpu, pHdr->IM & 3);

#	// copy RAM across
	for (int address = 0x4000; address < (1 << 16); address++)
	{
		pEmu->WriteByte( address, *pRAMData);
		pRAMData++;
	}

	// pop PC off stack
	z80_set_pc(&pSys->cpu, pEmu->ReadWord(pHdr->SP));
	z80_set_sp(&pSys->cpu, pHdr->SP + 2);


	return true;	// NOT implemented
}

