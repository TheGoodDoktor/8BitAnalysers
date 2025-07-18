#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m65C02.h"
#include "chips/mc6845.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"

#include <cstring>
#include <stdio.h>

#define TUBE_ELITE_SNAPSHOT_VERSION (1)


enum class ETubeRegister : uint8_t
{
	S1 = 0,	// Tube status register 1
	R1,		// Tube register 1
	S2,		// Tube status register 2
	R2,		// Tube register 2
	S3,		// Tube status register 3
	R3,		// Tube register 3
	S4,		// Tube status register 4
	R4,		// Tube register 4
};

class FTubeQueue
{
public:
	FTubeQueue()
		: QueuePos(0)
	{
		memset(Queue, 0, kQueueSize);
	}

	void Reset()
	{
		QueuePos = 0;
		memset(Queue, 0, kQueueSize);
	}

	void Empty()
	{
		QueuePos = 0;
	}

	bool HasSpace() const
	{
		return QueuePos < kQueueSize;
	}

	bool IsEmpty() const
	{
		return QueuePos == 0;
	}

	bool Enqueue(uint8_t value)
	{
		if (HasSpace())
		{
			Queue[QueuePos++] = value;
			return true;
		}
		return false;
	}

	const uint8_t* GetQueue() const
	{
		return Queue;
	}

	int GetQueueSize() const
	{
		return QueuePos;
	}

private:
	static const int	kQueueSize = 16;	
	uint8_t		Queue[kQueueSize];	
	int			QueuePos = 0;		
};

class FTube 
{
public:
	FTubeQueue		R1OutQueue;	
	uint8_t			R1InLatch = 0;

	bool			bR2OutLatchFull = false;	// R2 input ready flag
	uint8_t			R2OutLatch = 0;	// latch for R1 output
	bool GetR2Output(uint8_t &outByte) 
	{
		if (bR2OutLatchFull)
		{
			outByte = R2OutLatch;	// get the R2 output latch value
			bR2OutLatchFull = false;	// clear R2 output ready flag
			return true;	// return true if R2 output is ready
		}
		return false;
	}

	void SetR2Input(uint8_t val)
	{
		R2InLatch = val;
		bR2InReady = true;	// set R2 input ready flag
	}
	bool			bR2InReady = false;	// R2 input ready flag
	uint8_t			R2InLatch = 0;	// latch for R2 input
};

class ITubeDataHandler
{
public:
	virtual bool HandleIncomingR1Data(FTubeQueue& r1Queue) = 0;
	virtual bool HandleIncomingR2Data(uint8_t val) = 0;

};

struct FTubeEliteMachineDesc
{
	m65C02_desc_t		CPU;
	chips_debug_t		Debug;
	ITubeDataHandler*	pTubeDataHandler = nullptr;	// optional, can be used to handle incoming Tube data
};

class FTubeEliteMachine
{
public:
	bool Init(const FTubeEliteMachineDesc& desc);
	void Shutdown();
	void Reset();
	uint32_t Exec(uint32_t microSeconds);

	uint32_t SaveSnapshot(FILE* fp) const;
	bool LoadSnapshot(FILE* fp, uint32_t version);
private:
	void TickCPU();
	void Tick();

public:
	m65C02_t		CPU;
	uint64_t		Pins = 0;
	mem_t			Memory;	// cpu memory
	chips_debug_t	Debug;
	uint32_t		TickCounter;

	static const int kTubePollInterval = 100;	
	uint32_t		TubePollCounter = 0;

	bool		bValid = false;
	FTube		Tube;	// Tube registers

	// memory
	uint8_t		RAM[0x10000];		// 64K RAM

	ITubeDataHandler* pTubeDataHandler = nullptr;
};
