#include "CRTFile.h"
#include <stdio.h>
#include <cinttypes>

#include <climits>
#include <vector>
#include <cassert>
#include "../C64Emulator.h"
#include <Debug/DebugLog.h>

template <typename T>
T swap_endian(T u)
{
	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

struct FCRTHeader
{
	char Signature[16];
	uint32_t	HeaderLength;
	uint8_t		VersionMajor;
	uint8_t		VersionMinor;
	uint16_t	CartridgeType;
	uint8_t		EXROMLine;
	uint8_t		GAMELine;
	uint8_t		HardwareRevision;
	char		Reserved[5];
	char		Name[32];
};

struct FChipPacketHeader
{
	char		Signature[4];
	uint32_t	PacketLength;
	uint16_t	ChipType;
	uint16_t	BankNumber;
	uint16_t	StartingLoadAddress;
	uint16_t	ROMSizeBytes;
};

ECartridgeType GetCartridgeType(int typeNo)
{
	switch (typeNo)
	{
	case 0:
		return ECartridgeType::Generic;
	case 19:
		return ECartridgeType::MagicDesk;
	case 32:
		return ECartridgeType::EasyFlash;
	default:
		return ECartridgeType::Unknown;
	}
}

ECartridgeSlot GetSlotFromAddress(uint16_t address)
{
	if (address == 0x8000)
		return ECartridgeSlot::RomLow;
	else
		return ECartridgeSlot::RomHigh;
}

ECartridgeMemoryModel GetMemoryModelForCartLines(bool bGameLine, bool bExROMLine)
{
	if (bExROMLine == false && bGameLine == false)
		return ECartridgeMemoryModel::Game16k;
	else if (bExROMLine == false && bGameLine == true)
		return ECartridgeMemoryModel::Game8k;
	else if (bExROMLine == true && bGameLine == false)
		return ECartridgeMemoryModel::Ultimax;
	else
		return ECartridgeMemoryModel::Ram;
}

// Cartridge Manager

bool FCartridgeManager::LoadCRTFile(const char* pFName)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	FILE *fp = fopen(pFName,"rb");
	if(fp == nullptr)
		return false;

	FCRTHeader	header;
	fread(&header,sizeof(FCRTHeader),1,fp);
	header.HeaderLength = swap_endian<uint32_t>(header.HeaderLength);
	header.CartridgeType = swap_endian<uint16_t>(header.CartridgeType);

	LOGINFO("Loaded CRT file %s", pFName);
	LOGINFO("Name: %s",header.Name);
	LOGINFO("CRT Version %d.%d", header.VersionMajor, header.VersionMinor);
	LOGINFO("Cartridge Type: %d",header.CartridgeType);
	LOGINFO("GAME Line: %d",header.GAMELine);	// $A000
	LOGINFO("EXROM Line: %d",header.EXROMLine);	// $8000
	LOGINFO("Hardware Revision: %d", header.HardwareRevision);

	// this is more reliable for generic cartridges - specific carts should set their own memory model in constructor
	InitialMemoryModel = GetMemoryModelForCartLines(header.GAMELine != 0, header.EXROMLine != 0);

	const ECartridgeType cartridgeType = GetCartridgeType(header.CartridgeType);
	CreateCartridgeHandler(cartridgeType);

	while(true)
	{
		FChipPacketHeader chipHeader;
		size_t noRead = fread(&chipHeader, sizeof(FChipPacketHeader), 1, fp);
		if(noRead == 0)
			break;

		assert(memcmp(chipHeader.Signature,"CHIP",4) == 0);

		chipHeader.PacketLength = swap_endian<uint32_t>(chipHeader.PacketLength);
		chipHeader.ChipType = swap_endian<uint16_t>(chipHeader.ChipType);
		chipHeader.BankNumber = swap_endian<uint16_t>(chipHeader.BankNumber);
		chipHeader.StartingLoadAddress = swap_endian<uint16_t>(chipHeader.StartingLoadAddress);
		chipHeader.ROMSizeBytes = swap_endian<uint16_t>(chipHeader.ROMSizeBytes);

		//if(cartridgeType == ECartridgeType::EasyFlash && chipHeader.StartingLoadAddress == 0xA000)	//hack
		//	chipHeader.StartingLoadAddress = 0xE000;

		LOGINFO("--CHIP SECTION--");
		LOGINFO("Chip bank number: %d", chipHeader.BankNumber);
		LOGINFO("Type: %d",chipHeader.ChipType);
		LOGINFO("Address: $%04X",chipHeader.StartingLoadAddress);
		LOGINFO("Size: %d bytes", chipHeader.ROMSizeBytes);

		// Create bank & read in data
		FCartridgeBank& bank = AddCartridgeBank(chipHeader.BankNumber,chipHeader.StartingLoadAddress,chipHeader.ROMSizeBytes);
		fread(bank.Data, chipHeader.ROMSizeBytes,1,fp);
	}
	GetCartridgeSlot(ECartridgeSlot::RomLow).CurrentBank = 0;
	GetCartridgeSlot(ECartridgeSlot::RomHigh).CurrentBank = 0;
	CurrentMemoryModel = InitialMemoryModel;
	MapSlotsForMemoryModel(CurrentMemoryModel);
	InitCartMapping();

	fclose(fp);
	return true;
}

void	FCartridgeManager::MapSlotsForMemoryModel(ECartridgeMemoryModel model)
{
	switch (model)
	{
	case ECartridgeMemoryModel::Game8k:
		MapSlotIn(ECartridgeSlot::RomLow, 0x8000);
		MapSlotOut(ECartridgeSlot::RomHigh);
		break;
	case ECartridgeMemoryModel::Game16k:
		MapSlotIn(ECartridgeSlot::RomLow, 0x8000);
		MapSlotIn(ECartridgeSlot::RomHigh, 0xA000);
		break;
	case ECartridgeMemoryModel::Ultimax:
		MapSlotIn(ECartridgeSlot::RomLow, 0x8000);
		MapSlotIn(ECartridgeSlot::RomHigh, 0xE000);
		break;
	case ECartridgeMemoryModel::Ram:
		MapSlotOut(ECartridgeSlot::RomLow);
		MapSlotOut(ECartridgeSlot::RomHigh);
		break;
	}
	
}


void FCartridgeManager::ResetCartridgeBanks()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	for (int slotNo = 0; slotNo < (int)ECartridgeSlot::Max; slotNo++)
	{
		FCartridgeSlot& slot = CartridgeSlots[slotNo];
		slot.Reset();
	}
	if (FirstCartridgeBankId != -1)
		state.FreeBanksFrom(FirstCartridgeBankId);
}



bool FCartridgeManager::Init(FC64Emulator* pEmu)
{
	pEmulator = pEmu;
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	FirstCartridgeBankId = state.GetNextBankId();			// Record first cartridge bank
	GetCartridgeSlot(ECartridgeSlot::RomLow).Init(0x8000, 0x2000);
	GetCartridgeSlot(ECartridgeSlot::RomHigh).Init(0xA000, 0x2000);
	return true;
}

void FCartridgeManager::OnMachineReset()
{
	CurrentMemoryModel = InitialMemoryModel;
	MapSlotsForMemoryModel(CurrentMemoryModel);
	InitCartMapping();
}

FCartridgeSlot& FCartridgeManager::GetCartridgeSlot(ECartridgeSlot slot)
{ 
	assert(slot != ECartridgeSlot::Unknown); 
	return CartridgeSlots[(int)slot]; 
}

FCartridgeBank& FCartridgeManager::AddCartridgeBank(int bankNo, uint16_t address, uint32_t dataSize)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	ECartridgeSlot slot = GetSlotFromAddress(address);
	assert(slot != ECartridgeSlot::Unknown);
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);
	//if (cartridgeSlot.Banks.size() <= bankNo)
	//	cartridgeSlot.Banks.resize(bankNo + 1);
	//assert(cartridgeSlot.Banks.size() == bankNo);	// assume they get added in a linear way

	//FCartridgeBank& bank = cartridgeSlot.Banks[bankNo];
	FCartridgeBank bank;
	//bank.BankNo = bankNo;
	//bank.DataSize = dataSize;
	bank.Data = new uint8_t[dataSize];
	//bank.Address = address;

	// Create Analyser Bank
	char bankName[32];
	snprintf(bankName, 32, "CartBank%d", bankNo);
	bank.BankId = state.CreateBank(bankName, dataSize / 1024, bank.Data, false, address);
	cartridgeSlot.Banks.push_back(bank);

	return cartridgeSlot.Banks.back();
}

void FCartridgeManager::InitCartMapping(void)
{
	for (int slotNo = 0; slotNo < (int)ECartridgeSlot::Max; slotNo++)
	{
		if (CartridgeSlots[slotNo].bActive == true)//Banks.empty() == false)
		{
			SetSlotBank((ECartridgeSlot)slotNo, 0);//find first bank instead?
		}
	}
}

// Map Slot in(address)/out
// Set Slot Bank
bool	FCartridgeManager::MapSlotIn(ECartridgeSlot slot, uint16_t address)
{
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);

	if(cartridgeSlot.bActive && cartridgeSlot.BaseAddress == address)	// already mapped
		return false;

	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	c64_t* pC64 = pEmulator->GetEmu();
	FCartridgeBank& bank = cartridgeSlot.Banks[cartridgeSlot.CurrentBank];

	// if we change the address of an active slot then map it out first
	if (cartridgeSlot.bActive && cartridgeSlot.BaseAddress != address)
	{
		MapSlotOut(slot);
	}

	// get the bankId of underlying RAM - could be a util function?
	const FC64BankIds& bankIds = pEmulator->GetBankIds();
	if (address == 0x8000)
		cartridgeSlot.RAMBank = bankIds.LowerRAM2;
	else if (address == 0xA000)
		cartridgeSlot.RAMBank = bankIds.RAMBehindBasicROM;
	else if (address == 0xE000)
		cartridgeSlot.RAMBank = bankIds.RAMBehindKernelROM;;

	// Map slot into memory
	cartridgeSlot.BaseAddress = address;

	// Map in memory page
	mem_map_rw(&pC64->mem_cpu, 0, cartridgeSlot.BaseAddress, cartridgeSlot.Size, bank.Data, &pC64->ram[cartridgeSlot.BaseAddress]);

	// Map in analysis
	state.MapBank(bank.BankId, cartridgeSlot.BaseAddress / 1024, EBankAccess::Read);

	cartridgeSlot.bActive = true;
	return true;
}

bool	FCartridgeManager::MapSlotOut(ECartridgeSlot slot)
{
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);

	if (cartridgeSlot.bActive == false)
		return false;

	// Map RAM in instead
	c64_t* pC64 = pEmulator->GetEmu();
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	mem_map_ram(&pC64->mem_cpu, 0, cartridgeSlot.BaseAddress, cartridgeSlot.Size, &pC64->ram[cartridgeSlot.BaseAddress]);

	// map in old RAM bank to the analysis
	state.MapBank(cartridgeSlot.RAMBank, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);

	cartridgeSlot.bActive = false;
	return true;
}

bool FCartridgeManager::SetSlotBank(ECartridgeSlot slot, int bankNo)
{
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);
	if(cartridgeSlot.CurrentBank == bankNo)
		return false;

	c64_t* pC64 = pEmulator->GetEmu();

	if (cartridgeSlot.bActive)	// only map memory if slot is active
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
		FCartridgeBank& bank = cartridgeSlot.Banks[bankNo];

		// Map in memory page
		mem_map_rw(&pC64->mem_cpu, 0, cartridgeSlot.BaseAddress, cartridgeSlot.Size, bank.Data, &pC64->ram[cartridgeSlot.BaseAddress]);

		// Map in analysis
		state.MapBank(bank.BankId, cartridgeSlot.BaseAddress / 1024, EBankAccess::Read);
	}

	cartridgeSlot.CurrentBank = bankNo;
	return true;
}

#if 0
bool FCartridgeManager::MapCartridgeBank(ECartridgeSlot slot, int bankNo)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	c64_t* pC64 = pEmulator->GetEmu();
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);
	FCartridgeBank& bank = cartridgeSlot.Banks[bankNo];

	// Map in memory page
	mem_map_rw(&pC64->mem_cpu, 0, bank.Address, bank.DataSize, bank.Data, &pC64->ram[bank.Address]);

	// Map in analysis
	state.MapBank(bank.BankId, bank.Address / 1024, EBankAccess::Read);

	cartridgeSlot.CurrentBank = bankNo;
	cartridgeSlot.bActive = true;

	return true;
}

void	FCartridgeManager::UnMapCartridge(ECartridgeSlot slot)
{
	c64_t* pC64 = pEmulator->GetEmu();
	const FC64BankIds& bankIds = pEmulator->GetBankIds();
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);

	mem_map_ram(&pC64->mem_cpu, 0, cartridgeSlot.BaseAddress, cartridgeSlot.Size, &pC64->ram[cartridgeSlot.BaseAddress]);

	if (slot == ECartridgeSlot::Addr_8000)
		state.MapBank(bankIds.LowerRAM2, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);
	else if (slot == ECartridgeSlot::Addr_A000)
		state.MapBank(bankIds.RAMBehindBasicROM, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);
	else if (slot == ECartridgeSlot::Addr_E000)
		state.MapBank(bankIds.RAMBehindKernelROM, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);

	cartridgeSlot.CurrentBank = -1;
	cartridgeSlot.bActive = false;
}
#endif

bool FCartridgeManager::HandleIOWrite(uint16_t address, uint8_t value)
{
	return pCartridgeHandler && pCartridgeHandler->HandleIOWrite(address, value);
}

bool FCartridgeManager::HandleIORead(uint16_t address, uint8_t& value)
{
	return (pCartridgeHandler && pCartridgeHandler->HandleIORead(address, value));
}

static const uint32_t kCartridgeMagic = 0xdeadcafe;
static const uint32_t kVersionNo = 1;

bool FCartridgeManager::SaveData(FILE* fp)
{
	// Write identifier & version
	fwrite(&kCartridgeMagic, sizeof(uint32_t), 1, fp);
	fwrite(&kVersionNo, sizeof(uint32_t), 1, fp);

	// Write Slots
	const uint32_t noSlots = (uint32_t)ECartridgeSlot::Max;
	fwrite(&noSlots, sizeof(uint32_t), 1, fp);
	for (int slotNo = 0; slotNo < (int)noSlots; slotNo++)
	{
		const FCartridgeSlot& slot = CartridgeSlots[slotNo];
		fwrite(&slot.BaseAddress, sizeof(uint16_t), 1, fp);
		fwrite(&slot.bActive, sizeof(bool), 1, fp);

		const uint32_t noCartBanks = (uint32_t)slot.Banks.size();
		fwrite(&noCartBanks, sizeof(uint32_t), 1, fp);
		for (int bankNo = 0; bankNo < (int)noCartBanks; bankNo++)
		{
			const FCartridgeBank& bank = slot.Banks[bankNo];
			fwrite(bank.Data, slot.Size, 1, fp);
		}
		fwrite(&slot.CurrentBank, sizeof(int), 1, fp);
		fwrite(&slot.RAMBank, sizeof(int16_t), 1, fp);
	}
	fwrite(&CartridgeType, sizeof(ECartridgeType), 1, fp);
	fwrite(&InitialMemoryModel, sizeof(ECartridgeMemoryModel), 1, fp);
	fwrite(&CurrentMemoryModel, sizeof(ECartridgeMemoryModel), 1, fp);
	return true;
}

bool FCartridgeManager::LoadData(FILE* fp)
{
	uint32_t magicVal = 0;

	// Write identifier & version
	fread(&magicVal, sizeof(uint32_t), 1, fp);
	if(magicVal != kCartridgeMagic)
		return false;

	uint32_t versionNo = 0;
	fread(&versionNo, sizeof(uint32_t), 1, fp);
	if(versionNo != kVersionNo)
		return false;

	uint32_t noCartSlots = 0;
	fread(&noCartSlots, sizeof(uint32_t), 1, fp);
	
	for (int slotNo = 0; slotNo < (int)noCartSlots; slotNo++)	// load each slot
	{
		FCartridgeSlot& slot = CartridgeSlots[slotNo];
		fread(&slot.BaseAddress, sizeof(uint16_t), 1, fp);
		fread(&slot.bActive, sizeof(bool), 1, fp);

		uint32_t noCartBanks = 0;
		fread(&noCartBanks, sizeof(uint32_t), 1, fp);
		for (int i = 0; i < (int)noCartBanks; i++)
		{
			FCartridgeBank& bank = AddCartridgeBank(i, slot.BaseAddress, slot.Size);
			fread(bank.Data, slot.Size, 1, fp);
		}

		fread(&slot.CurrentBank, sizeof(int), 1, fp);
		fread(&slot.RAMBank, sizeof(int16_t), 1, fp);

		if (slot.CurrentBank != -1)
			SetSlotBank((ECartridgeSlot)slotNo, slot.CurrentBank);
	}

	CartridgeType = ECartridgeType::Generic;
	fread(&CartridgeType, sizeof(ECartridgeType), 1, fp);
	fread(&InitialMemoryModel, sizeof(ECartridgeMemoryModel), 1, fp);
	fread(&CurrentMemoryModel, sizeof(ECartridgeMemoryModel), 1, fp);
	CreateCartridgeHandler(CartridgeType);
	return true;
	
}

// UI Code

const char* GetMemoryModelName(ECartridgeMemoryModel model)
{
	switch (model)
	{
		case ECartridgeMemoryModel::Game8k:
			return "16k Game";
		case ECartridgeMemoryModel::Game16k:
			return "16k Game";
		case ECartridgeMemoryModel::Ultimax:
			return "Ultimax";
		case ECartridgeMemoryModel::Ram:
			return "Ram/Off";
		default:
			return "Unknown";

	}
}

void DrawSlotUI(const FCartridgeSlot& slot)
{
	ImGui::Text("Address $ % 04X, $ % 04X bytes", slot.BaseAddress, slot.Size);
	ImGui::Text("%s", slot.bActive ? "Active" : "Inactive");
	ImGui::Text("Bank %d",slot.CurrentBank);
}

void FCartridgeManager::DrawUI(void)
{
	if (ImGui::Begin("Cartridge Manager"))
	{
		ImGui::Text("Initial Memory Model: %s", GetMemoryModelName(InitialMemoryModel));
		ImGui::Text("Current Memory Model: %s", GetMemoryModelName(CurrentMemoryModel));

		// Draw Info on slots
		ImGui::Separator();
		ImGui::Text("Cartridge Slots");
		ImGui::Separator();
		ImGui::Text("ROM Low Slot");
		DrawSlotUI(GetCartridgeSlot(ECartridgeSlot::RomLow));
		ImGui::Separator();
		ImGui::Text("ROM High Slot");
		DrawSlotUI(GetCartridgeSlot(ECartridgeSlot::RomHigh));
		ImGui::Separator();

		if(pCartridgeHandler)
			pCartridgeHandler->DrawUI();
	}
	ImGui::End();
}


// Cartridge handlers - other file?

class FMagicDeskCartridgeHandler : public FCartridgeHandler
{
public:
	FMagicDeskCartridgeHandler(FCartridgeManager* pManager):FCartridgeHandler(pManager){}

	bool	HandleIOWrite(uint16_t address, uint8_t value) override
	{
		if (address == 0xDE00)
		{
			if (value & (1 << 7))	// map RAM back in
			{
				pCartridgeManager->MapSlotsForMemoryModel(ECartridgeMemoryModel::Ram);
				pCartridgeManager->SetMemoryModel(ECartridgeMemoryModel::Ram);
			}
			else
			{
				pCartridgeManager->SetSlotBank(ECartridgeSlot::RomLow, value & 0x7f);
			}
		}

		return true;
	}

	void DrawUI(void)
	{
		ImGui::Text("MagicDesk Cartridge");
	}
};

class FEasyFlashCartridgeHandler : public FCartridgeHandler
{
public:
	FEasyFlashCartridgeHandler(FCartridgeManager* pManager) :FCartridgeHandler(pManager) 
	{
		MemoryModel = ECartridgeMemoryModel::Ultimax;
		pCartridgeManager->SetInitialMemoryModel(MemoryModel);
	}

	bool	HandleMachineReset() override 
	{ 
		MemoryModel = ECartridgeMemoryModel::Ultimax;
		pCartridgeManager->SetInitialMemoryModel(MemoryModel);
		return true; 
	}

	bool	HandleIOWrite(uint16_t address, uint8_t value) override
	{
		if (address == 0xDE00)
		{
			pCartridgeManager->SetSlotBank(ECartridgeSlot::RomLow, value);
			pCartridgeManager->SetSlotBank(ECartridgeSlot::RomHigh, value);
		}

		if (address == 0xDE02)
		{
			// Set up memory model
			const bool bGameLine = (value & 1) != 0;
			const bool bExROMLine = (value & 2) != 0;
			MemoryModel = GetMemoryModelForCartLines(bGameLine,bExROMLine);
			// What do we do now?
			pCartridgeManager->SetMemoryModel(MemoryModel);
			pCartridgeManager->MapSlotsForMemoryModel(MemoryModel);
		}

		if ((address & 0xFF00) == 0xDF00)	// RAM
		{
			RAM[address & 0xff] = value;
		}

		return true;
	}

	bool	HandleIORead(uint16_t address, uint8_t& value)	override
	{
		if ((address & 0xFF00) == 0xDF00)	// RAM
		{ 
			value =  RAM[address & 0xff];
			return true;
		}
		return false;
	}

	void DrawUI(void)
	{
		ImGui::Text("EasyFlash Cartridge");
	}

	ECartridgeMemoryModel	MemoryModel = ECartridgeMemoryModel::Unknown;
	uint8_t	RAM[256] = {0};
};

bool FCartridgeManager::CreateCartridgeHandler(ECartridgeType type)
{
	CartridgeType = type;
	pCartridgeHandler = nullptr;

	switch (type)
	{
	case ECartridgeType::Generic:
		return false;
	case ECartridgeType::MagicDesk:
		pCartridgeHandler = new FMagicDeskCartridgeHandler(this);
		break;
	case ECartridgeType::EasyFlash:
		pCartridgeHandler = new FEasyFlashCartridgeHandler(this);
		break;
	default:
		break;
	}

	return pCartridgeHandler != nullptr;
}