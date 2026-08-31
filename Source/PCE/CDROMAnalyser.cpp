#include "CDROMAnalyser.h"
#include "PCEEmu.h"

#include <geargrafx_core.h>
#include <cdrom_media.h>

FCDROMAnalyser::FCDROMAnalyser(FPCEEmu* pEmu)
	: pPCEEmu(pEmu)
{
}

void FCDROMAnalyser::Reset()
{
	FirstDataTrackLBA = kInvalidLBA;

	if (!pPCEEmu->IsCDROM())
		return;

	// Cache the first data track LBA (sector).
	// The BIOS sector number is relative to the first data track, not an absolute disc LBA.
	// Find the first data track's start LBA so we can convert to absolute when a read happens.
	const auto& tracks = pPCEEmu->GetCore()->GetCDROMMedia()->GetTracks();
	uint32_t firstDataTrackLBA = 0;
	for (const auto& track : tracks)
	{
		if (track.type != GG_CDROM_AUDIO_TRACK)
		{
			FirstDataTrackLBA = track.start_lba;
			return;
		}
	}
}

void FCDROMAnalyser::RegisterCDRead(const FBiosCDReadArgs& args)
{
	assert(FirstDataTrackLBA != kInvalidLBA);

	CdRomMedia* pMedia = pPCEEmu->GetCore()->GetCDROMMedia();
	const uint32_t absoluteLBA = args.LBAOffset + FirstDataTrackLBA;
	const s32 trackIndex = pMedia->GetTrackFromLBA(absoluteLBA);
	if (trackIndex < 0)
		return;

	if ((int)TrackTransfers.size() <= trackIndex)
		TrackTransfers.resize(trackIndex + 1);

	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	const FAddressRef destAddr = state.AddressRefFromPhysicalAddress(args.PhysicalAddr);

	FCDROMTransfer transfer;
	transfer.LBAOffset    = args.LBAOffset;
	transfer.DestAddr     = destAddr;
	transfer.SizeInBytes  = static_cast<uint16_t>(args.TransferSizeBytes);
	transfer.TrackIndex   = trackIndex;

	TrackTransfers[trackIndex].push_back(transfer);

	const FCodeAnalysisBank* pBank = state.GetBank(destAddr.GetBankId());
	LOGINFO("CD_READ [Track %d] %d bytes from LBA %d -> %s %x", trackIndex, transfer.SizeInBytes, transfer.LBAOffset, pBank ? pBank->Name.c_str() : "UNKNOWN", transfer.DestAddr.GetAddress());
}

const uint16_t zipBaseAddr = 0x2000;
const uint16_t _al = zipBaseAddr + 0xF8;
//const uint16_t _ah = zipBaseAddr + 0xF9;
//const uint16_t _bl = zipBaseAddr + 0xFA;
//const uint16_t _bh = zipBaseAddr + 0xFB;
const uint16_t _cl = zipBaseAddr + 0xFC;
const uint16_t _ch = zipBaseAddr + 0xFD;
const uint16_t _dl = zipBaseAddr + 0xFE;
const uint16_t _dh = zipBaseAddr + 0xFF;

//const uint16_t _ax = zipBaseAddr + 0xF8;
const uint16_t _bx = zipBaseAddr + 0xFA;
//const uint16_t _cx = zipBaseAddr + 0xFC;
//const uint16_t _dx = zipBaseAddr + 0xFE;

bool GetBiosCDReadArgs(FPCEEmu* pEmu, FBiosCDReadArgs& args)
{
	const uint8_t mode = pEmu->ReadByte(_dh); // $ff
	uint32_t numToRead = pEmu->ReadByte(_al); // $f8

	const uint8_t clByte = pEmu->ReadByte(_cl); // $fc
	const uint8_t chByte = pEmu->ReadByte(_ch); // $fd
	const uint8_t dlByte = pEmu->ReadByte(_dl); // $fe
	args.LBAOffset = dlByte + (chByte << 8) + (clByte << 16);

	switch (mode)
	{
		case 0: // Physical memory: size in bytes
			// todo
			LOGINFO("CD_READ mode %d unsupported (Physical - size in bytes)", mode);
			return false;
		case 1: // Physical memory: size in sectors
			args.Destination = EBiosCDReadDest::PhysicalMemory;
			args.PhysicalAddr = pEmu->ReadWord(_bx);
			args.TransferSizeBytes = numToRead * 2048;
			return true;
		case 2: // MPR num
		case 3:
		case 4:
		case 5:
		case 6:
			// todo
			LOGINFO("CD_READ mode %d unsupported (MPR mode)", mode);
			return false;
		case 0xfe: // VRAM
		case 0xff: // VRAM
			// todo
			LOGINFO("CD_READ mode %d unsupported (VRAM)", mode);
			return false;
	}

	return false;
}