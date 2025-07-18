#include "TubeEliteMachine.h"

#include <string.h>
#include <assert.h>
#include "Debug/DebugLog.h"

#define CHIPS_ASSERT(c) assert(c)

#define TUBE_FREQUENCY (3000000)	// clock frequency in Hz 


// initialize a new tube elite instance
bool FTubeEliteMachine::Init(const FTubeEliteMachineDesc& desc)
{
	if (desc.Debug.callback.func) 
	{ 
		CHIPS_ASSERT(desc.Debug.stopped); 
	}

	memset(&CPU, 0, sizeof(CPU));

	bValid = true;
	Debug = desc.Debug;

	// initialize the hardware
	Pins = m65C02_init(&CPU, &desc.CPU);
	mem_init(&Memory);

	// 64K RAM
	mem_map_ram(&Memory, 0, 0x0000, 0x10000, RAM);

	// Tube Handler
	pTubeDataHandler = desc.pTubeDataHandler;

	return true;
}

// discard a tube elite instance
void FTubeEliteMachine::Shutdown()
{
	CHIPS_ASSERT(bValid);
	bValid = false;
}

// reset a tube elite instance
void FTubeEliteMachine::Reset()
{
	CHIPS_ASSERT(bValid);

	Pins |= M6502_RES;
}

//uint64_t _tube_elite_tick_cpu(tube_elite_t* sys, uint64_t pins)
void FTubeEliteMachine::TickCPU()
{
	Pins = m65C02_tick(&CPU, Pins);

	// the IRQ and NMI pins will be set by the HW each tick
	Pins &= ~(M6502_IRQ | M6502_NMI);

	const uint16_t addr = M6502_GET_ADDR(Pins);

	// Tube registers
	if (addr >= 0xFEF8 && addr <= 0xFEFF)
	{
		const ETubeRegister reg = (ETubeRegister)(addr - 0xFEF8);
		if (Pins & M6502_RW)	// read
		{
			uint8_t val = 0;

			switch (reg)
			{
				case ETubeRegister::S1:
					// set bit 6 if queue is not empty
					if (Tube.R1OutQueue.HasSpace())
					{
						val |= 0x40; // set bit 6
					}
					break;
				case ETubeRegister::R1:
					// read from incoming R1 data
					val = Tube.R1InLatch;
					break;	
				case ETubeRegister::S2:
					if (Tube.bR2InReady)
					{
						val |= 0x80; // set bit 7 if R2 input is ready
					}
					if (Tube.bR2OutLatchFull == false)
					{
						val |= 0x40; // set bit 6 if R2 output is ready
					}
					break;
				case ETubeRegister::R2:
					// read from R2 input latch
					val = Tube.R2InLatch;
					Tube.bR2InReady = false; // clear R2 input ready flag
					break;
				case ETubeRegister::S3:
					// S3 is not implemented
					val = 0;
					break;
				case ETubeRegister::S4:
					// S4 is not implemented
					val = 0;
					break;
				default:
					LOGWARNING("Reading from unimplemented Tube Reg: %d",reg);
			}

			M6502_SET_DATA(Pins, val);
		}
		else // write
		{
			const uint8_t data = M6502_GET_DATA(Pins);
			switch (reg)
			{
			case ETubeRegister::S1:
				// TODO: what do we do?
				break;
			case ETubeRegister::R1:
				// write to R1 queue
				if (Tube.R1OutQueue.HasSpace())
				{
					Tube.R1OutQueue.Enqueue(data);
				}
				break;
			case ETubeRegister::R2:
				Tube.R2OutLatch = data;	// set R2 output latch
				Tube.bR2OutLatchFull = true;	// latch is full
				break;
			default:
				LOGWARNING("Writing to unimplemented Tube Reg: %d", reg);
			}

		}
	}
	else
	{ 
		// regular memory access
		if (Pins & M6502_RW)
		{
			M6502_SET_DATA(Pins, mem_rd(&Memory, addr));
		}
		else
		{
			mem_wr(&Memory, addr, M6502_GET_DATA(Pins));
		}
	}
}

//uint64_t _tube_elite_tick(tube_elite_t* sys, uint64_t pins)
void FTubeEliteMachine::Tick()
{
	CHIPS_ASSERT(bValid);

	// tick the CPU & CRTC at alternate cycles
	// not sure if we need this and it causes problems with stepping in the debugger
	//if ((sys->tick_counter & 1) == 0)
	{
		//pins = _tube_elite_tick_cpu(sys, pins);
		TickCPU();
	}

	TubePollCounter++;
	if (TubePollCounter == kTubePollInterval)
	{
		if (Tube.R1OutQueue.IsEmpty() == false)
		{
			// push data
			if(pTubeDataHandler)
				pTubeDataHandler->HandleIncomingR1Data(Tube.R1OutQueue);
		}

		uint8_t r2OutByte = 0;
		if (Tube.GetR2Output(r2OutByte))
		{
			if (pTubeDataHandler)
				pTubeDataHandler->HandleIncomingR2Data(r2OutByte);
		}
		TubePollCounter = 0;
	}
	
	TickCounter++;
}

// run tube elite instance for given amount of micro_seconds, returns number of ticks executed
uint32_t FTubeEliteMachine::Exec(uint32_t microSeconds)
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

uint32_t FTubeEliteMachine::SaveSnapshot(FILE* fp) const
{
	CHIPS_ASSERT(fp != nullptr);
	// save a snapshot, patches pointers to zero and offsets, returns snapshot version
	chips_debug_t saveDebug = Debug;
	m65C02_t saveCPU = CPU;
	mem_t saveMemory = Memory;

	chips_debug_snapshot_onsave(&saveDebug);
	m65C02_snapshot_onsave(&saveCPU);
	mem_snapshot_onsave(&saveMemory, (void*)this);

	fwrite(&saveDebug, sizeof(chips_debug_t), 1, fp);
	fwrite(&saveCPU, sizeof(m65C02_t), 1, fp);
	fwrite(&saveMemory, sizeof(mem_t), 1, fp);

	return TUBE_ELITE_SNAPSHOT_VERSION;
}

bool FTubeEliteMachine::LoadSnapshot(FILE* fp, uint32_t version)
{
	CHIPS_ASSERT(fp != nullptr);
	if (version != TUBE_ELITE_SNAPSHOT_VERSION)
	{
		return false;
	}

	chips_debug_t loadDebug;
	m65C02_t loadCPU;
	mem_t loadMemory;
	fread(&loadDebug, sizeof(chips_debug_t), 1, fp);
	fread(&loadCPU, sizeof(m65C02_t), 1, fp);
	fread(&loadMemory, sizeof(mem_t), 1, fp);
	chips_debug_snapshot_onload(&loadDebug, &Debug);
	m65C02_snapshot_onload(&loadCPU, &CPU);
	mem_snapshot_onload(&loadMemory, this);
	Debug = loadDebug;
	CPU = loadCPU;
	Memory = loadMemory;

	return true;
}
#if 0
// save a snapshot, patches pointers to zero and offsets, returns snapshot version
uint32_t tube_elite_save_snapshot(tube_elite_t* sys, tube_elite_t* dst)
{
	CHIPS_ASSERT(sys && dst);
	*dst = *sys;
	chips_debug_snapshot_onsave(&dst->debug);
	m65C02_snapshot_onsave(&dst->cpu);
	mem_snapshot_onsave(&dst->mem_cpu, sys);

	return TUBE_ELITE_SNAPSHOT_VERSION;
}

// load a snapshot, returns false if snapshot versions don't match
bool tube_elite_load_snapshot(tube_elite_t* sys, uint32_t version, tube_elite_t* src)
{
	CHIPS_ASSERT(sys && src);
	if (version != TUBE_ELITE_SNAPSHOT_VERSION)
	{
		return false;
	}
	static tube_elite_t im;
	im = *src;
	chips_debug_snapshot_onload(&im.debug, &sys->debug);
	m65C02_snapshot_onload(&im.cpu, &sys->cpu);
	mem_snapshot_onload(&im.mem_cpu, sys);
	*sys = im;
	return true;
}

#endif