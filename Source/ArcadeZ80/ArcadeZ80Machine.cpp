#include "ArcadeZ80Machine.h"

#include <string.h>
#include <assert.h>
#include <vector>
#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"
#include "CodeAnalyser/CodeAnalyser.h"

#define CHIPS_ASSERT(c) assert(c)

#define TUBE_FREQUENCY (3000000)	// clock frequency in Hz 


// initialize a new tube elite instance
bool FArcadeZ80Machine::Init(const FArcadeZ80MachineDesc& desc)
{
	if (desc.Debug.callback.func) 
	{ 
		CHIPS_ASSERT(desc.Debug.stopped); 
	}

	memset(&CPU, 0, sizeof(CPU));

	bValid = true;
	Debug = desc.Debug;

	// initialize the hardware
	Pins = z80_init(&CPU);
	mem_init(&Memory);

	// 64K RAM
	//mem_map_ram(&Memory, 0, 0x0000, 0x10000, RAM);
	
	if (InitMachine(desc) == false)
	{
		LOGERROR("Unable to init specific machine");
		return false;
	}

	return true;
}

// discard a tube elite instance
void FArcadeZ80Machine::Shutdown()
{
	CHIPS_ASSERT(bValid);
	bValid = false;
}

// reset a tube elite instance
void FArcadeZ80Machine::Reset()
{
	CHIPS_ASSERT(bValid);

	Pins |= Z80_RES;
}

//uint64_t _tube_elite_tick_cpu(tube_elite_t* sys, uint64_t pins)
void FArcadeZ80Machine::TickCPU()
{
	Pins = z80_tick(&CPU, Pins);

	// TODOL: handle memory accesses
	

	
}

void FArcadeZ80Machine::Tick()
{
	CHIPS_ASSERT(bValid);

	// tick the CPU & CRTC at alternate cycles
	// not sure if we need this and it causes problems with stepping in the debugger
	//if ((sys->tick_counter & 1) == 0)
	{
		//pins = _tube_elite_tick_cpu(sys, pins);
		TickCPU();
	}
	
	TickCounter++;
}

// run arcade Z80 instance for given amount of micro_seconds, returns number of ticks executed
uint32_t FArcadeZ80Machine::Exec(uint32_t microSeconds)
{
	CHIPS_ASSERT(bValid);
	const uint32_t numTicks = clk_us_to_ticks(TUBE_FREQUENCY, microSeconds);
	if (Debug.callback.func == nullptr)
	{
		// run without debug hook
		for (uint32_t tick = 0; tick < numTicks; tick++)
		{
			Tick();
			//pins = _tube_elite_tick(sys, pins);
		}
	}
	else
	{
		// run with debug hook
		for (uint32_t tick = 0; (tick < numTicks) && !(*Debug.stopped); tick++)
		{
			//pins = _tube_elite_tick(sys, pins);
			Tick();
			Debug.callback.func(Debug.callback.user_data, Pins);
		}
	}

	//sys->pins = pins;

	return numTicks;
}

uint32_t FArcadeZ80Machine::SaveSnapshot(FILE* fp) const
{
	CHIPS_ASSERT(fp != nullptr);
	// save a snapshot, patches pointers to zero and offsets, returns snapshot version
	chips_debug_t saveDebug = Debug;
	z80_t saveCPU = CPU;
	mem_t saveMemory = Memory;

	chips_debug_snapshot_onsave(&saveDebug);
	//z80_snapshot_onsave(&saveCPU);
	mem_snapshot_onsave(&saveMemory, (void*)this);

	fwrite(&saveDebug, sizeof(chips_debug_t), 1, fp);
	fwrite(&saveCPU, sizeof(z80_t), 1, fp);
	fwrite(&saveMemory, sizeof(mem_t), 1, fp);

	return ARCADEZ80_SNAPSHOT_VERSION;
}

bool FArcadeZ80Machine::LoadSnapshot(FILE* fp, uint32_t version)
{
	CHIPS_ASSERT(fp != nullptr);
	if (version != ARCADEZ80_SNAPSHOT_VERSION)
	{
		return false;
	}

	chips_debug_t loadDebug;
	z80_t loadCPU;
	mem_t loadMemory;
	fread(&loadDebug, sizeof(chips_debug_t), 1, fp);
	fread(&loadCPU, sizeof(z80_t), 1, fp);
	fread(&loadMemory, sizeof(mem_t), 1, fp);
	chips_debug_snapshot_onload(&loadDebug, &Debug);
	//z80_snapshot_onload(&loadCPU, &CPU);
	mem_snapshot_onload(&loadMemory, this);
	Debug = loadDebug;
	CPU = loadCPU;
	Memory = loadMemory;

	return true;
}


// Time Pilot Specifics
bool FTimePilotMachine::InitMachine(const FArcadeZ80MachineDesc& desc)
{
	// TODO: Load ROMS etc.

	LoadBinaryFileToMem("Roms/TimePilot/tm1", ROM1, 0x2000);
	LoadBinaryFileToMem("Roms/TimePilot/tm2", ROM2, 0x2000);
	LoadBinaryFileToMem("Roms/TimePilot/tm3", ROM3, 0x2000);

	

	return true;
}

void FTimePilotMachine::SetupCodeAnalysisForMachine(FCodeAnalysisState& codeAnalysis)
{
	// Set up memory banks
	ROM1BankId = codeAnalysis.CreateBank("ROM1", 8, ROM1, true, 0x0000, true);
	ROM2BankId = codeAnalysis.CreateBank("ROM2", 8, ROM2, true, 0x2000, true);
	ROM3BankId = codeAnalysis.CreateBank("ROM3", 8, ROM3, true, 0x4000, true);

	// map in banks
	codeAnalysis.MapBank(ROM1BankId, 0, EBankAccess::ReadWrite);
	codeAnalysis.MapBank(ROM2BankId, 8, EBankAccess::ReadWrite);
	codeAnalysis.MapBank(ROM3BankId, 16, EBankAccess::ReadWrite);
	mem_map_rom(&Memory, 0, 0x0000, 0x2000, ROM1);
	mem_map_rom(&Memory, 0, 0x2000, 0x2000, ROM2);
	mem_map_rom(&Memory, 0, 0x4000, 0x2000, ROM3);

	// RAM
	RAMBankId = codeAnalysis.CreateBank("RAM", 40, RAM, false, 0x6000, true);					// RAM - $6000 - $FFFF - pages 24-63 - 40K
	codeAnalysis.MapBank(RAMBankId, 24, EBankAccess::ReadWrite);
	mem_map_ram(&Memory, 0, 0x6000, 40 * 1024, RAM);
}