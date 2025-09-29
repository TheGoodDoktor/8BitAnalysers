#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint8_t		status_reg;
	uint8_t		result_reg;
	uint8_t		command_reg;
	uint8_t		param_reg;
	uint8_t		reset_reg;
	uint8_t		data_reg;
	uint8_t		regs[8];
} fdc8271_t;

// 8271 FDC

void fdc8271_init(fdc8271_t* fdc);
void fdc8271_reset(fdc8271_t* fdc);
uint8_t fdc8271_read(fdc8271_t* fdc, uint8_t reg);
void fdc8271_write(fdc8271_t* fdc, uint8_t reg, uint8_t data);

#ifdef __cplusplus
} // extern "C"
#endif