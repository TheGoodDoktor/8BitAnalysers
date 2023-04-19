#include "SNALoader.h"
#include "../SpectrumEmu.h"

#include <cstdint>
#include <Util/FileUtil.h>
#include <cassert>
#include <systems/zx.h>

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
	if (!pData)
		return false;
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
	pSys->cpu.af = pHdr->AF;
	pSys->cpu.bc = pHdr->BC; 
	pSys->cpu.de = pHdr->DE; 
	pSys->cpu.hl = pHdr->HL; 
	pSys->cpu.ix = pHdr->IX;
	pSys->cpu.iy = pHdr->IY;
	pSys->cpu.af2 = pHdr->AF_;
	pSys->cpu.bc2 = pHdr->BC_;
	pSys->cpu.de2 = pHdr->DE_;
	pSys->cpu.hl2 = pHdr->HL_;
	pSys->cpu.i = pHdr->I;
	pSys->cpu.r = pHdr->R;
	pSys->cpu.iff2 = pHdr->Interrupt & (1 << 2);
	//z80_set_ei_pending(&pSys->cpu, pHdr->Interrupt != 0);
	pSys->cpu.im = pHdr->IM & 3;

#	// copy RAM across
	for (int address = 0x4000; address < (1 << 16); address++)
	{
		pEmu->WriteByte( address, *pRAMData);
		pRAMData++;
	}

	// pop PC off stack
	pSys->cpu.pc = pEmu->ReadWord(pHdr->SP);
	pSys->cpu.sp = pHdr->SP + 2;


	return true;	// NOT implemented
}

