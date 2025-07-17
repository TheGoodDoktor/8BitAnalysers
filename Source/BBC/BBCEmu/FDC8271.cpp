#include "FDC8271.h"

// 8271 Status register
const unsigned char STATUS_REG_COMMAND_BUSY = 0x80;
const unsigned char STATUS_REG_COMMAND_FULL = 0x40;
const unsigned char STATUS_REG_PARAMETER_FULL = 0x20;
const unsigned char STATUS_REG_RESULT_FULL = 0x10;
const unsigned char STATUS_REG_INTERRUPT_REQUEST = 0x08;
const unsigned char STATUS_REG_NON_DMA_MODE = 0x04;

// 8271 Result register
const unsigned char RESULT_REG_SUCCESS = 0x00;
const unsigned char RESULT_REG_SCAN_NOT_MET = 0x00;
const unsigned char RESULT_REG_SCAN_MET_EQUAL = 0x02;
const unsigned char RESULT_REG_SCAN_MET_NOT_EQUAL = 0x04;
const unsigned char RESULT_REG_CLOCK_ERROR = 0x08;
const unsigned char RESULT_REG_LATE_DMA = 0x0A;
const unsigned char RESULT_REG_ID_CRC_ERROR = 0x0C;
const unsigned char RESULT_REG_DATA_CRC_ERROR = 0x0E;
const unsigned char RESULT_REG_DRIVE_NOT_READY = 0x10;
const unsigned char RESULT_REG_WRITE_PROTECT = 0x12;
const unsigned char RESULT_REG_TRACK_0_NOT_FOUND = 0x14;
const unsigned char RESULT_REG_WRITE_FAULT = 0x16;
const unsigned char RESULT_REG_SECTOR_NOT_FOUND = 0x18;
const unsigned char RESULT_REG_DRIVE_NOT_PRESENT = 0x1E; // Undocumented, see http://beebwiki.mdfs.net/OSWORD_%267F
const unsigned char RESULT_REG_DELETED_DATA_FOUND = 0x20;
const unsigned char RESULT_REG_DELETED_DATA_CRC_ERROR = 0x2E;

// 8271 special registers
const unsigned char SPECIAL_REG_SCAN_SECTOR_NUMBER = 0x06;
const unsigned char SPECIAL_REG_SCAN_COUNT_MSB = 0x14;
const unsigned char SPECIAL_REG_SCAN_COUNT_LSB = 0x13;
const unsigned char SPECIAL_REG_SURFACE_0_CURRENT_TRACK = 0x12;
const unsigned char SPECIAL_REG_SURFACE_1_CURRENT_TRACK = 0x1A;
const unsigned char SPECIAL_REG_MODE_REGISTER = 0x17;
const unsigned char SPECIAL_REG_DRIVE_CONTROL_OUTPUT_PORT = 0x23;
const unsigned char SPECIAL_REG_DRIVE_CONTROL_INPUT_PORT = 0x22;
const unsigned char SPECIAL_REG_SURFACE_0_BAD_TRACK_1 = 0x10;
const unsigned char SPECIAL_REG_SURFACE_0_BAD_TRACK_2 = 0x11;
const unsigned char SPECIAL_REG_SURFACE_1_BAD_TRACK_1 = 0x18;
const unsigned char SPECIAL_REG_SURFACE_1_BAD_TRACK_2 = 0x19;

typedef void (*CommandFunc)();

typedef struct  
{
	unsigned char CommandNum;
	unsigned char Mask; // Mask command with this before comparing with CommandNum - allows drive ID to be removed
	int NParams; // Number of parameters to follow
	CommandFunc ToCall; // Called after all paameters have arrived
	CommandFunc IntHandler; // Called when interrupt requested by command is about to happen
	const char* Ident; // Mainly for debugging
} PrimaryCommandLookupType;


// 8271 FDC
void fdc8271_init(fdc8271_t* fdc)
{

}

void fdc8271_reset(fdc8271_t* fdc)
{

}

void _fdc8271_update_nmi_status(fdc8271_t* fdc)
{
}

uint8_t fdc8271_read(fdc8271_t* fdc, uint8_t reg)
{
	uint8_t val = 0;

	switch (reg)
	{
	case 0:	// Status register
		val = fdc->status_reg;
		break;

	case 1:	// Result register
	{
		// Clear interrupt request and result reg full flag
		fdc->status_reg &= ~(STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST);
		_fdc8271_update_nmi_status(fdc);
		val = fdc->result_reg;
		fdc->status_reg = RESULT_REG_SUCCESS; // Register goes to 0 after its read
	}
	break;

	case 4:	// Data register
		// Clear interrupt and non-dma request - not stated but DFS never looks at result reg!
		fdc->status_reg &= ~(STATUS_REG_INTERRUPT_REQUEST | STATUS_REG_NON_DMA_MODE);
		_fdc8271_update_nmi_status(fdc);
		val = fdc->data_reg;
		break;

	default:
		val = fdc->regs[reg];
		break;
	}

	return val;
}

void _fdc8271_command_reg_write(fdc8271_t* fdc, uint8_t data)
{
	fdc->command_reg = data;
	fdc->status_reg = STATUS_REG_COMMAND_BUSY | STATUS_REG_COMMAND_FULL;
}

void _fdc8271_param_reg_write(fdc8271_t* fdc, uint8_t data)
{

}

void _fdc8271_disk_reset(fdc8271_t* fdc)
{
}


void fdc8271_write(fdc8271_t* fdc, uint8_t reg, uint8_t data)
{
	switch (reg)
	{
	case 0:	// Command register
	{
		_fdc8271_command_reg_write(fdc, data);
	}
	break;

	case 1:	// Parameter register
	{
		_fdc8271_param_reg_write(fdc, data);
	}
	break;

	case 2: // reset
	{
		_fdc8271_disk_reset(fdc);
	}
	break;

	case 4:	// Data register
	{
		fdc->data_reg = data;
		fdc->status_reg &= ~(STATUS_REG_INTERRUPT_REQUEST | STATUS_REG_NON_DMA_MODE);
		_fdc8271_update_nmi_status(fdc);
	}
	break;

	default:
		fdc->regs[reg] = data;
		break;
	}
}