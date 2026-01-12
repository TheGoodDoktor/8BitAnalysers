	DEVICE ZXSPECTRUMNEXT


; ROM Labels

; Screen Memory Labels
data_6000: 	equ $6000
ColourRAM: 	equ $A000
VideoRAM: 	equ $A400
data_A800: 	equ $A800
data_A801: 	equ $A801
PlayerAngle: 	equ $A802
data_A808: 	equ $A808
data_A80A: 	equ $A80A
data_A811: 	equ $A811
FrameCountdown2: 	equ $A812
data_A814: 	equ $A814
FrameCountdown1: 	equ $A817
data_A821: 	equ $A821
data_A827: 	equ $A827
data_A837: 	equ $A837
data_A840: 	equ $A840
data_A844: 	equ $A844
data_A8A0: 	equ $A8A0
data_A8C0: 	equ $A8C0
data_A8C6: 	equ $A8C6
data_A8D0: 	equ $A8D0
data_A8D6: 	equ $A8D6
data_A8DC: 	equ $A8DC
data_A8E0: 	equ $A8E0
data_A8E6: 	equ $A8E6
FrameCountdown3: 	equ $A8F4
data_A8F6: 	equ $A8F6
data_A8F7: 	equ $A8F7
FrameCounter: 	equ $A980
data_A981: 	equ $A981
data_A982: 	equ $A982
data_A983: 	equ $A983
data_A985: 	equ $A985
Credits: 	equ $A986
data_A987: 	equ $A987
data_A988: 	equ $A988
data_A989: 	equ $A989
data_A98D: 	equ $A98D
data_A990: 	equ $A990
data_A991: 	equ $A991
data_A993: 	equ $A993
data_A995: 	equ $A995
data_A999: 	equ $A999
data_A99D: 	equ $A99D
data_A99E: 	equ $A99E
FunctionChecksum: 	equ $A9AB
ProgramPhase: 	equ $A9AC
data_A9AD: 	equ $A9AD
Input0: 	equ $A9AE
Input1: 	equ $A9AF
Input2: 	equ $A9B0
DipSwitch1: 	equ $A9B1
CommandWriteIndex: 	equ $A9B2
CommandReadIndex: 	equ $A9B3
data_A9C0: 	equ $A9C0
data_A9C1: 	equ $A9C1
data_A9C2: 	equ $A9C2
BonusScheme: 	equ $A9C3
data_A9C4: 	equ $A9C4
data_A9C6: 	equ $A9C6
data_A9C9: 	equ $A9C9
data_A9CC: 	equ $A9CC
data_A9CD: 	equ $A9CD
data_A9CE: 	equ $A9CE
data_A9CF: 	equ $A9CF
data_A9D0: 	equ $A9D0
data_A9D3: 	equ $A9D3
data_A9D6: 	equ $A9D6
data_A9D7: 	equ $A9D7
data_A9E2: 	equ $A9E2
FPCharYPos: 	equ $A9E3
data_A9E4: 	equ $A9E4
FPCharXPos: 	equ $A9E5
data_A9E6: 	equ $A9E6
FPCharLineYStep: 	equ $A9E7
FPCharLineXStep: 	equ $A9E9
data_A9EB: 	equ $A9EB
data_A9F0: 	equ $A9F0
data_A9F1: 	equ $A9F1
data_A9F2: 	equ $A9F2
data_A9F3: 	equ $A9F3
data_A9F4: 	equ $A9F4
data_A9F6: 	equ $A9F6
data_A9F7: 	equ $A9F7
PlayerSprite_X: 	equ $AA10
PlayerSprite_No: 	equ $AA11
data_AA24: 	equ $AA24
data_AA28: 	equ $AA28
data_AA2A: 	equ $AA2A
SpriteData1_Sprites_0To2: 	equ $AA30
SpriteData1_19To23: 	equ $AA36
data_AA3F: 	equ $AA3F
PlayerSprite_Attr: 	equ $AA40
PlayerSprite_Y: 	equ $AA41
data_AA55: 	equ $AA55
data_AA59: 	equ $AA59
data_AA5B: 	equ $AA5B
SpriteData2_Sprites_0To2: 	equ $AA60
SpriteData_AA66: 	equ $AA66
data_AA6F: 	equ $AA6F
data_AA80: 	equ $AA80
data_AA81: 	equ $AA81
data_AA82: 	equ $AA82
data_AB08: 	equ $AB08
data_AB30: 	equ $AB30
data_AB37: 	equ $AB37
data_AB43: 	equ $AB43
data_ABFE: 	equ $ABFE
CommandQueue: 	equ $AC00
data_AC43: 	equ $AC43
data_AC64: 	equ $AC64
data_AC65: 	equ $AC65
data_AC74: 	equ $AC74
data_AC7F: 	equ $AC7F
data_ACC0: 	equ $ACC0
data_ACC1: 	equ $ACC1
data_ACC2: 	equ $ACC2
data_ACC3: 	equ $ACC3
data_ACC4: 	equ $ACC4
data_ACC5: 	equ $ACC5
data_ACC6: 	equ $ACC6
data_ACC7: 	equ $ACC7
data_AD00: 	equ $AD00
data_AD01: 	equ $AD01
RemainingShipsInWave: 	equ $AD02
data_AD04: 	equ $AD04
data_AD05: 	equ $AD05
data_AD06: 	equ $AD06
data_AD0A: 	equ $AD0A
CharacterCode: 	equ $AD0B
CharacterAttrib: 	equ $AD0C
data_AD0D: 	equ $AD0D
data_AD0E: 	equ $AD0E
data_AD10: 	equ $AD10
data_AD11: 	equ $AD11
data_AD12: 	equ $AD12
data_AD13: 	equ $AD13
data_AD14: 	equ $AD14
data_AD16: 	equ $AD16
data_AD1A: 	equ $AD1A
data_AD1B: 	equ $AD1B
data_AD1D: 	equ $AD1D
data_AD1E: 	equ $AD1E
data_AD20: 	equ $AD20
data_AD21: 	equ $AD21
data_AD22: 	equ $AD22
data_AD23: 	equ $AD23
data_AD24: 	equ $AD24
data_AD26: 	equ $AD26
data_AD2A: 	equ $AD2A
data_AD2B: 	equ $AD2B
data_AD2D: 	equ $AD2D
data_AD2E: 	equ $AD2E
data_AD30: 	equ $AD30
data_AD31: 	equ $AD31
data_AD32: 	equ $AD32
data_AD33: 	equ $AD33
data_AD34: 	equ $AD34
data_AD35: 	equ $AD35
data_AD36: 	equ $AD36
data_AD37: 	equ $AD37
data_ADF2: 	equ $ADF2
CopyRightStringChar: 	equ $ADFB
BulletDrawList_EndPtr: 	equ $AE00
BulletDrawList: 	equ $AE04
BulletEraseList_EndPtr: 	equ $AE80
BulletEraseList: 	equ $AE84
StackTop: 	equ $B000
SpriteRam1: 	equ $B010
data_B012: 	equ $B012
data_B014: 	equ $B014
data_B036: 	equ $B036
data_B038: 	equ $B038
data_B03A: 	equ $B03A
data_B03C: 	equ $B03C
data_B03E: 	equ $B03E
SpriteRAM2: 	equ $B410
data_B411: 	equ $B411
data_B413: 	equ $B413
data_B415: 	equ $B415
data_B437: 	equ $B437
data_B439: 	equ $B439
data_B43B: 	equ $B43B
data_B43D: 	equ $B43D
data_B43F: 	equ $B43F
label_BC00: 	equ $BC00
W_AudioCommand_R_Scanline: 	equ $C000
WatchdogReset_DipSW2: 	equ $C200
HWReg_Input0_EnableInterrupts: 	equ $C300
HWReg_FlipScreen: 	equ $C302
HWReg_TriggerAudioInterrupt: 	equ $C304
HWReg_VideoEnable: 	equ $C308
data_C30A: 	equ $C30A
data_C30C: 	equ $C30C
HWReg_Input1: 	equ $C320
HWReg_Input2: 	equ $C340
HWReg_DSW1: 	equ $C360
label_C600: 	equ $C600
function_CC00: 	equ $CC00
label_DE00: 	equ $DE00
data_E400: 	equ $E400
label_F700: 	equ $F700
function_FDC4: 	equ $FDC4
data_FFE0: 	equ $FFE0

	org $0000
RST00_Boot:
	JP   InitProgram
	db $FF
	db $FF
	db $FF
	db $33
	db $4B
; Function Name: RST08_AddAToHLAndGetContentsOfHL
; Description: Adds A to HL and puts contents of address in A
; Parameters:
; 	A : index
; 	HL : Byte Table
; Returns:
; 	A : Table Value
RST08_AddAToHLAndGetContentsOfHL:
	ADD  A,L
	LD   L,A
	JR   NC,.no_carry
	INC  H
.no_carry:
	LD   A,(HL)
	RET 
	db $4F
; Function Name: RST10_AddDoubleAToHL
RST10_AddDoubleAToHL:
	ADD  A,A
	RST  $18
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  HL
	RET 
	db $4E
; Function Name: RST18_AddAToHL
RST18_AddAToHL:
	ADD  A,L
	LD   L,A
	RET  NC
	INC  H
	RET 
	db $FF
	db $FF
	db $41
; Function Name: RST20_Subtract32FromDE
RST20_Subtract32FromDE:
	LD   A,E
	SUB  $20
	LD   E,A
	RET  NC
	DEC  D
	RET 
	db $4D
; Function Name: RST28_Add32ToDE
RST28_Add32ToDE:
	LD   A,E
	ADD  A,$20
	LD   E,A
	RET  NC
	INC  D
	RET 
	db $49
; Function Name: RST30_ExecuteProceedingFunction
; Description: Use A to index a function pointer proceeding the call, execute function
; Parameters:
; 	A : FunctionIndex
RST30_ExecuteProceedingFunction:
	POP  HL			; return address
	RST  $10
	EX   DE,HL
	JP   (HL)
	db $FF
	db $FF
	db $FF
	db $FF
; Function Name: RST38_QueueCommand
; Description: Queue command to be processed
; Parameters:
; 	D : Function
; 	E : Param
RST38_QueueCommand:
	PUSH HL
	LD   H,$AC			; CommandQueue
	LD   A,(CommandWriteIndex)
	LD   L,A
	BIT  7,(HL)
	JR   Z,.exit_func			; queue full? - exit
	LD   (HL),D			; set function
	INC  L
	LD   (HL),E			; set param
	INC  L
	LD   A,L
	AND  %00111111			; wrap around
	LD   (CommandWriteIndex),A
.exit_func:
	POP  HL
	RET 
OutputString_ScoreRankingTable:
	dw $A70F
	db $11
	db $ED
	db $77
	db $68
	db $D7
	db $34
	db $F1
	db $D7
	db $A5
	db $3B
	db $7C
	db $FD
	db $3B
	db $7D
	db $F1
	db $DC
	db $A5
	db $8C
	db $57
	db $34
	db $B9
; Function Name: RST66_NMI
RST66_NMI:
	JP   InterruptRoutine
; Function Name: ResetSpritesAndRAM
ResetSpritesAndRAM:
	LD   (WatchdogReset_DipSW2),A			; reset watchdog
	LD   HL,data_B411
	LD   B,48			; clear 48 bytes
.sprite_clear_loop1:
	LD   (HL),$00
	INC  HL
	DJNZ .sprite_clear_loop1
	LD   (WatchdogReset_DipSW2),A
	LD   HL,SpriteRAM2
	LD   B,48			; clear 48 bytes
.sprite_clear_loop2:
	LD   (HL),$00
	INC  HL
	DJNZ .sprite_clear_loop2
	LD   (WatchdogReset_DipSW2),A
	LD   HL,data_A800
	LD   DE,data_A801
	LD   BC,data_07FE+1
	LD   (HL),$00
	LDIR			; clear RAM
	LD   (WatchdogReset_DipSW2),A
	LD   B,$00
	LD   HL,InterruptRoutine
	XOR  A
.checksum_loop:
	ADD  A,(HL)
.operand_009D:
	INC  HL
	DJNZ .checksum_loop
.operand_00A0:
	SUB  $87
.operand_00A2:
	CALL NZ,InterruptRoutine			; checksum failed
	JP   ClearVideoRAM
; Function Name: EnableInterrupts
EnableInterrupts:
	LD   (HWReg_Input0_EnableInterrupts),A			; Interrupt Enable
	LD   (WatchdogReset_DipSW2),A
	JP   ProcessCommandLoop
; Function Name: DrawTestGrid
DrawTestGrid:
	LD   HL,VideoRAM+32
	LD   C,$0E			; vertical count
.vertical_grid_loop:
	LD   DE,RST20_Subtract32FromDE			; offset to next line
	ADD  HL,DE
	LD   B,$10			; horizontal grid size - 16 squares
.horizontal_grid_loop:
	CALL DrawGridSquare
	INC  HL
	INC  HL
	DJNZ .horizontal_grid_loop
	DEC  C
	JR   NZ,.vertical_grid_loop
	RET 
; Function Name: DrawGridSquare
DrawGridSquare:
	PUSH HL
	LD   (HL),$56
	INC  HL
	LD   (HL),$83
	LD   DE,RST18_AddAToHL+7			; next line
	ADD  HL,DE
	LD   (HL),$C7
	INC  HL
	LD   (HL),$EF
	POP  HL
	RET 
; Function Name: InterruptRoutine
; Description: This is called by the NMI hadler (caused by VBLank) starts after initial tests are done
InterruptRoutine:
	PUSH AF
.label_00D9:
	PUSH BC
	PUSH DE
	PUSH HL
	EX   AF,AF'
	EXX 
	PUSH AF
	PUSH BC
	PUSH DE
	PUSH HL
	PUSH IX
	PUSH IY
	CALL CopySpriteDataToHW			; update sprites
	CALL RedrawBullets
	XOR  A
	LD   (HWReg_Input0_EnableInterrupts),A			; disable NMI
	LD   (WatchdogReset_DipSW2),A			; update watchdog
	INC  A
	LD   (data_A987),A			; set value to 1
	LD   A,(data_AD32)
	AND  A
	JR   Z,.label_0106			; jump if value is 0
	LD   A,(data_A9C2)
	AND  A
	JR   NZ,.label_0106
	LD   (data_A987),A
.label_0106:
	LD   A,(data_A987)
	LD   (HWReg_FlipScreen),A
	LD   A,(WatchdogReset_DipSW2)			; read DIP switches
	CPL
	LD   (data_A9AD),A
	LD   A,(HWReg_Input0_EnableInterrupts)			; read input 0
	CPL			; invert because bits are low when controls are down
	LD   (Input0),A			; store inputs
	LD   A,(HWReg_Input1)			; read input 1
	CPL
	LD   (Input1),A			; store with bits inverted
	LD   A,(HWReg_Input2)
	CPL
	LD   (Input2),A			; store with bits inverted
	LD   A,(HWReg_DSW1)			; get DIP switch settings
	CPL
	LD   (DipSwitch1),A			; store with bits inverted
	LD   HL,FrameCounter
	INC  (HL)			; inc frame counter
	LD   HL,data_A9CE
	LD   A,(HL)
	INC  A
	DAA			; adjust for BCD
	LD   (HL),A
	LD   HL,FrameCountdown1
	LD   A,(HL)
	AND  A
	JR   Z,.no_dec1			; already 0, don't dec
	DEC  (HL)
.no_dec1:
	LD   HL,FrameCountdown2
	LD   A,(HL)
	AND  A
	JR   Z,.no_dec2
	DEC  (HL)
.no_dec2:
	LD   HL,FrameCountdown3
	LD   A,(HL)
	AND  A
	JR   Z,.no_dec3
	DEC  (HL)
.no_dec3:
	CALL function_48BE
	LD   HL,label_0174
	PUSH HL			; return address for below?
	LD   A,(FunctionChecksum)
	AND  $03			; mask to 0-3 range to execute function pointer below
	RST  $30
	dw function_15C2
	dw function_1651
	dw function_17FE
	dw ExecutePhaseFunction
; Function Name: OutputString_Player1
OutputString_Player1:
	db $6F
	db $A6
	db $14
	db $88
	db $57
	db $A5
	db $BF
	db $34
	db $D7
	db $F1
	db $96
	db $F1
	db $B9
; Function Name: label_0174
label_0174:
	CALL function_55D4
	POP  IY
	POP  IX
	POP  HL
	POP  DE
	POP  BC
	POP  AF
	EXX 
	EX   AF,AF'
	POP  HL
	POP  DE
	POP  BC
	LD   A,(function_15E2.operand_15FE+2)
	LD   (HWReg_Input0_EnableInterrupts),A
	POP  AF
	RET 
; Function Name: GetTablePtr
; Description: Index pointer table at HL  with A and fetch pointer, return in DE
; Parameters:
; 	A : PointerIndex
; 	HL : PointerTable
; Returns:
; 	DE : TablePointer
GetTablePtr:
	ADD  A,A
	JR   NC,.no_carry
	INC  H
.no_carry:
	ADD  A,L
	LD   L,A
	JR   NC,.no_carry2
	INC  H
.no_carry2:
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  HL
	RET 
; Function Name: function_019A
function_019A:
	LD   HL,VideoRAM
	LD   (data_A989),HL
	LD   A,$20
	LD   (data_A988),A
	LD   B,$F0
	LD   HL,function_4BA5
	XOR  A
.label_01AB:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_01AB
	SUB  $11
	CALL NZ,OutputString_Player1
	RET 
; Function Name: function_01B5
function_01B5:
	LD   HL,VideoRAM+4
	LD   (data_A989),HL
	LD   A,(function_0C90.operand_0CCD)
	LD   (data_A988),A
	RET 
; Function Name: function_01C2
function_01C2:
	LD   HL,(data_A989)
	LD   B,$20
	LD   DE,RST20_Subtract32FromDE
.label_01CA:
	LD   (HL),$F1
	RES  2,H
	LD   (HL),$10
	SET  2,H
	ADD  HL,DE
	DJNZ .label_01CA
	LD   HL,(data_A989)
	INC  HL
	LD   (data_A989),HL
	LD   HL,data_A988
	DEC  (HL)
	RET 
; Function Name: function_01E1
function_01E1:
	XOR  A
	LD   (data_A9E2),A
	LD   HL,(data_0D45)
	LD   (FPCharYPos),HL
	LD   HL,(Phase0Function.operand_280B+1)
	LD   (FPCharXPos),HL
	LD   B,$00
	LD   HL,function_0DD7.label_0E28+11
	XOR  A
.label_01F7:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_01F7
	SUB  $FD
	CALL NZ,ResetSpritesAndRAM
	RET 
; Function Name: PerformRadialWipeStep
PerformRadialWipeStep:
	CALL DrawCharXY
	LD   HL,(label_32EB.delay_outer+2)
	LD   BC,(FPCharYPos)
	AND  A
	SBC  HL,BC
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	LD   A,$00
	SBC  A,$00
	LD   L,H
	LD   H,A
	LD   (FPCharLineYStep),HL
	LD   HL,(QueueKonamiLogoDraw.operand_0B43+2)
	LD   BC,(FPCharXPos)
	AND  A
	SBC  HL,BC
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	LD   A,$00
	SBC  A,$00
	LD   L,H
	LD   H,A
	LD   (FPCharLineXStep),HL
.radial_line_loop:
	LD   HL,(FPCharYPos)
	LD   BC,(FPCharLineYStep)
	ADD  HL,BC
	LD   (FPCharYPos),HL
	LD   HL,(FPCharXPos)
	LD   BC,(FPCharLineXStep)
	ADD  HL,BC
	LD   (FPCharXPos),HL
	CALL DrawCharXY
	LD   DE,(data_14B2)
	AND  A
	SBC  HL,DE
	JP   NZ,.radial_line_loop
	LD   HL,data_A9E2
	INC  (HL)
	LD   A,(HL)			; fetch which pair
	LD   HL,CoordinatePairs
	RST  $10			; look up coordinate pair
	LD   HL,FPCharYPos
	LD   (HL),$00
	INC  HL
	LD   (HL),E			; Y pos
	LD   HL,FPCharXPos
	LD   (HL),$00
	INC  HL
	LD   (HL),D			; X pos
	LD   A,E
	AND  A
	RET 
; Function Name: DrawCharXY
DrawCharXY:
	LD   A,(data_A9E4)			; get Y value (or is it X as screen is rotated!)
	ADD  A,A			; multiply A by 8
	ADD  A,A
	ADD  A,A
	LD   L,A			; put in HL
	LD   H,$00
	ADD  HL,HL			; multiply up by 32
	ADD  HL,HL
	LD   A,(data_A9E6)			; get X
	ADD  A,L
	LD   L,A
	LD   A,$A4			; high byte to point at VideoRAM
	ADD  A,H
	LD   H,A
	LD   A,(CharacterCode)			; get character
	LD   (HL),A			; write to VideoRAM
	RES  2,H			; change address to colour RAM
	LD   A,(CharacterAttrib)			; fetch attrib
	LD   (HL),A			; write attrib
	SET  2,H			; set back to Video RAM
	RET 
CoordinatePairs:
	db $10
	db $04
	db $11
	db $04
	db $12
	db $04
	db $13
	db $04
	db $14
	db $04
	db $15
	db $04
	db $16
	db $04
	db $17
	db $04
	db $18
	db $04
	db $19
	db $04
	db $1A
	db $04
	db $1B
	db $04
	db $1C
	db $04
	db $1D
	db $04
	db $1D
	db $05
	db $1D
	db $06
	db $1D
	db $07
	db $1D
	db $08
	db $1D
	db $09
	db $1D
	db $0A
	db $1D
	db $0B
	db $1D
	db $0C
	db $1D
	db $0D
	db $1D
	db $0E
	db $1D
	db $0F
	db $1D
	db $10
	db $1D
	db $11
	db $1D
	db $12
	db $1D
	db $13
	db $1D
	db $14
	db $1D
	db $15
	db $1D
	db $16
	db $1D
	db $17
	db $1D
	db $18
	db $1D
	db $19
	db $1D
	db $1A
	db $1D
	db $1B
	db $1D
	db $1C
	db $1D
	db $1D
	db $1D
	db $1E
	db $1C
	db $1E
	db $1B
	db $1E
	db $1A
	db $1E
	db $19
	db $1E
	db $18
	db $1E
	db $17
	db $1E
	db $16
	db $1E
	db $15
	db $1E
	db $14
	db $1E
	db $13
	db $1E
	db $12
	db $1E
	db $11
	db $1E
	db $10
	db $1E
	db $0F
	db $1E
	db $0E
	db $1E
	db $0D
	db $1E
	db $0C
	db $1E
	db $0B
	db $1E
	db $0A
	db $1E
	db $09
	db $1E
	db $08
	db $1E
	db $07
	db $1E
	db $06
	db $1E
	db $05
	db $1E
	db $04
	db $1E
	db $03
	db $1E
	db $02
	db $1E
	db $02
	db $1D
	db $02
	db $1C
	db $02
	db $1B
	db $02
	db $1A
	db $02
	db $19
	db $02
	db $18
	db $02
	db $17
	db $02
	db $16
	db $02
	db $15
	db $02
	db $14
	db $02
	db $13
	db $02
	db $12
	db $02
	db $11
	db $02
	db $10
	db $02
	db $0F
	db $02
	db $0E
	db $02
	db $0D
	db $02
	db $0C
	db $02
	db $0B
	db $02
	db $0A
	db $02
	db $09
	db $02
	db $08
	db $02
	db $07
	db $02
	db $06
	db $02
	db $05
	db $02
	db $04
	db $03
	db $04
	db $04
	db $04
	db $05
	db $04
	db $06
	db $04
	db $07
	db $04
	db $08
	db $04
	db $09
	db $04
	db $0A
	db $04
	db $0B
	db $04
	db $0C
	db $04
	db $0D
	db $04
	db $0E
	db $04
	db $0F
	db $04
	db $00
; Function Name: CopySpriteDataToHW
CopySpriteDataToHW:
	LD   HL,SpriteData1_Sprites_0To2
	LD   DE,SpriteRam1
	LD   A,(data_A987)			; some kind of coord flip check?
	AND  A
	JP   Z,.CopySpritesFlipped
; Update 3 sprites here
	LDI			; X position
	LDI			; Sprite No
	LDI			; X position
	LDI			; Sprite No
	LDI			; X pos
	LDI			; Sprite No
	LD   HL,PlayerSprite_X
	LDI			; 3 - x
	LDI			; sprite no
	LDI			; 4 - x
	LDI
	LDI			; 5 - x
	LDI
	LDI			; 6 - x
	LDI
	LDI			; 7 - x
	LDI
	LDI			; 8 - x
	LDI
	LDI			; 9 - x
	LDI
	LDI			; 10 - x
	LDI
	LDI			; 11 - x
	LDI
	LDI			; 12 - x
	LDI
	LDI			; 13 - x
	LDI
	LDI			; 14 - x
	LDI
	LDI			; 15 - x
	LDI
	LDI			; 16 - x
	LDI
	LDI			; 17 - x
	LDI
	LDI			; 18 - x
	LDI
	LD   HL,SpriteData1_19To23
	LDI			; 19 x
	LDI
	LDI			; 20 - x
	LDI
	LDI			; 21 - x
	LDI
	LDI			; 22 - x
	LDI
	LDI			; 23 - x
	LDI
	LD   HL,SpriteData2_Sprites_0To2
	LD   DE,SpriteRAM2
	LDI			; set attribs
	LD   A,(HL)			; get Y pos
	ADD  A,$0E
	CPL
	LD   (DE),A			; write Y pos
	INC  L
	INC  E
	LDI			; set attrib
	LD   A,(HL)			; get Y
	ADD  A,$0E
	CPL
	LD   (DE),A			; write Y pos
	INC  L
	INC  E
	LDI			; set attrib
	LD   A,(HL)			; get Y
	ADD  A,$0E
	CPL
	LD   (DE),A			; write Y pos
	INC  L
	INC  E
	LD   HL,PlayerSprite_Attr
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LD   HL,SpriteData_AA66
	LDI			; attrib
	LD   A,(HL)			; y
	ADD  A,$0E
	CPL
	LD   (DE),A			; y
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0E
	CPL
	LD   (DE),A
	INC  L
	INC  E
.label_04BC:
	LD   A,(FunctionChecksum)
	CP   $03
	RET  NZ
	LD   A,(ProgramPhase)
	LD   HL,DrawRemainingWaveTally.wave_char_loop+4
	CP   (HL)			; compare to 5
	RET  C
	CP   $08
	RET  NC
	LD   A,(data_B411)
	ADD  A,$80
	JR   C,.label_04DE
	LD   (data_B411),A
	LD   HL,SpriteRam1
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_04DE:
	LD   A,(data_B413)
	ADD  A,$80
	JR   C,.label_04EF
	LD   (data_B413),A
	LD   HL,data_B012
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_04EF:
	LD   A,(data_B415)
	ADD  A,$80
	JR   C,.label_0500
	LD   (data_B415),A
	LD   HL,data_B014
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_0500:
	LD   A,(data_B437)
	ADD  A,$80
	JR   C,.label_0511
	LD   (data_B437),A
	LD   HL,data_B036
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_0511:
	LD   A,(data_B439)
	ADD  A,$80
	JR   C,.label_0522
	LD   (data_B439),A
	LD   HL,data_B038
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_0522:
	LD   A,(data_B43B)
	ADD  A,$80
	JR   C,.label_0533
	LD   (data_B43B),A
	LD   HL,data_B03A
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_0533:
	LD   A,(data_B43D)
	ADD  A,$80
	JR   C,.label_0544
	LD   (data_B43D),A
	LD   HL,data_B03C
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_0544:
	LD   A,(data_B43F)
	ADD  A,$80
	JR   C,.label_0555
	LD   (data_B43F),A
	LD   HL,data_B03E
	LD   A,(HL)
	ADD  A,$80
	LD   (HL),A
.label_0555:
	RET 
.CopySpritesFlipped:
	LD   A,(HL)			; x coord
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI			; colour
	LD   A,(HL)			; x
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A			; x
	INC  L
	INC  E
	LDI
	LD   HL,PlayerSprite_X
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   HL,SpriteData1_19To23
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   A,(HL)
	ADD  A,$0F
	CPL
	LD   (DE),A
	INC  L
	INC  E
	LDI
	LD   HL,SpriteData2_Sprites_0To2
	LD   DE,SpriteRAM2
	LD   A,(HL)			; attrib
	XOR  %11000000			; X & Y Flip
	LD   (DE),A			; set attrib
	INC  L
	INC  E
	LD   A,(HL)			; Y pos
	INC  A
	LD   (DE),A			; write Y pos
	INC  L
	INC  E
	LD   A,(HL)			; attrib
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)			; y pos
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   HL,PlayerSprite_Attr
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   HL,SpriteData_AA66
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	XOR  $C0
	LD   (DE),A
	INC  L
	INC  E
	LD   A,(HL)
	INC  A
	LD   (DE),A
	INC  L
	INC  E
	JP   .label_04BC
; Function Name: function_074B
function_074B:
	LD   B,$00
	LD   HL,data_4AA0
	XOR  A
.label_0751:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_0751
	SUB  $B8
	JP   NZ,TamperDetected
	LD   A,(CharacterAttrib)
	CP   $05
	PUSH AF
	LD   A,$05
	LD   (CharacterAttrib),A
	LD   A,$F1
	LD   (CharacterCode),A
	CALL function_01E1
	POP  AF
	CALL Z,AdvancePhase
	JP   AdvancePhase
; Function Name: Phase4Function
Phase4Function:
	LD   B,$00
	LD   HL,data_4C87+18
	SUB  A
.label_077A:
	XOR  (HL)
	INC  HL
	DJNZ .label_077A
	ADD  A,$95
	CALL NZ,function_0F11
	LD   A,(data_AD30)
	AND  A
	JR   Z,.label_07A0
	LD   DE,(OutputString_AndEvery50000.operand_125A+1)
	LD   A,(data_AD32)
	AND  A
	JR   Z,.label_0794
	INC  E			; inc param
.label_0794:
	RST  $38
	LD   A,(data_AD0E)
	AND  A
	JR   Z,.label_07A0
.operand_079B:
	LD   D,$07
	RST  $38
	JR   .label_07A4
.label_07A0:
	LD   DE,PerformRadialWipeStep+1
	RST  $38
.label_07A4:
	CALL DrawRemainingWaveTally
	CALL function_19F0
	JP   AdvancePhase
.label_07AD:
	LD   B,A
	JP   DrawBulletList.label_5303
; Function Name: InitProgram
InitProgram:
	LD   A,(data_6000)
	CP   $55
	JP   Z,data_6000
	LD   SP,StackTop			; set up stack pointer to end of RAM
	LD   (WatchdogReset_DipSW2),A			; reset watchdog
	LD   HL,HWReg_Input0_EnableInterrupts			; first HW reg to write to
	LD   B,$08			; clear 8 values
.reg_clear_loop:
	LD   (HL),$00			; set HW register to 0
	INC  HL
	DJNZ .reg_clear_loop
	LD   A,(function_2D3F.operand_2D49+2)
	LD   (HWReg_VideoEnable),A			; Video Enable
.operand_07CF:
	JP   ResetSpritesAndRAM
; Function Name: ClearChars_07D2
ClearChars_07D2:
	LD   HL,VideoRAM+927
	LD   DE,data_FFE0			; offset to previous line
.operand_07D8:
	LD   B,$0E			; 14 characters
.char_loop:
	LD   (HL),$F1			; clear char
	RES  2,H
	LD   (HL),$16			; set attrib
	SET  2,H
	ADD  HL,DE
	DJNZ .char_loop
	RET 
; Function Name: label_07E6
label_07E6:
	CALL SetKonamicSpriteParams
	CALL QueueKonamiLogoDraw
	LD   HL,VideoRAM+540
	LD   DE,data_ABFE
	CALL function_1AFC
	LD   A,(Input0)
	BIT  3,A
	JP   NZ,function_31B4.label_3215
	db $3A
data_07FE:
	db $86
	db $A9
	db $3D
	db $C8
	db $11
	db $19
	db $01
	db $FF
	db $C3
	db $1A
	db $0F
; Function Name: DrawRemainingWaveTally
DrawRemainingWaveTally:
	LD   A,(data_AD04)			; index to ?
	ADD  A,A			; *2
	LD   B,A
	ADD  A,A			; *4
	ADD  A,A			; *8
	ADD  A,B			; + *2 = *10
	LD   HL,EnemyTallyChars			; base
	RST  $18			; add index * 10
	LD   B,(HL)			; get first 2 bytes into BC
	INC  HL
	LD   C,(HL)
	INC  HL
	LD   A,(RemainingShipsInWave)			; index to ?
	LD   E,A			; store unmasked value
	AND  %00000111			; mask 0-7
	RST  $08
	EX   AF,AF'
	LD   A,E			; restore unmasked value
	LD   HL,VideoRAM+927			; start position of wave tally display
	LD   DE,$FFE0			; -32 - up one chat line
	RRCA			; rotate right twice
	RRCA
	AND  %00011111			; mask bottom 5 buts
	JR   Z,.draw_empty_char
.wave_char_loop:
	LD   (HL),B			; write first char
	ADD  HL,DE
	DEC  A
	JR   Z,.draw_empty_char
	LD   (HL),C			; write second char
	ADD  HL,DE
	DEC  A
	JR   NZ,.wave_char_loop
.draw_empty_char:
	EX   AF,AF'
	LD   (HL),A			; draw fraction element
	ADD  HL,DE
	LD   (HL),$F1			; draw empty char
	RET 
; Function Name: QueueTitleScreenDraw
QueueTitleScreenDraw:
	CALL QueueKonamiLogoDraw
.operand_0841:
	CALL SetKonamicSpriteParams
	LD   DE,InterruptRoutine.label_00D9+39			; draw copyright info
	LD   B,$02
.label_0849:
	RST  $38
	INC  E			;  skip to 'Play'
	DJNZ .label_0849
	INC  E
	LD   B,$05			; loop count
.label_0850:
	RST  $38
	INC  E
	DJNZ .label_0850
.operand_0854:
	LD   E,$14			; 'Time Pilot' logo top
	RST  $38
	INC  E			; logo bottom
	RST  $38
; this checks if function at CheckCopyrightString has been tampered with
	LD   HL,CheckCopyrightString
	LD   B,$18			; loop count
	XOR  A
.xor_byte_loop:
	XOR  (HL)
	INC  L
.operand_0861:
	DJNZ .xor_byte_loop
	SUB  $C9
	JP   NZ,TamperDetected
	JP   AdvancePhase
OutputString_CopyrightKonami:
	dw $A6BC
data_086D:
	db $10
	db $30
	db $F1
data_0870:
	db $7C
	db $68
	db $3B
	db $A5
data_0874:
	db $38
	db $FD
	db $F1
	db $96
	db $5D
	db $17
	db $9B
	db $B9
; Each item is 10 bytes
EnemyTallyChars:
	db $4C			; first wave char
	db $4F			; second wave char
	db $F1
	db $41
	db $72
	db $A6
	db $F1
	db $8D
	db $E2
	db $FB
	db $37
	db $A7
	db $F1
	db $AB
	db $31
	db $07
	db $F1
	db $5A
	db $75
	db $85
	db $D9
	db $1B
	db $F1
	db $C1
	db $E1
	db $FA
	db $F1
	db $B3
	db $A0
	db $47
	db $7B
	db $78
	db $F1
	db $04
	db $05
	db $C2
	db $F1
	db $DE
	db $F9
	db $BB
	db $93
	db $AC
	db $F1
	db $36
	db $06
	db $4B
	db $F1
	db $EE
	db $D3
	db $D4
.label_08AE:
	LD   HL,Phase1Function
	LD   B,$1E
	RET 
; Function Name: Phase9Function
Phase9Function:
	CALL PerformRadialWipeStep
	RET  NZ
	LD   B,$00
	LD   HL,function_47B3.label_4853+45
	SUB  A
.label_08BE:
	XOR  (HL)
	INC  HL
	DJNZ .label_08BE
	ADD  A,$D0
	JP   NZ,InterruptRoutine.label_00D9
.operand_08C7:
	LD   DE,InterruptRoutine.label_0106+13			; Output string 'Score Ranking Table'
	RST  $38
	LD   E,$00			; (C) Konami
	RST  $38
	LD   E,$14			; Time Pilot Logo Top
	RST  $38
	INC  E			; Logo bottom
	RST  $38
	LD   E,$0C			; Input your initials
	RST  $38
	CALL function_4BDC
	LD   HL,data_A995
	XOR  A
	LD   B,$05
.label_08DF:
	LD   (HL),A
	INC  HL
	DJNZ .label_08DF
	LD   (HL),$03
	LD   DE,(data_A993)
	LD   A,(data_A999)
	LD   HL,data_12C7
	RST  $08
	LD   (DE),A
	RES  2,D
	LD   A,(DE)
	LD   (data_A990),A
	JP   AdvancePhase
TamperDetected:
	db $4B
	db $01
	db $4A
	db $01
	db $49
	LD   BC,InterruptRoutine.no_dec1+6
	LD   B,A
	LD   BC,InterruptRoutine.no_dec1+4
	LD   B,L
	LD   BC,InterruptRoutine.label_0106+58
	LD   A,$01
	INC  A
	LD   BC,InterruptRoutine.label_0106+52
	JR   C,.label_0913
	LD   (data_2F01),A
.label_0913:
	LD   BC,InterruptRoutine.label_0106+39
	DAA
	LD   BC,InterruptRoutine.label_0106+30
	LD   HL,OutputString_FreePlay+605
	LD   BC,InterruptRoutine.label_0106+18
	DEC  D
	LD   BC,InterruptRoutine.label_0106+12
	INC  C
	LD   BC,InterruptRoutine.label_0106+3
	LD   B,$01
	NOP 
	LD   BC,InterruptRoutine.label_00D9+36
	JP   M,data_E400.label_F700
	NOP 
	POP  AF
	NOP 
	XOR  $00
	EX   DE,HL
	NOP 
	PUSH HL
	NOP 
	JP   PO,function_CDCD.label_DE00
	NOP 
	RET  C
	NOP 
	PUSH DE
	NOP 
	POP  DE
	NOP 
	JP   Z,HWReg_DSW1.label_C600
	NOP 
	JP   data_B43F.label_BC00
	db $00
	db $B6
	db $00
	db $AE
	db $00
	db $A9
	db $00
	db $9F
	db $00
	db $9C
	db $00
	db $93
	db $00
	db $8A
	db $00
	db $84
	db $00
	db $7B
	db $00
	db $71
	db $00
	db $6B
	db $00
	db $61
	db $00
	db $57
	db $00
	db $50
	db $00
	db $45
	db $00
	db $3B
	db $00
	db $34
	db $00
	db $29
	db $00
	db $1E
	db $00
	db $13
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $FF
	db $ED
	db $FF
	db $00
	db $00
	db $D7
	db $FF
	db $CC
	db $FF
	db $C5
	db $FF
	db $BB
	db $FF
	db $B0
	db $FF
	db $A9
	db $FF
	db $9F
	db $FF
	db $95
	db $FF
	db $8F
	db $FF
	db $85
	db $FF
	db $7C
	db $FF
	db $76
	db $FF
	db $6D
	db $FF
	db $64
	db $FF
	db $61
	db $FF
	db $64
	db $FF
	db $52
	db $FF
	db $4A
	db $FF
	db $44
	db $FF
	db $3D
	db $FF
	db $3A
	db $FF
	db $36
	db $FF
	db $2F
	db $FF
	db $2B
	db $FF
	db $28
	db $FF
	db $22
	db $FF
	db $1E
	db $FF
	db $1B
	db $FF
	db $15
	db $FF
	db $12
	db $FF
	db $0F
	db $FF
	db $0F
	db $FF
	db $06
	db $FF
	db $03
	db $FF
	db $00
	db $FF
	db $FA
	db $FE
	db $F7
	db $FE
	db $F4
	db $FE
	db $EE
	db $FE
	db $EB
	db $FE
	db $E8
	db $FE
	db $E2
	db $FE
	db $DF
	db $FE
	db $DC
	db $FE
	db $D9
	db $FE
	db $D3
	db $FE
	db $D1
	db $FE
	db $CE
	db $FE
	db $C8
	db $FE
	db $C6
	db $FE
	db $C4
	db $FE
	db $C2
	db $FE
	db $C0
	db $FE
	db $BB
	db $FE
	db $BA
	db $FE
	db $B9
	db $FE
	db $B8
	db $FE
	db $B7
	db $FE
	db $B6
	db $FE
	db $B5
	db $FE
	db $B5
	db $FE
	db $B6
	db $FE
	db $B7
	db $FE
	db $B8
	db $FE
	db $B9
	db $FE
	db $BA
	db $FE
	db $BB
	db $FE
	db $C0
	db $FE
	db $C2
	db $FE
	db $C4
	db $FE
	db $C6
	db $FE
	db $C8
	db $FE
	db $CE
	db $FE
	db $D1
	db $FE
	db $D3
	db $FE
	db $D9
	db $FE
	db $DC
	db $FE
	db $DF
	db $FE
	db $E2
	db $FE
	db $E8
	db $FE
	db $EB
	db $FE
	db $EE
	db $FE
	db $F4
	db $FE
	db $F7
	db $FE
	db $FA
	db $FE
	db $00
	db $FF
	db $03
	db $FF
	db $06
	db $FF
	db $09
	db $FF
	db $0F
	db $FF
	db $12
	db $FF
	db $15
	db $FF
	db $1B
	db $FF
	db $1E
	db $FF
	db $22
	db $FF
	db $28
	db $FF
	db $2B
	db $FF
	db $2F
	db $FF
	db $36
	db $FF
	db $3A
	db $FF
	db $3D
	db $FF
	db $44
	db $FF
	db $4A
	db $FF
	db $52
	db $FF
	db $57
	db $FF
	db $61
	db $FF
	db $64
	db $FF
	db $6D
	db $FF
	db $76
	db $FF
	db $7C
	db $FF
	db $85
	db $FF
	db $8F
	db $FF
	db $95
	db $FF
	db $9F
	db $FF
	db $A9
	db $FF
	db $B0
	db $FF
	db $BB
	db $FF
	db $C5
	db $FF
	db $CC
	db $FF
	db $D7
	db $FF
	db $E2
	db $FF
	db $ED
	db $FF
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $13
	db $00
	db $1E
	db $00
	db $29
	db $00
	db $34
	db $00
	db $3B
	db $00
	db $45
	db $00
	db $50
	db $00
	db $57
	db $00
	db $61
	db $00
	db $6B
	db $00
	db $71
	db $00
	db $7B
	db $00
	db $84
	db $00
	db $8A
	db $00
	db $93
	db $00
data_0A9C:
	db $9C
	db $00
	db $9F
	db $00
	db $9F
	db $00
	db $AE
	db $00
	db $B6
	db $00
	db $BC
	db $00
	db $C3
	db $00
	db $C6
	db $00
	db $CA
	db $00
	db $D1
	db $00
	db $D5
	db $00
	db $D8
	db $00
	db $DE
	db $00
	db $E2
	db $00
	db $E5
	db $00
	db $EB
	db $00
	db $EE
	db $00
	db $F1
	db $00
	db $EE
	db $00
	db $FA
	db $00
	db $FD
	db $00
	db $00
	db $01
	db $06
	db $01
	db $09
	db $01
	db $0C
	db $01
	db $12
	db $01
	db $15
	db $01
	db $18
	db $01
	db $1E
	db $01
	db $21
	db $01
	db $24
	db $01
	db $27
	db $01
	db $2D
	db $01
	db $2F
	db $01
	db $27
	db $01
	db $38
	db $01
	db $3A
	db $01
	db $3C
	db $01
	db $3E
	db $01
	db $40
	db $01
	db $45
	db $01
	db $46
	db $01
	db $47
	db $01
	db $48
	db $01
	db $49
	db $01
	db $4A
	db $01
	db $4B
	db $01
OutputString_Stage:
	dw $A677
	db $13
	db $ED
	db $DC
	db $A5
	db $7D
	db $34
	db $F1
	db $F1
	db $F1
	db $B9
; Function Name: SetKonamicSpriteParams
; Description: This sets the sprites to draw the Konami copyright
SetKonamicSpriteParams:
	LD   IY,PlayerSprite_X
	LD   B,$04			; 4 sprites
	LD   C,$04			; sprite no of first (C)Konami sprites
	LD   D,$A0			; y coord
	LD   E,$D8			; x coord
.konami_sprite_loop:
	LD   (IY+$31),D			; y
	LD   (IY+$00),E			; x
	LD   (IY+$01),C			; sprite no
	LD   (IY+$30),$6C			; flip & colour
	INC  IY			; next sprite
	INC  IY
	INC  C
	LD   A,D			; subtract 16 pixels from Y
	SUB  $10
	LD   D,A
	DJNZ .konami_sprite_loop
	RET 
; Function Name: function_0B2B
function_0B2B:
	LD   HL,PlayerSprite_Y
	LD   DE,$0002
data_0B31:
	LD   B,$04
	XOR  A
.label_0B34:
	LD   (HL),A
	ADD  HL,DE
	DJNZ .label_0B34
	RET 
; Function Name: QueueKonamiLogoDraw
; Description: This queues different commands on alternate frames
QueueKonamiLogoDraw:
	LD   A,(FrameCounter)
	BIT  0,A
	JR   Z,.alt_frame
	LD   DE,InterruptRoutine.label_00D9+39			; command 1, param 0 (Konami Logo 1)
.operand_0B43:
	JP   RST38_QueueCommand			; tail call
.alt_frame:
	LD   DE,InterruptRoutine.label_0106+25			; command 1, param 1F (Konami Logo 2)
	JP   RST38_QueueCommand			; tail call
; Function Name: function_0B4C
function_0B4C:
	XOR  A
.label_0B4D:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_0B4D
	CP   C
	RET  Z
	RET 
	db $AF
	db $AE
	db $23
	db $10
	db $FC
	db $B9
	db $C8
	db $C3
	db $00
	db $00
	db $AF
	db $86
	db $23
	db $0D
	db $28
	db $02
	db $18
	db $F9
	db $CB
	db $47
	db $C8
	db $C3
	db $00
	db $00
	db $21
	db $06
	db $0B
	db $06
	db $24
	db $0E
	db $00
	db $7E
	db $91
	db $23
	db $10
	db $FB
	db $EB
	db $BE
	db $C9
OutputString_PushStartButton:
	dw $A70F
	db $13
	db $88
	db $0D
	db $ED
	db $C4
	db $F1
	db $ED
	db $DC
	db $A5
	db $D7
	db $DC
	db $F1
	db $8C
	db $0D
	db $DC
	db $DC
	db $68
	db $3B
	db $B9
; Function Name: ReturnPoint
ReturnPoint:
	JP   ProcessCommandLoop
; Function Name: ProcessCommandLoop
ProcessCommandLoop:
	LD   H,$AC			; CommandQueue
	LD   A,(CommandReadIndex)
	LD   L,A
	LD   A,(HL)
	RLCA
	JP   C,ReturnPoint			; loop back if queue is empty
	LD   C,(HL)			; function
	LD   (HL),$FF			; clear entry
	INC  HL
	LD   B,(HL)			; param?
	LD   (HL),$FF			; clear entry
	INC  HL			; inc read index and wrap around
	LD   A,L
	AND  $3F
	LD   (CommandReadIndex),A
	LD   A,C
	AND  $0F
	LD   HL,FunctionTable
	CALL GetTablePtr
	LD   A,B			; param for function
	LD   HL,ReturnPoint
	PUSH HL			; push return address
	EX   DE,HL			; put function pointer in HL
	JP   (HL)			; jump to function pointer
; this tbale has 16 entries
; Each entry points to a function
FunctionTable:
	dw function_0BDD			; 0
	dw OutputString			; 1
	dw OutputStringColoured			; 2
	dw ClearString			; 3
	dw function_0C90			; 4
	dw function_4D72			; 5
	dw function_0DD7			; 6
	dw function_0EAC
	dw NullFunction
	dw NullFunction
	dw function_3421
	dw OutputStringColouredPlus10
	dw NullFunction
	dw NullFunction
	dw NullFunction
	dw NullFunction
; Function Name: NullFunction
NullFunction:
	RET 
; Function Name: function_0BDD
function_0BDD:
	LD   HL,OutputStringTable
	CALL GetTablePtr
	EX   DE,HL
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  HL
	INC  HL
.string_loop:
	LD   A,(HL)
	CP   $B9
	RET  Z
	LD   (DE),A
	INC  HL
	RST  $20
	JR   .string_loop
; Function Name: OutputString
; Parameters:
; 	A : String No
OutputString:
	LD   HL,OutputStringTable
	CALL GetTablePtr
	EX   DE,HL
	LD   E,(HL)			; fetch video RAM address
	INC  HL
	LD   D,(HL)
	INC  HL
	LD   C,(HL)			; fetch attribute
	INC  HL
.string_loop:
	LD   A,(HL)			; fetch character
	CP   $B9			; check for terminator?
	RET  Z			; terminator
	LD   (DE),A			; write character
	RES  2,D			; switch to colour RAM address
	LD   A,C
	LD   (DE),A			; write attribute to colour RAM
	SET  2,D
	INC  HL
	RST  $20			; next line
	JP   .string_loop
; Function Name: OutputStringColoured
OutputStringColoured:
	LD   HL,OutputStringTable
	CALL GetTablePtr
	EX   DE,HL
	LD   E,(HL)			; get video address
	INC  HL
	LD   D,(HL)
	INC  HL			; skip attribute
	INC  HL
	LD   A,(CharacterAttrib)			; fetch attribute colour
	AND  $0F			; mask colour bits
	LD   C,A
	JR   OutputString.string_loop
; Function Name: OutputStringColouredPlus10
OutputStringColouredPlus10:
	LD   HL,OutputStringTable
	CALL GetTablePtr
	EX   DE,HL
	LD   E,(HL)			; string address
	INC  HL
	LD   D,(HL)
	INC  HL			; skip attribute
	INC  HL
	LD   A,(CharacterAttrib)
	ADD  A,$0A
	AND  $0F
	LD   C,A
	JR   OutputString.string_loop			; jump to other function to output string
; Function Name: ClearString
; Description: Erase a string from view
ClearString:
	LD   HL,OutputStringTable
.operand_0C3C:
	CALL GetTablePtr
	EX   DE,HL
	LD   E,(HL)			; get string addess
	INC  HL
	LD   D,(HL)
	INC  HL			; skip attrib
	INC  HL
.string_loop:
	LD   A,(HL)			; read char
	CP   $B9			; check for terminator
	RET  Z
	LD   A,$F1			; clear character
	LD   (DE),A
	INC  HL
	RST  $20
	JR   .string_loop
OutputStringTable:
	dw OutputString_CopyrightKonami			; 0
	dw OutputString_Play			; 1
	dw OutputString_Ready			; 2
	dw OutputString_PleaseDepositCoin			; 3
	dw OutputString_AndTryThisGame			; 4
	dw OutputString_HI_SCORE			; 5
data_0C5B:
	dw OutputString_1UP			; 6
	dw OutputString_2UP			; 7
	dw OutputString_Credit			; 8
	dw OutputString_Player1			; 9
	dw OutputString_Player2			; A
	dw Outputstring_GameOver			; B
	dw OutputString_InputYourInitials			; C
	dw OutputString_FreePlay			; D
	dw OutputString_Stage			; E
	dw OutputString_FirstBonus10000			; F
	dw OutputString_AndEvery50000			; 0x10
	dw OutputString_FirstBonus20000			; 0x11
	dw OutputString_AndEvery60000Pts			; 0x12
	dw OutputString_ScoreRankingTable			; 0x13
data_0C78:
	dw OutputString_TimePilotLogo_Top			; 0x14
	dw OutputString_TimePilotLogo_Bottom			; 0x15
	dw OutputString_PushStartButton			; 0x16
	dw OutputString_OnePlayerOnly			; 0x17
	dw OutputString_Empty			; 0x18
	dw OutputString_OneOrTwoPlayers			; 0x19
	dw OutputString_AD1910			; 0x1A
	dw OutputString_AD1940			; 0x1B
	dw OutputString_AD1970			; 0x1C
	dw OutputString_AD1982			; 0x1D
	dw OutputString_AD2001			; 0x1E
	dw OutputString_KonamiCopyright2			; 0x1F
; Function Name: function_0C90
function_0C90:
	LD   C,A
	LD   B,$00
	LD   A,(data_AD30)
	AND  A
	JP   Z,.label_0CE8
	LD   A,C
	AND  A
	JP   Z,.label_0CE9
	LD   HL,.label_0D0A+29
	ADD  HL,BC
	ADD  HL,BC
	ADD  HL,BC
	LD   DE,data_AD33
	LD   A,(data_AD32)
	AND  A
	JR   Z,.label_0CB1
	LD   DE,data_AD36
.label_0CB1:
	LD   A,(DE)
	ADD  A,(HL)
	DAA
	LD   (DE),A
	INC  DE
	INC  HL
	LD   A,(DE)
	ADC  A,(HL)
	DAA
	LD   (DE),A
	INC  DE
	INC  HL
	LD   A,(DE)
	ADC  A,(HL)
	DAA
	LD   (DE),A
	LD   HL,data_A98D
	LD   BC,$0003
.label_0CC7:
	LD   A,(DE)
	CP   (HL)
	JR   C,.label_0CDA
	JR   NZ,.label_0CD4
.operand_0CCD:
	DEC  DE
	DEC  HL
	DEC  C
	JR   NZ,.label_0CC7
	JR   .label_0CDA
.label_0CD4:
	EX   DE,HL
	LDDR
	CALL function_0D6B
.label_0CDA:
	LD   A,(data_AD32)
	AND  A
	JR   NZ,.label_0CE5
	CALL function_0D57
	JR   .label_0CE8
.label_0CE5:
	CALL function_0D61
.label_0CE8:
	RET 
.label_0CE9:
	LD   A,(data_AD31)
	AND  A
	JR   NZ,.label_0D0A
	LD   A,(data_0B31)
	CALL OutputString
	CALL function_0D57
	LD   A,(function_15C2.operand_15C5+1)
	CALL ClearString
	LD   DE,VideoRAM+257
	LD   B,$06
.label_0D03:
	LD   A,$F1
	LD   (DE),A
	RST  $20
	DJNZ .label_0D03
	RET 
.label_0D0A:
	db $3E
	db $06
	db $CD
	db $F2
	db $0B
	db $CD
	db $57
	db $0D
	db $3E
	db $07
	db $CD
	db $F2
	db $0B
	db $CD
	db $61
	db $0D
	db $C9
	db $3C
	db $A2
	db $C7
	db $AC
	db $7C
	db $A2
	db $43
	db $AB
	db $FC
	db $A1
	db $BE
	db $AC
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $00
	db $02
	db $00
	db $00
	db $03
	db $00
	db $00
	db $04
	db $00
	db $00
	db $05
	db $00
	db $00
	db $06
	db $00
	db $00
	db $07
	db $00
	db $00
	db $08
	db $00
	db $00
	db $09
	db $00
data_0D45:
	db $00
data_0D46:
	db $10
	db $00
	db $00
	db $15
	db $00
	db $00
	db $20
	db $00
	db $00
	db $30
	db $00
	db $00
	db $40
	db $00
	db $00
	db $50
	db $00
; Function Name: function_0D57
function_0D57:
	LD   DE,VideoRAM+897
	LD   HL,data_AD35
	LD   C,$10
	JR   function_0D6B.label_0D73
; Function Name: function_0D61
function_0D61:
	db $11
	db $01
	db $A5
	db $21
	db $38
	db $AD
	db $0E
	db $10
	db $18
	db $08
; Function Name: function_0D6B
function_0D6B:
	LD   DE,VideoRAM+577
	LD   HL,data_A98D
	LD   C,$10
.label_0D73:
	LD   B,$00
	CALL function_0DA0
	DEC  HL
	CALL function_0DA0
	DEC  HL
	CALL function_0D81
	RET 
; Function Name: function_0D81
function_0D81:
	LD   A,(HL)
	RRCA
	RRCA
	RRCA
	RRCA
	CALL function_0D90
	RST  $20
	LD   A,(HL)
	CALL function_0D90
	RST  $20
	RET 
; Function Name: function_0D90
function_0D90:
	AND  $0F
	PUSH HL
	LD   HL,data_0DCC
	RST  $08
	POP  HL
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	RET 
; Function Name: function_0DA0
function_0DA0:
	LD   A,(HL)
	RRCA
	RRCA
	RRCA
	RRCA
	CALL function_0DAF
	RST  $20
	LD   A,(HL)
	CALL function_0DAF
	RST  $20
	RET 
; Function Name: function_0DAF
function_0DAF:
	AND  $0F
	JR   Z,.label_0DB6
	INC  B
	JR   .label_0DBE
.label_0DB6:
	LD   A,(function_323A.operand_3244+2)
	INC  B
	DEC  B
	JR   Z,.label_0DBE
	XOR  A
.label_0DBE:
	PUSH HL
	LD   HL,data_0DCC
	RST  $08
	POP  HL
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	RET 
data_0DCC:
	db $13
	db $96
	db $9B
	db $CD
	db $F3
	db $7F
	db $65
	db $02
	db $17
	db $5D
	db $F1
; Function Name: function_0DD7
function_0DD7:
	LD   DE,VideoRAM+99
	CP   $64
	JR   C,.label_0DE0			; jump if param less than 64h
	LD   A,$63			; clamp to 63h
.label_0DE0:
	EXX 			; swap all paired regs
	LD   B,$00
.label_0DE3:
	SUB  $1E
	JR   C,.label_0DEA
	INC  B
	JR   .label_0DE3
.label_0DEA:
	ADD  A,$1E
	LD   C,$00
.label_0DEE:
	SUB  $0A
	JR   C,.label_0DF5
	INC  C
	JR   .label_0DEE
.label_0DF5:
	ADD  A,$0A
	LD   D,$00
.label_0DF9:
	SUB  $05
	JR   C,.label_0E00
	INC  D
	JR   .label_0DF9
.label_0E00:
	ADD  A,$05
	LD   E,A
	EXX 
	EXX 
	LD   A,E
	EXX 
	AND  A
	JR   Z,.label_0E16
	LD   B,$01
	LD   C,$13
.label_0E0E:
	EX   AF,AF'
	CALL DrawLifeIcon
	EX   AF,AF'
	DEC  A
	JR   NZ,.label_0E0E
.label_0E16:
	EXX 
	LD   A,D
	EXX 
	AND  A
	JR   Z,.label_0E28
	LD   B,$32
	LD   C,$11
	EX   AF,AF'
	CALL function_0E9C
	db $08
	db $3D
	db $20
	db $F8
.label_0E28:
	EXX 
	LD   A,C
	EXX 
	AND  A
	JR   Z,.label_0E3A
	db $06
	db $CE
	db $0E
	db $16
	db $08
	db $CD
	db $70
	db $0E
	db $08
	db $3D
	db $20
	db $F8
.label_0E3A:
	EXX 
	LD   A,B
	EXX 
	AND  A
	JR   Z,.label_0E4C
	LD   B,$23
	LD   C,$11
.label_0E44:
	EX   AF,AF'
	CALL function_0E70
	EX   AF,AF'
	DEC  A
	JR   NZ,.label_0E44
.label_0E4C:
	LD   BC,data_E400+3344
.label_0E4F:
	LD   HL,data_59DD
	ADD  HL,DE
	JR   C,.label_0E5A
	CALL DrawLifeIcon
	JR   .label_0E4F
.label_0E5A:
	XOR  A
	LD   HL,(ResetSpritesAndRAM.operand_00A0)
	LD   DE,(ResetSpritesAndRAM.operand_00A2+1)
	LD   BC,(ResetSpritesAndRAM.operand_009D)
	ADD  HL,DE
	ADD  HL,BC
	ADD  A,L
	ADD  A,H
	SUB  $69
	JP   NZ,$0000
	RET 
; Function Name: function_0E70
function_0E70:
	db $78
	db $3C
	db $12
	db $3D
	db $1B
	db $12
	db $EF
	db $78
	db $C6
	db $02
	db $12
	db $3C
	db $13
	db $12
	db $21
	db $00
	db $FC
	db $19
	db $EF
	db $71
	db $2B
	db $71
	db $EB
	db $E7
	db $EB
	db $71
	db $23
	db $71
	db $C9
; Function Name: DrawLifeIcon
; Parameters:
; 	DE : VideoRAMAddr
; 	B : Character
; 	C : Attrib
DrawLifeIcon:
	EX   DE,HL
	LD   (HL),B			; write to VideoRAM
	DEC  HL
	LD   (HL),$F1			; write to videoRAM
	RES  2,H
	LD   (HL),C			; write to ColourRAM
	INC  HL
	LD   (HL),C			; write to Colour RAM
	SET  2,H
	EX   DE,HL
	RST  $28
	RET 
; Function Name: function_0E9C
function_0E9C:
	db $EB
	db $04
	db $70
	db $05
	db $2B
	db $70
	db $CB
	db $94
	db $71
	db $23
	db $71
	db $CB
	db $D4
	db $EB
	db $EF
	db $C9
; Function Name: function_0EAC
function_0EAC:
	LD   A,(data_AD01)
	CP   $64
	RET  NC
	LD   A,$0E
	CALL OutputStringColoured
	RST  $28
	RST  $28
	LD   HL,data_AD01
	LD   B,$01
	LD   A,(CharacterAttrib)
	LD   C,A
	PUSH BC
	LD   C,$00
	LD   A,(HL)
.label_0EC6:
	SUB  $0A
	JR   C,.label_0ECD
	INC  C
	JR   .label_0EC6
.label_0ECD:
	ADD  A,$0A
	EX   AF,AF'
	LD   A,C
	POP  BC
	CALL function_0EEB
	RST  $20
	EX   AF,AF'
	CALL function_0EEB
	RST  $20
	LD   DE,function_1748
	LD   BC,function_0F97.label_1077+21
.label_0EE1:
	LD   A,(DE)
	ADD  A,C
	LD   C,A
	INC  DE
	DJNZ .label_0EE1
	JP   NZ,OutputString_2UP
	RET 
; Function Name: function_0EEB
function_0EEB:
	AND  $0F
	JR   Z,.label_0EFF
	LD   B,$00
.label_0EF1:
	PUSH HL
	LD   HL,.label_0EFF+7
	RST  $08
	POP  HL
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	RET 
.label_0EFF:
	LD   A,B
	AND  A
	JR   Z,.label_0EF1
	DEC  B
	RST  $28
	RET 
	db $E3
	db $49
	db $A8
	db $64
	db $27
	db $AE
	db $42
	db $B0
	db $D5
	db $86
	db $F1
; Function Name: function_0F11
function_0F11:
	LD   HL,FunctionChecksum
	INC  (HL)
	XOR  A
	LD   (ProgramPhase),A
	RET 
; Function Name: AdvancePhase
AdvancePhase:
	LD   HL,ProgramPhase
	INC  (HL)
	RET 
; Function Name: ExecutePhaseFunction
ExecutePhaseFunction:
	LD   HL,label_0F54			; return point?
	PUSH HL
	LD   A,(ProgramPhase)
	AND  $0F
	RST  $30
	dw Phase0Function			; 0
	dw Phase1Function			; 1
	dw Phase2Function_RadialWipe			; 2
	dw Phase3Function			; 3
	dw Phase4Function			; 4
	dw Phase5Function			; 5
	dw Phase6Function			; 6
	dw Phase7Function			; 7
	dw Phase8Function			; 8
	dw Phase9Function			; 9
	dw PhaseAFunction			; A
	dw PhaseBFunction			; B
	dw PhaseCFunction			; C
	dw PhaseDFunction			; D
	dw PhaseEFunction			; E
	dw PhaseFFunction			; F
OutputString_AD1910:
	dw $A673
	db $14
	db $7E
	db $29
	db $F8
	db $96
	db $5D
	db $96
	db $13
	db $B9
; Function Name: label_0F54
label_0F54:
	LD   A,(data_AD30)
	AND  A
	RET  NZ
	LD   A,(Credits)
	AND  A
	JR   NZ,.credits_available
	LD   A,(data_A9C0)
	AND  A
	RET  Z
	LD   A,(Input0)			; check input
	AND  $18
	RET  Z
	CALL HideSprites
	JP   label_167B.label_1690
.credits_available:
	XOR  A
	LD   (ProgramPhase),A
	LD   A,(function_1734+2)
	LD   (FunctionChecksum),A
	RET 
; Function Name: function_0F7B
function_0F7B:
	ADD  A,A
	ADD  A,A
	LD   HL,data_186A
	LD   DE,data_A9D3
	RST  $18
	LDI
	LDI
	LDI
	LDI
	RET 
; Function Name: label_0F8D
label_0F8D:
	db $F1
	db $01
	db $F1
	db $02
	db $F1
	db $03
	db $F1
	db $04
	db $F1
	db $05
; Function Name: function_0F97
function_0F97:
	LD   A,(data_B411)
	BIT  7,A
	JR   Z,.label_0FB7
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_0FB7
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B411),A
	LD   A,(SpriteRam1)
	ADD  A,$80
	LD   (SpriteRam1),A
.label_0FB7:
	LD   A,(data_B413)
	BIT  7,A
	JR   Z,.label_0FD7
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_0FD7
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B413),A
	LD   A,(data_B012)
	ADD  A,$80
	LD   (data_B012),A
.label_0FD7:
	LD   A,(data_B415)
	BIT  7,A
	JR   Z,.label_0FF7
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_0FF7
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B415),A
	LD   A,(data_B014)
	ADD  A,$80
	LD   (data_B014),A
.label_0FF7:
	LD   A,(data_B437)
	BIT  7,A
	JR   Z,.label_1017
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1017
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B437),A
	LD   A,(data_B036)
	ADD  A,$80
	LD   (data_B036),A
.label_1017:
	LD   A,(data_B439)
	BIT  7,A
	JR   Z,.label_1037
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1037
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B439),A
	LD   A,(data_B038)
	ADD  A,$80
	LD   (data_B038),A
.label_1037:
	LD   A,(data_B43B)
	BIT  7,A
	JR   Z,.label_1057
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1057
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B43B),A
	LD   A,(data_B03A)
	ADD  A,$80
	LD   (data_B03A),A
.label_1057:
	LD   A,(data_B43D)
	BIT  7,A
	JR   Z,.label_1077
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1077
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B43D),A
	LD   A,(data_B03C)
	ADD  A,$80
	LD   (data_B03C),A
.label_1077:
	LD   A,(data_B43F)
	BIT  7,A
	JR   Z,.label_1097
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1097
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B43F),A
	LD   A,(data_B03E)
	ADD  A,$80
	LD   (data_B03E),A
.label_1097:
	RET 
; Function Name: function_1098
function_1098:
	LD   A,(data_B411)
	BIT  7,A
	JR   Z,.label_10B8			; jump if top bit not set
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)			; get scanline position
	ADD  A,C
	JR   NC,function_1098			; loop back if scanline not at desired location
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  %01111111
	LD   (data_B411),A
	LD   A,(SpriteRam1)
	ADD  A,$80
	LD   (SpriteRam1),A
.label_10B8:
	LD   A,(data_B413)
	BIT  7,A
	JR   Z,.label_10D8
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_10B8
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B413),A
	LD   A,(data_B012)
	ADD  A,$80
	LD   (data_B012),A
.label_10D8:
	LD   A,(data_B415)
	BIT  7,A
	JR   Z,.label_10F8
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_10D8
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B415),A
	LD   A,(data_B014)
	ADD  A,$80
	LD   (data_B014),A
.label_10F8:
	LD   A,(data_B437)
	BIT  7,A
	JR   Z,.label_1118
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_10F8
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B437),A
	LD   A,(data_B036)
	ADD  A,$80
	LD   (data_B036),A
.label_1118:
	LD   A,(data_B439)
	BIT  7,A
	JR   Z,.label_1138
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1118
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B439),A
	LD   A,(data_B038)
	ADD  A,$80
	LD   (data_B038),A
.label_1138:
	LD   A,(data_B43B)
	BIT  7,A
	JR   Z,.label_1158
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1138
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B43B),A
	LD   A,(data_B03A)
	ADD  A,$80
	LD   (data_B03A),A
.label_1158:
	LD   A,(data_B43D)
	BIT  7,A
	JR   Z,.label_1178
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1158
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B43D),A
	LD   A,(data_B03C)
	ADD  A,$80
	LD   (data_B03C),A
.label_1178:
	LD   A,(data_B43F)
	BIT  7,A
	JR   Z,.label_1198
	LD   C,A
	LD   A,(W_AudioCommand_R_Scanline)
	ADD  A,C
	JR   NC,.label_1178
	INC  HL
	INC  HL
	DEC  HL
	DEC  HL
	LD   A,C
	AND  $7F
	LD   (data_B43F),A
	LD   A,(data_B03E)
	ADD  A,$80
	LD   (data_B03E),A
.label_1198:
	RET 
; Function Name: Phase7Function
Phase7Function:
	CALL function_31B4
	CALL function_1EDF
	CALL function_23E3
	CALL function_36AF
	CALL function_0F97
	CALL function_47B3
	CALL function_43B7
	CALL function_28A1
	CALL function_0F97
	CALL function_2CBC
	CALL function_40D6
	CALL function_0F97
	CALL function_3B5F
	CALL function_3DDA
	CALL function_3E36
	CALL function_0F97
	CALL function_3FEA
	CALL function_4E4F
	CALL function_40B8
	CALL function_0F97
	CALL function_4DDE
	CALL function_5205
	CALL function_4D3A
	CALL DrawRemainingWaveTally
	CALL function_1098
	LD   A,(data_A800)
	INC  A
	JP   Z,OutputString_AndEvery50000.label_1271
	DEC  A
	RET  NZ
	CALL HideSprites
	LD   A,(data_ACC6)
	AND  A
	CALL NZ,function_2DB8
	CALL function_5634
	LD   HL,data_AD00
	DEC  (HL)
	PUSH AF
	LD   A,(data_AD32)
	AND  A
	LD   DE,data_AD10
	JR   Z,.label_120B
	db $11
	db $20
	db $AD
.label_120B:
	LD   HL,data_AD00
	LD   BC,RST10_AddDoubleAToHL
	LDIR
	POP  AF
	JR   Z,OutputString_AndEvery50000.label_1253
	LD   A,(data_AD32)
	AND  A
	LD   HL,data_AD20
	JR   Z,.label_1222
	db $21
	db $10
	db $AD
.label_1222:
	LD   A,(HL)
	AND  A
	JR   Z,.label_122F
.label_1226:
	db $3A
	db $32
	db $AD
	db $3C
	db $E6
	db $01
	db $32
	db $32
	db $AD
.label_122F:
	LD   A,$5A
	LD   (data_A9EB),A
	LD   A,(data_4B52)
	LD   (ProgramPhase),A
	RET 
OutputString_AndEvery50000:
	dw $A718
	db $13
	db $A5
	db $3B
	db $87
	db $F1
	db $34
	db $0E
	db $34
	db $D7
	db $BF
	db $F1
	db $7F
	db $13
	db $13
	db $13
	db $13
	db $F1
	db $88
	db $DC
	db $ED
	db $11
	db $B9
.label_1253:
	LD   A,(data_AD30)
	AND  A
	JP   Z,PhaseCFunction
.operand_125A:
	LD   DE,PerformRadialWipeStep+8
	LD   A,(data_AD32)
	AND  A
	JR   Z,.label_1264
	db $1C
.label_1264:
	RST  $38
	LD   DE,TamperDetected.label_0913+248
	RST  $38
	LD   A,$B4
	LD   (data_A9EB),A
	JP   AdvancePhase
.label_1271:
	LD   A,(RemainingShipsInWave)
	AND  A
	RET  NZ
	LD   A,(data_ACC6)
	AND  A
	RET  Z
	LD   HL,data_A80A+6
	LD   DE,RST10_AddDoubleAToHL
	LD   B,$0F
.label_1283:
	LD   A,(HL)
	AND  A
	RET  NZ
	ADD  HL,DE
	DJNZ .label_1283
	CALL function_5634
	LD   A,(data_AD30)
	AND  A
	JR   Z,.label_12BB
	db $21
	db $43
	db $AA
	db $06
	db $17
	db $AF
	db $77
	db $2C
	db $2C
	db $10
	db $FB
	db $CD
	db $B8
	db $2D
	db $3A
	db $32
	db $AD
	db $A7
	db $11
	db $10
	db $AD
	db $28
	db $03
	db $11
	db $20
	db $AD
	db $21
	db $00
	db $AD
	db $01
	db $10
	db $00
	db $ED
	db $B0
	db $3A
	db $35
	db $4A
	db $32
	db $AC
	db $A9
	db $C9
.label_12BB:
	LD   A,(InitProgram.operand_07CF+2)
	LD   (data_ACC6),A
	CALL HideSprites
	JP   PhaseCFunction
data_12C7:
	db $74
	db $B1
	db $CC
	db $EC
	db $5C
	db $16
	db $39
	db $50
	db $67
	db $21
	db $7A
	db $C5
	db $F7
	db $BE
	db $54
	db $80
	db $2F
	db $5F
	db $9F
	db $6D
	db $44
	db $B8
	db $E7
	db $BD
	db $89
	db $59
	db $1A
; Function Name: PhaseBFunction
PhaseBFunction:
	LD   HL,data_A9EB
	DEC  (HL)
	RET  NZ
.label_12E7:
	LD   A,(data_AD32)
	AND  A
	LD   HL,data_AD20
	JR   Z,.label_12F3
	db $21
	db $10
	db $AD
.label_12F3:
	LD   A,(HL)
	AND  A
	JP   NZ,Phase7Function.label_1226
	JP   AdvancePhase
; Function Name: PhaseCFunction
PhaseCFunction:
	XOR  A
	LD   (data_AD30),A
	LD   (ProgramPhase),A
	LD   (data_AD32),A
	LD   A,(Phase5Function.operand_16D2+1)
	LD   (FunctionChecksum),A
	LD   A,(data_4901)
	LD   HL,(data_4902)
	RST  $18
	XOR  H
	SUB  $9B
	LD   (ProgramPhase),A
	RET 
; Function Name: function_1319
function_1319:
	db $11
	db $E0
	db $FF
	db $06
	db $0D
	db $77
	db $19
	db $10
	db $FC
	db $C9
; Function Name: PhaseEFunction
PhaseEFunction:
	LD   A,(FrameCounter)
	AND  $02
	RET  NZ
	LD   A,(data_A9F0)
	AND  A
	JR   NZ,.label_1333
	CALL function_1367
	db $C9
.label_1333:
	db $3D
	db $20
	db $07
	db $CD
	db $67
	db $13
	db $CD
	db $2A
	db $14
	db $C9
	db $3D
	db $20
	db $07
	db $CD
	db $93
	db $13
	db $CD
	db $C5
	db $14
	db $C9
	db $3D
	db $20
	db $04
	db $CD
	db $C5
	db $14
	db $C9
	db $3D
	db $20
	db $04
	db $CD
	db $CC
	db $13
	db $C9
	db $3E
	db $5A
	db $32
	db $EB
	db $A9
	db $CD
	db $B6
	db $15
	db $CD
	db $75
	db $4C
	db $3A
	db $50
	db $27
	db $32
	db $AC
	db $A9
	db $C9
; Function Name: function_1367
function_1367:
	db $3A
	db $F1
	db $A9
	db $FE
	db $08
	db $20
	db $08
	db $3E
	db $01
	db $32
	db $F0
	db $A9
	db $CD
	db $11
	db $58
	db $3A
	db $F1
	db $A9
	db $E6
	db $01
	db $3E
	db $3E
	db $28
	db $02
	db $3E
	db $00
	db $47
	db $3A
	db $40
	db $AA
	db $E6
	db $C0
	db $80
	db $32
	db $40
	db $AA
	db $3A
	db $F1
	db $A9
	db $3C
	db $32
	db $F1
	db $A9
	db $C9
	db $3A
	db $F3
	db $A9
	db $A7
	db $20
	db $09
	db $3E
	db $03
	db $32
	db $F0
	db $A9
	db $3E
	db $3F
	db $18
	db $18
	db $E6
	db $04
	db $20
	db $04
	db $3E
	db $3F
	db $18
	db $10
	db $3D
	db $20
	db $04
	db $3E
	db $36
	db $18
	db $09
	db $3D
	db $20
	db $04
	db $3E
	db $3E
	db $18
	db $02
	db $3E
	db $37
	db $47
	db $3A
	db $40
	db $AA
	db $E6
	db $C0
	db $80
	db $32
	db $40
	db $AA
	db $3A
	db $F3
	db $A9
	db $3D
	db $32
	db $F3
	db $A9
	db $C9
	db $3E
	db $05
	db $32
	db $F0
	db $A9
	db $3A
	db $32
	db $AD
	db $A7
	db $3A
	db $1C
	db $AD
	db $47
	db $28
	db $04
	db $3A
	db $2C
	db $AD
	db $47
	db $3A
	db $87
	db $A9
	db $A7
	db $78
	db $28
	db $22
	db $21
	db $44
	db $A0
	db $11
	db $45
	db $A0
	db $D9
	db $06
	db $1C
	db $D9
	db $01
	db $1A
	db $00
	db $77
	db $ED
	db $B0
	db $11
	db $06
	db $00
	db $19
	db $54
	db $5D
	db $13
	db $D9
	db $10
	db $EF
	db $3A
	db $F6
	db $A9
	db $3D
	db $32
	db $F6
	db $A9
	db $C9
	db $21
	db $BE
	db $A3
	db $11
	db $BD
	db $A3
	db $D9
	db $06
	db $1C
	db $D9
	db $01
	db $1A
	db $00
	db $77
	db $ED
	db $B8
	db $11
	db $FA
	db $FF
	db $19
	db $54
	db $5D
	db $1B
	db $D9
	db $10
	db $EF
	db $3A
	db $F6
	db $A9
	db $3D
	db $32
	db $F6
	db $A9
	db $C9
	db $3A
	db $F2
	db $A9
	db $CB
	db $47
	db $28
	db $6C
	db $2A
	db $F7
	db $A9
	db $7E
	db $FE
	db $FF
	db $20
	db $12
	db $3E
	db $00
	db $32
	db $F2
	db $A9
	db $3E
	db $02
	db $32
	db $F0
	db $A9
	db $2A
	db $F7
	db $A9
	db $2B
	db $22
	db $F7
	db $A9
	db $C9
	db $CD
	db $63
	db $15
	db $2A
	db $F7
	db $A9
	db $7E
	db $E6
	db $01
	db $23
	db $22
	db $F7
	db $A9
	db $28
	db $0F
	db $11
	db $20
	db $00
	db $21
	db $F0
	db $A5
	db $34
	db $19
	db $34
	db $21
	db $F2
	db $A5
	db $34
	db $19
	db $34
	db $2A
	db $F7
	db $A9
	db $7E
	db $E6
	db $01
	db $23
	db $22
	db $F7
	db $A9
	db $28
	db $09
	db $11
	db $20
	db $00
	db $21
	db $F1
	db $A5
	db $34
	db $19
	db $34
	db $0E
	db $02
	db $11
	db $D1
	db $A5
	db $CD
data_1484:
	db $9D
	db $4A
	db $2A
	db $F7
	db $A9
	db $11
	db $F3
	db $FF
	db $19
	db $22
	db $F7
	db $A9
	db $0E
	db $00
	db $11
	db $31
	db $A6
	db $CD
	db $9D
	db $4A
	db $CD
	db $8C
	db $15
	db $18
	db $20
	db $3E
	db $F1
	db $21
	db $B1
	db $A7
	db $CD
	db $19
	db $13
	db $21
	db $D1
	db $A5
	db $CD
	db $19
	db $13
	db $21
	db $10
	db $A6
	db $77
	db $19
	db $77
	db $21
data_14B2:
	db $11
	db $A6
	db $77
	db $19
	db $77
	db $21
	db $12
	db $A6
	db $77
	db $19
	db $77
	db $3A
	db $F2
	db $A9
	db $3D
	db $32
	db $F2
	db $A9
	db $C9
	db $3A
	db $F4
	db $A9
	db $CB
	db $47
	db $28
	db $6F
	db $2A
	db $F7
	db $A9
	db $7E
	db $E6
	db $FE
	db $28
	db $15
	db $3E
	db $00
	db $32
	db $F4
	db $A9
	db $3E
	db $04
	db $32
	db $F0
	db $A9
	db $CD
	db $E4
	db $56
	db $2A
	db $F7
	db $A9
	db $23
	db $22
	db $F7
	db $A9
	db $C9
	db $CD
	db $63
	db $15
	db $0E
	db $01
	db $11
	db $51
	db $A4
	db $CD
	db $9D
	db $4A
	db $2A
	db $F7
	db $A9
	db $11
	db $0D
	db $00
	db $19
	db $22
	db $F7
	db $A9
	db $0E
	db $03
	db $11
	db $B1
	db $A7
	db $CD
	db $9D
	db $4A
	db $2A
	db $F7
	db $A9
	db $7E
	db $E6
	db $01
	db $2B
	db $22
	db $F7
	db $A9
	db $28
	db $09
	db $11
	db $20
	db $00
	db $21
	db $F1
	db $A5
	db $35
	db $19
	db $35
	db $2A
	db $F7
	db $A9
	db $7E
	db $E6
	db $01
	db $2B
	db $22
	db $F7
	db $A9
	db $28
	db $0F
	db $11
	db $20
	db $00
	db $21
	db $F0
	db $A5
	db $35
	db $19
	db $35
	db $21
	db $F2
	db $A5
	db $35
	db $19
	db $35
	db $CD
	db $8C
	db $15
	db $18
	db $20
	db $3E
	db $F1
	db $21
	db $B1
	db $A7
	db $CD
	db $19
	db $13
	db $21
	db $D1
	db $A5
	db $CD
	db $19
	db $13
	db $21
	db $10
	db $A6
	db $77
	db $19
	db $77
	db $21
data_1550:
	db $11
	db $A6
	db $77
	db $19
	db $77
	db $21
	db $12
	db $A6
	db $77
	db $19
	db $77
	db $3A
	db $F4
	db $A9
	db $3D
	db $32
	db $F4
	db $A9
	db $C9
	db $11
	db $00
	db $A4
	db $21
	db $51
	db $A4
	db $01
	db $20
	db $00
	db $D9
	db $06
	db $1C
	db $D9
	db $1A
	db $77
	db $13
	db $09
	db $D9
	db $10
	db $F8
	db $D9
	db $21
	db $F0
	db $A5
	db $1A
	db $77
	db $09
	db $13
	db $1A
	db $77
	db $13
	db $21
	db $F2
	db $A5
	db $1A
	db $77
	db $09
	db $13
	db $1A
	db $77
	db $C9
	db $11
	db $00
	db $A4
	db $21
	db $51
	db $A4
	db $01
	db $20
	db $00
	db $D9
	db $06
	db $1C
	db $D9
	db $7E
	db $12
	db $13
	db $09
	db $D9
	db $10
	db $F8
	db $D9
	db $21
	db $F0
	db $A5
	db $7E
	db $12
	db $09
	db $13
	db $7E
	db $12
	db $13
	db $21
	db $F2
	db $A5
	db $7E
	db $12
	db $09
	db $13
	db $7E
	db $12
	db $C9
; Function Name: PhaseFFunction
PhaseFFunction:
	RET 
; Function Name: HideSprites
HideSprites:
	LD   HL,PlayerSprite_Y
	LD   B,$18
	XOR  A
.hide_sprites_loop:
	LD   (HL),A
	INC  L
	INC  L
	DJNZ .hide_sprites_loop
	RET 
; Function Name: function_15C2
function_15C2:
	LD   A,(ProgramPhase)			; a has to be 0 or it'll break
.operand_15C5:
	AND  $07
	RST  $30
	dw function_15E2
OutputString_Credit:
	dw $A55F
	db $13
	db $77
	db $D7
	db $34
	db $87
	db $FD
	db $DC
	db $B9
	db $FE
	db $15
OutputString_HI_SCORE:
	dw $A660			; address
	db $14			; attribute
	db $C4
	db $FD
	db $10
	db $ED
	db $77
	db $68
	db $D7
	db $34
	db $B9			; terminator
; Function Name: function_15E2
function_15E2:
	CALL function_019A
	LD   A,(function_1748+1)			; set phase 6
	LD   (ProgramPhase),A
	LD   C,$00
	LD   HL,function_5634+20
	LD   A,(FunctionChecksum)
.checksum_loop:
	SUB  (HL)
	INC  HL
	DEC  C
	JR   NZ,.checksum_loop
	XOR  $4E
	LD   (FunctionChecksum),A
	RET 
.operand_15FE:
	CALL function_01C2
	RET  NZ
	LD   DE,InterruptRoutine.label_00D9+44			; String 'HI-SCORE'
	RST  $38
	INC  E			; 1-UP
	RST  $38
	INC  E			; 2-UP
	RST  $38
	LD   DE,CopySpriteDataToHW.CopySpritesFlipped+171
	RST  $38
	LD   A,$13			; '0'
	LD   (VideoRAM+769),A			; write '00'
	LD   (VideoRAM+737),A
	LD   HL,data_163F
	LD   B,$06			; 6 loop iterations
.label_161B:
	LD   E,(HL)			; get address
	INC  HL
	LD   D,(HL)
	INC  HL
	LD   A,(HL)			; get value
	LD   (DE),A			; write value
	INC  DE
	EX   DE,HL
	LD   (HL),$05			; write to next byte
	EX   DE,HL
	INC  HL
	DJNZ .label_161B
	CALL function_0D6B
	LD   A,$01
	LD   (FunctionChecksum),A
	INC  A
	LD   (ProgramPhase),A			; phase 2
	LD   A,(data_A9C0)
	AND  A
	RET  Z
	LD   DE,$010D			; draw 'Free Play' string
	RST  $38
	RET 
data_163F:
	dw CopyRightStringChar
	db $FD
	dw $AD39
	db $68
	dw data_AB43
	db $7C
	dw data_ABFE
	db $A5
	dw $ACBE
	db $38
	dw data_ACC7
	db $3B
; Function Name: function_1651
function_1651:
	LD   HL,label_167B
	PUSH HL			; return address?
	LD   A,(ProgramPhase)			; pick function to execute
	RST  $30
	dw function_074B
	dw function_1734
	dw function_2D3F
	dw QueueTitleScreenDraw
	dw function_1748
	dw CheckCopyrightString
	dw function_178C
	dw function_17B9
	dw label_3252
	dw function_17E2
	dw label_4B19
	dw label_17FB
	db $30
	db $27
OutputString_Play:
	dw $A626
data_1675:
	db $13
	db $88
	db $57
	db $A5
	db $BF
	db $B9
; Function Name: label_167B
label_167B:
	LD   A,(Credits)
	AND  A
	JP   NZ,function_0F11
	LD   A,(data_A9C0)
	AND  A
	RET  Z
	LD   A,(Input0)
	AND  %00011000
	RET  Z
	CALL HideSprites
.label_1690:
	LD   A,(Input0)
	BIT  4,A
	JR   NZ,.label_169C
	BIT  3,A
	JR   NZ,label_1719
	RET 
.label_169C:
	LD   A,$FF
	LD   (data_AD30),A
	LD   (data_AD31),A
	LD   A,(data_A9C1)
	LD   (data_AD10),A
	LD   (data_AD20),A
	JR   label_1719.label_172A
; Function Name: Phase5Function
Phase5Function:
	LD   B,$00
	LD   HL,function_4D72.label_4D9F
	LD   A,(FunctionChecksum)
.label_16B7:
	SUB  (HL)
	INC  HL
	DJNZ .label_16B7
	XOR  $A2
	LD   (FunctionChecksum),A
	CALL function_0F97
	CALL function_1EDF
	CALL function_0F97
	CALL function_2CBC
	CALL function_1098
	LD   A,(FrameCounter)
.operand_16D2:
	AND  $01
	JR   Z,.label_16F2
	LD   HL,data_A9EB
	DEC  (HL)
	JR   NZ,.label_16F2
.operand_16DC:
	LD   DE,CoordinatePairs+121
	RST  $38
	LD   E,$0E
	RST  $38
	LD   E,$1A
	RST  $38
	XOR  A
	LD   (data_AD0E),A
	LD   A,$2A
	LD   (data_A9EB),A
	JP   AdvancePhase
.label_16F2:
	LD   A,(data_AD0E)
	AND  A
	RET  Z
	LD   A,(FrameCounter)
	AND  $0F
	JR   Z,.label_1707
	CP   $05
	JR   Z,.label_170B
	CP   $0A
	JR   Z,.label_170F
	RET 
.label_1707:
	LD   D,$02
	JR   .label_1711
.label_170B:
	LD   D,$0A
	JR   .label_1711
.label_170F:
	LD   D,$0B
.label_1711:
	LD   A,(data_AD04)
	ADD  A,$1A
	LD   E,A
	RST  $38
	RET 
; Function Name: label_1719
label_1719:
	XOR  A
	LD   (data_AD31),A
	LD   (data_AD20),A
	DEC  A
	LD   (data_AD30),A
	LD   A,(data_A9C1)
	LD   (data_AD10),A
.label_172A:
	LD   A,$03
	LD   (FunctionChecksum),A
	XOR  A
	LD   (ProgramPhase),A
	RET 
; Function Name: function_1734
function_1734:
	CALL PerformRadialWipeStep
	RET  NZ
	LD   HL,function_1748
	LD   B,$22
	XOR  A
.label_173E:
	SUB  (HL)
	INC  HL
	DJNZ .label_173E
	LD   (FrameCountdown1),A
	JP   AdvancePhase
; Function Name: function_1748
function_1748:
	CALL SetKonamicSpriteParams
	CALL QueueKonamiLogoDraw
	LD   HL,data_A9EB
	DEC  (HL)			; dec timer?
	RET  NZ
	LD   HL,VideoRAM+572
	LD   DE,data_ACC7
	LD   A,(HL)			; read video ram char
	LD   (DE),A			; store
	INC  DE			; inc dest
	RES  2,H
	LD   A,(HL)			; read colour RAM
	LD   (DE),A
	LD   DE,CoordinatePairs+115			; command 3 - clear string
	RST  $38			; Please Deposit Coin
	INC  E
	RST  $38			; And Try This Game
.operand_1766:
	JP   AdvancePhase
	db $31
; Function Name: CheckCopyrightString
CheckCopyrightString:
	CALL CheckCopyrightAttributes
	LD   A,(VideoRAM+636)			; check for character in copyright string
	CP   $7C
	JP   NZ,OutputString_FirstBonus20000			; crash
	LD   DE,InterruptRoutine.label_0106+13			; 'SCORE RANKING TABLE'
	RST  $38
	CALL function_4BDC
	LD   HL,VideoRAM+476			; last letter of Konami
	LD   DE,CopyRightStringChar
	LD   A,(HL)			; read screen char
	LD   (DE),A			; store
	INC  DE
	RES  2,H
	LD   A,(HL)			; read attribute
	LD   (DE),A			; store
	JP   AdvancePhase
; Function Name: function_178C
function_178C:
	CALL SetKonamicSpriteParams
	CALL QueueKonamiLogoDraw
	LD   HL,data_A9EB
	DEC  (HL)
	RET  NZ
	CALL CheckCopyrightAttributes
	LD   A,(function_47B3)
	ADD  A,$02
	LD   L,A
	ADD  A,$6A
	LD   H,A
	LD   A,(HL)
	CP   $3B
	JP   NZ,OutputString_Credit			; crash
	LD   HL,VideoRAM+636			; K of Konami
	LD   DE,data_AB43			; Store char & attribute
	LD   A,(HL)
	LD   (DE),A
	INC  DE
	RES  2,H
	LD   A,(HL)
	LD   (DE),A
	JP   AdvancePhase
; Function Name: function_17B9
function_17B9:
	LD   A,(data_590D)
	LD   C,A
	LD   A,(PhaseDFunction.operand_4A3F+1)
	LD   HL,SetKonamicSpriteParams
	LD   B,$33
.label_17C5:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_17C5
	CP   $EF
	JP   Z,AdvancePhase
	db $3A
	db $89
	db $4C
	db $32
	db $08
	db $C3
	db $21
	db $5C
	db $A6
	db $11
	db $39
	db $AD
	db $7E
	db $12
	db $13
	db $CB
	db $94
	db $7E
	db $12
	db $C9
; Function Name: function_17E2
function_17E2:
	LD   A,$FF
	LD   (data_AA3F),A
	LD   DE,function_17B9
	LD   C,$08
	CALL function_4BD9
	LD   A,(Phase0Function.operand_27BE+2)
	CALL function_291E
	LD   (data_AA6F),A
	JP   AdvancePhase
; Function Name: label_17FB
label_17FB:
	JP   AdvancePhase
; Function Name: function_17FE
function_17FE:
	LD   HL,label_181D
	PUSH HL
	LD   A,(ProgramPhase)
	RST  $30
	dw label_181E
	dw label_2CDB
	dw label_1830
	dw label_07E6
	dw data_188A
Outputstring_GameOver:
	dw $A672
	db $14
	db $7D
	db $A5
	db $38
	db $34
	db $F1
	db $68
	db $0E
	db $34
	db $D7
	db $B9
; Function Name: label_181D
label_181D:
	RET 
; Function Name: label_181E
label_181E:
	CALL HideSprites
	LD   HL,VideoRAM+508
	LD   DE,data_AC7F+63
	CALL function_1AFC
	CALL function_01B5
	JP   AdvancePhase
; Function Name: label_1830
label_1830:
	CALL SetKonamicSpriteParams
	CALL QueueKonamiLogoDraw
	LD   DE,InterruptRoutine.label_00D9+40			; output string 'PLAY'
	RST  $38
	LD   E,$14			; Time Pilot Logo - top
	RST  $38
	INC  E			; Time Pilot Logo - bottom
	RST  $38
	LD   E,$0F			; 1st bonus text
	LD   A,(BonusScheme)
	AND  A
	JR   Z,.skip_harderbonus
	INC  E			; advance to harder bonus strings
	INC  E
.skip_harderbonus:
	RST  $38			; 1st bonus points
	INC  E
	RST  $38			; subsequent bonus points
	LD   E,$16
	RST  $38			; 'PUSH START BUTTON'
	LD   E,$00
	RST  $38			; Konami (C)
	LD   A,(Credits)
	CP   $02			; 2 credits or more to allow 2 players
	JR   NC,.two_player_available
	LD   DE,InterruptRoutine.label_0106+17			; one player only
	RST  $38
	JP   AdvancePhase
.two_player_available:
	LD   DE,InterruptRoutine.label_0106+19			; one or two players
	RST  $38
	CALL AdvancePhase			; advance phase twice?
	JP   AdvancePhase
data_186A:
	db $00
	db $02
	db $06
	db $0D
	db $00
	db $03
	db $07
	db $0C
	db $00
	db $04
	db $08
	db $0B
	db $02
	db $06
	db $0A
	db $0A
	db $04
	db $08
	db $0C
	db $09
	db $07
	db $0A
	db $0D
	db $07
	db $0B
	db $0D
	db $0E
	db $05
	db $0F
	db $0F
	db $0F
	db $05
data_188A:
	db $CD
	db $06
	db $0B
	db $CD
	db $39
	db $0B
	db $3A
	db $AE
	db $A9
	db $CB
	db $67
	db $C2
	db $9E
	db $18
	db $CB
	db $5F
	db $C2
	db $15
	db $32
	db $C9
	db $CD
	db $2B
	db $0B
	db $3E
	db $FF
	db $32
	db $30
	db $AD
	db $32
	db $31
	db $AD
	db $3A
	db $C1
	db $A9
	db $32
	db $10
	db $AD
	db $32
	db $20
	db $AD
	db $CD
	db $0E
	db $46
	db $21
	db $86
	db $A9
	db $7E
	db $D6
	db $02
	db $27
	db $77
	db $CD
	db $FB
	db $4A
	db $C3
	db $2A
	db $17
; Function Name: PhaseAFunction
PhaseAFunction:
	LD   A,(FrameCounter)
	AND  $01
	JP   NZ,.label_1984
	CALL function_1ED1
	db $21
	db $95
	db $A9
	db $0F
	db $CB
	db $16
	db $23
	db $0F
	db $CB
	db $16
	db $23
	db $0F
	db $0F
	db $0F
	db $CB
	db $16
	db $23
	db $0F
	db $CB
	db $16
	db $7E
	db $E6
	db $07
	db $3D
	db $28
	db $3B
	db $2B
	db $7E
	db $E6
	db $07
	db $3D
	db $28
	db $34
	db $2B
	db $7E
	db $FE
	db $FF
	db $CC
	db $80
	db $19
	db $E6
	db $07
	db $3D
	db $28
	db $1B
	db $2B
	db $7E
	db $FE
	db $7F
	db $CC
	db $80
	db $19
	db $E6
	db $07
	db $3D
	db $28
	db $02
	db $18
	db $5A
	db $21
	db $99
	db $A9
	db $35
	db $7E
	db $FE
	db $80
	db $38
	db $3C
	db $36
	db $1A
	db $18
	db $38
	db $21
	db $99
	db $A9
	db $34
	db $7E
	db $FE
	db $1B
	db $38
	db $2F
	db $36
	db $00
	db $18
	db $2B
	db $3A
	db $99
	db $A9
	db $21
	db $C7
	db $12
	db $CF
	db $2A
	db $91
	db $A9
	db $ED
	db $5B
	db $93
	db $A9
	db $12
	db $77
	db $3A
	db $90
	db $A9
	db $CB
	db $92
	db $12
	db $CB
	db $D2
	db $E7
	db $23
	db $22
	db $91
	db $A9
	db $ED
	db $53
	db $93
	db $A9
	db $21
	db $9A
	db $A9
	db $35
	db $28
	db $2B
	db $AF
	db $32
	db $99
	db $A9
	db $ED
	db $5B
	db $93
	db $A9
	db $3A
	db $99
	db $A9
	db $21
	db $C7
	db $12
	db $CF
	db $12
	db $CB
	db $92
	db $3E
	db $10
	db $12
	db $AF
	db $32
	db $9C
	db $A9
	db $3A
	db $80
	db $A9
	db $E6
	db $07
	db $20
	db $30
	db $21
	db $EB
	db $A9
	db $35
	db $20
	db $2A
	db $2A
	db $93
	db $A9
	db $36
	db $F1
	db $3E
	db $3C
	db $32
	db $EB
	db $A9
	db $CD
	db $34
	db $56
	db $C3
	db $1A
	db $0F
	db $36
	db $00
	db $AF
	db $C9
.label_1984:
	db $21
	db $9C
	db $A9
	db $34
	db $2A
	db $93
	db $A9
	db $CB
	db $94
	db $3A
	db $9C
	db $A9
	db $CB
	db $67
	db $28
	db $04
	db $36
	db $14
	db $18
	db $02
	db $36
	db $10
	db $21
	db $20
	db $AD
	db $3A
	db $10
	db $AD
	db $B6
	db $C0
	db $3A
	db $C0
	db $A9
	db $A7
	db $20
	db $26
	db $3A
	db $86
	db $A9
	db $FE
	db $01
	db $D8
	db $28
	db $10
	db $3A
	db $AE
	db $A9
	db $E6
	db $18
	db $C8
	db $FE
	db $08
	db $28
	db $0E
	db $CD
	db $B6
	db $15
	db $C3
	db $9E
	db $18
	db $3A
	db $AE
	db $A9
	db $E6
	db $18
	db $FE
	db $08
	db $C0
	db $CD
	db $B6
	db $15
	db $C3
	db $15
	db $32
	db $3A
	db $AE
	db $A9
	db $E6
	db $18
	db $C8
	db $CD
	db $B6
	db $15
	db $C3
	db $90
	db $16
; Function Name: CheckCopyrightAttributes
CheckCopyrightAttributes:
	LD   HL,ColourRAM+700
	LD   B,$0D
.attrib_loop:
	LD   A,(HL)			; read attrib
	CP   $10
	JR   Z,.attrib_match
	CP   $05
	JP   NZ,OutputString_AndTryThisGame			; crash
.attrib_match:
	LD   DE,data_FFE0			; go up one char
	ADD  HL,DE
	DJNZ .attrib_loop
	RET 
; Function Name: function_19F0
function_19F0:
	LD   HL,$0000
	LD   (data_A808),HL
	LD   (data_A80A),HL
	LD   (data_AD06),HL
	XOR  A
	LD   (data_AD0D),A
	LD   (data_A8F7),A
	LD   (data_AD05),A
	LD   A,(data_A9D6)
	LD   (data_A9D7),A
	LD   A,(data_AD0A)
	LD   (data_ACC0),A
	XOR  A
	LD   (data_AA81),A
	LD   (data_ACC6),A
	LD   A,$80
	LD   (PlayerAngle),A
	XOR  A
	LD   (data_A801),A
	LD   A,$FF
	LD   (data_A800),A
	LD   A,$78
	LD   (PlayerSprite_Y),A
	LD   A,$84
	LD   (PlayerSprite_X),A
	CALL UpdatePlayerSprite
	CALL function_2755
	LD   IX,data_A8C0
	LD   IY,data_AA28
	CALL function_3C0D
	LD   B,$07
	LD   IX,data_A844+12
	LD   IY,PlayerSprite_No+9
	LD   IX,data_A8E0
.operand_1A50:
	LD   IY,data_AA2A+2
	CALL function_3DFB
	LD   IX,data_A8E6+10
	LD   IY,data_AA2A+4
	CALL function_48AD
.label_1A62:
	CALL function_2BDE
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	DJNZ .label_1A62
	CALL function_1AE4
	LD   IY,data_AA28
	LD   (IY+$00),$00
	LD   (IY+$02),$00
	LD   (IY+$04),$00
	LD   (IY+$06),$00
	LD   (IY+$31),$00
	LD   (IY+$33),$00
	LD   (IY+$35),$00
	LD   (IY+$37),$00
	CALL function_30A5
.label_1A9A:
	LD   A,(data_AD04)
	RLCA
	RLCA
	RLCA
	RLCA
	AND  $F0
	LD   B,A
	LD   A,(data_ACC0)
	ADD  A,B
	LD   HL,function_1AFC+8
	RST  $10
	LD   A,(DE)
	LD   (data_A844),A
	INC  DE
	LD   A,(DE)
	LD   (data_A837),A
	INC  DE
	LD   A,(DE)
	LD   (data_A827),A
	INC  DE
	LD   A,(DE)
	LD   (FrameCountdown1),A
	LD   (data_A814),A
	INC  DE
	LD   A,(DE)
	LD   (data_ACC1),A
	INC  DE
	LD   A,(DE)
	LD   (data_ACC4),A
	INC  DE
	LD   A,(DE)
	LD   (data_A8C6),A
	INC  DE
	LD   A,(DE)
	LD   (data_A8D6),A
	INC  DE
	LD   A,(DE)
	LD   (data_A8E6),A
	INC  DE
	LD   A,(DE)
	LD   (FrameCountdown3),A
	LD   (data_A8F6),A
	RET 
; Function Name: function_1AE4
function_1AE4:
	LD   IX,data_A80A+6
	LD   A,$01
	LD   B,$17
	LD   DE,RST10_AddDoubleAToHL
.label_1AEF:
	LD   (IX+$00),$00
	LD   (IX+$0F),A
	INC  A
	ADD  IX,DE
	DJNZ .label_1AEF
	RET 
; Function Name: function_1AFC
function_1AFC:
	LD   A,(HL)
	LD   (DE),A
	INC  DE
	RES  2,H
	LD   A,(HL)
	LD   (DE),A
	RET 
	db $B1
	db $1B
	db $BB
	db $1B
	db $C5
	db $1B
	db $CF
	db $1B
	db $D9
	db $1B
	db $E3
	db $1B
	db $ED
	db $1B
	db $F7
	db $1B
	db $01
	db $1C
	db $0B
	db $1C
	db $15
	db $1C
	db $1F
	db $1C
	db $29
	db $1C
	db $33
	db $1C
	db $3D
	db $1C
	db $47
	db $1C
	db $51
	db $1C
	db $5B
	db $1C
	db $65
	db $1C
	db $6F
	db $1C
	db $79
	db $1C
	db $83
	db $1C
	db $8D
	db $1C
	db $97
	db $1C
	db $A1
	db $1C
	db $AB
	db $1C
	db $B5
	db $1C
	db $BF
	db $1C
	db $C9
	db $1C
	db $D3
	db $1C
	db $DD
	db $1C
	db $E7
	db $1C
	db $F1
	db $1C
	db $FB
	db $1C
	db $05
	db $1D
	db $0F
	db $1D
	db $19
	db $1D
	db $23
	db $1D
	db $2D
	db $1D
	db $37
	db $1D
	db $41
	db $1D
	db $4B
	db $1D
	db $55
	db $1D
	db $5F
	db $1D
	db $69
	db $1D
	db $73
	db $1D
	db $7D
	db $1D
	db $87
	db $1D
	db $91
	db $1D
	db $9B
	db $1D
	db $A5
	db $1D
	db $AF
	db $1D
	db $B9
	db $1D
	db $C3
	db $1D
	db $CD
	db $1D
	db $D7
	db $1D
	db $E1
	db $1D
	db $EB
	db $1D
	db $F5
	db $1D
	db $FF
	db $1D
	db $09
	db $1E
	db $13
	db $1E
	db $1D
	db $1E
	db $27
	db $1E
	db $31
	db $1E
	db $3B
	db $1E
	db $45
	db $1E
	db $4F
	db $1E
	db $59
	db $1E
	db $63
	db $1E
	db $6D
	db $1E
	db $77
	db $1E
	db $81
	db $1E
	db $8B
	db $1E
	db $95
	db $1E
	db $9F
	db $1E
	db $A9
	db $1E
	db $B3
	db $1E
	db $BD
	db $1E
	db $C7
	db $1E
OutputString_FreePlay:
	dw $A55F
	db $13
	db $00
	db $D7
	db $34
	db $34
	db $F1
	db $88
	db $57
	db $A5
	db $BF
	db $B9
	db $00
	db $20
	db $50
	db $3C
	db $04
	db $50
	db $00
	db $50
	db $18
	db $5A
	db $01
	db $20
	db $4E
	db $3C
	db $04
	db $50
	db $00
	db $4E
	db $18
	db $54
	db $01
	db $28
	db $4C
	db $32
	db $05
	db $60
	db $01
	db $4C
	db $1C
	db $4E
	db $02
	db $28
	db $48
	db $28
	db $05
	db $60
	db $01
	db $48
	db $1C
	db $48
	db $02
	db $30
	db $46
	db $1E
	db $06
	db $70
	db $01
	db $46
	db $1C
	db $42
	db $03
	db $30
	db $44
	db $1E
	db $06
	db $70
	db $02
	db $44
	db $20
	db $3C
	db $03
	db $38
	db $42
	db $1E
	db $06
	db $80
	db $02
	db $42
	db $20
	db $36
	db $03
	db $38
	db $40
	db $1E
	db $06
	db $80
	db $02
	db $40
	db $20
	db $30
	db $04
	db $40
	db $3F
	db $1E
	db $07
	db $90
	db $03
	db $3F
	db $24
	db $2A
	db $04
	db $40
	db $3E
	db $1E
	db $07
	db $90
	db $03
	db $3E
	db $24
	db $24
	db $04
	db $40
	db $3D
	db $1E
	db $07
	db $A0
	db $03
	db $3D
	db $24
	db $1E
	db $04
	db $40
	db $3C
	db $1E
	db $07
	db $B0
	db $03
	db $3C
	db $28
	db $1E
	db $04
	db $48
	db $3B
	db $1E
	db $07
	db $C0
	db $03
	db $3B
	db $28
	db $1E
	db $04
	db $48
	db $3A
	db $1E
	db $07
	db $D0
	db $03
	db $3A
	db $2C
	db $1E
	db $04
	db $48
	db $39
	db $1E
	db $07
	db $E0
	db $03
	db $39
	db $30
	db $1E
	db $04
	db $48
	db $38
	db $19
	db $07
	db $F0
	db $03
	db $38
	db $30
	db $19
	db $01
	db $28
	db $48
	db $32
	db $05
	db $50
	db $01
	db $5C
	db $00
	db $1E
	db $01
	db $28
	db $48
	db $28
	db $05
	db $50
	db $01
	db $5A
	db $00
	db $1E
	db $02
	db $30
	db $48
	db $1E
	db $05
	db $60
	db $01
	db $58
	db $00
	db $1E
	db $02
	db $30
	db $48
	db $1E
	db $06
	db $60
	db $01
	db $56
	db $00
	db $1E
	db $02
	db $30
	db $48
	db $1E
	db $06
	db $70
	db $02
	db $54
	db $00
	db $1E
	db $03
	db $38
	db $40
	db $1E
	db $06
	db $70
	db $02
	db $52
	db $00
	db $1E
	db $03
	db $38
	db $40
	db $1E
	db $06
	db $80
	db $02
	db $50
	db $00
	db $1E
	db $03
	db $38
	db $40
	db $1E
	db $06
	db $80
	db $02
	db $4C
	db $00
	db $1E
	db $04
	db $40
	db $40
	db $1E
	db $07
	db $90
	db $02
	db $4C
	db $00
	db $1E
	db $04
	db $40
	db $40
	db $1E
	db $07
	db $90
	db $02
	db $48
	db $00
	db $1E
	db $04
	db $48
	db $38
	db $1E
	db $07
	db $A0
	db $02
	db $48
	db $00
	db $1E
	db $04
	db $48
	db $38
	db $1E
	db $07
	db $B0
	db $02
	db $48
	db $00
	db $1E
	db $04
	db $48
	db $38
	db $1E
	db $07
	db $C0
	db $02
	db $48
	db $00
	db $1E
	db $04
	db $48
	db $38
	db $1E
	db $07
	db $D0
	db $02
	db $48
	db $00
	db $1E
	db $04
	db $50
	db $38
	db $1E
	db $07
	db $E0
	db $02
	db $48
	db $00
	db $1E
	db $04
	db $58
	db $30
	db $19
	db $07
	db $F0
	db $02
	db $48
	db $00
	db $19
	db $01
	db $20
	db $50
	db $32
	db $03
	db $50
	db $01
	db $50
	db $08
	db $1E
	db $01
	db $20
	db $50
	db $28
	db $04
	db $50
	db $01
	db $50
	db $08
	db $1E
	db $01
	db $20
	db $50
	db $1E
	db $04
	db $60
	db $01
	db $50
	db $0C
	db $1E
	db $01
	db $28
	db $50
	db $1E
	db $04
	db $60
	db $02
	db $50
	db $0C
	db $1E
	db $01
	db $28
	db $48
	db $1E
	db $05
	db $70
	db $02
	db $48
	db $10
	db $1E
	db $01
	db $28
	db $48
	db $1E
	db $05
	db $80
	db $02
	db $48
	db $10
	db $1E
	db $01
	db $30
	db $48
	db $1E
	db $05
	db $90
	db $03
	db $48
	db $14
	db $1E
	db $01
	db $30
	db $48
	db $1E
	db $06
	db $A0
	db $03
	db $48
	db $14
	db $1E
	db $02
	db $30
	db $40
	db $1E
	db $06
	db $B0
	db $03
	db $40
	db $18
	db $1E
	db $02
	db $38
	db $40
	db $1E
	db $06
	db $C0
	db $03
	db $40
	db $18
	db $1E
	db $02
	db $38
	db $40
	db $1E
	db $06
	db $D0
	db $03
	db $40
	db $18
	db $1E
	db $02
	db $38
	db $40
	db $1E
	db $06
	db $D0
	db $03
	db $40
	db $18
	db $1E
	db $02
	db $40
	db $38
	db $1E
	db $06
	db $E0
	db $03
	db $38
	db $18
	db $1E
	db $02
	db $48
	db $38
	db $1E
	db $06
	db $E0
	db $03
	db $38
	db $18
	db $1E
	db $02
	db $50
	db $38
	db $1E
	db $06
	db $F0
	db $03
	db $38
	db $18
	db $1E
	db $03
	db $58
	db $30
	db $19
	db $07
	db $F0
	db $03
	db $30
	db $18
	db $19
	db $01
	db $20
	db $50
	db $1E
	db $04
	db $60
	db $01
	db $50
	db $00
	db $1E
	db $01
	db $20
	db $50
	db $1E
	db $04
	db $70
	db $01
	db $50
	db $00
	db $1E
	db $01
	db $28
	db $50
	db $1E
	db $04
	db $80
	db $01
	db $50
	db $00
	db $1E
	db $01
	db $28
	db $50
	db $1E
	db $05
	db $90
	db $02
	db $50
	db $00
	db $1E
	db $01
	db $30
	db $48
	db $1E
	db $05
	db $A0
	db $02
	db $48
	db $00
	db $1E
	db $01
	db $30
	db $48
	db $1E
	db $05
	db $B0
	db $02
	db $48
	db $00
	db $1E
	db $01
	db $38
	db $48
	db $1E
	db $05
	db $C0
	db $03
	db $48
	db $00
	db $1E
	db $01
	db $38
	db $48
	db $1E
	db $06
	db $D0
	db $03
	db $48
	db $00
	db $1E
	db $01
	db $40
	db $40
	db $1E
	db $06
	db $E0
	db $03
	db $40
	db $00
	db $1E
	db $01
	db $40
	db $40
	db $1E
	db $06
	db $F0
	db $03
	db $40
	db $00
	db $1E
	db $01
	db $48
	db $40
	db $1E
	db $06
	db $F0
	db $03
	db $40
	db $00
	db $1E
	db $01
	db $48
	db $40
	db $1E
	db $06
	db $F0
	db $03
	db $40
	db $00
	db $1E
	db $01
	db $50
	db $38
	db $1E
	db $06
	db $F0
	db $03
	db $38
	db $00
	db $1E
	db $01
	db $50
	db $38
	db $1E
	db $06
	db $F0
	db $03
	db $38
	db $00
	db $1E
	db $01
	db $58
	db $38
	db $1E
	db $06
	db $F0
	db $03
	db $38
	db $00
	db $1E
	db $01
	db $58
	db $30
	db $19
	db $06
	db $F0
	db $03
	db $30
	db $00
	db $19
	db $01
	db $20
	db $50
	db $5A
	db $03
	db $00
	db $01
	db $58
	db $3C
	db $64
	db $01
	db $20
	db $50
	db $5A
	db $03
	db $10
	db $01
	db $54
	db $46
	db $5A
	db $01
	db $28
	db $50
	db $50
	db $04
	db $20
	db $01
	db $52
	db $50
	db $50
	db $01
	db $28
	db $50
	db $46
	db $04
	db $30
	db $02
	db $50
	db $5A
	db $46
	db $01
	db $30
	db $48
	db $46
	db $04
	db $40
	db $02
	db $4E
	db $64
	db $46
	db $01
	db $30
	db $48
	db $3C
	db $05
	db $50
	db $02
	db $4B
	db $6E
	db $3C
	db $01
	db $38
	db $48
	db $3C
	db $05
	db $60
	db $03
	db $48
	db $78
	db $3C
	db $01
	db $38
	db $40
	db $32
	db $05
	db $70
	db $03
	db $46
	db $82
	db $3C
	db $01
	db $40
	db $40
	db $32
	db $05
	db $80
	db $03
	db $44
	db $8C
	db $32
	db $01
	db $40
	db $40
	db $28
	db $05
	db $90
	db $03
	db $44
	db $96
	db $32
	db $01
	db $48
	db $40
	db $28
	db $05
	db $A0
	db $03
	db $42
	db $A0
	db $32
	db $01
	db $48
	db $3C
	db $1E
	db $05
	db $B0
	db $03
	db $42
	db $AA
	db $28
	db $01
	db $50
	db $3C
	db $1E
	db $05
	db $C0
	db $03
	db $40
	db $B4
	db $28
	db $01
	db $50
	db $3C
	db $1E
	db $05
	db $D0
	db $03
	db $3C
	db $BE
	db $28
	db $01
	db $58
	db $38
	db $1E
	db $05
	db $E0
	db $03
	db $38
	db $C8
	db $1E
	db $01
	db $58
	db $30
	db $19
	db $05
	db $F0
	db $03
	db $34
	db $D2
	db $19
; Function Name: function_1ED1
function_1ED1:
	LD   A,(data_A987)
	AND  A
	LD   HL,Input1
	JR   NZ,.label_1EDD
	db $21
	db $B0
	db $A9
.label_1EDD:
	LD   A,(HL)
	RET 
; Function Name: function_1EDF
function_1EDF:
	LD   IX,data_A800
	LD   IY,PlayerSprite_X
	LD   A,(data_A800)
	AND  A
	RET  Z
	INC  A
	JP   NZ,function_200C.label_2010
	LD   A,(data_AD30)
	AND  A
	JP   Z,function_210E.label_214B
	CALL function_1ED1
	AND  $0F
	JR   NZ,.label_1F01
	db $C3
	db $42
	db $1F
.label_1F01:
	LD   HL,.label_1F1D+17
	RST  $08
	LD   B,A
	LD   A,(PlayerAngle)
	SUB  B
	JP   Z,.label_1F42
	LD   C,A
	LD   A,(data_AD04)
	AND  $0F
	CP   $03
	JR   NC,.label_1F1B
	LD   D,$03
	JR   .label_1F1D
.label_1F1B:
	db $16
	db $04
.label_1F1D:
	LD   A,C
	ADD  A,$01
	CP   $03
	JP   C,.label_1F3E
	LD   A,C
	CP   $80
	JP   NC,.label_1F6F
	db $C3
	db $68
	db $1F
	db $00
	db $00
	db $80
	db $00
	db $C0
	db $E0
	db $A0
	db $00
	db $40
	db $20
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
.label_1F3E:
	LD   A,B
	LD   (PlayerAngle),A
.label_1F42:
	LD   HL,.label_1F42+19
	PUSH HL
	LD   A,(data_AD04)
	AND  A
	JP   Z,function_5942.label_594E
	CP   $03
	JP   C,OutputString_AD1970.label_5965
	JP   OutputString_AD1970.label_596B
	XOR  A
	LD   H,A
	LD   L,A
	SBC  HL,DE
	LD   (data_A808),HL
	XOR  A
	LD   H,A
	LD   L,A
	SBC  HL,BC
	LD   (data_A80A),HL
	JP   UpdatePlayerSprite
	db $92
	db $80
	db $32
	db $02
	db $A8
	db $18
	db $D3
.label_1F6F:
	ADD  A,D
	ADD  A,B
	LD   (PlayerAngle),A
	JR   .label_1F42
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $DD
	db $F1
	db $F1
	db $F1
	db $F1
	db $F0
	db $F1
	db $F1
	db $F1
	db $F1
	db $C3
	db $F1
	db $F1
	db $F1
	db $F1
	db $EA
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $B7
	db $F1
	db $F1
	db $F1
	db $F1
	db $4D
	db $F1
	db $F1
	db $F1
	db $E5
	db $2D
	db $6E
	db $F1
	db $F1
	db $5E
	db $61
	db $E6
	db $F1
	db $F1
	db $F1
	db $B2
	db $F1
	db $F1
	db $F1
	db $F1
	db $53
	db $F1
	db $F1
	db $F1
	db $F1
	db $95
	db $F1
	db $F1
	db $F1
	db $45
	db $CA
	db $F1
	db $F1
	db $F1
	db $C6
	db $2C
	db $97
	db $F1
	db $F1
	db $81
	db $69
	db $1E
	db $F1
	db $F1
	db $BC
	db $A1
	db $60
	db $F1
	db $F1
	db $F4
	db $EB
	db $F1
	db $F1
	db $F1
	db $F1
	db $48
	db $F1
	db $F1
	db $F1
	db $E0
	db $63
	db $35
	db $F1
	db $F1
	db $AA
	db $B4
	db $8A
	db $F1
	db $F1
	db $51
	db $E9
	db $F6
	db $F1
	db $F1
	db $82
	db $92
	db $98
	db $F1
	db $F1
	db $F1
	db $46
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
	db $F1
; Function Name: function_200C
function_200C:
	ADD  HL,DE
	RST  $18
	LD   A,B
	RET 
.label_2010:
	LD   A,(IX+$00)
	CP   $B4
	JR   C,.label_2040
	LD   (IX+$00),$B4
	LD   (IY+$01),$FF
	LD   A,(data_AD04)
	CP   $02
	CALL NC,function_5679
	CALL function_56D2
	LD   A,(data_ABFE)
	CP   $A5
	JP   NZ,.label_2063
	LD   DE,data_ABFE+1
	LD   A,(DE)
	CP   $05
	JP   Z,.label_2040
	CP   $10
	JP   NZ,.label_2063
.label_2040:
	DEC  (IX+$00)
	LD   A,(IX+$00)
	CP   $B3
	JR   Z,.label_2066
	CP   $AB
	JR   Z,.label_206B
	CP   $A3
	JR   Z,.label_2070
	CP   $9B
	JR   Z,.label_2075
	CP   $93
	JR   Z,.label_207A
	CP   $8B
	JR   Z,.label_207F
	CP   $83
	JR   Z,.label_2084
	RET 
.label_2063:
	db $C3
	db $2E
	db $1F
.label_2066:
	LD   DE,function_1EDF.label_1F6F+7
	JR   .label_2089
.label_206B:
	LD   DE,function_1EDF.label_1F6F+37
	JR   .label_2089
.label_2070:
	LD   DE,function_1EDF.label_1F6F+67
	JR   .label_2089
.label_2075:
	LD   DE,function_1EDF.label_1F6F+97
	JR   .label_2089
.label_207A:
	LD   DE,function_1EDF.label_1F6F+97
	JR   .label_2089
.label_207F:
	LD   DE,function_1EDF.label_1F6F+67
	JR   .label_2089
.label_2084:
	LD   DE,function_1EDF.label_1F6F+127
	JR   .label_2089
.label_2089:
	LD   HL,VideoRAM+431
	LD   B,$C1
	LD   A,(data_AD04)
	ADD  A,B
	LD   C,A
	EXX 
	LD   A,(Phase1Function.operand_3379+1)
	LD   B,A
.label_2098:
	EXX 
	LD   A,(data_4902)
	LD   B,A
.label_209D:
	LD   A,(DE)
	LD   (HL),A
	RES  2,H
	LD   (HL),C
	SET  2,H
	INC  HL
	INC  DE
	DJNZ .label_209D
	LD   A,$1B
	RST  $18
	EXX 
	DJNZ .label_2098
	RET 
; Function Name: UpdatePlayerSprite
UpdatePlayerSprite:
	LD   IX,data_A800
	LD   DE,$0020
	LD   A,(PlayerAngle)
	ADD  A,$04			; why?
	RRCA
	RRCA
	RRCA
	AND  $1F			; mask bottom 5 bits - 32 sprites
	LD   HL,PlayerSpriteLUT
	RST  $18
	LD   A,(HL)
	LD   (PlayerSprite_No),A
	ADD  HL,DE
	LD   A,(HL)
	LD   (PlayerSprite_Attr),A
	RET 
PlayerSpriteLUT:
	db $F0
	db $F1
	db $F2
	db $F3
	db $F4
	db $F5
	db $F6
	db $F7
	db $E8
	db $F7
	db $F6
	db $F5
	db $F4
	db $F3
	db $F2
	db $F1
	db $F0
	db $EF
	db $EE
	db $ED
	db $EC
	db $EB
	db $EA
	db $E9
	db $E8
	db $E9
	db $EA
	db $EB
	db $EC
	db $ED
	db $EE
	db $EF
PlayerSpriteAttrLUT:
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
	db $80
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
; Function Name: function_210E
function_210E:
	LD   HL,data_ADF2+1
	EX   DE,HL
	LD   A,(data_AD14)
	AND  A
	JR   Z,.label_2140
	CP   $03
	JR   Z,.label_2140
	CP   $01
	JR   Z,.label_2145
	LD   HL,.label_2181+377
.label_2123:
	LD   A,(HL)
	INC  A
	LD   (data_ADF2),A
	EX   DE,HL
	LD   (HL),E
	INC  L
	LD   (HL),D
	LD   HL,CopyRightStringChar
	LD   A,(HL)
	CP   $FD
	JP   NZ,.label_213D
	INC  HL
	LD   A,(HL)
	CP   $10
	RET  Z
	CP   $05
	RET  Z
.label_213D:
	db $C3
	db $51
	db $22
.label_2140:
	LD   HL,.label_2181+11
	JR   .label_2123
.label_2145:
	LD   HL,.label_2181+208
	JR   .label_2123
	db $C9
.label_214B:
	LD   HL,data_ADF2
	LD   A,(HL)
	LD   B,A
	AND  $3F
	JR   Z,.label_215B
	DEC  A
	JR   Z,.label_215B
	DEC  B
	LD   (HL),B
	JR   .label_216A
.label_215B:
	INC  HL
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  DE
	LD   (HL),D
	DEC  HL
	LD   (HL),E
	EX   DE,HL
	LD   A,(HL)
	DEC  DE
	INC  A
	LD   (DE),A
	JR   .label_214B
.label_216A:
	LD   A,B
	EXX 
	RLCA
	RLCA
	AND  $03
	JP   Z,function_1EDF.label_1F42
	DEC  A
	JR   Z,.label_2181
	LD   A,(PlayerAngle)
	ADD  A,$03
	LD   (PlayerAngle),A
	JP   function_1EDF.label_1F42
.label_2181:
	LD   A,(PlayerAngle)
	SUB  $03
	LD   (PlayerAngle),A
	JP   function_1EDF.label_1F42
	db $3C
	db $3C
	db $3C
	db $3C
	db $0B
	db $95
	db $03
	db $66
	db $95
	db $7C
	db $59
	db $8D
	db $4B
	db $8E
	db $4A
	db $02
	db $8B
	db $1A
	db $55
	db $0E
	db $8A
	db $7C
	db $4E
	db $05
	db $8A
	db $0B
	db $86
	db $46
	db $03
	db $4A
	db $0D
	db $7C
	db $5A
	db $36
	db $AB
	db $08
	db $55
	db $08
	db $56
	db $01
	db $4A
	db $05
	db $56
	db $03
	db $7C
	db $4D
	db $BC
	db $83
	db $0A
	db $4B
	db $07
	db $BC
	db $81
	db $72
	db $02
	db $56
	db $02
	db $6A
	db $01
	db $95
	db $3B
	db $88
	db $53
	db $03
	db $BC
	db $95
	db $46
	db $0B
	db $95
	db $04
	db $A0
	db $0C
	db $4A
	db $02
	db $56
	db $03
	db $55
	db $01
	db $95
	db $03
	db $4A
	db $04
	db $8A
	db $02
	db $4A
	db $02
	db $8A
	db $29
	db $8B
	db $06
	db $4B
	db $16
	db $4A
	db $01
	db $95
	db $0D
	db $88
	db $53
	db $01
	db $6A
	db $0F
	db $8A
	db $08
	db $8B
	db $0D
	db $4B
	db $08
	db $8B
	db $07
	db $55
	db $02
	db $69
	db $89
	db $03
	db $4B
	db $01
	db $7C
	db $6F
	db $05
	db $8B
	db $4B
	db $0D
	db $8B
	db $01
	db $4E
	db $83
	db $01
	db $8B
	db $0F
	db $55
	db $05
	db $A2
	db $42
	db $10
	db $60
	db $26
	db $4B
	db $02
	db $8B
	db $08
	db $4B
	db $05
	db $8F
	db $4F
	db $01
	db $95
	db $17
	db $4A
	db $0E
	db $8A
	db $04
	db $A0
	db $1B
	db $8B
	db $11
	db $4B
	db $0A
	db $52
	db $97
	db $4D
	db $8F
	db $47
	db $06
	db $8B
	db $02
	db $55
	db $03
	db $9D
	db $67
	db $8A
	db $0A
	db $56
	db $05
	db $8B
	db $02
	db $48
	db $88
	db $03
	db $55
	db $09
	db $60
	db $03
	db $76
	db $13
	db $8B
	db $24
	db $4B
	db $2F
	db $8B
	db $05
	db $8B
	db $08
	db $8A
	db $15
	db $96
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $0A
	db $95
	db $60
	db $04
	db $9E
	db $53
	db $0D
	db $8B
	db $02
	db $4B
	db $0F
	db $93
	db $53
	db $07
	db $A9
	db $54
	db $0A
	db $96
	db $03
	db $60
	db $0F
	db $8A
	db $23
	db $48
	db $B9
	db $02
	db $82
	db $59
	db $9F
	db $59
	db $01
	db $8B
	db $22
	db $AB
	db $02
	db $4B
	db $02
	db $8B
	db $07
	db $55
	db $AC
	db $42
	db $01
	db $50
	db $90
	db $02
	db $55
	db $35
	db $90
	db $50
	db $04
	db $92
	db $5B
	db $89
	db $1F
	db $48
	db $88
	db $05
	db $8C
	db $42
	db $05
	db $4A
	db $3C
	db $0C
	db $46
	db $86
	db $3C
	db $04
	db $93
	db $5E
	db $06
	db $4B
	db $09
	db $4A
	db $0A
	db $7C
	db $7C
	db $6F
	db $BC
	db $01
	db $8B
	db $07
	db $92
	db $48
	db $07
	db $88
	db $7C
	db $7C
	db $45
	db $11
	db $90
	db $50
	db $01
	db $8B
	db $07
	db $4B
	db $0C
	db $8B
	db $0A
	db $76
	db $AB
	db $12
	db $87
	db $47
	db $18
	db $8B
	db $03
	db $8A
	db $02
	db $96
	db $08
	db $4B
	db $02
	db $8B
	db $07
	db $95
	db $3C
	db $3C
	db $17
	db $55
	db $3C
	db $05
	db $56
	db $20
	db $7C
	db $44
	db $06
	db $67
	db $BC
	db $4D
	db $8E
	db $0C
	db $56
	db $02
	db $4A
	db $1A
	db $4B
	db $39
	db $55
	db $25
	db $56
	db $20
	db $55
	db $0B
	db $4B
	db $03
	db $60
	db $06
	db $4A
	db $03
	db $41
	db $01
	db $BC
	db $9F
	db $50
	db $04
	db $96
	db $0F
	db $4B
	db $07
	db $8B
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $02
	db $90
	db $45
	db $02
	db $4B
	db $02
	db $48
	db $88
	db $07
	db $8A
	db $55
	db $01
	db $4A
	db $01
	db $58
	db $82
	db $03
	db $8A
	db $5F
	db $01
	db $60
	db $07
	db $B2
	db $52
	db $03
	db $46
	db $86
	db $1E
	db $49
	db $89
	db $08
	db $4B
	db $01
	db $94
	db $49
	db $05
	db $8A
	db $4A
	db $3C
	db $3C
	db $0A
	db $BC
	db $84
	db $11
	db $53
	db $88
	db $01
	db $4A
	db $0B
	db $6B
	db $06
	db $4B
	db $24
	db $4A
	db $11
	db $56
	db $08
	db $4A
	db $0E
	db $4B
	db $07
	db $55
	db $07
	db $4B
	db $07
	db $7C
	db $72
	db $8E
	db $01
	db $AF
	db $44
	db $02
	db $56
	db $8B
	db $04
	db $5A
	db $85
	db $02
	db $8A
	db $02
	db $90
	db $45
	db $09
	db $8B
	db $01
	db $48
	db $89
	db $41
	db $02
	db $4B
	db $05
	db $B5
	db $10
	db $4D
	db $83
	db $03
	db $B5
	db $4B
	db $03
	db $A0
	db $07
	db $72
	db $88
	db $08
	db $4B
	db $01
	db $50
	db $85
	db $03
	db $8B
	db $02
	db $55
	db $05
	db $95
	db $06
	db $60
	db $06
	db $55
	db $01
	db $4B
	db $09
	db $48
	db $8F
	db $47
	db $03
	db $4B
	db $01
	db $96
	db $07
	db $8A
	db $05
	db $6A
	db $18
	db $4B
	db $0A
	db $8B
	db $06
	db $8A
	db $02
	db $44
	db $84
	db $06
	db $8B
	db $08
	db $8B
	db $14
	db $BC
	db $84
	db $03
	db $59
	db $83
	db $02
	db $8B
	db $03
	db $60
	db $08
	db $8B
	db $05
	db $7C
	db $5A
	db $01
	db $B6
	db $0A
	db $48
	db $95
	db $4D
	db $01
	db $8A
	db $09
	db $51
	db $BC
	db $85
	db $65
	db $2D
	db $6B
	db $01
	db $95
	db $4D
	db $83
	db $02
	db $8A
	db $4A
	db $01
	db $8B
	db $02
	db $72
	db $85
	db $53
	db $01
	db $95
	db $02
	db $8B
	db $06
	db $95
	db $03
	db $8B
	db $01
	db $8A
	db $01
	db $4A
	db $07
	db $95
	db $01
	db $6B
	db $03
	db $97
	db $41
	db $05
	db $4B
	db $0B
	db $48
	db $88
	db $05
	db $60
	db $3C
	db $3C
	db $3C
	db $3C
OutputString_AD2001:
	dw $A673
	db $14
	db $7E
	db $29
	db $F8
	db $9B
	db $13
	db $13
	db $96
	db $B9
; Function Name: function_23E3
function_23E3:
	LD   A,(data_A800)
	INC  A
	JP   NZ,OutputString_FirstBonus10000.label_2496
	LD   A,(data_ACC6)
	AND  A
	JP   NZ,OutputString_FirstBonus10000.label_2496
	CALL function_1ED1
	RLCA
	RLCA
	RLCA
	RLCA
	LD   HL,data_A98D+1
	RL   (HL)
	LD   A,(HL)
	AND  $03
	CP   $01
	LD   HL,data_AA81
	JR   NZ,.label_2409
	LD   (HL),$03
.label_2409:
	LD   A,(data_AD30)
	AND  A
	JR   Z,.label_2414
	LD   A,(HL)
	AND  A
	JP   Z,OutputString_FirstBonus10000.label_2496
.label_2414:
	INC  HL
	LD   A,(HL)
	AND  A
	JP   NZ,OutputString_FirstBonus10000.label_2496
	LD   IX,data_AA80
	LD   B,$06
.label_2420:
	LD   A,(IX+$00)
	AND  A
	JR   Z,OutputString_FirstBonus10000.label_2449
	LD   DE,(data_0D46)
	ADD  IX,DE
	DJNZ .label_2420
	JP   OutputString_FirstBonus10000.label_2496
OutputString_FirstBonus10000:
	dw $A716
	db $13
	db $96
	db $ED
	db $DC
	db $F1
	db $8C
	db $68
	db $3B
	db $0D
	db $ED
	db $F1
	db $96
	db $13
	db $13
	db $13
	db $13
	db $F1
	db $88
	db $DC
	db $ED
	db $11
	db $B9
.label_2449:
	CALL function_567E
	XOR  A
	LD   H,A
	LD   L,A
	LD   BC,(data_A808)
	SBC  HL,BC
	ADD  HL,HL
	ADD  HL,HL
	LD   (IX+$0A),L
	LD   (IX+$0B),H
	XOR  A
	LD   H,A
	LD   L,A
	LD   BC,(data_A80A)
	SBC  HL,BC
	ADD  HL,HL
	ADD  HL,HL
	LD   (IX+$0C),L
	LD   (IX+$0D),H
	LD   A,(PlayerAngle)
	ADD  A,$04
	RRCA
	RRCA
	RRCA
	AND  $1F
	LD   HL,function_2755.operand_2769+8
	CALL GetTablePtr
	DEC  (IX+$00)
	LD   (IX+$03),$00
	LD   (IX+$04),E
	LD   (IX+$05),$00
	LD   (IX+$06),D
	LD   HL,data_AA81
	DEC  (HL)
	INC  HL
	LD   (HL),$06
.label_2496:
	LD   A,(data_AA82)
	AND  A
	JR   Z,.label_24A0
	DEC  A
	LD   (data_AA82),A
.label_24A0:
	LD   IX,data_AA80
	LD   B,$06
.label_24A6:
	EXX 
	LD   A,(IX+$00)
	AND  A
	JR   Z,.label_24F3
	INC  A
	JR   NZ,.label_24FC
	LD   L,(IX+$0A)
	LD   H,(IX+$0B)
	LD   DE,(data_A808)
	ADD  HL,DE
	LD   D,(IX+$04)
	LD   E,(IX+$03)
	ADD  HL,DE
	LD   A,H
	ADD  A,$10
	CP   $10
	JP   C,.label_24FC
	LD   (IX+$04),H
	LD   (IX+$03),L
	LD   L,(IX+$0C)
	LD   H,(IX+$0D)
	LD   DE,(data_A80A)
	ADD  HL,DE
	LD   D,(IX+$06)
	LD   E,(IX+$05)
	ADD  HL,DE
	LD   A,H
	ADD  A,$08
	CP   $18
	JP   C,.label_24FC
	LD   (IX+$06),H
	LD   (IX+$05),L
	CALL function_5337
.label_24F3:
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	EXX 
	DJNZ .label_24A6
	RET 
.label_24FC:
	XOR  A
	LD   (IX+$00),A
	LD   (IX+$04),A
	LD   (IX+$06),A
	JP   .label_24F3
OutputString_2UP:
	dw $A4E0
	db $14
	db $9B
	db $10
	db $0D
	db $88
	db $B9
; Function Name: InitProgram_2511
InitProgram_2511:
	LD   HL,CommandQueue
	LD   B,64			; set 64 bytes
.byte_set_loop:
	LD   (HL),$FF
	INC  HL
	DJNZ .byte_set_loop
	CALL function_4B67
	LD   (WatchdogReset_DipSW2),A
	CALL function_4BA5
	LD   (WatchdogReset_DipSW2),A
	CALL ResetBulletDrawLists
	LD   (WatchdogReset_DipSW2),A
	JP   label_52AA
.label_2530:
	db $19
	db $01
	db $18
	db $01
	db $17
	db $01
	db $16
	db $01
	db $15
	db $01
	db $14
	db $01
	db $13
	db $01
	db $10
	db $01
	db $0E
	db $01
	db $0C
	db $01
	db $0A
	db $01
	db $08
	db $01
	db $04
	db $01
	db $01
	db $01
	db $FF
	db $00
	db $FB
	db $00
	db $F8
	db $00
	db $F5
	db $00
	db $F2
	db $00
	db $EE
	db $00
	db $EB
	db $00
	db $E8
	db $00
	db $E4
	db $00
	db $E1
	db $00
	db $DE
	db $00
	db $DA
	db $00
	db $D7
	db $00
	db $D4
	db $00
	db $D1
	db $00
	db $CD
	db $00
	db $CA
	db $00
	db $C7
	db $00
	db $C3
	db $00
	db $C0
	db $00
	db $BC
	db $00
	db $B8
	db $00
	db $B5
	db $00
	db $B1
	db $00
	db $AC
	db $00
	db $A8
	db $00
	db $A5
ColourRAMPtr:
	dw ColourRAM
	db $00
	db $9A
	db $00
	db $94
	db $00
	db $8F
	db $00
	db $87
	db $00
	db $84
	db $00
	db $7D
	db $00
	db $76
	db $00
	db $70
	db $00
	db $69
	db $00
	db $61
	db $00
	db $5B
	db $00
	db $53
	db $00
	db $4B
	db $00
	db $44
	db $00
	db $3B
	db $00
	db $33
	db $00
	db $2C
	db $00
	db $23
	db $00
	db $1A
	db $00
	db $11
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $FF
	db $EF
	db $FF
	db $00
	db $00
	db $DD
	db $FF
	db $D4
	db $FF
	db $CD
	db $FF
	db $C5
	db $FF
	db $BC
	db $FF
	db $B5
	db $FF
	db $AD
	db $FF
	db $A5
	db $FF
	db $9F
	db $FF
	db $97
	db $FF
	db $90
	db $FF
	db $8A
	db $FF
	db $83
	db $FF
	db $7C
	db $FF
	db $79
	db $FF
	db $7C
	db $FF
	db $6C
	db $FF
	db $66
	db $FF
	db $60
	db $FF
	db $5B
	db $FF
	db $58
	db $FF
	db $54
	db $FF
	db $4F
	db $FF
	db $4B
	db $FF
	db $48
	db $FF
	db $44
	db $FF
	db $40
	db $FF
	db $3D
	db $FF
	db $39
	db $FF
	db $36
	db $FF
	db $33
	db $FF
	db $33
	db $FF
	db $2C
	db $FF
	db $29
	db $FF
	db $26
	db $FF
	db $22
	db $FF
	db $1F
	db $FF
	db $1C
	db $FF
	db $18
	db $FF
	db $15
	db $FF
	db $12
	db $FF
	db $0E
	db $FF
	db $0B
	db $FF
	db $08
	db $FF
	db $05
	db $FF
	db $01
	db $FF
	db $FF
	db $FE
	db $FC
	db $FE
	db $F8
	db $FE
	db $F6
	db $FE
	db $F4
	db $FE
	db $F2
	db $FE
	db $F0
	db $FE
	db $ED
	db $FE
	db $EC
	db $FE
	db $EB
	db $FE
	db $EA
	db $FE
	db $E9
	db $FE
	db $E8
	db $FE
	db $E7
	db $FE
	db $E7
	db $FE
	db $E8
	db $FE
	db $E9
	db $FE
	db $EA
	db $FE
	db $EB
	db $FE
	db $EC
	db $FE
	db $ED
	db $FE
	db $F0
	db $FE
	db $F2
	db $FE
	db $F4
	db $FE
	db $F6
	db $FE
	db $F8
	db $FE
	db $FC
	db $FE
	db $FF
	db $FE
	db $01
	db $FF
	db $05
	db $FF
	db $08
	db $FF
	db $0B
	db $FF
	db $0E
	db $FF
	db $12
	db $FF
	db $15
	db $FF
	db $18
	db $FF
	db $1C
	db $FF
	db $1F
	db $FF
	db $22
	db $FF
	db $26
	db $FF
	db $29
	db $FF
	db $2C
	db $FF
	db $2F
	db $FF
	db $33
	db $FF
	db $36
	db $FF
	db $39
	db $FF
	db $3D
	db $FF
	db $40
	db $FF
	db $44
	db $FF
	db $48
	db $FF
	db $4B
	db $FF
	db $4F
	db $FF
	db $54
	db $FF
	db $58
	db $FF
	db $5B
	db $FF
	db $60
	db $FF
	db $66
	db $FF
	db $6C
	db $FF
	db $71
	db $FF
	db $79
	db $FF
	db $7C
	db $FF
	db $83
	db $FF
	db $8A
	db $FF
	db $90
	db $FF
	db $97
	db $FF
	db $9F
	db $FF
	db $A5
	db $FF
	db $AD
	db $FF
	db $B5
	db $FF
	db $BC
	db $FF
	db $C5
	db $FF
	db $CD
	db $FF
	db $D4
	db $FF
	db $DD
	db $FF
	db $E6
	db $FF
	db $EF
	db $FF
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $11
	db $00
	db $1A
	db $00
	db $23
	db $00
	db $2C
	db $00
	db $33
	db $00
	db $3B
	db $00
	db $44
	db $00
	db $4B
	db $00
	db $53
	db $00
	db $5B
	db $00
	db $61
	db $00
	db $69
	db $00
	db $70
	db $00
	db $76
	db $00
	db $7D
	db $00
	db $84
	db $00
	db $87
	db $00
	db $87
	db $00
	db $94
	db $00
	db $9A
	db $00
	db $A0
	db $00
	db $A5
	db $00
	db $A8
	db $00
	db $AC
	db $00
	db $B1
	db $00
	db $B5
	db $00
	db $B8
	db $00
	db $BC
	db $00
	db $C0
	db $00
	db $C3
	db $00
	db $C7
	db $00
	db $CA
	db $00
	db $CD
	db $00
	db $CA
	db $00
	db $D4
	db $00
	db $D7
	db $00
	db $DA
	db $00
	db $DE
	db $00
	db $E1
	db $00
	db $E4
	db $00
	db $E8
	db $00
	db $EB
	db $00
	db $EE
	db $00
	db $F2
	db $00
	db $F5
	db $00
	db $F8
	db $00
	db $FB
	db $00
	db $FF
	db $00
	db $01
	db $01
	db $FB
	db $00
	db $08
	db $01
	db $0A
	db $01
	db $0C
	db $01
	db $0E
	db $01
	db $10
	db $01
	db $13
	db $01
	db $14
	db $01
	db $15
	db $01
	db $16
	db $01
	db $17
	db $01
	db $18
	db $01
	db $19
	db $01
	LD   A,(data_AA6F)
	CP   $76
	JP   NZ,InitProgram_2511.label_2530
.operand_2738:
	CALL function_0B2B
	CALL function_210E
	XOR  A
	LD   (data_AD31),A
	LD   (data_AD20),A
	LD   (data_AD30),A
	LD   (ProgramPhase),A
	INC  A
	LD   (data_AD10),A
	LD   A,$03
	LD   (FunctionChecksum),A
	RET 
; Function Name: function_2755
function_2755:
	LD   IX,data_AA80
	LD   HL,.operand_2769+5
	LD   A,(QueueTitleScreenDraw.operand_0861)
	LD   E,A
	LD   A,(data_5C01)
	LD   D,A
	LD   B,$06
.label_2766:
	LD   (IX+$00),A
.operand_2769:
	LD   (IX+$04),A
	ADD  IX,DE
	DJNZ .label_2766
	RET 
	db $7E
	db $84
	db $7E
	db $85
	db $7E
	db $86
	db $7D
	db $87
	db $7C
	db $88
	db $7B
	db $89
	db $7A
	db $8A
	db $79
	db $8A
	db $78
	db $8A
	db $77
	db $8A
	db $76
	db $8A
	db $75
	db $89
	db $74
	db $88
	db $73
	db $87
	db $72
	db $86
	db $72
	db $85
	db $72
	db $84
	db $72
	db $83
	db $72
	db $82
	db $73
	db $81
	db $74
	db $80
	db $75
	db $7F
	db $76
	db $7E
	db $77
	db $7E
	db $78
	db $7E
	db $79
	db $7E
	db $7A
	db $7E
	db $7B
	db $7F
	db $7C
	db $80
	db $7D
	db $81
	db $7E
	db $82
	db $7E
	db $83
; Function Name: Phase0Function
; Description: This appears to reset a lot of things
Phase0Function:
	CALL function_5834
	LD   A,$78
	LD   (data_AC64),A
	LD   A,$84
	LD   (data_AC65),A
.operand_27BE:
	LD   HL,$0000
	LD   (data_AD16),HL
	LD   (data_AD26),HL
	LD   A,(data_A9CD)
.operand_27CA:
	LD   (data_AD12),A
	LD   (data_AD22),A
	XOR  A
	LD   (data_AD14),A
	LD   (data_AD24),A
	LD   (data_AD32),A
	LD   (data_AD13),A
	LD   (data_AD23),A
	LD   (data_AD1D),A
	LD   (data_AD2D),A
	LD   (CharacterAttrib),A
	INC  A
	LD   (data_AD11),A
	LD   (data_AD21),A
	LD   (data_AD1E),A
	LD   (data_AD2E),A
	LD   A,(data_AD30)
	AND  A
	JR   Z,.label_2835
	XOR  A
	LD   H,A
	LD   L,A
	LD   (data_AD33),A
	LD   (data_AD34),HL
	LD   (data_AD36),A
	LD   (data_AD37),HL
.operand_280B:
	LD   DE,CopySpriteDataToHW+155
	RST  $38
	LD   A,(data_A9C4)
	CALL function_0F7B
	LD   B,$00
	LD   HL,data_1550
	SUB  A
.label_281B:
	XOR  (HL)
	INC  HL
	DJNZ .label_281B
	ADD  A,$01
	LD   (HWReg_VideoEnable),A
	LD   A,(data_A9D3)
	LD   (data_AD1A),A
	LD   (data_AD2A),A
	LD   A,$96
	LD   (data_A9EB),A
	JP   AdvancePhase
.label_2835:
	LD   HL,data_A9D0
	LD   A,(HL)
	INC  A
	CP   $04
	JR   C,.label_2840
	LD   A,$01
.label_2840:
	LD   (HL),A
	LD   (data_AD14),A
	INC  A
	LD   (data_AD11),A
	XOR  A
	LD   (FrameCounter),A
	LD   (data_A9CE),A
	LD   (data_A9CF),A
	CALL function_4B67
	LD   HL,data_AA80
	LD   DE,data_AA81
	LD   (HL),$00
	LD   BC,OutputString_ScoreRankingTable+16
	LDIR
	LD   HL,data_A800
	LD   DE,data_A801
	LD   (HL),$00
	LD   BC,label_0174+11
	LDIR
	LD   A,$02
	CALL function_0F7B
	LD   A,(data_A9D3)
	LD   (data_AD1A),A
	LD   (data_AD2A),A
	LD   C,$00
	LD   HL,Phase8Function+5
	LD   A,(FunctionChecksum)
.label_2885:
	SUB  (HL)
	INC  HL
	DEC  C
	JR   NZ,.label_2885
	XOR  $90
	LD   (FunctionChecksum),A
	LD   HL,data_AC74
	LD   B,$10
.label_2894:
	LD   (HL),$80
	INC  HL
	DJNZ .label_2894
	LD   A,$5A
	LD   (data_A9EB),A
	JP   AdvancePhase
; Function Name: function_28A1
function_28A1:
	CALL function_28B7
	CALL function_28C2
	CALL function_28CD
	CALL function_28D8
	CALL function_28E3
	CALL function_28EE
	CALL function_28FE
	RET 
; Function Name: function_28B7
function_28B7:
	LD   IX,data_A844+12
	LD   IY,PlayerSprite_No+9
	JP   label_290E
; Function Name: function_28C2
function_28C2:
	LD   IX,data_A844+28
	LD   IY,PlayerSprite_No+11
	JP   label_290E
; Function Name: function_28CD
function_28CD:
	LD   IX,data_A844+44
	LD   IY,PlayerSprite_No+13
	JP   label_290E
; Function Name: function_28D8
function_28D8:
	LD   IX,data_A844+60
	LD   IY,PlayerSprite_No+15
	JP   label_290E
; Function Name: function_28E3
function_28E3:
	LD   IX,data_A844+76
	LD   IY,PlayerSprite_No+17
	JP   label_290E
; Function Name: function_28EE
function_28EE:
	LD   A,(data_AD0D)
	AND  A
	RET  NZ
	LD   IX,data_A8A0
	LD   IY,data_AA24
	JP   label_290E
; Function Name: function_28FE
function_28FE:
	LD   A,(data_AD0D)
	AND  A
	RET  NZ
	LD   IX,data_A8A0+16
	LD   IY,data_AA24+2
	JP   label_290E
; Function Name: label_290E
; Parameters:
; 	IY : SpriteXAddr
; 	IX : NewParam
label_290E:
	LD   A,(data_AD04)
	AND  $07
	RST  $30
	dw function_2927
	dw function_294C
	dw function_2984
	dw function_29B0
	dw function_29D5
; Function Name: function_291E
function_291E:
	ADD  A,(HL)
	EX   DE,HL
	LD   C,(HL)
	EX   DE,HL
	INC  HL
	INC  DE
	DJNZ function_291E
	RET 
; Function Name: function_2927
function_2927:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	INC  A
	JR   Z,.label_2936
	INC  A
	JP   Z,label_2B52
	JP   label_2B93
.label_2936:
	CALL function_2BEF
	CALL function_5840
	CALL function_2B83
	JP   C,function_2BDE
	CALL function_3ED6
	CALL function_2A3C
	CALL function_4243
	RET 
; Function Name: function_294C
function_294C:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	INC  A
	JR   Z,.label_295B
	INC  A
	JP   Z,label_2B52
	JP   label_2B93
.label_295B:
	CALL function_2BEF
	CALL function_5854
	CALL function_2B83
	JP   C,function_2BDE
	CALL function_3ED6
	CALL function_2A47
	RET 
OutputString_TimePilotLogo_Bottom:
	dw $A709
	db $32
	db $82
	db $6E
	db $58
	db $B5
	db $77
	db $E4
	db $E8
	db $EC
	db $9D
	db $CB
	db $4F
	db $55
	db $FE
	db $A3
	db $31
	db $81
	db $5B
	db $9A
	db $B9
; Function Name: function_2984
function_2984:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	INC  A
	JR   Z,.label_2993
	INC  A
	JP   Z,label_2B52
	JP   label_2B93
.label_2993:
	LD   A,(FrameCounter)
	AND  $03
	CP   $03
	CALL C,function_2BEF
	CALL function_5840
	CALL function_2B83
	JP   C,function_2BDE
	CALL function_3ED6
	CALL function_2A97
	CALL function_4243
	RET 
; Function Name: function_29B0
function_29B0:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	INC  A
	JR   Z,data_29BF
	INC  A
	JP   Z,label_2B52
	JP   label_2B93
; Function Name: data_29BF
data_29BF:
	CALL function_2BEF
	CALL function_58A4
	CALL function_2B83
	JP   C,function_2BDE
	CALL function_3ED6
	CALL function_2AFC
	CALL function_4243
	RET 
; Function Name: function_29D5
function_29D5:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	INC  A
	JR   Z,.label_29E4
	INC  A
	JP   Z,label_2B52
	JP   label_2B93
.label_29E4:
	db $CD
	db $F7
	db $29
	db $CD
	db $83
	db $2B
	db $DA
	db $DE
	db $2B
	db $CD
	db $38
	db $2B
	db $CD
	db $D6
	db $3E
	db $CD
	db $43
	db $42
	db $C9
	db $3E
	db $78
	db $FD
	db $96
	db $31
	db $C6
	db $48
	db $FE
	db $90
	db $38
	db $1A
	db $3E
	db $84
	db $FD
	db $96
	db $31
	db $C6
	db $48
	db $FE
	db $90
	db $38
	db $0F
	db $CD
	db $EF
	db $2B
	db $3A
	db $80
	db $A9
	db $0F
	db $E6
	db $01
	db $CA
	db $AA
	db $58
	db $C3
	db $60
	db $58
	db $AF
	db $32
	db $04
	db $AD
	db $CD
	db $EF
	db $2B
	db $3E
	db $04
	db $32
	db $04
	db $AD
	db $18
	db $E6
	db $DD
	db $7E
	db $04
	db $3D
	db $CA
	db $93
	db $2B
	db $DD
	db $77
	db $04
	db $DD
	db $36
	db $00
	db $FF
	db $CD
	db $BA
	db $2B
	db $C9
; Function Name: function_2A3C
function_2A3C:
	CALL function_2A57
	LD   (IY+$30),C
	LD   A,B
	LD   (IY+$01),A
	RET 
; Function Name: function_2A47
function_2A47:
	CALL function_2A57
	LD   A,C
	ADD  A,$35
	LD   (IY+$30),A
	LD   A,B
	ADD  A,$10
	LD   (IY+$01),A
	RET 
; Function Name: function_2A57
function_2A57:
	LD   DE,RST10_AddDoubleAToHL
	LD   A,(IX+$02)
	ADD  A,$08
	RRCA
	RRCA
	RRCA
	RRCA
	AND  $0F
	LD   HL,function_2A57+32
	RST  $18
	LD   B,(HL)
	ADD  HL,DE
	LD   C,(HL)
	LD   A,(FrameCounter)
	BIT  1,A
	RET  Z
	LD   A,B
	ADD  A,$08
	LD   B,A
	RET 
	db $0C
	db $0D
	db $0E
	db $0F
	db $08
	db $0F
	db $0E
	db $0D
	db $0C
	db $0B
	db $0A
	db $09
	db $08
	db $09
	db $0A
	db $0B
	db $41
	db $41
	db $41
	db $41
	db $81
	db $C1
	db $C1
	db $C1
	db $C1
	db $C1
	db $C1
	db $C1
	db $41
	db $41
	db $41
	db $41
; Function Name: function_2A97
function_2A97:
	LD   A,(IX+$02)
	ADD  A,$04
	AND  $F8
	RRCA
	RRCA
	AND  $3F
	LD   HL,.label_2AB8+4
	RST  $18
	LD   B,(HL)
	LD   A,(FrameCounter)
	AND  $02
	JR   NZ,.label_2AB8
.label_2AAE:
	ADD  A,B
	LD   (IY+$01),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$30),A
	RET 
.label_2AB8:
	LD   A,$08
	JR   .label_2AAE
	db $80
	db $DC
	db $80
	db $DC
	db $80
	db $DC
	db $80
	db $DC
	db $81
	db $DC
	db $81
	db $DC
	db $82
	db $DC
	db $83
	db $DC
	db $84
	db $5C
	db $84
	db $5C
	db $83
	db $5C
	db $82
	db $5C
	db $81
	db $5C
	db $81
	db $5C
	db $80
	db $5C
	db $80
	db $5C
	db $80
	db $5C
	db $80
	db $5C
	db $80
	db $5C
	db $80
	db $5C
	db $81
	db $5C
	db $81
	db $5C
	db $82
	db $5C
	db $83
	db $5C
	db $84
	db $DC
	db $84
	db $DC
	db $83
	db $DC
	db $82
	db $DC
	db $81
	db $DC
	db $81
	db $DC
	db $80
	db $DC
	db $80
	db $DC
; Function Name: function_2AFC
function_2AFC:
	LD   DE,RST10_AddDoubleAToHL
	LD   A,(IX+$02)
	ADD  A,$08
	RRCA
	RRCA
	RRCA
	RRCA
	AND  $0F
	LD   HL,function_2AFC+28
	RST  $18
	LD   A,(HL)
	LD   (IY+$01),A
	ADD  HL,DE
	LD   A,(HL)
	LD   (IY+$30),A
	RET 
	db $2C
	db $2D
	db $2E
	db $2F
	db $28
	db $2F
	db $2E
	db $2D
	db $2C
	db $2B
	db $2A
	db $29
	db $28
	db $29
	db $2A
	db $2B
	db $5B
	db $5B
	db $5B
	db $5B
	db $9B
	db $DB
	db $DB
	db $DB
	db $DB
	db $DB
	db $DB
	db $DB
	db $5B
	db $5B
	db $5B
	db $5B
	db $3A
	db $80
	db $A9
	db $0F
	db $0F
	db $E6
	db $03
	db $C6
	db $D8
	db $47
	db $DD
	db $7E
	db $04
	db $D6
	db $01
	db $87
	db $87
	db $80
	db $FD
	db $77
	db $01
	db $FD
	db $36
	db $30
	db $61
	db $C9
; Function Name: label_2B52
label_2B52:
	DEC  (IX+$0E)
	JR   Z,.label_2B58
	RET 
.label_2B58:
	INC  (IX+$00)
	LD   (IX+$0E),$80
	RET 
.label_2B60:
	LD   H,(IY+$31)
	LD   L,(IX+$03)
	LD   DE,(data_A808)
	ADD  HL,DE
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   H,(IY+$00)
	LD   L,(IX+$05)
	LD   DE,(data_A80A)
	ADD  HL,DE
	LD   (IY+$00),H
	LD   (IX+$05),L
	RET 
; Function Name: function_2B83
function_2B83:
	LD   A,(IY+$31)
	ADD  A,$09
	CP   $03
	RET  C
	LD   A,(IY+$00)
	SUB  $03
	CP   $03
	RET 
; Function Name: label_2B93
label_2B93:
	LD   A,(IX+$00)
	CP   $F0
	JP   Z,.label_2BAC
	CP   $3C
	CALL Z,function_2BBA
	JP   NC,.label_2BB4
	DEC  (IX+$00)
	JR   Z,function_2BDE
	CALL function_2C22
	RET 
.label_2BAC:
	LD   (IX+$00),$3B
	CALL function_2BBA
	RET 
.label_2BB4:
	DEC  (IX+$00)
	JP   function_5840
; Function Name: function_2BBA
function_2BBA:
	CALL function_5683
	LD   HL,RemainingShipsInWave
	LD   A,(HL)
	AND  A
	JR   Z,.label_2BC5
	DEC  (HL)
.label_2BC5:
	LD   A,(IX+$0E)
	BIT  7,A
	RET  Z
	LD   A,(FrameCountdown2)
	AND  A
	RET  Z
	LD   HL,data_A811
	DEC  (HL)
	RET  NZ
	LD   A,(IX+$0F)
	ADD  A,$80
	LD   (data_A821),A
	RET 
; Function Name: function_2BDE
function_2BDE:
	XOR  A
	LD   (IX+$00),A
	LD   (IX+$03),A
	LD   (IX+$05),A
	LD   (IY+$00),A
	LD   (IY+$31),A
	RET 
; Function Name: function_2BEF
function_2BEF:
	LD   A,(IX+$01)
	SUB  (IX+$02)
	LD   C,A
	ADD  A,$02
	CP   $04
	RET  C
	LD   B,(IX+$02)
	LD   A,C
.operand_2BFF:
	CP   $80
	JR   NC,.label_2C0F
	LD   HL,data_2C1D
	LD   A,(data_AD04)
	RST  $08
	ADD  A,B
	LD   (IX+$02),A
	RET 
.label_2C0F:
	LD   HL,data_2C1D
	LD   A,(data_AD04)
	RST  $08
	SUB  B
	NEG  
	LD   (IX+$02),A
	RET 
data_2C1D:
	db $01
	db $01
	db $02
	db $02
	db $05
; Function Name: function_2C22
function_2C22:
	LD   HL,label_2C31
	PUSH HL
	LD   A,(IX+$00)
	CP   $20
	JP   NC,label_2B93.label_2BB4
	JP   label_2B52.label_2B60
; Function Name: label_2C31
label_2C31:
	LD   A,(IX+$00)
	CP   $2A
	JP   NC,.label_2C71
	CP   $0A
	JR   NC,.label_2C82
	LD   A,(data_A821)
	BIT  7,A
	JP   Z,function_2BDE
	LD   A,(data_A821)
	RES  7,A
	CP   (IX+$0F)
	JP   NZ,function_2BDE
	LD   A,(FrameCounter)
	AND  $07
	JR   Z,.label_2C5A
	INC  (IX+$00)
.label_2C5A:
	LD   (IY+$01),$FC
	LD   (IY+$30),$6C
	LD   A,(IX+$00)
	CP   $01
	RET  NZ
	LD   DE,CopySpriteDataToHW+167
	RST  $38
	XOR  A
	LD   (data_A821),A
	RET 
.label_2C71:
	LD   A,(IY+$30)
	LD   C,A
	AND  $C0
	LD   B,A
	LD   A,(FrameCounter)
	AND  $0F
	ADD  A,B
	LD   (IY+$30),A
	RET 
.label_2C82:
	SUB  $0A
	RRCA
	AND  $0F
	LD   B,A
	LD   HL,data_2C94
	RST  $08
	LD   (IY+$01),A
	LD   (IY+$30),$3C
	RET 
data_2C94:
	db $FF
	db $FF
	db $7D
	db $7D
	db $7E
	db $7E
	db $7D
	db $7D
	db $5B
	db $5B
	db $5A
	db $5A
	db $59
	db $59
	db $58
	db $58
OutputString_AndEvery60000Pts:
	dw $A718
	db $13
	db $A5
	db $3B
	db $87
	db $F1
	db $34
	db $0E
	db $34
	db $D7
	db $BF
	db $F1
	db $65
	db $13
	db $13
	db $13
	db $13
	db $F1
	db $88
	db $DC
	db $ED
	db $11
	db $B9
; Function Name: function_2CBC
function_2CBC:
	LD   IX,data_A8F7+9
	LD   IY,SpriteData1_Sprites_0To2
	LD   A,(data_AD04)
	AND  A
	JR   Z,label_2CDB.label_2CF5
	CP   $04
	JR   Z,label_2CDB.label_2D02
	CALL function_2D21
	CALL function_2D36
	CALL function_2D36
	CALL function_2D68
	RET 
; Function Name: label_2CDB
label_2CDB:
	CALL function_01C2
	RET  NZ
	LD   BC,$0004
	LD   HL,data_4980
	SUB  A
.label_2CE6:
	XOR  (HL)
	INC  HL
	DJNZ .label_2CE6
	DEC  C
	JR   NZ,.label_2CE6
	ADD  A,$BD
	JP   NZ,function_0F11
	JP   AdvancePhase
.label_2CF5:
	CALL function_2D15
	CALL function_2D36
	CALL function_2D36
	CALL function_2D68
	RET 
.label_2D02:
	CALL function_2D2D
	CALL function_2D2D
	db $CD
	db $62
	db $2D
	db $CD
	db $62
	db $2D
	db $CD
	db $68
	db $2D
	db $CD
	db $68
	db $2D
	db $C9
; Function Name: function_2D15
function_2D15:
	CALL function_2D6E
	CALL function_3058
	CALL function_3058
	JP   function_308A.label_309B
; Function Name: function_2D21
function_2D21:
	CALL function_2D6E
	CALL function_3058
	CALL function_308A
	JP   function_308A.label_309B
; Function Name: function_2D2D
function_2D2D:
	CALL function_2D6E
	CALL function_3058
	db $C3
	db $9B
	db $30
; Function Name: function_2D36
function_2D36:
	CALL function_2D93
	CALL function_3058
	JP   function_308A.label_309B
; Function Name: function_2D3F
function_2D3F:
	LD   A,(data_A9C0)
	AND  A
	JP   NZ,AdvancePhase
	CALL function_4AFB
.operand_2D49:
	LD   DE,InterruptRoutine.label_0106+2
	RST  $38
	LD   A,(FrameCountdown1)
	AND  A
	JP   NZ,function_2E31.label_2E3E
	CALL SetKonamicSpriteParams
	CALL QueueKonamiLogoDraw
	LD   HL,OutputString_CopyrightKonami
	LD   B,$14
	JP   function_43B7.label_43E8
	db $CD
	db $93
	db $2D
	db $C3
	db $9B
	db $30
; Function Name: function_2D68
function_2D68:
	CALL function_2DF4
	JP   function_308A.label_309B
; Function Name: function_2D6E
function_2D6E:
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	LD   HL,(data_A808)
	CALL function_2E31
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   D,(IY+$00)
	LD   E,(IX+$05)
.operand_2D86:
	LD   HL,(data_A80A)
	CALL function_2E31
	LD   (IY+$00),H
	LD   (IX+$05),L
	RET 
; Function Name: function_2D93
function_2D93:
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	LD   HL,(data_A808)
	CALL function_303E
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   D,(IY+$00)
	LD   E,(IX+$05)
	LD   HL,(data_A80A)
	CALL function_303E
	LD   (IY+$00),H
	LD   (IX+$05),L
	RET 
; Function Name: function_2DB8
function_2DB8:
	db $21
	db $01
	db $AD
	db $34
	db $21
	db $04
	db $AD
	db $7E
	db $3C
	db $FE
	db $05
	db $38
	db $01
	db $AF
	db $77
	db $3A
	db $01
	db $AD
	db $FE
	db $06
	db $38
	db $09
	db $FE
	db $0B
	db $38
	db $0A
	db $3A
	db $D5
	db $A9
	db $18
	db $08
	db $3A
	db $D3
	db $A9
	db $18
	db $03
	db $3A
	db $D4
	db $A9
	db $32
	db $0A
	db $AD
	db $3A
	db $CD
	db $A9
	db $32
	db $02
	db $AD
	db $AF
	db $32
	db $0D
	db $AD
	db $32
	db $C6
	db $AC
	db $3D
	db $32
	db $0E
	db $AD
	db $C9
; Function Name: function_2DF4
function_2DF4:
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	LD   HL,(data_A808)
	CALL function_304D
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   D,(IY+$00)
	LD   E,(IX+$05)
	LD   HL,(data_A80A)
	CALL function_304D
	LD   (IY+$00),H
	LD   (IX+$05),L
	RET 
.label_2E19:
	LD   (data_A9C1),A
	LD   A,C
	RRCA
	RRCA
	LD   C,A
	AND  $01
	LD   (data_A9C2),A
	LD   A,C
	RRCA
	LD   C,A
	AND  $01
	LD   (BonusScheme),A
	LD   A,C
	JP   TestGrid
; Function Name: function_2E31
function_2E31:
	LD   B,H
	LD   C,L
	SRA  B
	RR   C
	SRA  B
	RR   C
	ADD  HL,BC
	ADD  HL,DE
	RET 
.label_2E3E:
	db $32
	db $01
	db $31
	db $01
	db $30
	db $01
	db $2F
	db $01
	db $2E
	db $01
	db $2D
	db $01
	db $2C
	db $01
	db $28
	db $01
	db $26
	db $01
	db $24
	db $01
	db $22
	db $01
	db $20
	db $01
	db $1B
	db $01
	db $18
	db $01
	db $16
	db $01
	db $11
	db $01
	db $0E
	db $01
	db $0B
	db $01
	db $08
	db $01
	db $03
	db $01
	db $00
	db $01
	db $FD
	db $00
	db $F8
	db $00
	db $F5
	db $00
	db $F2
	db $00
	db $ED
	db $00
	db $EA
	db $00
	db $E7
	db $00
	db $E4
	db $00
	db $DF
	db $00
	db $DC
	db $00
	db $D9
	db $00
	db $D4
	db $00
	db $D1
	db $00
	db $CD
	db $00
	db $C8
	db $00
	db $C5
	db $00
	db $C1
	db $00
	db $BB
	db $00
	db $B7
	db $00
	db $B4
	db $00
	db $AE
	db $00
	db $A8
	db $00
	db $A1
	db $00
	db $9C
	db $00
	db $93
	db $00
	db $90
	db $00
	db $88
	db $00
	db $80
	db $00
	db $7A
	db $00
	db $72
	db $00
	db $69
	db $00
	db $63
	db $00
	db $5A
	db $00
	db $51
	db $00
	db $4A
	db $00
	db $40
	db $00
	db $37
	db $00
	db $30
	db $00
	db $26
	db $00
	db $1C
	db $00
	db $12
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $FF
	db $EE
	db $FF
	db $00
	db $00
	db $DA
	db $FF
	db $D0
	db $FF
	db $C9
	db $FF
	db $C0
	db $FF
	db $B6
	db $FF
	db $AF
	db $FF
	db $A6
	db $FF
	db $9D
	db $FF
	db $97
	db $FF
	db $8E
	db $FF
	db $86
	db $FF
	db $80
	db $FF
	db $78
	db $FF
	db $70
	db $FF
	db $6D
	db $FF
	db $70
	db $FF
	db $5F
	db $FF
	db $58
	db $FF
	db $52
	db $FF
	db $4C
	db $FF
	db $49
	db $FF
	db $45
	db $FF
	db $3F
	db $FF
	db $3B
	db $FF
	db $38
	db $FF
	db $33
	db $FF
	db $2F
	db $FF
	db $2C
	db $FF
	db $27
	db $FF
	db $24
data_2F01:
	db $FF
	db $21
	db $FF
	db $21
	db $FF
	db $19
	db $FF
	db $16
	db $FF
	db $13
	db $FF
	db $0E
	db $FF
	db $0B
	db $FF
	db $08
	db $FF
	db $03
	db $FF
	db $00
	db $FF
	db $FD
	db $FE
	db $F8
	db $FE
	db $F5
	db $FE
	db $F2
	db $FE
	db $EF
	db $FE
	db $EA
	db $FE
	db $E8
	db $FE
	db $E5
	db $FE
	db $E0
	db $FE
	db $DE
	db $FE
	db $DC
	db $FE
	db $DA
	db $FE
	db $D8
	db $FE
	db $D4
	db $FE
	db $D3
	db $FE
	db $D2
	db $FE
	db $D1
	db $FE
	db $D0
	db $FE
	db $CF
	db $FE
	db $CE
	db $FE
	db $CE
	db $FE
	db $CF
	db $FE
	db $D0
	db $FE
	db $D1
	db $FE
	db $D2
	db $FE
	db $D3
	db $FE
	db $D4
	db $FE
	db $D8
	db $FE
	db $DA
	db $FE
	db $DC
	db $FE
	db $DE
	db $FE
	db $E0
	db $FE
	db $E5
	db $FE
	db $E8
	db $FE
	db $EA
	db $FE
	db $EF
	db $FE
	db $F2
	db $FE
	db $F5
	db $FE
	db $F8
	db $FE
	db $FD
	db $FE
	db $00
	db $FF
	db $03
	db $FF
	db $08
	db $FF
	db $0B
	db $FF
	db $0E
	db $FF
	db $13
	db $FF
	db $16
	db $FF
	db $19
	db $FF
	db $1C
	db $FF
	db $21
	db $FF
	db $24
	db $FF
	db $27
	db $FF
	db $2C
	db $FF
	db $2F
	db $FF
	db $33
	db $FF
	db $38
	db $FF
	db $3B
	db $FF
	db $3F
	db $FF
	db $45
	db $FF
	db $49
	db $FF
	db $4C
	db $FF
	db $52
	db $FF
	db $58
	db $FF
	db $5F
	db $FF
	db $64
	db $FF
	db $6D
	db $FF
	db $70
	db $FF
	db $78
	db $FF
	db $80
	db $FF
	db $86
	db $FF
	db $8E
	db $FF
	db $97
	db $FF
	db $9D
	db $FF
	db $A6
	db $FF
	db $AF
	db $FF
	db $B6
	db $FF
	db $C0
	db $FF
	db $C9
	db $FF
	db $D0
	db $FF
	db $DA
	db $FF
	db $E4
	db $FF
	db $EE
	db $FF
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $12
	db $00
	db $1C
	db $00
	db $26
	db $00
	db $30
	db $00
	db $37
	db $00
	db $40
	db $00
	db $4A
	db $00
	db $51
	db $00
	db $5A
	db $00
	db $63
	db $00
	db $69
	db $00
	db $72
	db $00
	db $7A
	db $00
	db $80
	db $00
	db $88
	db $00
	db $90
	db $00
	db $93
	db $00
	db $93
	db $00
	db $A1
	db $00
	db $A8
	db $00
	db $AE
	db $00
	db $B4
	db $00
	db $B7
	db $00
	db $BB
	db $00
	db $C1
	db $00
	db $C5
	db $00
	db $C8
	db $00
	db $CD
	db $00
	db $D1
	db $00
	db $D4
	db $00
	db $D9
	db $00
	db $DC
	db $00
	db $DF
	db $00
	db $DC
	db $00
	db $E7
	db $00
	db $EA
	db $00
	db $ED
	db $00
	db $F2
	db $00
	db $F5
	db $00
	db $F8
	db $00
	db $FD
	db $00
	db $00
	db $01
	db $03
	db $01
	db $08
	db $01
	db $0B
	db $01
	db $0E
	db $01
	db $11
	db $01
	db $16
	db $01
	db $18
	db $01
	db $11
	db $01
	db $20
	db $01
	db $22
	db $01
	db $24
	db $01
	db $26
	db $01
	db $28
	db $01
	db $2C
	db $01
	db $2D
	db $01
	db $2E
	db $01
	db $2F
	db $01
	db $30
	db $01
	db $31
	db $01
	db $32
	db $01
; Function Name: function_303E
function_303E:
	LD   B,H
	LD   C,L
	SRA  B
	RR   C
	SRA  B
	RR   C
	AND  A
	SBC  HL,BC
	ADD  HL,DE
	RET 
; Function Name: function_304D
function_304D:
	LD   B,H
	LD   C,L
	SRA  B
	RR   C
	AND  A
	SBC  HL,BC
	ADD  HL,DE
	RET 
; Function Name: function_3058
function_3058:
	LD   B,(IY+$31)
	LD   C,(IY+$00)
	LD   A,$10
	ADD  A,B
	LD   (IY+$33),A
	LD   (IY+$02),C
	JP   function_308A.label_309B
	db $FD
	db $46
	db $31
	db $FD
	db $4E
	db $00
	db $26
	db $08
	db $2E
	db $6E
	db $7E
	db $81
	db $FD
	db $70
	db $33
	db $FD
	db $77
	db $02
	db $C3
	db $9B
	db $30
OutputString_Ready:
	dw $A673
	db $10
	db $F1
	db $D7
	db $34
	db $A5
	db $87
	db $BF
	db $F1
	db $B9
; Function Name: function_308A
function_308A:
	LD   B,(IY+$31)
	LD   C,(IY+$00)
	LD   H,$F0
	LD   L,$10
	ADD  HL,BC
	LD   (IY+$33),H
	LD   (IY+$02),L
.label_309B:
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	RET 
; Function Name: function_30A5
function_30A5:
	LD   HL,OutputString_CopyrightKonami
	LD   C,$22
	LD   B,$10
	CALL function_0B4C
	LD   A,(data_AD04)
	ADD  A,A
	ADD  A,A
	ADD  A,A
	LD   C,A
	LD   HL,.label_3156+32
	RST  $18
	LD   DE,SpriteData1_Sprites_0To2+1
	LD   B,$08
.label_30BF:
	LD   A,(HL)
	LD   (DE),A
	INC  HL
	INC  DE
	INC  DE
	DJNZ .label_30BF
	LD   A,(data_AD04)
	CP   $04
	LD   C,A
	JP   Z,.label_3156
	LD   A,$CC
	LD   HL,SpriteData2_Sprites_0To2
	LD   DE,$0002
	LD   B,$08
.label_30D9:
	LD   (HL),A
	ADD  HL,DE
	DJNZ .label_30D9
	LD   A,C
	CP   $04
	JP   C,.label_3117
	db $21
	db $C7
	db $AC
	db $7E
	db $FE
	db $3B
	db $C2
	db $5B
	db $31
	db $23
	db $7E
	db $FE
	db $05
	db $CA
	db $F8
	db $30
	db $FE
	db $10
	db $C2
	db $5B
	db $31
	db $06
	db $08
	db $FD
	db $21
	db $30
	db $AA
	db $21
	db $5E
	db $31
	db $7E
	db $FD
	db $77
	db $31
	db $23
	db $7E
	db $FD
	db $77
	db $00
	db $23
	db $FD
	db $23
	db $FD
	db $23
	db $10
	db $F0
	db $C3
	db $BC
	db $2C
.label_3114:
	db $C3
	db $7F
	db $30
.label_3117:
	LD   HL,data_AD37+2
	LD   A,(HL)
	CP   $68
	JP   NZ,.label_3114
	INC  HL
	LD   A,(HL)
	CP   $10
	JP   Z,.label_312C
	CP   $05
	JP   NZ,.label_3114
.label_312C:
	LD   HL,.label_3156+24
	LD   B,$04
	LD   IY,SpriteData1_Sprites_0To2
.label_3135:
	LD   A,(HL)
	LD   (IY+$31),A
	ADD  A,$10
	LD   (IY+$33),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$00),A
	LD   (IY+$02),A
	INC  HL
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	LD   DE,$0004
	ADD  IY,DE
	DJNZ .label_3135
	JP   function_2CBC
.label_3156:
	db $3E
	db $28
	db $C3
	db $D1
	db $30
	db $C3
	db $76
	db $31
	db $40
	db $68
	db $38
	db $62
	db $60
	db $70
	db $68
	db $D8
	db $88
	db $58
	db $99
	db $B0
	db $37
	db $43
	db $CF
	db $78
	db $20
	db $D0
	db $50
	db $60
	db $A0
	db $A0
	db $D0
	db $60
	db $60
	db $68
	db $61
	db $60
	db $61
	db $62
	db $63
	db $5C
	db $74
	db $75
	db $76
	db $60
	db $61
	db $64
	db $65
	db $5D
	db $77
	db $78
	db $79
	db $66
	db $67
	db $64
	db $65
	db $5E
	db $7A
	db $7B
	db $7C
	db $60
	db $61
	db $62
	db $63
	db $5F
	db $31
	db $30
	db $33
	db $32
	db $85
	db $86
	db $87
	db $85
OutputString_TimePilotLogo_Top:
	dw $A708
	db $32
	db $CA
	db $7E
	db $C8
	db $FF
	db $5F
	db $93
	db $FB
	db $C4
	db $AF
	db $D8
	db $2A
	db $6C
	db $E1
	db $7A
	db $42
	db $BD
	db $B0
	db $5A
	db $B9
; Function Name: function_31B4
function_31B4:
	LD   A,(data_AD05)
	LD   C,A
	AND  $F0
	JR   Z,.label_31C9
	CP   $30
	JP   NZ,label_3252.label_326C
	LD   A,(data_4903)
	CP   $30
	JP   NZ,.label_31C9
.label_31C9:
	LD   A,C
	AND  $0F
	CP   $07
	RET  NC
	LD   IX,data_A844+12
	LD   IY,PlayerSprite_No+9
	ADD  A,A
	LD   C,A
	LD   B,$00
	ADD  IY,BC
	ADD  A,A
	ADD  A,A
	ADD  A,A
	LD   C,A
	ADD  IX,BC
	LD   A,(IX+$00)
	INC  A
	RET  NZ
	CALL function_323A
	LD   A,(IX+$08)
	CP   $10
	RET  Z
	CP   $11
	JR   Z,.label_3201
	ADD  A,A
	LD   HL,data_AC65
	RST  $18
	CALL function_33B8
	LD   (IX+$01),A
	RET 
.label_3201:
	LD   HL,data_AC65
	CALL function_33B8
	ADD  A,$80
	LD   (IX+$01),A
	LD   (IX+$08),$10
.operand_3210:
	LD   (IX+$09),$00
	RET 
.label_3215:
	CALL function_0B2B
	XOR  A
	LD   (data_AD31),A
	LD   (data_AD20),A
	DEC  A
	LD   (data_AD30),A
	LD   A,(data_A9C1)
	LD   (data_AD10),A
	LD   HL,Credits
	LD   A,(HL)
.operand_322D:
	SUB  $01
	DAA
	LD   (HL),A
	CALL function_4AFB
	CALL function_4B30
	JP   label_1719.label_172A
; Function Name: function_323A
function_323A:
	LD   A,(IX+$09)
	AND  A
	RET  Z
	DEC  A
	LD   (IX+$09),A
	LD   C,A
.operand_3244:
	LD   A,(IX+$0A)
	LD   HL,function_3421+23
	RST  $10
	EX   DE,HL
	LD   A,C
	RST  $08
	LD   (IX+$08),A
	RET 
; Function Name: label_3252
label_3252:
	LD   BC,CoordinatePairs+112
	LD   HL,RST08_AddAToHLAndGetContentsOfHL
	LD   E,$00
.label_325A:
	LD   A,E
	XOR  (HL)
	INC  HL
	DEC  BC
	LD   E,A
	LD   A,C
	OR   B
	JR   NZ,.label_325A
	LD   A,$52
	ADD  A,E
	JP   NZ,function_0F11
	JP   AdvancePhase
.label_326C:
	LD   A,C
	AND  $0F
.operand_326F:
	CP   $07
	RET  NZ
	LD   IX,data_AC64
	LD   A,(PlayerAngle)
	ADD  A,$40
	CALL function_59D1
	EX   DE,HL
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	LD   A,H
	ADD  A,$78
	LD   (IX+$10),A
	LD   A,H
	NEG  
	ADD  A,$78
	LD   (IX+$14),A
	ADD  HL,HL
	LD   A,H
	ADD  A,$78
	LD   (IX+$12),A
	LD   A,H
	NEG  
	ADD  A,$78
	LD   (IX+$16),A
	LD   H,B
	LD   L,C
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	LD   A,H
	ADD  A,$84
	LD   (IX+$11),A
	LD   A,H
	NEG  
	ADD  A,$84
	LD   (IX+$15),A
	ADD  HL,HL
	LD   A,H
	ADD  A,$84
	LD   (IX+$13),A
	LD   A,H
	NEG  
	ADD  A,$84
	LD   (IX+$17),A
	LD   A,(PlayerAngle)
	CALL function_59D1
	EX   DE,HL
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	LD   A,H
	ADD  A,$78
	LD   (IX+$18),A
	ADD  HL,HL
	LD   A,H
	ADD  A,$78
	LD   (IX+$1A),A
	LD   H,B
	LD   L,C
	ADD  HL,HL
	ADD  HL,HL
	ADD  HL,HL
	LD   A,H
	ADD  A,$84
	LD   (IX+$19),A
	ADD  HL,HL
	LD   A,H
	ADD  A,$84
	LD   (IX+$1B),A
	RET 
; Function Name: label_32EB
label_32EB:
	LD   (WatchdogReset_DipSW2),A
	LD   HL,data_A9EB
	LD   (HL),$0C
.delay_outer:
	LD   BC,$0000
.delay_inner:
	DJNZ .delay_inner
	LD   (WatchdogReset_DipSW2),A
	DEC  C
	JR   NZ,.delay_inner
	DEC  (HL)
	JR   NZ,.delay_outer
	XOR  A
	CALL SendAudioCommand			; send 0
	LD   A,(data_4C87)
	JP   EnableInterrupts
; Function Name: Phase8Function
Phase8Function:
	LD   HL,data_A9EB
	DEC  (HL)
	RET  NZ
	CALL function_4CC3
	JP   NC,.label_3326
	LD   DE,CoordinatePairs+121
	RST  $38
	LD   E,$0B
	RST  $38
	LD   A,(QueueTitleScreenDraw.operand_0841+2)
	LD   (ProgramPhase),A
	JP   PhaseBFunction.label_12E7
.label_3326:
	CALL function_583A
	LD   A,$00
	LD   (CharacterAttrib),A
	LD   A,$F1
	LD   (CharacterCode),A
	CALL function_01E1
	LD   B,$00
	LD   HL,function_01E1+16
	XOR  A
.label_333C:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_333C
	SUB  $19
	CALL NZ,function_0F11
	JP   AdvancePhase
OutputString_OneOrTwoPlayers:
	dw $A711
	db $13
	db $68
	db $3B
	db $34
	db $F1
	db $68
	db $D7
	db $F1
	db $DC
	db $0F
	db $68
	db $F1
	db $88
	db $57
	db $A5
	db $BF
	db $34
	db $D7
	db $ED
	db $B9
; Function Name: Phase1Function
Phase1Function:
	LD   A,(FunctionChecksum)
	LD   HL,function_178C
	LD   B,$1E
.label_3366:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_3366
	ADD  A,$2C
	LD   (FunctionChecksum),A
	LD   A,(data_AD32)
	AND  A
	LD   DE,data_AD1B
	LD   A,(data_AD14)
.operand_3379:
	JR   Z,.label_3381
	LD   DE,data_AD2B
	LD   A,(data_AD24)
.label_3381:
	ADD  A,A
	LD   HL,label_0F8D
	RST  $08
	LD   (DE),A			; get character code
	LD   (CharacterCode),A
	INC  HL
	INC  DE
	LD   A,(HL)
	LD   (DE),A			; get attrib
	LD   HL,CharacterAttrib
	CP   (HL)			; compare to current
	LD   (HL),A
	CALL Z,AdvancePhase			; advance to next phase if they are the same
	CALL function_01E1
	JP   AdvancePhase
; Function Name: function_339C
function_339C:
	db $3A
	db $32
	db $AD
	db $A7
data_33A0:
	db $11
	db $1B
	db $AD
	db $3A
	db $14
	db $AD
	db $28
	db $06
	db $11
	db $2B
	db $AD
	db $3A
	db $24
	db $AD
	db $87
	db $21
	db $8D
	db $0F
	db $DF
	db $ED
data_33B4:
	db $A0
	db $ED
	db $A0
	db $C9
; Function Name: function_33B8
function_33B8:
	LD   C,$00
	LD   B,(IY+$31)
	LD   E,(HL)
	DEC  L
	LD   A,(HL)
	SUB  B
	JR   NC,.label_33C7
	NEG  
	SET  0,C
.label_33C7:
	LD   D,A
	LD   B,(IY+$00)
	LD   A,E
	SUB  B
	JR   NC,.label_33D3
	NEG  
	SET  1,C
.label_33D3:
	LD   E,A
	EX   AF,AF'
	LD   A,E
	EX   AF,AF'
	SUB  D
	JR   Z,.label_340F
	JR   NC,.label_33DE
	SET  2,C
.label_33DE:
	LD   L,$00
	BIT  2,C
	JR   NZ,.label_33E7
	LD   H,D
	JR   .label_33E9
.label_33E7:
	LD   H,E
	LD   E,D
.label_33E9:
	LD   B,$08
	XOR  A
.label_33EC:
	ADC  HL,HL
	LD   A,H
	JR   C,.label_33F4
	CP   E
	JR   C,.label_33F7
.label_33F4:
	SUB  E
	LD   H,A
	XOR  A
.label_33F7:
	CCF
	DJNZ .label_33EC
	LD   B,L
	LD   A,C
	LD   HL,.label_340F+6
	RST  $18
	LD   A,B
	RRCA
	RRCA
	AND  $1F
	BIT  5,(HL)
	JR   Z,.label_340D
	LD   B,A
	LD   A,$1F
	SUB  B
.label_340D:
	ADD  A,(HL)
	RET 
.label_340F:
	LD   HL,.label_340F+14
	LD   A,C
	RST  $08
	RET 
	db $20
	db $40
	db $C0
	db $A0
	db $00
	db $60
	db $E0
	db $80
	db $20
	db $60
	db $E0
	db $A0
; Function Name: function_3421
function_3421:
	LD   HL,OutputStringTable
	CALL GetTablePtr
	EX   DE,HL
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  HL
	INC  HL
	LD   A,(CharacterAttrib)
	ADD  A,$05
	AND  $0F
	LD   C,A
	JP   OutputString.string_loop
	db $6F
	db $34
	db $8F
	db $34
	db $AF
	db $34
	db $CF
	db $34
	db $EF
	db $34
	db $0F
	db $35
	db $2F
	db $35
	db $4F
	db $35
	db $6F
	db $35
	db $8F
	db $35
	db $AF
	db $35
	db $CF
	db $35
	db $EF
	db $35
	db $0F
	db $36
	db $2F
	db $36
	db $4F
	db $36
	db $6F
	db $36
	db $8F
	db $36
OutputString_OnePlayerOnly:
	dw $A711
	db $13
	db $68
	db $3B
	db $34
	db $F1
	db $88
	db $57
	db $A5
	db $BF
	db $34
	db $D7
	db $F1
	db $68
	db $3B
	db $57
	db $BF
	db $B9
	db $11
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $11
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $09
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $11
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $11
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $11
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $0D
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $0C
	db $0D
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $0C
	db $0D
	db $0D
	db $11
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $09
	db $10
	db $10
	db $10
	db $11
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $08
	db $10
	db $10
	db $10
	db $11
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $10
	db $10
	db $10
	db $11
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $0A
	db $10
	db $10
	db $10
	db $11
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $10
	db $10
	db $10
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $0D
	db $10
	db $10
	db $10
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $11
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $0D
	db $10
	db $10
	db $10
; Function Name: function_36AF
function_36AF:
	LD   A,(data_ACC6)
	AND  A
	RET  NZ
	LD   A,(data_AD04)
	CP   $04
	JP   Z,function_382D.label_386E
	LD   HL,data_AD05
	LD   A,(data_AD06)
	AND  $0F
	CP   $07
	JP   Z,function_382D.label_3855
	JP   C,.label_37BD
	CP   $09
	JP   C,.label_379F
	LD   A,(HL)
	AND  A
	RET  NZ
	CALL function_4B4B
	RRCA
	LD   A,(data_AD04)
	ADC  A,A
	LD   HL,data_ACC2
	LD   (HL),$FF
	INC  HL
	LD   (HL),A
	LD   A,(PlayerAngle)
	ADD  A,$08
	RRCA
	RRCA
	RRCA
	RRCA
	AND  $0F
	LD   HL,function_382D.label_386E+107
	RST  $18
	LD   C,(HL)
	LD   A,(data_ACC3)
	ADD  A,A
	ADD  A,A
	ADD  A,A
	ADD  A,A
	LD   HL,function_382D.label_386E+269
	RST  $18
	EX   DE,HL
	LD   A,(data_ACC1)
	LD   B,A
	LD   A,(RemainingShipsInWave)
	AND  A
	JR   NZ,.label_370B
	LD   B,$05
.label_370B:
	XOR  A
	LD   (data_A811),A
	LD   IX,data_A844+12
	LD   IY,PlayerSprite_No+9
.label_3717:
	LD   A,(IX+$00)
	AND  A
	JP   NZ,.label_3768
	LD   A,(DE)
	ADD  A,C
	ADD  A,A
	LD   HL,function_382D.label_386E+123
	RST  $08
	LD   (IY+$31),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$00),A
	LD   A,(PlayerAngle)
	ADD  A,$80
	LD   (IX+$01),A
	LD   (IX+$02),A
	CALL function_382D
	ADD  A,$09
	LD   (IX+$0A),A
	INC  DE
	LD   A,(DE)
	LD   (IX+$0E),A
	INC  DE
	LD   (IX+$03),$00
	LD   (IX+$05),$00
	LD   (IX+$09),$20
	EXX 
	CALL function_323A
	EXX 
	LD   (IX+$00),$FE
	LD   A,(IX+$0E)
	AND  A
	JR   NZ,.label_3764
	db $DD
	db $34
	db $00
.label_3764:
	LD   HL,data_A811
	INC  (HL)
.label_3768:
	EX   DE,HL
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	EX   DE,HL
	DJNZ .label_3717
	XOR  A
	LD   (data_ACC2),A
	LD   A,$E4
	LD   (FrameCountdown2),A
	LD   HL,data_A811
	LD   A,(HL)
	CP   $05
	JP   NC,function_580B.label_5817
	LD   HL,data_ACC1
	CP   (HL)
	LD   A,(HL)
	LD   (data_A811),A
	JP   NC,function_580B.label_5817
	RET 
.label_3793:
	LD   B,$05
	LD   IX,data_A844+76
	LD   IY,PlayerSprite_No+17
	JR   .label_37D6
.label_379F:
	LD   A,(HL)
	AND  A
	JR   Z,.label_37A6
	CP   $30
	RET  NZ
.label_37A6:
	LD   HL,data_A844+12
	LD   DE,RST10_AddDoubleAToHL
	LD   BC,CopySpriteDataToHW.CopySpritesFlipped+426
.label_37AF:
	LD   A,(HL)
	AND  A
	JR   Z,.label_37B4
	INC  C
.label_37B4:
	ADD  HL,DE
	DJNZ .label_37AF
	LD   A,C
	CP   $02
	RET  NC
	JR   .label_37C4
.label_37BD:
	LD   A,(HL)
	AND  A
	JR   Z,.label_37C4
	CP   $30
	RET  NZ
.label_37C4:
	LD   A,(RemainingShipsInWave)
	AND  A
	JR   Z,.label_3793
	LD   A,(data_ACC1)
	LD   B,A
	LD   IX,data_A8A0+16
	LD   IY,data_AA24+2
.label_37D6:
	LD   A,(IX+$00)
	AND  A
	JP   NZ,function_382D.label_3847
	DEC  (IX+$00)
	LD   A,(PlayerAngle)
	RRCA
	RRCA
	AND  $3F
	LD   C,A
	CALL function_4B4B
	AND  $0F
	SUB  $08
	ADD  A,C
	AND  $3F
	LD   HL,function_382D.label_386E+397
	RST  $08
	ADD  A,A
	ADD  A,A
	LD   HL,function_382D.label_386E+461
	RST  $08
	LD   (IY+$31),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$00),A
	LD   A,(PlayerAngle)
	ADD  A,$80
	LD   (IX+$01),A
	LD   (IX+$02),A
	CALL function_382D
	LD   (IX+$0A),A
	XOR  A
	LD   (data_ACC5),A
	LD   (IX+$03),$00
	LD   (IX+$05),$00
	LD   (IX+$09),$20
	CALL function_323A
	LD   (IX+$0E),$00
	RET 
; Function Name: function_382D
function_382D:
	CALL function_4B4B
	LD   HL,data_ACC4
	CP   (HL)
	JR   NC,.label_3842
	LD   HL,data_A9CF
	LD   A,(HL)
	INC  A
	CP   $05
	JR   C,.label_3840
	XOR  A
.label_3840:
	LD   (HL),A
	RET 
.label_3842:
	AND  $03
	ADD  A,$05
	RET 
.label_3847:
	LD   DE,data_FFE0+16
	ADD  IX,DE
	DEC  IY
	DEC  IY
	DEC  B
	JP   NZ,function_36AF.label_37D6
	RET 
.label_3855:
	LD   A,(HL)
	AND  A
	RET  NZ
	LD   IX,data_A844+12
	LD   DE,RST10_AddDoubleAToHL
	LD   B,$05
.label_3861:
	LD   (IX+$08),$11
	LD   (IX+$09),$00
	ADD  IX,DE
	DJNZ .label_3861
	RET 
.label_386E:
	db $DD
	db $21
	db $50
	db $A8
	db $FD
	db $21
	db $1A
	db $AA
	db $3A
	db $C1
	db $AC
	db $47
	db $3A
	db $0D
	db $AD
	db $A7
	db $28
	db $02
	db $06
	db $05
	db $C5
	db $DD
	db $7E
	db $00
	db $A7
	db $C2
	db $C0
	db $38
	db $CD
	db $4B
	db $4B
	db $E6
	db $FC
	db $21
	db $3B
	db $3A
	db $CF
	db $FD
	db $77
	db $31
	db $23
	db $7E
	db $FD
	db $77
	db $00
	db $23
	db $7E
	db $DD
	db $77
	db $01
	db $DD
	db $77
	db $02
	db $3A
	db $C1
	db $AC
	db $90
	db $21
	db $D2
	db $38
	db $CF
	db $DD
	db $77
	db $0A
	db $DD
	db $36
	db $09
	db $20
	db $CD
	db $3A
	db $32
	db $DD
	db $36
	db $04
	db $01
	db $DD
	db $36
	db $0E
	db $00
	db $DD
	db $35
	db $00
	db $11
	db $10
	db $00
	db $DD
	db $19
	db $FD
	db $23
	db $FD
	db $23
	db $C1
	db $10
	db $B6
	db $3E
	db $E4
	db $32
	db $12
	db $A8
	db $C9
	db $0A
	db $0B
	db $0D
	db $0E
	db $0F
	db $09
	db $0C
	db $08
	db $0C
	db $0F
	db $13
	db $16
	db $1A
	db $1D
	db $21
	db $24
	db $28
	db $2B
	db $2F
	db $33
	db $37
	db $3A
	db $3D
	db $F0
	db $10
	db $F0
	db $20
	db $F0
	db $30
	db $F0
	db $40
	db $F0
	db $50
	db $F0
	db $60
	db $F0
	db $70
	db $F0
	db $80
	db $F0
	db $90
	db $F0
	db $A0
	db $F0
	db $B0
	db $F0
	db $C0
	db $F0
	db $D0
	db $F0
	db $E0
	db $F0
	db $F0
	db $E0
	db $F8
	db $D0
	db $F8
	db $C0
	db $F8
	db $B0
	db $F8
	db $A0
	db $F8
	db $90
	db $F8
	db $80
	db $F8
	db $70
	db $F8
	db $60
	db $F8
	db $50
	db $F8
	db $40
	db $F8
	db $30
	db $F8
	db $20
	db $F8
	db $10
	db $F8
	db $00
	db $F0
	db $00
	db $E0
	db $00
	db $D0
	db $00
	db $C0
	db $00
	db $B0
	db $00
	db $A0
	db $00
	db $90
	db $00
	db $80
	db $00
	db $70
	db $00
	db $60
	db $00
	db $50
	db $00
	db $40
	db $00
	db $30
	db $00
	db $20
	db $00
	db $10
	db $10
	db $10
	db $20
	db $10
	db $30
	db $10
	db $40
	db $10
	db $50
	db $10
	db $60
	db $10
	db $70
	db $10
	db $80
	db $10
	db $90
	db $10
	db $A0
	db $10
	db $B0
	db $10
	db $C0
	db $10
	db $D0
	db $10
	db $E0
	db $10
	db $F0
	db $10
	db $F0
	db $20
	db $F0
	db $30
	db $F0
	db $40
	db $F0
	db $50
	db $F0
	db $60
	db $F0
	db $70
	db $F0
	db $80
	db $F0
	db $90
	db $F0
	db $A0
	db $F0
	db $B0
	db $F0
	db $C0
	db $F0
	db $D0
	db $F0
	db $E0
	db $F0
	db $F0
	db $00
	db $01
	db $01
	db $11
	db $FF
	db $11
	db $02
	db $21
	db $FE
	db $21
	db $03
	db $31
	db $FD
	db $31
	db $00
	db $00
	db $00
	db $11
	db $01
	db $01
	db $FF
	db $01
	db $02
	db $11
	db $FE
	db $11
	db $03
	db $21
	db $FD
	db $21
	db $00
	db $00
	db $00
	db $01
	db $02
	db $11
	db $FE
	db $11
	db $03
	db $21
	db $FD
	db $21
	db $04
	db $31
	db $FC
	db $31
	db $00
	db $00
	db $00
	db $31
	db $03
	db $01
	db $FD
	db $01
	db $04
	db $11
	db $FC
	db $11
	db $03
	db $11
	db $FD
	db $11
	db $00
	db $00
	db $00
	db $01
	db $03
	db $01
	db $FD
	db $01
	db $04
	db $11
	db $FC
	db $11
	db $05
	db $21
	db $FB
	db $21
	db $00
	db $00
	db $00
	db $01
	db $03
	db $11
	db $FD
	db $11
	db $00
	db $21
	db $03
	db $21
	db $FD
	db $21
	db $00
	db $31
	db $00
	db $00
	db $03
	db $01
	db $FD
	db $01
	db $03
	db $11
	db $FD
	db $11
	db $05
	db $11
	db $FB
	db $11
	db $00
	db $29
	db $00
	db $00
	db $00
	db $01
	db $03
	db $11
	db $FD
	db $11
	db $05
	db $21
	db $FB
	db $21
	db $03
	db $31
	db $FD
	db $31
	db $00
	db $00
	db $08
	db $09
	db $0A
	db $0B
	db $0C
	db $0D
	db $0D
	db $0E
	db $0F
	db $10
	db $11
	db $12
	db $13
	db $14
	db $14
	db $15
	db $16
	db $17
	db $18
	db $19
	db $1A
	db $1B
	db $1B
	db $1C
	db $1D
	db $1E
	db $1F
	db $20
	db $21
	db $22
	db $22
	db $23
	db $24
	db $25
	db $26
	db $27
	db $28
	db $29
	db $29
	db $2A
	db $2B
	db $2C
	db $2D
	db $2E
	db $2F
	db $30
	db $31
	db $32
	db $33
	db $34
	db $35
	db $36
	db $37
	db $38
	db $38
	db $39
	db $00
	db $01
	db $02
	db $03
	db $04
	db $05
	db $06
	db $07
	db $F0
	db $10
	db $60
	db $00
	db $F0
	db $20
	db $80
	db $00
	db $F0
	db $30
	db $80
	db $00
	db $F0
	db $40
	db $80
	db $00
	db $F0
	db $50
	db $80
	db $00
	db $F0
	db $60
	db $80
	db $00
	db $F0
	db $70
	db $80
	db $00
	db $F0
	db $80
	db $80
	db $00
	db $F0
	db $90
	db $80
	db $00
	db $F0
	db $A0
	db $80
	db $00
	db $F0
	db $B0
	db $80
	db $00
	db $F0
	db $C0
	db $80
	db $00
	db $F0
	db $D0
	db $80
	db $00
	db $F0
	db $E0
	db $80
	db $00
	db $F0
	db $F0
	db $A0
	db $00
	db $E0
	db $F8
	db $C0
	db $00
	db $D0
	db $F8
	db $C0
	db $00
	db $C0
	db $F8
	db $C0
	db $00
	db $B0
	db $F8
	db $C0
	db $00
	db $A0
	db $F8
	db $C0
	db $00
	db $90
	db $F8
	db $C0
	db $00
	db $80
	db $F8
	db $C0
	db $00
	db $70
	db $F8
	db $C0
	db $00
	db $60
	db $F8
	db $C0
	db $00
	db $50
	db $F8
	db $C0
	db $00
	db $40
	db $F8
	db $C0
	db $00
	db $30
	db $F8
	db $C0
	db $00
	db $20
	db $F8
	db $C0
	db $00
	db $10
	db $F8
	db $C0
	db $00
	db $00
	db $F0
	db $E0
	db $00
	db $00
	db $E0
	db $00
	db $00
	db $00
	db $D0
	db $00
	db $00
	db $00
	db $C0
	db $00
	db $00
	db $00
	db $B0
	db $00
	db $00
	db $00
	db $A0
	db $00
	db $00
	db $00
	db $90
	db $00
	db $00
	db $00
	db $80
	db $00
	db $00
	db $00
	db $70
	db $00
	db $00
	db $00
	db $60
	db $00
	db $00
	db $00
	db $50
	db $00
	db $00
	db $00
	db $40
	db $00
	db $00
	db $00
	db $30
	db $00
	db $00
	db $00
	db $20
	db $00
	db $00
	db $00
	db $10
	db $20
	db $00
	db $10
	db $10
	db $40
	db $00
	db $20
	db $10
	db $40
	db $00
	db $30
	db $10
	db $40
	db $00
	db $40
	db $10
	db $40
	db $00
	db $50
	db $10
	db $40
	db $00
	db $60
	db $10
	db $40
	db $00
	db $70
	db $10
	db $40
	db $00
	db $80
	db $10
	db $40
	db $00
	db $90
	db $10
	db $40
	db $00
	db $A0
	db $10
	db $40
	db $00
	db $B0
	db $10
	db $40
	db $00
	db $C0
	db $10
	db $40
	db $00
	db $D0
	db $10
	db $40
	db $00
	db $E0
	db $10
	db $40
	db $00
	db $F0
	db $10
	db $60
	db $00
	db $F0
	db $20
	db $80
	db $00
	db $F0
	db $30
	db $80
	db $00
	db $F0
	db $40
	db $80
	db $00
	db $F0
	db $50
	db $80
	db $00
	db $F0
	db $60
	db $80
	db $00
	db $F0
	db $70
	db $80
	db $00
	db $F0
	db $80
	db $80
	db $00
	db $F0
	db $90
	db $80
	db $00
	db $F0
	db $A0
	db $80
	db $00
	db $F0
	db $B0
	db $80
	db $00
	db $F0
	db $C0
	db $80
	db $00
	db $F0
	db $D0
	db $80
	db $00
	db $F0
	db $E0
	db $80
	db $00
	db $F0
	db $F0
	db $A0
	db $00
; Function Name: function_3B5F
function_3B5F:
	LD   A,(data_AD04)
	DEC  A
	RET  NZ
	LD   IX,data_A8C0
	LD   IY,data_AA28
	LD   A,(IX+$00)
	AND  A
	JP   Z,function_3C0D.label_3C25
	INC  A
	JP   NZ,.label_3B94
.label_3B77:
	CALL function_3E05
	LD   A,(IY+$31)
	ADD  A,$10
	LD   (IY+$33),A
	LD   A,(IY+$00)
	LD   (IY+$02),A
	CALL function_3CC4
	JP   C,function_3C0D
	CALL function_3CE9
	JP   function_3CE9.label_3D25
.label_3B94:
	DEC  A
	LD   C,A
	LD   HL,data_A8DC
	LD   A,(HL)
	AND  A
	JP   Z,.label_3BA9
	DEC  (HL)
	LD   (IX+$00),$FF
	CALL function_5683
	JP   .label_3B77
.label_3BA9:
	LD   A,C
	CP   $61
	JR   C,.label_3BBD
	LD   (IX+$00),$61
	CALL function_5683
	LD   (IY+$30),$3D
	LD   (IY+$32),$3D
.label_3BBD:
	DEC  (IX+$00)
	JR   Z,function_3C0D
	CALL label_2B52.label_2B60
	LD   A,(IY+$31)
	ADD  A,$10
	LD   (IY+$33),A
	LD   A,(IY+$00)
	LD   (IY+$02),A
	LD   A,(IX+$00)
	SUB  $40
	JP   Z,.label_3BF1
	RET  C
	LD   C,A
	AND  $07
	RET  NZ
	LD   A,C
	RRCA
	RRCA
	RRCA
	DEC  A
	LD   HL,.label_3BF1+24
	RST  $08
	LD   (IY+$03),A
	INC  A
	LD   (IY+$01),A
	RET 
.label_3BF1:
	LD   DE,CopySpriteDataToHW+166
	RST  $38
	LD   (IY+$03),$FA
	LD   (IY+$01),$FB
	LD   (IY+$30),$6C
	LD   (IY+$32),$6C
	DEC  (IX+$00)
	RET 
	db $96
	db $94
	db $92
	db $90
; Function Name: function_3C0D
function_3C0D:
	XOR  A
	LD   (IX+$00),A
	LD   (IX+$10),A
	LD   (IY+$00),A
	LD   (IY+$31),A
	LD   (data_AA5B),A
	LD   (data_AA2A),A
	LD   (IX+$0E),$80
	RET 
.label_3C25:
	LD   A,(FrameCounter)
	AND  $01
	RET  NZ
	DEC  (IX+$0E)
	JP   Z,.label_3C32
	RET 
.label_3C32:
	LD   A,(data_AD0D)
	AND  A
	RET  NZ
	LD   A,(PlayerAngle)
	LD   B,A
	ADD  A,$08
	AND  $7F
	CP   $10
	JR   C,.label_3C75
	LD   A,B
	RRCA
	RRCA
	AND  $3E
	LD   HL,.label_3C75+15
	RST  $08
	LD   (IY+$31),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$00),A
	LD   A,B
	ADD  A,$C0
	AND  $80
	LD   (IX+$02),A
	CALL function_5942
	LD   (IX+$0A),E
	LD   (IX+$0B),D
	LD   (IX+$0C),C
	LD   (IX+$0D),B
	LD   A,$03
	LD   (data_A8DC),A
	LD   (IX+$00),$FF
	RET 
.label_3C75:
	db $3A
	db $80
	db $A9
	db $4F
	db $3E
	db $10
	db $CB
	db $59
	db $20
	db $02
	db $ED
	db $44
	db $80
	db $18
	db $C0
	db $EC
	db $80
	db $EC
	db $88
	db $EC
	db $90
	db $EC
	db $A0
	db $EC
	db $B0
	db $EC
	db $C0
	db $EC
	db $D0
	db $EC
	db $E0
	db $F0
	db $EC
	db $F0
	db $EC
	db $F0
	db $E0
	db $F0
	db $D0
	db $F0
	db $C0
	db $F0
	db $B0
	db $F0
	db $A0
	db $F0
	db $90
	db $F0
	db $80
	db $F0
	db $78
	db $F0
	db $70
	db $F0
	db $60
	db $F0
	db $50
	db $F0
	db $40
	db $F0
	db $30
	db $F0
	db $28
	db $F0
	db $20
	db $EC
	db $20
	db $EC
	db $30
	db $EC
	db $40
	db $EC
	db $50
	db $EC
	db $60
	db $EC
	db $70
	db $EC
	db $78
; Function Name: function_3CC4
function_3CC4:
	LD   A,(IX+$02)
	ADD  A,$40
	BIT  7,A
	JP   NZ,.label_3CD9
	LD   A,(IY+$31)
	ADD  A,$13
	CP   $03
	RET  C
	JP   .label_3CE1
.label_3CD9:
	LD   A,(IY+$31)
	ADD  A,$10
	CP   $03
	RET  C
.label_3CE1:
	LD   A,(IY+$00)
	ADD  A,$02
	CP   $04
	RET 
; Function Name: function_3CE9
function_3CE9:
	LD   A,(FrameCounter)
	AND  $02
	LD   B,A
	LD   A,(data_A8DC)
	LD   C,A
	LD   A,$03
	SUB  C
	ADD  A,A
	ADD  A,A
	ADD  A,$A0
	ADD  A,B
	LD   C,A
	LD   A,(IX+$02)
	ADD  A,$40
	CP   $80
	JR   C,.label_3D15
	LD   (IY+$01),C
	INC  C
	LD   (IY+$03),C
	LD   (IY+$30),$ED
	LD   (IY+$32),$ED
	RET 
.label_3D15:
	LD   (IY+$03),C
	INC  C
	LD   (IY+$01),C
	LD   (IY+$30),$6D
	LD   (IY+$32),$6D
	RET 
.label_3D25:
	LD   A,(IX+$00)
	INC  A
	RET  NZ
	LD   A,(FrameCountdown3)
	AND  A
	RET  NZ
	LD   A,(data_A8C6)
	AND  A
	RET  Z
	CP   $01
	JP   Z,.label_3D40
	db $3A
	db $E0
	db $A8
	db $A7
	db $CA
	db $45
	db $3D
.label_3D40:
	LD   A,(data_A840)
	AND  A
	RET  NZ
	LD   B,$02
	LD   A,(data_A8D6)
	LD   D,A
	ADD  A,A
	LD   E,A
.label_3D4D:
	LD   A,$84
	SUB  (IY+$00)
	ADD  A,D
	CP   E
	JP   NC,.label_3D6F
	LD   A,$78
	SUB  (IY+$31)
	ADD  A,D
	CP   E
	JP   NC,.label_3D6F
	EXX 
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	EXX 
	DJNZ .label_3D4D
	RET 
.label_3D6F:
	CALL function_5634.label_565F
	LD   HL,data_AC7F
	CALL function_33B8
	LD   H,A
	LD   A,$18
	EX   DE,HL
	LD   HL,data_A8D0+4
	INC  (HL)
	LD   B,(HL)
	BIT  0,B
	JR   NZ,.label_3D87
	NEG  
.label_3D87:
	EX   DE,HL
	ADD  A,H
	EX   AF,AF'
	LD   B,(IY+$31)
	LD   C,(IY+$00)
	LD   A,(data_A8C6)
	CP   $01
	JP   Z,.label_3D9F
	db $3A
	db $E0
	db $A8
	db $A7
	db $CA
	db $CF
	db $3D
.label_3D9F:
	LD   IX,data_A840
	LD   IY,PlayerSprite_No+7
	LD   (IY+$31),B
	LD   (IY+$00),C
	EX   AF,AF'
	CALL function_59C5
	LD   (IX+$0A),E
	LD   (IX+$0B),D
	LD   (IX+$0C),C
	LD   (IX+$0D),B
	LD   (IY+$01),$4D
	LD   (IY+$30),$62
	DEC  (IX+$00)
	LD   A,(data_A8F6)
	LD   (FrameCountdown3),A
	RET 
	db $DD
	db $21
	db $E0
	db $A8
	db $FD
	db $21
	db $2C
	db $AA
	db $C3
	db $A7
	db $3D
; Function Name: function_3DDA
function_3DDA:
	LD   A,(data_AD04)
	DEC  A
	RET  NZ
	LD   IX,data_A8E0
	LD   IY,data_AA2A+2
	CALL function_3DEB
	RET 
; Function Name: function_3DEB
function_3DEB:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	db $3C
	db $C2
	db $FB
	db $3D
	db $CD
	db $05
	db $3E
	db $CD
	db $83
	db $2B
	db $D0
; Function Name: function_3DFB
function_3DFB:
	CALL function_40AB
	LD   A,(data_A8F6)
	LD   (IX+$0E),A
	RET 
; Function Name: function_3E05
function_3E05:
	LD   H,(IX+$0B)
	LD   L,(IX+$0A)
	LD   DE,(data_A808)
	ADD  HL,DE
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	ADD  HL,DE
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   H,(IX+$0D)
	LD   L,(IX+$0C)
	LD   DE,(data_A80A)
	ADD  HL,DE
	LD   D,(IY+$00)
	LD   E,(IX+$05)
	ADD  HL,DE
	LD   (IY+$00),H
	LD   (IX+$05),L
	RET 
; Function Name: function_3E36
function_3E36:
	LD   IX,data_A80A+6
	LD   IY,PlayerSprite_No+1
	CALL function_3E63
	LD   IX,FrameCountdown1+9
	LD   IY,PlayerSprite_No+3
	CALL function_3E63
	LD   IX,data_A827+9
	LD   IY,PlayerSprite_No+5
	CALL function_3E63
	LD   IX,data_A840
	LD   IY,PlayerSprite_No+7
	CALL function_3E63
	RET 
; Function Name: function_3E63
function_3E63:
	LD   A,(IX+$00)
	AND  A
	RET  Z
	INC  A
	JP   NZ,function_3E7E.label_3E8E
; Function Name: function_3E6C
function_3E6C:
	LD   A,(data_AD04)
	CP   $04
	CALL Z,function_3E7E
	CALL function_3E05
	CALL function_2B83
	RET  NC
	JP   function_40AB
; Function Name: function_3E7E
function_3E7E:
	db $3A
	db $80
	db $A9
	db $0F
	db $E6
	db $07
	db $C6
	db $40
	db $FD
	db $77
	db $01
	db $FD
	db $36
	db $30
	db $44
	db $C9
.label_3E8E:
	LD   A,(data_AD04)
	CP   $04
	JR   Z,.label_3E98
	JP   function_40AB
.label_3E98:
	db $DD
	db $7E
	db $00
	db $FE
	db $01
	db $CA
	db $AB
	db $40
	db $DD
	db $35
	db $00
	db $FE
	db $3C
	db $D4
	db $CB
	db $3E
	db $CD
	db $60
	db $2B
	db $DD
	db $7E
	db $00
	db $FE
	db $1C
	db $D8
	db $D6
	db $1C
	db $0F
	db $0F
	db $E6
	db $07
	db $21
	db $C3
	db $3E
	db $CF
	db $FD
	db $77
	db $01
	db $FD
	db $36
	db $30
	db $03
	db $C9
	db $FF
	db $E6
	db $E7
	db $E7
	db $E6
	db $E6
	db $E5
	db $E4
	db $DD
	db $36
	db $00
	db $3B
	db $C3
	db $83
	db $56
OutputString_Empty:
	dw $A692
	db $14
	db $B9
; Function Name: function_3ED6
function_3ED6:
	LD   A,(FrameCounter)
	AND  $07
	ADD  A,$05
	CP   (IX+$0F)
	RET  NZ
	LD   A,(FrameCountdown1)
	AND  A
	RET  NZ
	LD   HL,data_A80A+6
	LD   DE,PlayerSprite_No+1
	LD   A,(data_A844)
	AND  A
	RET  Z
	LD   B,A
.label_3EF2:
	LD   A,(HL)
	AND  A
	JR   Z,.label_3EFF
	LD   A,L
	ADD  A,$10
	LD   L,A
	INC  E
	INC  E
	DJNZ .label_3EF2
	RET 
.label_3EFF:
	LD   (data_A991),HL
	LD   (data_A993),DE
	LD   A,(data_A827)
	LD   B,A
	ADD  A,A
	LD   C,A
	LD   A,$78
	SUB  (IY+$31)
	ADD  A,B
	CP   C
	JR   NC,.label_3F1D
	LD   A,$84
	SUB  (IY+$00)
	ADD  A,B
	CP   C
	RET  C
.label_3F1D:
	LD   A,(data_A837)
	LD   B,A
	ADD  A,A
	LD   C,A
	LD   A,(PlayerAngle)
	SUB  (IX+$02)
	ADD  A,B
	CP   C
	RET  NC
	LD   A,D
	CP   $02
	JP   Z,function_3F93.label_3F9E
	LD   HL,data_AC7F
	CALL function_33B8
	LD   C,A
	SUB  (IX+$02)
	ADD  A,$10
	CP   $20
	RET  NC
	CALL function_3F93
	PUSH IX
	PUSH IY
	LD   D,(IY+$31)
	LD   E,(IY+$00)
	LD   IX,(data_A991)
	LD   IY,(data_A993)
	LD   (IY+$31),D
	LD   (IY+$00),E
	LD   A,(data_AD04)
	AND  A
	LD   A,C
	JR   NZ,.label_3F68
	CALL function_59CB
	JR   .label_3F6B
.label_3F68:
	CALL function_59D1
.label_3F6B:
	LD   (IX+$0A),E
	LD   (IX+$0B),D
	LD   (IX+$0C),C
	LD   (IX+$0D),B
	LD   A,(IY+$31)
	LD   A,(IY+$00)
	LD   (IY+$01),$4D
	LD   (IY+$30),$62
	LD   A,(data_A814)
	LD   (FrameCountdown1),A
	DEC  (IX+$00)
	POP  IY
	POP  IX
	RET 
; Function Name: function_3F93
function_3F93:
	LD   A,(data_AD04)
	CP   $03
	JP   C,function_5634.label_565F
	JP   function_5634.label_5669
.label_3F9E:
	db $3A
	db $E6
	db $A8
	db $47
	db $87
	db $4F
	db $3E
	db $84
	db $FD
	db $96
	db $00
	db $80
	db $B9
	db $D0
	db $C3
	db $32
	db $3F
; Function Name: function_3FAF
function_3FAF:
	LD   A,(IX+$02)
	ADD  A,$08
	RRCA
	RRCA
	RRCA
	RRCA
	AND  $0F
	LD   HL,function_3FAF+27
	RST  $08
	LD   (IY+$01),A
	LD   DE,RST10_AddDoubleAToHL
	ADD  HL,DE
	LD   A,(HL)
	LD   (IY+$30),A
	RET 
	db $48
	db $49
	db $4A
	db $4B
	db $4C
	db $4B
	db $4A
	db $49
	db $48
	db $49
	db $4A
	db $4B
	db $4C
	db $4B
	db $4A
	db $49
	db $F4
	db $B4
	db $B4
	db $B4
	db $B4
	db $34
	db $34
	db $34
	db $34
	db $74
	db $74
	db $74
	db $74
	db $F4
	db $F4
	db $F4
; Function Name: function_3FEA
function_3FEA:
	LD   A,(data_AD04)
	AND  A
	RET  NZ
	LD   IX,data_A8C0
	LD   IY,data_AA28
	LD   B,$03
.label_3FF9:
	LD   A,(IX+$00)
	AND  A
	JP   Z,.label_400B
	INC  A
	JR   NZ,.label_4008
	CALL function_4017
	JR   .label_400B
.label_4008:
	CALL function_406C
.label_400B:
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	DJNZ .label_3FF9
	RET 
; Function Name: function_4017
function_4017:
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	LD   A,(IX+$01)
	AND  A
	JR   Z,.label_4028
	LD   HL,function_FDC4+188
	JR   .label_402B
.label_4028:
	LD   HL,label_0174+12
.label_402B:
	ADD  HL,DE
	LD   DE,(data_A808)
	ADD  HL,DE
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   L,(IX+$07)
	LD   H,(IX+$08)
	LD   DE,RST08_AddAToHLAndGetContentsOfHL+1
	ADD  HL,DE
	LD   (IX+$07),L
	LD   (IX+$08),H
	LD   D,(IY+$00)
	LD   E,(IX+$05)
	ADD  HL,DE
	LD   DE,(data_A80A)
	ADD  HL,DE
	LD   (IY+$00),H
	LD   (IX+$05),L
	LD   A,(IY+$31)
	ADD  A,$10
	CP   $20
	JP   C,function_40AB
	LD   A,(IY+$00)
	CP   $F8
	JP   NC,function_40AB
	RET 
; Function Name: function_406C
function_406C:
	LD   A,(IX+$00)
	CP   $3C
	CALL NC,function_409D
	DEC  (IX+$00)
	JR   Z,function_40AB
	CALL label_2B52.label_2B60
	LD   A,(IX+$00)
	CP   $1C
	RET  C
	SUB  $1C
	RRCA
	RRCA
	AND  $0F
	LD   HL,function_406C+40
	RST  $08
	LD   (IY+$01),A
	LD   (IY+$30),$0E
	RET 
	db $FF
	db $9A
	db $99
	db $98
	db $98
	db $99
	db $99
	db $9A
	db $9B
; Function Name: function_409D
function_409D:
	LD   (IX+$00),$3B
	LD   A,(data_AD04)
	AND  A
	JP   Z,function_5683.label_568E
	JP   function_5683.label_568E
; Function Name: function_40AB
function_40AB:
	LD   (IX+$00),$00
	LD   (IY+$00),$00
	LD   (IY+$31),$00
	RET 
; Function Name: function_40B8
function_40B8:
	LD   A,(data_AD04)
	CP   $02
	RET  C
	LD   A,(FrameCounter)
	AND  $1F
	RET  NZ
	LD   A,(data_A8C0)
	INC  A
	RET  Z
	LD   A,(data_A8D0)
	INC  A
	RET  Z
	LD   A,(data_A8E0)
	INC  A
	RET  Z
	JP   function_5679
; Function Name: function_40D6
function_40D6:
	LD   A,(data_AD04)
	CP   $02
	RET  C
	LD   IX,data_A8C0
	LD   IY,data_AA28
	LD   A,(data_A8C6)
	AND  A
	RET  Z
	LD   B,A
.label_40EA:
	LD   A,(IX+$00)
	AND  A
	JP   Z,.label_410B
	INC  A
	JR   NZ,.label_4108
	LD   A,(data_AD04)
	CP   $04
	JP   Z,function_413C.label_4194
	LD   A,(IX+$0E)
	AND  A
	JP   NZ,function_413C.label_418B
	CALL function_4117
	JR   .label_410B
.label_4108:
	CALL function_413C
.label_410B:
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	DJNZ .label_40EA
	RET 
; Function Name: function_4117
function_4117:
	PUSH BC
	LD   A,(FrameCounter)
	AND  $0F
	CP   (IX+$0F)
	JR   NZ,.label_412B
	LD   HL,data_AC7F
	CALL function_33B8
	LD   (IX+$01),A
.label_412B:
	CALL function_4201
	CALL function_58AA
	CALL function_3FAF
	POP  BC
	CALL function_2B83
	RET  NC
	JP   function_40AB
; Function Name: function_413C
function_413C:
	LD   A,(IX+$00)
	CP   $3C
	CALL NC,function_409D
	CALL label_2B52.label_2B60
	DEC  (IX+$00)
	JP   Z,function_40AB
	LD   A,(IX+$00)
	CP   $1C
	RET  C
	SUB  $1C
	RRCA
	RRCA
	AND  $07
	LD   D,A
	LD   A,(data_AD04)
	CP   $04
	JR   NC,.label_4176
	LD   HL,function_413C+50
	LD   A,D
	RST  $08
	LD   (IY+$01),A
	LD   (IY+$30),$0D
	RET 
	db $FF
	db $9E
	db $9F
	db $9F
	db $9E
	db $9E
	db $9D
	db $9C
.label_4176:
	db $21
	db $83
	db $41
	db $7A
	db $CF
	db $FD
	db $77
	db $01
	db $FD
	db $36
	db $30
	db $02
	db $C9
	db $FF
	db $E2
	db $E3
	db $E3
	db $E2
	db $E2
	db $E1
	db $E0
.label_418B:
	CALL function_3E6C
	DEC  (IX+$0E)
	JP   function_40D6.label_410B
.label_4194:
	db $DD
	db $7E
	db $04
	db $A7
	db $CA
	db $A4
	db $41
	db $DD
	db $35
	db $04
	db $CD
	db $B8
	db $41
	db $C3
	db $0B
	db $41
	db $C5
	db $CD
	db $B6
	db $58
	db $CD
	db $F1
	db $41
	db $CD
	db $83
	db $2B
	db $C1
	db $D2
	db $0B
	db $41
	db $CD
	db $AB
	db $40
	db $C3
	db $0B
	db $41
	db $C5
	db $3A
	db $80
	db $A9
	db $E6
	db $0F
	db $20
	db $1F
	db $21
	db $75
	db $AC
	db $DD
	db $CB
	db $0F
	db $46
	db $20
	db $03
	db $21
	db $79
	db $AC
	db $CD
	db $B8
	db $33
	db $47
	db $7A
	db $FE
	db $10
	db $30
	db $07
	db $08
	db $FE
	db $10
	db $DC
	db $EC
	db $41
	db $08
	db $DD
	db $70
	db $01
	db $CD
	db $1F
	db $42
	db $CD
	db $B6
	db $58
	db $CD
	db $F1
	db $41
	db $C1
	db $C3
	db $83
	db $2B
	db $DD
	db $36
	db $04
	db $00
	db $C9
	db $3A
	db $80
	db $A9
	db $0F
	db $E6
	db $07
	db $C6
	db $50
	db $FD
	db $77
	db $01
	db $FD
	db $36
	db $30
	db $0A
	db $C9
; Function Name: function_4201
function_4201:
	LD   A,(IX+$01)
	SUB  (IX+$02)
	ADD  A,$01
	CP   $02
	RET  C
	CP   $80
	LD   A,(IX+$02)
	JR   NC,.label_4219
	ADD  A,$01
	LD   (IX+$02),A
	RET 
.label_4219:
	SUB  $01
	LD   (IX+$02),A
	RET 
	db $3A
	db $80
	db $A9
	db $E6
	db $03
	db $C8
	db $DD
	db $7E
	db $01
	db $DD
	db $96
	db $02
	db $C6
	db $01
	db $FE
	db $02
	db $D8
	db $FE
	db $80
	db $DD
	db $7E
	db $02
	db $30
	db $06
	db $C6
	db $02
	db $DD
	db $77
	db $02
	db $C9
	db $D6
	db $02
	db $DD
	db $77
	db $02
	db $C9
; Function Name: function_4243
function_4243:
	LD   A,(FrameCounter)
	AND  $07
	ADD  A,$05
	CP   (IX+$0F)
	RET  NZ
	LD   HL,FrameCountdown3
	LD   A,(HL)
	AND  A
	JR   Z,.label_4257
	DEC  (HL)
	RET 
.label_4257:
	LD   HL,data_A8C0
	LD   DE,data_AA28
	LD   A,(data_A8C6)
	AND  A
	RET  Z
	LD   B,A
.label_4263:
	LD   A,(HL)
	AND  A
	JP   Z,.label_4271
	LD   A,L
	ADD  A,$10
	LD   L,A
	INC  DE
	INC  DE
	DJNZ .label_4263
	RET 
.label_4271:
	LD   (data_A991),HL
	LD   (data_A993),DE
	LD   A,(data_A8D6)
	LD   D,A
	ADD  A,A
	LD   C,A
	LD   A,$78
	SUB  (IY+$31)
	ADD  A,D
	CP   C
	JR   NC,.label_428F
	LD   A,$84
	SUB  (IY+$00)
	ADD  A,D
	CP   C
	RET  C
.label_428F:
	LD   C,(IX+$02)
	LD   A,(data_AD04)
	AND  A
	JP   Z,.label_429C
	JP   .label_42B7
.label_429C:
	LD   A,(data_A8E6)
	LD   D,A
	ADD  A,A
	LD   C,A
	LD   A,$84
	SUB  (IY+$00)
	ADD  A,D
	CP   C
	RET  NC
	LD   A,$78
	SUB  (IY+$31)
	JR   C,.label_42B5
	LD   C,$00
	JR   .label_42B7
.label_42B5:
	LD   C,$01
.label_42B7:
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	LD   H,(IY+$00)
	LD   L,(IX+$05)
	EXX 
	PUSH IX
	PUSH IY
	LD   IX,(data_A991)
	LD   IY,(data_A993)
	EXX 
	LD   (IX+$03),E
	LD   (IY+$31),D
	LD   (IX+$05),L
	LD   (IY+$00),H
	LD   (IX+$01),C
	LD   A,(data_AD04)
	CP   $04
	JP   Z,.label_43AE
	AND  A
	JP   NZ,.label_4313
	LD   (IY+$01),$4F
	LD   A,C
	RRCA
	SRA  A
	AND  $C0
	ADD  A,$0B
	LD   (IY+$30),A
	LD   (IX+$07),$00
	LD   (IX+$08),$FF
	DEC  (IX+$00)
	LD   A,(data_A8F6)
	LD   (FrameCountdown3),A
	POP  IY
	POP  IX
	JP   function_5634.label_5664
.label_4313:
	LD   A,(data_AD04)
	CP   $03
	JP   Z,.label_436F
	JP   NC,.label_434C
	LD   HL,data_AC7F
	CALL function_33B8
	LD   (IX+$01),A
	LD   A,(IX+$0F)
	RRCA
	AND  $80
	ADD  A,$40
	ADD  A,(IX+$01)
	LD   (IX+$02),A
	CALL function_3FAF
	DEC  (IX+$00)
	LD   A,(data_A8F6)
	LD   (FrameCountdown3),A
	LD   (IX+$0E),$00
	POP  IY
	POP  IX
	JP   function_5634.label_566E
.label_434C:
	db $21
	db $7F
	db $AC
	db $CD
	db $B8
	db $33
	db $DD
	db $77
	db $01
	db $DD
	db $77
	db $02
	db $CD
	db $AF
	db $3F
	db $DD
	db $35
	db $00
	db $3A
	db $F6
	db $A8
	db $32
	db $F4
	db $A8
	db $DD
	db $36
	db $0E
	db $00
	db $FD
	db $E1
	db $DD
	db $E1
	db $C3
	db $74
	db $56
.label_436F:
	PUSH BC
	LD   A,C
	ADD  A,$40
	AND  $80
	LD   A,C
	JR   NZ,.label_437F
	ADD  A,$1A
	LD   (IX+$02),A
	JR   .label_4384
.label_437F:
	SUB  $1A
	LD   (IX+$02),A
.label_4384:
	CALL function_598E
	LD   (IX+$0A),E
	LD   (IX+$0B),D
	LD   (IX+$0C),C
	LD   (IX+$0D),B
	POP  BC
	LD   (IX+$02),C
	CALL function_3FAF
	LD   (IX+$0E),$20
	DEC  (IX+$00)
	LD   A,(data_A8F6)
	LD   (FrameCountdown3),A
	POP  IY
	POP  IX
	JP   function_5634.label_566E
.label_43AE:
	db $3A
	db $E6
	db $A8
	db $DD
	db $77
	db $04
	db $C3
	db $13
	db $43
; Function Name: function_43B7
function_43B7:
	LD   A,(data_ACC6)
	INC  A
	RET  Z
	LD   A,(data_AD0D)
	AND  A
	JR   NZ,.label_43F0
	LD   A,(FrameCounter)
	AND  $07
	CP   $05
	RET  NZ
	LD   IX,data_A8A0
	LD   IY,data_AA24
	LD   A,(RemainingShipsInWave)
	OR   (IX+$00)
	OR   (IX+$10)
	RET  NZ
	LD   A,$FF
	LD   (data_AD0D),A
	LD   (IX+$04),$07
	JP   function_46BA.label_46DB
.label_43E8:
	XOR  A
.label_43E9:
	ADD  A,(HL)
	INC  HL
	DJNZ .label_43E9
	JP   Phase4Function.label_07AD
.label_43F0:
	LD   IX,data_A8A0
	LD   IY,data_AA24
	LD   A,(IX+$00)
	AND  A
	JP   Z,function_4447.label_4535
	INC  A
	JP   NZ,function_4447.label_4540
.label_4403:
	LD   H,(IX+$0C)
	LD   L,(IX+$0D)
	LD   DE,(data_A808)
	ADD  HL,DE
	LD   D,(IY+$31)
	LD   E,(IX+$03)
	ADD  HL,DE
	LD   (IY+$31),H
	LD   (IX+$03),L
	LD   H,(IX+$1C)
	LD   L,(IX+$1D)
	LD   DE,(data_A80A)
	ADD  HL,DE
	LD   D,(IY+$00)
	LD   E,(IX+$05)
	ADD  HL,DE
	LD   (IY+$00),H
	LD   (IX+$05),L
	LD   A,(IY+$31)
	ADD  A,$10
	LD   (IY+$33),A
	LD   A,(IY+$00)
	LD   (IY+$02),A
	CALL function_4447
	JP   function_46BA.label_46F0
; Function Name: function_4447
function_4447:
	CALL function_3CC4
	JP   C,function_46BA.label_46DB
	LD   A,(data_AD04)
	LD   D,A
	CP   $04
	JP   Z,.label_44A2
	LD   A,D
	ADD  A,A
	ADD  A,A
	ADD  A,A
	ADD  A,A
	LD   B,A
	LD   A,(FrameCounter)
	AND  $02
	ADD  A,B
	LD   B,A
	LD   A,$07
	SUB  (IX+$04)
	RRCA
	AND  $03
	LD   E,A
	ADD  A,A
	ADD  A,A
	ADD  A,B
	LD   HL,.label_44A2+79
	RST  $18
	LD   B,(HL)
	INC  HL
	LD   C,(HL)
	LD   HL,.label_44A2+143
	LD   A,D
	RST  $18
	LD   D,(HL)
	LD   A,(IX+$02)
	ADD  A,$40
	CP   $80
	JR   C,.label_4495
	LD   (IY+$01),B
	LD   (IY+$03),C
	LD   A,D
	ADD  A,$80
	LD   (IY+$30),A
	LD   (IY+$32),A
	RET 
.label_4495:
	LD   (IY+$01),C
	LD   (IY+$03),B
	LD   (IY+$30),D
	LD   (IY+$32),D
	RET 
.label_44A2:
	db $DD
	db $7E
	db $04
	db $5F
	db $FE
	db $07
	db $CA
	db $BF
	db $44
	db $DD
	db $34
	db $06
	db $DD
	db $4E
	db $06
	db $CB
	db $79
	db $20
	db $14
	db $7B
	db $C6
	db $02
	db $B9
	db $30
	db $04
	db $DD
	db $36
	db $06
	db $80
	db $FD
	db $36
	db $30
	db $70
	db $FD
	db $36
	db $32
	db $70
	db $18
	db $13
	db $79
	db $E6
	db $7F
	db $FE
	db $03
	db $38
	db $04
	db $DD
	db $36
	db $06
	db $00
	db $FD
	db $36
	db $30
	db $51
	db $FD
	db $36
	db $32
	db $51
	db $11
	db $02
	db $02
	db $21
	db $D5
	db $D4
	db $3A
	db $80
	db $A9
	db $CB
	db $57
	db $20
	db $01
	db $19
	db $FD
	db $75
	db $01
	db $FD
	db $74
	db $03
	db $C9
	db $39
	db $38
	db $39
	db $38
	db $3B
	db $3A
	db $3D
	db $3C
	db $3B
	db $3A
	db $3D
	db $3C
	db $3D
	db $3C
	db $3F
	db $3E
	db $B0
	db $B1
	db $B2
	db $B3
	db $B4
	db $B5
	db $B6
	db $B7
	db $B8
	db $B9
	db $BA
	db $BB
	db $BC
	db $BD
	db $BE
	db $BF
	db $C0
	db $C1
	db $C2
	db $C3
	db $C4
	db $C5
	db $C6
	db $C7
	db $C6
	db $C7
	db $C8
	db $C9
	db $C8
	db $C9
	db $CA
	db $CB
	db $CC
	db $CD
	db $CC
	db $CD
	db $CE
	db $CF
	db $D0
	db $D1
	db $CE
	db $CF
	db $D0
	db $D1
	db $D0
	db $D1
	db $D2
	db $D3
	db $E9
	db $58
	db $6F
	db $6E
.label_4535:
	LD   A,(IX+$0E)
	AND  A
	JP   Z,OutputString_FirstBonus20000.label_4663
	DEC  (IX+$0E)
	RET 
.label_4540:
	LD   C,A
	LD   A,(IX+$04)
	AND  A
	JR   Z,.label_4554
	DEC  (IX+$04)
	LD   (IX+$00),$FF
	CALL function_5683
	JP   function_43B7.label_4403
.label_4554:
	LD   A,C
	CP   $F0
	JP   NZ,OutputString_FirstBonus20000.label_45B3
	XOR  A
	LD   (data_A8DC),A
	CALL function_5634
	CALL function_56D2
	LD   HL,data_A80A+6
	LD   DE,RST10_AddDoubleAToHL
	LD   B,$0F
	LD   C,$14
.label_456E:
	LD   A,(HL)
	INC  A
	JR   NZ,.label_4594
	LD   (HL),C
	EXX 
	LD   DE,CopySpriteDataToHW+157
	RST  $38
	EXX 
.label_4579:
	ADD  HL,DE
	LD   A,C
	ADD  A,$0A
	LD   C,A
	DJNZ .label_456E
	LD   C,$3C
	LD   A,$FE
	LD   (data_ACC6),A
	LD   (IX+$00),$E4
	LD   (IY+$30),$3D
	LD   (IY+$32),$3D
	RET 
.label_4594:
	INC  A
	JR   NZ,.label_4579
	db $36
	db $00
	db $18
	db $DE
; Function Name: OutputString_FirstBonus20000
OutputString_FirstBonus20000:
	dw $A716
	db $13
	db $96
	db $ED
	db $DC
	db $F1
	db $8C
	db $68
	db $3B
	db $0D
	db $ED
	db $F1
	db $9B
	db $13
	db $13
	db $13
	db $13
	db $F1
	db $88
	db $DC
	db $ED
	db $11
	db $B9
.label_45B3:
	CALL label_2B52.label_2B60
	LD   A,(IY+$31)
	LD   B,A
	ADD  A,$13
	CP   $03
	JR   C,.label_45D5
	LD   A,B
	ADD  A,$10
	LD   (IY+$33),A
	LD   A,(IY+$00)
	LD   B,A
	ADD  A,$08
	CP   $28
	JR   C,.label_45D5
	LD   (IY+$02),B
	JR   .label_45DD
.label_45D5:
	LD   (IY+$01),$FF
	LD   (IY+$03),$FF
.label_45DD:
	LD   A,(IX+$00)
	CP   $B4
	JR   Z,.label_4623
	JR   C,.label_45F9
	SUB  $B4
	RRCA
	RRCA
	RRCA
	DEC  A
	AND  $07
	LD   HL,.label_45F9+34
	RST  $08
	LD   (IY+$03),A
	INC  A
	LD   (IY+$01),A
.label_45F9:
	DEC  (IX+$00)
	JP   Z,.label_4646
	LD   A,(IX+$00)
	CP   $5A
	RET  NZ
	LD   (IY+$01),$FF
	LD   (IY+$03),$FF
	RET 
	db $21
	db $7C
	db $A6
	db $7E
	db $4F
	db $3A
	db $43
	db $AB
	db $91
	db $C2
	db $43
	db $46
	db $C9
	db $94
	db $96
	db $96
	db $94
	db $92
	db $90
	db $90
	db $94
.label_4623:
	DEC  (IX+$00)
	LD   (IY+$01),$FE
	LD   (IY+$03),$FD
	LD   (IY+$30),$6C
	LD   (IY+$32),$6C
	LD   A,(data_A800)
	INC  A
	CALL Z,function_580B
	LD   DE,CopySpriteDataToHW+168
	JP   RST38_QueueCommand
	db $C3
	db $1B
	db $46
.label_4646:
	LD   A,$FF
	LD   (data_ACC6),A
	LD   (IX+$00),$00
	LD   HL,data_AB43
	LD   A,(HL)
	CP   $7C
	JP   NZ,.label_4660
	INC  HL
	LD   A,(HL)
	CP   $10
	RET  Z
	CP   $05
	RET  Z
.label_4660:
	db $C3
	db $9B
	db $45
.label_4663:
	LD   A,(data_ACC6)
	AND  A
	RET  NZ
	LD   A,(PlayerAngle)
	LD   B,A
	LD   A,(FrameCounter)
	LD   C,A
	LD   A,$10
	BIT  3,C
	JR   NZ,.label_4678
	NEG  
.label_4678:
	ADD  A,B
	RRCA
	RRCA
	AND  $3E
	LD   HL,function_3C0D.label_3C75+15
	RST  $08
	LD   (IY+$31),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$00),A
	LD   A,B
	ADD  A,$C0
	AND  $80
	LD   (IX+$02),A
	CALL function_46BA
	LD   A,(IX+$04)
	CP   $06
	JR   NC,.label_469F
	db $DD
	db $36
	db $04
	db $05
.label_469F:
	LD   (IX+$00),$FF
	JP   function_57F1.label_57F7
	db $3A
	db $80
	db $A9
	db $4F
	db $E6
	db $1C
	db $CB
	db $41
	db $20
	db $02
	db $ED
	db $44
	db $80
	db $0F
	db $0F
	db $E6
	db $3E
	db $C3
	db $7D
	db $46
; Function Name: function_46BA
function_46BA:
	LD   HL,function_46BA+20
	PUSH HL
	LD   A,(data_AD04)
	AND  $07
	RST  $30
	db $42
	db $59
	db $4E
	db $59
	db $4E
	db $59
	db $65
	db $59
	db $6B
	db $59
	LD   (IX+$0C),D
	LD   (IX+$0D),E
	LD   (IX+$1C),B
	LD   (IX+$1D),C
	RET 
.label_46DB:
	XOR  A
	LD   (IX+$00),A
	LD   (IY+$00),A
	LD   (IY+$02),A
	LD   (IY+$31),A
	LD   (IY+$33),A
	LD   (IX+$0E),$5F
	RET 
.label_46F0:
	LD   A,(IX+$00)
	INC  A
	RET  NZ
	LD   A,(FrameCountdown1)
	AND  A
	RET  NZ
	LD   B,$02
	LD   A,(data_A827)
	LD   D,A
	ADD  A,A
	LD   E,A
.label_4702:
	LD   A,(IY+$00)
	ADD  A,$08
	CP   $28
	JR   C,.label_4726
	LD   A,(IY+$31)
	ADD  A,$10
	CP   $20
	JR   C,.label_4726
	LD   A,$84
	SUB  (IY+$00)
	ADD  A,D
	CP   E
	JR   NC,.label_4734
	LD   A,$78
	SUB  (IY+$31)
	ADD  A,D
	CP   E
	JR   NC,.label_4734
.label_4726:
	EXX 
	LD   DE,RST10_AddDoubleAToHL
	ADD  IX,DE
	INC  IY
	INC  IY
	EXX 
	DJNZ .label_4702
	RET 
.label_4734:
	LD   HL,data_A827+9
	EXX 
	LD   HL,PlayerSprite_No+5
	LD   B,$02
.label_473D:
	EXX 
	LD   A,(HL)
	AND  A
	JR   Z,.label_474C
	LD   DE,RST10_AddDoubleAToHL
	ADD  HL,DE
	EXX 
	INC  HL
	INC  HL
	DJNZ .label_473D
	RET 
.label_474C:
	LD   (data_A991),HL
	EXX 
	LD   (data_A993),HL
	CALL function_5634.label_565F
	LD   HL,data_AC7F
	CALL function_33B8
	LD   H,A
	EX   DE,HL
	LD   HL,data_A8A0+20
	INC  (HL)
	LD   A,$18
	BIT  0,(HL)
	JR   NZ,.label_476A
	NEG  
.label_476A:
	EX   DE,HL
	ADD  A,H
	LD   B,(IY+$31)
	LD   C,(IY+$00)
	LD   IX,(data_A991)
	LD   IY,(data_A993)
	LD   (IX+$02),A
	LD   (IY+$31),B
	LD   (IY+$00),C
	LD   HL,.label_476A+43
	PUSH HL
	LD   A,(data_AD04)
	RST  $30
	db $8E
	db $59
	db $8E
	db $59
	db $94
	db $59
	db $94
	db $59
	db $94
	db $59
	LD   (IX+$0A),E
	LD   (IX+$0B),D
	LD   (IX+$0C),C
	LD   (IX+$0D),B
	LD   (IY+$01),$4D
	LD   (IY+$30),$62
	DEC  (IX+$00)
	LD   A,(data_A814)
	LD   (FrameCountdown1),A
	RET 
; Function Name: function_47B3
function_47B3:
	LD   A,(data_AD04)
	CP   $04
	RET  Z
	LD   IX,data_A8E6+10
	LD   IY,data_AA2A+4
	LD   A,(IX+$00)
	AND  A
	JP   Z,.label_4853
	INC  A
	JP   NZ,.label_47F2
	CALL function_3E05
	CALL function_2B83
	JP   C,function_48AD
	LD   A,(FrameCounter)
	RRCA
	RRCA
	RRCA
	RRCA
	AND  $07
	LD   HL,function_47B3+55
	RST  $08
	LD   (IY+$01),A
	LD   (IY+$30),$75
	RET 
	db $00
	db $01
	db $02
	db $03
	db $03
	db $02
	db $01
	db $00
.label_47F2:
	CALL label_2B52.label_2B60
	LD   A,(IX+$00)
	CP   $10
	JP   Z,.label_4831
	CP   $3C
	JP   NC,.label_4809
	DEC  (IX+$00)
	RET  NZ
	JP   function_48AD
.label_4809:
	LD   (IX+$00),$3B
	CALL function_57FF
	LD   A,(IX+$07)
	CP   $04
	JP   NC,.label_4824
	LD   HL,.label_4824+9
	RST  $08
	LD   (IY+$01),A
	LD   (IY+$30),$6C
	RET 
.label_4824:
	db $FD
	db $36
	db $01
	db $8F
	db $FD
	db $36
	db $30
	db $6C
	db $C9
	db $F9
	db $FC
	db $8D
	db $8E
.label_4831:
	DEC  (IX+$00)
	LD   A,(IX+$07)
	INC  (IX+$07)
	CP   $04
	JP   NC,.label_4849
	LD   HL,.label_4849+6
	RST  $18
	LD   E,(HL)
	LD   D,$04
	JP   RST38_QueueCommand
.label_4849:
	db $11
	db $0F
	db $04
	db $C3
	db $38
	db $00
	db $0A
	db $0C
	db $0D
	db $0E
.label_4853:
	LD   A,(data_AD0D)
	AND  A
	RET  NZ
	LD   A,(FrameCounter)
	AND  $01
	RET  Z
	DEC  (IX+$0E)
	RET  NZ
	LD   A,(PlayerAngle)
	ADD  A,$08
	RRCA
	RRCA
	RRCA
	AND  $1E
	LD   HL,.label_4853+58
	RST  $08
	LD   (IY+$31),A
	INC  HL
	LD   A,(HL)
	LD   (IY+$00),A
	LD   (IX+$0A),$00
	LD   (IX+$0B),$00
	LD   (IX+$0C),$40
	LD   (IX+$0D),$00
	LD   (IX+$00),$FF
	RET 
	db $F0
	db $40
	db $F0
	db $80
	db $F0
	db $F8
	db $60
	db $F8
	db $80
	db $F8
	db $A0
	db $F8
	db $10
	db $F8
	db $00
	db $80
	db $00
	db $90
	db $10
	db $10
	db $30
	db $10
	db $60
	db $10
	db $80
	db $10
	db $A0
	db $10
	db $C0
	db $10
	db $F0
	db $28
; Function Name: function_48AD
function_48AD:
	LD   (IX+$00),$00
	LD   (IY+$00),$00
	LD   (IY+$31),$00
	LD   (IX+$0E),$F0
	RET 
; Function Name: function_48BE
function_48BE:
	CALL function_48E7
	CALL function_4941
	CALL function_4911
	CALL function_4984
	CALL function_49D6
	RET 
OutputString_InputYourInitials:
	dw $A72C
	db $13
	db $FD
	db $3B
	db $88
	db $0D
	db $DC
	db $F1
	db $BF
	db $68
	db $0D
	db $D7
	db $F1
	db $FD
	db $3B
	db $FD
	db $DC
	db $FD
	db $A5
	db $57
	db $ED
	db $F1
	db $52
	db $B9
; Function Name: function_48E7
function_48E7:
	LD   A,(Input0)
	RRCA			; shift down 3
	RRCA
	RRCA
	LD   HL,data_A983
	RL   (HL)			; rotate carry bit (bit 2 of Input 0) into number at address
	LD   A,(HL)
	AND  %00000111
	CP   $01			; check if new bit has been added
	RET  NZ			; exit if not
	CALL function_57F1
	LD   C,$01
	JP   function_4941.label_496E
OutputString_KonamiCopyright2:
	dw $A6BC
data_4901:
data_4902:
	db $05
data_4903:
	db $30
	db $F1
	db $7C
	db $68
	db $3B
	db $A5
	db $38
	db $FD
	db $F1
	db $96
	db $5D
	db $17
	db $9B
	db $B9
; Function Name: function_4911
function_4911:
	LD   A,(Input0)
	LD   HL,data_A9C9+1
	RRCA
	RRCA
	RL   (HL)
	LD   A,(HL)
	AND  $07
	CP   $01
	RET  NZ
	db $EB
	db $CD
	db $F1
	db $57
	db $21
	db $82
	db $A9
	db $34
	db $EB
	db $23
	db $7E
	db $C6
	db $10
	db $77
	db $47
	db $23
	db $7E
	db $90
	db $D0
	db $7E
	db $4F
	db $E6
	db $F0
	db $C6
	db $10
	db $2B
	db $ED
	db $44
	db $86
	db $77
	db $18
	db $2D
; Function Name: function_4941
function_4941:
	LD   A,(Input0)
	LD   HL,data_A9C6+1
	RRCA
	RL   (HL)
	LD   A,(HL)
	AND  $07
	CP   $01
	RET  NZ
	db $EB
	db $CD
	db $F1
	db $57
	db $21
	db $81
	db $A9
	db $34
	db $EB
	db $23
	db $7E
	db $C6
	db $10
	db $77
	db $47
	db $23
	db $7E
	db $90
	db $D0
	db $7E
	db $4F
	db $E6
	db $F0
	db $C6
	db $10
	db $2B
	db $ED
	db $44
	db $86
	db $77
.label_496E:
	LD   A,(data_A9C0)
	AND  A
	JR   NZ,function_4984
	LD   A,C
	AND  $0F
	LD   HL,Credits
	ADD  A,(HL)
	DAA
	LD   (HL),A
	JR   NC,data_4980.label_4981
	db $36
data_4980:
	db $99
.label_4981:
	CALL function_4AFB
; Function Name: function_4984
function_4984:
	LD   A,(data_A981)
	AND  A
	RET  Z
	LD   HL,data_A983+1
	LD   A,(HL)
	AND  A
	JR   NZ,.label_4997
	LD   (HL),$30
	INC  A
	LD   (data_C30A),A
	RET 
.label_4997:
	DEC  (HL)
	JR   Z,.label_49A3
	LD   A,(HL)
	CP   $18
	RET  NZ
	XOR  A
	LD   (data_C30A),A
	RET 
.label_49A3:
	LD   HL,data_A981
	DEC  (HL)
	RET 
; Function Name: TestGrid
TestGrid:
	RRCA
	LD   C,A
	AND  $07
	LD   (data_A9C4),A
	LD   A,C
	RRCA
	RRCA
	RRCA
	AND  $01
	LD   (data_A9C6),A
	LD   (WatchdogReset_DipSW2),A
	LD   A,(ClearString.operand_0C3C+2)
	LD   (HWReg_FlipScreen),A			; Flip Screen
	CALL DrawTestGrid
	LD   B,$00
	LD   HL,Phase0Function.operand_27CA+20
	XOR  A
.checksum_loop:
	ADD  A,(HL)
	INC  HL
	DJNZ .checksum_loop
	SUB  $C5
	CALL NZ,InterruptRoutine
	JP   label_32EB
; Function Name: function_49D6
function_49D6:
	LD   A,(data_A982)
	AND  A
	RET  Z
	LD   HL,data_A985
	LD   A,(HL)
	AND  A
	JR   NZ,.label_49E9
	LD   (HL),$30
	INC  A
	LD   (data_C30C),A
	RET 
.label_49E9:
	DEC  (HL)
	JR   Z,.label_49F5
	LD   A,(HL)
.operand_49ED:
	CP   $18
	RET  NZ
	XOR  A
	LD   (data_C30C),A
	RET 
.label_49F5:
	LD   HL,data_A982
	DEC  (HL)
	RET 
OutputString_AndTryThisGame:
	db $EE
	db $A6
	db $14
	db $A5
	db $3B
	db $87
	db $F1
	db $DC
	db $D7
	db $BF
	db $F1
	db $DC
	db $C4
	db $FD
	db $ED
	db $F1
	db $7D
	db $A5
	db $38
	db $34
	db $B9
; Function Name: PhaseDFunction
PhaseDFunction:
	LD   A,(function_31B4.operand_3210+3)
	LD   (data_A9F0),A
	LD   A,$00
	LD   (data_A9F1),A
	LD   A,$FF
	LD   (data_A9F2),A
	LD   A,$04
	LD   (data_A9F3),A
	LD   A,$FF
	LD   (data_A9F4),A
	LD   A,$08
	LD   (data_A9F6),A
	LD   HL,function_56D2+31
	LD   (data_A9F7),HL
	LD   B,$0D
.operand_4A36:
	LD   HL,VideoRAM
	LD   C,$14
.label_4A3B:
	LD   (HL),C
	INC  HL
	DJNZ .label_4A3B
.operand_4A3F:
	LD   A,$00
	LD   (HL),A
.label_4A42:
	INC  HL
	LD   (HL),A
	INC  HL
	LD   B,$0D
.label_4A47:
	LD   (HL),C
	INC  HL
	DJNZ .label_4A47
	LD   A,$0E
	LD   B,$04
.label_4A4F:
	LD   (HL),A
	INC  HL
	DJNZ .label_4A4F
	LD   HL,VideoRAM+945
	RES  2,H
	LD   A,(CharacterAttrib)
	LD   C,A
	LD   A,$A0
	ADD  A,C
	CALL function_1319
	LD   HL,VideoRAM+465
	RES  2,H
	LD   A,$20
	ADD  A,C
	CALL function_1319
	LD   HL,VideoRAM+528
	RES  2,H
	LD   A,$A0
	ADD  A,C
	LD   (HL),A
	ADD  HL,DE
	LD   A,$20
	ADD  A,C
	LD   (HL),A
	LD   HL,VideoRAM+530
	RES  2,H
	LD   A,$E0
	ADD  A,C
	LD   (HL),A
	ADD  HL,DE
	LD   A,$60
	ADD  A,C
	LD   (HL),A
	LD   HL,VideoRAM+529
	RES  2,H
	LD   A,$A0
	ADD  A,C
	LD   (HL),A
	ADD  HL,DE
	LD   A,$20
	ADD  A,C
	LD   (HL),A
	CALL function_339C
	JP   AdvancePhase
	db $06
	db $0D
	db $2A
data_4AA0:
	db $F7
	db $A9
	db $7E
	db $A7
	db $EB
	db $28
	db $09
	db $7E
	db $3C
	db $CB
	db $41
	db $28
	db $02
	db $3D
	db $3D
	db $77
	db $CB
	db $49
	db $11
	db $20
	db $00
	db $28
	db $03
	db $11
	db $E0
	db $FF
	db $19
	db $EB
	db $2A
	db $F7
	db $A9
	db $23
	db $CB
	db $41
	db $28
	db $02
	db $2B
	db $2B
	db $22
	db $F7
	db $A9
	db $10
	db $D7
	db $C9
; Function Name: function_4ACC
function_4ACC:
	LD   A,(DipSwitch1)
	AND  $0F
	CP   $0F
	JR   NZ,.label_4ADA
	LD   HL,data_A9C0
	LD   (HL),$FF
.label_4ADA:
	LD   HL,data_4B95
	RST  $08
	LD   (data_A9C9),A
	LD   A,(DipSwitch1)
	RRCA
	RRCA
	RRCA
	RRCA
	AND  $0F
	CP   $0F
	JR   NZ,.label_4AF3
	LD   HL,data_A9C0
	LD   (HL),$FF
.label_4AF3:
	LD   HL,data_4B95
	RST  $08
	LD   (data_A9CC),A
	RET 
; Function Name: function_4AFB
function_4AFB:
	LD   C,$10
	LD   DE,VideoRAM+127
	LD   HL,Credits
	CALL function_0D81
	RET 
	db $2A
	db $41
	db $AB
	db $7D
	db $AC
	db $2F
	db $87
	db $87
	db $ED
	db $6A
	db $22
	db $41
	db $AB
	db $ED
	db $5F
	db $85
	db $AC
	db $C9
; Function Name: label_4B19
label_4B19:
	LD   DE,FunctionTable+16
	LD   BC,ResetSpritesAndRAM.sprite_clear_loop2+11
	LD   A,(function_19F0.operand_1A50)
	LD   H,A
.label_4B23:
	LD   A,(DE)
	ADD  A,C
	LD   C,A
	INC  DE
	DJNZ .label_4B23
	SUB  H
	CALL NZ,function_0F11
	JP   AdvancePhase
; Function Name: function_4B30
function_4B30:
	LD   HL,function_0C90.label_0D0A+17
	LD   B,$03
.label_4B35:
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  HL
	LD   A,(DE)
	EX   AF,AF'
	LD   A,$04
	ADD  A,D
	LD   D,A
	LD   A,(DE)
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	INC  HL
	LD   (DE),A
	INC  E
	EX   AF,AF'
	LD   (DE),A
	DJNZ .label_4B35
	RET 
; Function Name: function_4B4B
function_4B4B:
	EXX 
	LD   HL,data_AB37+8
	LD   DE,data_AB37+9
data_4B52:
	LD   BC,RST10_AddDoubleAToHL
	LDDR
	LD   HL,data_AB37+9
	LD   A,(data_AB37)
	XOR  (HL)
	LD   (data_AB30),A
	LD   HL,FrameCounter
	ADD  A,(HL)
	EXX 
	RET 
; Function Name: function_4B67
function_4B67:
	LD   HL,data_4B84			; ROM
	LD   DE,data_AB30			; RAM
	LD   BC,RST10_AddDoubleAToHL+1
	LDIR
	LD   IX,(data_086D)
	LD   HL,(data_0870)
	LD   A,IXL
	ADD  A,IXH
	ADD  A,L
	ADD  A,$44
	JP   NZ,data_6000
	RET 
data_4B84:
	db $FF
	db $05
	db $F6
	db $80
	db $32
	db $17
	db $9C
	db $C9
	db $DD
	db $21
	db $74
	db $98
	db $FD
	db $BF
	db $24
	db $AE
	db $46
data_4B95:
	db $01
	db $02
	db $03
	db $04
	db $05
	db $06
	db $07
	db $11
	db $13
	db $15
	db $21
	db $22
	db $24
	db $31
	db $33
	db $01
; Function Name: function_4BA5
function_4BA5:
	LD   HL,data_4BB1			; ROM
	LD   DE,data_AB08			; RAM
	LD   BC,40			; 40 bytes
	LDIR
	RET 
data_4BB1:
	db $00
	db $00
	db $00
	db $01
	db $7C
	db $11
	db $68
	db $F1
	db $01
	db $00
	db $88
	db $00
	db $3B
	db $11
	db $A5
	db $F1
	db $02
	db $60
	db $84
	db $00
	db $38
	db $11
	db $FD
	db $F1
	db $03
	db $20
	db $65
	db $00
	db $68
	db $11
	db $68
	db $F1
	db $04
	db $00
	db $43
	db $00
	db $BF
	db $11
	db $A5
	db $F1
; Function Name: function_4BD9
function_4BD9:
	JP   EnemyTallyChars.label_08AE
; Function Name: function_4BDC
function_4BDC:
	LD   HL,data_AB08
	LD   DE,VideoRAM+785
	LD   C,$14
	CALL function_4C1F
	LD   HL,data_AB08+8
	LD   DE,VideoRAM+787
	LD   C,$16
	CALL function_4C1F
	LD   HL,data_AB08+16
	LD   DE,VideoRAM+789
	LD   C,$12
	CALL function_4C1F
	LD   HL,data_AB08+24
	LD   DE,VideoRAM+791
	LD   C,$15
	CALL function_4C1F
	LD   HL,data_AB08+32
	LD   DE,VideoRAM+793
	LD   C,$13
	CALL function_4C1F
	RET 
OutputString_AD1940:
	dw $A673
	db $14
	db $7E
	db $29
	db $F8
	db $96
	db $5D
	db $F3
	db $13
	db $B9
; Function Name: function_4C1F
function_4C1F:
	PUSH HL
	LD   A,(HL)
	ADD  A,A
	ADD  A,(HL)
	LD   HL,data_4C87.label_4CA8+12
	RST  $08
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	INC  HL
	RST  $20
	LD   A,(HL)
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	INC  HL
	RST  $20
	LD   A,(HL)
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	LD   HL,function_FDC4+444
	ADD  HL,DE
	EX   DE,HL
	POP  HL
	INC  HL
	INC  HL
	INC  HL
	CALL function_0D6B.label_0D73
	PUSH HL
	LD   HL,function_FDC4+476
	ADD  HL,DE
	EX   DE,HL
	POP  HL
	INC  HL
	INC  HL
	INC  HL
	LD   A,(HL)
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	INC  HL
	RST  $20
	LD   A,(HL)
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	INC  HL
	RST  $20
	LD   A,(HL)
	LD   (DE),A
	RES  2,D
	LD   A,C
	LD   (DE),A
	SET  2,D
	RET 
; Function Name: Phase3Function
Phase3Function:
	CALL ClearChars_07D2
	LD   A,(data_AD32)
	AND  A
	LD   HL,data_AD10
	JR   Z,.label_4C84
	LD   HL,data_AD20
.label_4C84:
	LD   DE,data_AD00
data_4C87:
	LD   BC,RST10_AddDoubleAToHL
	LDIR
	LD   A,(data_AD30)
	AND  A
	JP   Z,AdvancePhase
	LD   A,(data_AD01)
	LD   D,$06			; command 6 - ??
	LD   E,A
	RST  $38
	LD   A,(data_AD00)
	DEC  A
.operand_4C9E:
	LD   D,$05			; command 5
	LD   E,A
	RST  $38
	LD   B,$00
	LD   HL,data_5B50
	SUB  A
.label_4CA8:
	XOR  (HL)
	INC  HL
	DJNZ .label_4CA8
	ADD  A,$FF
	LD   (HWReg_VideoEnable),A
	JP   AdvancePhase
	db $96
	db $ED
	db $DC
	db $9B
	db $3B
	db $87
	db $CD
	db $D7
	db $87
	db $F3
	db $DC
	db $C4
	db $7F
	db $DC
	db $C4
; Function Name: function_4CC3
function_4CC3:
	LD   HL,data_AB08+3
	LD   B,$05
	LD   A,(data_AD32)
	AND  A
	LD   DE,data_AD35
	JR   Z,.label_4CD4
	db $11
	db $38
	db $AD
.label_4CD4:
	PUSH HL
	PUSH DE
	CALL function_4D2B
.operand_4CD9:
	JR   NC,.label_4CE4
	POP  DE
	POP  HL
	LD   A,$08
	RST  $08
	DJNZ .label_4CD4
	SCF
	RET 
.label_4CE4:
	db $05
	db $28
	db $3F
	db $21
	db $27
	db $AB
	db $11
	db $2F
	db $AB
	db $78
	db $87
	db $87
	db $87
	db $4F
	db $06
	db $00
	db $ED
	db $B8
	db $EB
	db $2B
	db $36
	db $F1
	db $2B
	db $36
	db $F1
	db $2B
	db $36
	db $F1
	db $22
	db $91
	db $A9
	db $2B
	db $D1
	db $01
	db $03
	db $00
	db $EB
	db $ED
	db $B8
	db $1A
	db $E1
	db $21
	db $31
	db $A5
	db $87
	db $CF
	db $22
	db $93
	db $A9
	db $21
	db $08
	db $AB
	db $11
	db $08
	db $00
	db $06
	db $05
	db $AF
	db $77
	db $19
	db $3C
	db $10
	db $FB
	db $37
	db $3F
	db $C9
	db $21
	db $2F
	db $AB
	db $18
	db $CC
; Function Name: function_4D2B
function_4D2B:
	LD   C,$03
.label_4D2D:
	LD   A,(DE)
	CP   (HL)
	RET  C
	JR   NZ,.label_4D37
	DEC  DE
	DEC  HL
	DEC  C
	JR   NZ,.label_4D2D
.label_4D37:
	db $37
	db $3F
	db $C9
; Function Name: function_4D3A
function_4D3A:
	LD   HL,data_AD05
	CALL function_4D67
	RET  C
	INC  L
	CALL function_4D67
	JR   C,.label_4D4B
	db $2C
	db $CD
	db $67
	db $4D
.label_4D4B:
	LD   HL,data_A9D7
	LD   A,(HL)
	AND  A
	RET  Z
	DEC  (HL)
	RET  NZ
	LD   A,(data_A9D6)
	LD   (HL),A
	LD   A,(data_ACC0)
	INC  A
	CP   $10
	JR   C,.label_4D61
	db $3E
	db $0F
.label_4D61:
	LD   (data_ACC0),A
	JP   function_19F0.label_1A9A
; Function Name: function_4D67
function_4D67:
	LD   A,(HL)
	ADD  A,$01
	DAA
	LD   (HL),A
	CP   $60
	RET  C
	LD   (HL),$00
	RET 
; Function Name: function_4D72
function_4D72:
	LD   C,A
	LD   A,(data_AD30)
	AND  A
	RET  Z
	LD   DE,VideoRAM+899
	LD   A,C
	CP   $07
	JR   C,.label_4D82
	LD   A,$06
.label_4D82:
	AND  A
	JR   Z,.label_4D91
	LD   B,$09
	LD   C,$18
.label_4D89:
	EX   AF,AF'
	CALL function_4DAF
	EX   AF,AF'
	DEC  A
	JR   NZ,.label_4D89
.label_4D91:
	LD   BC,data_E400+3344
.label_4D94:
	LD   HL,data_59DD
	ADD  HL,DE
	JR   NC,.label_4D9F
	CALL function_4DCF
	JR   .label_4D94
.label_4D9F:
	LD   B,$00
	LD   HL,CopySpriteDataToHW.CopySpritesFlipped+443
	SUB  A
.label_4DA5:
	XOR  (HL)
	INC  HL
	DJNZ .label_4DA5
	ADD  A,$19
	JP   NZ,data_4BB1
	RET 
; Function Name: function_4DAF
function_4DAF:
	LD   A,B
	ADD  A,$03
	LD   (DE),A
	DEC  A
	DEC  DE
	LD   (DE),A
	RST  $20
	LD   A,B
	LD   (DE),A
	INC  A
	INC  DE
	LD   (DE),A
	LD   HL,data_E400.label_F700+1280
	ADD  HL,DE
	RST  $20
	LD   (HL),C
	DEC  HL
	LD   (HL),C
	LD   A,L
	ADD  A,$20
	LD   L,A
	JR   NC,.label_4DCB
	INC  H
.label_4DCB:
	LD   (HL),C
	INC  HL
	LD   (HL),C
	RET 
; Function Name: function_4DCF
function_4DCF:
	EX   DE,HL
	LD   (HL),B
	DEC  HL
	LD   (HL),$F1
	RES  2,H
	LD   (HL),C
	INC  HL
	LD   (HL),C
	SET  2,H
	EX   DE,HL
	RST  $20
	RET 
; Function Name: function_4DDE
function_4DDE:
	LD   A,(data_AD30)
	AND  A
	RET  Z
	LD   A,(BonusScheme)
	AND  $01
	LD   HL,.label_4E18+3
	JR   Z,.label_4DF0
	LD   HL,.label_4E18+24
.label_4DF0:
	LD   C,(HL)
	LD   B,$00
	INC  HL
	LD   A,(data_AD32)
	AND  A
	LD   A,(data_AD35)
	JR   Z,.label_4E00
	db $3A
	db $38
	db $AD
.label_4E00:
	CPIR
	LD   HL,RemainingShipsInWave+1
	JR   NZ,.label_4E18
	db $CB
	db $46
	db $C0
	db $CB
	db $C6
	db $21
	db $00
	db $AD
	db $7E
	db $34
	db $16
	db $05
	db $5F
	db $FF
	db $C3
	db $05
	db $58
.label_4E18:
	RES  0,(HL)
	RET 
	db $14
	db $01
	db $06
	db $11
	db $16
	db $21
	db $26
	db $31
	db $36
	db $41
	db $46
	db $51
	db $56
	db $61
	db $66
	db $71
	db $76
	db $81
	db $86
	db $91
	db $96
	db $11
	db $02
	db $08
	db $14
	db $20
	db $26
	db $32
	db $38
	db $44
	db $50
	db $56
	db $62
	db $68
	db $74
	db $80
	db $86
	db $92
	db $98
OutputString_Player2:
	dw $A66F
	db $14
	db $88
	db $57
	db $A5
	db $BF
	db $34
	db $D7
	db $F1
	db $9B
	db $F1
	db $B9
; Function Name: function_4E4F
function_4E4F:
	LD   A,(data_AD04)
	CP   $04
	JP   Z,.label_4F2A
	DEC  A
	JP   Z,.label_4EBC
	LD   A,(FrameCounter)
	AND  $01
	JP   NZ,.label_4F35
	CALL function_4F5D
	LD   B,$04
	LD   DE,data_A80A+6
	LD   IY,PlayerSprite_No+1
	LD   L,$05
	LD   H,$0B
	CALL function_5185
	LD   A,(data_AD0D)
	AND  A
	JR   NZ,.label_4E97
	LD   B,$07
	LD   L,$07
	LD   H,$0F
	CALL function_5152
	LD   B,$03
	LD   L,$06
	LD   H,$0D
	CALL function_5121
	LD   B,$01
	LD   L,$08
	LD   H,$11
	JP   function_5185.label_51B3
.label_4E97:
	LD   B,$05
	LD   L,$07
	LD   H,$0F
	CALL function_5152
	CALL function_50B1
	LD   B,$03
	LD   DE,data_A8C0
	LD   IY,data_AA28
	LD   L,$06
	LD   H,$0D
	CALL function_5121
	LD   B,$01
	LD   L,$08
	LD   H,$11
	JP   function_5185.label_51B3
.label_4EBC:
	LD   A,(FrameCounter)
	AND  $01
	JP   NZ,.label_4F35
	CALL function_4F7E
	LD   B,$04
	LD   DE,data_A80A+6
	LD   IY,PlayerSprite_No+1
	LD   L,$05
	LD   H,$0B
	CALL function_5185
	LD   A,(data_AD0D)
	AND  A
	JR   NZ,.label_4F02
	LD   B,$07
	LD   L,$07
	LD   H,$0F
	CALL function_5152
	CALL function_507E
	LD   B,$01
	LD   DE,data_A8E0
	LD   IY,data_AA2A+2
	LD   L,$05
	LD   H,$0B
	CALL function_5185
	LD   B,$01
	LD   L,$08
	LD   H,$11
	JP   function_5185.label_51B3
.label_4F02:
	LD   B,$05
	LD   L,$07
	LD   H,$0F
	CALL function_5152
	CALL function_50B1
	CALL function_507E
	LD   B,$01
	LD   DE,data_A8E0
	LD   IY,data_AA2A+2
	LD   L,$05
	LD   H,$0B
	CALL function_5185
	LD   B,$01
	LD   L,$08
	LD   H,$11
	JP   function_5185.label_51B3
.label_4F2A:
	db $3A
	db $80
	db $A9
	db $E6
	db $01
	db $CA
	db $63
	db $4E
	db $C3
	db $32
	db $50
.label_4F35:
	LD   A,(data_AD0D)
	AND  A
	JP   NZ,function_4F7E.label_4FBF
	LD   DE,data_A844+12
	LD   IY,PlayerSprite_No+9
	LD   IX,data_AA80
	EX   AF,AF'
	LD   A,$07
	LD   B,A
	EX   AF,AF'
	LD   C,$06
	LD   (data_A993),DE
	LD   (data_A991),IY
	LD   L,$07
	LD   H,$0F
	JP   function_5205.label_5211
; Function Name: function_4F5D
function_4F5D:
	LD   DE,data_A8C0
	LD   IY,data_AA28
	LD   IX,data_AA80
	EX   AF,AF'
	LD   A,$03
	LD   B,A
	EX   AF,AF'
	LD   C,$06
	LD   (data_A993),DE
	LD   (data_A991),IY
	LD   L,$07
	LD   H,$0F
	JP   function_5205.label_5211
; Function Name: function_4F7E
function_4F7E:
	LD   L,$06
	LD   H,$0D
	LD   E,$17
	LD   D,$1F
	LD   IY,data_AA80
	LD   B,$06
	LD   A,(data_A8C0)
	INC  A
	RET  NZ
.label_4F91:
	LD   A,(IY+$00)
	INC  A
	JR   NZ,.label_4FB6
	LD   A,(data_AA28)
	SUB  (IY+$06)
	ADD  A,L
	CP   H
	JR   NC,.label_4FB6
	LD   A,(data_AA59)
	SUB  (IY+$04)
	ADD  A,E
	CP   D
	JR   NC,.label_4FB6
	LD   A,$F0
	LD   (data_A8C0),A
	LD   (IY+$00),A
	CALL function_51DE
.label_4FB6:
	LD   A,IYL
	ADD  A,$10
	LD   IYL,A
	DJNZ .label_4F91
	RET 
.label_4FBF:
	LD   DE,data_A844+12
	LD   IY,PlayerSprite_No+9
	LD   IX,data_AA80
	EX   AF,AF'
	LD   A,$05
	LD   B,A
	EX   AF,AF'
	LD   C,$06
	LD   (data_A993),DE
	LD   (data_A991),IY
	LD   L,$07
	LD   H,$0F
	CALL function_5205.label_5211
	LD   A,(data_AD04)
	AND  A
	JR   Z,.label_502B
	CP   $04
	JR   Z,.label_502B
	LD   L,$06
	LD   H,$0D
	LD   E,$17
	LD   D,$1F
	LD   IY,data_AA80
	LD   B,$06
	LD   A,(data_A8A0)
	INC  A
	RET  NZ
.label_4FFD:
	LD   A,(IY+$00)
	INC  A
	JR   NZ,.label_5022
	LD   A,(data_AA24)
	SUB  (IY+$06)
	ADD  A,L
	CP   H
	JR   NC,.label_5022
	LD   A,(data_AA55)
	SUB  (IY+$04)
	ADD  A,E
	CP   D
	JR   NC,.label_5022
	LD   A,$F0
	LD   (data_A8A0),A
	LD   (IY+$00),A
	CALL function_51DE
.label_5022:
	LD   A,IYL
	ADD  A,$10
	LD   IYL,A
	DJNZ .label_4FFD
	RET 
.label_502B:
	db $2E
	db $08
	db $26
	db $11
	db $C3
	db $EE
	db $4F
	db $3A
	db $0D
	db $AD
	db $A7
	db $C2
	db $5A
	db $50
	db $11
	db $10
	db $A8
	db $FD
	db $21
	db $12
	db $AA
	db $DD
	db $21
	db $80
	db $AA
	db $08
	db $3E
	db $0B
	db $47
	db $08
	db $0E
	db $06
	db $ED
	db $53
	db $93
	db $A9
	db $FD
	db $22
	db $91
	db $A9
	db $2E
	db $07
	db $26
	db $0F
	db $C3
	db $11
	db $52
	db $11
	db $10
	db $A8
	db $FD
	db $21
	db $12
	db $AA
	db $DD
	db $21
	db $80
	db $AA
	db $08
	db $3E
	db $09
	db $47
	db $08
	db $0E
	db $06
	db $ED
	db $53
	db $93
	db $A9
	db $FD
	db $22
	db $91
	db $A9
	db $2E
	db $07
	db $26
	db $0F
	db $CD
	db $11
	db $52
	db $C3
	db $E0
	db $4F
; Function Name: function_507E
function_507E:
	LD   IX,PlayerSprite_X
	LD   A,(data_A800)
	INC  A
	RET  NZ
	LD   A,(data_A8C0)
	INC  A
	RET  NZ
	LD   A,(data_AA28)
	SUB  (IX+$00)
	ADD  A,$06
	CP   $0D
	RET  NC
	LD   A,(data_AA59)
	SUB  (IX+$31)
	ADD  A,$18
	CP   $21
	RET  NC
	db $3E
	db $F0
	db $32
	db $00
	db $A8
	db $32
	db $C0
	db $A8
	db $AF
	db $32
	db $DC
	db $A8
	db $C3
	db $DE
	db $51
; Function Name: function_50B1
function_50B1:
	LD   A,(data_AD04)
	AND  A
	JR   Z,.label_50EE
	CP   $04
	JR   Z,.label_50EE
	LD   IX,PlayerSprite_X
	LD   A,(data_A800)
	INC  A
	RET  NZ
	LD   A,(data_A8A0)
	INC  A
	RET  NZ
	LD   A,(data_AA24)
	SUB  (IX+$00)
	ADD  A,$06
	CP   $0D
	RET  NC
	LD   A,(data_AA55)
	SUB  (IX+$31)
	ADD  A,$19
	CP   $23
	RET  NC
	db $3E
	db $F0
	db $32
	db $00
	db $A8
	db $32
	db $A0
	db $A8
	db $AF
	db $32
	db $A4
	db $A8
	db $C3
	db $DE
	db $51
.label_50EE:
	db $DD
	db $21
	db $10
	db $AA
	db $3A
	db $00
	db $A8
	db $3C
	db $C0
	db $3A
	db $A0
	db $A8
	db $3C
	db $C0
	db $3A
	db $24
	db $AA
	db $DD
	db $96
	db $00
	db $C6
	db $08
	db $FE
	db $11
	db $D0
	db $3A
	db $55
	db $AA
	db $DD
	db $96
	db $31
	db $C6
	db $19
	db $FE
	db $23
	db $D0
	db $3E
	db $F0
	db $32
.label_5115:
	db $00
	db $A8
	db $32
	db $A0
	db $A8
	db $AF
	db $32
	db $A4
	db $A8
	db $C3
	db $DE
	db $51
; Function Name: function_5121
function_5121:
	LD   A,(data_A800)
	INC  A
	RET  NZ
.label_5126:
	LD   A,(DE)
	INC  A
	JR   NZ,.label_5147
	LD   A,(PlayerSprite_X)
	SUB  (IY+$00)
	ADD  A,L
	CP   H
	JR   NC,.label_5147
	LD   A,(PlayerSprite_Y)
	SUB  (IY+$31)
	ADD  A,L
	CP   H
	JR   NC,.label_5147
	LD   A,$F0
	LD   (data_A800),A
	LD   (DE),A
	CALL function_51DE
.label_5147:
	LD   A,E
	ADD  A,$10
	LD   E,A
	INC  IY
	INC  IY
	DJNZ .label_5126
	RET 
; Function Name: function_5152
function_5152:
	LD   A,(data_A800)
	INC  A
	RET  NZ
.label_5157:
	LD   A,(DE)
	INC  A
	JR   NZ,.label_517A
	LD   A,(PlayerSprite_X)
	SUB  (IY+$00)
	ADD  A,L
	CP   H
	JR   NC,.label_517A
	LD   A,(PlayerSprite_Y)
	SUB  (IY+$31)
	ADD  A,$08
	CP   $11
	JR   NC,.label_517A
	LD   A,$F0
	LD   (data_A800),A
	LD   (DE),A
	CALL function_51DE
.label_517A:
	LD   A,E
	ADD  A,$10
	LD   E,A
	INC  IY
	INC  IY
	DJNZ .label_5157
	RET 
; Function Name: function_5185
function_5185:
	LD   A,(data_A800)
	INC  A
	RET  NZ
.label_518A:
	LD   A,(DE)
	INC  A
	JR   NZ,.label_51A8
	LD   A,(PlayerSprite_X)
	SUB  (IY+$00)
	ADD  A,L
	CP   H
	JR   NC,.label_51A8
	LD   A,(PlayerSprite_Y)
	SUB  (IY+$31)
	ADD  A,L
	CP   H
	JR   NC,.label_51A8
	LD   A,$F0
	LD   (data_A800),A
	LD   (DE),A
.label_51A8:
	LD   A,E
	ADD  A,$10
	LD   E,A
	INC  IY
	INC  IY
	DJNZ .label_518A
	RET 
.label_51B3:
	LD   A,(data_A800)
	INC  A
	RET  NZ
.label_51B8:
	LD   A,(DE)
	INC  A
	JR   NZ,.label_51D3
	LD   A,(PlayerSprite_X)
	SUB  (IY+$00)
	ADD  A,L
	CP   H
	JR   NC,.label_51D3
	LD   A,(PlayerSprite_Y)
	SUB  (IY+$31)
	ADD  A,L
	CP   H
	JR   NC,.label_51D3
	LD   A,$F0
	LD   (DE),A
.label_51D3:
	LD   A,E
	ADD  A,$10
	LD   E,A
	INC  IY
	INC  IY
	DJNZ .label_51B8
	RET 
; Function Name: function_51DE
function_51DE:
	PUSH DE
	LD   A,(data_A99D)
	AND  A
	JR   Z,.label_51FA
	LD   A,(data_A99E)
	INC  A
	LD   (data_A99E),A
	AND  $07
	INC  A
	LD   E,A
	LD   D,$04
	RST  $38
	POP  DE
	LD   A,$1E
	LD   (data_A99D),A
	RET 
.label_51FA:
	LD   DE,CopySpriteDataToHW+156
	RST  $38
	POP  DE
	LD   A,$1E
	LD   (data_A99D),A
	RET 
; Function Name: function_5205
function_5205:
	LD   HL,data_A99D
	LD   A,(HL)
	AND  A
	JR   Z,.label_520E
	DEC  (HL)
	RET 
.label_520E:
	INC  L
	LD   (HL),A
	RET 
.label_5211:
	LD   A,(IX+$00)
	INC  A
	JR   NZ,.label_5254
.label_5217:
	LD   A,(DE)
	INC  A
	JR   NZ,.label_524A
	LD   A,(IY+$00)
	ADD  A,$08
	CP   $19
	JR   C,.label_524A
	LD   A,(IY+$31)
	ADD  A,$10
	CP   $11
	JR   C,.label_524A
	LD   A,(IX+$06)
	SUB  (IY+$00)
	ADD  A,L
	CP   H
	JR   NC,.label_524A
	LD   A,(IX+$04)
	SUB  (IY+$31)
	ADD  A,L
	CP   H
	JR   NC,.label_524A
	LD   A,$F0
	LD   (IX+$00),A
	LD   (DE),A
	CALL function_51DE
.label_524A:
	LD   A,E
	ADD  A,$10
	LD   E,A
	INC  IY
	INC  IY
	DJNZ .label_5217
.label_5254:
	LD   IY,(data_A991)
	LD   DE,(data_A993)
	EX   AF,AF'
	LD   B,A
	EX   AF,AF'
	LD   A,IXL
	ADD  A,$10
	LD   IXL,A
	DEC  C
	JP   NZ,.label_5211
	RET 
; Function Name: ResetBulletDrawLists
ResetBulletDrawLists:
	LD   HL,BulletEraseList
	LD   (BulletEraseList_EndPtr),HL
	LD   HL,BulletDrawList
	LD   (BulletDrawList_EndPtr),HL
	RET 
	db $06
	db $00
	db $21
	db $DE
	db $27
	db $AF
	db $86
	db $23
	db $10
	db $FC
	db $D6
	db $C5
	db $C4
	db $D4
	db $53
; Function Name: RedrawBullets
RedrawBullets:
	CALL EraseBullets
	CALL DrawBulletList
	LD   A,(BulletDrawList_EndPtr)
	CP   $04			; empty bullet list?
	JR   Z,ResetBulletDrawLists			; tail call ret
	LD   C,A
	LD   B,$00
	LD   HL,BulletDrawList_EndPtr
	LD   DE,BulletEraseList_EndPtr
	LDIR			; copy bullet draw list
	ADD  A,$80
	LD   (BulletEraseList_EndPtr),A			; patch endptr
	LD   HL,BulletDrawList			; reset bullet draw list
	LD   (BulletDrawList_EndPtr),HL
	RET 
; Function Name: label_52AA
label_52AA:
	LD   A,(Phase9Function.operand_08C7+2)
	LD   (data_A98D),A
	LD   A,(data_0874)
	LD   (data_A9CD),A
	LD   A,(HWReg_DSW1)			; DSW1 Read
	CPL
	LD   (DipSwitch1),A
	CALL function_4ACC
	LD   A,(WatchdogReset_DipSW2)			; DSW2 Read
	CPL
	LD   C,A
	AND  $03
	ADD  A,$03
	CP   $06
	JR   NZ,.label_52CF
	LD   A,$FF
.label_52CF:
	JP   function_2DF4.label_2E19
; Function Name: DrawBulletList
DrawBulletList:
	LD   A,(CharacterAttrib)			; bullet colour?
	AND  $0F
	LD   C,A
	LD   HL,(BulletDrawList_EndPtr)
	LD   A,L
	SUB  $04			; sub 4 to get list length
	RET  Z			; no bullets - exit
	RRCA			; divide by 4 to get no items
	RRCA
	AND  $1F
	LD   B,A			; loop count
	LD   HL,BulletDrawList
.bullet_draw_loop:
	LD   E,(HL)			; fetch colour RAM address
	INC  L
	LD   D,(HL)
	INC  L
	LD   A,(DE)			; read from Colour RAM
	AND  $10			; check bit 4
	JR   NZ,.skip_bullet			; jump if set
	LD   A,(HL)			; fetch char code
	SET  2,D			; put into video RAM addr space
	LD   (DE),A			; write to video RAM
	RES  2,D			; back to colour RAM
	INC  L
	LD   A,(HL)			; read attrib
	INC  L
	ADD  A,C			; add colour
	LD   (DE),A
	DJNZ .bullet_draw_loop
	RET 
.skip_bullet:
	INC  L
	INC  L
	DJNZ .bullet_draw_loop
	RET 
.label_5303:
	CALL function_200C
	CP   $67
	JP   NZ,label_0F8D
	JP   AdvancePhase
; Function Name: EraseBullets
; Description: Erases previous bullets
EraseBullets:
	LD   HL,(BulletEraseList_EndPtr)
	LD   A,L
	AND  $7F			; mask bottom 7 bits
	SUB  $04			; fixup to list length
	RET  Z
	RRCA
	RRCA
	AND  $1F
	LD   B,A
	LD   HL,BulletEraseList
.bullet_erase_loop:
	LD   E,(HL)			; fetch address
	INC  L
	LD   D,(HL)
	INC  L
	LD   A,(DE)			; fetch attrib at address
	AND  $10
	JR   NZ,.skip_bullet
	INC  L			; skip char & attrib
	INC  L
	SET  2,D			; swich to video RAM
	LD   A,$20
	LD   (DE),A			; clear character
	DJNZ .bullet_erase_loop
	RET 
.skip_bullet:
	INC  L
	INC  L
	DJNZ .bullet_erase_loop
	RET 
; Function Name: function_5337
function_5337:
	LD   A,(IX+$04)			; y coord?
	ADD  A,$07
	LD   B,A			; offset high byte?
	LD   D,$28
	RLCA
	RL   D
	RLCA
	RL   D
	AND  $E0
	LD   E,A
	LD   A,(IX+$06)			; x coord?
	ADD  A,$07
	LD   C,A
	RRCA
	RRCA
	RRCA
	AND  $1F
	ADD  A,E
	LD   E,A			; colour RAM low byte?
	LD   A,C
	RLCA
	RLCA
	RLCA
	AND  $38
	LD   C,A			; offset low byte
	LD   A,B
	LD   B,$00
	BIT  2,A
	JR   Z,.label_5364
	INC  B
.label_5364:
	RRCA
	RRCA
	AND  $C0
	ADD  A,C
	LD   C,A
	LD   HL,data_53D4
	ADD  HL,BC			; add offset
	LD   A,(HL)			; bullet char
	INC  HL
	LD   B,(HL)			; bullet attrib
	INC  HL
	AND  A
	JR   Z,.label_5385
	PUSH HL
	LD   HL,(BulletDrawList_EndPtr)
	LD   (HL),E			; set Colour RAM address
	INC  L
	LD   (HL),D
	INC  L
	LD   (HL),A			; bullet char
	INC  L
	LD   (HL),B			; bullet attrib
	INC  L
	LD   (BulletDrawList_EndPtr),HL			; update ptr
	POP  HL
.label_5385:
	INC  DE
	LD   A,(HL)			; fetch bullet char
	INC  HL
	LD   B,(HL)			; fetch attrib
	INC  HL
	AND  A
	JR   Z,.label_539D
	PUSH HL
	LD   HL,(BulletDrawList_EndPtr)
	LD   (HL),E			; set colour RAM address
	INC  L
	LD   (HL),D
	INC  L
	LD   (HL),A			; bullet char
	INC  L
	LD   (HL),B			; bullet attrib
	INC  L
	LD   (BulletDrawList_EndPtr),HL			; update ptr
	POP  HL
.label_539D:
	LD   A,E
	ADD  A,$1F
	LD   E,A
	JR   NC,.label_53A4
	INC  D
.label_53A4:
	LD   A,(HL)
	INC  HL
	LD   B,(HL)
	INC  HL
	AND  A
	JR   Z,.label_53BB
	PUSH HL
	LD   HL,(BulletDrawList_EndPtr)
	LD   (HL),E			; bullet address
	INC  L
	LD   (HL),D
	INC  L
	LD   (HL),A			; bullet char
	INC  L
	LD   (HL),B			; bullet attrib
	INC  L
	LD   (BulletDrawList_EndPtr),HL			; update ptr
	POP  HL
.label_53BB:
	INC  DE
	LD   A,(HL)
	INC  HL
	LD   B,(HL)
	INC  HL
	AND  A
	JR   Z,.label_53D3
	PUSH HL
	LD   HL,(BulletDrawList_EndPtr)
	LD   (HL),E
	INC  L
	LD   (HL),D
	INC  L
	LD   (HL),A
	INC  L
	LD   (HL),B
	INC  L
	LD   (BulletDrawList_EndPtr),HL
	POP  HL
.label_53D3:
	RET 
data_53D4:
	db $24
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $DD
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $61
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3C
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $61
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $DD
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $24
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $39
	db $20
	db $39
	db $60
	db $00
	db $00
	db $00
	db $00
	db $30
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $A1
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B7
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $D0
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B7
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $A1
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $30
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $6D
	db $20
	db $6D
	db $60
	db $00
	db $00
	db $00
	db $00
	db $40
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $34
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $2B
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B1
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $2B
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $34
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $40
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $8E
	db $20
	db $8E
	db $60
	db $00
	db $00
	db $00
	db $00
	db $74
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $54
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $4C
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $2D
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $4C
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $54
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $74
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $D5
	db $20
	db $D5
	db $60
	db $00
	db $00
	db $00
	db $00
	db $40
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $34
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $2B
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B1
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $2B
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $34
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $40
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $8E
	db $A0
	db $8E
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $30
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $A1
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B7
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $D0
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B7
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $A1
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $30
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $6D
	db $A0
	db $6D
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $24
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $DD
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $61
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3C
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $61
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $DD
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $24
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $39
	db $A0
	db $39
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $3A
	db $20
	db $00
	db $00
	db $3A
	db $A0
	db $00
	db $00
	db $8F
	db $20
	db $00
	db $00
	db $8F
	db $A0
	db $00
	db $00
	db $70
	db $20
	db $00
	db $00
	db $70
	db $A0
	db $00
	db $00
	db $66
	db $20
	db $00
	db $00
	db $66
	db $A0
	db $00
	db $00
	db $70
	db $60
	db $00
	db $00
	db $70
	db $E0
	db $00
	db $00
	db $8F
	db $60
	db $00
	db $00
	db $8F
	db $E0
	db $00
	db $00
	db $3A
	db $60
	db $00
	db $00
	db $3A
	db $E0
	db $00
	db $00
	db $C7
	db $20
	db $C7
	db $60
	db $C7
	db $A0
	db $C7
	db $E0
; Function Name: function_55D4
function_55D4:
	LD   HL,data_AC43
	LD   A,(HL)
	AND  A
	RET  Z
	DEC  (HL)
	PUSH AF
	INC  HL
	LD   A,(HL)
	CALL SendAudioCommand
	POP  AF
	RET  Z
	DEC  A
	LD   B,$00
	LD   C,A
	LD   E,L
	LD   D,H
	INC  HL
	LDIR
	RET 
OutputString_AD1982:
	dw $A673
	db $14
	db $7E
	db $29
	db $F8
	db $96
	db $5D
	db $17
	db $9B
	db $B9
; Function Name: SendAudioCommand
SendAudioCommand:
	LD   (W_AudioCommand_R_Scanline),A			; Send Audio Command
	LD   A,$01
	LD   (HWReg_TriggerAudioInterrupt),A			; Trigger Audio Interrupt
	NOP 
	NOP 
	NOP 
	NOP 
	NOP 
	NOP 
	LD   A,$00
	LD   (HWReg_TriggerAudioInterrupt),A			; Trigger Audio Interrupt
	RET 
; Function Name: function_560C
function_560C:
	PUSH HL
	PUSH AF
	LD   A,(data_AD30)
	AND  A
	JR   NZ,label_5628.label_562A
	POP  AF
	POP  HL
	RET 
; Function Name: label_5617
label_5617:
	PUSH HL
	PUSH AF
	LD   A,(data_AD30)
	AND  A
	JR   NZ,label_5628.label_562A
	LD   A,(data_A9C6)
	AND  A
	JR   NZ,label_5628.label_562A
	POP  AF
	POP  HL
	RET 
; Function Name: label_5628
label_5628:
	PUSH HL
	PUSH AF
.label_562A:
	LD   HL,data_AC43
	INC  (HL)
	LD   A,(HL)
	RST  $08
	POP  AF
	LD   (HL),A
	POP  HL
	RET 
; Function Name: function_5634
function_5634:
	LD   A,(label_167B+1)
	CALL label_5628
	LD   A,(data_0A9C)
	CALL label_5628
	LD   A,(data_1484)
	CALL label_5628
	LD   A,(data_0C78)
	CALL label_5628
	LD   A,(ClearChars_07D2+1)
	CALL label_5628
	LD   A,(data_33B4)
	CALL label_5628
	LD   A,(data_AD04)
	ADD  A,$8C
	JR   label_5628
.label_565F:
	LD   A,(Phase4Function.label_07A0+2)
	JR   function_560C
.label_5664:
	LD   A,(Phase5Function.operand_16DC+2)
	JR   function_560C
.label_5669:
	LD   A,(data_4C87.operand_4C9E+1)
	JR   function_560C
.label_566E:
	LD   A,(ClearChars_07D2.operand_07D8)
	CALL function_560C
	LD   A,(function_2755.operand_2769+2)
	JR   function_560C
; Function Name: function_5679
function_5679:
	LD   A,(data_07FE)
	JR   function_560C
; Function Name: function_567E
function_567E:
	LD   A,(label_3252.operand_326F+1)
	JR   label_5617
; Function Name: function_5683
function_5683:
	LD   A,(Phase4Function.label_07A4+2)
	CALL label_5617
	LD   A,(function_4CC3.operand_4CD9+1)
	JR   label_5617
.label_568E:
	LD   A,(function_2D6E.operand_2D86+1)
	JP   function_560C
; Function Name: Phase6Function
Phase6Function:
	LD   C,$00
	LD   HL,DrawRemainingWaveTally.wave_char_loop+3
	LD   A,(FunctionChecksum)
.label_569C:
	SUB  (HL)
	INC  HL
	DEC  C
	JR   NZ,.label_569C
	XOR  $C2
	LD   (FunctionChecksum),A
	CALL function_0F97
	CALL function_1EDF
	CALL function_0F97
	CALL function_2CBC
	CALL function_23E3
	CALL function_1098
	LD   HL,data_A9EB
	DEC  (HL)
	RET  NZ
	LD   C,$00			; 256 loops
	LD   HL,OutputString_AndEvery50000.label_1283+36
	LD   A,(FunctionChecksum)
.label_56C5:
	SUB  (HL)
	INC  HL
	DEC  C
	JR   NZ,.label_56C5
	XOR  $59
	LD   (FunctionChecksum),A
	JP   AdvancePhase
; Function Name: function_56D2
function_56D2:
	LD   A,(data_0C5B)
	CALL function_560C
	LD   A,(QueueTitleScreenDraw.operand_0854+1)
	CALL function_560C
	LD   A,(data_1675)
	CALL function_560C
	LD   A,(Phase0Function.operand_27CA+1)
	CALL label_5617
	LD   A,(data_33A0)
	JP   label_5617
	db $FF
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $01
	db $01
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $01
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $01
	db $00
	db $00
	db $01
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $01
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FF
; Function Name: function_57F1
function_57F1:
	LD   A,(function_31B4.operand_322D+1)
	JP   label_5628
.label_57F7:
	LD   A,(data_AD04)
	ADD  A,$0C
	JP   function_560C
; Function Name: function_57FF
function_57FF:
	LD   A,(Phase4Function.operand_079B)
	JP   function_560C
	db $3A
	db $4E
	db $2D
	db $C3
	db $0C
	db $56
; Function Name: function_580B
function_580B:
	LD   A,(function_49D6.operand_49ED+1)
	JP   function_560C
	db $3A
	db $A9
	db $07
	db $C3
	db $0C
	db $56
.label_5817:
	LD   A,(ColourRAMPtr.operand_2738+2)
	JP   function_560C
OutputString_PleaseDepositCoin:
	dw $A70C
	db $13
	db $88
	db $57
	db $34
	db $A5
	db $ED
	db $34
	db $F1
	db $87
	db $34
	db $88
	db $68
	db $ED
	db $FD
	db $DC
	db $F1
	db $77
	db $68
	db $FD
	db $3B
	db $B9
; Function Name: function_5834
function_5834:
	LD   A,(function_1748.operand_1766+1)
	JP   function_560C
; Function Name: function_583A
function_583A:
	db $3A
	db $FA
	db $18
	db $C3
	db $0C
	db $56
; Function Name: function_5840
function_5840:
	LD   HL,ChecksumFailed
	JP   label_58BC
	db $21
	db $00
	db $5C
	db $C3
	db $BC
	db $58
OutputString_1UP:
	db $60
	db $A7
	db $14
	db $96
	db $10
	db $0D
	db $88
	db $B9
; Function Name: function_5854
function_5854:
	LD   HL,data_5E00
	JP   label_58BC
	db $21
	db $30
	db $25
	db $C3
	db $BC
	db $58
	db $21
	db $3E
	db $2E
	db $C3
	db $BC
	db $58
; Function Name: ClearVideoRAM
ClearVideoRAM:
	LD   HL,(ColourRAMPtr)
	LD   BC,CopySpriteDataToHW+155
	LD   D,$10
.clear_colour_ram_loop:
	LD   (HL),D
	INC  HL
	DEC  BC
	LD   A,C
	OR   B
	JR   NZ,.clear_colour_ram_loop
	LD   (WatchdogReset_DipSW2),A
	LD   HL,(PhaseDFunction.operand_4A36+1)
	LD   BC,CopySpriteDataToHW+155
	LD   D,$F1
.clear_video_ram_loop:
	LD   (HL),D
	INC  HL
	DEC  BC
	LD   A,C
	OR   B
	JR   NZ,.clear_video_ram_loop
	LD   HL,$0000
	LD   A,($0000)
.checksum_loop:
	ADD  A,(HL)
	INC  HL
	EX   AF,AF'
	LD   A,H
	CP   $60
	JR   NC,.label_589B
	EX   AF,AF'
	LD   (WatchdogReset_DipSW2),A
	JR   .checksum_loop
.label_589B:
	EX   AF,AF'
	SUB  $AF			; checksum value
	JP   NZ,ChecksumFailed
	JP   InitProgram_2511
; Function Name: function_58A4
function_58A4:
	LD   HL,TamperDetected
	JP   label_58BC
; Function Name: function_58AA
function_58AA:
	LD   HL,ChecksumFailed
	JP   label_58BC.label_58FE
	db $21
	db $00
	db $5C
	db $C3
	db $FE
	db $58
	db $21
	db $00
	db $5E
	db $C3
	db $FE
	db $58
; Function Name: label_58BC
label_58BC:
	LD   A,(IX+$02)
	LD   C,A
	ADD  A,A
	JR   NC,.label_58C4
	INC  H
.label_58C4:
	ADD  A,L
	LD   L,A
	JR   NC,.label_58C9
	INC  H
.label_58C9:
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	LD   A,C
	ADD  A,$C0
	LD   BC,label_0174+12
	JR   NC,.label_58D7
	LD   BC,function_FDC4+444
.label_58D7:
	ADD  HL,BC
	LD   B,(HL)
	DEC  HL
	LD   C,(HL)
	LD   HL,(data_A808)
	ADD  HL,DE
	LD   E,(IX+$03)
	LD   D,(IY+$31)
	ADD  HL,DE
	LD   (IX+$03),L
	LD   (IY+$31),H
	LD   HL,(data_A80A)
	ADD  HL,BC
	LD   E,(IX+$05)
	LD   D,(IY+$00)
	ADD  HL,DE
	LD   (IX+$05),L
	LD   (IY+$00),H
	RET 
.label_58FE:
	LD   A,(IX+$02)
	LD   C,A
	ADD  A,A
	JR   NC,.label_5906
	INC  H
.label_5906:
	ADD  A,L
	LD   L,A
	JR   NC,.label_590B
	INC  H
.label_590B:
	LD   E,(HL)
	INC  HL
data_590D:
	LD   D,(HL)
	LD   A,C
	ADD  A,$C0
	LD   BC,label_0174+12
	JR   NC,.label_5919
	LD   BC,function_FDC4+444
.label_5919:
	ADD  HL,BC
	LD   B,(HL)
	DEC  HL
	LD   C,(HL)
	LD   HL,(data_A808)
	ADD  HL,DE
	ADD  HL,DE
	LD   E,(IX+$03)
	LD   D,(IY+$31)
	ADD  HL,DE
	LD   (IX+$03),L
	LD   (IY+$31),H
	LD   HL,(data_A80A)
	ADD  HL,BC
	ADD  HL,BC
	LD   E,(IX+$05)
	LD   D,(IY+$00)
	ADD  HL,DE
	LD   (IX+$05),L
	LD   (IY+$00),H
	RET 
; Function Name: function_5942
function_5942:
	LD   HL,ChecksumFailed
	JP   label_596E
	db $21
	db $00
	db $5C
	db $C3
	db $6E
	db $59
.label_594E:
	LD   HL,data_5E00
	JP   label_596E
OutputString_AD1970:
	dw $A673
	db $14
	db $7E
	db $29
	db $F8
	db $96
	db $5D
	db $02
	db $13
	db $B9
	db $21
	db $30
	db $25
	db $C3
	db $6E
	db $59
.label_5965:
	LD   HL,function_2E31.label_2E3E
	JP   label_596E
.label_596B:
	LD   HL,TamperDetected
; Function Name: label_596E
label_596E:
	LD   A,(IX+$02)
	LD   C,A
	ADD  A,A
	JR   NC,.label_5976
	INC  H
.label_5976:
	ADD  A,L
	LD   L,A
	JR   NC,.label_597B
	INC  H
.label_597B:
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	LD   A,C
	ADD  A,$C0
	LD   BC,label_0174+12
	JR   NC,.label_5989
	LD   BC,function_FDC4+444
.label_5989:
	ADD  HL,BC
	LD   B,(HL)
	DEC  HL
	LD   C,(HL)
	RET 
; Function Name: function_598E
function_598E:
	LD   HL,ChecksumFailed
	JP   .label_599D
	LD   HL,data_5C00
	JP   .label_599D
	db $21
	db $00
	db $5E
.label_599D:
	LD   A,(IX+$02)
.label_59A0:
	LD   C,A
	ADD  A,A
	JR   NC,.label_59A5
	INC  H
.label_59A5:
	ADD  A,L
	LD   L,A
	JR   NC,.label_59AA
	INC  H
.label_59AA:
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	SLA  E
	RL   D
	LD   A,C
	ADD  A,$C0
	LD   BC,label_0174+12
	JR   NC,.label_59BC
	LD   BC,function_FDC4+444
.label_59BC:
	ADD  HL,BC
	LD   B,(HL)
	DEC  HL
	LD   C,(HL)
	SLA  C
	RL   B
	RET 
; Function Name: function_59C5
function_59C5:
	LD   HL,ChecksumFailed
	JP   function_598E.label_59A0
; Function Name: function_59CB
function_59CB:
	LD   HL,data_5C00
	JP   function_598E.label_59A0
; Function Name: function_59D1
function_59D1:
	LD   HL,data_5E00
	JP   function_598E.label_59A0
; Function Name: ChecksumFailed
ChecksumFailed:
	ADC  A,$00
	CALL function_CC00
	db $00
data_59DD:
	db $CB
	db $00
	db $CA
	db $00
	db $C9
	db $00
	db $C8
	db $00
	db $C8
	db $00
	db $C6
	db $00
	db $C4
	db $00
	db $C2
	db $00
	db $C0
	db $00
	db $BF
	db $00
	db $BC
	db $00
	db $BA
	db $00
	db $B9
	db $00
	db $B6
	db $00
	db $B3
	db $00
	db $B0
	db $00
	db $AF
	db $00
	db $AC
	db $00
	db $A9
	db $00
	db $A8
	db $00
	db $A5
	db $00
	db $A2
	db $00
	db $A1
	db $00
	db $9E
	db $00
	db $9B
	db $00
	db $98
	db $00
	db $97
	db $00
	db $94
	db $00
	db $91
	db $00
	db $90
	db $00
	db $8D
	db $00
	db $89
	db $00
	db $88
	db $00
	db $85
	db $00
	db $81
	db $00
	db $7F
	db $00
	db $7B
	db $00
	db $78
	db $00
	db $76
	db $00
	db $70
	db $00
	db $6D
	db $00
	db $68
	db $00
	db $63
	db $00
	db $60
	db $00
	db $5C
	db $00
	db $58
	db $00
	db $52
	db $00
	db $4E
	db $00
	db $49
	db $00
	db $43
	db $00
	db $3E
	db $00
	db $39
	db $00
	db $32
	db $00
	db $2C
	db $00
	db $27
	db $00
	db $20
	db $00
	db $1A
	db $00
	db $14
	db $00
	db $0E
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $FF
	db $F2
	db $FF
	db $00
	db $00
	db $E6
	db $FF
	db $E0
	db $FF
	db $D9
	db $FF
	db $D4
	db $FF
	db $CE
	db $FF
	db $C7
	db $FF
	db $C2
	db $FF
	db $BD
	db $FF
	db $B7
	db $FF
	db $B2
	db $FF
	db $AE
	db $FF
	db $A8
	db $FF
	db $A4
	db $FF
	db $A0
	db $FF
	db $9D
	db $FF
	db $A0
	db $FF
	db $93
	db $FF
	db $90
	db $FF
	db $8A
	db $FF
	db $88
	db $FF
	db $85
	db $FF
	db $81
	db $FF
	db $7F
	db $FF
	db $7B
	db $FF
	db $78
	db $FF
	db $77
	db $FF
	db $73
	db $FF
	db $70
	db $FF
	db $6F
	db $FF
	db $6C
	db $FF
	db $69
	db $FF
	db $69
	db $FF
	db $65
	db $FF
	db $62
	db $FF
	db $5F
	db $FF
	db $5E
	db $FF
	db $5B
	db $FF
	db $58
	db $FF
	db $57
	db $FF
	db $54
	db $FF
	db $51
	db $FF
	db $50
	db $FF
	db $4D
	db $FF
	db $4A
	db $FF
	db $47
	db $FF
	db $46
	db $FF
	db $44
	db $FF
	db $41
	db $FF
	db $40
	db $FF
	db $3E
	db $FF
	db $3C
	db $FF
	db $3A
	db $FF
	db $38
	db $FF
	db $38
	db $FF
	db $37
	db $FF
	db $36
	db $FF
	db $35
	db $FF
	db $34
	db $FF
	db $33
	db $FF
	db $32
	db $FF
	db $32
	db $FF
	db $33
	db $FF
	db $34
	db $FF
	db $35
	db $FF
	db $36
	db $FF
	db $37
	db $FF
	db $38
	db $FF
	db $38
	db $FF
	db $3A
	db $FF
	db $3C
	db $FF
	db $3E
	db $FF
	db $40
	db $FF
	db $41
	db $FF
	db $44
	db $FF
	db $46
	db $FF
	db $47
	db $FF
	db $4A
	db $FF
	db $4D
	db $FF
	db $50
	db $FF
	db $51
	db $FF
	db $54
	db $FF
	db $57
	db $FF
	db $58
	db $FF
	db $5B
	db $FF
	db $5E
	db $FF
	db $5F
	db $FF
	db $62
	db $FF
	db $65
	db $FF
	db $68
	db $FF
	db $69
	db $FF
	db $6C
	db $FF
	db $6F
	db $FF
	db $70
	db $FF
	db $73
	db $FF
	db $77
	db $FF
	db $78
	db $FF
	db $7B
	db $FF
	db $7F
	db $FF
	db $81
	db $FF
	db $85
	db $FF
	db $88
	db $FF
	db $8A
	db $FF
	db $90
	db $FF
	db $93
	db $FF
	db $98
	db $FF
	db $9D
	db $FF
	db $A0
	db $FF
	db $A4
	db $FF
	db $A8
	db $FF
	db $AE
	db $FF
	db $B2
	db $FF
	db $B7
	db $FF
	db $BD
	db $FF
	db $C2
	db $FF
	db $C7
	db $FF
	db $CE
	db $FF
	db $D4
	db $FF
	db $D9
	db $FF
	db $E0
	db $FF
	db $E6
	db $FF
	db $EC
data_5B50:
	db $FF
	db $F2
	db $FF
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $0E
	db $00
	db $14
	db $00
	db $1A
	db $00
	db $20
	db $00
	db $27
	db $00
	db $2C
	db $00
	db $32
	db $00
	db $39
	db $00
	db $3E
	db $00
	db $43
	db $00
	db $49
	db $00
	db $4E
	db $00
	db $52
	db $00
	db $58
	db $00
	db $5C
	db $00
	db $60
	db $00
	db $63
	db $00
	db $63
	db $00
	db $6D
	db $00
	db $70
	db $00
	db $76
	db $00
	db $78
	db $00
	db $7B
	db $00
	db $7F
	db $00
	db $81
	db $00
	db $85
	db $00
	db $88
	db $00
	db $89
	db $00
	db $8D
	db $00
	db $90
	db $00
	db $91
	db $00
	db $94
	db $00
	db $97
	db $00
	db $94
	db $00
	db $9B
	db $00
	db $9E
	db $00
	db $A1
	db $00
	db $A2
	db $00
	db $A5
	db $00
	db $A8
	db $00
	db $A9
	db $00
	db $AC
	db $00
	db $AF
	db $00
	db $B0
	db $00
	db $B3
	db $00
	db $B6
	db $00
	db $B9
	db $00
	db $BA
	db $00
	db $BC
	db $00
	db $B9
	db $00
	db $C0
	db $00
	db $C2
	db $00
	db $C4
	db $00
	db $C6
	db $00
	db $C8
	db $00
	db $C8
	db $00
	db $C9
	db $00
	db $CA
	db $00
	db $CB
	db $00
	db $CC
	db $00
	db $CD
	db $00
	db $CE
	db $00
; Function Name: Phase2Function_RadialWipe
; Description: This phase does the radial wipe and then some integrity checks
Phase2Function_RadialWipe:
	CALL ClearChars_07D2
	CALL PerformRadialWipeStep
	RET  NZ			; return if not finished
	LD   HL,function_0BDD			; function to check
	SUB  A
	LD   B,A
.func_xor_loop:
	XOR  (HL)
	INC  HL
	DJNZ .func_xor_loop
	ADD  A,$E4			; check value
	CALL NZ,function_0F11			; call if function has been tampered with
	LD   A,(FunctionChecksum)
	LD   HL,function_1734			; function to check
	LD   B,$14
.func_checksum_loop:
	ADD  A,(HL)
	INC  HL
	DJNZ .func_checksum_loop
	ADD  A,$77
	LD   (FunctionChecksum),A			; store checksum
	JP   AdvancePhase
data_5C00:
	db $E7
data_5C01:
	db $00
	db $E6
	db $00
	db $E5
	db $00
	db $E4
	db $00
	db $E3
	db $00
	db $E2
	db $00
	db $E1
	db $00
	db $E0
	db $00
	db $DE
	db $00
	db $DC
	db $00
	db $DA
	db $00
	db $D8
	db $00
	db $D6
	db $00
	db $D3
	db $00
	db $D1
	db $00
	db $CF
	db $00
	db $CC
	db $00
	db $C9
	db $00
	db $C6
	db $00
	db $C4
	db $00
	db $C1
	db $00
	db $BE
	db $00
	db $BC
	db $00
	db $B9
	db $00
	db $B6
	db $00
	db $B4
	db $00
	db $B1
	db $00
	db $AE
	db $00
	db $AB
	db $00
	db $A9
	db $00
	db $A6
	db $00
	db $A3
	db $00
	db $A1
	db $00
	db $9E
	db $00
	db $9A
	db $00
	db $98
	db $00
	db $95
	db $00
	db $91
	db $00
	db $8E
	db $00
	db $8A
	db $00
	db $87
	db $00
	db $84
	db $00
	db $7E
	db $00
	db $7A
	db $00
	db $75
	db $00
	db $6F
	db $00
	db $6C
	db $00
	db $67
	db $00
	db $62
	db $00
	db $5C
	db $00
	db $57
	db $00
	db $51
	db $00
	db $4B
	db $00
	db $45
	db $00
	db $3F
	db $00
	db $38
	db $00
	db $31
	db $00
	db $2B
	db $00
	db $24
	db $00
	db $1D
	db $00
	db $16
	db $00
	db $0F
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $FF
	db $F1
	db $FF
	db $00
	db $00
	db $E3
	db $FF
	db $DC
	db $FF
	db $D5
	db $FF
	db $CF
	db $FF
	db $C8
	db $FF
	db $C1
	db $FF
	db $BB
	db $FF
	db $B5
	db $FF
	db $AF
	db $FF
	db $A9
	db $FF
	db $A4
	db $FF
	db $9E
	db $FF
	db $99
	db $FF
	db $94
	db $FF
	db $91
	db $FF
	db $94
	db $FF
	db $86
	db $FF
	db $82
	db $FF
	db $7C
	db $FF
	db $79
	db $FF
	db $76
	db $FF
	db $72
	db $FF
	db $6F
	db $FF
	db $6B
	db $FF
	db $68
	db $FF
	db $66
	db $FF
	db $62
	db $FF
	db $5F
	db $FF
	db $5D
	db $FF
	db $5A
	db $FF
	db $57
	db $FF
	db $57
	db $FF
	db $52
	db $FF
	db $4F
	db $FF
	db $4C
	db $FF
	db $4A
	db $FF
	db $47
	db $FF
	db $44
	db $FF
	db $42
	db $FF
	db $3F
	db $FF
	db $3C
	db $FF
	db $3A
	db $FF
	db $37
	db $FF
	db $34
	db $FF
	db $31
	db $FF
	db $2F
	db $FF
	db $2D
	db $FF
	db $2A
	db $FF
	db $28
	db $FF
	db $26
	db $FF
	db $24
	db $FF
	db $22
	db $FF
	db $20
	db $FF
	db $1F
	db $FF
	db $1E
	db $FF
	db $1D
	db $FF
	db $1C
	db $FF
	db $1B
	db $FF
	db $1A
	db $FF
	db $19
	db $FF
	db $19
	db $FF
	db $1A
	db $FF
	db $1B
	db $FF
	db $1C
	db $FF
	db $1D
	db $FF
	db $1E
	db $FF
	db $1F
	db $FF
	db $20
	db $FF
	db $22
	db $FF
	db $24
	db $FF
	db $26
	db $FF
	db $28
	db $FF
	db $2A
	db $FF
	db $2D
	db $FF
	db $2F
	db $FF
	db $31
	db $FF
	db $34
	db $FF
	db $37
	db $FF
	db $3A
	db $FF
	db $3C
	db $FF
	db $3F
	db $FF
	db $42
	db $FF
	db $44
	db $FF
	db $47
	db $FF
	db $4A
	db $FF
	db $4C
	db $FF
	db $4F
	db $FF
	db $52
	db $FF
	db $55
	db $FF
	db $57
	db $FF
	db $5A
	db $FF
	db $5D
	db $FF
	db $5F
	db $FF
	db $62
	db $FF
	db $66
	db $FF
	db $68
	db $FF
	db $6B
	db $FF
	db $6F
	db $FF
	db $72
	db $FF
	db $76
	db $FF
	db $79
	db $FF
	db $7C
	db $FF
	db $82
	db $FF
	db $86
	db $FF
	db $8B
	db $FF
	db $91
	db $FF
	db $94
	db $FF
	db $99
	db $FF
	db $9E
	db $FF
	db $A4
	db $FF
	db $A9
	db $FF
	db $AF
	db $FF
	db $B5
	db $FF
	db $BB
	db $FF
	db $C1
	db $FF
	db $C8
	db $FF
	db $CF
	db $FF
	db $D5
	db $FF
	db $DC
	db $FF
	db $E3
	db $FF
	db $EA
	db $FF
	db $F1
	db $FF
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $0F
	db $00
	db $16
	db $00
	db $1D
	db $00
	db $24
	db $00
	db $2B
	db $00
	db $31
	db $00
	db $38
	db $00
	db $3F
	db $00
	db $45
	db $00
	db $4B
	db $00
	db $51
	db $00
	db $57
	db $00
	db $5C
	db $00
	db $62
	db $00
	db $67
	db $00
	db $6C
	db $00
	db $6F
	db $00
	db $6F
	db $00
	db $7A
	db $00
	db $7E
	db $00
	db $84
	db $00
	db $87
	db $00
	db $8A
	db $00
	db $8E
	db $00
	db $91
	db $00
	db $95
	db $00
	db $98
	db $00
	db $9A
	db $00
	db $9E
	db $00
	db $A1
	db $00
	db $A3
	db $00
	db $A6
	db $00
	db $A9
	db $00
	db $A6
	db $00
	db $AE
	db $00
	db $B1
	db $00
	db $B4
	db $00
	db $B6
	db $00
	db $B9
	db $00
	db $BC
	db $00
	db $BE
	db $00
	db $C1
	db $00
	db $C4
	db $00
	db $C6
	db $00
	db $C9
	db $00
	db $CC
	db $00
	db $CF
	db $00
	db $D1
	db $00
	db $D3
	db $00
	db $CF
	db $00
	db $D8
	db $00
	db $DA
	db $00
	db $DC
	db $00
	db $DE
	db $00
	db $E0
	db $00
	db $E1
	db $00
	db $E2
	db $00
	db $E3
	db $00
	db $E4
	db $00
	db $E5
	db $00
	db $E6
	db $00
	db $E7
	db $00
data_5E00:
	db $00
	db $01
	db $FF
	db $00
	db $FE
	db $00
	db $FD
	db $00
	db $FC
	db $00
	db $FB
	db $00
	db $FA
	db $00
	db $F8
	db $00
	db $F6
	db $00
	db $F4
	db $00
	db $F2
	db $00
	db $F0
	db $00
	db $ED
	db $00
	db $EA
	db $00
	db $E8
	db $00
	db $E5
	db $00
	db $E2
	db $00
	db $DF
	db $00
	db $DC
	db $00
	db $D9
	db $00
	db $D6
	db $00
	db $D3
	db $00
	db $D0
	db $00
	db $CD
	db $00
	db $CA
	db $00
	db $C7
	db $00
	db $C4
	db $00
	db $C1
	db $00
	db $BE
	db $00
	db $BB
	db $00
	db $B8
	db $00
	db $B5
	db $00
	db $B2
	db $00
	db $AF
	db $00
	db $AB
	db $00
	db $A8
	db $00
	db $A5
	db $00
	db $A1
	db $00
	db $9D
	db $00
	db $99
	db $00
	db $96
	db $00
	db $92
	db $00
	db $8C
	db $00
	db $87
	db $00
	db $82
	db $00
	db $7B
	db $00
	db $78
	db $00
	db $72
	db $00
	db $6C
	db $00
	db $66
	db $00
	db $60
	db $00
	db $59
	db $00
	db $53
	db $00
	db $4C
	db $00
	db $45
	db $00
	db $3E
	db $00
	db $36
	db $00
	db $2F
	db $00
	db $28
	db $00
	db $20
	db $00
	db $18
	db $00
	db $10
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $FF
	db $F0
	db $FF
	db $00
	db $00
	db $E0
	db $FF
	db $D8
	db $FF
	db $D1
	db $FF
	db $CA
	db $FF
	db $C2
	db $FF
	db $BB
	db $FF
	db $B4
	db $FF
	db $AD
	db $FF
	db $A7
	db $FF
	db $A0
	db $FF
	db $9A
	db $FF
	db $94
	db $FF
	db $8E
	db $FF
	db $88
	db $FF
	db $85
	db $FF
	db $88
	db $FF
	db $79
	db $FF
	db $74
	db $FF
	db $6E
	db $FF
	db $6A
	db $FF
	db $67
	db $FF
	db $63
	db $FF
	db $5F
	db $FF
	db $5B
	db $FF
	db $58
	db $FF
	db $55
	db $FF
	db $51
	db $FF
	db $4E
	db $FF
	db $4B
	db $FF
	db $48
	db $FF
	db $45
	db $FF
	db $45
	db $FF
	db $3F
	db $FF
	db $3C
	db $FF
	db $39
	db $FF
	db $36
	db $FF
	db $33
	db $FF
	db $30
	db $FF
	db $2D
	db $FF
	db $2A
	db $FF
	db $27
	db $FF
	db $24
	db $FF
	db $21
	db $FF
	db $1E
	db $FF
	db $1B
	db $FF
	db $18
	db $FF
	db $16
	db $FF
	db $13
	db $FF
	db $10
	db $FF
	db $0E
	db $FF
	db $0C
	db $FF
	db $0A
	db $FF
	db $08
	db $FF
	db $06
	db $FF
	db $05
	db $FF
	db $04
	db $FF
	db $03
	db $FF
	db $02
	db $FF
	db $01
	db $FF
	db $00
	db $FF
	db $00
	db $FF
	db $01
	db $FF
	db $02
	db $FF
	db $03
	db $FF
	db $04
	db $FF
	db $05
	db $FF
	db $06
	db $FF
	db $08
	db $FF
	db $0A
	db $FF
	db $0C
	db $FF
	db $0E
	db $FF
	db $10
	db $FF
	db $13
	db $FF
	db $16
	db $FF
	db $18
	db $FF
	db $1B
	db $FF
	db $1E
	db $FF
	db $21
	db $FF
	db $24
	db $FF
	db $27
	db $FF
	db $2A
	db $FF
	db $2D
	db $FF
	db $30
	db $FF
	db $33
	db $FF
	db $36
	db $FF
	db $39
	db $FF
	db $3C
	db $FF
	db $3F
	db $FF
	db $42
	db $FF
	db $45
	db $FF
	db $48
	db $FF
	db $4B
	db $FF
	db $4E
	db $FF
	db $51
	db $FF
	db $55
	db $FF
	db $58
	db $FF
	db $5B
	db $FF
	db $5F
	db $FF
	db $63
	db $FF
	db $67
	db $FF
	db $6A
	db $FF
	db $6E
	db $FF
	db $74
	db $FF
	db $79
	db $FF
	db $7E
	db $FF
	db $85
	db $FF
	db $88
	db $FF
	db $8E
	db $FF
	db $94
	db $FF
	db $9A
	db $FF
	db $A0
	db $FF
	db $A7
	db $FF
	db $AD
	db $FF
	db $B4
	db $FF
	db $BB
	db $FF
	db $C2
	db $FF
	db $CA
	db $FF
	db $D1
	db $FF
	db $D8
	db $FF
	db $E0
	db $FF
	db $E8
	db $FF
	db $F0
	db $FF
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $10
	db $00
	db $18
	db $00
	db $20
	db $00
	db $28
	db $00
	db $2F
	db $00
	db $36
	db $00
	db $3E
	db $00
	db $45
	db $00
	db $4C
	db $00
	db $53
	db $00
	db $59
	db $00
	db $60
	db $00
	db $66
	db $00
	db $6C
	db $00
	db $72
	db $00
	db $78
	db $00
	db $7B
	db $00
	db $7B
	db $00
	db $87
	db $00
	db $8C
	db $00
	db $92
	db $00
	db $96
	db $00
	db $99
	db $00
	db $9D
	db $00
	db $A1
	db $00
	db $A5
	db $00
	db $A8
	db $00
	db $AB
	db $00
	db $AF
	db $00
	db $B2
	db $00
	db $B5
	db $00
	db $B8
	db $00
	db $BB
	db $00
	db $B8
	db $00
	db $C1
	db $00
	db $C4
	db $00
	db $C7
	db $00
	db $CA
	db $00
	db $CD
	db $00
	db $D0
	db $00
	db $D3
	db $00
	db $D6
	db $00
	db $D9
	db $00
	db $DC
	db $00
	db $DF
	db $00
	db $E2
	db $00
	db $E5
	db $00
	db $E8
	db $00
	db $EA
	db $00
	db $E5
	db $00
	db $F0
	db $00
	db $F2
	db $00
	db $F4
	db $00
	db $F6
	db $00
	db $F8
	db $00
	db $FA
	db $00
	db $FB
	db $00
	db $FC
	db $00
	db $FD
	db $00
	db $FE
	db $00
	db $FF
	db $00
	db $00
	db $01
