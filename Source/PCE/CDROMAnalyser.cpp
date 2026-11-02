#include "CDROMAnalyser.h"
#include "PCEEmu.h"

#include <stdio.h>
#include <string>
#include <algorithm>

#include <geargrafx_core.h>
#include <cdrom_media.h>

FCDROMAnalyser::FCDROMAnalyser(FPCEEmu* pEmu)
	: pPCEEmu(pEmu)
{
}

void FCDROMAnalyser::Reset()
{
	FirstDataTrackLBA = kInvalidLBA;
	TransferLog.clear();
	ActiveTransferIndex = -1;
	bActiveHasReturnAddr = false;

	for (FCDBankState& bankState : BankStates)
		bankState = FCDBankState();
	ReplacedPieces.clear();
	bTrackingUsage = false;	// bUsageTrackingEnabled is a user setting so it survives a reset

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

	// A new read started before the previous one was seen finishing.
	if (ActiveTransferIndex != -1)
		EndTransfer(false);

	CdRomMedia* pMedia = pPCEEmu->GetCore()->GetCDROMMedia();
	const uint32_t absoluteLBA = args.LBAOffset + FirstDataTrackLBA;
	const s32 trackIndex = pMedia->GetTrackFromLBA(absoluteLBA);
	if (trackIndex < 0)
		return;

	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	FCDROMTransfer transfer;
	transfer.Args         = args;
	// The destination can be a dupe mapping so redirect to the canonical bank.
	transfer.DestAddr     = args.bDecoded ? state.GetCanonicalAddressRef(args.PhysicalAddr) : FAddressRef::Invalid();
	transfer.TrackIndex   = trackIndex;
	transfer.FrameNo      = state.CurrentFrameNo;

	// A read can be up to 255 sectors so it can run past the end of the address space.
	// Only the part that fits can be captured.
	if (args.bDecoded)
	{
		const uint32_t bytesToEnd = 0x10000 - args.PhysicalAddr;
		transfer.ExpectedBytes = args.TransferSizeBytes < bytesToEnd ? args.TransferSizeBytes : bytesToEnd;
		if (transfer.ExpectedBytes < args.TransferSizeBytes)
			LOGWARNING("CD_READ from LBA %d: %u bytes to %04x runs past $FFFF. Only the first %u bytes are tracked.", args.LBAOffset, args.TransferSizeBytes, args.PhysicalAddr, transfer.ExpectedBytes);
	}

	// Record every bank the transfer writes to, using the mpr values at the time of the read.
	if (args.bDecoded && transfer.ExpectedBytes > 0)
	{
		const uint32_t lastAddr = args.PhysicalAddr + transfer.ExpectedBytes - 1;
		const int firstSlot = args.PhysicalAddr >> 13;
		const int lastSlot = lastAddr >> 13;
		for (int slot = firstSlot; slot <= lastSlot && transfer.NumDestBanks < FCDROMTransfer::kMaxDestBanks; slot++)
		{
			const uint8_t bankIndex = pPCEEmu->GetMemory()->GetMpr(slot);
			const FBankSet* pBankSet = pPCEEmu->GetBankSetPtr(bankIndex);
			transfer.DestBankIndices[transfer.NumDestBanks] = bankIndex;
			transfer.DestBankIds[transfer.NumDestBanks] = pBankSet ? pBankSet->GetBankId() : -1;
			transfer.NumDestBanks++;
		}
	}

	// The caller is the innermost call on the callstack made from outside the BIOS bank we are in now.
	// The callstack is only used for reporting the caller and for ending reads in modes that aren't decoded.
	const int16_t biosBankId = pPCEEmu->GetPC().GetBankId();
	const auto& callStack = state.Debugger.GetCallstack();
	bActiveHasReturnAddr = false;
	for (int i = (int)callStack.size() - 1; i >= 0; i--)
	{
		if (callStack[i].CallAddr.GetBankId() != biosBankId)
		{
			transfer.CallerAddr = callStack[i].CallAddr;
			transfer.CallerPhysAddr = transfer.CallerAddr.GetAddress();
			if (callStack[i].ReturnAddr.IsValid())
			{
				// Physical address and bank index of the return address as they are now, while the caller is mapped in.
				bActiveHasReturnAddr = true;
				ActiveReturnPhysAddr = callStack[i].ReturnAddr.GetAddress();
				ActiveReturnBankIndex = pPCEEmu->GetMemory()->GetMpr(ActiveReturnPhysAddr >> 13);
			}
			break;
		}
	}

	// Start capturing writes.
	TransferLog.push_back(transfer);
	ActiveTransferIndex = (int)TransferLog.size() - 1;
	ActiveCoverage.assign(args.bDecoded ? transfer.ExpectedBytes : 0, 0);

	if (args.bDecoded)
	{
		std::string destBanks;
		for (int b = 0; b < transfer.NumDestBanks; b++)
		{
			const FCodeAnalysisBank* pBank = state.GetBank(transfer.DestBankIds[b]);
			char bankTxt[48];
			snprintf(bankTxt, sizeof(bankTxt), "%s%s(0x%02x)", b ? " " : "", pBank ? pBank->Name.c_str() : "UNKNOWN", transfer.DestBankIndices[b]);
			destBanks += bankTxt;
		}
		LOGINFO("CD_READ mode %d [Track %d] %u bytes (%d sectors) from LBA %d -> %04x [%s]. Frame %d. Caller %04x", args.Mode, trackIndex, args.TransferSizeBytes, args.RawAL, args.LBAOffset, args.PhysicalAddr, destBanks.c_str(), transfer.FrameNo, transfer.CallerPhysAddr);
	}
	else
	{
		LOGINFO("CD_READ mode %d (not decoded) [Track %d] from LBA %d. al=%02x bx=%04x. Frame %d. Caller %04x", args.Mode, trackIndex, args.LBAOffset, args.RawAL, args.RawBX, transfer.FrameNo, transfer.CallerPhysAddr);
	}

	// Nothing to capture (e.g. a sector count of 0), so the finish check in RecordTransferWrite() would never fire.
	if (args.bDecoded && transfer.ExpectedBytes == 0)
		EndTransfer(true);
}

// A read that hasn't finished after this many frames is ended as incomplete.
static const int kMaxTransferFrames = 600;

void FCDROMAnalyser::RecordTransferWrite(uint16_t pc, uint16_t physAddr)
{
	FCDROMTransfer& transfer = TransferLog[ActiveTransferIndex];
	Memory* pMemory = pPCEEmu->GetMemory();

	// Reject writes outside the destination.
	if (transfer.Args.bDecoded)
	{
		if (physAddr < transfer.Args.PhysicalAddr || physAddr >= transfer.Args.PhysicalAddr + transfer.ExpectedBytes)
		{
			transfer.OtherWrites++;
			return;
		}
	}
	else
	{
		// Not decoded so we don't know the destination. Skip the hardware page and WRAM, where the
		// BIOS and interrupt handlers keep their variables.
		if (physAddr < 0x4000)
		{
			transfer.OtherWrites++;
			return;
		}
	}

	// Only the BIOS copy loop writes CD data. Rejecting writes from any other code (e.g. a game interrupt handler)
	// stops them being recorded as disc data. This has never been seen in Rondo - the count is here to report it if it ever happens.
	if (pMemory->GetBankType(pMemory->GetMpr(pc >> 13)) != Memory::MEMORY_BANK_TYPE_BIOS)
	{
		transfer.NonBiosWrites++;
		return;
	}

	// Work out where this byte sits in the transfer.
	uint32_t transferOffset = 0;
	if (transfer.Args.bDecoded)
	{
		transferOffset = physAddr - transfer.Args.PhysicalAddr;

		// Only count each destination byte once, so a rewrite (e.g. a retry) can't make the read look complete.
		if (ActiveCoverage[transferOffset])
		{
			transfer.DuplicateWrites++;
		}
		else
		{
			ActiveCoverage[transferOffset] = 1;
			transfer.BytesWritten++;
		}
	}
	else
	{
		// Assume the data is written in order.
		transferOffset = transfer.BytesWritten;
		transfer.BytesWritten++;
	}

	const uint8_t bankIndex = pMemory->GetMpr(physAddr >> 13);
	const uint16_t bankOffset = physAddr & 0x1fff;

	// Extend the last run if this byte follows on from it, otherwise start a new run.
	FCDWriteRun* pLastRun = transfer.WriteRuns.empty() ? nullptr : &transfer.WriteRuns.back();
	if (pLastRun && pLastRun->HWBankIndex == bankIndex && pLastRun->BankOffset + pLastRun->Length == bankOffset && pLastRun->TransferOffset + pLastRun->Length == transferOffset)
	{
		pLastRun->Length++;
	}
	else
	{
		const FBankSet* pBankSet = pPCEEmu->GetBankSetPtr(bankIndex);
		FCDWriteRun run;
		run.HWBankIndex = bankIndex;
		run.BankId = pBankSet ? pBankSet->GetBankId() : -1;
		run.BankOffset = bankOffset;
		run.Length = 1;
		run.TransferOffset = transferOffset;
		transfer.WriteRuns.push_back(run);
	}

	// A decoded read has finished once every byte of the destination range has been written.
	if (transfer.Args.bDecoded && transfer.BytesWritten >= transfer.ExpectedBytes)
		EndTransfer(true);
}

void FCDROMAnalyser::CheckForTransferEnd(uint16_t nextPc)
{
	FCDROMTransfer& transfer = TransferLog[ActiveTransferIndex];

	// The read routine has returned and the caller's next instruction is about to run. Checking the next pc rather
	// than the one that just ran ends the read before the caller executes anything, so its writes aren't seen.
	// The return address comes from the debugger callstack so it's best effort.
	if (bActiveHasReturnAddr && nextPc == ActiveReturnPhysAddr && pPCEEmu->GetMemory()->GetMpr(nextPc >> 13) == ActiveReturnBankIndex)
	{
		if (transfer.Args.bDecoded)
		{
			// Still active, so not every expected byte arrived - otherwise RecordTransferWrite() would have ended it.
			transfer.bReturnedEarly = true;
			EndTransfer(false);
		}
		else
		{
			EndTransfer(true);
		}
		return;
	}

	if (pPCEEmu->GetCodeAnalysis().CurrentFrameNo - transfer.FrameNo > kMaxTransferFrames)
		EndTransfer(false);
}

void FCDROMAnalyser::EndTransfer(bool bCompleted)
{
	FCDROMTransfer& transfer = TransferLog[ActiveTransferIndex];
	transfer.bCompleted = bCompleted;
	transfer.EndFrameNo = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;

	// Apply even if incomplete - the captured bytes were still written.
	ApplyTransferToBanks(ActiveTransferIndex);
	ActiveTransferIndex = -1;

	if (transfer.bReturnedEarly)
		LOGWARNING("CD_READ from LBA %d: the read routine returned before all the data arrived. %u of %u bytes captured.", transfer.Args.LBAOffset, transfer.BytesWritten, transfer.ExpectedBytes);
	else if (!bCompleted)
		LOGWARNING("CD_READ from LBA %d: didn't finish before the next read or the timeout. %u bytes captured.", transfer.Args.LBAOffset, transfer.BytesWritten);
	if (transfer.Args.bDecoded && transfer.BytesWritten != transfer.ExpectedBytes)
		LOGWARNING("CD_READ from LBA %d: expected %u bytes but captured %u.", transfer.Args.LBAOffset, transfer.ExpectedBytes, transfer.BytesWritten);
	if (transfer.DuplicateWrites > 0)
		LOGWARNING("CD_READ from LBA %d: %u writes to destination bytes that had already been written (a retry?).", transfer.Args.LBAOffset, transfer.DuplicateWrites);
	if (transfer.Args.bDecoded && !transfer.WritesMatchSnapshot())
		LOGWARNING("CD_READ from LBA %d: the banks written don't match the mpr snapshot taken when the read started.", transfer.Args.LBAOffset);

	if (transfer.NonBiosWrites > 0)
		LOGWARNING("CD_READ from LBA %d: rejected %u destination writes made by code outside the BIOS.", transfer.Args.LBAOffset, transfer.NonBiosWrites);
}

// Update the disc origin of every bank range the transfer wrote. Any piece it overwrites is archived
// along with how it was used while it was in memory.
void FCDROMAnalyser::ApplyTransferToBanks(int transferIndex)
{
	const FCDROMTransfer& transfer = TransferLog[transferIndex];

	for (const FCDWriteRun& run : transfer.WriteRuns)
	{
		FCDBankState& bankState = BankStates[run.HWBankIndex];
		if (bUsageTrackingEnabled && bankState.Executed.empty())
		{
			bankState.Executed.resize(0x2000, 0);
			bankState.Read.resize(0x2000, 0);
		}

		const uint32_t runStart = run.BankOffset;
		const uint32_t runEnd = run.BankOffset + run.Length;	// runs never cross a bank boundary

		std::vector<FCDBankPiece> keptPieces;
		keptPieces.reserve(bankState.Pieces.size() + 2);	// at most one piece gets split in two, plus the new piece
		for (const FCDBankPiece& piece : bankState.Pieces)
		{
			const uint32_t pieceStart = piece.Start;
			const uint32_t pieceEnd = piece.Start + piece.Length;
			if (pieceEnd <= runStart || pieceStart >= runEnd)
			{
				keptPieces.push_back(piece);
				continue;
			}

			// Keep the parts either side of the overwritten range.
			if (pieceStart < runStart)
			{
				FCDBankPiece left = piece;
				left.Length = (uint16_t)(runStart - pieceStart);
				keptPieces.push_back(left);
			}
			if (pieceEnd > runEnd)
			{
				FCDBankPiece right = piece;
				right.Start = (uint16_t)runEnd;
				right.Length = (uint16_t)(pieceEnd - runEnd);
				right.TransferOffset = piece.TransferOffset + (runEnd - pieceStart);
				keptPieces.push_back(right);
			}

			// Archive the overwritten part. The history is only kept while usage tracking is enabled.
			if (bUsageTrackingEnabled)
			{
				const uint32_t overlapStart = pieceStart > runStart ? pieceStart : runStart;
				const uint32_t overlapEnd = pieceEnd < runEnd ? pieceEnd : runEnd;
				FCDBankPiece replaced = piece;
				replaced.Start = (uint16_t)overlapStart;
				replaced.Length = (uint16_t)(overlapEnd - overlapStart);
				replaced.TransferOffset = piece.TransferOffset + (overlapStart - pieceStart);
				replaced.ReplacedFrame = transfer.EndFrameNo;
				GetPieceUsage(replaced, replaced.ExecutedInstructions, replaced.BytesRead);
				ReplacedPieces.push_back(replaced);
			}
		}

		// The new bytes haven't been used yet.
		if (!bankState.Executed.empty())
		{
			for (uint32_t offset = runStart; offset < runEnd; offset++)
			{
				bankState.Executed[offset] = 0;
				bankState.Read[offset] = 0;
			}
		}

		FCDBankPiece newPiece;
		newPiece.HWBankIndex = run.HWBankIndex;
		newPiece.Start = (uint16_t)runStart;
		newPiece.Length = (uint16_t)run.Length;
		newPiece.LBAOffset = transfer.Args.LBAOffset;
		newPiece.TransferOffset = run.TransferOffset;
		newPiece.TransferIndex = transferIndex;
		newPiece.LoadFrame = transfer.EndFrameNo;
		// keptPieces is still sorted by Start (pieces were visited in order and split in place), so insert rather than sort.
		const auto insertPos = std::lower_bound(keptPieces.begin(), keptPieces.end(), newPiece.Start, [](const FCDBankPiece& piece, uint16_t start) { return piece.Start < start; });
		keptPieces.insert(insertPos, newPiece);
		bankState.Pieces = std::move(keptPieces);
		if (bUsageTrackingEnabled)
			bTrackingUsage = true;
	}
}

void FCDROMAnalyser::SetUsageTrackingEnabled(bool bEnabled)
{
	if (bEnabled == bUsageTrackingEnabled)
		return;

	bUsageTrackingEnabled = bEnabled;
	bTrackingUsage = false;
	for (FCDBankState& bankState : BankStates)
	{
		bankState.Executed.clear();
		bankState.Read.clear();
		bankState.Executed.shrink_to_fit();
		bankState.Read.shrink_to_fit();

		// Start tracking banks that already hold CD data. Usage only counts from now on.
		if (bEnabled && !bankState.Pieces.empty())
		{
			bankState.Executed.resize(0x2000, 0);
			bankState.Read.resize(0x2000, 0);
			bTrackingUsage = true;
		}
	}
}

void FCDROMAnalyser::RecordUsage(uint16_t physAddr, bool bExecuted)
{
	FCDBankState& bankState = BankStates[pPCEEmu->GetMemory()->GetMpr(physAddr >> 13)];
	if (bankState.Executed.empty())
		return;

	if (bExecuted)
		bankState.Executed[physAddr & 0x1fff] = 1;
	else
		bankState.Read[physAddr & 0x1fff] = 1;
}

void FCDROMAnalyser::GetPieceUsage(const FCDBankPiece& piece, uint32_t& outExecuted, uint32_t& outRead) const
{
	outExecuted = 0;
	outRead = 0;
	const FCDBankState& bankState = BankStates[piece.HWBankIndex];
	if (bankState.Executed.empty())
		return;

	for (uint32_t offset = piece.Start; offset < (uint32_t)piece.Start + piece.Length; offset++)
	{
		outExecuted += bankState.Executed[offset];
		outRead += bankState.Read[offset];
	}
}

bool FCDROMAnalyser::SaveBankPiecesCSV(const char* pFileName) const
{
	FILE* fp = fopen(pFileName, "w");
	if (fp == nullptr)
		return false;

	const FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	// Gather the overwritten pieces and the ones still in memory, with up to date usage for the latter.
	std::vector<FCDBankPiece> pieces = ReplacedPieces;
	for (const FCDBankState& bankState : BankStates)
	{
		for (FCDBankPiece piece : bankState.Pieces)
		{
			GetPieceUsage(piece, piece.ExecutedInstructions, piece.BytesRead);
			pieces.push_back(piece);
		}
	}
	std::sort(pieces.begin(), pieces.end(), [](const FCDBankPiece& a, const FCDBankPiece& b)
	{
		if (a.HWBankIndex != b.HWBankIndex)
			return a.HWBankIndex < b.HWBankIndex;
		if (a.LoadFrame != b.LoadFrame)
			return a.LoadFrame < b.LoadFrame;
		return a.Start < b.Start;
	});

	// Start, End and Length are offsets within the bank. DiscByte is the byte offset of Start from the start of the first data track.
	// ExecutedInstructions counts distinct addresses an instruction was executed from while the piece was in memory.
	// BytesRead counts distinct bytes read as data while the piece was in memory.
	fprintf(fp, "Bank,BankName,Start,End,Length,LBAOffset,TransferOffset,DiscByte,TransferIndex,LoadFrame,ReplacedFrame,InMemory,ExecutedInstructions,BytesRead\n");
	for (const FCDBankPiece& piece : pieces)
	{
		const FBankSet* pBankSet = pPCEEmu->GetBankSetPtr(piece.HWBankIndex);
		const FCodeAnalysisBank* pBank = pBankSet ? state.GetBank(pBankSet->GetBankId()) : nullptr;
		const bool bInMemory = piece.ReplacedFrame == -1;

		fprintf(fp, "0x%02x,%s,0x%04x,0x%04x,0x%x,%u,0x%x,0x%x,%d,%d,",
			piece.HWBankIndex, pBank ? pBank->Name.c_str() : "UNKNOWN", piece.Start, piece.Start + piece.Length - 1, piece.Length,
			piece.LBAOffset, piece.TransferOffset, piece.LBAOffset * 2048 + piece.TransferOffset, piece.TransferIndex, piece.LoadFrame);
		if (!bInMemory)
			fprintf(fp, "%d", piece.ReplacedFrame);
		fprintf(fp, ",%d,%u,%u\n", bInMemory ? 1 : 0, piece.ExecutedInstructions, piece.BytesRead);
	}

	fclose(fp);
	return true;
}

bool FCDROMTransfer::WritesMatchSnapshot() const
{
	// Every bank written must be in the snapshot.
	for (const FCDWriteRun& run : WriteRuns)
	{
		bool bFound = false;
		for (int b = 0; b < NumDestBanks; b++)
		{
			if (DestBankIndices[b] == run.HWBankIndex)
				bFound = true;
		}
		if (!bFound)
			return false;
	}

	// Every bank in the snapshot must have been written.
	for (int b = 0; b < NumDestBanks; b++)
	{
		bool bFound = false;
		for (const FCDWriteRun& run : WriteRuns)
		{
			if (run.HWBankIndex == DestBankIndices[b])
				bFound = true;
		}
		if (!bFound)
			return false;
	}

	return true;
}

// Export a csv of disc transfers for offline analysis.
// Replace this once cd rom transfers are persisted to disk?
bool FCDROMAnalyser::SaveTransferLogCSV(const char* pFileName) const
{
	FILE* fp = fopen(pFileName, "w");
	if (fp == nullptr)
		return false;

	const FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	// DestAddr and CallerAddr are physical addresses at the time of the read.
	// DestBankIndices and DestBanks are the banks mapped over the destination when the read started, separated by spaces.
	// WrittenBanks are the banks the captured writes actually landed in.
	// WriteRuns list each run of bytes written as bank:offset+length@transferoffset, all in hex.
	// ExpectedBytes is SizeBytes clamped to the end of the address space. BytesWritten should match it.
	fprintf(fp, "Index,Frame,Mode,Decoded,Track,LBAOffset,Sectors(al),bx,SizeBytes,ExpectedBytes,DestAddr,DestBankIndices,DestBanks,CallerBank,CallerAddr,Completed,EndFrame,BytesWritten,OtherWrites,MatchesSnapshot,DuplicateWrites,ReturnedEarly,NonBiosWrites,WrittenBanks,WriteRuns\n");
	for (int i = 0; i < (int)TransferLog.size(); i++)
	{
		const FCDROMTransfer& transfer = TransferLog[i];
		const FCodeAnalysisBank* pCallerBank = transfer.CallerAddr.IsValid() ? state.GetBank(transfer.CallerAddr.GetBankId()) : nullptr;

		fprintf(fp, "%d,%d,%d,%d,%d,%u,%u,0x%04x,%u,%u,",
			i, transfer.FrameNo, transfer.Args.Mode, transfer.Args.bDecoded ? 1 : 0, transfer.TrackIndex + 1, transfer.Args.LBAOffset, transfer.Args.RawAL, transfer.Args.RawBX, transfer.Args.TransferSizeBytes, transfer.ExpectedBytes);
		if (transfer.Args.bDecoded)
			fprintf(fp, "0x%04x", transfer.Args.PhysicalAddr);
		fprintf(fp, ",");
		for (int b = 0; b < transfer.NumDestBanks; b++)
			fprintf(fp, "%s0x%02x", b ? " " : "", transfer.DestBankIndices[b]);
		fprintf(fp, ",");
		for (int b = 0; b < transfer.NumDestBanks; b++)
		{
			const FCodeAnalysisBank* pBank = state.GetBank(transfer.DestBankIds[b]);
			fprintf(fp, "%s%s", b ? " " : "", pBank ? pBank->Name.c_str() : "UNKNOWN");
		}
		fprintf(fp, ",%s,", pCallerBank ? pCallerBank->Name.c_str() : "");
		if (transfer.CallerAddr.IsValid())
			fprintf(fp, "0x%04x", transfer.CallerPhysAddr);

		fprintf(fp, ",%d,%d,%u,%u,%d,%u,%d,%u,", transfer.bCompleted ? 1 : 0, transfer.EndFrameNo, transfer.BytesWritten, transfer.OtherWrites, transfer.WritesMatchSnapshot() ? 1 : 0, transfer.DuplicateWrites, transfer.bReturnedEarly ? 1 : 0, transfer.NonBiosWrites);

		// Distinct banks written, in the order they were first written.
		std::vector<uint8_t> writtenBanks;
		for (const FCDWriteRun& run : transfer.WriteRuns)
		{
			if (std::find(writtenBanks.begin(), writtenBanks.end(), run.HWBankIndex) == writtenBanks.end())
				writtenBanks.push_back(run.HWBankIndex);
		}
		for (int b = 0; b < (int)writtenBanks.size(); b++)
			fprintf(fp, "%s0x%02x", b ? " " : "", writtenBanks[b]);
		fprintf(fp, ",");

		for (int r = 0; r < (int)transfer.WriteRuns.size(); r++)
		{
			const FCDWriteRun& run = transfer.WriteRuns[r];
			fprintf(fp, "%s0x%02x:%04x+%x@%x", r ? " " : "", run.HWBankIndex, run.BankOffset, run.Length, run.TransferOffset);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
	return true;
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

// Fills in args for any read mode. Returns true if the mode was decoded (destination and size are valid).
// The raw arguments are always filled in so reads in unsupported modes can still be recorded.
bool GetBiosCDReadArgs(FPCEEmu* pEmu, FBiosCDReadArgs& args)
{
	const uint8_t mode = pEmu->ReadByte(_dh); // $ff
	const uint32_t numToRead = pEmu->ReadByte(_al); // $f8

	const uint8_t clByte = pEmu->ReadByte(_cl); // $fc
	const uint8_t chByte = pEmu->ReadByte(_ch); // $fd
	const uint8_t dlByte = pEmu->ReadByte(_dl); // $fe
	args.LBAOffset = dlByte + (chByte << 8) + (clByte << 16);

	args.Mode = mode;
	args.RawAL = (uint8_t)numToRead;
	args.RawBX = pEmu->ReadWord(_bx);
	args.bDecoded = false;

	switch (mode)
	{
		case 0: // Physical memory: size in bytes
			// todo
			break;
		case 1: // Physical memory: size in sectors
			args.Destination = EBiosCDReadDest::PhysicalMemory;
			args.PhysicalAddr = args.RawBX;
			args.TransferSizeBytes = numToRead * 2048;
			args.bDecoded = true;
			break;
		case 2: // MPR num
		case 3:
		case 4:
		case 5:
		case 6:
			// todo
			args.Destination = EBiosCDReadDest::MPRBank;
			break;
		case 0xfe: // VRAM
		case 0xff: // VRAM
			// todo
			args.Destination = EBiosCDReadDest::VRAM;
			break;
	}

	return args.bDecoded;
}