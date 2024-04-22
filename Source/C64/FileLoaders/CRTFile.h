#pragma once

#include <stdio.h>
#include <cinttypes>
#include <vector>

class FC64Emulator;
class FCartridgeManager;
class FCartridgeHandler;

struct FCartridgeBankCreate;

enum class ECartridgeType
{
	None,
	Generic,
	MagicDesk,
	EasyFlash,

	Unknown,
};

enum class ECartridgeSlot
{
	RomLow,
	RomHigh,

	Unknown,
	Max = Unknown
};

enum class ECartridgeMemoryModel
{
	Game16k,
	Game8k,
	Ultimax,
	Ram,	// no cart mapping

	Unknown
};


enum class ELoadDataResult
{
	OK,
	NotFound,
	InvalidData
};


class FCartridgeHandler
{
public:
			FCartridgeHandler(FCartridgeManager* pManager):pCartridgeManager(pManager){}
			virtual ~FCartridgeHandler(){}

			virtual bool	HandleMachineReset() { return false; }
			virtual bool	HandleIOWrite(uint16_t address, uint8_t value) {return false;}
			virtual bool	HandleIORead(uint16_t address, uint8_t& value) { return false;};

			virtual void	DrawUI(void) {};

protected:
	FCartridgeManager* pCartridgeManager = nullptr;
};

// information on cartridge bank
struct FCartridgeBank
{
	//~FCartridgeBank() { delete[] Data; Data = nullptr; }

	int			BankNo = -1;
	int16_t		BankId = -1;
	//uint16_t	Address = 0;
	//uint32_t	DataSize = 0;
	uint8_t* Data = nullptr;
};

struct FCartridgeSlot
{
	void Init(uint16_t baseAddress, uint16_t size)
	{
		BaseAddress = baseAddress;
		Size = size;
	}
	void Reset()
	{
		for (auto& bank : Banks)
			delete[] bank.Data;

		Banks.clear();
		CurrentBank = -1;
	}

	const FCartridgeBank& GetBank(int bankNo) const 
	{
		for (auto& bank : Banks)
		{
			if(bank.BankNo == bankNo)
				return bank;
		}
		return Banks[bankNo];
	}

	bool		bActive = false;
	uint16_t	BaseAddress = 0;	// where slot is mapped in address space
	uint16_t	Size = 0;			// size in bytes
	int16_t		RAMBank = -1;		// RAM bank behind

	int			CurrentBank = -1;
	std::vector<FCartridgeBank>	Banks;
};

class FCartridgeManager
{
public:
	friend class FCartridgeHandler;

	bool	Init(FC64Emulator* pEmu);

	void	MapSlotsForMemoryModel();
	bool	LoadCRTFile(const char* pFName);
	void	ResetCartridgeBanks();
	void	OnMachineReset();

	FCartridgeSlot& GetCartridgeSlot(ECartridgeSlot slot);
	FCartridgeBank& AddCartridgeBankToSlot(ECartridgeSlot slot, const FCartridgeBankCreate& create);

	void	SetInitialMemoryModel(ECartridgeMemoryModel model) { InitialMemoryModel = model; }
	void	SetMemoryModel(ECartridgeMemoryModel model) { CurrentMemoryModel = model; }
	void	InitCartMapping(void);

	bool	MapSlotIn(ECartridgeSlot slot, uint16_t address);
	bool	MapSlotOut(ECartridgeSlot slot);
	bool	SetSlotBank(ECartridgeSlot slot, int bankNo);

	//bool	MapCartridgeBank(ECartridgeSlot slot, int bankNo);
	//void	UnMapCartridge(ECartridgeSlot slot);

	bool	HandleIOWrite(uint16_t address, uint8_t value);
	bool	HandleIORead(uint16_t address, uint8_t& value);


	void	WriteSlotToFile(const FCartridgeSlot& slot, FILE* fp);
	void	ReadSlotFromFile(FCartridgeSlot& slot, FILE* fp);
	ELoadDataResult	LoadData(FILE* fp);
	bool	SaveData(FILE* fp);

	void	DrawUI(void);

private:
	bool	CreateCartridgeHandler(ECartridgeType type);
	void	SetHandler(FCartridgeHandler* pHandler) { delete pCartridgeHandler; pCartridgeHandler = pHandler; }

private:
	FC64Emulator*		pEmulator;
	FCartridgeHandler*	pCartridgeHandler = nullptr;
	ECartridgeMemoryModel	InitialMemoryModel = ECartridgeMemoryModel::Unknown;
	ECartridgeMemoryModel	CurrentMemoryModel = ECartridgeMemoryModel::Unknown;
	ECartridgeType		CartridgeType = ECartridgeType::None;
	FCartridgeSlot		CartridgeSlots[(int)ECartridgeSlot::Max];
	//FCartridgeSlot		UltimaxSlot;
	int16_t				FirstCartridgeBankId = -1;
};

