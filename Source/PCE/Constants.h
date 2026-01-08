struct FDebugLabel
{
	u16 Address;
	const char* Label;
};

// Taken from GearGrafx
static const int kDebugLabelCount = 43;
static const FDebugLabel kDebugLabels[kDebugLabelCount] =
{
	 { 0x0000, "VDC_ADDRESS" },
	 { 0x0002, "VDC_DATA_LO" },
	 { 0x0003, "VDC_DATA_HI" },
	 { 0x0400, "VCE_CONTROL" },
	 { 0x0402, "VCE_ADDR_LO" },
	 { 0x0403, "VCE_ADDR_HI" },
	 { 0x0404, "VCE_DATA_LO" },
	 { 0x0405, "VCE_DATA_HI" },
	 { 0x0800, "PSG_CH_SELECT" },
	 { 0x0801, "PSG_MAIN_VOL" },
	 { 0x0802, "PSG_FREQ_LO" },
	 { 0x0803, "PSG_FREQ_HI" },
	 { 0x0804, "PSG_CH_CTRL" },
	 { 0x0805, "PSG_CH_VOL" },
	 { 0x0806, "PSG_CH_DATA" },
	 { 0x0807, "PSG_NOISE" },
	 { 0x0808, "PSG_LFO_FREQ" },
	 { 0x0809, "PSG_LFO_CTRL" },
	 { 0x0C00, "TIMER_COUNTER" },
	 { 0x0C01, "TIMER_CONTROL" },
	 { 0x1000, "JOYPAD" },
	 { 0x1402, "IRQ_DISABLE" },
	 { 0x1403, "IRQ_STATUS" },
	 { 0x1800, "CD_STATUS" },
	 { 0x1801, "CD_DATA_BUS" },
	 { 0x1802, "CD_ENABLED_IRQS" },
	 { 0x1803, "CD_ACTIVE_IRQS" },
	 { 0x1804, "CD_RESET" },
	 { 0x1805, "CD_PCM_LSB" },
	 { 0x1806, "CD_PCM_MSB" },
	 { 0x1807, "CD_BRAM_UNLOCK" },
	 { 0x1808, "CD_DATA_ACK_ADPCM_LSB" },
	 { 0x1809, "CD_ADPCM_MSB" },
	 { 0x180A, "CD_ADPCM_DATA" },
	 { 0x180B, "CD_ADPCM_DMA" },
	 { 0x180C, "CD_ADPCM_STATUS" },
	 { 0x180D, "CD_ADPCM_CONTROL" },
	 { 0x180E, "CD_ADPCM_RATE" },
	 { 0x180F, "CD_AUDIO_FADER" },
	 { 0x18C0, "CD_SIGNATURE0" },
	 { 0x18C1, "CD_SIGNATURE1" },
	 { 0x18C2, "CD_SIGNATURE2" },
	 { 0x18C3, "CD_SIGNATURE3" }
};

// EX_MEMOPEN is missing
static const int kBiosSymbolCount = 76;
static const FDebugLabel kBiosJmpSymbols[kBiosSymbolCount] =
{
	// CD commands
	{ 0xE000, "CD_BOOT"     },
	{ 0xE003, "CD_RESET"    },
	{ 0xE006, "CD_BASE"     },
	{ 0xE009, "CD_READ"     },
	{ 0xE00C, "CD_SEEK"     },
	{ 0xE00F, "CD_EXEC"     },
	{ 0xE012, "CD_PLAY"     },
	{ 0xE015, "CD_SEARCH"   },
	{ 0xE018, "CD_PAUSE"    },
	{ 0xE01B, "CD_STAT"     },
	{ 0xE01E, "CD_SUBQ"     },
	{ 0xE021, "CD_DINFO"    },
	{ 0xE024, "CD_CONTENTS" },
	{ 0xE027, "CD_SUBRD"    },
	{ 0xE02A, "CD_PCMRD"    },
	{ 0xE02D, "CD_FADE"     },

	// ADPCM commands
	{ 0xE030, "AD_RESET"    },
	{ 0xE033, "AD_TRANS"    },
	{ 0xE036, "AD_READ"     },
	{ 0xE039, "AD_WRITE"    },
	{ 0xE03C, "AD_PLAY"     },
	{ 0xE03F, "AD_CPLAY"    },
	{ 0xE042, "AD_STOP"     },
	{ 0xE045, "AD_STAT"     },

	// Block manager (Save RAM)
	{ 0xE048, "BM_FORMAT"   },
	{ 0xE04B, "BM_FREE"     },
	{ 0xE04E, "BM_READ"     },
	{ 0xE051, "BM_WRITE"    },
	{ 0xE054, "BM_DELETE"   },
	{ 0xE057, "BM_FILES"    },

	// System extensions (I)
	{ 0xE05A, "EX_GETVER"   },
	{ 0xE05D, "EX_SETVEC"   },
	{ 0xE060, "EX_GETFNT"   },
	{ 0xE063, "EX_JOYSNS"   },
	{ 0xE066, "EX_JOYREP"   },
	{ 0xE069, "EX_SCRSIZ"   },

	// System extensions (II)
	{ 0xE06C, "EX_DOTMOD"   },
	{ 0xE06F, "EX_SCRMOD"   },
	{ 0xE072, "EX_IMODE"    },
	{ 0xE075, "EX_VMODE"    },
	{ 0xE078, "EX_HMODE"    },
	{ 0xE07B, "EX_VSYNC"    },
	{ 0xE07E, "EX_RCRON"    },
	{ 0xE081, "EX_RCROFF"   },
	{ 0xE084, "EX_IRQON"    },
	{ 0xE087, "EX_IRQOFF"   },
	{ 0xE08A, "EX_BGON"     },
	{ 0xE08D, "EX_BGOFF"    },
	{ 0xE090, "EX_SPRON"    },
	{ 0xE093, "EX_SPROFF"   },
	{ 0xE096, "EX_DSPON"    },
	{ 0xE099, "EX_DSPOFF"   },
	{ 0xE09C, "EX_DMAMOD"   },
	{ 0xE09F, "EX_SPRDMA"   },
	{ 0xE0A2, "EX_SATCLR"   },
	{ 0xE0A5, "EX_SPRPUT"   },
	{ 0xE0A8, "EX_SETRCR"   },
	{ 0xE0AB, "EX_SETRED"   },
	{ 0xE0AE, "EX_SETWRT"   },
	{ 0xE0B1, "EX_SETDMA"   },
	{ 0xE0B4, "EX_COLORCMD" },
	{ 0xE0B7, "EX_BINBCD"   },
	{ 0xE0BA, "EX_BCDBIN"   },
	{ 0xE0BD, "EX_RND"      },

	// Maths
	{ 0xE0C0, "MA_MUL8U"    },
	{ 0xE0C3, "MA_MUL8S"    },
	{ 0xE0C6, "MA_MUL16U"   },
	{ 0xE0C9, "MA_DIV16S"   },
	{ 0xE0CC, "MA_DIV16U"   },
	{ 0xE0CF, "MA_SQRT"     },
	{ 0xE0D2, "MA_SIN"      },
	{ 0xE0D5, "MA_COS"      },
	{ 0xE0D8, "MA_ATNI"     },

	// PSG
	{ 0xE0DB, "PSG_BIOS"    },
	{ 0xE0DE, "GRP_BIOS"    },
	{ 0xE0E1, "PSG_DRIVER"  }
};

// System Card 3.0 routine addresses.
// Info taken from https://www.stum.de/2025/pcenginebiosoffsets/
static const FDebugLabel kBiosRoutineSymbols[kBiosSymbolCount] =
{
	// CD commands
	{ 0xE0F3, "CD_BOOT"     },
	{ 0xE8E3, "CD_RESET"    },
	{ 0xEB8F, "CD_BASE"     },
	{ 0xEC05, "CD_READ"     },
	{ 0xEDCB, "CD_SEEK"     },
	{ 0xEBEC, "CD_EXEC"     },
	{ 0xEE10, "CD_PLAY"     },
	{ 0xEF34, "CD_SEARCH"   },
	{ 0xEF94, "CD_PAUSE"    },
	{ 0xF347, "CD_STAT"     },
	{ 0xEFBF, "CD_SUBQ"     },
	{ 0xEFF1, "CD_DINFO"    },
	{ 0xF0A9, "CD_CONTENTS" },
	{ 0xF354, "CD_SUBRD"    },
	{ 0xF364, "CD_PCMRD"    },
	{ 0xF379, "CD_FADE"     },

	// ADPCM commands
	{ 0xF37F, "AD_RESET"    },
	{ 0xF393, "AD_TRANS"    },
	{ 0xF407, "AD_READ"     },
	{ 0xF4D8, "AD_WRITE"    },
	{ 0xF5C6, "AD_PLAY"     },
	{ 0xF61F, "AD_CPLAY"    },
	{ 0xF6C1, "AD_STOP"     },
	{ 0xF6DB, "AD_STAT"     },

	// Block manager (Save RAM)
	{ 0xF858, "BM_FORMAT"   },
	{ 0xF8B8, "BM_FREE"     },
	{ 0xF8E3, "BM_READ"     },
	{ 0xF955, "BM_WRITE"    },
	{ 0xFA1A, "BM_DELETE"   },
	{ 0xFA72, "BM_FILES"    },

	// System extensions (I)
	{ 0xF02D, "EX_GETVER"   },
	{ 0xF034, "EX_SETVEC"   },
	{ 0xF124, "EX_GETFNT"   },
	{ 0xE49A, "EX_JOYSNS"   },
	{ 0xE175, "EX_JOYREP"   },
	{ 0xE267, "EX_SCRSIZ"   },

	// System extensions (II)
	{ 0xE272, "EX_DOTMOD"   },
	{ 0xE29D, "EX_SCRMOD"   },
	{ 0xE382, "EX_IMODE"    },
	{ 0xE391, "EX_VMODE"    },
	{ 0xE3A4, "EX_HMODE"    },
	{ 0xE3B5, "EX_VSYNC"    },
	{ 0xE3C7, "EX_RCRON"    },
	{ 0xE3CB, "EX_RCROFF"   },
	{ 0xE3CF, "EX_IRQON"    },
	{ 0xE3D3, "EX_IRQOFF"   },
	{ 0xE3E2, "EX_BGON"     },
	{ 0xE3E5, "EX_BGOFF"    },
	{ 0xE3E8, "EX_SPRON"    },
	{ 0xE3EB, "EX_SPROFF"   },
	{ 0xE3EE, "EX_DSPON"    },
	{ 0xE3F3, "EX_DSPOFF"   },
	{ 0xE3F8, "EX_DMAMOD"   },
	{ 0xE40B, "EX_SPRDMA"   },
	{ 0xE5DA, "EX_SATCLR"   },
	{ 0xE63C, "EX_SPRPUT"   },
	{ 0xE41F, "EX_SETRCR"   },
	{ 0xE42F, "EX_SETRED"   },
	{ 0xE446, "EX_SETWRT"   },
	{ 0xE45D, "EX_SETDMA"   },
	{ 0xE509, "EX_COLORCMD" },
	{ 0xE621, "EX_BINBCD"   },
	{ 0xE600, "EX_BCDBIN"   },
	{ 0xE67E, "EX_RND"      },

	// Maths
	{ 0xFDC6, "MA_MUL8U"    },
	{ 0xFDBF, "MA_MUL8S"    },
	{ 0xFDD4, "MA_MUL16U"   },
	{ 0xFDE2, "MA_DIV16S"   },
	{ 0xFDE9, "MA_DIV16U"   },
	{ 0xFDF0, "MA_SQRT"     },
	{ 0xFDFE, "MA_SIN"      },
	{ 0xFDF7, "MA_COS"      },
	{ 0xFE05, "MA_ATNI"     },

	// PSG
	{ 0xFE0C, "PSG_BIOS"    },
	{ 0xFE57, "GRP_BIOS"    },
	{ 0xFE92, "PSG_DRIVER" }
};