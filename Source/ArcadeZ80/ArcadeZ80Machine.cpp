#include "ArcadeZ80Machine.h"

#include <string.h>
#include <assert.h>
#include <vector>
#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"
#include "CodeAnalyser/CodeAnalyser.h"

#define CHIPS_ASSERT(c) assert(c)

#define CPU_FREQUENCY (3000000)	// clock frequency in Hz 
#define VSYNC_PERIOD_4MHZ (CPU_FREQUENCY/60)
#define VBLANK_DURATION_4MHZ (((CPU_FREQUENCY/60)/525)*(525-483))

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

	// calculate number of ticks for VBlank (60Hz)
	//TicksPerFrame = clk_us_to_ticks(CPU_FREQUENCY, 16 * 1000);
	//FrameTicks = 0;
	
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
	if (Pins & Z80_MREQ)
	{
		// TODO: Handle memory mapped ports
		
		// a memory request
		const uint16_t addr = Z80_GET_ADDR(Pins);
		if (Pins & Z80_RD) 
		{
			uint8_t value = 0;

			if (addr == 0xC000)	// video scan line
			{ 
				value = 0;	// TODO:
			}
			
			else if (addr == 0xC300)	// IN0
			{
				value = 0xff;
			}
			else if (addr == 0xC320)	// IN1
			{
				value = 0xff;
			}
			else if (addr == 0xC340)	// IN2
			{
				value = 0xff;
			}
			else if (addr == 0xC360)	// DSW0
			{
				// Dip switch 0 - freeplay
				value = 0;
			}
			else if (addr == 0xC200)	// DSW1
			{
				// Dip switch 1
					// bits 0-1 : lives
					// bit	2	: Cocktail / Upright
					// bit  3	: Bonus life at 10k/50k or 20k/60k
					// bits 4-6 : difficulty 0-7, 0 = most difficult
					// bit	7	: Demo sounds on/off

				value = (3 << 0) |	// lives
					(0 << 2) |	// upright
					(0 << 3) |	// bonus life 10k/50k
					(4 << 4) |	// medium difficulty
					(1 << 7);	// demo sounds on 
			}
			else
			{ 
				value = mem_rd(&Memory, addr);
			}

			Z80_SET_DATA(Pins, value);
		}	
		else if (Pins & Z80_WR) 
		{
			if (addr == 0xC000)	// write audio command
			{

			}
			else if (addr == 0xC200)	// watchdog reset
			{
			}
			else if (addr == 0xC300)	// interrupt enable
			{
				NMIMask = (Z80_GET_DATA(Pins) & 0x01) ? true : false;
			}
			else if (addr == 0xC302)	// flip screen
			{
			}
			else if (addr == 0xC304)	// trigger audio interrupt
			{
			}
			else if (addr == 0xC308)	// video enable
			{
			}
			else if (addr == 0xC30A)	// coin counter 1
			{
			}
			else if (addr == 0xC30C)	// coin counter 2
			{
			}
			else
			{
				mem_wr(&Memory, addr, Z80_GET_DATA(Pins));
			}
		}
	}

	
}



void FArcadeZ80Machine::Tick()
{
	CHIPS_ASSERT(bValid);

	// activate NMI pin during VBLANK
	VSyncCount--;
	if (VSyncCount < 0) 
	{
		VSyncCount += VSYNC_PERIOD_4MHZ;
		VBlankCount = VBLANK_DURATION_4MHZ;
	}
	if (VBlankCount != 0) 
	{
		VBlankCount--;
		if (VBlankCount < 0) 
			VBlankCount = 0;
	}
	if (NMIMask && (VBlankCount > 0))
	{		
		Pins |= Z80_NMI;
	}
	else 
	{		
		Pins &= ~Z80_NMI;
	}

	TickCPU();

	TickCounter++;
}

// run arcade Z80 instance for given amount of micro_seconds, returns number of ticks executed
uint32_t FArcadeZ80Machine::Exec(uint32_t microSeconds)
{
	CHIPS_ASSERT(bValid);
	const uint32_t numTicks = clk_us_to_ticks(CPU_FREQUENCY, microSeconds);
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

	LoadBinaryFileToMem("Roms/TimePilot/tm4", SpriteROM, 0x2000);
	LoadBinaryFileToMem("Roms/TimePilot/tm5", SpriteROM + 0x2000, 0x2000);

	LoadBinaryFileToMem("Roms/TimePilot/tm6", TilesROM, 0x2000);

	// Code for Audio CPU
	LoadBinaryFileToMem("Roms/TimePilot/tm7", AudioROM, 0x1000);

	// PROMS
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.b4", PROMs + 0x0000, 0x0020);
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.b5", PROMs + 0x0020, 0x0020);
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.e9", PROMs + 0x0040, 0x0100);
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.e12", PROMs + 0x0140, 0x0100);
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