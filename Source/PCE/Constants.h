struct FDebugLabel
{
	u16 Address;
	const char* Label;
	const char* Comment = nullptr;
};

// Taken from GearGrafx
static const int kDebugLabelCount = 43;
static const FDebugLabel kDebugLabels[kDebugLabelCount] =
{
	{ 0x0000, "VDC_ADDRESS",
		"--------------------------------------------------------------------------------\n"
		"                           VDC Index/Status Register\n"
		"--------------------------------------------------------------------------------\n"
		"Write: Select internal VDC register index (Bits 0-4, range 0-19).\n"
		"Read:  Status flags.\n"
		"  Bit 0: Sprite collision (sprite #0 hit any other sprite)\n"
		"  Bit 1: Sprite overflow (>16 sprites on a scanline)\n"
		"  Bit 2: Scanline IRQ (raster compare match)\n"
		"  Bit 3: VRAM-to-SATB DMA complete\n"
		"  Bit 4: VRAM-to-VRAM DMA complete\n"
		"  Bit 5: VBlank\n"
		"  Bit 6: Busy\n"
		"\n"
		"VDC Internal Registers (index written to VDC_ADDRESS, data via VDC_DATA_LO/HI):\n"
		"  $00 MAWR  VRAM write address pointer (auto-increments on VDC_DATA_HI write)\n"
		"  $01 MARR  VRAM read address pointer  (auto-increments on VDC_DATA_HI read)\n"
		"  $02 VRW   VRAM data read/write\n"
		"  $05 CR    Control:\n"
		"              Bit 0: Collision IRQ enable\n"
		"              Bit 1: Sprite overflow IRQ enable\n"
		"              Bit 2: Scanline IRQ enable\n"
		"              Bit 3: VBlank IRQ enable\n"
		"              Bit 6: Sprites enable\n"
		"              Bit 7: Background enable\n"
		"              Bits 12:11 - Address auto-increment: 00=+1 01=+32 10=+64 11=+128\n"
		"  $06 RCR   Raster Compare: scanline number for IRQ (scanline 64 = first visible line)\n"
		"  $07 BXR   Background X Scroll (Bits 0-9, range 0-1023)\n"
		"  $08 BYR   Background Y Scroll (Bits 0-8, range 0-511)\n"
		"  $09 MWR   BG Map Size (Bits 4-6): 000=32x32 001=64x32 010/011=128x32 100=32x64 101=64x64 110/111=128x64\n"
		"  $0A HSR   Horizontal Sync: Bits 0-4=sync pulse width, Bits 8-14=display start-1 [CAUTION: wrong values damage display]\n"
		"  $0B HDR   Horizontal Display: Bits 0-6=display width-1, Bits 8-14=display end-1\n"
		"  $0C VPR   Vertical Sync: Bits 0-4=sync pulse width, Bits 8-15=display start-2 [CAUTION]\n"
		"  $0D VDW   Vertical Display: Bits 0-8=display height-1\n"
		"  $0E VCR   Vertical Display End: Bits 0-7=end position\n"
		"  $0F DCR   DMA Control:\n"
		"              Bit 0: VRAM-SATB done IRQ enable\n"
		"              Bit 1: VRAM-VRAM done IRQ enable\n"
		"              Bit 2: Source address decrement\n"
		"              Bit 3: Destination address decrement\n"
		"              Bit 4: SATB auto-transfer each VBlank\n"
		"  $10 SOUR  DMA Source Address (VRAM-to-VRAM transfer)\n"
		"  $11 DESR  DMA Destination Address (VRAM-to-VRAM transfer)\n"
		"  $12 LENR  DMA Block Length in words (writing HIGH byte auto-starts the transfer)\n"
		"  $13 DVSSR VRAM-to-SATB Source Address (auto-starts at next VBlank; repeats each VBlank if DCR Bit 4 set)"
	},
	{ 0x0002, "VDC_DATA_LO",
		"--------------------------------------------------------------------------------\n"
		"                          VDC Data Register - Low Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Access to the 16-bit data of the currently selected VDC internal register.\n"
		"Reading returns the VRAM read latch (register $02 VRW).\n"
		"Writing sends the low byte to the selected register."
	},
	{ 0x0003, "VDC_DATA_HI",
		"--------------------------------------------------------------------------------\n"
		"                         VDC Data Register - High Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Upper byte of the 16-bit VDC data port.\n"
		"Writing this byte triggers VRAM address auto-increment (amount set by CR Bits 12:11: +1/+32/+64/+128 words)."
	},
	{ 0x0400, "VCE_CONTROL",
		"--------------------------------------------------------------------------------\n"
		"                              VCE Control Register\n"
		"--------------------------------------------------------------------------------\n"
		"  Bit 0: Dot clock frequency - 0 = 5.37 MHz, 1 = 7.16 MHz\n"
		"Higher dot clock allows wider display modes but reduces available rendering time."
	},
	{ 0x0402, "VCE_ADDR_LO",
		"--------------------------------------------------------------------------------\n"
		"                       VCE Color Table Address - Low Byte\n"
		"--------------------------------------------------------------------------------\n"
		"9-bit palette index (range 0-511). Write the low byte here.\n"
		"512 entries: 16 palettes x 16 colours for BG, plus 16 palettes x 16 colours for sprites.\n"
		"Index auto-increments after each write to VCE_DATA_HI."
	},
	{ 0x0403, "VCE_ADDR_HI",
		"--------------------------------------------------------------------------------\n"
		"                      VCE Color Table Address - High Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Bit 0: MSB (bit 8) of the 9-bit palette index. Upper bits ignored."
	},
	{ 0x0404, "VCE_DATA_LO",
		"--------------------------------------------------------------------------------\n"
		"                        VCE Color Table Data - Low Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Bits 7:0 of the 9-bit BGR colour value.\n"
		"  Bits 0-2: Blue (0-7)\n"
		"  Bits 3-5: Red (0-7)"
	},
	{ 0x0405, "VCE_DATA_HI",
		"--------------------------------------------------------------------------------\n"
		"                        VCE Color Table Data - High Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Full colour layout across both bytes (9 bits): Bits 0-2=Blue  Bits 3-5=Red  Bits 6-8=Green\n"
		"  Bits 0-2: Green MSBs (Bit 2 is the most significant green bit)\n"
		"Writing this byte commits the colour to the palette and auto-increments VCE_ADDR."
	},
	{ 0x0800, "PSG_CH_SELECT",
		"--------------------------------------------------------------------------------\n"
		"                               PSG Channel Select\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-2: Active channel number (0-5)\n"
		"All subsequent writes to PSG_FREQ, PSG_CH_CTRL, PSG_CH_VOL, PSG_CH_DATA, PSG_NOISE\n"
		"affect the selected channel."
	},
	{ 0x0801, "PSG_MAIN_VOL",
		"--------------------------------------------------------------------------------\n"
		"                               PSG Master Volume\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-3: Right output volume (0=mute, 15=max)\n"
		"  Bits 4-7: Left output volume  (0=mute, 15=max)"
	},
	{ 0x0802, "PSG_FREQ_LO",
		"--------------------------------------------------------------------------------\n"
		"                        PSG Channel Frequency - Low Byte\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-7: Lower 8 bits of the 12-bit frequency divider for the selected channel.\n"
		"Frequency (Hz) = 3,580,000 / (32 x freq_value). Lower value = higher pitch."
	},
	{ 0x0803, "PSG_FREQ_HI",
		"--------------------------------------------------------------------------------\n"
		"                       PSG Channel Frequency - High Byte\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-3: Upper 4 bits of the 12-bit frequency divider (Bits 11:8)."
	},
	{ 0x0804, "PSG_CH_CTRL",
		"--------------------------------------------------------------------------------\n"
		"                              PSG Channel Control\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-4: Channel volume (0=mute, 31=max)\n"
		"  Bit 6:   DDA (Direct Digital Audio) mode - outputs PSG_CH_DATA directly as 5-bit PCM\n"
		"  Bit 7:   Channel enable (1 = on)\n"
		"In DDA mode the waveform table is bypassed; raw 5-bit PCM samples written to PSG_CH_DATA\n"
		"are output directly, allowing arbitrary waveform or speech playback."
	},
	{ 0x0805, "PSG_CH_VOL",
		"--------------------------------------------------------------------------------\n"
		"                        PSG Channel Balance (Stereo Pan)\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-3: Right output level (0=mute, 15=max)\n"
		"  Bits 4-7: Left output level  (0=mute, 15=max)\n"
		"Combined with PSG_MAIN_VOL to determine the final output level."
	},
	{ 0x0806, "PSG_CH_DATA",
		"--------------------------------------------------------------------------------\n"
		"                      PSG Channel Waveform / DDA PCM Data\n"
		"--------------------------------------------------------------------------------\n"
		"Normal mode: Write 5-bit samples (Bits 4:0) sequentially to fill the 32-sample waveform table.\n"
		"  32 consecutive writes define one complete waveform period. The waveform then plays in a loop.\n"
		"DDA mode (PSG_CH_CTRL Bit 6 = 1): Each write outputs the 5-bit sample directly as PCM audio."
	},
	{ 0x0807, "PSG_NOISE",
		"--------------------------------------------------------------------------------\n"
		"                  PSG Noise Generator (channels 4 and 5 only)\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-4: Noise frequency (0=highest frequency, 31=lowest frequency)\n"
		"  Bit 7:   Noise enable for the selected channel (overrides the waveform table)\n"
		"Only effective when channel 4 or 5 is selected via PSG_CH_SELECT."
	},
	{ 0x0808, "PSG_LFO_FREQ",
		"--------------------------------------------------------------------------------\n"
		"                 PSG LFO (Low Frequency Oscillator) Frequency\n"
		"--------------------------------------------------------------------------------\n"
		"Sets the LFO tick rate used to modulate channel 0's frequency using channel 1's waveform output.\n"
		"Higher register value = slower LFO. Value 0 disables LFO frequency modulation."
	},
	{ 0x0809, "PSG_LFO_CTRL",
		"--------------------------------------------------------------------------------\n"
		"                                PSG LFO Control\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-1: LFO modulation depth shift: 00=no shift, 01=>>1, 10=>>4, 11=>>8\n"
		"  Bit 7:   LFO enable (1 = channel 1 waveform modulates channel 0 frequency)\n"
		"When enabled, channel 1 still outputs audio normally in addition to acting as the LFO source."
	},
	{ 0x0C00, "TIMER_COUNTER",
		"--------------------------------------------------------------------------------\n"
		"                               Timer Reload Value\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-6: 7-bit value (range 0-127) loaded into the countdown register.\n"
		"The timer decrements every 1024 CPU cycles (approx. 7 kHz at 7.16 MHz master clock).\n"
		"On underflow the counter reloads from this register and fires a timer IRQ.\n"
		"Writing any value here also immediately resets the current countdown."
	},
	{ 0x0C01, "TIMER_CONTROL",
		"--------------------------------------------------------------------------------\n"
		"                                  Timer Enable\n"
		"--------------------------------------------------------------------------------\n"
		"  Bit 0: Timer run: 1 = running, 0 = stopped\n"
		"Writing 1 when the timer is already running reloads the counter from TIMER_COUNTER.\n"
		"The timer IRQ must also be unmasked via IRQ_DISABLE Bit 2."
	},
	{ 0x1000, "JOYPAD",
		"--------------------------------------------------------------------------------\n"
		"                               Joypad / I/O Port\n"
		"--------------------------------------------------------------------------------\n"
		"Write:\n"
		"  Bit 0: CLR - resets the internal shift register; set SEL low\n"
		"  Bit 1: SEL - toggles between button groups (0=D-pad, 1=buttons)\n"
		"Read:\n"
		"  Bits 0-3: Button data (active low; 0 = pressed):\n"
		"    SEL=0: Bit 0=Left  Bit 1=Down  Bit 2=Right  Bit 3=Up\n"
		"    SEL=1: Bit 0=Run   Bit 1=Select  Bit 2=II   Bit 3=I\n"
		"  Bit 6: Country code (0 = Japan, 1 = USA/EU)\n"
		"  Bit 7: CD-ROM sense (0 = CD unit attached)\n"
		"For multi-tap adapters: cycle SEL to read up to 5 controllers sequentially."
	},
	{ 0x1402, "IRQ_DISABLE",
		"--------------------------------------------------------------------------------\n"
		"                                IRQ Disable Mask\n"
		"--------------------------------------------------------------------------------\n"
		"Write 1 to a bit to DISABLE the corresponding interrupt source:\n"
		"  Bit 0: IRQ2 (CD-ROM / external)\n"
		"  Bit 1: IRQ1 (VDC: VBlank, scanline compare, sprite overflow, sprite collision)\n"
		"  Bit 2: Timer IRQ\n"
		"Reset state: all bits 1 (all IRQs disabled). Clear a bit to enable the interrupt."
	},
	{ 0x1403, "IRQ_STATUS",
		"--------------------------------------------------------------------------------\n"
		"                        IRQ Pending Status / Acknowledge\n"
		"--------------------------------------------------------------------------------\n"
		"Read:  Pending (unserviced) interrupt flags using the same bit layout as IRQ_DISABLE.\n"
		"  Bit 0: IRQ2 pending\n"
		"  Bit 1: IRQ1 pending\n"
		"  Bit 2: Timer IRQ pending\n"
		"Write: Acknowledges (clears) the timer IRQ pending flag.\n"
		"VDC IRQ is cleared by reading VDC_ADDRESS (status register read)."
	},
	{ 0x1800, "CD_STATUS",
		"--------------------------------------------------------------------------------\n"
		"                        CD-ROM Controller Status (SCSI)\n"
		"--------------------------------------------------------------------------------\n"
		"Read: SCSI bus and controller status flags.\n"
		"  Bit 0: Data end\n"
		"  Bit 1: Data transfer ready\n"
		"  Bit 2: ADPCM busy\n"
		"  Bit 3: I/O direction (1=input to host, 0=output from host)\n"
		"  Bit 4: Cmd/Data (1=command, 0=data phase)\n"
		"  Bit 5: Msg (message phase)\n"
		"  Bit 6: Req (data requested)\n"
		"  Bit 7: Busy"
	},
	{ 0x1801, "CD_DATA_BUS",
		"--------------------------------------------------------------------------------\n"
		"                              CD-ROM SCSI Data Bus\n"
		"--------------------------------------------------------------------------------\n"
		"Read/Write: 8-bit SCSI data bus for sending commands and receiving data from the CD drive.\n"
		"Used together with CD_STATUS for SCSI bus handshaking."
	},
	{ 0x1802, "CD_ENABLED_IRQS",
		"--------------------------------------------------------------------------------\n"
		"                             CD-ROM IRQ Enable Mask\n"
		"--------------------------------------------------------------------------------\n"
		"  Bit 2: CD data end IRQ enable\n"
		"  Bit 3: CD data transfer ready IRQ enable\n"
		"  Bit 4: ADPCM DMA enable\n"
		"  Bit 5: ADPCM half-buffer IRQ enable\n"
		"  Bit 6: ADPCM playback end IRQ enable"
	},
	{ 0x1803, "CD_ACTIVE_IRQS",
		"--------------------------------------------------------------------------------\n"
		"                       CD-ROM Active (Pending) IRQ Flags\n"
		"--------------------------------------------------------------------------------\n"
		"Read:  Which CD-ROM interrupt flags are currently set (same bit layout as CD_ENABLED_IRQS).\n"
		"Write: Acknowledge / clear the flagged interrupts."
	},
	{ 0x1804, "CD_RESET",
		"--------------------------------------------------------------------------------\n"
		"                            CD-ROM Controller Reset\n"
		"--------------------------------------------------------------------------------\n"
		"Write any value to reset the CD-ROM controller and SCSI interface.\n"
		"Typically written during CD-ROM subsystem initialisation."
	},
	{ 0x1805, "CD_PCM_LSB",
		"--------------------------------------------------------------------------------\n"
		"                          CD Audio PCM Data - Low Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Low byte of the current 16-bit signed PCM sample from the CD audio stream.\n"
		"Read during VBlank to sample the audio output; used for software volume fading."
	},
	{ 0x1806, "CD_PCM_MSB",
		"--------------------------------------------------------------------------------\n"
		"                         CD Audio PCM Data - High Byte\n"
		"--------------------------------------------------------------------------------\n"
		"High byte of the 16-bit signed CD audio PCM sample (see CD_PCM_LSB)."
	},
	{ 0x1807, "CD_BRAM_UNLOCK",
		"--------------------------------------------------------------------------------\n"
		"                      Backup RAM (Save RAM) Lock / Unlock\n"
		"--------------------------------------------------------------------------------\n"
		"Write $80 to unlock BRAM (battery-backed save memory) for read/write access.\n"
		"Write $00 to lock it again. Always lock after use to prevent data corruption.\n"
		"BRAM is 2 KB, mapped at $1EE000-$1EE7FF when unlocked."
	},
	{ 0x1808, "CD_DATA_ACK_ADPCM_LSB",
		"--------------------------------------------------------------------------------\n"
		"                CD Data Acknowledge / ADPCM Address - Low Byte\n"
		"--------------------------------------------------------------------------------\n"
		"Write: Acknowledge a received CD data byte (SCSI handshake; clear Req flag).\n"
		"Also functions as the low byte of the 16-bit ADPCM RAM address register."
	},
	{ 0x1809, "CD_ADPCM_MSB",
		"--------------------------------------------------------------------------------\n"
		"                           ADPCM Address - High Byte\n"
		"--------------------------------------------------------------------------------\n"
		"High byte of the 16-bit ADPCM RAM address pointer.\n"
		"ADPCM RAM is 64 KB (address range $0000-$FFFF)."
	},
	{ 0x180A, "CD_ADPCM_DATA",
		"--------------------------------------------------------------------------------\n"
		"                              ADPCM RAM Data Port\n"
		"--------------------------------------------------------------------------------\n"
		"Read/Write: Access the byte at the current ADPCM address. Address auto-increments after each access.\n"
		"Used to load OKI/IMA ADPCM-encoded audio data (4 bits per sample) into ADPCM RAM."
	},
	{ 0x180B, "CD_ADPCM_DMA",
		"--------------------------------------------------------------------------------\n"
		"                                ADPCM DMA Enable\n"
		"--------------------------------------------------------------------------------\n"
		"  Bit 2: Enable DMA transfer from the CD-ROM data bus directly into ADPCM RAM.\n"
		"Set this before issuing a CD read command to stream audio data without CPU intervention."
	},
	{ 0x180C, "CD_ADPCM_STATUS",
		"--------------------------------------------------------------------------------\n"
		"                                  ADPCM Status\n"
		"--------------------------------------------------------------------------------\n"
		"  Bit 2: Half-way flag (playback has passed the mid-point of the buffer)\n"
		"  Bit 3: End flag (playback has reached the end address)\n"
		"  Bit 7: ADPCM busy (playback in progress)"
	},
	{ 0x180D, "CD_ADPCM_CONTROL",
		"--------------------------------------------------------------------------------\n"
		"                             ADPCM Playback Control\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-1: Latch end address from ADPCM address registers\n"
		"  Bits 2-3: Latch start address from ADPCM address registers\n"
		"  Bit 4:   Repeat / loop enable\n"
		"  Bit 5:   Play enable (1 = start or continue playback)\n"
		"  Bit 7:   Reset ADPCM decoder"
	},
	{ 0x180E, "CD_ADPCM_RATE",
		"--------------------------------------------------------------------------------\n"
		"                               ADPCM Sample Rate\n"
		"--------------------------------------------------------------------------------\n"
		"Sets the playback clock divider for the ADPCM decoder:\n"
		"  $00 = ~32 kHz   $01 = ~16 kHz   $02 = ~8 kHz   $03 = ~4 kHz\n"
		"Higher quality (higher rate) uses more RAM bandwidth during DMA streaming."
	},
	{ 0x180F, "CD_AUDIO_FADER",
		"--------------------------------------------------------------------------------\n"
		"                         CD / ADPCM Audio Volume Fader\n"
		"--------------------------------------------------------------------------------\n"
		"  Bits 0-1: ADPCM audio fade speed\n"
		"  Bits 2-3: CD audio fade speed\n"
		"Values: 00=no fade  01=fast fade  10=slow fade  11=immediate mute"
	},
	{ 0x18C0, "CD_SIGNATURE0",
		"--------------------------------------------------------------------------------\n"
		"    CD-ROM Presence Signature Byte 0 (reads $55 if CD-ROM unit is present)\n"
		"--------------------------------------------------------------------------------"
	},
	{ 0x18C1, "CD_SIGNATURE1",
		"--------------------------------------------------------------------------------\n"
		"    CD-ROM Presence Signature Byte 1 (reads $AA if CD-ROM unit is present)\n"
		"--------------------------------------------------------------------------------"
	},
	{ 0x18C2, "CD_SIGNATURE2",
		"--------------------------------------------------------------------------------\n"
		"    CD-ROM Presence Signature Byte 2 (reads $00 if CD-ROM unit is present)\n"
		"--------------------------------------------------------------------------------"
	},
	{ 0x18C3, "CD_SIGNATURE3",
		"--------------------------------------------------------------------------------\n"
		"    CD-ROM Presence Signature Byte 3 (reads $00 if CD-ROM unit is present)\n"
		"--------------------------------------------------------------------------------"
	},
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