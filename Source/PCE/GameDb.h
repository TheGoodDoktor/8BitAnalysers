#include <vector>
#include <string>
#include <map>

class FPCEEmu;

struct FGameDbBank
{
	uint16_t GetMappedAddress() const { return MprSlot == -1 ? 0 : MprSlot * 0x2000; }

	// Is this bank mapped to a fixed mpr slot?
	// If this is false, the bank is dynamic and can be mapped to multiple mpr slots.
	bool bFixed = true;
	
	// todo make this a bitfield of all slots this banks has been mapped to?
	int MprSlot = -1;
};

struct FGameDbEntry
{
	std::vector<FGameDbBank> Banks;

	// these could be flags?
	bool bValidated = false;
	bool bAssemblesOk = false;
	bool bRomFilePartialMatch = false;
	bool bRomFileIdentical = false;
	bool bEmulatorTestOk = false;

	// Max simultaneous duplicate banks ever seen
	int MaxDupeBanks = 0;

	// Version 1 Criteria:
	// Run for 1800 frames (30 seconds) without joypad input

	// If this is set to -1 it means the criteria wasn't valid or automation wasn't active.
	int TestingMethodology = -1;

	// Number of banks that do not have a fixed physical memory address.
	// Note: this is not saved in the json file.
	int NumDynamicBanks = 0;
};

typedef std::map<std::string, FGameDbEntry> TGameDb;

FGameDbEntry* GetGameDbEntry(const std::string& name);

FGameDbEntry& CreateGameDbEntry(const std::string& name, int bankCount);
TGameDb& GetGameDb();

bool SaveGameDbEntry(const std::string& gameName, const std::string& fname);
bool LoadGameDbEntry(const std::string& gameName, const std::string& fname);
