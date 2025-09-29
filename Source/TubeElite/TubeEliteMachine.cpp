#include "TubeEliteMachine.h"

#include <string.h>
#include <assert.h>
#include <vector>
#include "Debug/DebugLog.h"

#define CHIPS_ASSERT(c) assert(c)

#define TUBE_FREQUENCY (3000000)	// clock frequency in Hz 

void FTube::ParasiteWriteRegister(ETubeRegister reg, uint8_t val)
{
	switch (reg)
	{
	case ETubeRegister::R1:
		R1OutQueue.Enqueue(val); // write to R1 output queue
		break;
	case ETubeRegister::R2:
		R2OutLatch.SetValue(val); // set R2 output latch
		break;
	case ETubeRegister::R3:
		R3OutQueue.Enqueue(val); // write to R3 output queue
		break;
	case ETubeRegister::R4:
		R4OutLatch.SetValue(val); // set R4 output latch
		break;
	default:
		LOGWARNING("Writing to unimplemented Tube Reg: %d", reg);
		return; // unimplemented register
	}
}

bool FTube::ParasiteReadRegister(ETubeRegister reg, uint8_t& outVal) 
{
	outVal = 0; // default value

	switch (reg)
	{
	// Status Registers
	case ETubeRegister::S1:
		outVal |= R1InLatch.HasData() ? 0x80 : 0; // set bit 7 if R1 input is ready
		outVal |= R1OutQueue.HasSpace() ? 0x40 : 0;
		// TODO: other flags
		return true;
	case ETubeRegister::S2:
		outVal |= R2InLatch.HasData() ? 0x80 : 0;
		outVal |= R2OutLatch.HasSpace() ? 0x40 : 0;
		return true;
	case ETubeRegister::S3:
		outVal |= R3InQueue.HasData() ? 0x80 : 0;
		outVal |= R3OutQueue.HasSpace() ? 0x40 : 0;
		return true;
	case ETubeRegister::S4:
		outVal |= R4InLatch.HasData() ? 0x80 : 0;
		outVal |= R4OutLatch.HasSpace() ? 0x40 : 0;
		return true;

	// Data Registers
	case ETubeRegister::R1:
		return R1InLatch.GetValue(outVal); // read from R1 input latch
	case ETubeRegister::R2:
		return R2InLatch.GetValue(outVal); // read from R2 input latch
	case ETubeRegister::R3:
		return R3InQueue.Dequeue(outVal); // read from R3 input queue
	case ETubeRegister::R4:
		return R4InLatch.GetValue(outVal); // read from R4 input latch
	default:
		LOGWARNING("Reading from unimplemented Tube Reg: %d", reg);
		return false; // unimplemented register
	}
}

bool FTube::HostWriteRegister(ETubeRegister reg, uint8_t val)
{
	switch (reg)
	{
	case ETubeRegister::R1:
		if(R1InLatch.HasSpace() == false)
			return false;
		R1InLatch.SetValue(val); // write to R1 input latch
		return true;
		break;
	case ETubeRegister::R2:
		if (R2InLatch.HasSpace() == false)
			return false;
		R2InLatch.SetValue(val); // set R2 input latch
		return true;
		break;
	case ETubeRegister::R3:
		if (R3InQueue.HasSpace() == false)
			return false; // no space in R3 input queue
		R3InQueue.Enqueue(val); // write to R3 input queue
		return true;
		break;
	case ETubeRegister::R4:
		if (R4InLatch.HasSpace() == false)
			return false; // no space in R4 input latch
		R4InLatch.SetValue(val); // set R4 input latch
		return true;
		break;
	default:
		LOGWARNING("Writing to unimplemented Tube Reg: %d", reg);
		return false; // unimplemented register
	}
}

bool FTube::HostReadRegister(ETubeRegister reg, uint8_t& outVal)
{
	outVal = 0; // default value

	switch (reg)
	{
		// Status Registers
	case ETubeRegister::S1:
		outVal |= R1OutQueue.HasData() ? 0x80 : 0; // set bit 7 if R1 input is ready
		outVal |= R1InLatch.HasSpace() ? 0x40 : 0;
		// TODO: other flags
		return true;
	case ETubeRegister::S2:
		outVal |= R2OutLatch.HasData() ? 0x80 : 0;
		outVal |= R2InLatch.HasSpace() ? 0x40 : 0;
		return true;
	case ETubeRegister::S3:
		outVal |= R3OutQueue.HasData() ? 0x80 : 0;
		outVal |= R3InQueue.HasSpace() ? 0x40 : 0;
		return true;
	case ETubeRegister::S4:
		outVal |= R4OutLatch.HasData() ? 0x80 : 0;
		outVal |= R4InLatch.HasSpace() ? 0x40 : 0;
		return true;

		// Data Registers
	case ETubeRegister::R1:
		return R1OutQueue.Dequeue(outVal); // read from R1 output queue
	case ETubeRegister::R2:
		return R2OutLatch.GetValue(outVal); // read from R2 output latch
	case ETubeRegister::R3:
		return R3OutQueue.Dequeue(outVal); // read from R3 output queue
	case ETubeRegister::R4:
		return R4OutLatch.GetValue(outVal); // read from R4 output latch
	default:
		LOGWARNING("Reading from unimplemented Tube Reg: %d", reg);
		return false; // unimplemented register
	}
}

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
			Tube.ParasiteReadRegister(reg,val);
			M6502_SET_DATA(Pins, val);
		}
		else // write
		{
			const uint8_t data = M6502_GET_DATA(Pins);
			Tube.ParasiteWriteRegister(reg,data);
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

void FTubeEliteMachine::FlushTube()
{
	uint8_t inByte = 0;
	const ETubeRegister dataRegs[] = {ETubeRegister::R1, ETubeRegister::R2, ETubeRegister::R3, ETubeRegister::R4};
	for(int i=0;i<4;i++)
	{
		while (Tube.HostReadRegister(dataRegs[i], inByte))	// while is for flushing queues
		{
			if(pTubeDataHandler)
				pTubeDataHandler->HandleIncomingByte(dataRegs[i], inByte);
		}
	}

	// TODO: handle IRQs from Tube HW
	uint8_t val = 0;
	if (Tube.ParasiteReadRegister(ETubeRegister::R4, val))
	{
		if (val & 0x80)	// if bit 7 is set, request an IRQ
			R4IRQ = true;
	}

	if (R4IRQ)
		Pins |= M6502_IRQ;

	if(CPU.brk_flags & M6502_BRK_IRQ)
		R4IRQ = false;

	if(pTubeDataHandler)
		pTubeDataHandler->PollTubeCommand();	// poll for any pending commands
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
	//if (TubePollCounter == kTubePollInterval)
	{
		FlushTube();	// flush the Tube data
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

	FlushTube();	// flush the Tube data
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