#pragma once

#include <vector>
#include "CodeAnalyser/CodeAnalyserTypes.h"
class FPCEEmu;

enum class EBiosCDReadDest
{
	PhysicalMemory,
	VRAM,
	MPRBank,
	Unknown
};

struct FBiosCDReadArgs
{
	EBiosCDReadDest Destination = EBiosCDReadDest::Unknown;
	uint32_t TransferSizeBytes = 0;	// 0 if the mode isn't decoded
	uint16_t PhysicalAddr = 0;		// Destination in physical memory, as it was at the time of the read. 0 if the mode isn't decoded.
	uint32_t LBAOffset = 0;			// CD sector number relative to the first data track
	bool bDecoded = false;			// False if the read mode isn't supported yet. Destination and size are unknown.

	// Raw BIOS arguments, recorded for every mode so unsupported modes can still be analysed.
	uint8_t Mode = 0;		// _dh
	uint8_t RawAL = 0;	// _al. Sector count for mode 1.
	uint16_t RawBX = 0;	// _bx. Destination address for mode 1.
};

// A run of consecutive bytes a transfer actually wrote into one bank.
struct FCDWriteRun
{
	uint8_t HWBankIndex = 0;		// PCE HW bank index (mpr value) mapped at the time of the write
	int16_t BankId = -1;				// Canonical analysis bank id for BankIndex
	uint16_t BankOffset = 0;		// Offset of the first byte within the bank
	uint32_t Length = 0;				// Number of bytes
	uint32_t TransferOffset = 0;	// Offset of the first byte within the transfer. Disc byte = LBAOffset * 2048 + TransferOffset
};

// A read from disc.
struct FCDROMTransfer
{
	static const int kMaxDestBanks = 8;

	FBiosCDReadArgs Args;	// What the BIOS was asked to read
	FAddressRef DestAddr = FAddressRef::Invalid(); // canonical ref to the start of where the data was written. Invalid if the mode isn't decoded.

	// Bytes that can land in the destination range: Args.TransferSizeBytes clamped to the end of the 64KB address space.
	// A decoded read finishes once this many bytes have been written.
	uint32_t ExpectedBytes = 0;
	int TrackIndex = -1; // 0-based index into the disc TOC

	// Every bank the transfer wrote to, in physical address order. A transfer can span several mpr slots.
	int NumDestBanks = 0;
	uint8_t DestBankIndices[kMaxDestBanks] = {};	// HW bank index (mpr value) mapped in each slot at the time of the read
	int16_t DestBankIds[kMaxDestBanks] = {};		// Canonical analysis bank id for each bank index

	int FrameNo = 0;	// code analysis frame number the read happened on
	FAddressRef CallerAddr = FAddressRef::Invalid(); // Game code that called the BIOS read routine
	uint16_t CallerPhysAddr = 0; // Physical address of the caller at the time of the read

	// Writes made by the BIOS code while the read was in progress.
	// For decoded modes they must also be inside the destination range. Other writes (e.g. the BIOS's own
	// variables, or interrupt handlers) are only counted in OtherWrites or NonBiosWrites.
	// For modes that aren't decoded, every BIOS write outside the hardware page and WRAM is recorded (best effort).
	std::vector<FCDWriteRun> WriteRuns;
	uint32_t BytesWritten = 0;	// Distinct destination bytes written. For modes that aren't decoded, every recorded write.
	uint32_t OtherWrites = 0;	// Writes not counted as transfer data
	
	// True if every expected byte arrived. Decoded reads finish as soon as they have. A read can also end when
	// execution reaches the caller's return address, on a timeout, or when the next read starts.
	bool bCompleted = false;
	int EndFrameNo = 0;

	// Detectors for cases that haven't been seen in Rondo. Each logs a warning when the read ends if it happens.
	uint32_t DuplicateWrites = 0;	// Decoded only: writes to destination bytes this read had already written (e.g. a retry)
	bool bReturnedEarly = false;	// Decoded only: the read routine returned before every expected byte arrived

	uint32_t NonBiosWrites = 0;		// Destination writes rejected because they weren't made by BIOS code

	// True if the banks the transfer actually wrote to match the mpr snapshot taken when the read started
	bool WritesMatchSnapshot() const;
};

typedef std::vector<FCDROMTransfer> TCDTransferList;

// A range of a bank whose contents came from one place on the disc.
struct FCDBankPiece
{
	uint8_t HWBankIndex = 0;		// PCE bank index (mpr value)
	uint16_t Start = 0;				// First byte within the bank
	uint16_t Length = 0;				// Number of bytes. A whole bank is 0x2000.
	uint32_t LBAOffset = 0;			// Sector the transfer started at, relative to the first data track
	uint32_t TransferOffset = 0;	// Offset of Start within the transfer. Disc byte = LBAOffset * 2048 + TransferOffset
	int TransferIndex = -1;			// Index into the transfer log
	int LoadFrame = 0;				// Frame the transfer finished
	int ReplacedFrame = -1;			// Frame another transfer overwrote this piece. -1 if it's still in memory.

	// Usage while the piece was in memory. Only filled in once a piece is archived - see GetPieceUsage() for live pieces.
	uint32_t ExecutedInstructions = 0;	// Distinct addresses an instruction was executed from
	uint32_t BytesRead = 0;					// Distinct bytes read as data
};

// Disc origin and usage tracking for one PCE bank.
struct FCDBankState
{
	std::vector<FCDBankPiece> Pieces;	// pieces currently in memory, sorted by Start
	// One flag per byte in the bank, set since the byte was last written by a CD transfer.
	// Only allocated once the bank has been written by a transfer.
	std::vector<uint8_t> Executed;
	std::vector<uint8_t> Read;
};

class FCDROMAnalyser
{
public:
	static const uint32_t kInvalidLBA = 0xffffffff;

	FCDROMAnalyser(FPCEEmu* pEmu);

	void Reset();

	void RegisterCDRead(const FBiosCDReadArgs& args);

	// Called for every memory write. pc is the start of the instruction doing the write. Records the write if a read is in progress.
	void OnMemoryWritten(uint16_t pc, uint16_t physAddr) { if (ActiveTransferIndex != -1) RecordTransferWrite(pc, physAddr); }
	
	// Called for every memory read. Records usage of bytes that came from the disc when usage tracking is enabled.
	// pc is the start of the executing instruction. Opcode and operand fetches also arrive here, so reads of the
	// instruction's own bytes are ignored. The longest HuC6280 instruction (the block transfers) is 7 bytes.
	void OnMemoryRead(uint16_t pc, uint16_t physAddr)
	{
		if (bTrackingUsage && (uint16_t)(physAddr - pc) >= 7)
			RecordUsage(physAddr, false);
	}
	// Called after every instruction. pc is the instruction that just executed, nextPc the one that will execute next.
	// Records usage of pc (when usage tracking is enabled) and detects reads finishing.
	void OnInstructionFinished(uint16_t pc, uint16_t nextPc)
	{
		if (bTrackingUsage)
			RecordUsage(pc, true);
		if (ActiveTransferIndex != -1)
			CheckForTransferEnd(nextPc);
	}
	bool IsTransferActive() const { return ActiveTransferIndex != -1; }

	const TCDTransferList& GetTransferLog() const { return TransferLog; }

	// Write every transfer, in the order they happened, to a CSV file.
	bool SaveTransferLogCSV(const char* pFileName) const;

	// Write every piece of every bank the CD has written, both still in memory and overwritten, to a CSV file.
	// The usage columns and the overwritten pieces are only recorded while usage tracking is enabled.
	bool SaveBankPiecesCSV(const char* pFileName) const;

	// Diagnostic: record which bytes loaded from the CD get executed or read, and keep a history of overwritten
	// pieces. Off by default because it costs a bank lookup on every instruction and every memory read.
	// Usage only counts from when it's enabled.
	void SetUsageTrackingEnabled(bool bEnabled);
	bool IsUsageTrackingEnabled() const { return bUsageTrackingEnabled; }

	uint32_t GetFirstDataTrackLBA() const { return FirstDataTrackLBA; }

private:
	void RecordTransferWrite(uint16_t pc, uint16_t physAddr);
	void CheckForTransferEnd(uint16_t nextPc);
	void EndTransfer(bool bCompleted);
	void ApplyTransferToBanks(int transferIndex);
	void RecordUsage(uint16_t physAddr, bool bExecuted);
	void GetPieceUsage(const FCDBankPiece& piece, uint32_t& outExecuted, uint32_t& outRead) const;

	FCDBankState BankStates[256];					// Indexed by PCE bank index
	std::vector<FCDBankPiece> ReplacedPieces;	// Pieces that have been overwritten by a later transfer. Only kept while usage tracking is enabled.
	bool bUsageTrackingEnabled = false;			// Diagnostic setting, see SetUsageTrackingEnabled()
	bool bTrackingUsage = false;					// True while usage tracking is enabled and a bank holds CD data. Checked on every instruction and read.

	TCDTransferList TransferLog;					// Every transfer in the order they happened

	// The transfer currently being read, as an index into TransferLog. -1 if none.
	int ActiveTransferIndex = -1;
	
	// Where the read routine returns to in the calling code. Used to detect the end of reads in modes that aren't
	// decoded - decoded reads end when all their bytes have been written. Taken from the debugger's callstack, so
	// it's best effort. The timeout catches reads where it's wrong.
	bool bActiveHasReturnAddr = false;
	uint16_t ActiveReturnPhysAddr = 0;
	uint8_t ActiveReturnBankIndex = 0;
	
	// One flag per destination byte of the active decoded read, so rewrites aren't counted twice. Reused between reads.
	std::vector<uint8_t> ActiveCoverage;

	uint32_t FirstDataTrackLBA = kInvalidLBA; // only 19 bits are used

	FPCEEmu* pPCEEmu = nullptr;
};

bool GetBiosCDReadArgs(FPCEEmu* pEmu, FBiosCDReadArgs& args);