#include "SNALoader.h"

#include "../CPCEmu.h"

#include "Debug/DebugLog.h"
#include <Util/FileUtil.h>
#include <systems/cpc.h>
#include "../CPCChipsImpl.h"

#ifndef _NDEBUG
#define SNAPSHOT_LOADER_DEBUG
#endif

#ifdef SNAPSHOT_LOADER_DEBUG
#define SNAPSHOT_LOG(...)  { LOGINFO("[SNA Loader] " __VA_ARGS__); }
#else
#define SNAPSHOT_LOG(...)
#endif

bool LoadSNAFile(FCPCEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	if (!pData)
		return false;

	const bool bSuccess = LoadSNAFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadSNAFileCached(FCPCEmu* pEmu, const char* fName, uint8_t*& pData , size_t& dataSize)
{
	if (pData == nullptr)
	{
		pData = (uint8_t*)LoadBinaryFile(fName, dataSize);
		if (!pData)
			return false;
	}
	return LoadSNAFromMemory(pEmu, pData, dataSize);
}

// This snapshot loading code is based on the Chips code but is modified to offer improved compatibility.
// Snapshot format https://www.cpcwiki.eu/index.php/Snapshot
#pragma pack(1)
struct FCPCSnapHeader
{
	uint8_t Magic[8];     // must be "MV - SNA"
	uint8_t Pad0[8];
	uint8_t Version;
	uint8_t F, A, C, B, E, D, L, H, R, I;
	uint8_t IFF1, IFF2;
	uint8_t IX_l, IX_h;
	uint8_t IY_l, IY_h;
	uint8_t SP_l, SP_h;
	uint8_t PC_l, PC_h;
	uint8_t InterruptMode;
	uint8_t F_, A_, C_, B_, E_, D_, L_, H_;
	uint8_t SelectedPen;
	uint8_t CurrentPalette[17];             // palette + border colors
	uint8_t GateArrayConfig;
	uint8_t RAMConfig;
	uint8_t CRTCSelectedReg;
	uint8_t CRTCRegisters[18];
	uint8_t ROMConfig;
	uint8_t PPIPortA;
	uint8_t PPIPortB;
	uint8_t PPIPortC;
	uint8_t PPIControlPort;
	uint8_t PSGSelectedReg;
	uint8_t PSGRegisters[16];
	uint8_t DumpSizeL;		// 0x6b
	uint8_t DumpSizehH;		// 0x6c
	uint8_t MachineType;		// 0x6d >v1 only
	uint8_t Pad1[0x92];
};
#pragma pack()

// Decompress RLE snapshot data into RAM.
int DecompressSnapshotDataToRAM(FCPCEmu* pEmu, const uint8_t* pData, int dataSize, int dstAddr)
{
	const uint8_t kControlByte = 0xe5;
	const uint8_t* pCur = pData;
	const uint8_t* pEnd = pData + dataSize;
	uint16_t addr = dstAddr;

	int bytesWritten = 0;
	while (pCur < pEnd)
	{
		const uint8_t byteVal = *pCur;
		if (byteVal == kControlByte)
		{
			pCur++;
			const uint8_t count = *pCur;
			if (count == 0)
			{
				pEmu->WriteByte(addr++, kControlByte);
				bytesWritten++;
			}
			else
			{
				pCur++;
				const uint8_t repeatVal = *pCur;
				for (int i = 0; i < count; i++)
				{
					pEmu->WriteByte(addr++, repeatVal);
					bytesWritten++;
				}
			}
		}
		else
		{
			pEmu->WriteByte(addr++, byteVal);
			bytesWritten++;
		}
		pCur++;
	}
	return bytesWritten;
}

const std::string GetSnapshotMachineName(uint8_t machineType)
{
	std::string machineTypes[7] =
	{
		"CPC 464",
		"CPC 664",
		"CPC 6128",
		"unknown",
		"6128 Plus",
		"464 Plus",
		"GX4000"
	};
	if (machineType < 7)
		return machineTypes[machineType];
	return "unknown machine type";
}

bool LoadSNAFromMemory(FCPCEmu * pEmu, uint8_t * pData, size_t dataSize)
{	
	const uint8_t* const pEnd = pData + dataSize;
	const uint8_t* pCur = pData;

	if (dataSize <= 0x100)
	{
		pEmu->SetLastError("Invalid snapshot size.");
		return false;
	}

	static uint8_t magic[8] = { 'M', 'V', 0x20, '-', 0x20, 'S', 'N', 'A' };
	for (size_t i = 0; i < 8; i++)
	{
		if (magic[i] != pCur[i])
		{
			pEmu->SetLastError("Invalid snapshot header");
			return false;
		}
	}

	cpc_t& cpc = pEmu->CPCEmuState;

	const FCPCSnapHeader* pHdr = (const FCPCSnapHeader*)pCur;
	pCur += sizeof(FCPCSnapHeader);

	z80_reset(&cpc.cpu);
	cpc.cpu.f = pHdr->F; cpc.cpu.a = pHdr->A;
	cpc.cpu.c = pHdr->C; cpc.cpu.b = pHdr->B;
	cpc.cpu.e = pHdr->E; cpc.cpu.d = pHdr->D;
	cpc.cpu.l = pHdr->L; cpc.cpu.h = pHdr->H;
	cpc.cpu.r = pHdr->R; cpc.cpu.i = pHdr->I;
	cpc.cpu.iff1 = (pHdr->IFF1 & 1) != 0;
	cpc.cpu.iff2 = (pHdr->IFF2 & 1) != 0;
	cpc.cpu.ix = (pHdr->IX_h << 8) | pHdr->IX_l;
	cpc.cpu.iy = (pHdr->IY_h << 8) | pHdr->IY_l;
	cpc.cpu.sp = (pHdr->SP_h << 8) | pHdr->SP_l;
	cpc.cpu.pc = (pHdr->PC_h << 8) | pHdr->PC_l;
	cpc.cpu.im = pHdr->InterruptMode;
	cpc.cpu.af2 = (pHdr->A_ << 8) | pHdr->F_;
	cpc.cpu.bc2 = (pHdr->B_ << 8) | pHdr->C_;
	cpc.cpu.de2 = (pHdr->D_ << 8) | pHdr->E_;
	cpc.cpu.hl2 = (pHdr->H_ << 8) | pHdr->L_;

	for (int i = 0; i < 16; i++) 
	{
		cpc.ga.regs.ink[i] = pHdr->CurrentPalette[i] & 0x1F;
	}
	cpc.ga.regs.border = pHdr->CurrentPalette[16] & 0x1F;
	cpc.ga.regs.inksel = pHdr->SelectedPen & 0x1F;
	cpc.ga.regs.config = pHdr->GateArrayConfig & 0x3F;
	cpc.ga.ram_config = pHdr->RAMConfig & 0x3F;
	cpc.ga.rom_select = pHdr->ROMConfig;
	CPCBankSwitchCB(cpc.ga.ram_config, cpc.ga.regs.config, cpc.ga.rom_select, cpc.ga.user_data);

	for (int i = 0; i < 18; i++) 
	{
		cpc.crtc.reg[i] = pHdr->CRTCRegisters[i];
	}
	cpc.crtc.sel = pHdr->CRTCSelectedReg;

	cpc.ppi.pa.outp = pHdr->PPIPortA;
	cpc.ppi.pb.outp = pHdr->PPIPortB;
	cpc.ppi.pc.outp = pHdr->PPIPortC;
	cpc.ppi.control = pHdr->PPIControlPort;

	for (int i = 0; i < 16; i++) 
	{
		ay38910_set_register(&cpc.psg, i, pHdr->PSGRegisters[i]);
	}
	ay38910_set_addr_latch(&cpc.psg, pHdr->PSGSelectedReg);

	cpc.ga.video.mode = cpc.ga.regs.config & AM40010_CONFIG_MODE;
	cpc.ga.rom_select = 0;

	// todo: maybe set rom and ram banks here for 464 too
	
	if (pHdr->Version > 1)
	{
		const std::string machine = GetSnapshotMachineName(pHdr->MachineType);
		SNAPSHOT_LOG("Machine type is %s", machine.c_str());
		if (pEmu->CPCEmuState.type == CPC_TYPE_464 && pHdr->MachineType != 0)
		{
			LOGWARNING("Snapshot is for '%s' and may not be compatible with the current machine: CPC 464", machine.c_str());
		}
	}

	const uint16_t dumpSize = pHdr->DumpSizehH << 8 | pHdr->DumpSizeL;
	SNAPSHOT_LOG("Dump size is %d", dumpSize);
	
	// If dumpSize is non-zero then an uncompressed memory dump will follow the header. 
	// Dump size can be either 64 or 128.
	if (dumpSize)
	{
		if (pEmu->CPCEmuState.type == CPC_TYPE_464 && dumpSize > 64)
		{
			pEmu->SetLastError("Snapshot is not a 464 snapshot.");
			return false;
		}

		// copy 64 or 128 KByte memory dump
		const uint32_t dumpNumBytes = (dumpSize == 64) ? 0x10000 : 0x20000;
		if (dataSize > (sizeof(FCPCSnapHeader) + dumpNumBytes))
		{
			return false;
		}

		if (dumpNumBytes > sizeof(cpc.ram))
		{
			pEmu->SetLastError("Memory dump size is too big for RAM.");
			return false;
		}
		SNAPSHOT_LOG("Copying %d bytes to ram", dumpNumBytes);

		memcpy(cpc.ram, pCur, dumpNumBytes);
	}
	
	// If the dump size is 0 then the memory dump will be in MEM0-MEM8 chunks following the header.
	// For 464 snapshots, we are only interested in the MEM0 chunk which describes the main 64k of RAM.
	if (!dumpSize)
	{
		while (pCur < pEnd)
		{
			const uint8_t* pChunkId = pCur;
			pCur += 4; // skip chunk ID
			const uint32_t chunkSize = (pCur[3] << 24) | (pCur[2] << 16) | (pCur[1] << 8) | pCur[0];
			pCur += 4; // skip chunkSize
			SNAPSHOT_LOG("Found %c%c%c%c chunk of size %d", pChunkId[0], pChunkId[1], pChunkId[2], pChunkId[3], chunkSize);

			if (pChunkId[0] == 'M' && pChunkId[1] == 'E' && pChunkId[2] == 'M' && pChunkId[3] == '0')
			{
				if (chunkSize == 65536)
				{
					// todo if chunk size is 65536 then the data is uncompressed
					pEmu->SetLastError("MEM0 chunk contains uncompressed data. Not currently supported");
					return false;
				}
				else
				{
					DecompressSnapshotDataToRAM(pEmu, pCur, chunkSize, 0);
				}
			}
			pCur += chunkSize;
		}
	}

	if (pEmu->CPCEmuState.type == CPC_TYPE_6128)
	{
		// todo: set rom bank here

		pEmu->SetRAMBanksPreset(pEmu->CPCEmuState.ga.ram_config & 7);
	}
	
	return true;
}

