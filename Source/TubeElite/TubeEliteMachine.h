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

// Key Codes
#define BBC_KEYCODE_CURSOR_LEFT	0x08
#define BBC_KEYCODE_CURSOR_RIGHT	0x09
#define BBC_KEYCODE_CURSOR_DOWN	0x0A
#define BBC_KEYCODE_CURSOR_UP	0x0B

#define BBC_KEYCODE_BACKSPACE	0x0C
#define BBC_KEYCODE_ENTER		0x0D
#define BBC_KEYCODE_SHIFT		0x0E
#define BBC_KEYCODE_CTRL		0x0F
#define BBC_KEYCODE_CAPS_LOCK	0x10
#define BBC_KEYCODE_SHIFT_LOCK	0x11
#define BBC_KEYCODE_ESCAPE		0x12
#define BBC_KEYCODE_SPACE		0x20
#define BBC_KEYCODE_F0			0xF0
#define BBC_KEYCODE_F1			0xF1
#define BBC_KEYCODE_F2			0xF2
#define BBC_KEYCODE_F3			0xF3
#define BBC_KEYCODE_F4			0xF4
#define BBC_KEYCODE_F5			0xF5
#define BBC_KEYCODE_F6			0xF6
#define BBC_KEYCODE_F7			0xF7
#define BBC_KEYCODE_F8			0xF8
#define BBC_KEYCODE_F9			0xF9


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

template<int kQueueSize>
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

	bool HasData() const
	{
		return QueuePos != 0;
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

	bool Dequeue(uint8_t& outByte)
	{
		if(QueuePos == 0)
			return false;
		outByte = Queue[0];

		// Shift the queue left
		for (int i = 1; i < QueuePos; ++i)
		{
			Queue[i - 1] = Queue[i];
		}

		QueuePos--;	// Decrease the queue position
		Queue[QueuePos] = 0;	// Clear the last position
		return true;
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
	uint8_t		Queue[kQueueSize];	
	int			QueuePos = 0;		
};

class FTubeLatch
{
public:
	FTubeLatch() = default;
	void Reset()
	{
		Value = 0;
		bHasData = false;
	}
	void SetValue(uint8_t val)
	{
		Value = val;
		bHasData = true;	// set has data flag
	}
	bool GetValue(uint8_t& outByte)
	{
		if (bHasData)
		{
			outByte = Value; // get the latch value
			bHasData = false; // clear has data flag
			return true; // return true if value is ready
		}
		return false; // return false if value is not ready
	}
	bool HasData() const
	{
		return bHasData; // return true if latch has data
	}
	bool HasSpace() const
	{
		return !bHasData;
	}
private:
	uint8_t Value = 0;	// latch value
	bool bHasData = false;	// has data
};

class FTube 
{
public:
	FTube() = default;

	void Reset()
	{
		R1OutQueue.Reset();
		R1InLatch.Reset();
		R2OutLatch.Reset();
		R2InLatch.Reset();
		R3OutQueue.Reset();
		R3InQueue.Reset();
		R4OutLatch.Reset();
		R4InLatch.Reset();
	}

	void	ParasiteWriteRegister(ETubeRegister reg, uint8_t val);
	bool	ParasiteReadRegister(ETubeRegister reg, uint8_t& outVal);

	void	HostWriteRegister(ETubeRegister reg, uint8_t val);
	bool	HostReadRegister(ETubeRegister reg, uint8_t& outVal);


	// R1
	FTubeQueue<24>	R1OutQueue;	// Tube R1 output queue
	FTubeLatch		R1InLatch;	// Tube R1 input latch

	// R2
	FTubeLatch		R2OutLatch;	// Tube R2 output latch
	FTubeLatch		R2InLatch;	// Tube R2 input latch

	// R3
	FTubeQueue<2>	R3OutQueue;	// Tube R3 output queue
	FTubeQueue<2>	R3InQueue;	// Tube R3 input queue

	// R4
	FTubeLatch		R4OutLatch;	// Tube R4 output latch
	FTubeLatch		R4InLatch;	// Tube R4 input latch
};

class ITubeDataHandler
{
public:
	virtual bool HandleIncomingByte(ETubeRegister reg, uint8_t val) = 0;
	virtual void PollTubeCommand(void) = 0;

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
	void FlushTube();

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
