	db $00
	db $00
	db $00
	db $0D
	db $05
	db $23
	db $0D
LAST-K:	;Last key pressed
	db $0D
REPDEL:	;Key repeat delay
	db $23
REPPER:	;Key repeat delay
	db $05
DEFADD:
	db $00
	db $00
K-DATA:
	db $00
TVDATA:
	db $16
TVDATA+1:
	db $14
STRMS:	;Addresses of channels attached to streams
	db $01
	db $00
	db $06
	db $00
	db $0B
	db $00
data_5C16:
	db $01
	db $00
	db $01
	db $00
	db $06
	db $00
	db $10
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
CHARS:	;256 less than the current character set
	dw $ACD4
RASP:	;Length of warning buzz
	db $40
PIP:	;Length of keyboard click
	db $00
ERR-NR:
	db $FF
FLAGS:
	db $CC
TVFLAG:
	db $00
ERR-SP:
	db $17
	db $5E
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FF
data_5C45:
	db $0A
	db $00
	db $06
BORDCR:
	db $38
	db $00
	db $00
VARS:
	dw $5D0A
DEST:
	dw $0000
CHANS:
	dw $5CB6
CHURCHL:
	dw $5CBB
PROG:
	dw $5CCB
	dw $5D0A
DATADD:
	dw $5CCA
E-LINE:
	dw $5D0B
K-CUR:
	dw $5D0E
CH-ADD:
	dw $5D09
X-PTR:
	dw $5D65
WORKSP:
	dw $5D10
STKBOT:
	dw $5D10
STKEND:
	dw $5D10
	db $2D
data_5C68:
	db $92
	db $5C
	db $00
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B6
	db $1A
	db $00
	db $00
FRAMES:
	db $93
data_5C79:
	db $04
data_5C7A:
	db $00
data_5C7B:
	db $AC
	db $AE
	db $00
	db $00
	db $21
data_5C80:
	db $00
	db $5B
data_5C82:
	db $21
	db $17
DF-CC:
	dw $508A
DF-CCL:
	dw $50E0
S-POSN:
	db $17
	db $04
S-POSNL:
	db $21
	db $17
SCR-CT:
	db $03
data_5C8D:
	db $38
	db $00
data_5C8F:
	db $44
	db $00
data_5C91:
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
data_5CB0:
	db $00
	db $00
data_5CB2:
	db $1A
	db $5E
data_5CB4:
	db $FF
	db $FF
	db $F4
	db $09
	db $A8
	db $10
	db $4B
	db $F4
	db $09
	db $C4
	db $15
	db $53
	db $81
	db $0F
	db $C4
	db $15
	db $52
	db $F4
	db $09
	db $C4
	db $15
	db $50
	db $80
	db $00
	db $0A
	db $3B
	db $00
	db $FD
	db $32
	db $34
	db $30
	db $39
	db $30
	db $0E
	db $00
	db $00
	db $1A
	db $5E
	db $00
	db $3A
	db $EF
	db $22
	db $22
	db $AF
	db $3A
	db $FB
	db $3A
	db $EF
	db $22
	db $22
	db $AF
	db $31
	db $36
	db $33
	db $38
	db $34
	db $0E
	db $00
	db $00
	db $00
	db $40
	db $00
	db $3A
	db $F2
	db $30
	db $0E
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3A
	db $F9
	db $C0
	db $34
	db $34
	db $31
	db $30
	db $30
	db $0E
	db $00
	db $00
	db $44
	db $AC
	db $00
	db $0D
	db $80
	db $EF
	db $22
	db $22
	db $0D
	db $80
	db $00
	db $00
	db $44
	db $AC
	db $00
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $00
	db $00
	db $00
	db $40
	db $00
	db $00
	db $03
	db $73
	db $74
	db $61
	db $72
	db $2F
	db $73
	db $63
	db $72
	db $6E
	db $20
	db $00
	db $1B
	db $50
	db $C3
	db $00
	db $80
	db $00
	db $00
	db $00
	db $40
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
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $CE
	db $0B
	db $CE
	db $0B
	db $89
	db $50
	db $18
	db $04
	db $DC
	db $0A
	db $FE
	db $15
	db $58
	db $27
	db $21
	db $00
	db $09
	db $DA
	db $49
	db $01
	db $23
	db $01
	db $04
	db $00
	db $44
	db $6A
	db $D6
	db $5E
	db $76
	db $1B
	db $03
	db $13
	db $00
	db $3E
	db $5E
	db $24
	db $5E
	db $24
	db $5E
	db $24
	db $5E
	db $24
	db $5E
	db $18
	db $5B
	db $C3
	db $3F
	db $69
label_5E29:
	JP label_64A0	;[label_64A0]
	db $DA
	db $D3
	db $76
	db $6F
	db $1A
	db $08
	db $FB
	db $FC
	db $94
	db $9D
	db $24
	db $35
	db $9B
	db $8B
	db $1C
	db $15
	db $D0
	db $B0
	db $46
	db $07
	db $85
	db $9E
	db $EE
	db $F1
	db $2A
	db $2B
	db $EF
	db $1F
	db $28
	db $29
	db $71
	db $61
	db $39
	db $3A
	db $E9
	db $BA
KeyItemRoomLocations:
	db $04
	db $14
	db $54
	db $5B
	db $4B
	db $63
	db $64
	db $7B
data_5E58:
	dw $0704
data_5E59:
	db $02
data_5E5B:
	db $05
	db $35
	db $38
	db $36
	db $37
	db $30
	db $31
	db $32
	db $33
	db $34
	db $35
	db $4F
	db $50
	db $41
	db $51
	db $4D
	db $51
	db $57
	db $45
	db $52
	db $54
	db $2A
	db $00
	db $00
	db $00
	db $18
	db $18
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $61
	db $6F
	db $4F
	db $4F
	db $41
	db $7F
label_5E81:
	LD HL,$ACD4	;[function_ABF0 + 228]
	LD (CHARS),HL	;[CHARS]
	LD HL,TVFLAG	;[TVFLAG]
	RES 0,(HL)	;set bit 0 to 0
	LD HL,$5E71	;[data_5E5B + 22]
	LD (data_5C7B),HL	;[data_5C7B]
	LD A,$05
	CALL function_6615	;[function_6615]
	LD BC,$1609	;[CHAN_OPEN + 8]	;set coords
	LD L,$88	;first half of (C) section on title page
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD C,$11	;xpos 
	INC L	;second half of (C) section on title page
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD A,$07
	LD (data_5E59),A	;[data_5E59]
	CALL DrawText	;[DrawText]
	ascii ''
	db $00
	ascii 'S'
	db $90
	db $54
	db $90
	db $41
	db $90
	db $52
	db $90
	db $51
	db $90
	db $55
	db $90
	db $41
	db $90
	db $4B
	db $90
	db $45
	db $FF
	LD HL,$AEAC	;[function_ABF0 + 700]
	LD (data_5C7B),HL	;[data_5C7B]
	CALL DrawMainMenu	;[DrawMainMenu]
	LD B,$03
	CALL function_6600	;[function_6600]
	JP MainMenuLoop	;[MainMenuLoop]
DrawMainMenu:
	XOR A
	LD (data_5E5B),A	;[data_5E5B]
	CALL function_5FB4	;[function_5FB4]
	CALL DrawText	;[DrawText]
	ascii '1.KEMPSTON'	;string data is after function call
	db $FF
	CALL DrawJoystickText	;[DrawJoystickText]
	CALL function_5FB4	;[function_5FB4]
	CALL DrawText	;[DrawText]
	ascii '2.CURSOR'
	db $FF
	CALL DrawJoystickText	;[DrawJoystickText]
	CALL function_5FB4	;[function_5FB4]
	CALL DrawText	;[DrawText]
	ascii '<cr>
3.SINCLAIR ZX2'
	db $FF
	CALL DrawJoystickText	;[DrawJoystickText]
	CALL function_5FB4	;[function_5FB4]
	CALL DrawText	;[DrawText]
	ascii '4.'
	db $FF
	LD HL,$5E66	;[data_5E5B + 11]
	CALL DrawKeyboardText	;[DrawKeyboardText]
	CALL function_5FB4	;[function_5FB4]
	CALL DrawText	;[DrawText]
	ascii '5.UDK '
	db $FF
	LD HL,$5E6B	;[data_5E5B + 16]
	CALL DrawKeyboardText	;[DrawKeyboardText]
	CALL DrawText	;[DrawText]
	ascii '6.DEFINE YOUR OWN KEYS0.START GAMEQ.QUIT'
	db $FF
	RET
DrawJoystickText:
	CALL DrawText	;[DrawText]
	ascii ' JOYSTICK'
	db $FF
	RET
DrawKeyboardText:
	CALL DrawText	;[DrawText]
	ascii 'KEYBOARD ... '
	db $FF
	LD B,$05
label_5FA7:
	LD A,(HL)
	INC HL
	LD (data_5FAF),A	;[data_5FAF]
	CALL DrawText	;[DrawText]
data_5FAF:
	ascii 'T'
	db $FF
	DJNZ label_5FA7	;[label_5FA7]
	RET
function_5FB4:
	LD HL,data_5E5B	;[data_5E5B]
	INC (HL)
	LD A,$03
	LD (data_5FCD),A	;[data_5FCD]
	LD A,(data_5E58)	;[data_5E58]
	CP (HL)
	JR NZ,label_5FC9	;[label_5FC9]
	LD A,(data_5E59)	;[data_5E59]
	LD (data_5FCD),A	;[data_5FCD]
label_5FC9:
	CALL DrawText	;[DrawText]
	db $10
data_5FCD:
	db $03
	db $FF
	RET
function_5FD0:
	XOR A
	CALL function_5FE2	;[function_5FE2]
	LD HL,ScreenPixels	;[ScreenPixels]
	LD E,A
	LD D,$19
label_5FDA:
	LD (HL),A
	INC HL
	DEC DE
	CP D
	JR NZ,label_5FDA	;[label_5FDA]
	LD A,$07
function_5FE2:
	LD HL,ScreenAttributes	;[ScreenAttributes]
	LD D,$03
label_5FE7:
	LD B,$00
label_5FE9:
	LD (HL),A
	INC HL
	DJNZ label_5FE9	;[label_5FE9]
	DEC D
	JR NZ,label_5FE7	;[label_5FE7]
	XOR A
	OUT ($FE),A
	RET
MainMenuLoop:
	CALL DrawMainMenu	;[DrawMainMenu]
	LD HL,$5E5A	;[data_5E59 + 1]
	DEC (HL)
	JR NZ,label_6021	;[label_6021]
	LD (HL),$02
	DEC HL
	LD A,$09
	SUB (HL)
	LD (HL),A
	CP $07
	JR NZ,label_6021	;[label_6021]
	XOR A
	LD (data_EA61),A	;[data_EA61]
	LD BC,$001E	;[TEST_CHAR + 2]
	LD L,$90	;eye platform
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	CP $28
	JR NC,label_6021	;[label_6021]
	CALL DrawTitleCornerPlatforms	;[DrawTitleCornerPlatforms]
label_6021:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $30
	JP C,MainMenuLoop	;[MainMenuLoop]
	CP $51	;Q to quit
	JP Z,Menu_QuitGame	;[Menu_QuitGame]
	CP $37
	JR NC,MainMenuLoop	;[MainMenuLoop]
	CP $36
	JP Z,Menu_DefineKeys	;[Menu_DefineKeys]
	CP $30
	JP Z,label_629D	;[label_629D]
	SUB $30
	CP $01
	JR NZ,label_6051	;[label_6051]
	LD B,$00
label_6044:
	LD A,$E0
	OUT ($1F),A
	IN A,($1F)
	AND $E0
	JR NZ,label_605D	;[label_605D]
	DJNZ label_6044	;[label_6044]
	INC A
label_6051:
	LD HL,data_5E58	;[data_5E58]
	CP (HL)
	JR Z,label_605D	;[label_605D]
	LD (HL),A
	LD A,$0C
	CALL function_D7C0	;[function_D7C0]
label_605D:
	JP MainMenuLoop	;[MainMenuLoop]
Menu_QuitGame:
	LD A,$04
	CALL function_6615	;[function_6615]
	CALL DrawText	;[DrawText]
Quit_Game_Screen_Text:
	ascii '	QUIT THE GAME	ARE YOU SURE...	Y OR N...'
	db $FF
label_6099:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	AND A
	JR NZ,label_6099	;[label_6099]
label_609F:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	AND A
	JR Z,label_609F	;[label_609F]
	CP $59
	JP NZ,label_5E81	;[label_5E81]
	CALL DrawText	;[DrawText]
	ascii 'SAY GOODBYE TO OLLY...'
	db $FF
	LD BC,$0C0C	;[PO_MSG + 2]
	LD L,$56
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	db $06
	db $FF
	db $76
	db $10
	db $FD
	db $21
	db $30
	db $75
	db $36
	db $00
	db $23
	db $AF
	db $BC
	db $20
	db $F9
	db $C7
	db $31
	db $32
	db $33
	db $34
	db $35
	db $36
	db $37
	db $38
	db $39
	db $30
	db $51
	db $57
	db $45
	db $52
	db $54
	db $59
	db $55
	db $49
	db $4F
	db $50
	db $41
	db $53
	db $44
	db $46
	db $47
	db $48
	db $4A
	db $4B
	db $4C
	db $5C
	db $5B
	db $5A
	db $58
	db $43
	db $56
	db $42
	db $4E
	db $4D
	db $5D
	db $2A
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
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
function_6131:
	LD HL,$6109	;[label_609F + 106]
	LD B,$04
	LD D,$00
label_6138:
	LD C,$0A
	LD A,$04
	SUB B
	CP $03
	JR NZ,label_6142	;[label_6142]
	XOR A
label_6142:
	LD E,A
label_6143:
	XOR A
	LD (data_6176),A	;[data_6176]
	INC A
	LD (data_6171),A	;[data_6171]
	LD A,(HL)
	LD (data_6177),A	;[data_6177]
	CP $90
	JR C,label_615C	;[label_615C]
	LD A,$02
	LD (data_6171),A	;[data_6171]
	DEC A
	LD (data_6176),A	;[data_6176]
label_615C:
	LD A,D
	LD (data_6173),A	;[data_6173]
	INC A
	LD (data_617C),A	;[data_617C]
	LD A,E
	LD (data_6174),A	;[data_6174]
	LD (data_617D),A	;[data_617D]
	CALL DrawText	;[DrawText]
	db $10
	db $07
	db $11
data_6171:
	db $01
	db $16
data_6173:
	db $01
data_6174:
	db $01
	db $13
data_6176:
	db $01
data_6177:
	db $00
	db $10
	db $00
	db $2B
	db $16
data_617C:
	db $01
data_617D:
	db $01
	db $8C
	db $2C
	db $10
	db $05
	db $13
	db $01
	db $11
	db $00
	db $FF
	INC E
	INC E
	INC E
	INC HL
	DEC C
	JR NZ,label_6143	;[label_6143]
	INC D
	INC D
	INC D
	DJNZ label_6138	;[label_6138]
	RET
Menu_DefineKeys:
	CALL function_5FD0	;[function_5FD0]
	LD HL,$60E1	;[label_609F + 66]
	LD DE,$6109	;[label_609F + 106]
	LD B,$28
label_619F:
	LD A,(HL)
	LD (DE),A
	INC HL
	INC DE
	DJNZ label_619F	;[label_619F]
	CALL function_6131	;[function_6131]
	XOR A
	LD (data_6230),A	;[data_6230]
	CALL DrawText	;[DrawText]
	db $16
	db $0E
	db $00
	db $48
	db $49
	db $54
	db $20
	db $4B
	db $45
	db $59
	db $20
	db $52
	db $45
	db $51
	db $55
	db $49
	db $52
	db $45
	db $44
	db $20
	db $2E
	db $2E
	db $2E
	db $16
	db $11
	db $05
	db $4C
	db $45
	db $46
	db $54
	db $20
	db $20
	db $FF
	CALL function_6233	;[function_6233]
	CALL DrawText	;[DrawText]
	db $16
	db $11
	db $14
	db $52
	db $49
	db $47
	db $48
	db $54
	db $20
	db $FF
	CALL function_6233	;[function_6233]
	CALL DrawText	;[DrawText]
	db $16
	db $13
	db $05
	db $44
	db $4F
	db $57
	db $4E
	db $20
	db $20
	db $FF
	CALL function_6233	;[function_6233]
	CALL DrawText	;[DrawText]
	db $16
	db $13
	db $14
	db $55
	db $50
	db $20
	db $20
	db $20
	db $20
	db $FF
	CALL function_6233	;[function_6233]
	CALL DrawText	;[DrawText]
	db $16
	db $15
	db $05
	db $46
	db $49
	db $52
	db $45
	db $20
	db $20
	db $FF
	CALL function_6233	;[function_6233]
	CALL DrawText	;[DrawText]
	db $16
	db $15
	db $14
	db $50
	db $41
	db $55
	db $53
	db $45
	db $20
	db $FF
	CALL function_6233	;[function_6233]
	LD B,$78
label_6225:
	HALT
	DJNZ label_6225	;[label_6225]
	LD A,$05
	LD (data_5E58),A	;[data_5E58]
	JP label_5E81	;[label_5E81]
data_6230:
	db $00
	db $32
data_6232:
	db $07
function_6233:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $00
	JR NZ,function_6233	;[function_6233]
label_623A:
	LD HL,$6231	;[data_6230 + 1]
	DEC (HL)
	JR NZ,label_6247	;[label_6247]
	LD (HL),$32
	INC HL
	LD A,$09
	SUB (HL)
	LD (HL),A
label_6247:
	LD A,(data_6232)	;[data_6232]
	LD (data_6251),A	;[data_6251]
	CALL DrawText	;[DrawText]
	db $10
data_6251:
	db $01
	db $2D
	db $08
	db $10
	db $07
	db $FF
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $00
	JR Z,label_623A	;[label_623A]
	CP $21
	JR NC,label_626A	;[label_626A]
	ADD A,$5A
	CP $7A
	JR NZ,label_626A	;[label_626A]
	LD A,$2A
label_626A:
	LD B,$28
	LD HL,$6109	;[label_609F + 106]
label_626F:
	CP (HL)
	JR Z,label_6277	;[label_6277]
	INC HL
	DJNZ label_626F	;[label_626F]
	JR label_623A	;[label_623A]
label_6277:
	PUSH AF
	LD A,(data_6230)	;[data_6230]
	ADD A,$90
	PUSH AF
	LD B,$01
	CALL label_5FA7	;[label_5FA7]
	DEC HL
	POP AF
	LD (HL),A
	POP AF
	LD HL,data_6230	;[data_6230]
	LD D,$00
	LD E,(HL)
	INC (HL)
	LD HL,$5E6B	;[data_5E5B + 16]
	ADD HL,DE
	LD (HL),A
	LD A,$01
	CALL function_D7C0	;[function_D7C0]
	CALL function_6131	;[function_6131]
	RET
data_629C:
	db $00
label_629D:
	XOR A
	LD (data_629C),A	;[data_629C]
	LD HL,$C567	;[PauseNotPressed + 1]
	LD (HL),A
	LD A,(data_5E58)	;[data_5E58]
	CP $01
	JR NZ,label_62AF	;[label_62AF]
	LD (HL),A
	JR label_62D0	;[label_62D0]
label_62AF:
	LD C,A
	RLCA
	RLCA
	ADD A,C
	LD D,$00
	LD E,A
	LD HL,$5E52	;[KeyItemRoomLocations + 2]
	ADD HL,DE
	LD B,$05
label_62BC:
	PUSH HL
	PUSH BC
	CALL function_6307	;[function_6307]
	LD HL,data_629C	;[data_629C]
	INC (HL)
	POP BC
	POP HL
	INC HL
	DJNZ label_62BC	;[label_62BC]
	LD HL,$5E70	;[data_5E5B + 21]
	CALL function_6307	;[function_6307]
label_62D0:
	JP label_6351	;[label_6351]
	db $5B
	db $5A
	db $58
	db $43
	db $56
	db $41
	db $53
	db $44
	db $46
	db $47
	db $51
	db $57
	db $45
	db $52
	db $54
	db $31
	db $32
	db $33
	db $34
	db $35
	db $30
	db $39
	db $38
	db $37
	db $36
	db $50
	db $4F
	db $49
	db $55
	db $59
	db $5C
	db $4C
	db $4B
	db $4A
	db $48
	db $2A
	db $5D
	db $4D
	db $4E
	db $42
	db $79
	db $C5
	db $9A
	db $C5
	db $84
	db $C5
	db $8F
	db $C5
	db $A5
	db $C5
	db $5B
	db $C5
function_6307:
	LD B,$FE
	LD A,(HL)
	LD HL,$62D3	;[label_62D0 + 3]
label_630D:
	LD C,$42
	LD E,$05
label_6311:
	CP (HL)
	JR Z,label_6322	;[label_6322]
	INC HL
	EX AF,AF'
	LD A,C
	ADD A,$08
	LD C,A
	EX AF,AF'
	DEC E
	JR NZ,label_6311	;[label_6311]
	RLC B
	JR C,label_630D	;[label_630D]
label_6322:
	LD HL,$62FB	;[label_62D0 + 43]
	LD A,(data_629C)	;[data_629C]
	RLCA
	LD D,$00
	LD E,A
	ADD HL,DE
	LD E,(HL)
	INC HL
	LD D,(HL)
	EX DE,HL
	LD (HL),$FE
	INC HL
	LD (HL),B
	INC HL
	INC HL
	INC HL
	INC HL
	CP $0A
	JR NZ,label_6341	;[label_6341]
	LD A,C
	ADD A,$05
	LD C,A
label_6341:
	LD (HL),C
	RET
	db $0A
	db $08
	db $0E
	db $04
	db $0F
	db $FF
	db $10
	db $32
	db $11
	db $FF
	db $29
	db $09
	db $00
	db $00
label_6351:
	LD B,$2D
	LD HL,data_D2BE	;[data_D2BE]
	LD DE,$6343	;[label_6341 + 2]
	LD C,$00
label_635B:
	LD (HL),$00
	LD A,(DE)
	CP C
	JR NZ,label_6365	;[label_6365]
	INC DE
	LD A,(DE)
	INC DE
	LD (HL),A
label_6365:
	INC HL
	INC C
	DJNZ label_635B	;[label_635B]
	LD HL,(FRAMES)	;[FRAMES]
	LD (RandomNo1),HL	;[RandomNo1]
	LD (data_D2C6),HL	;[data_D2C6]
	LD HL,$94E9	;[CollectableItemInfo + 1]
	LD (CollectableItemPointer),HL	;[CollectableItemPointer]
	LD HL,KeyItemRoomLocations	;[KeyItemRoomLocations]
	LD BC,$020F	;[KEYTABLE_A + 10]	;set loop count to 2 & item number to 15 (access card)
KeyItemPlacementLoop:
	PUSH HL	;store pointer
	LD D,$00
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]	;generate random number 0-3
	LD A,(RandomNo1)	;[RandomNo1]
	AND $03
	LD E,A	;store in E
	ADD HL,DE	;use as offset
	CALL PlaceItemInScreen	;[PlaceItemInScreen]
	INC C	;increment item number
	POP HL	;restore pointer
	INC HL	;advance 4 bytes
	INC HL
	INC HL
	INC HL
	DJNZ KeyItemPlacementLoop	;[KeyItemPlacementLoop]
	LD B,$05
label_6398:
	PUSH BC
label_6399:
	LD HL,$D2DE	;[data_D2DC + 2]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_63A2:
	SUB $0F
	JR NC,label_63A2	;[label_63A2]
	ADD A,$98
	CP $8F
	JR C,label_63AE	;[label_63AE]
	ADD A,$0B
label_63AE:
	LD B,$09
label_63B0:
	CP (HL)
	JR Z,label_6399	;[label_6399]
	INC HL
	DJNZ label_63B0	;[label_63B0]
	LD C,A
label_63B7:
	LD HL,$D2DE	;[data_D2DC + 2]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_63C0:
	SUB $09
	JR NC,label_63C0	;[label_63C0]
	ADD A,$09
	LD E,A
	LD D,$00
	ADD HL,DE
	XOR A
	CP (HL)
	JR NZ,label_63B7	;[label_63B7]
	LD (HL),C
	POP BC
	DJNZ label_6398	;[label_6398]
	LD B,$09
	LD HL,$D2DE	;[data_D2DC + 2]
label_63D7:
	XOR A
	CP (HL)
	JR NZ,label_63DF	;[label_63DF]
	LD A,$89
	SUB B
	LD (HL),A
label_63DF:
	INC HL
	DJNZ label_63D7	;[label_63D7]
	LD HL,$5E2C	;[label_5E29 + 3]
	LD B,$02
label_63E7:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	AND $07
	LD C,A
	LD D,$09
label_63F2:
	PUSH DE
	PUSH HL
	INC C
	LD A,C
	CP $09
	JR C,label_63FB	;[label_63FB]
	XOR A
label_63FB:
	LD C,A
	PUSH BC
	LD DE,$D2DD	;[data_D2DC + 1]
	INC A
label_6401:
	INC DE
	DEC A
	JR NZ,label_6401	;[label_6401]
	LD A,(DE)
	DEC B
	JR NZ,label_6416	;[label_6416]
	LD C,A
	LD A,(data_DAC1)	;[data_DAC1]
	CP $96
	LD A,C
	JR C,label_6416	;[label_6416]
	AND $07
	ADD A,$1A
label_6416:
	AND $7F
	LD C,A
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	CP $7F
	JR C,label_6424	;[label_6424]
	INC HL
label_6424:
	CALL PlaceItemInScreen	;[PlaceItemInScreen]
	POP BC
	POP HL
	POP DE
	INC HL
	INC HL
	DEC D
	JR NZ,label_63F2	;[label_63F2]
	DJNZ label_63E7	;[label_63E7]
	LD HL,$A350	;[label_A345 + 11]
	LD B,$80
label_6436:
	LD (HL),$FF
	INC HL
	DJNZ label_6436	;[label_6436]
	LD HL,$95F1	;[RoomNumbersOf??? + 1]
	LD B,$08
label_6440:
	LD A,(HL)
	OR $01
	LD (HL),A
	INC HL
	INC HL
	DJNZ label_6440	;[label_6440]
	LD B,$0C
	LD HL,$D413	;[label_D406 + 13]
	XOR A
label_644E:
	LD (HL),A
	INC HL
	DJNZ label_644E	;[label_644E]
	LD HL,Sprite0	;[Sprite0]
	LD B,$C0
label_6457:
	LD (HL),A
	INC HL
	DJNZ label_6457	;[label_6457]
	LD HL,$5B00	;[ScreenAttributes + 768]
	LD B,$20
label_6460:
	LD (HL),$40
	INC HL
	DJNZ label_6460	;[label_6460]
	CALL function_A423	;[function_A423]
	LD HL,$3F88	;[label_3F87 + 1]
	LD (Sprite0_XPixelPos),HL	;[Sprite0_XPixelPos]
	LD HL,$E734	;[PlayerSpriteData + 1728]
	LD (Sprite0_SpriteAddr2),HL	;[Sprite0_SpriteAddr2]
	XOR A
	LD (Sprite0_OnTransport),A	;[Sprite0_OnTransport]
	INC A
	LD (data_DD2B),A	;[data_DD2B]
	LD HL,START
	LD (FRAMES),HL	;[FRAMES]
	LD (data_5C79),HL	;[data_5C79]
	JP label_666D	;[label_666D]
CollectableItemPointer:
	dw $94E9
PlaceItemInScreen:	;HL points to room number?, C is item number
	LD E,(HL)
	LD D,$00
	OR A
	RL E
	RR D
	LD HL,(CollectableItemPointer)	;[CollectableItemPointer]
	LD (HL),D	;room number high bit
	INC HL
	LD (HL),E	;room number
	INC HL
	LD (HL),C	;item number
	INC HL
	INC HL
	LD (CollectableItemPointer),HL	;[CollectableItemPointer]
	RET
label_64A0:
	LD A,$01
	LD (data_D41B),A	;[data_D41B]
	CALL function_D425	;[function_D425]
	CALL function_679C	;[function_679C]
	LD HL,$D413	;[label_D406 + 13]
	LD DE,RandomNo1	;[RandomNo1]
	LD BC,$0003	;[START + 3]
	LDIR
	LD B,$03
label_64B8:
	LD (DE),A
	INC DE
	DJNZ label_64B8	;[label_64B8]
	LD HL,$0303	;[K_NEW + 18]
	LD (RNGCounter1),HL	;[RNGCounter1]
	LD B,$1E
label_64C4:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	DJNZ label_64C4	;[label_64C4]
	LD B,$02
	LD DE,$D416	;[label_D406 + 16]
label_64CE:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_64D4:
	SUB $0A
	JR NC,label_64D4	;[label_64D4]
	ADD A,$0A
	LD (DE),A
	INC DE
	DJNZ label_64CE	;[label_64CE]
	LD A,(data_DAC1)	;[data_DAC1]
	AND $01
	JR Z,label_64E7	;[label_64E7]
	LD A,$05
label_64E7:
	LD (DE),A
	CALL function_D425	;[function_D425]
	RET
function_64EC:
	LD B,$06
label_64EE:
	LD A,(DE)
	CP (HL)
	RET NZ
	INC DE
	INC HL
	DJNZ label_64EE	;[label_64EE]
	LD A,$01
	CP $08
	RET
	db $53
	db $54
	db $41
	db $31
	db $30
	db $39
	db $38
	db $32
	db $35
	db $1F
	db $54
	db $41
	db $52
	db $30
	db $39
	db $33
	db $39
	db $30
	db $30
	db $17
	db $41
	db $52
	db $51
	db $30
	db $38
	db $32
	db $39
	db $37
	db $35
	db $13
	db $52
	db $51
	db $55
	db $30
	db $36
	db $32
	db $30
	db $35
	db $30
	db $11
	db $51
	db $55
	db $41
	db $30
	db $34
	db $36
	db $31
	db $32
	db $35
	db $0D
	db $55
	db $41
	db $4B
	db $30
	db $33
	db $30
	db $32
	db $31
	db $30
	db $0B
	db $41
	db $4B
	db $45
	db $30
	db $31
	db $34
	db $32
	db $37
	db $35
	db $07
	db $4B
	db $45
	db $53
	db $30
	db $30
	db $39
	db $38
	db $37
	db $35
	db $05
data_654A:
	db $00
label_654B:
	LD A,$03
	CALL function_6615	;[function_6615]
	CALL DrawText	;[DrawText]
	db $10
	db $05
	db $16
	db $03
	db $09
	db $43
	db $4F
	db $52
	db $45
	db $20
	db $4F
	db $46
	db $20
	db $48
	db $45
	db $52
	db $4F
	db $45
	db $53
	db $FF
	LD HL,$64FA	;[label_64EE + 12]
	LD B,$08
	LD C,$06
label_656E:
	PUSH BC
	LD A,$39
	SUB B
	LD (data_6592),A	;[data_6592]
	LD A,C
	LD (data_6590),A	;[data_6590]
	LD DE,$6594	;[data_6592 + 2]
	LD B,$0A
label_657E:
	LD A,B
	CP $07
	JR Z,label_6586	;[label_6586]
	LD A,(HL)
	INC HL
	LD (DE),A
label_6586:
	INC DE
	DJNZ label_657E	;[label_657E]
	CALL function_D55F	;[function_D55F]
	CALL DrawText	;[DrawText]
	db $16
data_6590:
	db $01
	db $08
data_6592:
	db $30
	db $2E
	db $58
	db $59
	db $5A
	db $20
	db $30
	db $31
	db $32
	db $33
	db $34
	db $35
	db $20
	db $FF
	LD E,(HL)
	LD D,$00
	INC HL
	EX DE,HL
	CALL function_65BB	;[function_65BB]
	CALL DrawText	;[DrawText]
	db $2F
	db $FF
	EX DE,HL
	POP BC
	INC C
	INC C
	DJNZ label_656E	;[label_656E]
	LD B,$02
	CALL function_6600	;[function_6600]
	JP label_5E81	;[label_5E81]
function_65BB:
	PUSH HL
	PUSH DE
	XOR A
	LD DE,$2710	;[S_PUSH_PO + 3]
	CALL function_65DD	;[function_65DD]
	LD DE,$03E8	;[BE_H_L_LP + 18]
	CALL function_65DD	;[function_65DD]
	LD DE,$0064	;[ERROR_3 + 15]
	CALL function_65DD	;[function_65DD]
	LD DE,$000A	;[ERROR_1 + 2]
	CALL function_65DD	;[function_65DD]
	LD A,L
	ADD A,$30
	POP DE
	POP HL
	JR label_65ED	;[label_65ED]
function_65DD:
	OR A
	JR NZ,label_65E4	;[label_65E4]
	SBC HL,DE
	ADD HL,DE
	RET C
label_65E4:
	LD A,$2F
label_65E6:
	INC A
	OR A
	SBC HL,DE
	JR NC,label_65E6	;[label_65E6]
	ADD HL,DE
label_65ED:
	LD (data_65F3),A	;[data_65F3]
	CALL DrawText	;[DrawText]
data_65F3:
	db $30
	db $FF
	RET
	db $DC
	db $69
	db $08
	db $6A
	db $34
	db $6A
	db $61
	db $6A
	db $FE
	db $74
function_6600:
	LD HL,$65F4	;[data_65F3 + 1]
label_6603:
	INC HL
	INC HL
	DJNZ label_6603	;[label_6603]
	LD E,(HL)
	INC HL
	LD D,(HL)
label_660A:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $00
	JR NZ,label_660A	;[label_660A]
	EX DE,HL
	JP label_D9DE	;[label_D9DE]
function_6615:
	LD (PlatformLUTColour),A	;[PlatformLUTColour]
	CALL function_5FD0	;[function_5FD0]
	XOR A
	LD (data_EA61),A	;[data_EA61]
	LD E,$07
	LD D,$04
	LD C,$02
HorizontalBorderPanelLoop:
	LD B,$00	;y pos
	LD L,$8A	;border panel - horizontal
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD B,$16	;ypos
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD A,C
	ADD A,D
	LD C,A
	DEC E
	JR NZ,HorizontalBorderPanelLoop	;[HorizontalBorderPanelLoop]
	LD E,$05
	LD B,$02
VerticalBorderPanelLoop:
	LD C,$00	;xpos 0
	LD L,$8B	;Border panel vertical
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD C,$1E	;xpos 30
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD A,B
	ADD A,D
	LD B,A
	DEC E
	JR NZ,VerticalBorderPanelLoop	;[VerticalBorderPanelLoop]
DrawTitleCornerPlatforms:
	LD HL,data_6661	;[data_6661]
	LD D,$04	;loop count
label_6652:
	LD C,(HL)	;load X
	INC HL
	LD B,(HL)	;load y
	INC HL
	LD E,(HL)	;platform no
	INC HL
	EX DE,HL
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	EX DE,HL
	DEC D
	JR NZ,label_6652	;[label_6652]
	RET
data_6661:
	db $00	;x pos
	db $00	;y pos
	db $8C	;platform no
	db $1E	;x
	db $00	;y
	db $8D	;platform
	db $00	;x
	db $16	;y
	db $8E	;platform
	db $1E	;x
	db $16	;y
	db $8F	;platform
label_666D:
	LD A,$04
	CALL function_6615	;[function_6615]
	CALL DrawText	;[DrawText]
	ascii 'FLIGHT COMPUTER REPORTTOUCHDOWN IMMINENT PREPARE	FOR MISSION STARQUAKE...CRASH... BANG... SMASH...<cr>
TOUCTHDOWNCOMTHUTER MALTHUNCTIONMALFUNNYTHINKIN ...'
	db $FF
	LD B,$04
	CALL function_6600	;[function_6600]
	CALL function_5FD0	;[function_5FD0]
	CALL function_A410	;[function_A410]
	LD A,$02
	CALL function_6615	;[function_6615]
	CALL DrawText	;[DrawText]
	db $16
	db $03
	db $0B
	db $10
	db $07
	db $47
	db $41
	db $4D
	db $45
	db $20
	db $20
	db $4F
	db $56
	db $45
	db $52
	db $16
	db $06
	db $0A
	db $10
	db $05
	db $53
	db $43
	db $4F
	db $52
	db $45
	db $16
	db $09
	db $07
	db $10
	db $03
	db $41
	db $44
	db $56
	db $45
	db $4E
	db $54
	db $55
	db $52
	db $45
	db $20
	db $53
	db $43
	db $4F
	db $52
	db $45
	db $16
	db $0C
	db $08
	db $10
	db $04
	db $54
	db $49
	db $4D
	db $45
	db $20
	db $54
	db $41
	db $4B
	db $45
	db $4E
	db $16
	db $0F
	db $05
	db $10
	db $07
	db $43
	db $4F
	db $52
	db $45
	db $20
	db $45
	db $4C
	db $45
	db $4D
	db $45
	db $4E
	db $54
	db $53
	db $16
	db $11
	db $07
	db $52
	db $45
	db $50
	db $4C
	db $41
	db $43
	db $45
	db $44
	db $16
	db $09
	db $17
	db $10
	db $03
	db $FF
	CALL function_679C	;[function_679C]
	JR label_67C7	;[label_67C7]
function_679C:
	LD DE,START
	LD (CurrentScreenNo),DE	;[CurrentScreenNo]
	LD BC,$0200	;[label_0140 + 192]
label_67A6:
	LD HL,$A390	;[label_A345 + 75]
	CALL function_A804	;[function_A804]
	DEC A
	JR Z,label_67B0	;[label_67B0]
	INC DE
label_67B0:
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	INC HL
	LD (CurrentScreenNo),HL	;[CurrentScreenNo]
	DEC BC
	LD A,B
	OR C
	JR NZ,label_67A6	;[label_67A6]
	LD HL,$0032	;[BC_SPACES + 2]
	CALL HL_HLxDE	;[HL_HLxDE]
	LD A,H
	LD L,H
	LD H,$00
	RET
label_67C7:
	LD (data_654A),A	;[data_654A]
	CALL function_65BB	;[function_65BB]
	CALL DrawText	;[DrawText]
	db $2F
	db $10
	db $05
	db $16
	db $06
	db $10
	db $FF
	LD HL,$D413	;[label_D406 + 13]
	LD DE,$67EA	;[label_67DF + 11]
	LD B,$06
label_67DF:
	LD A,(HL)
	INC HL
	ADD A,$30
	LD (DE),A
	INC DE
	DJNZ label_67DF	;[label_67DF]
	CALL DrawText	;[DrawText]
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $10
	db $04
	db $16
	db $0C
	db $14
	db $FF
	LD HL,(FRAMES)	;[FRAMES]
	LD A,(data_5C7A)	;[data_5C7A]
	INC A
	LD C,A
	LD DE,START
label_6801:
	PUSH BC
	LD BC,$0032	;[BC_SPACES + 2]
	OR A
	SBC HL,BC
	POP BC
	JR NC,label_680E	;[label_680E]
	DEC C
	JR Z,label_6811	;[label_6811]
label_680E:
	INC DE
	JR label_6801	;[label_6801]
label_6811:
	EX DE,HL
	LD BC,$003C	;[MASK_INT + 4]
	LD DE,START
label_6818:
	OR A
	SBC HL,BC
	JR C,label_6820	;[label_6820]
	INC DE
	JR label_6818	;[label_6818]
label_6820:
	ADD HL,BC
	EX DE,HL
	CALL function_65BB	;[function_65BB]
	CALL DrawText	;[DrawText]
	db $2E
	db $FF
	EX DE,HL
	LD A,L
	CP $0A
	JR NC,label_6835	;[label_6835]
	CALL DrawText	;[DrawText]
	db $30
	db $FF
label_6835:
	CALL function_65BB	;[function_65BB]
	CALL DrawText	;[DrawText]
	db $10
	db $07
	db $16
	db $13
	db $0A
	db $FF
	LD A,(data_D2E7)	;[data_D2E7]
	LD L,A
	LD A,$09
	SUB L
	LD L,A
	LD H,$00
	CP $0A
	JR NC,label_6854	;[label_6854]
	CALL DrawText	;[DrawText]
	db $30
	db $FF
label_6854:
	CALL function_65BB	;[function_65BB]
	LD BC,$0F15	;[COPY_L_3 + 1]
	CALL function_C352	;[function_C352]
	LD B,$01
	CALL function_6600	;[function_6600]
	LD DE,$67EA	;[label_67DF + 11]
	LD HL,$6543	;[label_64EE + 85]
	CALL function_64EC	;[function_64EC]
	JP C,label_654B	;[label_654B]
	LD A,$07
	CALL function_6615	;[function_6615]
	db $CD
	db $C1
	db $D3
	db $16
	db $05
	db $06
	db $10
	db $06
	db $45
	db $4E
	db $43
	db $4F
	db $44
	db $45
	db $20
	db $59
	db $4F
	db $55
	db $52
	db $20
	db $49
	db $4E
	db $49
	db $54
	db $49
	db $41
	db $4C
	db $53
	db $16
	db $07
	db $09
	db $49
	db $4E
	db $54
	db $4F
	db $20
	db $54
	db $48
	db $45
	db $20
	db $43
	db $4F
	db $52
	db $45
	db $16
	db $09
	db $06
	db $46
	db $4F
	db $52
	db $20
	db $59
	db $4F
	db $55
	db $20
	db $48
	db $41
	db $56
	db $45
	db $20
	db $41
	db $43
	db $48
	db $49
	db $45
	db $56
	db $45
	db $44
	db $16
	db $0B
	db $09
	db $41
	db $20
	db $48
	db $45
	db $52
	db $4F
	db $45
	db $53
	db $20
	db $53
	db $43
	db $4F
	db $52
	db $45
	db $10
	db $05
	db $16
	db $10
	db $0D
	db $2D
	db $20
	db $2D
	db $20
	db $2D
	db $16
	db $10
	db $0D
	db $10
	db $07
	db $FF
	db $06
	db $03
	db $21
	db $40
	db $65
	db $CD
	db $C8
	db $D5
	db $FE
	db $00
	db $20
	db $F9
	db $CD
	db $C8
	db $D5
	db $FE
	db $20
	db $38
	db $F9
	db $77
	db $23
	db $32
	db $F3
	db $68
	db $CD
	db $C1
	db $D3
	db $41
	db $20
	db $FF
	db $3E
	db $07
	db $CD
	db $C0
	db $D7
	db $10
	db $E0
	db $11
	db $EA
	db $67
	db $06
	db $06
	db $1A
	db $77
	db $23
	db $13
	db $10
	db $FA
	db $23
	db $7E
	db $2B
	db $77
	db $21
	db $36
	db $65
	db $11
	db $40
	db $65
	db $06
	db $07
	db $C5
	db $E5
	db $D5
	db $06
	db $03
	db $23
	db $13
	db $10
	db $FC
	db $CD
	db $EC
	db $64
	db $38
	db $0F
	db $D1
	db $E1
	db $E5
	db $D5
	db $06
	db $0A
	db $4E
	db $1A
	db $77
	db $79
	db $12
	db $13
	db $23
	db $10
	db $F7
	db $D1
	db $E1
	db $06
	db $0A
	db $2B
	db $1B
	db $10
	db $FC
	db $C1
	db $10
	db $D8
	db $C3
	db $4B
	db $65
	db $3E
	db $02
	db $CD
	db $15
	db $66
	db $01
	db $0D
	db $0E
	db $CD
	db $52
	db $C3
	db $CD
	db $C1
	db $D3
	db $16
	db $03
	db $07
	db $10
	db $05
	db $54
	db $48
	db $45
	db $20
	db $43
	db $4F
	db $52
	db $45
	db $53
	db $20
	db $43
	db $4F
	db $4D
	db $50
	db $4C
	db $45
	db $54
	db $45
	db $16
	db $05
	db $06
	db $42
	db $55
	db $54
	db $20
	db $48
	db $4F
	db $57
	db $20
	db $41
	db $52
	db $45
	db $20
	db $59
	db $4F
	db $55
	db $20
	db $47
	db $4F
	db $4E
	db $4E
	db $41
	db $16
	db $07
	db $06
	db $47
	db $45
	db $54
	db $20
	db $48
	db $4F
	db $4D
	db $45
	db $20
	db $57
	db $48
	db $45
	db $4E
	db $20
	db $4F
	db $4E
	db $4C
	db $59
	db $20
	db $41
	db $16
	db $09
	db $03
	db $54
	db $48
	db $54
	db $55
	db $50
	db $49
	db $44
	db $20
	db $4C
	db $4F
	db $4F
	db $4E
	db $59
	db $20
	db $57
	db $4F
	db $55
	db $4C
	db $44
	db $20
	db $57
	db $41
	db $4E
	db $44
	db $45
	db $52
	db $16
	db $0B
	db $03
	db $54
	db $48
	db $49
	db $53
	db $20
	db $46
	db $41
	db $52
	db $20
	db $4F
	db $55
	db $54
	db $20
	db $49
	db $4E
	db $20
	db $54
	db $48
	db $45
	db $20
	db $47
	db $41
	db $4C
	db $41
	db $58
	db $59
	db $FF
	db $06
	db $05
	db $CD
	db $00
	db $66
	db $C3
	db $30
	db $67
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $82
	db $0B
	db $0A
	db $68
	db $26
	db $28
	db $2A
	db $2B
	db $2F
	db $ED
	db $EA
	db $0B
	db $0A
	db $68
	db $26
	db $28
	db $2A
	db $2B
	db $2F
	db $ED
	db $ED
	db $0F
	db $0D
	db $AF
	db $28
	db $2A
	db $2B
	db $2F
	db $0D
	db $0B
	db $AD
	db $26
	db $28
	db $2A
	db $2D
	db $0B
	db $0A
	db $AB
	db $23
	db $28
	db $2A
	db $2B
	db $E8
	db $00
	db $46
	db $A3
	db $2A
	db $23
	db $28
	db $23
	db $26
	db $A3
	db $2A
	db $23
	db $28
	db $23
	db $26
	db $A1
	db $28
	db $21
	db $26
	db $21
	db $25
	db $A1
	db $28
	db $21
	db $26
	db $21
	db $25
	db $A3
	db $6A
	db $68
	db $26
	db $A3
	db $6A
	db $68
	db $26
	db $A1
	db $68
	db $66
	db $25
	db $A1
	db $68
	db $66
	db $25
	db $E3
	db $E3
	db $00
data_6A34:
	db $64
	db $2A
	db $26
	db $23
	db $6A
	db $26
	db $63
	db $2B
	db $28
	db $23
	db $6B
	db $28
	db $63
	db $28
	db $25
	db $21
	db $68
	db $25
	db $61
	db $2A
	db $26
	db $21
	db $6A
	db $26
	db $61
	db $2A
	db $26
	db $23
	db $6A
	db $26
	db $63
	db $2B
	db $28
	db $23
	db $6B
	db $28
	db $63
	db $28
	db $25
	db $21
	db $68
	db $25
	db $61
	db $E6
	db $00
	db $46
	db $62
	db $29
	db $22
	db $27
	db $A9
	db $62
	db $29
	db $6C
	db $AE
	db $62
	db $29
	db $22
	db $27
	db $A9
	db $62
	db $29
	db $67
	db $A5
	db $62
	db $29
	db $22
	db $27
	db $A9
	db $62
	db $29
	db $6C
	db $AE
	db $62
	db $29
	db $22
	db $27
	db $A9
	db $87
	db $85
	db $A2
	db $00
	db $E7
	db $00
	db $AF
	db $54
	db $32
	db $6D
	db $52
	db $54
	db $32
	db $AD
	db $52
	db $31
	db $72
	db $54
	db $52
	db $31
	db $AF
	db $51
	db $2F
	db $71
	db $52
	db $51
	db $2F
	db $AD
	db $EF
	db $EF
	db $00
	db $00
	db $00
	db $00
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $F3
	db $29
	db $21
	db $F2
	db $FF
	db $FF
	db $00
	db $FF
	db $FE
	db $E5
	db $C4
	db $7E
	db $FF
	db $FF
	db $00
	db $FF
	db $7F
	db $27
	db $21
	db $78
	db $FF
	db $FF
	db $00
	db $FF
	db $CF
	db $A4
	db $84
	db $4F
	db $7E
	db $23
	db $27
	db $7F
	db $FF
	db $00
	db $00
	db $FF
	db $4F
	db $A4
	db $84
	db $CF
	db $FF
	db $00
	db $00
	db $FF
	db $F2
	db $29
	db $21
	db $F3
	db $FF
	db $00
	db $00
	db $FF
	db $1E
	db $85
	db $E4
	db $FE
	db $FF
	db $00
	db $00
	db $FF
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $C0
	db $C0
	db $C0
	db $C0
	db $00
	db $00
	db $D9
	db $D9
	db $DF
	db $D9
	db $D4
	db $D0
	db $D9
	db $DE
	db $09
	db $99
	db $F9
	db $99
	db $99
	db $39
	db $39
	db $79
	db $DE
	db $DC
	db $DC
	db $D9
	db $D9
	db $DF
	db $D9
	db $D4
	db $79
	db $99
	db $49
	db $09
	db $99
	db $F9
	db $99
	db $99
	db $D0
	db $D9
	db $DF
	db $D9
	db $D9
	db $DD
	db $DF
	db $DE
	db $99
	db $99
	db $F9
	db $99
	db $49
	db $09
	db $99
	db $79
	db $DE
	db $D9
	db $D4
	db $D0
	db $D9
	db $DF
	db $D9
	db $D9
	db $79
	db $F9
	db $B9
	db $99
	db $99
	db $F9
	db $99
	db $49
	db $76
	db $76
	db $76
	db $76
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $01
	db $00
	db $03
	db $0C
	db $13
	db $13
	db $2D
	db $AE
	db $FF
	db $3F
	db $D8
	db $6F
	db $EF
	db $F7
	db $F7
	db $F7
	db $AF
	db $BF
	db $DF
	db $E7
	db $F8
	db $DF
	db $DF
	db $DF
	db $7B
	db $BB
	db $DB
	db $EB
	db $F7
	db $08
	db $FA
	db $F9
	db $76
	db $76
	db $76
	db $76
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FC
	db $1B
	db $F6
	db $F7
	db $EF
	db $EF
	db $EF
	db $80
	db $00
	db $C0
	db $30
	db $C8
	db $C8
	db $B4
	db $75
	db $DE
	db $DD
	db $DB
	db $D7
	db $EF
	db $10
	db $5F
	db $DF
	db $F5
	db $FD
	db $FB
	db $E7
	db $1D
	db $F9
	db $F9
	db $F9
	db $76
	db $76
	db $76
	db $76
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $DF
	db $DF
	db $DF
	db $F8
	db $E7
	db $DF
	db $BF
	db $AF
	db $F9
	db $FB
	db $08
	db $F7
	db $EB
	db $DB
	db $BB
	db $7B
	db $AE
	db $2D
	db $13
	db $13
	db $0C
	db $03
	db $00
	db $01
	db $F7
	db $F7
	db $F7
	db $EF
	db $6F
	db $D8
	db $30
	db $FF
	db $76
	db $76
	db $76
	db $76
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $9F
	db $5F
	db $10
	db $EF
	db $D7
	db $DB
	db $DD
	db $DE
	db $F9
	db $F9
	db $F9
	db $1D
	db $E7
	db $FB
	db $FD
	db $F5
	db $EF
	db $EF
	db $EF
	db $F7
	db $F6
	db $1B
	db $0C
	db $FF
	db $75
	db $B4
	db $C8
	db $C8
	db $30
	db $C0
	db $00
	db $80
	db $76
	db $76
	db $76
	db $76
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FC
	db $18
	db $F0
	db $F1
	db $E1
	db $E5
	db $E6
	db $80
	db $00
	db $00
	db $80
	db $30
	db $10
	db $00
	db $89
	db $CF
	db $CF
	db $CF
	db $CF
	db $E7
	db $10
	db $5F
	db $DF
	db $71
	db $89
	db $FB
	db $E7
	db $1D
	db $F9
	db $F9
	db $F9
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $3C
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $00
	db $1E
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $00
	db $00
	db $00
	db $81
	db $81
	db $81
	db $81
	db $3C
	db $00
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $FE
	db $FC
	db $00
	db $1E
	db $3F
	db $7E
	db $7D
	db $7D
	db $41
	db $3D
	db $1E
	db $00
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $C2
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $20
	db $1E
	db $78
	db $00
	db $3C
	db $7E
	db $7F
	db $FF
	db $FF
	db $FF
	db $7E
	db $FF
	db $FE
	db $FE
	db $FE
	db $7E
	db $3C
	db $00
	db $BC
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $1E
	db $00
	db $7E
	db $BC
	db $82
	db $BE
	db $BE
	db $7E
	db $FC
	db $78
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $3C
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $40
	db $1E
	db $00
	db $00
	db $01
	db $01
	db $01
	db $01
	db $1E
	db $3F
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $FE
	db $FD
	db $78
	db $00
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $C2
	db $BC
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $02
	db $3C
	db $7F
	db $BF
	db $BF
	db $BF
	db $BF
	db $1E
	db $00
	db $00
	db $80
	db $80
	db $80
	db $BC
	db $7E
	db $FF
	db $FF
	db $FF
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $00
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $00
	db $1E
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $1E
	db $FF
	db $7E
	db $BD
	db $81
	db $81
	db $81
	db $00
	db $00
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $FE
	db $FC
	db $78
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $50
	db $00
	db $00
	db $00
	db $3C
	db $7E
	db $FE
	db $FD
	db $FD
	db $FD
	db $7D
	db $00
	db $00
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $E0
	db $DE
	db $3C
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $00
	db $78
	db $7C
	db $7E
	db $7E
	db $7E
	db $FE
	db $FC
	db $1E
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $1E
	db $3F
	db $7F
	db $BF
	db $BF
	db $BF
	db $BF
	db $02
	db $3C
	db $02
	db $BF
	db $BF
	db $BF
	db $BF
	db $7F
	db $3F
	db $1E
	db $78
	db $00
	db $BC
	db $BE
	db $BF
	db $BF
	db $7F
	db $FF
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $00
	db $7E
	db $3C
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $60
	db $00
	db $00
	db $00
	db $3C
	db $7E
	db $FD
	db $FB
	db $FB
	db $FB
	db $7B
	db $21
	db $00
	db $F1
	db $FB
	db $FD
	db $FD
	db $FD
	db $FD
	db $F8
	db $F0
	db $F8
	db $FC
	db $FD
	db $FD
	db $FD
	db $E1
	db $DE
	db $00
	db $78
	db $FC
	db $FE
	db $FE
	db $FE
	db $C2
	db $BC
	db $1E
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $1E
	db $F0
	db $00
	db $80
	db $80
	db $9E
	db $BF
	db $7F
	db $7F
	db $3F
	db $7F
	db $7F
	db $7F
	db $7F
	db $3F
	db $82
	db $BC
	db $7E
	db $BF
	db $BF
	db $BF
	db $BF
	db $7E
	db $3C
	db $00
	db $7F
	db $7E
	db $3E
	db $1E
	db $00
	db $00
	db $00
	db $00
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $7E
	db $3C
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $E0
	db $E1
	db $E1
	db $E0
	db $EF
	db $EF
	db $EF
	db $00
	db $00
	db $FF
	db $FF
	db $07
	db $F7
	db $F7
	db $F7
	db $00
	db $00
	db $80
	db $80
	db $80
	db $80
	db $BF
	db $BF
	db $7F
	db $7F
	db $7F
	db $00
	db $00
	db $00
	db $0F
	db $0F
	db $EF
	db $EF
	db $EF
	db $0F
	db $3F
	db $3F
	db $BF
	db $80
	db $F7
	db $F7
	db $F7
	db $F7
	db $F0
	db $F0
	db $F7
	db $0F
	db $BF
	db $BF
	db $BF
	db $BF
	db $3F
	db $03
	db $FB
	db $FB
	db $0F
	db $0F
	db $0F
	db $0F
	db $0F
	db $0F
	db $0F
	db $0F
	db $FC
	db $FC
	db $FC
	db $FC
	db $FC
	db $FC
	db $FC
	db $FC
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $03
	db $03
	db $03
	db $03
	db $7B
	db $7B
	db $7B
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $F0
	db $F7
	db $00
	db $03
	db $03
	db $03
	db $03
	db $03
	db $03
	db $FB
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $7B
	db $7B
	db $7B
	db $78
	db $7F
	db $7F
	db $7F
	db $00
	db $F7
	db $F7
	db $F7
	db $07
	db $E7
	db $E7
	db $E7
	db $07
	db $FB
	db $FB
	db $FB
	db $F8
	db $FE
	db $FE
	db $FE
	db $FE
	db $FF
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $F8
	db $F8
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $F7
	db $F0
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $FE
	db $00
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $1F
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $F8
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $0F
	db $0F
	db $0F
	db $EF
	db $EF
	db $EF
	db $EF
	db $00
	db $FC
	db $FB
	db $FB
	db $FB
	db $80
	db $BF
	db $BF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $07
	db $F7
	db $F7
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $00
	db $EF
	db $EF
	db $EF
	db $00
	db $7F
	db $7F
	db $7F
	db $7F
	db $BF
	db $BF
	db $BF
	db $0F
	db $EF
	db $EF
	db $EF
	db $EF
	db $F7
	db $F7
	db $F7
	db $F7
	db $F0
	db $F0
	db $F0
	db $F0
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $00
	db $E0
	db $E0
	db $E0
	db $E0
	db $E0
	db $E0
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $60
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $E0
	db $E0
	db $EF
	db $EF
	db $EF
	db $EF
	db $EF
	db $00
	db $03
	db $03
	db $FB
	db $00
	db $7F
	db $7F
	db $7F
	db $00
	db $FF
	db $FF
	db $FF
	db $0F
	db $EF
	db $EF
	db $EF
	db $7F
	db $7F
	db $7F
	db $00
	db $0F
	db $0F
	db $0F
	db $0F
	db $EF
	db $EF
	db $EF
	db $0F
	db $EC
	db $E3
	db $FB
	db $FB
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $FF
	db $FF
	db $FF
	db $EF
	db $EF
	db $EF
	db $E0
	db $60
	db $60
	db $60
	db $00
	db $03
	db $03
	db $03
	db $03
	db $03
	db $03
	db $03
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $C0
	db $F0
	db $EF
	db $DF
	db $B8
	db $00
	db $00
	db $00
	db $07
	db $1F
	db $EF
	db $F7
	db $3B
	db $00
	db $00
	db $00
	db $C0
	db $F0
	db $EF
	db $DC
	db $B8
	db $00
	db $00
	db $00
	db $07
	db $1F
	db $EF
	db $37
	db $1B
	db $70
	db $E8
	db $EE
	db $E6
	db $F0
	db $F8
	db $F8
	db $7A
	db $1D
	db $2E
	db $EE
	db $CE
	db $1E
	db $3E
	db $3E
	db $BC
	db $7B
	db $F3
	db $F6
	db $E4
	db $E1
	db $E1
	db $F0
	db $78
	db $6D
	db $6E
	db $26
	db $06
	db $86
	db $8E
	db $CE
	db $4C
	db $BD
	db $DD
	db $EF
	db $F0
	db $FB
	db $F7
	db $E0
	db $C0
	db $7B
	db $77
	db $EF
	db $1F
	db $BF
	db $DF
	db $0F
	db $07
	db $BC
	db $DC
	db $EF
	db $F0
	db $FB
	db $F7
	db $E0
	db $C0
	db $1B
	db $37
	db $EF
	db $1F
	db $BF
	db $DF
	db $0F
	db $07
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $C0
	db $F0
	db $EF
	db $DB
	db $BB
	db $00
	db $00
	db $00
	db $07
	db $1F
	db $EF
	db $B7
	db $BB
	db $00
	db $00
	db $00
	db $C0
	db $F0
	db $EF
	db $DF
	db $B8
	db $00
	db $00
	db $00
	db $07
	db $1F
	db $EF
	db $F7
	db $3B
	db $73
	db $F3
	db $F0
	db $E6
	db $E2
	db $F0
	db $FB
	db $79
	db $9D
	db $9E
	db $1E
	db $CE
	db $8E
	db $1E
	db $BE
	db $3C
	db $60
	db $E0
	db $C0
	db $CA
	db $E6
	db $E0
	db $F5
	db $75
	db $0D
	db $0E
	db $06
	db $A6
	db $CE
	db $0E
	db $5E
	db $5C
	db $BD
	db $DC
	db $EF
	db $F0
	db $FB
	db $F7
	db $E0
	db $C0
	db $7B
	db $77
	db $EF
	db $1F
	db $BF
	db $DF
	db $0F
	db $07
	db $BB
	db $DC
	db $EF
	db $F0
	db $FB
	db $F7
	db $E0
	db $C0
	db $BB
	db $77
	db $EF
	db $1F
	db $BF
	db $DF
	db $0F
	db $07
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $23
	db $23
	db $23
	db $23
	db $00
	db $06
	db $06
	db $03
	db $F8
	db $FF
	db $FF
	db $FF
	db $00
	db $3F
	db $3F
	db $18
	db $3F
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $1F
	db $F4
	db $F4
	db $F4
	db $F4
	db $00
	db $A0
	db $A0
	db $00
	db $00
	db $07
	db $09
	db $09
	db $0F
	db $07
	db $04
	db $02
	db $E7
	db $EC
	db $EC
	db $ED
	db $F7
	db $FB
	db $F7
	db $7B
	db $E0
	db $9B
	db $9B
	db $F7
	db $3B
	db $3D
	db $BB
	db $F7
	db $E0
	db $F8
	db $E4
	db $C4
	db $FC
	db $F8
	db $C8
	db $90
	db $01
	db $00
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $D3
	db $2C
	db $1F
	db $1F
	db $00
	db $8F
	db $8F
	db $8F
	db $77
	db $89
	db $FE
	db $FE
	db $00
	db $FD
	db $FD
	db $FD
	db $E0
	db $00
	db $80
	db $80
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $03
	db $FD
	db $FE
	db $FF
	db $00
	db $7F
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $7F
	db $00
	db $FE
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FE
	db $00
	db $00
	db $00
	db $80
	db $C0
	db $BF
	db $7F
	db $FF
	db $FF
	db $FF
	db $FE
	db $FD
	db $03
	db $FD
	db $06
	db $03
	db $80
	db $7F
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $7F
	db $01
	db $FE
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FE
	db $FF
	db $FF
	db $7F
	db $BF
	db $C0
	db $BF
	db $60
	db $C0
	db $01
	db $03
	db $06
	db $FD
	db $03
	db $01
	db $00
	db $00
	db $80
	db $7F
	db $C0
	db $80
	db $00
	db $80
	db $C0
	db $00
	db $01
	db $FE
	db $03
	db $01
	db $00
	db $01
	db $03
	db $00
	db $80
	db $C0
	db $60
	db $BF
	db $C0
	db $80
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
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $0F
	db $1F
	db $3F
	db $7F
	db $FC
	db $F8
	db $E8
	db $00
	db $EF
	db $F7
	db $FB
	db $FD
	db $6E
	db $36
	db $36
	db $00
	db $EF
	db $DF
	db $BF
	db $7C
	db $F7
	db $E3
	db $F7
	db $00
	db $E0
	db $F0
	db $F8
	db $3C
	db $1E
	db $8E
	db $8E
	db $D8
	db $DF
	db $E0
	db $7B
	db $BF
	db $5F
	db $6F
	db $30
	db $6E
	db $9E
	db $7E
	db $FC
	db $FB
	db $F7
	db $EF
	db $1F
	db $FF
	db $FF
	db $FF
	db $7E
	db $BF
	db $DF
	db $EF
	db $F0
	db $8E
	db $8E
	db $1E
	db $3C
	db $FA
	db $F4
	db $EC
	db $18
	db $3B
	db $17
	db $0F
	db $07
	db $03
	db $01
	db $00
	db $00
	db $DF
	db $EF
	db $F7
	db $F0
	db $EF
	db $DF
	db $0F
	db $00
	db $F7
	db $EF
	db $DF
	db $1F
	db $EF
	db $F7
	db $E0
	db $00
	db $B8
	db $D0
	db $E0
	db $C0
	db $80
	db $00
	db $00
	db $00
	db $40
	db $40
	db $40
	db $40
	db $00
	db $60
	db $60
	db $00
	db $00
	db $00
	db $02
	db $07
	db $47
	db $4F
	db $BD
	db $8A
	db $AB
	db $F7
	db $10
	db $38
	db $78
	db $FD
	db $ED
	db $D5
	db $5B
	db $3B
	db $5F
	db $4F
	db $27
	db $13
	db $00
	db $09
	db $13
	db $27
	db $BE
	db $FA
	db $F4
	db $E8
	db $00
	db $D0
	db $E8
	db $F4
	db $36
	db $36
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $60
	db $60
	db $00
	db $00
	db $00
	db $03
	db $07
	db $00
	db $00
	db $03
	db $07
	db $01
	db $00
	db $83
	db $68
	db $1F
	db $1B
	db $61
	db $81
	db $85
	db $0E
	db $C0
	db $83
	db $7C
	db $E6
	db $83
	db $80
	db $A0
	db $70
	db $C0
	db $60
	db $00
	db $00
	db $B0
	db $F0
	db $00
	db $00
	db $0B
	db $1F
	db $37
	db $27
	db $3D
	db $19
	db $0F
	db $02
	db $F0
	db $F8
	db $FC
	db $FC
	db $7C
	db $D8
	db $F0
	db $40
	db $01
	db $01
	db $00
	db $01
	db $03
	db $07
	db $06
	db $06
	db $80
	db $C0
	db $C0
	db $C0
	db $80
	db $00
	db $00
	db $00
	db $05
	db $05
	db $05
	db $05
	db $07
	db $07
	db $07
	db $07
	db $F0
	db $60
	db $60
	db $00
	db $00
	db $00
	db $1F
	db $38
	db $7A
	db $F0
	db $72
	db $32
	db $1F
	db $00
	db $FF
	db $81
	db $95
	db $95
	db $95
	db $95
	db $FF
	db $00
	db $FF
	db $10
	db $B3
	db $B2
	db $13
	db $10
	db $FF
	db $00
	db $FF
	db $63
	db $EB
	db $43
	db $4B
	db $4B
	db $FF
	db $00
	db $02
	db $00
	db $02
	db $02
	db $02
	db $02
	db $02
	db $02
	db $C0
	db $00
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $C0
	db $02
	db $02
	db $00
	db $02
	db $00
	db $0B
	db $0B
	db $0B
	db $C0
	db $C0
	db $00
	db $C0
	db $00
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $60
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $7F
	db $FF
	db $E0
	db $0E
	db $1F
	db $1F
	db $6F
	db $EE
	db $EE
	db $DC
	db $1C
	db $D8
	db $5C
	db $3E
	db $0F
	db $07
	db $07
	db $0F
	db $00
	db $1C
	db $38
	db $38
	db $70
	db $B0
	db $C0
	db $F0
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
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $78
	db $7E
	db $7E
	db $FE
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $05
	db $1F
	db $00
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $C0
	db $1B
	db $3C
	db $18
	db $30
	db $1C
	db $1C
	db $08
	db $1D
	db $C6
	db $B0
	db $71
	db $66
	db $0D
	db $DD
	db $9B
	db $BA
	db $00
	db $FE
	db $FF
	db $FF
	db $FF
	db $FF
	db $3C
	db $18
	db $70
	db $18
	db $CC
	db $E6
	db $F2
	db $FB
	db $F9
	db $79
	db $00
	db $00
	db $00
	db $01
	db $03
	db $07
	db $07
	db $87
	db $00
	db $00
	db $00
	db $C0
	db $F0
	db $F8
	db $BF
	db $9E
	db $0D
	db $19
	db $0D
	db $21
	db $3D
	db $41
	db $7C
	db $4E
	db $3A
	db $3A
	db $3A
	db $06
	db $3F
	db $BF
	db $9F
	db $DE
	db $18
	db $DB
	db $99
	db $5A
	db $1C
	db $FF
	db $FF
	db $73
	db $7C
	db $7C
	db $7C
	db $7C
	db $FC
	db $D9
	db $B9
	db $BB
	db $87
	db $83
	db $81
	db $81
	db $80
	db $80
	db $00
	db $00
	db $80
	db $E0
	db $E0
	db $F0
	db $F8
	db $F8
	db $78
	db $78
	db $00
	db $70
	db $F8
	db $BC
	db $1C
	db $18
	db $38
	db $3E
	db $4F
	db $4F
	db $4F
	db $4F
	db $7F
	db $01
	db $7E
	db $FF
	db $5E
	db $6F
	db $37
	db $99
	db $8E
	db $83
	db $0C
	db $8F
	db $21
	db $0E
	db $9F
	db $FF
	db $7E
	db $81
	db $FF
	db $00
	db $B2
	db $76
	db $EC
	db $98
	db $70
	db $C0
	db $21
	db $E0
	db $00
	db $00
	db $00
	db $01
	db $07
	db $E7
	db $F0
	db $77
	db $78
	db $78
	db $F8
	db $F0
	db $F0
	db $E0
	db $C0
	db $00
	db $3D
	db $3B
	db $1B
	db $01
	db $0E
	db $0F
	db $1B
	db $1D
	db $FF
	db $FF
	db $E7
	db $D9
	db $5C
	db $5E
	db $5F
	db $5F
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $07
	db $BF
	db $7E
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $E3
	db $F7
	db $FF
	db $FF
	db $FF
	db $FE
	db $F1
	db $FF
	db $BB
	db $BB
	db $BB
	db $87
	db $3F
	db $1E
	db $E0
	db $F7
	db $80
	db $81
	db $A7
	db $B2
	db $7C
	db $3E
	db $DF
	db $EF
	db $8C
	db $9E
	db $18
	db $1C
	db $1C
	db $7C
	db $F8
	db $E0
	db $1C
	db $0F
	db $07
	db $00
	db $07
	db $0F
	db $7F
	db $FC
	db $CF
	db $8F
	db $03
	db $F0
	db $FD
	db $FF
	db $FF
	db $1F
	db $7E
	db $7E
	db $7D
	db $7C
	db $FB
	db $FB
	db $F7
	db $C1
	db $FF
	db $EF
	db $EF
	db $F7
	db $F3
	db $F3
	db $F1
	db $E0
	db $FF
	db $BF
	db $88
	db $FF
	db $FF
	db $FF
	db $FF
	db $7C
	db $FB
	db $FD
	db $7C
	db $BE
	db $DF
	db $8F
	db $0F
	db $07
	db $C3
	db $00
	db $0F
	db $3F
	db $FF
	db $FC
	db $F8
	db $E0
	db $C0
	db $00
	db $80
	db $80
	db $C0
	db $C0
	db $80
	db $00
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $F8
	db $FB
	db $FB
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $1C
	db $DE
	db $1E
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $30
	db $36
	db $30
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $10
	db $12
	db $10
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $30
	db $31
	db $10
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $3F
	db $FF
	db $1F
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $03
	db $1F
	db $01
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FC
	db $FE
	db $9E
	db $FB
	db $FB
	db $F8
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $1C
	db $DC
	db $1C
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $1E
	db $1E
	db $1E
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $12
	db $12
	db $10
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $1E
	db $10
	db $10
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $1F
	db $1F
	db $1F
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $E1
	db $01
	db $01
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $9E
	db $F2
	db $F0
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $76
	db $FC
	db $FC
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $1F
	db $1F
	db $19
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $80
	db $84
	db $87
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $81
	db $89
	db $80
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $80
	db $84
	db $84
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $84
	db $84
	db $84
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $9F
	db $9F
	db $9F
	db $19
	db $1F
	db $1F
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $87
	db $84
	db $80
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $84
	db $84
	db $84
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $84
	db $84
	db $80
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $84
	db $80
	db $80
	db $FF
	db $FF
	db $00
	db $00
	db $FF
	db $9F
	db $1F
	db $1F
	db $FF
	db $FF
	db $00
	db $00
	db $FF
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
	db $8C
	db $20
	db $27
	db $AC
	db $2F
	db $2E
	db $AA
	db $25
	db $27
	db $AA
	db $25
	db $27
	db $AC
	db $20
	db $27
	db $AC
	db $2E
	db $2F
	db $B3
	db $2E
	db $2C
	db $AA
	db $25
	db $27
	db $AC
	db $20
	db $27
	db $AC
	db $2F
	db $2E
	db $AA
	db $25
	db $27
	db $AA
	db $25
	db $27
	db $AC
	db $20
	db $27
	db $AC
	db $2E
	db $2F
	db $B3
	db $2E
	db $2C
	db $AA
	db $A5
	db $AC
	db $00
	db $00
Screens:
	db $02
	db $02
	db $05
	db $05
	db $64
	db $07
	db $07
	db $00
	db $14
	db $00
	db $00
	db $0D
	db $03
	db $02
	db $03
	db $05
	db $07
	db $00
	db $0C
	db $00
	db $09
	db $0C
	db $0D
	db $09
	db $04
	db $03
	db $02
	db $04
	db $00
	db $0C
	db $00
	db $00
	db $09
	db $09
	db $0F
	db $09
	db $02
	db $04
	db $02
	db $04
	db $00
	db $01
	db $0D
	db $0A
	db $0C
	db $08
	db $00
	db $0A
	db $02
	db $05
	db $02
	db $03
	db $0B
	db $09
	db $00
	db $00
	db $0B
	db $00
	db $00
	db $0A
	db $03
	db $04
	db $03
	db $04
	db $00
	db $00
	db $07
	db $00
	db $10
	db $09
	db $0F
	db $09
	db $05
	db $04
	db $05
	db $02
	db $00
	db $06
	db $03
	db $00
	db $09
	db $00
	db $0F
	db $09
	db $02
	db $04
	db $03
	db $04
	db $00
	db $06
	db $00
	db $00
	db $09
	db $00
	db $08
	db $10
	db $05
	db $02
	db $05
	db $05
	db $07
	db $00
	db $11
	db $00
	db $28
	db $21
	db $14
	db $0D
	db $04
	db $04
	db $05
	db $05
	db $00
	db $06
	db $0C
	db $00
	db $09
	db $0F
	db $0F
	db $09
	db $04
	db $03
	db $03
	db $02
	db $00
	db $0D
	db $0C
	db $00
	db $0C
	db $00
	db $00
	db $0D
	db $02
	db $03
	db $05
	db $02
	db $00
	db $00
	db $07
	db $04
	db $09
	db $00
	db $00
	db $0A
	db $02
	db $02
	db $03
	db $04
	db $05
	db $00
	db $0D
	db $00
	db $10
	db $09
	db $0F
	db $09
	db $05
	db $02
	db $03
	db $02
	db $00
	db $00
	db $01
	db $12
	db $0F
	db $00
	db $14
	db $14
	db $05
	db $05
	db $03
	db $03
	db $01
	db $01
	db $08
	db $00
	db $14
	db $14
	db $14
	db $14
	db $03
	db $03
	db $02
	db $02
	db $01
	db $22
	db $64
	db $04
	db $14
	db $13
	db $38
	db $02
	db $14
	db $00
	db $00
	db $29
	db $24
	db $00
	db $00
	db $00
	db $1C
	db $23
	db $09
	db $0F
	db $0A
	db $26
	db $29
	db $08
	db $00
	db $00
	db $26
	db $00
	db $0C
	db $0D
	db $0F
	db $10
	db $08
	db $08
	db $0A
	db $10
	db $01
	db $01
	db $65
	db $0A
	db $10
	db $10
	db $10
	db $10
	db $28
	db $00
	db $00
	db $27
	db $10
	db $09
	db $00
	db $0A
	db $0B
	db $00
	db $00
	db $27
	db $28
	db $00
	db $00
	db $29
	db $64
	db $00
	db $00
	db $00
	db $14
	db $16
	db $09
	db $09
	db $08
	db $08
	db $0A
	db $10
	db $00
	db $00
	db $09
	db $10
	db $0C
	db $26
	db $00
	db $27
	db $0B
	db $00
	db $08
	db $0A
	db $10
	db $08
	db $26
	db $27
	db $28
	db $0F
	db $09
	db $10
	db $0B
	db $00
	db $00
	db $0A
	db $10
	db $08
	db $0D
	db $27
	db $28
	db $00
	db $00
	db $27
	db $28
	db $1F
	db $14
	db $1F
	db $0B
	db $00
	db $1B
	db $14
	db $28
	db $00
	db $17
	db $13
	db $08
	db $08
	db $0A
	db $10
	db $24
	db $00
	db $00
	db $00
	db $1C
	db $22
	db $16
	db $17
	db $28
	db $00
	db $00
	db $27
	db $00
	db $0D
	db $00
	db $0A
	db $16
	db $00
	db $00
	db $27
	db $0B
	db $00
	db $00
	db $29
	db $28
	db $00
	db $0E
	db $00
	db $10
	db $09
	db $00
	db $0D
	db $08
	db $08
	db $0A
	db $10
	db $00
	db $2A
	db $0C
	db $27
	db $09
	db $0F
	db $09
	db $10
	db $0B
	db $00
	db $1A
	db $25
	db $28
	db $00
	db $00
	db $19
	db $10
	db $00
	db $21
	db $1D
	db $24
	db $1B
	db $15
	db $15
	db $18
	db $01
	db $2A
	db $16
	db $14
	db $1A
	db $00
	db $21
	db $15
	db $15
	db $1B
	db $04
	db $20
	db $2B
	db $64
	db $04
	db $16
	db $14
	db $38
	db $14
	db $24
	db $15
	db $1A
	db $25
	db $18
	db $00
	db $00
	db $19
	db $18
	db $21
	db $64
	db $19
	db $28
	db $29
	db $08
	db $08
	db $14
	db $12
	db $12
	db $00
	db $13
	db $14
	db $1C
	db $17
	db $08
	db $08
	db $0A
	db $0A
	db $00
	db $0D
	db $00
	db $0A
	db $16
	db $00
	db $00
	db $10
	db $28
	db $0D
	db $0D
	db $19
	db $0B
	db $0C
	db $0C
	db $19
	db $28
	db $0D
	db $00
	db $1B
	db $24
	db $1A
	db $1B
	db $0A
	db $18
	db $21
	db $22
	db $20
	db $18
	db $00
	db $13
	db $0F
	db $28
	db $00
	db $12
	db $29
	db $00
	db $08
	db $08
	db $00
	db $09
	db $0F
	db $09
	db $0F
	db $26
	db $08
	db $0B
	db $10
	db $00
	db $00
	db $29
	db $27
	db $09
	db $00
	db $0D
	db $10
	db $28
	db $00
	db $00
	db $29
	db $10
	db $08
	db $08
	db $00
	db $28
	db $00
	db $17
	db $16
	db $26
	db $00
	db $15
	db $14
	db $00
	db $0E
	db $2B
	db $13
	db $17
	db $17
	db $1D
	db $13
	db $14
	db $15
	db $1B
	db $14
	db $24
	db $00
	db $23
	db $2C
	db $1A
	db $00
	db $00
	db $1B
	db $14
	db $00
	db $00
	db $27
	db $2D
	db $20
	db $26
	db $00
	db $13
	db $23
	db $16
	db $17
	db $0B
	db $08
	db $00
	db $29
	db $00
	db $00
	db $00
	db $00
	db $16
	db $09
	db $09
	db $10
	db $08
	db $0A
	db $0A
	db $0B
	db $00
	db $0E
	db $0C
	db $00
	db $10
	db $10
	db $09
	db $0F
	db $08
	db $00
	db $19
	db $14
	db $00
	db $00
	db $1D
	db $13
	db $0C
	db $00
	db $1F
	db $13
	db $24
	db $1E
	db $00
	db $1F
	db $18
	db $00
	db $00
	db $25
	db $1C
	db $16
	db $17
	db $22
	db $15
	db $15
	db $1A
	db $25
	db $14
	db $2E
	db $2E
	db $1D
	db $13
	db $00
	db $00
	db $19
	db $18
	db $1F
	db $18
	db $1F
	db $1C
	db $00
	db $18
	db $00
	db $18
	db $00
	db $1C
	db $17
	db $15
	db $32
	db $1A
	db $25
	db $00
	db $33
	db $34
	db $2F
	db $20
	db $00
	db $00
	db $19
	db $14
	db $31
	db $30
	db $0A
	db $24
	db $00
	db $26
	db $27
	db $18
	db $00
	db $0D
	db $10
	db $28
	db $29
	db $00
	db $19
	db $0B
	db $08
	db $28
	db $14
	db $10
	db $23
	db $17
	db $2C
	db $18
	db $00
	db $32
	db $0A
	db $14
	db $31
	db $34
	db $00
	db $2D
	db $16
	db $17
	db $16
	db $0B
	db $0A
	db $0B
	db $08
	db $00
	db $00
	db $12
	db $00
	db $16
	db $17
	db $1C
	db $16
	db $08
	db $00
	db $29
	db $08
	db $00
	db $0E
	db $29
	db $00
	db $17
	db $16
	db $17
	db $16
	db $26
	db $00
	db $24
	db $25
	db $00
	db $25
	db $1C
	db $2F
	db $20
	db $19
	db $18
	db $19
	db $24
	db $15
	db $1A
	db $15
	db $1C
	db $30
	db $00
	db $00
	db $14
	db $14
	db $22
	db $14
	db $1E
	db $00
	db $20
	db $1F
	db $00
	db $00
	db $34
	db $00
	db $14
	db $17
	db $16
	db $17
	db $15
	db $15
	db $1A
	db $25
	db $00
	db $20
	db $21
	db $19
	db $20
	db $21
	db $00
	db $1D
	db $24
	db $08
	db $08
	db $0A
	db $18
	db $00
	db $0C
	db $00
	db $1C
	db $00
	db $00
	db $0D
	db $0B
	db $08
	db $08
	db $0A
	db $00
	db $2A
	db $28
	db $0A
	db $09
	db $0F
	db $09
	db $10
	db $28
	db $00
	db $00
	db $14
	db $0B
	db $0C
	db $26
	db $2C
	db $28
	db $00
	db $00
	db $1B
	db $14
	db $15
	db $1A
	db $25
	db $2D
	db $31
	db $33
	db $2F
	db $13
	db $00
	db $23
	db $1D
	db $1A
	db $00
	db $00
	db $19
	db $34
	db $25
	db $31
	db $1D
	db $64
	db $1F
	db $00
	db $19
	db $18
	db $12
	db $15
	db $25
	db $18
	db $15
	db $00
	db $2F
	db $1C
	db $00
	db $00
	db $19
	db $18
	db $00
	db $20
	db $19
	db $18
	db $20
	db $33
	db $1F
	db $1C
	db $17
	db $16
	db $22
	db $1C
	db $20
	db $29
	db $0A
	db $1E
	db $12
	db $2B
	db $37
	db $14
	db $14
	db $14
	db $13
	db $1A
	db $1B
	db $25
	db $1A
	db $34
	db $20
	db $1F
	db $12
	db $64
	db $00
	db $00
	db $14
	db $25
	db $1A
	db $1B
	db $1A
	db $1F
	db $33
	db $0F
	db $00
	db $14
	db $16
	db $17
	db $16
	db $1B
	db $32
	db $25
	db $1B
	db $00
	db $35
	db $1F
	db $00
	db $20
	db $33
	db $00
	db $21
	db $1A
	db $1B
	db $14
	db $14
	db $00
	db $23
	db $13
	db $13
	db $17
	db $16
	db $1D
	db $13
	db $18
	db $19
	db $18
	db $19
	db $1C
	db $1F
	db $1A
	db $2F
	db $18
	db $00
	db $33
	db $19
	db $24
	db $32
	db $1B
	db $25
	db $18
	db $34
	db $23
	db $2F
	db $18
	db $00
	db $00
	db $19
	db $32
	db $1B
	db $1A
	db $15
	db $35
	db $20
	db $35
	db $00
	db $34
	db $00
	db $33
	db $21
	db $1E
	db $1F
	db $00
	db $25
	db $00
	db $20
	db $1F
	db $19
	db $16
	db $17
	db $16
	db $1D
	db $24
	db $00
	db $00
	db $29
	db $14
	db $64
	db $0E
	db $00
	db $13
	db $1C
	db $16
	db $17
	db $08
	db $0B
	db $0B
	db $10
	db $00
	db $29
	db $29
	db $00
	db $20
	db $00
	db $0C
	db $21
	db $28
	db $00
	db $16
	db $1D
	db $00
	db $0C
	db $34
	db $2F
	db $17
	db $16
	db $17
	db $1D
	db $24
	db $00
	db $32
	db $1A
	db $18
	db $00
	db $35
	db $00
	db $1C
	db $64
	db $33
	db $25
	db $1E
	db $12
	db $21
	db $19
	db $01
	db $1E
	db $30
	db $2F
	db $14
	db $14
	db $14
	db $14
	db $1A
	db $00
	db $00
	db $19
	db $34
	db $25
	db $2E
	db $14
	db $64
	db $19
	db $23
	db $13
	db $32
	db $1A
	db $1B
	db $15
	db $35
	db $0D
	db $00
	db $00
	db $14
	db $00
	db $29
	db $21
	db $15
	db $15
	db $1A
	db $25
	db $00
	db $36
	db $00
	db $2F
	db $16
	db $14
	db $23
	db $1D
	db $18
	db $12
	db $00
	db $25
	db $18
	db $1E
	db $12
	db $1D
	db $18
	db $00
	db $1E
	db $19
	db $1A
	db $15
	db $1B
	db $14
	db $35
	db $2E
	db $2E
	db $1A
	db $1C
	db $23
	db $16
	db $16
	db $18
	db $00
	db $00
	db $1F
	db $1E
	db $20
	db $1F
	db $00
	db $20
	db $00
	db $00
	db $21
	db $15
	db $1A
	db $1B
	db $25
	db $00
	db $30
	db $30
	db $37
	db $22
	db $14
	db $14
	db $13
	db $18
	db $00
	db $30
	db $19
	db $18
	db $30
	db $1E
	db $2F
	db $18
	db $1E
	db $00
	db $37
	db $1C
	db $2E
	db $2E
	db $1D
	db $38
	db $2E
	db $2E
	db $1D
	db $24
	db $00
	db $00
	db $1D
	db $18
	db $12
	db $00
	db $1F
	db $18
	db $36
	db $29
	db $00
	db $14
	db $14
	db $16
	db $17
	db $15
	db $15
	db $1A
	db $15
	db $00
	db $00
	db $34
	db $00
	db $17
	db $16
	db $17
	db $16
	db $15
	db $1B
	db $13
	db $25
	db $00
	db $0C
	db $1A
	db $19
	db $20
	db $00
	db $00
	db $19
	db $18
	db $00
	db $26
	db $19
	db $38
	db $29
	db $00
	db $37
	db $13
	db $20
	db $21
	db $13
	db $24
	db $15
	db $1A
	db $15
	db $38
	db $21
	db $00
	db $20
	db $24
	db $00
	db $20
	db $21
	db $15
	db $1E
	db $12
	db $1F
	db $00
	db $12
	db $36
	db $12
	db $16
	db $14
	db $14
	db $1C
	db $15
	db $1B
	db $24
	db $25
	db $00
	db $00
	db $1E
	db $37
	db $16
	db $17
	db $22
	db $13
	db $18
	db $39
	db $15
	db $15
	db $38
	db $00
	db $36
	db $64
	db $13
	db $23
	db $14
	db $38
	db $15
	db $00
	db $0D
	db $19
	db $29
	db $0D
	db $00
	db $19
	db $20
	db $00
	db $0C
	db $19
	db $24
	db $15
	db $15
	db $15
	db $18
	db $2A
	db $00
	db $00
	db $18
	db $1F
	db $1F
	db $21
	db $1E
	db $00
	db $00
	db $1F
	db $00
	db $00
	db $00
	db $00
	db $20
	db $00
	db $00
	db $21
	db $15
	db $1B
	db $15
	db $25
	db $00
	db $2E
	db $20
	db $19
	db $20
	db $00
	db $00
	db $19
	db $18
	db $00
	db $00
	db $19
	db $1C
	db $00
	db $36
	db $1D
	db $18
	db $00
	db $1E
	db $19
	db $24
	db $15
	db $1A
	db $15
	db $38
	db $2E
	db $2E
	db $09
	db $24
	db $00
	db $00
	db $25
	db $1E
	db $00
	db $00
	db $14
	db $09
	db $09
	db $08
	db $13
	db $14
	db $14
	db $14
	db $13
	db $1C
	db $2E
	db $16
	db $14
	db $38
	db $17
	db $2E
	db $13
	db $1A
	db $00
	db $00
	db $13
	db $24
	db $1B
	db $15
	db $14
	db $38
	db $23
	db $2E
	db $1A
	db $13
	db $1C
	db $16
	db $17
	db $14
	db $25
	db $32
	db $25
	db $15
	db $39
	db $35
	db $19
	db $20
	db $00
	db $33
	db $19
	db $18
	db $20
	db $00
	db $19
	db $18
	db $34
	db $32
	db $19
	db $18
	db $00
	db $33
	db $19
	db $14
	db $18
	db $19
	db $14
	db $13
	db $24
	db $2F
	db $13
	db $13
	db $18
	db $1D
	db $13
	db $18
	db $00
	db $1C
	db $1F
	db $18
	db $1F
	db $1E
	db $00
	db $38
	db $17
	db $16
	db $17
	db $15
	db $25
	db $15
	db $15
	db $00
	db $39
	db $00
	db $00
	db $16
	db $17
	db $16
	db $17
	db $15
	db $32
	db $32
	db $25
	db $00
	db $33
	db $33
	db $19
	db $20
	db $00
	db $00
	db $19
	db $24
	db $32
	db $32
	db $25
	db $18
	db $35
	db $35
	db $37
	db $18
	db $33
	db $33
	db $13
	db $18
	db $20
	db $1E
	db $19
	db $18
	db $00
	db $20
	db $19
	db $18
	db $00
	db $2A
	db $1D
	db $18
	db $00
	db $00
	db $1F
	db $18
	db $00
	db $32
	db $00
	db $18
	db $00
	db $33
	db $21
	db $1E
	db $20
	db $21
	db $1F
	db $21
	db $00
	db $00
	db $20
	db $20
	db $1F
	db $00
	db $21
	db $1E
	db $00
	db $00
	db $19
	db $00
	db $2A
	db $3A
	db $19
	db $20
	db $1F
	db $1F
	db $19
	db $18
	db $00
	db $00
	db $19
	db $24
	db $15
	db $15
	db $2F
	db $1C
	db $16
	db $22
	db $19
	db $18
	db $00
	db $20
	db $1F
	db $18
	db $12
	db $34
	db $00
	db $14
	db $1C
	db $16
	db $17
	db $15
	db $32
	db $15
	db $25
	db $00
	db $33
	db $3A
	db $19
	db $20
	db $00
	db $1F
	db $19
	db $1A
	db $00
	db $00
	db $1B
	db $34
	db $2A
	db $20
	db $00
	db $1C
	db $00
	db $00
	db $21
	db $1A
	db $15
	db $15
	db $15
	db $00
	db $08
	db $00
	db $12
	db $16
	db $17
	db $16
	db $2C
	db $1E
	db $00
	db $00
	db $1F
	db $3A
	db $29
	db $0D
	db $00
	db $2D
	db $16
	db $17
	db $16
	db $1E
	db $00
	db $00
	db $19
	db $00
	db $26
	db $30
	db $37
	db $17
	db $16
	db $14
	db $13
	db $24
	db $1E
	db $14
	db $14
	db $18
	db $15
	db $15
	db $1B
	db $14
	db $14
	db $14
	db $21
	db $32
	db $15
	db $32
	db $15
	db $34
	db $3B
	db $34
	db $00
	db $17
	db $1C
	db $16
	db $17
	db $15
	db $15
	db $15
	db $25
	db $21
	db $22
	db $31
	db $19
	db $20
	db $1F
	db $00
	db $19
	db $18
	db $12
	db $3A
	db $19
	db $18
	db $1C
	db $22
	db $19
	db $18
	db $1E
	db $1F
	db $19
	db $18
	db $3A
	db $00
	db $15
	db $18
	db $19
	db $23
	db $00
	db $18
	db $1F
	db $00
	db $21
	db $1E
	db $21
	db $1F
	db $25
	db $1F
	db $00
	db $21
	db $2F
	db $16
	db $17
	db $16
	db $1D
	db $18
	db $00
	db $00
	db $1F
	db $18
	db $00
	db $00
	db $00
	db $1C
	db $16
	db $16
	db $16
	db $1E
	db $00
	db $00
	db $1F
	db $00
	db $3A
	db $3B
	db $00
	db $20
	db $1F
	db $1E
	db $21
	db $1E
	db $00
	db $00
	db $19
	db $00
	db $00
	db $00
	db $19
	db $16
	db $16
	db $16
	db $1D
	db $24
	db $1B
	db $15
	db $1F
	db $18
	db $20
	db $21
	db $00
	db $3F
	db $42
	db $42
	db $42
	db $15
	db $1A
	db $15
	db $15
	db $00
	db $34
	db $0D
	db $00
	db $42
	db $42
	db $42
	db $42
	db $1E
	db $00
	db $00
	db $19
	db $00
	db $2A
	db $0F
	db $1B
	db $45
	db $00
	db $00
	db $44
	db $32
	db $00
	db $00
	db $1F
	db $34
	db $23
	db $00
	db $00
	db $3F
	db $42
	db $42
	db $42
	db $15
	db $25
	db $1A
	db $15
	db $21
	db $39
	db $00
	db $1E
	db $42
	db $42
	db $42
	db $42
	db $15
	db $1A
	db $1B
	db $25
	db $00
	db $00
	db $00
	db $2F
	db $20
	db $00
	db $00
	db $19
	db $24
	db $1B
	db $15
	db $15
	db $18
	db $09
	db $00
	db $23
	db $38
	db $17
	db $16
	db $17
	db $15
	db $1A
	db $15
	db $1F
	db $17
	db $00
	db $00
	db $00
	db $20
	db $1F
	db $1E
	db $17
	db $15
	db $1A
	db $15
	db $25
	db $00
	db $33
	db $00
	db $37
	db $17
	db $16
	db $22
	db $13
	db $18
	db $3A
	db $12
	db $19
	db $18
	db $1F
	db $1E
	db $19
	db $18
	db $00
	db $00
	db $19
	db $18
	db $00
	db $46
	db $19
	db $18
	db $45
	db $00
	db $19
	db $18
	db $00
	db $47
	db $19
	db $18
	db $00
	db $20
	db $1F
	db $18
	db $20
	db $01
	db $00
	db $18
	db $00
	db $15
	db $21
	db $15
	db $15
	db $1B
	db $15
	db $00
	db $3B
	db $3A
	db $00
	db $20
	db $1E
	db $1F
	db $21
	db $15
	db $1A
	db $15
	db $25
	db $00
	db $00
	db $22
	db $2F
	db $20
	db $1E
	db $1F
	db $19
	db $18
	db $0C
	db $0D
	db $1F
	db $18
	db $0D
	db $00
	db $0C
	db $1C
	db $17
	db $16
	db $17
	db $15
	db $1A
	db $1B
	db $25
	db $29
	db $00
	db $0F
	db $19
	db $20
	db $26
	db $00
	db $19
	db $3D
	db $3E
	db $41
	db $41
	db $3D
	db $00
	db $2A
	db $42
	db $3F
	db $42
	db $42
	db $42
	db $41
	db $3E
	db $41
	db $3E
	db $42
	db $00
	db $2B
	db $44
	db $45
	db $40
	db $3C
	db $3C
	db $43
	db $46
	db $00
	db $44
	db $43
	db $45
	db $48
	db $44
	db $43
	db $00
	db $47
	db $44
	db $3D
	db $3E
	db $3D
	db $41
	db $43
	db $45
	db $35
	db $00
	db $43
	db $00
	db $33
	db $46
	db $41
	db $3D
	db $3E
	db $3E
	db $00
	db $12
	db $3A
	db $44
	db $45
	db $47
	db $48
	db $44
	db $18
	db $00
	db $00
	db $1F
	db $38
	db $64
	db $00
	db $36
	db $13
	db $38
	db $17
	db $2C
	db $15
	db $1A
	db $15
	db $25
	db $3A
	db $2A
	db $20
	db $19
	db $2D
	db $00
	db $00
	db $19
	db $18
	db $00
	db $00
	db $1B
	db $32
	db $00
	db $21
	db $00
	db $34
	db $1F
	db $00
	db $21
	db $15
	db $32
	db $25
	db $14
	db $00
	db $33
	db $37
	db $13
	db $17
	db $16
	db $2C
	db $13
	db $18
	db $20
	db $00
	db $19
	db $38
	db $34
	db $1F
	db $2F
	db $2D
	db $16
	db $23
	db $1D
	db $18
	db $46
	db $00
	db $19
	db $18
	db $45
	db $48
	db $19
	db $38
	db $00
	db $47
	db $19
	db $18
	db $00
	db $21
	db $19
	db $18
	db $20
	db $3B
	db $19
	db $18
	db $00
	db $1E
	db $19
	db $18
	db $00
	db $00
	db $1F
	db $18
	db $00
	db $36
	db $00
	db $38
	db $22
	db $1C
	db $16
	db $1E
	db $00
	db $00
	db $19
	db $4D
	db $20
	db $2B
	db $37
	db $4B
	db $16
	db $14
	db $13
	db $24
	db $15
	db $15
	db $15
	db $1C
	db $4E
	db $22
	db $01
	db $18
	db $00
	db $1F
	db $25
	db $1E
	db $00
	db $0C
	db $1F
	db $29
	db $0D
	db $0F
	db $00
	db $14
	db $17
	db $17
	db $16
	db $41
	db $3C
	db $3D
	db $3C
	db $00
	db $48
	db $34
	db $3E
	db $45
	db $47
	db $00
	db $44
	db $43
	db $41
	db $41
	db $3E
	db $3D
	db $41
	db $3D
	db $48
	db $43
	db $47
	db $42
	db $3C
	db $43
	db $00
	db $42
	db $44
	db $47
	db $45
	db $34
	db $44
	db $3F
	db $00
	db $00
	db $44
	db $43
	db $00
	db $2A
	db $40
	db $43
	db $48
	db $46
	db $44
	db $43
	db $47
	db $00
	db $44
	db $43
	db $12
	db $00
	db $48
	db $43
	db $36
	db $00
	db $12
	db $3C
	db $3C
	db $42
	db $49
	db $15
	db $32
	db $1A
	db $32
	db $00
	db $33
	db $34
	db $33
	db $17
	db $16
	db $17
	db $16
	db $15
	db $3A
	db $21
	db $1F
	db $00
	db $1F
	db $00
	db $00
	db $16
	db $17
	db $16
	db $17
	db $1E
	db $00
	db $00
	db $19
	db $00
	db $15
	db $1E
	db $37
	db $17
	db $16
	db $22
	db $13
	db $24
	db $1A
	db $15
	db $15
	db $38
	db $00
	db $3B
	db $00
	db $24
	db $00
	db $1E
	db $21
	db $25
	db $1A
	db $25
	db $15
	db $1F
	db $12
	db $1F
	db $12
	db $16
	db $14
	db $14
	db $1C
	db $15
	db $00
	db $00
	db $19
	db $2A
	db $42
	db $4C
	db $19
	db $17
	db $16
	db $4A
	db $1D
	db $18
	db $20
	db $00
	db $19
	db $18
	db $00
	db $4F
	db $15
	db $38
	db $22
	db $14
	db $1C
	db $32
	db $15
	db $15
	db $25
	db $33
	db $4D
	db $4E
	db $1D
	db $17
	db $4B
	db $20
	db $19
	db $24
	db $15
	db $1B
	db $15
	db $32
	db $00
	db $00
	db $16
	db $34
	db $00
	db $1E
	db $21
	db $1E
	db $21
	db $20
	db $1F
	db $20
	db $00
	db $00
	db $21
	db $16
	db $17
	db $16
	db $17
	db $15
	db $25
	db $32
	db $25
	db $20
	db $39
	db $33
	db $37
	db $16
	db $00
	db $00
	db $13
	db $43
	db $00
	db $00
	db $48
	db $3D
	db $30
	db $3B
	db $30
	db $3F
	db $49
	db $49
	db $49
	db $47
	db $00
	db $3D
	db $3E
	db $00
	db $47
	db $34
	db $44
	db $42
	db $50
	db $42
	db $40
	db $3D
	db $00
	db $46
	db $48
	db $34
	db $47
	db $45
	db $45
	db $3F
	db $50
	db $42
	db $42
	db $47
	db $46
	db $31
	db $48
	db $00
	db $48
	db $47
	db $00
	db $42
	db $42
	db $42
	db $42
	db $41
	db $41
	db $3D
	db $3E
	db $00
	db $00
	db $35
	db $44
	db $45
	db $00
	db $33
	db $44
	db $32
	db $32
	db $1A
	db $15
	db $34
	db $33
	db $35
	db $00
	db $64
	db $00
	db $33
	db $21
	db $15
	db $15
	db $32
	db $1A
	db $00
	db $4F
	db $33
	db $01
	db $22
	db $14
	db $14
	db $14
	db $15
	db $1B
	db $1A
	db $25
	db $00
	db $00
	db $4D
	db $37
	db $14
	db $15
	db $52
	db $13
	db $18
	db $00
	db $00
	db $1F
	db $38
	db $2E
	db $4C
	db $00
	db $13
	db $23
	db $4A
	db $14
	db $15
	db $25
	db $24
	db $15
	db $00
	db $1F
	db $53
	db $01
	db $14
	db $14
	db $14
	db $14
	db $15
	db $1A
	db $15
	db $25
	db $00
	db $21
	db $22
	db $19
	db $14
	db $00
	db $1F
	db $19
	db $24
	db $15
	db $1A
	db $15
	db $18
	db $3A
	db $00
	db $30
	db $18
	db $1F
	db $2A
	db $2C
	db $15
	db $1B
	db $1E
	db $19
	db $3A
	db $00
	db $30
	db $37
	db $2D
	db $16
	db $14
	db $13
	db $18
	db $00
	db $00
	db $1F
	db $18
	db $14
	db $31
	db $00
	db $1C
	db $2C
	db $14
	db $14
	db $15
	db $15
	db $1B
	db $32
	db $00
	db $22
	db $00
	db $34
	db $14
	db $2D
	db $16
	db $17
	db $15
	db $00
	db $20
	db $25
	db $00
	db $16
	db $00
	db $37
	db $20
	db $00
	db $00
	db $1B
	db $3D
	db $41
	db $41
	db $3D
	db $43
	db $50
	db $42
	db $00
	db $43
	db $00
	db $00
	db $46
	db $3E
	db $41
	db $54
	db $3E
	db $00
	db $3B
	db $36
	db $3B
	db $42
	db $49
	db $49
	db $49
	db $3C
	db $41
	db $41
	db $3C
	db $3C
	db $00
	db $00
	db $3C
	db $3C
	db $42
	db $42
	db $3C
	db $3D
	db $41
	db $41
	db $3C
	db $3F
	db $01
	db $4F
	db $47
	db $3C
	db $3C
	db $3C
	db $3C
	db $43
	db $00
	db $30
	db $44
	db $00
	db $30
	db $47
	db $40
	db $43
	db $47
	db $00
	db $44
	db $18
	db $55
	db $20
	db $19
	db $18
	db $35
	db $21
	db $2F
	db $18
	db $1E
	db $00
	db $19
	db $1A
	db $15
	db $32
	db $15
	db $34
	db $4C
	db $33
	db $00
	db $64
	db $51
	db $1E
	db $16
	db $1B
	db $00
	db $5F
	db $15
	db $00
	db $21
	db $52
	db $00
	db $17
	db $16
	db $4B
	db $16
	db $15
	db $1B
	db $15
	db $15
	db $4D
	db $3A
	db $12
	db $4C
	db $4B
	db $1D
	db $1C
	db $4A
	db $15
	db $32
	db $1A
	db $61
	db $00
	db $33
	db $20
	db $5B
	db $20
	db $1E
	db $00
	db $5B
	db $24
	db $55
	db $00
	db $19
	db $18
	db $35
	db $00
	db $19
	db $18
	db $1E
	db $2A
	db $1D
	db $18
	db $20
	db $39
	db $25
	db $18
	db $34
	db $20
	db $37
	db $18
	db $00
	db $00
	db $13
	db $60
	db $15
	db $15
	db $32
	db $57
	db $0D
	db $00
	db $33
	db $57
	db $00
	db $29
	db $21
	db $15
	db $15
	db $1B
	db $32
	db $63
	db $01
	db $4F
	db $33
	db $4B
	db $14
	db $14
	db $14
	db $15
	db $15
	db $15
	db $14
	db $00
	db $21
	db $22
	db $13
	db $14
	db $00
	db $1F
	db $1B
	db $18
	db $00
	db $00
	db $19
	db $38
	db $4E
	db $4E
	db $22
	db $1A
	db $00
	db $00
	db $19
	db $43
	db $00
	db $00
	db $48
	db $43
	db $47
	db $2B
	db $3A
	db $3F
	db $50
	db $3C
	db $3C
	db $3E
	db $41
	db $3E
	db $41
	db $12
	db $12
	db $12
	db $12
	db $3F
	db $3F
	db $3F
	db $3F
	db $41
	db $3E
	db $3E
	db $41
	db $00
	db $30
	db $30
	db $30
	db $42
	db $3F
	db $3F
	db $3F
	db $3E
	db $41
	db $41
	db $41
	db $00
	db $45
	db $47
	db $45
	db $45
	db $00
	db $47
	db $46
	db $47
	db $00
	db $00
	db $44
	db $00
	db $00
	db $00
	db $44
	db $42
	db $42
	db $42
	db $40
	db $18
	db $00
	db $00
	db $19
	db $1C
	db $00
	db $00
	db $19
	db $18
	db $00
	db $00
	db $19
	db $18
	db $51
	db $00
	db $25
	db $18
	db $51
	db $00
	db $5A
	db $1C
	db $4A
	db $00
	db $5B
	db $14
	db $32
	db $15
	db $15
	db $24
	db $35
	db $3A
	db $00
	db $18
	db $33
	db $4D
	db $16
	db $15
	db $15
	db $32
	db $25
	db $00
	db $12
	db $34
	db $19
	db $17
	db $4C
	db $00
	db $19
	db $18
	db $55
	db $20
	db $5B
	db $18
	db $35
	db $21
	db $5B
	db $18
	db $1E
	db $00
	db $5B
	db $18
	db $00
	db $1F
	db $25
	db $18
	db $00
	db $00
	db $1D
	db $18
	db $00
	db $00
	db $19
	db $18
	db $20
	db $55
	db $25
	db $18
	db $34
	db $35
	db $19
	db $18
	db $00
	db $1E
	db $19
	db $57
	db $00
	db $0D
	db $1F
	db $57
	db $29
	db $00
	db $30
	db $59
	db $17
	db $16
	db $14
	db $15
	db $1A
	db $15
	db $25
	db $00
	db $65
	db $01
	db $37
	db $14
	db $14
	db $14
	db $13
	db $14
	db $2E
	db $2E
	db $1D
	db $13
	db $00
	db $34
	db $1F
	db $13
	db $17
	db $16
	db $17
	db $18
	db $00
	db $55
	db $1F
	db $1E
	db $00
	db $35
	db $00
	db $17
	db $22
	db $14
	db $16
	db $41
	db $3D
	db $41
	db $3E
	db $00
	db $00
	db $3B
	db $40
	db $45
	db $00
	db $3E
	db $3C
	db $3D
	db $41
	db $3D
	db $41
	db $43
	db $47
	db $3A
	db $47
	db $43
	db $48
	db $4D
	db $42
	db $41
	db $3D
	db $3D
	db $41
	db $00
	db $35
	db $35
	db $00
	db $45
	db $33
	db $33
	db $46
	db $47
	db $00
	db $55
	db $44
	db $00
	db $45
	db $35
	db $44
	db $45
	db $00
	db $47
	db $44
	db $3D
	db $41
	db $3E
	db $41
	db $43
	db $4D
	db $00
	db $00
	db $3F
	db $52
	db $48
	db $46
	db $1E
	db $00
	db $00
	db $19
	db $00
	db $23
	db $64
	db $19
	db $20
	db $00
	db $1E
	db $19
	db $60
	db $1B
	db $2E
	db $5B
	db $57
	db $21
	db $16
	db $5D
	db $59
	db $17
	db $16
	db $17
	db $38
	db $3A
	db $52
	db $14
	db $1A
	db $1F
	db $52
	db $13
	db $17
	db $16
	db $4B
	db $2C
	db $24
	db $51
	db $63
	db $1F
	db $38
	db $4A
	db $52
	db $00
	db $2D
	db $16
	db $4B
	db $17
	db $1E
	db $00
	db $21
	db $5B
	db $21
	db $1F
	db $00
	db $5B
	db $17
	db $16
	db $17
	db $5D
	db $1C
	db $00
	db $00
	db $19
	db $18
	db $1F
	db $21
	db $19
	db $18
	db $00
	db $00
	db $19
	db $18
	db $00
	db $00
	db $37
	db $38
	db $66
	db $2E
	db $13
	db $1A
	db $00
	db $00
	db $1B
	db $24
	db $15
	db $15
	db $14
	db $18
	db $22
	db $66
	db $13
	db $18
	db $19
	db $17
	db $2C
	db $14
	db $15
	db $15
	db $15
	db $2D
	db $17
	db $1F
	db $14
	db $13
	db $21
	db $17
	db $13
	db $15
	db $15
	db $15
	db $15
	db $56
	db $3A
	db $12
	db $5A
	db $59
	db $1D
	db $1C
	db $5D
	db $15
	db $15
	db $15
	db $61
	db $14
	db $15
	db $15
	db $5B
	db $56
	db $17
	db $17
	db $5D
	db $3F
	db $55
	db $48
	db $41
	db $3C
	db $35
	db $47
	db $45
	db $3C
	db $3F
	db $42
	db $42
	db $47
	db $00
	db $52
	db $3C
	db $00
	db $47
	db $68
	db $3C
	db $42
	db $50
	db $3C
	db $3C
	db $3F
	db $00
	db $00
	db $44
	db $43
	db $48
	db $00
	db $44
	db $43
	db $00
	db $48
	db $44
	db $3F
	db $45
	db $00
	db $48
	db $43
	db $34
	db $00
	db $00
	db $3F
	db $42
	db $42
	db $42
	db $41
	db $5F
	db $00
	db $44
	db $3A
	db $52
	db $47
	db $40
	db $40
	db $69
	db $42
	db $40
	db $18
	db $00
	db $55
	db $19
	db $18
	db $00
	db $35
	db $39
	db $1C
	db $22
	db $14
	db $17
	db $14
	db $15
	db $25
	db $32
	db $1A
	db $1F
	db $39
	db $33
	db $17
	db $22
	db $17
	db $16
	db $15
	db $1B
	db $24
	db $15
	db $00
	db $00
	db $53
	db $12
	db $17
	db $22
	db $14
	db $14
	db $15
	db $1A
	db $1B
	db $15
	db $3A
	db $00
	db $00
	db $12
	db $14
	db $6A
	db $6A
	db $14
	db $15
	db $15
	db $1B
	db $14
	db $01
	db $00
	db $00
	db $13
	db $14
	db $00
	db $00
	db $1B
	db $18
	db $00
	db $00
	db $1D
	db $18
	db $20
	db $1E
	db $19
	db $18
	db $00
	db $00
	db $19
	db $1C
	db $66
	db $2E
	db $1D
	db $38
	db $2E
	db $66
	db $1D
	db $18
	db $00
	db $00
	db $19
	db $6B
	db $1F
	db $32
	db $25
	db $35
	db $1E
	db $34
	db $1D
	db $24
	db $00
	db $00
	db $19
	db $24
	db $19
	db $1A
	db $25
	db $18
	db $1B
	db $4D
	db $39
	db $38
	db $17
	db $4B
	db $16
	db $14
	db $1B
	db $1B
	db $25
	db $1A
	db $36
	db $36
	db $2F
	db $16
	db $4A
	db $4A
	db $19
	db $57
	db $1A
	db $15
	db $15
	db $57
	db $00
	db $00
	db $6B
	db $59
	db $17
	db $22
	db $14
	db $15
	db $1B
	db $1B
	db $15
	db $01
	db $30
	db $30
	db $30
	db $14
	db $14
	db $14
	db $14
	db $15
	db $1B
	db $1B
	db $1A
	db $01
	db $00
	db $30
	db $30
	db $14
	db $22
	db $14
	db $14
	db $1E
	db $00
	db $62
	db $1F
	db $00
	db $22
	db $51
	db $12
	db $14
	db $13
	db $4A
	db $14
	db $15
	db $1B
	db $1A
	db $15
	db $3A
	db $12
	db $3A
	db $12
	db $14
	db $14
	db $14
	db $14
	db $15
	db $15
	db $15
	db $32
	db $01
	db $00
	db $4F
	db $33
	db $14
	db $22
	db $14
	db $14
	db $15
	db $25
	db $24
	db $15
	db $01
	db $1F
	db $1E
	db $01
	db $14
	db $6C
	db $6C
	db $14
	db $15
	db $25
	db $24
	db $15
	db $01
	db $1F
	db $1E
	db $01
	db $14
	db $6A
	db $6A
	db $14
	db $15
	db $15
	db $15
	db $25
	db $01
	db $22
	db $12
	db $19
	db $14
	db $15
	db $1E
	db $19
	db $24
	db $15
	db $15
	db $15
	db $18
	db $65
	db $29
	db $00
	db $18
	db $08
	db $00
	db $21
	db $15
	db $00
	db $00
	db $19
	db $00
	db $0C
	db $08
	db $19
	db $16
	db $17
	db $16
	db $1D
	db $1C
	db $00
	db $00
	db $1D
	db $1C
	db $1F
	db $00
	db $1D
	db $1C
	db $00
	db $1E
	db $1D
	db $75
	db $00
	db $00
	db $76
	db $70
	db $7A
	db $78
	db $76
	db $75
	db $00
	db $00
	db $76
	db $70
	db $00
	db $3A
	db $76
	db $75
	db $12
	db $7A
	db $76
	db $75
	db $79
	db $00
	db $6F
	db $7C
	db $80
	db $80
	db $7D
	db $83
	db $82
	db $81
	db $82
	db $7E
	db $81
	db $82
	db $84
	db $7C
	db $80
	db $80
	db $84
	db $82
	db $82
	db $4C
	db $84
	db $7B
	db $7E
	db $99
	db $7F
	db $7C
	db $7C
	db $80
	db $80
	db $83
	db $34
	db $97
	db $00
	db $83
	db $00
	db $87
	db $86
	db $80
	db $7C
	db $80
	db $80
	db $00
	db $2A
	db $81
	db $00
	db $82
	db $81
	db $82
	db $81
	db $80
	db $7C
	db $80
	db $80
	db $00
	db $33
	db $00
	db $6B
	db $81
	db $82
	db $81
	db $7B
	db $80
	db $7B
	db $7C
	db $7D
	db $00
	db $84
	db $35
	db $84
	db $7E
	db $00
	db $33
	db $84
	db $24
	db $15
	db $1A
	db $15
	db $18
	db $00
	db $97
	db $00
	db $18
	db $1F
	db $1E
	db $21
	db $15
	db $1A
	db $15
	db $15
	db $00
	db $21
	db $4C
	db $00
	db $16
	db $17
	db $4A
	db $5A
	db $15
	db $32
	db $15
	db $61
	db $01
	db $33
	db $97
	db $5C
	db $24
	db $1E
	db $1E
	db $5B
	db $6D
	db $6E
	db $72
	db $72
	db $6E
	db $35
	db $77
	db $00
	db $75
	db $33
	db $00
	db $78
	db $72
	db $97
	db $2A
	db $71
	db $00
	db $79
	db $00
	db $76
	db $73
	db $74
	db $73
	db $6D
	db $75
	db $00
	db $77
	db $7A
	db $6E
	db $78
	db $00
	db $79
	db $70
	db $73
	db $74
	db $73
	db $72
	db $72
	db $6E
	db $6F
	db $00
	db $12
	db $3A
	db $76
	db $77
	db $79
	db $7A
	db $76
	db $8A
	db $00
	db $00
	db $8B
	db $91
	db $96
	db $30
	db $92
	db $8C
	db $00
	db $95
	db $8D
	db $75
	db $00
	db $00
	db $7A
	db $6E
	db $2A
	db $74
	db $00
	db $75
	db $00
	db $00
	db $78
	db $79
	db $00
	db $00
	db $76
	db $00
	db $79
	db $2B
	db $71
	db $74
	db $73
	db $6D
	db $6D
	db $7C
	db $80
	db $80
	db $84
	db $7B
	db $9B
	db $82
	db $7F
	db $83
	db $82
	db $7F
	db $7B
	db $7C
	db $7C
	db $80
	db $80
	db $35
	db $00
	db $6B
	db $00
	db $34
	db $00
	db $87
	db $86
	db $87
	db $00
	db $00
	db $84
	db $00
	db $00
	db $00
	db $84
	db $82
	db $81
	db $82
	db $7F
	db $7C
	db $7C
	db $80
	db $80
	db $83
	db $00
	db $3B
	db $00
	db $83
	db $00
	db $87
	db $86
	db $80
	db $7C
	db $7D
	db $7D
	db $00
	db $2A
	db $81
	db $84
	db $85
	db $00
	db $87
	db $84
	db $7C
	db $97
	db $85
	db $84
	db $83
	db $35
	db $86
	db $84
	db $83
	db $87
	db $00
	db $84
	db $18
	db $00
	db $C2
	db $19
	db $18
	db $C2
	db $1F
	db $19
	db $18
	db $1F
	db $00
	db $19
	db $24
	db $1B
	db $15
	db $5C
	db $18
	db $6B
	db $20
	db $5C
	db $18
	db $1E
	db $00
	db $5B
	db $1C
	db $00
	db $97
	db $5B
	db $18
	db $97
	db $1E
	db $5B
	db $18
	db $1E
	db $1F
	db $5B
	db $75
	db $00
	db $77
	db $7A
	db $70
	db $00
	db $00
	db $79
	db $75
	db $7A
	db $00
	db $78
	db $72
	db $6E
	db $72
	db $72
	db $00
	db $2A
	db $77
	db $00
	db $77
	db $00
	db $00
	db $78
	db $6F
	db $6E
	db $72
	db $72
	db $00
	db $35
	db $6B
	db $00
	db $77
	db $33
	db $79
	db $78
	db $79
	db $00
	db $00
	db $76
	db $00
	db $79
	db $30
	db $6F
	db $77
	db $00
	db $79
	db $76
	db $8A
	db $00
	db $00
	db $8B
	db $91
	db $95
	db $94
	db $92
	db $91
	db $96
	db $00
	db $92
	db $75
	db $00
	db $00
	db $7A
	db $75
	db $79
	db $77
	db $79
	db $75
	db $00
	db $00
	db $78
	db $72
	db $72
	db $72
	db $6F
	db $00
	db $79
	db $65
	db $76
	db $77
	db $00
	db $72
	db $76
	db $83
	db $7D
	db $7C
	db $7D
	db $7E
	db $4D
	db $00
	db $84
	db $7E
	db $9A
	db $7F
	db $84
	db $83
	db $00
	db $00
	db $88
	db $83
	db $00
	db $12
	db $00
	db $7E
	db $9C
	db $7E
	db $82
	db $80
	db $80
	db $80
	db $80
	db $00
	db $00
	db $00
	db $00
	db $81
	db $82
	db $81
	db $82
	db $87
	db $00
	db $97
	db $84
	db $00
	db $88
	db $87
	db $84
	db $82
	db $81
	db $9C
	db $7B
	db $83
	db $00
	db $00
	db $84
	db $83
	db $C2
	db $9C
	db $84
	db $83
	db $88
	db $87
	db $84
	db $83
	db $30
	db $86
	db $84
	db $83
	db $87
	db $30
	db $84
	db $83
	db $00
	db $87
	db $84
	db $18
	db $00
	db $00
	db $19
	db $1C
	db $2E
	db $66
	db $1D
	db $18
	db $00
	db $00
	db $19
	db $18
	db $00
	db $97
	db $5C
	db $56
	db $00
	db $1E
	db $5B
	db $57
	db $6C
	db $17
	db $5D
	db $18
	db $00
	db $21
	db $5B
	db $56
	db $20
	db $00
	db $5B
	db $57
	db $00
	db $1E
	db $5D
	db $75
	db $00
	db $97
	db $7A
	db $75
	db $00
	db $35
	db $00
	db $75
	db $00
	db $79
	db $78
	db $79
	db $00
	db $00
	db $7A
	db $00
	db $7A
	db $78
	db $00
	db $77
	db $79
	db $00
	db $78
	db $79
	db $00
	db $00
	db $7A
	db $00
	db $00
	db $00
	db $00
	db $77
	db $00
	db $00
	db $78
	db $79
	db $00
	db $77
	db $76
	db $00
	db $77
	db $35
	db $76
	db $77
	db $00
	db $33
	db $76
	db $91
	db $00
	db $3A
	db $92
	db $91
	db $12
	db $96
	db $92
	db $91
	db $95
	db $00
	db $92
	db $75
	db $78
	db $00
	db $7A
	db $75
	db $97
	db $2A
	db $77
	db $6D
	db $70
	db $73
	db $74
	db $79
	db $00
	db $77
	db $76
	db $7A
	db $78
	db $00
	db $76
	db $77
	db $00
	db $7A
	db $76
	db $7C
	db $80
	db $80
	db $84
	db $7E
	db $9B
	db $9B
	db $7B
	db $7E
	db $82
	db $81
	db $81
	db $7C
	db $7B
	db $7C
	db $80
	db $34
	db $88
	db $00
	db $00
	db $81
	db $82
	db $81
	db $82
	db $80
	db $7D
	db $80
	db $80
	db $00
	db $62
	db $63
	db $00
	db $81
	db $99
	db $9A
	db $82
	db $80
	db $80
	db $7D
	db $80
	db $00
	db $4D
	db $85
	db $00
	db $81
	db $9A
	db $81
	db $82
	db $87
	db $12
	db $00
	db $84
	db $00
	db $36
	db $00
	db $84
	db $81
	db $99
	db $9C
	db $7B
	db $83
	db $85
	db $85
	db $84
	db $83
	db $35
	db $35
	db $7F
	db $83
	db $33
	db $33
	db $84
	db $18
	db $00
	db $00
	db $37
	db $14
	db $9F
	db $9E
	db $1A
	db $13
	db $16
	db $17
	db $16
	db $58
	db $15
	db $15
	db $61
	db $15
	db $15
	db $15
	db $5B
	db $17
	db $16
	db $17
	db $5D
	db $57
	db $21
	db $00
	db $61
	db $58
	db $00
	db $97
	db $5B
	db $13
	db $1E
	db $1E
	db $5B
	db $75
	db $00
	db $12
	db $7A
	db $75
	db $00
	db $79
	db $00
	db $70
	db $73
	db $74
	db $73
	db $79
	db $00
	db $00
	db $7A
	db $00
	db $00
	db $6B
	db $00
	db $74
	db $A0
	db $6D
	db $73
	db $79
	db $00
	db $00
	db $7A
	db $00
	db $00
	db $00
	db $00
	db $73
	db $74
	db $73
	db $74
	db $79
	db $00
	db $00
	db $76
	db $00
	db $00
	db $C2
	db $76
	db $73
	db $A0
	db $6D
	db $6D
	db $91
	db $00
	db $30
	db $92
	db $91
	db $30
	db $95
	db $8D
	db $91
	db $95
	db $00
	db $92
	db $6E
	db $72
	db $72
	db $72
	db $75
	db $C2
	db $00
	db $00
	db $75
	db $7A
	db $00
	db $78
	db $79
	db $00
	db $00
	db $7A
	db $00
	db $00
	db $30
	db $00
	db $74
	db $73
	db $70
	db $74
	db $80
	db $80
	db $80
	db $80
	db $00
	db $00
	db $00
	db $00
	db $85
	db $00
	db $00
	db $86
	db $80
	db $7D
	db $7C
	db $80
	db $00
	db $00
	db $34
	db $00
	db $81
	db $82
	db $81
	db $82
	db $7D
	db $7C
	db $80
	db $7C
	db $00
	db $34
	db $6B
	db $34
	db $81
	db $82
	db $7E
	db $82
	db $80
	db $7D
	db $7C
	db $80
	db $00
	db $97
	db $3A
	db $00
	db $85
	db $87
	db $88
	db $86
	db $80
	db $80
	db $80
	db $80
	db $00
	db $87
	db $86
	db $00
	db $81
	db $82
	db $81
	db $82
	db $87
	db $00
	db $00
	db $88
	db $00
	db $00
	db $2B
	db $00
	db $9C
	db $7B
	db $7B
	db $7E
	db $80
	db $7D
	db $7C
	db $80
	db $00
	db $97
	db $C2
	db $00
	db $85
	db $87
	db $88
	db $86
	db $80
	db $80
	db $7C
	db $80
	db $4D
	db $00
	db $33
	db $00
	db $9A
	db $9C
	db $81
	db $82
	db $15
	db $00
	db $20
	db $5B
	db $4D
	db $20
	db $00
	db $5B
	db $4B
	db $00
	db $16
	db $5D
	db $8A
	db $8E
	db $8E
	db $8E
	db $91
	db $97
	db $3A
	db $00
	db $91
	db $95
	db $96
	db $94
	db $8E
	db $8E
	db $8E
	db $8B
	db $00
	db $00
	db $94
	db $92
	db $93
	db $95
	db $00
	db $92
	db $8A
	db $8E
	db $8E
	db $8E
	db $91
	db $00
	db $00
	db $00
	db $91
	db $00
	db $00
	db $94
	db $8E
	db $8E
	db $8E
	db $8B
	db $00
	db $00
	db $00
	db $92
	db $93
	db $00
	db $00
	db $92
	db $91
	db $8F
	db $00
	db $92
	db $91
	db $00
	db $90
	db $92
	db $91
	db $00
	db $00
	db $92
	db $75
	db $00
	db $00
	db $76
	db $6E
	db $4F
	db $00
	db $76
	db $70
	db $6E
	db $79
	db $76
	db $6E
	db $6E
	db $6E
	db $6F
	db $75
	db $34
	db $34
	db $71
	db $75
	db $00
	db $00
	db $76
	db $83
	db $00
	db $00
	db $84
	db $83
	db $6B
	db $6B
	db $7F
	db $83
	db $87
	db $87
	db $84
	db $7C
	db $80
	db $80
	db $80
	db $83
	db $88
	db $00
	db $87
	db $83
	db $87
	db $88
	db $86
	db $80
	db $80
	db $7D
	db $7D
	db $00
	db $9C
	db $00
	db $84
	db $85
	db $00
	db $00
	db $84
	db $43
	db $00
	db $12
	db $48
	db $43
	db $00
	db $4C
	db $00
	db $43
	db $00
	db $51
	db $46
	db $41
	db $41
	db $41
	db $3E
	db $00
	db $3A
	db $00
	db $44
	db $45
	db $4D
	db $00
	db $44
	db $6E
	db $6E
	db $6E
	db $6F
	db $75
	db $35
	db $35
	db $71
	db $75
	db $33
	db $33
	db $76
	db $83
	db $62
	db $63
	db $84
	db $A6
	db $67
	db $68
	db $A7
	db $83
	db $51
	db $52
	db $84
	db $24
	db $32
	db $32
	db $14
	db $18
	db $34
	db $35
	db $2C
	db $1C
	db $00
	db $33
	db $13
	db $24
	db $97
	db $25
	db $25
	db $18
	db $1E
	db $A2
	db $19
	db $1C
	db $22
	db $14
	db $13
	db $91
	db $0D
	db $00
	db $92
	db $91
	db $0C
	db $29
	db $92
	db $91
	db $00
	db $26
	db $92
	db $91
	db $00
	db $00
	db $96
	db $91
	db $00
	db $00
	db $00
	db $8C
	db $8F
	db $90
	db $8F
	db $95
	db $00
	db $00
	db $92
	db $00
	db $00
	db $00
	db $92
	db $8F
	db $90
	db $A3
	db $8D
	db $91
	db $97
	db $C2
	db $96
	db $91
	db $95
	db $96
	db $00
	db $8C
	db $A3
	db $90
	db $8F
	db $95
	db $00
	db $65
	db $92
	db $00
	db $93
	db $8E
	db $92
	db $8F
	db $8F
	db $8F
	db $8D
	db $6E
	db $79
	db $00
	db $76
	db $75
	db $6B
	db $C2
	db $00
	db $75
	db $79
	db $7A
	db $78
	db $70
	db $A1
	db $A1
	db $71
	db $00
	db $00
	db $00
	db $76
	db $73
	db $74
	db $73
	db $71
	db $83
	db $00
	db $00
	db $84
	db $7E
	db $A4
	db $A5
	db $7F
	db $83
	db $00
	db $00
	db $84
	db $83
	db $55
	db $65
	db $84
	db $83
	db $35
	db $80
	db $84
	db $83
	db $87
	db $00
	db $7F
	db $83
	db $30
	db $00
	db $7F
	db $83
	db $80
	db $30
	db $84
	db $7C
	db $00
	db $80
	db $84
	db $43
	db $00
	db $51
	db $48
	db $43
	db $97
	db $5E
	db $00
	db $3C
	db $3C
	db $3C
	db $3F
	db $47
	db $5F
	db $00
	db $44
	db $00
	db $52
	db $46
	db $44
	db $42
	db $69
	db $42
	db $40
	db $70
	db $00
	db $00
	db $76
	db $75
	db $07
	db $07
	db $76
	db $75
	db $06
	db $06
	db $6F
	db $83
	db $67
	db $68
	db $84
	db $A6
	db $67
	db $68
	db $A7
	db $83
	db $51
	db $52
	db $84
	db $24
	db $97
	db $00
	db $15
	db $18
	db $1E
	db $C2
	db $00
	db $18
	db $00
	db $1F
	db $21
	db $8E
	db $8E
	db $8A
	db $8B
	db $2A
	db $91
	db $35
	db $92
	db $93
	db $95
	db $33
	db $92
	db $91
	db $0C
	db $0D
	db $96
	db $91
	db $00
	db $00
	db $0C
	db $91
	db $00
	db $26
	db $94
	db $8E
	db $8E
	db $8E
	db $8B
	db $00
	db $2B
	db $C2
	db $92
	db $A3
	db $89
	db $89
	db $89
	db $8A
	db $8A
	db $8E
	db $8B
	db $91
	db $35
	db $65
	db $8D
	db $91
	db $33
	db $8E
	db $92
	db $8A
	db $8E
	db $8E
	db $8E
	db $91
	db $30
	db $30
	db $01
	db $A8
	db $8E
	db $8E
	db $8B
	db $8E
	db $8E
	db $8A
	db $8B
	db $3A
	db $3A
	db $33
	db $92
	db $8A
	db $8E
	db $00
	db $92
	db $75
	db $3A
	db $00
	db $7A
	db $75
	db $7A
	db $3A
	db $00
	db $75
	db $A9
	db $7A
	db $78
	db $6F
	db $72
	db $6E
	db $6F
	db $00
	db $AA
	db $00
	db $71
	db $AC
	db $6D
	db $AB
	db $71
	db $7E
	db $86
	db $85
	db $7F
	db $A6
	db $A5
	db $A4
	db $A7
	db $83
	db $00
	db $00
	db $84
	db $7E
	db $55
	db $00
	db $80
	db $7B
	db $35
	db $86
	db $00
	db $7B
	db $7E
	db $81
	db $82
	db $87
	db $C2
	db $97
	db $88
	db $00
	db $80
	db $80
	db $00
	db $81
	db $82
	db $81
	db $82
	db $6F
	db $72
	db $72
	db $6E
	db $00
	db $2A
	db $77
	db $00
	db $77
	db $00
	db $00
	db $78
	db $72
	db $6E
	db $6F
	db $72
	db $3A
	db $00
	db $00
	db $12
	db $71
	db $AB
	db $AB
	db $70
	db $79
	db $00
	db $00
	db $76
	db $00
	db $06
	db $07
	db $76
	db $73
	db $74
	db $73
	db $71
	db $83
	db $5E
	db $5F
	db $84
	db $A6
	db $51
	db $52
	db $A7
	db $83
	db $99
	db $9A
	db $84
	db $18
	db $00
	db $00
	db $19
	db $38
	db $00
	db $01
	db $37
	db $13
	db $AD
	db $14
	db $13
	db $91
	db $AA
	db $00
	db $8D
	db $91
	db $8E
	db $AA
	db $92
	db $8A
	db $00
	db $8E
	db $92
	db $91
	db $0C
	db $00
	db $96
	db $91
	db $0D
	db $29
	db $00
	db $8C
	db $8F
	db $8F
	db $90
	db $8E
	db $8E
	db $8E
	db $8E
	db $00
	db $4D
	db $4C
	db $00
	db $93
	db $52
	db $51
	db $94
	db $95
	db $97
	db $97
	db $92
	db $00
	db $95
	db $95
	db $92
	db $8F
	db $8F
	db $8F
	db $8D
	db $91
	db $3A
	db $00
	db $92
	db $91
	db $96
	db $00
	db $92
	db $91
	db $00
	db $00
	db $92
	db $91
	db $65
	db $00
	db $8D
	db $91
	db $8E
	db $65
	db $92
	db $91
	db $00
	db $8E
	db $92
	db $75
	db $79
	db $77
	db $7A
	db $75
	db $79
	db $77
	db $79
	db $70
	db $73
	db $73
	db $73
	db $72
	db $6E
	db $6E
	db $6F
	db $00
	db $35
	db $35
	db $71
	db $77
	db $33
	db $33
	db $76
	db $83
	db $00
	db $00
	db $88
	db $83
	db $9C
	db $85
	db $00
	db $83
	db $00
	db $00
	db $86
	db $80
	db $7B
	db $7C
	db $7C
	db $00
	db $34
	db $35
	db $34
	db $85
	db $00
	db $33
	db $86
	db $80
	db $80
	db $7C
	db $7D
	db $00
	db $00
	db $4C
	db $84
	db $85
	db $00
	db $51
	db $7D
	db $75
	db $AA
	db $00
	db $71
	db $75
	db $72
	db $AA
	db $76
	db $6E
	db $00
	db $72
	db $76
	db $7C
	db $80
	db $7C
	db $80
	db $83
	db $4C
	db $01
	db $00
	db $83
	db $51
	db $80
	db $86
	db $80
	db $7D
	db $7C
	db $80
	db $3A
	db $00
	db $00
	db $12
	db $7F
	db $00
	db $00
	db $7E
	db $87
	db $80
	db $80
	db $88
	db $00
	db $00
	db $00
	db $00
	db $85
	db $00
	db $00
	db $86
	db $7D
	db $80
	db $80
	db $7D
	db $00
	db $9C
	db $83
	db $84
	db $85
	db $00
	db $33
	db $84
	db $91
	db $00
	db $00
	db $92
	db $8C
	db $00
	db $8E
	db $8E
	db $91
	db $00
	db $8E
	db $8B
	db $8A
	db $8E
	db $8A
	db $8B
	db $8E
	db $95
	db $34
	db $8D
	db $8A
	db $95
	db $00
	db $92
	db $91
	db $68
	db $51
	db $92
	db $8C
	db $52
	db $67
	db $92
	db $91
	db $52
	db $51
	db $8D
	db $8A
	db $8E
	db $8A
	db $8E
	db $8C
	db $00
	db $34
	db $30
	db $8C
	db $AE
	db $AE
	db $89
	db $95
	db $00
	db $00
	db $92
	db $00
	db $00
	db $3A
	db $92
	db $91
	db $00
	db $96
	db $92
	db $8C
	db $00
	db $AA
	db $92
	db $91
	db $AA
	db $8E
	db $92
	db $91
	db $8E
	db $00
	db $8B
	db $6E
	db $72
	db $6E
	db $72
	db $70
	db $AA
	db $AA
	db $00
	db $6D
	db $6D
	db $6D
	db $A0
	db $79
	db $00
	db $00
	db $76
	db $00
	db $3B
	db $3B
	db $76
	db $77
	db $79
	db $79
	db $76
	db $83
	db $AA
	db $00
	db $88
	db $83
	db $80
	db $AA
	db $00
	db $83
	db $00
	db $80
	db $86
	db $87
	db $00
	db $85
	db $88
	db $88
	db $86
	db $00
	db $86
	db $85
	db $00
	db $88
	db $86
	db $87
	db $00
	db $51
	db $84
	db $85
	db $6B
	db $51
	db $7F
	db $85
	db $87
	db $51
	db $84
	db $70
	db $00
	db $00
	db $76
	db $75
	db $7A
	db $79
	db $71
	db $70
	db $7A
	db $79
	db $76
	db $A6
	db $51
	db $00
	db $88
	db $A6
	db $51
	db $AA
	db $00
	db $83
	db $51
	db $80
	db $86
	db $7D
	db $00
	db $00
	db $7C
	db $84
	db $3A
	db $12
	db $83
	db $7F
	db $88
	db $87
	db $7E
	db $87
	db $00
	db $00
	db $88
	db $00
	db $00
	db $00
	db $00
	db $85
	db $00
	db $00
	db $86
	db $87
	db $55
	db $55
	db $84
	db $00
	db $35
	db $35
	db $84
	db $85
	db $87
	db $87
	db $84
	db $91
	db $94
	db $93
	db $92
	db $91
	db $97
	db $C2
	db $92
	db $91
	db $95
	db $96
	db $92
	db $91
	db $00
	db $65
	db $96
	db $91
	db $97
	db $8E
	db $00
	db $89
	db $8C
	db $8F
	db $90
	db $95
	db $52
	db $51
	db $8E
	db $00
	db $68
	db $67
	db $00
	db $A3
	db $89
	db $89
	db $8F
	db $8E
	db $8B
	db $8A
	db $8E
	db $00
	db $96
	db $4F
	db $00
	db $8F
	db $8D
	db $89
	db $89
	db $95
	db $12
	db $3A
	db $92
	db $00
	db $95
	db $96
	db $92
	db $8C
	db $AE
	db $AE
	db $8D
	db $91
	db $00
	db $C2
	db $92
	db $91
	db $C2
	db $96
	db $92
	db $91
	db $96
	db $00
	db $92
	db $6E
	db $72
	db $72
	db $72
	db $75
	db $A9
	db $12
	db $A9
	db $75
	db $00
	db $79
	db $78
	db $79
	db $78
	db $00
	db $76
	db $00
	db $00
	db $2B
	db $76
	db $AC
	db $AB
	db $6D
	db $6D
	db $83
	db $00
	db $00
	db $88
	db $83
	db $AA
	db $00
	db $30
	db $7B
	db $7B
	db $9C
	db $7B
	db $87
	db $00
	db $00
	db $88
	db $00
	db $88
	db $87
	db $00
	db $7E
	db $AF
	db $AF
	db $7F
	db $87
	db $00
	db $51
	db $7F
	db $00
	db $00
	db $5E
	db $84
	db $7B
	db $7B
	db $B0
	db $7F
	db $6E
	db $00
	db $65
	db $76
	db $75
	db $6B
	db $72
	db $71
	db $75
	db $72
	db $00
	db $76
	db $A6
	db $67
	db $00
	db $88
	db $83
	db $5E
	db $2A
	db $85
	db $7C
	db $80
	db $00
	db $86
	db $7D
	db $00
	db $00
	db $7C
	db $B1
	db $00
	db $00
	db $34
	db $85
	db $00
	db $00
	db $86
	db $87
	db $00
	db $00
	db $88
	db $00
	db $0D
	db $0C
	db $00
	db $85
	db $00
	db $00
	db $86
	db $87
	db $00
	db $12
	db $84
	db $00
	db $9C
	db $83
	db $84
	db $85
	db $00
	db $00
	db $84
	db $91
	db $55
	db $94
	db $92
	db $91
	db $35
	db $B1
	db $92
	db $91
	db $95
	db $00
	db $92
	db $B2
	db $B6
	db $B7
	db $B3
	db $B9
	db $6B
	db $BE
	db $BA
	db $B9
	db $BF
	db $00
	db $BA
	db $B2
	db $B6
	db $B6
	db $B6
	db $B9
	db $65
	db $BE
	db $00
	db $B9
	db $08
	db $00
	db $C2
	db $B7
	db $B6
	db $B6
	db $B3
	db $00
	db $12
	db $3A
	db $BA
	db $C1
	db $BF
	db $C0
	db $BA
	db $B2
	db $B6
	db $C5
	db $B7
	db $B9
	db $BD
	db $35
	db $00
	db $B9
	db $00
	db $33
	db $BE
	db $BF
	db $BD
	db $BE
	db $BC
	db $00
	db $BE
	db $00
	db $BA
	db $C1
	db $00
	db $C0
	db $BA
	db $CF
	db $00
	db $00
	db $D6
	db $C7
	db $00
	db $D7
	db $00
	db $CF
	db $00
	db $D5
	db $D4
	db $CB
	db $CC
	db $C7
	db $CC
	db $00
	db $12
	db $34
	db $00
	db $D3
	db $D5
	db $00
	db $D4
	db $CC
	db $CC
	db $C7
	db $CB
	db $00
	db $97
	db $33
	db $D3
	db $CD
	db $C9
	db $CD
	db $CE
	db $CB
	db $C8
	db $CC
	db $CB
	db $00
	db $97
	db $D8
	db $00
	db $D3
	db $D5
	db $D6
	db $D4
	db $CB
	db $CC
	db $C8
	db $C7
	db $00
	db $D3
	db $DA
	db $33
	db $CE
	db $D9
	db $C6
	db $C9
	db $D5
	db $12
	db $D8
	db $D2
	db $00
	db $D5
	db $D6
	db $D2
	db $CD
	db $CE
	db $D9
	db $CA
	db $83
	db $D7
	db $65
	db $88
	db $83
	db $83
	db $80
	db $00
	db $7E
	db $82
	db $81
	db $82
	db $87
	db $86
	db $85
	db $88
	db $86
	db $00
	db $34
	db $00
	db $81
	db $82
	db $81
	db $82
	db $87
	db $30
	db $AA
	db $88
	db $00
	db $87
	db $80
	db $00
	db $81
	db $82
	db $9C
	db $82
	db $87
	db $12
	db $00
	db $84
	db $00
	db $4C
	db $2B
	db $7F
	db $81
	db $99
	db $7B
	db $7B
	db $D1
	db $12
	db $00
	db $D0
	db $CF
	db $1E
	db $3A
	db $D2
	db $D1
	db $00
	db $1F
	db $D0
	db $BB
	db $00
	db $D7
	db $C0
	db $B9
	db $26
	db $35
	db $0C
	db $B9
	db $00
	db $BF
	db $C2
	db $BF
	db $00
	db $97
	db $BC
	db $2A
	db $77
	db $BF
	db $BA
	db $B8
	db $B8
	db $B8
	db $B5
	db $BB
	db $00
	db $00
	db $BC
	db $B9
	db $DB
	db $2B
	db $BA
	db $B9
	db $DC
	db $CC
	db $BA
	db $BB
	db $D7
	db $3A
	db $BC
	db $B9
	db $35
	db $C0
	db $BA
	db $B9
	db $BF
	db $00
	db $BA
	db $BB
	db $30
	db $30
	db $BC
	db $B9
	db $BF
	db $BF
	db $BA
	db $B4
	db $DD
	db $DD
	db $B5
	db $D1
	db $D7
	db $C2
	db $D6
	db $CF
	db $D5
	db $D6
	db $00
	db $C9
	db $D9
	db $CD
	db $CE
	db $D5
	db $00
	db $62
	db $D2
	db $00
	db $12
	db $51
	db $CA
	db $CE
	db $C9
	db $DE
	db $CA
	db $C7
	db $CC
	db $C7
	db $CB
	db $C9
	db $00
	db $33
	db $30
	db $C9
	db $E2
	db $E1
	db $E0
	db $D5
	db $D4
	db $D4
	db $D2
	db $D6
	db $00
	db $D3
	db $D2
	db $D3
	db $D5
	db $00
	db $C8
	db $C7
	db $C7
	db $C8
	db $CC
	db $35
	db $34
	db $65
	db $00
	db $CF
	db $00
	db $CB
	db $D4
	db $CB
	db $CC
	db $CB
	db $CC
	db $00
	db $12
	db $3A
	db $00
	db $D3
	db $D5
	db $D6
	db $D4
	db $CC
	db $C7
	db $CC
	db $C8
	db $00
	db $33
	db $2B
	db $D2
	db $CD
	db $D9
	db $CA
	db $CA
	db $C7
	db $C8
	db $CB
	db $CC
	db $D1
	db $30
	db $D8
	db $00
	db $D1
	db $D5
	db $D6
	db $D4
	db $C8
	db $CB
	db $CC
	db $C7
	db $00
	db $E3
	db $E4
	db $33
	db $D3
	db $1E
	db $1F
	db $D4
	db $CC
	db $C8
	db $C7
	db $C8
	db $D4
	db $D2
	db $33
	db $D2
	db $D3
	db $E5
	db $D6
	db $D0
	db $D1
	db $12
	db $3A
	db $D2
	db $35
	db $BB
	db $BC
	db $D0
	db $CF
	db $1E
	db $1F
	db $D2
	db $BB
	db $00
	db $3A
	db $C0
	db $B9
	db $12
	db $C0
	db $00
	db $B9
	db $BF
	db $00
	db $E4
	db $B6
	db $B6
	db $B7
	db $B3
	db $00
	db $00
	db $00
	db $BA
	db $E6
	db $DD
	db $DD
	db $B5
	db $BB
	db $FD
	db $00
	db $BC
	db $B9
	db $E9
	db $E9
	db $BA
	db $FE
	db $E7
	db $E8
	db $FF
	db $BB
	db $97
	db $D8
	db $C4
	db $B9
	db $BF
	db $C0
	db $00
	db $EA
	db $EC
	db $EB
	db $B8
	db $B6
	db $B6
	db $B6
	db $B3
	db $00
	db $00
	db $00
	db $BA
	db $C1
	db $00
	db $00
	db $BA
	db $C7
	db $CB
	db $CC
	db $CB
	db $CF
	db $D7
	db $D8
	db $00
	db $D1
	db $1E
	db $1F
	db $D4
	db $CB
	db $CC
	db $CB
	db $CC
	db $00
	db $B2
	db $B3
	db $00
	db $CD
	db $ED
	db $EE
	db $CD
	db $C8
	db $C7
	db $CC
	db $C8
	db $DA
	db $4D
	db $98
	db $CA
	db $CA
	db $DF
	db $CF
	db $C6
	db $D1
	db $EF
	db $F0
	db $D6
	db $CF
	db $CB
	db $CC
	db $00
	db $C9
	db $CD
	db $CE
	db $CD
	db $D5
	db $00
	db $D4
	db $D0
	db $00
	db $D5
	db $00
	db $D2
	db $CD
	db $CD
	db $D9
	db $CA
	db $CF
	db $00
	db $3A
	db $D6
	db $C7
	db $CF
	db $D6
	db $00
	db $D1
	db $33
	db $00
	db $D4
	db $CC
	db $C8
	db $C8
	db $C7
	db $3A
	db $E5
	db $DA
	db $33
	db $CA
	db $E2
	db $CA
	db $CD
	db $D5
	db $00
	db $00
	db $D6
	db $00
	db $BC
	db $BB
	db $00
	db $D3
	db $1F
	db $1E
	db $D4
	db $D5
	db $F1
	db $00
	db $D0
	db $00
	db $D5
	db $F2
	db $D2
	db $CD
	db $CE
	db $CD
	db $CA
	db $D1
	db $00
	db $00
	db $D6
	db $CF
	db $F2
	db $F2
	db $00
	db $CF
	db $00
	db $00
	db $D4
	db $D5
	db $00
	db $00
	db $D0
	db $00
	db $00
	db $00
	db $D2
	db $D3
	db $00
	db $00
	db $D0
	db $BB
	db $00
	db $00
	db $C4
	db $B9
	db $F3
	db $F2
	db $00
	db $B9
	db $BF
	db $00
	db $C2
	db $B7
	db $B6
	db $C5
	db $B3
	db $00
	db $BD
	db $34
	db $BA
	db $C1
	db $00
	db $00
	db $BA
	db $B2
	db $B6
	db $C5
	db $B7
	db $B9
	db $BD
	db $33
	db $00
	db $B4
	db $F4
	db $F4
	db $E6
	db $B6
	db $B7
	db $B6
	db $B7
	db $00
	db $D8
	db $F1
	db $00
	db $F5
	db $1F
	db $1E
	db $F6
	db $C3
	db $00
	db $00
	db $BC
	db $00
	db $00
	db $00
	db $BA
	db $F7
	db $DD
	db $DD
	db $B5
	db $D1
	db $12
	db $3A
	db $D6
	db $CF
	db $95
	db $96
	db $00
	db $F8
	db $B8
	db $B8
	db $B8
	db $CB
	db $C8
	db $CC
	db $CB
	db $3A
	db $00
	db $00
	db $12
	db $B5
	db $EC
	db $EC
	db $B4
	db $C8
	db $CB
	db $D5
	db $C7
	db $D8
	db $00
	db $00
	db $F1
	db $B5
	db $DD
	db $EC
	db $B4
	db $CC
	db $C8
	db $C7
	db $C7
	db $00
	db $C2
	db $D1
	db $34
	db $B8
	db $FA
	db $F8
	db $B8
	db $C8
	db $CC
	db $CB
	db $C7
	db $E5
	db $00
	db $00
	db $33
	db $B8
	db $FB
	db $EB
	db $B8
	db $D5
	db $D4
	db $D3
	db $D0
	db $00
	db $00
	db $00
	db $D2
	db $B8
	db $F4
	db $F4
	db $F9
	db $C7
	db $CB
	db $CC
	db $C7
	db $35
	db $D3
	db $00
	db $33
	db $F8
	db $F4
	db $F4
	db $FC
	db $D5
	db $00
	db $00
	db $D0
	db $00
	db $00
	db $00
	db $D2
	db $B8
	db $EB
	db $F4
	db $E8
	db $C7
	db $CB
	db $C8
	db $CC
	db $C9
	db $00
	db $B1
	db $00
	db $F8
	db $EC
	db $EC
	db $FC
	db $D5
	db $00
	db $3A
	db $D2
	db $00
	db $F3
	db $D6
	db $D0
	db $B8
	db $B4
	db $EB
	db $F9
	db $D1
	db $00
	db $00
	db $D0
	db $35
	db $00
	db $D8
	db $D2
	db $B4
	db $FC
	db $FA
	db $B5
	db $B9
	db $F1
	db $F1
	db $BA
	db $B9
	db $D5
	db $D5
	db $BA
	db $B4
	db $DD
	db $DD
	db $B5
	db $BB
	db $00
	db $00
	db $C4
	db $B9
	db $00
	db $00
	db $00
	db $B4
	db $EB
	db $FC
	db $B8
	db $B6
	db $B6
	db $B6
	db $B6
	db $3A
	db $00
	db $00
	db $12
	db $E8
	db $EC
	db $EC
	db $E7
	db $BF
	db $00
	db $00
	db $C0
	db $00
	db $B2
	db $B3
	db $00
	db $B8
	db $E7
	db $E8
	db $B8
	db $B6
	db $C5
	db $C5
	db $B3
	db $00
	db $33
	db $33
	db $BA
	db $E6
	db $DD
	db $B8
	db $B5
	db $07
	db $04
	db $36
	db $36
	db $36
	db $36
	db $04
	db $36
	db $36
	db $04
	db $36
	db $04
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $18
	db $0C
	db $76
	db $FB
	db $00
	db $1E
	db $78
	db $00
	db $06
	db $06
	db $0E
	db $0C
	db $CF
	db $7E
	db $06
	db $03
	db $0F
	db $3E
	db $60
	db $FF
	db $06
	db $F8
	db $1E
	db $00
	db $00
	db $1C
	db $3E
	db $33
	db $1B
	db $03
	db $1E
	db $7C
	db $00
	db $00
	db $07
	db $CE
	db $FC
	db $3C
	db $0C
	db $0C
	db $70
	db $FE
	db $00
	db $BC
	db $CF
	db $70
	db $1C
	db $06
	db $F0
	db $FF
	db $6F
	db $73
	db $33
	db $71
	db $E0
	db $C0
	db $E0
	db $C0
	db $00
	db $0F
	db $FF
	db $F8
	db $00
	db $00
	db $0F
	db $07
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $81
	db $C3
	db $C7
	db $C6
	db $FE
	db $7E
	db $06
	db $06
	db $3F
	db $7E
	db $00
	db $BC
	db $CF
	db $60
	db $30
	db $10
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $04
	db $36
	db $36
	db $04
	db $36
	db $36
	db $04
	db $36
	db $04
	db $36
	db $36
	db $04
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $92
	db $92
	db $D6
	db $D6
	db $6C
	db $6C
	db $28
	db $06
	db $07
	db $03
	db $31
	db $79
	db $63
	db $7F
	db $3E
	db $00
	db $00
	db $80
	db $C0
	db $E7
	db $7C
	db $78
	db $30
	db $1F
	db $3C
	db $70
	db $CF
	db $00
	db $F8
	db $3F
	db $00
	db $10
	db $10
	db $18
	db $D8
	db $F8
	db $7C
	db $0F
	db $03
	db $00
	db $F8
	db $3E
	db $00
	db $E7
	db $1E
	db $70
	db $00
	db $30
	db $71
	db $F3
	db $BB
	db $79
	db $7C
	db $EF
	db $C3
	db $00
	db $E0
	db $F0
	db $77
	db $77
	db $70
	db $E0
	db $C0
	db $3C
	db $FA
	db $02
	db $36
	db $66
	db $C6
	db $04
	db $00
	db $C1
	db $F3
	db $3B
	db $1F
	db $0F
	db $06
	db $06
	db $06
	db $C0
	db $80
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3C
	db $5F
	db $40
	db $6C
	db $66
	db $30
	db $30
	db $10
	db $04
	db $04
	db $04
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $04
	db $36
	db $04
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $0C
	db $7F
	db $00
	db $3B
	db $76
	db $0C
	db $38
	db $00
	db $0E
	db $3F
	db $F3
	db $03
	db $07
	db $06
	db $06
	db $06
	db $44
	db $66
	db $32
	db $1A
	db $01
	db $1E
	db $7C
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $C0
	db $60
	db $60
	db $00
	db $07
	db $1F
	db $FC
	db $EE
	db $06
	db $36
	db $66
	db $0E
	db $FF
	db $FB
	db $61
	db $30
	db $30
	db $10
	db $10
	db $1C
	db $3B
	db $F7
	db $C6
	db $47
	db $63
	db $30
	db $18
	db $E0
	db $F8
	db $3C
	db $CF
	db $C3
	db $80
	db $00
	db $00
	db $7E
	db $3C
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $28
	db $6C
	db $D6
	db $DA
	db $9A
	db $98
	db $08
	db $08
	db $03
	db $08
	db $0D
	db $0D
	db $0C
	db $04
	db $04
	db $00
	db $E0
	db $F0
	db $00
	db $80
	db $C0
	db $60
	db $00
	db $00
	db $36
	db $04
	db $04
	db $04
	db $36
	db $04
	db $36
	db $36
	db $04
	db $36
	db $36
	db $04
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $11
	db $13
	db $1B
	db $6B
	db $32
	db $18
	db $07
	db $0F
	db $0C
	db $06
	db $00
	db $07
	db $1F
	db $38
	db $C0
	db $E0
	db $60
	db $60
	db $C0
	db $C7
	db $FC
	db $7F
	db $40
	db $CE
	db $DC
	db $B0
	db $40
	db $38
	db $FE
	db $00
	db $06
	db $03
	db $07
	db $DF
	db $FE
	db $73
	db $07
	db $04
	db $30
	db $70
	db $E0
	db $FF
	db $3C
	db $80
	db $C0
	db $40
	db $00
	db $3E
	db $F0
	db $0E
	db $E0
	db $7E
	db $1C
	db $00
	db $C0
	db $E0
	db $78
	db $3F
	db $3F
	db $30
	db $B0
	db $F0
	db $3A
	db $66
	db $1A
	db $12
	db $02
	db $00
	db $00
	db $00
	db $1F
	db $4F
	db $50
	db $6C
	db $66
	db $63
	db $20
	db $20
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $70
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $BF
	db $BF
	db $BF
	db $BF
	db $BF
	db $BF
	db $BF
	db $0F
	db $01
	db $01
	db $01
	db $01
	db $7D
	db $7D
	db $7D
	db $7D
	db $70
	db $70
	db $70
	db $70
	db $70
	db $70
	db $00
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $EF
	db $EF
	db $EF
	db $EF
	db $EF
	db $EF
	db $EF
	db $E0
	db $7D
	db $7D
	db $7D
	db $7D
	db $7C
	db $7C
	db $7C
	db $7C
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $00
	db $00
	db $E1
	db $E1
	db $E1
	db $E1
	db $E1
	db $E1
	db $01
	db $01
	db $7C
	db $7D
	db $7D
	db $7D
	db $7D
	db $7D
	db $7D
	db $7D
	db $07
	db $77
	db $70
	db $70
	db $70
	db $70
	db $70
	db $70
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $17
	db $17
	db $17
	db $17
	db $17
	db $17
	db $17
	db $01
	db $E1
	db $E1
	db $E1
	db $E1
	db $E1
	db $E1
	db $E1
	db $7C
	db $7C
	db $70
	db $75
	db $75
	db $75
	db $75
	db $75
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $01
	db $5D
	db $5D
	db $5D
	db $5D
	db $5C
	db $5C
	db $5C
	db $80
	db $AF
	db $AF
	db $AF
	db $AF
	db $2F
	db $2F
	db $2F
	db $05
	db $E5
	db $E5
	db $E5
	db $E4
	db $E5
	db $E5
	db $E5
	db $FF
	db $FF
	db $FF
	db $FF
	db $07
	db $77
	db $77
	db $77
	db $5C
	db $5C
	db $5C
	db $5C
	db $00
	db $00
	db $00
	db $00
	db $2F
	db $2F
	db $2F
	db $2F
	db $2F
	db $2F
	db $2F
	db $00
	db $E5
	db $E5
	db $E5
	db $E1
	db $E1
	db $E1
	db $E1
	db $01
	db $77
	db $77
	db $77
	db $70
	db $70
	db $70
	db $70
	db $70
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $70
	db $F0
	db $60
	db $00
	db $00
	db $00
	db $01
	db $01
	db $BD
	db $BD
	db $BD
	db $BD
	db $BD
	db $3D
	db $7D
	db $7D
	db $7D
	db $7D
	db $7D
	db $7C
	db $7C
	db $00
	db $70
	db $70
	db $70
	db $00
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $BD
	db $BD
	db $BD
	db $BD
	db $BD
	db $BD
	db $BE
	db $BE
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $3E
	db $BE
	db $BE
	db $00
	db $00
	db $00
	db $00
	db $00
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $B0
	db $F0
	db $50
	db $00
	db $00
	db $00
	db $00
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $01
	db $01
	db $BD
	db $BD
	db $BD
	db $BD
	db $BD
	db $3D
	db $70
	db $70
	db $70
	db $70
	db $70
	db $70
	db $70
	db $00
	db $5F
	db $5E
	db $5E
	db $5E
	db $5E
	db $5E
	db $5E
	db $5E
	db $2F
	db $03
	db $BB
	db $BB
	db $BB
	db $BB
	db $BB
	db $B8
	db $BD
	db $BD
	db $BD
	db $BC
	db $BC
	db $BC
	db $BC
	db $3C
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $5F
	db $B8
	db $B8
	db $B8
	db $B8
	db $B8
	db $B8
	db $B8
	db $B8
	db $5F
	db $5F
	db $5F
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
ItemImgData:	;32 bytes per item - 16 * 16 img
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
	db $0F
	db $1F
	db $1F
	db $1F
	db $0E
	db $36
	db $00
	db $00
	db $01
	db $1D
	db $3E
	db $3F
	db $3F
	db $3C
	db $70
	db $E0
	db $C0
	db $80
	db $00
	db $03
	db $07
	db $07
	db $03
	db $07
	db $07
	db $87
	db $81
	db $06
	db $CE
	db $EC
	db $C0
	db $E0
	db $E0
	db $E1
	db $81
	db $60
	db $73
	db $37
	db $5C
	db $18
	db $39
	db $35
	db $73
	db $67
	db $87
	db $E3
	db $2E
	db $1C
	db $B8
	db $B4
	db $C6
	db $E6
	db $E1
	db $C7
	db $00
	db $00
	db $F0
	db $F8
	db $F8
	db $F8
	db $70
	db $6C
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $0E
	db $07
	db $03
	db $01
	db $00
	db $C0
	db $E0
	db $E0
	db $00
	db $00
	db $80
	db $B8
	db $7C
	db $FC
	db $FC
	db $3C
	db $1B
	db $03
	db $01
	db $00
	db $01
	db $77
	db $F6
	db $F8
	db $03
	db $8D
	db $DD
	db $E0
	db $70
	db $B6
	db $0F
	db $3F
	db $F8
	db $F6
	db $77
	db $03
	db $00
	db $01
	db $03
	db $1B
	db $3F
	db $0F
	db $96
	db $E0
	db $F0
	db $3D
	db $8D
	db $03
	db $E0
	db $E0
	db $C3
	db $0F
	db $1F
	db $1B
	db $37
	db $37
	db $07
	db $07
	db $C3
	db $F0
	db $F8
	db $F8
	db $FC
	db $FC
	db $33
	db $33
	db $19
	db $1C
	db $0F
	db $C3
	db $E0
	db $E0
	db $FC
	db $FC
	db $F8
	db $78
	db $F0
	db $C3
	db $07
	db $07
	db $C0
	db $B1
	db $BC
	db $1F
	db $07
	db $69
	db $F0
	db $FC
	db $D8
	db $C0
	db $00
	db $00
	db $C0
	db $EE
	db $6F
	db $1F
	db $FC
	db $F0
	db $6C
	db $0E
	db $07
	db $BB
	db $B1
	db $C0
	db $1F
	db $6F
	db $EE
	db $C0
	db $00
	db $80
	db $C0
	db $D8
	db $3C
	db $3F
	db $3F
	db $3E
	db $1D
	db $01
	db $00
	db $00
	db $07
	db $07
	db $03
	db $00
	db $80
	db $C0
	db $E0
	db $70
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $37
	db $0F
	db $1F
	db $1F
	db $1F
	db $0F
	db $00
	db $00
	db $E3
	db $87
	db $67
	db $63
	db $2D
	db $1D
	db $38
	db $74
	db $C7
	db $E1
	db $E6
	db $CE
	db $AC
	db $9C
	db $18
	db $3A
	db $EC
	db $C6
	db $06
	db $81
	db $87
	db $07
	db $07
	db $03
	db $37
	db $73
	db $60
	db $81
	db $E1
	db $E0
	db $E0
	db $C0
	db $E0
	db $E0
	db $C0
	db $00
	db $01
	db $03
	db $07
	db $0E
	db $3C
	db $FC
	db $FC
	db $7C
	db $B8
	db $80
	db $00
	db $00
	db $6C
	db $70
	db $F8
	db $F8
	db $F8
	db $F0
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
	db $7F
	db $FF
	db $D8
	db $FB
	db $FB
	db $FB
	db $FB
	db $F8
	db $FC
	db $FE
	db $36
	db $BE
	db $9E
	db $9E
	db $9E
	db $1E
	db $DF
	db $FF
	db $49
	db $36
	db $36
	db $36
	db $12
	db $36
	db $F6
	db $FE
	db $24
	db $D8
	db $D8
	db $D8
	db $48
	db $D8
	db $7F
	db $FF
	db $DC
	db $FE
	db $FE
	db $FC
	db $FC
	db $FC
	db $FC
	db $FE
	db $F6
	db $FE
	db $FE
	db $7E
	db $7E
	db $7E
	db $DF
	db $FF
	db $49
	db $36
	db $36
	db $36
	db $12
	db $36
	db $F6
	db $FE
	db $24
	db $D8
	db $D8
	db $D8
	db $48
	db $D8
	db $7F
	db $FF
	db $D8
	db $FF
	db $F8
	db $FB
	db $F8
	db $F8
	db $FC
	db $FE
	db $36
	db $BE
	db $3E
	db $FE
	db $3E
	db $3E
	db $DF
	db $FF
	db $49
	db $36
	db $36
	db $36
	db $12
	db $36
	db $F6
	db $FE
	db $24
	db $D8
	db $D8
	db $D8
	db $48
	db $D8
	db $7F
	db $FF
	db $DF
	db $FB
	db $FB
	db $F8
	db $FF
	db $FF
	db $FC
	db $FE
	db $76
	db $7E
	db $7E
	db $3E
	db $3E
	db $3E
	db $DF
	db $FF
	db $49
	db $36
	db $36
	db $36
	db $12
	db $36
	db $F6
	db $FE
	db $24
	db $D8
	db $D8
	db $D8
	db $48
	db $D8
	db $7F
	db $FF
	db $D8
	db $FB
	db $F0
	db $F3
	db $F3
	db $F0
	db $FC
	db $FE
	db $36
	db $BE
	db $1E
	db $9E
	db $9E
	db $1E
	db $DF
	db $FF
	db $49
	db $36
	db $36
	db $36
	db $12
	db $36
	db $F6
	db $FE
	db $24
	db $D8
	db $D8
	db $D8
	db $48
	db $D8
	db $7F
	db $FF
	db $D8
	db $F9
	db $FF
	db $FC
	db $FF
	db $FC
	db $FC
	db $FE
	db $36
	db $3E
	db $3E
	db $3E
	db $FE
	db $FE
	db $DC
	db $FF
	db $49
	db $36
	db $36
	db $36
	db $12
	db $36
	db $F6
	db $FE
	db $24
	db $D8
	db $D8
	db $D8
	db $48
	db $D8
	db $00
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $F8
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FC
	db $3E
	db $FE
	db $FC
	db $F8
	db $F8
	db $E2
	db $C1
	db $FF
	db $7F
	db $00
	db $7E
	db $3E
	db $1E
	db $0E
	db $06
	db $FE
	db $FC
	db $00
	db $00
	db $00
	db $C0
	db $E0
	db $F0
	db $F8
	db $DC
	db $CB
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $CB
	db $DC
	db $F8
	db $F0
	db $E0
	db $C0
	db $00
	db $00
	db $C1
	db $3E
	db $3E
	db $36
	db $00
	db $00
	db $00
	db $00
	db $03
	db $0F
	db $1C
	db $1E
	db $3F
	db $3E
	db $3B
	db $13
	db $80
	db $E0
	db $70
	db $F0
	db $F8
	db $F8
	db $B8
	db $90
	db $1F
	db $0E
	db $11
	db $3B
	db $73
	db $20
	db $41
	db $E3
	db $F0
	db $E0
	db $10
	db $B8
	db $9C
	db $08
	db $04
	db $8E
	db $07
	db $1F
	db $3C
	db $6E
	db $5F
	db $DE
	db $BB
	db $B3
	db $C0
	db $F0
	db $78
	db $FC
	db $FC
	db $FE
	db $BE
	db $9E
	db $BF
	db $00
	db $5E
	db $5E
	db $00
	db $BF
	db $80
	db $FF
	db $FE
	db $00
	db $F4
	db $F4
	db $00
	db $FE
	db $02
	db $FE
	db $00
	db $03
	db $06
	db $07
	db $17
	db $34
	db $76
	db $77
	db $00
	db $80
	db $C0
	db $C0
	db $D0
	db $58
	db $DC
	db $DC
	db $6E
	db $5B
	db $33
	db $7F
	db $FC
	db $DB
	db $F7
	db $60
	db $EC
	db $B4
	db $98
	db $FC
	db $7E
	db $B6
	db $DE
	db $0C
	db $F0
	db $F7
	db $07
	db $18
	db $0B
	db $06
	db $1B
	db $0C
	db $0F
	db $EF
	db $E0
	db $18
	db $D0
	db $60
	db $D8
	db $30
	db $70
	db $DF
	db $BD
	db $BD
	db $BD
	db $BF
	db $00
	db $5F
	db $0E
	db $FB
	db $1D
	db $5D
	db $1D
	db $FD
	db $00
	db $FA
	db $00
	db $08
	db $1C
	db $B6
	db $63
	db $01
	db $02
	db $05
	db $10
	db $38
	db $7C
	db $FE
	db $CE
	db $80
	db $80
	db $C0
	db $05
	db $05
	db $05
	db $05
	db $08
	db $13
	db $27
	db $4F
	db $C0
	db $C0
	db $C0
	db $C0
	db $20
	db $D0
	db $E8
	db $F4
	db $03
	db $13
	db $00
	db $32
	db $30
	db $04
	db $01
	db $11
	db $40
	db $00
	db $30
	db $30
	db $80
	db $10
	db $80
	db $10
	db $04
	db $05
	db $05
	db $05
	db $08
	db $13
	db $27
	db $4F
	db $40
	db $C0
	db $C0
	db $C0
	db $20
	db $D0
	db $E8
	db $F4
	db $38
	db $35
	db $0D
	db $13
	db $17
	db $20
	db $37
	db $17
	db $0E
	db $D6
	db $D8
	db $E4
	db $F8
	db $14
	db $FC
	db $F8
	db $20
	db $37
	db $17
	db $20
	db $37
	db $17
	db $20
	db $17
	db $14
	db $FC
	db $F8
	db $14
	db $FC
	db $F8
	db $14
	db $F8
	db $03
	db $07
	db $05
	db $05
	db $02
	db $01
	db $01
	db $01
	db $C0
	db $E0
	db $E0
	db $E0
	db $40
	db $80
	db $80
	db $80
	db $02
	db $05
	db $05
	db $38
	db $47
	db $7F
	db $60
	db $3F
	db $40
	db $E0
	db $E0
	db $1C
	db $E2
	db $FE
	db $06
	db $FC
	db $01
	db $03
	db $06
	db $0E
	db $1E
	db $3E
	db $7E
	db $FE
	db $00
	db $80
	db $C0
	db $60
	db $30
	db $18
	db $0C
	db $06
	db $FF
	db $00
	db $FF
	db $7F
	db $32
	db $33
	db $30
	db $3F
	db $FE
	db $00
	db $FE
	db $1C
	db $58
	db $D8
	db $18
	db $F8
	db $0F
	db $1B
	db $17
	db $17
	db $17
	db $17
	db $60
	db $7F
	db $E0
	db $B0
	db $F0
	db $F0
	db $F0
	db $F0
	db $1C
	db $FC
	db $00
	db $1B
	db $19
	db $19
	db $31
	db $31
	db $61
	db $21
	db $00
	db $B0
	db $98
	db $98
	db $98
	db $98
	db $98
	db $8C
	db $0F
	db $30
	db $5F
	db $55
	db $AA
	db $80
	db $80
	db $BF
	db $E0
	db $18
	db $FC
	db $54
	db $AA
	db $02
	db $02
	db $FE
	db $5F
	db $5F
	db $27
	db $18
	db $30
	db $30
	db $18
	db $38
	db $FC
	db $FC
	db $C8
	db $30
	db $18
	db $18
	db $30
	db $38
	db $00
	db $00
	db $24
	db $24
	db $FF
	db $24
	db $24
	db $00
	db $04
	db $24
	db $A4
	db $A4
	db $FF
	db $A4
	db $A4
	db $24
	db $01
	db $01
	db $01
	db $00
	db $03
	db $04
	db $09
	db $0B
	db $84
	db $80
	db $80
	db $00
	db $C0
	db $E0
	db $F0
	db $F0
	db $07
	db $1F
	db $3B
	db $37
	db $37
	db $37
	db $1B
	db $0D
	db $C0
	db $F0
	db $F8
	db $F8
	db $F8
	db $F8
	db $F0
	db $E0
	db $05
	db $05
	db $07
	db $00
	db $05
	db $03
	db $05
	db $02
	db $C0
	db $C0
	db $C0
	db $40
	db $C0
	db $80
	db $C0
	db $80
	db $FF
	db $80
	db $B4
	db $8A
	db $80
	db $FF
	db $FC
	db $FB
	db $FE
	db $FE
	db $FE
	db $FE
	db $F8
	db $F8
	db $7E
	db $BE
	db $DB
	db $FB
	db $FC
	db $FF
	db $FE
	db $FE
	db $FE
	db $FF
	db $BE
	db $BE
	db $7E
	db $FE
	db $FE
	db $FE
	db $FE
	db $FE
	db $07
	db $05
	db $07
	db $08
	db $19
	db $00
	db $33
	db $33
	db $80
	db $C0
	db $C0
	db $20
	db $D0
	db $08
	db $E8
	db $E8
	db $33
	db $33
	db $33
	db $33
	db $33
	db $20
	db $4F
	db $4F
	db $E8
	db $E8
	db $E8
	db $E8
	db $E8
	db $08
	db $F4
	db $F4
	db $00
	db $03
	db $06
	db $0C
	db $1D
	db $3B
	db $7B
	db $B3
	db $60
	db $BE
	db $6C
	db $F2
	db $B8
	db $3C
	db $7C
	db $44
	db $93
	db $63
	db $03
	db $03
	db $03
	db $03
	db $03
	db $01
	db $54
	db $4C
	db $44
	db $7C
	db $7C
	db $7C
	db $1C
	db $F8
	db $00
	db $43
	db $24
	db $18
	db $00
	db $00
	db $01
	db $03
	db $1C
	db $BE
	db $76
	db $E6
	db $D4
	db $D8
	db $EC
	db $8C
	db $07
	db $0E
	db $1C
	db $38
	db $70
	db $E0
	db $C0
	db $00
	db $0C
	db $38
	db $60
	db $C0
	db $C0
	db $CC
	db $FC
	db $78
	db $83
	db $4F
	db $30
	db $3F
	db $5F
	db $4E
	db $86
	db $99
	db $F0
	db $C0
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $A1
	db $C0
	db $80
	db $80
	db $00
	db $01
	db $01
	db $00
	db $80
	db $C0
	db $60
	db $50
	db $30
	db $B0
	db $F0
	db $E0
CollectableItemInfo:	;4 bytes per item
	db $DD	;bottom 5 bits: X, top 3 bits: ink colour
	db $00	;high bit used for room number bit 9, also used for UI slot num & y pos
	db $A8	;room number
	db $0F	;item number
	db $AD
	db $00
	db $96
	db $10
	db $B5
	db $80
	db $B4
	db $21
	db $61
	db $00
	db $EC
	db $0C
	db $73
	db $00
	db $10
	db $08
	db $B6
	db $80
	db $F8
	db $00
	db $65
	db $80
	db $28
	db $0B
	db $85
	db $00
	db $48
	db $09
	db $49
	db $80
	db $16
	db $0A
	db $D2
	db $00
	db $2A
	db $04
	db $C1
	db $80
	db $A0
	db $05
	db $48
	db $00
	db $0E
	db $0A
	db $C5
	db $80
	db $3C
	db $04
	db $E8
	db $80
	db $E2
	db $05
	db $88
	db $00
	db $56
	db $21
	db $D5
	db $00
	db $3E
	db $1E
	db $61
	db $00
	db $50
	db $1A
	db $7C
	db $00
	db $C2
	db $1A
	db $95
	db $00
	db $72
	db $0B
	db $65
	db $80
	db $74
	db $1B
	db $CC
	db $0A
	db $C7
	db $0C
	db $D4
	db $0A
	db $C7
	db $0E
	db $D4
	db $0A
	db $C7
	db $0E
	db $CC
	db $0A
	db $C7
	db $0E
	db $D5
	db $0A
	db $C7
	db $0E
	db $D5
	db $0A
	db $C7
	db $0E
	db $D5
	db $0A
	db $C7
	db $0E
	db $41
	db $0A
	db $C7
	db $0F
	db $61
	db $0A
	db $C7
	db $10
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $FF
	db $0A
	db $C7
	db $FF
	db $00
	db $0F
	db $40
	db $DF
	db $03
	db $20
	db $8D
	db $00
	db $AA
	db $48
	db $BA
	db $00
	db $02
	db $02
	db $07
	db $03
	db $00
	db $00
	db $08
	db $08
	db $02
	db $40
	db $1F
	db $48
	db $B1
	db $02
	db $50
	db $11
	db $00
	db $02
	db $48
	db $B7
	db $00
	db $02
	db $02
	db $06
	db $05
	db $00
	db $10
	db $02
	db $02
	db $01
	db $48
	db $87
	db $48
	db $B7
	db $02
	db $50
	db $8D
	db $00
	db $02
	db $48
	db $B7
	db $00
	db $02
	db $02
	db $04
	db $03
	db $01
	db $00
	db $04
	db $04
	db $01
	db $2E
	db $23
	db $48
	db $B1
	db $03
	db $40
	db $11
	db $00
	db $AA
	db $88
	db $B9
	db $00
	db $02
	db $02
	db $06
	db $02
	db $00
	db $09
	db $02
	db $01
	db $00
RoomNumbersOf???:
	db $BE
	db $01
	db $FC
	db $01
	db $C4
	db $81
	db $E2
	db $81
	db $E6
	db $81
	db $86
	db $01
	db $09
	db $53
	db $55
	db $43
ScreenStateData:
	db $00
	db $00
LockedDoorCoords:
	db $00	;X pos
	db $00	;Y pos
	db $00
data_9605:
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
PatrollingEnemyPtr:
	dw $9621
NoPatrollingEnemies:
	db $00
PatrollingEnemies:
	db $00	;X char coord
	db $00	;Y char coord
	db $00	;X char coord
	db $00	;Y char coord
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
ElectricHazardPtr:
	dw $963D
ElectricHazardUpdateCounter:
	db $00
ElecticHazardList:
	db $15	;xpos
	db $0C	;ypos
	db $00
	db $01	;timer?
	db $08
	db $00	;on
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
TravelTubeCounter:
	dw $601B
TransportTubeCoordTable:
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
	db $00
	db $00
	db $00
	db $00
	db $00
LastScreenItemPtr:
	dw $96D7
NoScreenItemPositions:
	db $06
CurrScreenItemPositions:
	db $0D
	db $0D
	db $05
	db $13
	db $01
	db $13
	db $0D
	db $13
	db $09
	db $13
	db $11
	db $13
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
	db $00
	db $00
	db $00
	db $00
data_96FA:
	dw $9702
	db $A0	;X Pos
	db $3F	;Y Pos
	db $0D	;Type
	db $88
	db $27
	db $05
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
SmallPlatformTypeData:
	db $51
	db $51
	db $51
	db $51
	db $00
	db $00
	db $00
	db $71
	db $00
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
	db $00
	db $20
	db $20
	db $20
	db $20
	db $00
	db $00
	db $62
	db $40
	db $40
	db $00
	db $40
	db $00
	db $C5
	db $C5
	db $C5
	db $D4
	db $04
	db $06
	db $00
	db $75
	db $95
	db $E5
	db $F5
	db $F5
	db $B1
	db $65
	db $10
	db $85
	db $00
	db $30
	db $30
	db $30
	db $00
	db $00
	db $C5
	db $9D
	db $40
	db $00
	db $40
	db $40
	db $40
	db $40
	db $40
	db $40
	db $10
	db $10
	db $10
	db $10
	db $20
	db $20
	db $20
	db $20
	db $10
	db $10
	db $10
	db $10
	db $10
	db $10
	db $40
	db $30
	db $00
	db $65
	db $40
	db $00
	db $00
	db $00
	db $BC
	db $BC
	db $10
	db $40
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
Big_PlatformData:
	db $14
	db $14
	db $14
	db $14
	db $29
	db $29
	db $14
	db $14
	db $35
	db $36
	db $36
	db $35
	db $14
	db $14
	db $35
	db $36
	db $35
	db $14
	db $36
	db $36
	db $14
	db $36
	db $35
	db $35
	db $14
	db $14
	db $14
	db $35
	db $14
	db $14
	db $36
	db $14
	db $14
	db $14
	db $34
	db $33
	db $33
	db $34
	db $29
	db $29
	db $33
	db $14
	db $32
	db $32
	db $14
	db $34
	db $32
	db $33
	db $14
	db $33
	db $14
	db $29
	db $32
	db $14
	db $29
	db $14
	db $32
	db $14
	db $29
	db $33
	db $34
	db $32
	db $29
	db $29
	db $34
	db $32
	db $32
	db $33
	db $31
	db $14
	db $14
	db $14
	db $14
	db $07
	db $14
	db $14
	db $0A
	db $09
	db $09
	db $0A
	db $0A
	db $09
	db $13
	db $12
	db $14
	db $14
	db $0E
	db $11
	db $11
	db $10
	db $29
	db $29
	db $10
	db $0E
	db $29
	db $29
	db $14
	db $09
	db $14
	db $0A
	db $0A
	db $14
	db $09
	db $14
	db $14
	db $0E
	db $0F
	db $0A
	db $0D
	db $14
	db $0A
	db $0E
	db $0F
	db $09
	db $29
	db $0B
	db $09
	db $10
	db $0C
	db $29
	db $14
	db $14
	db $14
	db $0F
	db $14
	db $14
	db $0E
	db $14
	db $14
	db $10
	db $14
	db $29
	db $11
	db $14
	db $29
	db $14
	db $09
	db $20
	db $13
	db $21
	db $10
	db $20
	db $29
	db $22
	db $14
	db $0B
	db $0F
	db $0C
	db $0C
	db $14
	db $13
	db $10
	db $14
	db $14
	db $14
	db $33
	db $33
	db $14
	db $32
	db $14
	db $14
	db $32
	db $14
	db $33
	db $14
	db $14
	db $32
	db $14
	db $20
	db $14
	db $37
	db $14
	db $24
	db $14
	db $07
	db $14
	db $0A
	db $13
	db $09
	db $2B
	db $12
	db $0A
	db $2C
	db $09
	db $28
	db $10
	db $14
	db $29
	db $09
	db $1B
	db $0A
	db $14
	db $14
	db $2E
	db $14
	db $14
	db $14
	db $19
	db $14
	db $14
	db $14
	db $1D
	db $0D
	db $0E
	db $14
	db $14
	db $14
	db $1E
	db $14
	db $1F
	db $14
	db $1C
	db $14
	db $1D
	db $14
	db $1C
	db $14
	db $04
	db $14
	db $08
	db $09
	db $2F
	db $09
	db $29
	db $12
	db $0A
	db $19
	db $09
	db $0E
	db $14
	db $0A
	db $14
	db $07
	db $14
	db $38
	db $14
	db $14
	db $08
	db $14
	db $07
	db $15
	db $16
	db $16
	db $15
	db $14
	db $18
	db $17
	db $16
	db $17
	db $14
	db $16
	db $18
	db $17
	db $16
	db $29
	db $15
	db $17
	db $18
	db $16
	db $29
	db $14
	db $14
	db $17
	db $18
	db $17
	db $17
	db $29
	db $29
	db $14
	db $16
	db $14
	db $15
	db $15
	db $14
	db $16
	db $14
	db $14
	db $18
	db $14
	db $29
	db $17
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $17
	db $14
	db $14
	db $18
	db $14
	db $15
	db $15
	db $29
	db $04
	db $10
	db $0D
	db $29
	db $01
	db $0E
	db $11
	db $01
	db $29
	db $14
	db $00
	db $14
	db $04
	db $00
	db $14
	db $04
	db $14
	db $27
	db $10
	db $2A
	db $29
	db $26
	db $25
	db $14
	db $14
	db $17
	db $20
	db $29
	db $22
	db $14
	db $00
	db $14
	db $00
	db $00
	db $14
	db $00
	db $14
	db $59
	db $2D
	db $59
	db $57
	db $14
	db $55
	db $17
	db $17
	db $14
	db $39
	db $14
	db $14
	db $00
	db $09
	db $06
	db $09
	db $00
	db $0A
	db $00
	db $0A
	db $03
	db $09
	db $00
	db $0A
	db $10
	db $0A
	db $03
	db $09
	db $09
	db $00
	db $0A
	db $05
	db $09
	db $00
	db $0A
	db $00
	db $0A
	db $02
	db $0A
	db $00
	db $09
	db $0E
	db $09
	db $02
	db $14
	db $01
	db $14
	db $00
	db $01
	db $14
	db $00
	db $14
	db $06
	db $0A
	db $0D
	db $09
	db $0A
	db $05
	db $09
	db $0E
	db $14
	db $04
	db $14
	db $14
	db $04
	db $14
	db $14
	db $14
	db $14
	db $0A
	db $14
	db $2F
	db $14
	db $3A
	db $14
	db $14
	db $11
	db $28
	db $29
	db $14
	db $14
	db $03
	db $14
	db $00
	db $02
	db $14
	db $00
	db $14
	db $17
	db $17
	db $01
	db $14
	db $0D
	db $10
	db $30
	db $14
	db $14
	db $1C
	db $14
	db $39
	db $0D
	db $11
	db $2E
	db $2E
	db $49
	db $49
	db $4A
	db $49
	db $14
	db $4A
	db $4B
	db $49
	db $49
	db $14
	db $4A
	db $4C
	db $4B
	db $49
	db $29
	db $4A
	db $4A
	db $4B
	db $49
	db $29
	db $14
	db $14
	db $4B
	db $4C
	db $49
	db $4B
	db $29
	db $29
	db $4C
	db $4B
	db $29
	db $29
	db $14
	db $4A
	db $14
	db $49
	db $49
	db $14
	db $4A
	db $14
	db $14
	db $49
	db $14
	db $29
	db $4A
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $4B
	db $14
	db $14
	db $4C
	db $14
	db $41
	db $42
	db $42
	db $41
	db $14
	db $41
	db $44
	db $42
	db $42
	db $14
	db $41
	db $43
	db $43
	db $42
	db $29
	db $41
	db $41
	db $44
	db $42
	db $29
	db $14
	db $14
	db $44
	db $43
	db $43
	db $41
	db $29
	db $29
	db $44
	db $43
	db $29
	db $14
	db $14
	db $42
	db $14
	db $41
	db $41
	db $14
	db $42
	db $14
	db $14
	db $41
	db $14
	db $29
	db $42
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $42
	db $14
	db $14
	db $43
	db $14
	db $45
	db $46
	db $46
	db $45
	db $14
	db $45
	db $47
	db $45
	db $46
	db $14
	db $45
	db $48
	db $47
	db $46
	db $29
	db $45
	db $46
	db $48
	db $45
	db $29
	db $14
	db $14
	db $48
	db $47
	db $47
	db $46
	db $29
	db $29
	db $46
	db $47
	db $29
	db $29
	db $14
	db $46
	db $14
	db $45
	db $45
	db $14
	db $46
	db $14
	db $14
	db $48
	db $14
	db $29
	db $47
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $48
	db $14
	db $14
	db $47
	db $14
	db $14
	db $3B
	db $14
	db $14
	db $27
	db $40
	db $2A
	db $29
	db $42
	db $42
	db $29
	db $01
	db $42
	db $41
	db $01
	db $29
	db $28
	db $42
	db $14
	db $29
	db $20
	db $43
	db $21
	db $29
	db $49
	db $20
	db $4A
	db $23
	db $14
	db $0D
	db $1A
	db $13
	db $0E
	db $14
	db $12
	db $1A
	db $49
	db $20
	db $29
	db $22
	db $1A
	db $49
	db $14
	db $29
	db $2C
	db $25
	db $0A
	db $14
	db $20
	db $45
	db $37
	db $29
	db $42
	db $28
	db $29
	db $14
	db $28
	db $42
	db $14
	db $29
	db $28
	db $42
	db $29
	db $41
	db $41
	db $28
	db $42
	db $29
	db $14
	db $46
	db $28
	db $45
	db $14
	db $4C
	db $14
	db $07
	db $2E
	db $2E
	db $14
	db $14
	db $4C
	db $49
	db $30
	db $30
	db $49
	db $20
	db $4A
	db $21
	db $20
	db $0A
	db $23
	db $12
	db $47
	db $46
	db $30
	db $30
	db $43
	db $42
	db $30
	db $30
	db $41
	db $42
	db $29
	db $03
	db $1E
	db $14
	db $1D
	db $14
	db $14
	db $4F
	db $4E
	db $4D
	db $4F
	db $14
	db $4E
	db $4D
	db $50
	db $50
	db $29
	db $3C
	db $50
	db $50
	db $3C
	db $29
	db $14
	db $14
	db $4E
	db $4D
	db $14
	db $14
	db $4D
	db $4E
	db $50
	db $50
	db $29
	db $29
	db $14
	db $3C
	db $14
	db $3C
	db $3C
	db $14
	db $3C
	db $14
	db $14
	db $3C
	db $14
	db $4F
	db $3C
	db $14
	db $4F
	db $14
	db $14
	db $51
	db $14
	db $29
	db $51
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $51
	db $14
	db $14
	db $51
	db $14
	db $14
	db $3C
	db $14
	db $29
	db $3C
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $4F
	db $14
	db $14
	db $4F
	db $14
	db $14
	db $1D
	db $4E
	db $4D
	db $3D
	db $3E
	db $3E
	db $3D
	db $14
	db $1D
	db $40
	db $3E
	db $3D
	db $14
	db $3E
	db $3F
	db $3F
	db $3E
	db $29
	db $3D
	db $3D
	db $3F
	db $3E
	db $29
	db $14
	db $14
	db $40
	db $3F
	db $14
	db $14
	db $3D
	db $3F
	db $3F
	db $3D
	db $29
	db $29
	db $40
	db $3E
	db $29
	db $29
	db $14
	db $3E
	db $14
	db $3D
	db $3D
	db $14
	db $3E
	db $14
	db $14
	db $1D
	db $14
	db $3E
	db $1D
	db $14
	db $3D
	db $14
	db $14
	db $40
	db $14
	db $29
	db $3F
	db $14
	db $29
	db $14
	db $14
	db $14
	db $14
	db $40
	db $14
	db $14
	db $3F
	db $14
	db $14
	db $53
	db $14
	db $14
	db $52
	db $14
	db $14
	db $14
	db $20
	db $3E
	db $21
	db $29
	db $2D
	db $5A
	db $58
	db $5A
	db $22
	db $3D
	db $29
	db $07
	db $14
	db $14
	db $20
	db $40
	db $50
	db $50
	db $52
	db $30
	db $3F
	db $3E
	db $29
	db $01
	db $3E
	db $40
	db $01
	db $29
	db $20
	db $3E
	db $22
	db $1D
	db $3F
	db $3D
	db $30
	db $52
	db $40
	db $3F
	db $30
	db $30
	db $14
	db $4F
	db $14
	db $29
	db $4F
	db $14
	db $29
	db $14
	db $14
	db $14
	db $1F
	db $14
	db $50
	db $20
	db $07
	db $21
	db $50
	db $50
	db $14
	db $4F
	db $50
	db $50
	db $4F
	db $14
	db $4D
	db $4E
	db $14
	db $14
	db $20
	db $50
	db $23
	db $3C
	db $50
	db $50
	db $14
	db $3A
	db $50
	db $50
	db $30
	db $30
	db $40
	db $3F
	db $14
	db $3C
	db $3D
	db $40
	db $3C
	db $14
	db $30
	db $07
	db $14
	db $14
	db $07
	db $30
	db $14
	db $14
	db $14
	db $52
	db $14
	db $14
	db $40
	db $3F
	db $52
	db $52
	db $14
	db $1D
	db $14
	db $3B
	db $50
	db $50
	db $52
	db $52
	db $14
	db $3C
	db $14
	db $07
	db $3C
	db $29
	db $07
	db $14
	db $50
	db $50
	db $29
	db $07
	db $50
	db $50
	db $29
	db $3E
	db $50
	db $50
	db $3D
	db $29
	db $50
	db $50
	db $24
	db $29
	db $50
	db $50
	db $54
	db $29
	db $50
	db $20
	db $14
	db $22
	db $56
	db $29
	db $14
	db $14
	db $50
	db $50
	db $52
	db $3C
	db $50
	db $50
	db $3C
	db $52
data_9C40:
	db $0A
data_9C41:
	db $28
	db $00
data_9C43:
	db $04
data_9C44:
	db $18
	db $00
data_9C46:
	db $04
function_9C47:
	JR label_9C4F	;[label_9C4F]
function_9C49:
	JP label_9D96	;[label_9D96]
function_9C4C:
	JP function_9FFC	;[function_9FFC]
label_9C4F:
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	XOR A
	CP H
	JR NZ,label_9C78	;[label_9C78]
	LD A,L
	CP $C7
	JP Z,label_9F78	;[label_9F78]
	CP $C6
	JR NZ,label_9C78	;[label_9C78]
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	LD (data_9C44),HL	;[data_9C44]
	LD A,$04
	LD (data_9C46),A	;[data_9C46]
	LD (data_9C40),A	;[data_9C40]
	LD HL,$959C	;[CollectableItemInfo + 180]
	LD B,$54
	XOR A
label_9C74:
	LD (HL),A
	INC HL
	DJNZ label_9C74	;[label_9C74]
label_9C78:
	LD HL,data_DD32	;[data_DD32]
	LD DE,$959C	;[CollectableItemInfo + 180]
	LD C,$04
label_9C80:
	PUSH BC
	LD BC,$000B	;[ERROR_1 + 3]
	ADD HL,BC
	LD B,$15
label_9C87:
	LD A,(DE)
	LD C,(HL)
	EX DE,HL
	LD (HL),C
	LD (DE),A
	EX DE,HL
	INC HL
	INC DE
	DJNZ label_9C87	;[label_9C87]
	POP BC
	DEC C
	JR NZ,label_9C80	;[label_9C80]
	LD DE,(data_9C44)	;[data_9C44]
	LD A,(data_9C46)	;[data_9C46]
	EX AF,AF'
	LD HL,(data_9C41)	;[data_9C41]
	LD (data_9C44),HL	;[data_9C44]
	LD A,(data_9C43)	;[data_9C43]
	LD (data_9C46),A	;[data_9C46]
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	LD (data_9C41),HL	;[data_9C41]
	LD B,H
	LD C,L
	LD HL,data_9C40	;[data_9C40]
	LD A,(HL)
	LD (HL),$B4
	CP $00
	JR Z,label_9CD8	;[label_9CD8]
	LD A,B
	CP D
	JR NZ,label_9CD8	;[label_9CD8]
	LD A,C
	CP E
	JR NZ,label_9CD8	;[label_9CD8]
	EX AF,AF'
	CP $03
	JR NZ,label_9CD0	;[label_9CD0]
	LD B,$04
	CALL GetEnemyPtr	;[GetEnemyPtr]
	CALL function_9DC2	;[function_9DC2]
label_9CD0:
	LD A,$04
	LD (data_9C43),A	;[data_9C43]
	JP label_9F49	;[label_9F49]
label_9CD8:
	LD A,$04
	LD (data_9C43),A	;[data_9C43]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD HL,(RandomNo1)	;[RandomNo1]
	LD A,H
	LD (data_9DB9),A	;[data_9DB9]
	RLC L
	JR NC,label_9D03	;[label_9D03]
	LD HL,RAMTOP	;[RAMTOP]
	LD (data_9DBA),HL	;[data_9DBA]
	LD (data_9DBC),HL	;[data_9DBC]
	LD B,$04
label_9CF6:
	PUSH BC
	CALL GetEnemyPtr	;[GetEnemyPtr]
	CALL function_9DC2	;[function_9DC2]
	POP BC
	DJNZ label_9CF6	;[label_9CF6]
	JP label_9F05	;[label_9F05]
label_9D03:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_9D09:
	SUB $0F
	JR NC,label_9D09	;[label_9D09]
	ADD A,$11
	LD (data_9DBE),A	;[data_9DBE]
	LD HL,$FFE0	;[SmallPlatform3Data + 4664]
	LD (data_9DBA),HL	;[data_9DBA]
	LD HL,RAMTOP	;[RAMTOP]
	LD (data_9DBC),HL	;[data_9DBC]
	LD HL,data_9DBE	;[data_9DBE]
	LD A,(data_DAC1)	;[data_DAC1]
	LD C,A
	RRC C
	JR NC,label_9D3E	;[label_9D3E]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_9D2F:
	SUB $05
	JR NC,label_9D2F	;[label_9D2F]
	ADD A,$07
	RRCA
	RRCA
	RRCA
	OR (HL)
	LD (HL),A
	XOR A
	LD (data_9DBA),A	;[data_9DBA]
label_9D3E:
	INC HL
	INC HL
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	LD (HL),A
	RRC C
	JR NC,label_9D53	;[label_9D53]
	LD A,(data_9DBC)	;[data_9DBC]
	AND $0F
	LD (data_9DBC),A	;[data_9DBC]
label_9D53:
	RRC C
	JR NC,label_9D5B	;[label_9D5B]
	LD A,(HL)
	AND $F0
	LD (HL),A
label_9D5B:
	RRC C
	JR NC,label_9D67	;[label_9D67]
	LD A,(data_9DBC)	;[data_9DBC]
	AND $F0
	LD (data_9DBC),A	;[data_9DBC]
label_9D67:
	INC HL
	LD A,(data_DAC1)	;[data_DAC1]
	LD (HL),A
	RRC C
	JR NC,label_9D74	;[label_9D74]
	XOR A
	LD (data_9DBD),A	;[data_9DBD]
label_9D74:
	LD B,$04
label_9D76:
	PUSH BC
	CALL GetEnemyPtr	;[GetEnemyPtr]
	CALL function_9DC2	;[function_9DC2]
	POP BC
	DJNZ label_9D76	;[label_9D76]
	RRC C
	JP NC,label_9F05	;[label_9F05]
	RRC C
	JP NC,label_9F05	;[label_9F05]
	LD HL,RAMTOP	;[RAMTOP]
	LD (data_9DBA),HL	;[data_9DBA]
	LD (data_9DBC),HL	;[data_9DBC]
	JP label_9F05	;[label_9F05]
label_9D96:
	LD (IX+$05),$00
	LD (IX+$06),$00
	LD HL,$DF40	;[ScanlineTable + 354]
	LD (IX+$07),L
	LD (IX+$08),H
	LD A,(RandomNo1)	;[RandomNo1]
	AND $E0
	LD HL,data_9DB9	;[data_9DB9]
	CP (HL)
	RET C
	LD (IX+$06),$0F
	CALL function_9DC2	;[function_9DC2]
	RET
data_9DB9:
	db $56
data_9DBA:
	db $00
	db $FF
data_9DBC:
	db $F0
data_9DBD:
	db $FF
data_9DBE:
	db $4F
data_9DBF:
	db $DC
data_9DC0:
	db $F0
data_9DC1:
	db $AC
function_9DC2:
	LD HL,data_9DBE	;[data_9DBE]
	LD DE,data_9DBA	;[data_9DBA]
	LD B,$04
label_9DCA:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	LD C,A
	LD A,(DE)
	INC DE
	AND C
	XOR (HL)
	LD (HL),A
	INC HL
	DJNZ label_9DCA	;[label_9DCA]
	LD A,(data_9DBE)	;[data_9DBE]
	AND $1F
	LD C,A
	LD A,(data_9DBA)	;[data_9DBA]
	AND $1F
	JR Z,label_9DF0	;[label_9DF0]
	LD A,(data_DAC1)	;[data_DAC1]
label_9DE9:
	SUB $0F
	JR NC,label_9DE9	;[label_9DE9]
	ADD A,$11
	LD C,A
label_9DF0:
	LD D,$00
	LD E,C
	LD HL,$00C0	;[TOKENS + 43]
	CALL HL_HLxDE	;[HL_HLxDE]
	LD DE,$B208	;[Enemy_SpriteData + 576]
	ADD HL,DE
	LD (IX+$0E),L
	LD (IX+$0F),H
	LD A,(data_9DBE)	;[data_9DBE]
	RLCA
	RLCA
	RLCA
	LD HL,data_9DBA	;[data_9DBA]
	BIT 7,(HL)
	JR Z,label_9E1C	;[label_9E1C]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_9E16:
	SUB $05
	JR NC,label_9E16	;[label_9E16]
	ADD A,$07
label_9E1C:
	AND $07
	LD (IX+$09),A
	LD A,(data_9DBF)	;[data_9DBF]
	LD (IX+$14),A
	LD A,(data_9DC0)	;[data_9DC0]
	RRCA
	RRCA
	RRCA
	RRCA
	AND $0F
label_9E30:
	SUB $05
	JR NC,label_9E30	;[label_9E30]
	ADD A,$09
	LD (IX+$13),A
	LD A,(data_9DC0)	;[data_9DC0]
	AND $0F
	JR Z,label_9E4F	;[label_9E4F]
label_9E40:
	SUB $05
	JR NC,label_9E40	;[label_9E40]
	ADD A,$05
	LD B,A
	INC B
	LD A,$01
label_9E4A:
	RLCA
	DJNZ label_9E4A	;[label_9E4A]
	JR label_9E51	;[label_9E51]
label_9E4F:
	LD A,$64
label_9E51:
	LD (IX+$17),A
	LD (IX+$18),$08
	LD A,(data_9DC1)	;[data_9DC1]
	AND $0F
label_9E5D:
	SUB $05
	JR NC,label_9E5D	;[label_9E5D]
	ADD A,$05
	LD (IX+$19),A
	LD A,C
	CP $02
	JR NZ,label_9E6F	;[label_9E6F]
	LD (IX+$19),$05
label_9E6F:
	LD A,(data_9DC1)	;[data_9DC1]
	LD B,A
	LD A,$55
label_9E75:
	RRCA
	DJNZ label_9E75	;[label_9E75]
	LD (IX+$0D),A
	XOR A
	PUSH IX
	POP HL
	LD DE,$0005	;[START + 5]
	ADD HL,DE
	LD (HL),$00
	INC HL
	LD (HL),$0F
	INC HL
	LD (HL),$40
	INC HL
	LD (HL),$DF
	ADD HL,DE
	ADD HL,DE
	LD (HL),$02
	DEC HL
	LD (HL),$02
	DEC HL
	ADD HL,DE
	LD (HL),A
	INC HL
	LD (HL),A
	XOR A
	LD (data_9F04),A	;[data_9F04]
label_9E9E:
	LD A,(data_9F04)	;[data_9F04]
	INC A
	LD (data_9F04),A	;[data_9F04]
	SUB $64
	JR NZ,label_9EAE	;[label_9EAE]
	LD (IX+$06),A
	JR label_9F03	;[label_9F03]
label_9EAE:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	RRCA
	JR C,label_9ECD	;[label_9ECD]
label_9EB7:
	SUB $17
	JR NC,label_9EB7	;[label_9EB7]
	ADD A,$1B
	RLCA
	RLCA
	RLCA
	LD C,A
	LD B,$11
	LD A,(data_DAC1)	;[data_DAC1]
	RRCA
	JR C,label_9EE2	;[label_9EE2]
	LD B,$8D
	JR label_9EE2	;[label_9EE2]
label_9ECD:
	SUB $09
	JR NC,label_9ECD	;[label_9ECD]
	ADD A,$0F
	RLCA
	RLCA
	RLCA
	DEC A
	LD B,A
	LD C,$02
	LD A,(data_DAC1)	;[data_DAC1]
	RLCA
	JR C,label_9EE2	;[label_9EE2]
	LD C,$EE
label_9EE2:
	CALL function_9FFC	;[function_9FFC]
	LD DE,NEXT_CHAR	;[NEXT_CHAR]
	LD A,(HL)
	AND $60
	XOR $40
	JR NZ,label_9E9E	;[label_9E9E]
	LD A,$40
	INC HL
	AND (HL)
	JR Z,label_9E9E	;[label_9E9E]
	ADD HL,DE
	AND (HL)
	JR Z,label_9E9E	;[label_9E9E]
	DEC HL
	AND (HL)
	JR Z,label_9E9E	;[label_9E9E]
	LD (IX+$0A),C
	LD (IX+$0B),B
label_9F03:
	RET
data_9F04:
	db $03
label_9F05:
	LD HL,NoPatrollingEnemies	;[NoPatrollingEnemies]
	LD B,(HL)	;use number of patrolling enemies as loop count
	XOR A
	CP B
	JR Z,label_9F49	;[label_9F49]	;no enemies?
	INC HL	;point to first patrolling enemy
PatrolEnemySpawnLoop:
	PUSH BC
	PUSH HL	;backup HL
	CALL GetEnemyPtr	;[GetEnemyPtr]
	POP HL	;restore HL
	LD A,(HL)	;x char coord
	RLCA	;multiplay by 8 to get pixel coord
	RLCA
	RLCA
	LD (IX+$05),A	;store x pixel coord
	INC HL
	LD A,$18
	SUB (HL)	;subtract y char coord
	RLCA	;multiply by 8 to get pixel coord
	RLCA
	RLCA
	DEC A
	LD (IX+$06),A	;store y pixel coord
	INC HL
	LD (IX+$07),$C8
	LD (IX+$08),$B2
	LD C,$01
	LD (IX+$0D),C
	LD A,(IX+$13)
	OR $08
	LD (IX+$13),A
	LD (IX+$14),C
	LD (IX+$15),C
	LD (IX+$19),$06
	POP BC	; pop count
	DJNZ PatrolEnemySpawnLoop	;[PatrolEnemySpawnLoop]
label_9F49:
	LD HL,(FlyingTransportCoords)	;[FlyingTransportCoords]
	LD A,H
	OR L
	JR Z,label_9F57	;[label_9F57]
	LD A,H
	SUB $08
	LD H,A
	CALL function_9F64	;[function_9F64]
label_9F57:
	LD A,(Sprite0_OnTransport)	;[Sprite0_OnTransport]
	CP $02
	RET NZ
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,H	;get player Y
	SUB $08	;subtract 8
	LD H,A	;write back to H
function_9F64:
	LD (Sprite4_XPixelPos),HL	;[Sprite4_XPixelPos]
	LD HL,Enemy_SpriteData	;[Enemy_SpriteData]
	LD (data_DD9F),HL	;[data_DD9F]
	LD A,$07
	LD (data_DDA1),A	;[data_DDA1]
	LD A,$03
	LD (data_9C43),A	;[data_9C43]
	RET
label_9F78:
	LD A,$04
	LD (data_9C43),A	;[data_9C43]
	CALL function_9FD3	;[function_9FD3]
	db $21
	db $B2
	db $9F
	db $CD
	db $C6
	db $DA
	db $3A
	db $C0
	db $DA
	db $E6
	db $06
	db $5F
	db $16
	db $00
	db $19
	db $11
	db $3D
	db $DD
	db $3A
	db $E8
	db $D2
	db $FE
	db $00
	db $C8
	db $47
	db $C5
	db $01
	db $02
	db $00
	db $ED
	db $B0
	db $E5
	db $21
	db $C0
	db $9F
	db $01
	db $13
	db $00
	db $ED
	db $B0
	db $21
	db $0B
	db $00
	db $19
	db $EB
	db $E1
	db $C1
	db $10
	db $E8
	db $C9
	db $50
	db $6F
	db $A8
	db $2F
	db $50
	db $2F
	db $A8
	db $6F
	db $50
	db $6F
	db $A8
	db $2F
	db $50
	db $2F
	db $08
	db $B2
	db $06
	db $00
	db $00
	db $00
	db $05
	db $00
	db $00
	db $00
	db $02
	db $02
	db $04
	db $04
	db $01
	db $00
	db $0A
	db $0A
	db $00
function_9FD3:
	LD DE,$DD3D	;[Sprite1 + 5]
	LD B,$04
label_9FD8:
	PUSH BC
	LD HL,$9FEA	;[label_9FD8 + 18]
	LD BC,$0005	;[START + 5]
	LDIR
	LD HL,$001B	;[GET_CHAR + 3]
	ADD HL,DE
	EX DE,HL
	POP BC
	DJNZ label_9FD8	;[label_9FD8]
	RET
	db $00
	db $00
	db $40
	db $DF
	db $00
GetEnemyPtr:	;B - enemy no, Ptr returned in IX
	LD A,B	;enemy number
	RRCA	;put in top 3 bits
	RRCA
	RRCA
	LD DE,Sprite0	;[Sprite0]
	ADD A,E	;offset by enemy no * 32
	LD E,A
	PUSH DE	;Transfer enemy pointer to IX
	POP IX
	RET
function_9FFC:
	LD H,$00
	LD A,$BF
	SUB B
	AND $F8
	RLA
	RL H
	RLA
	RL H
	LD L,A
	LD A,H
	OR $58
	LD H,A
	LD A,C
	AND $F8
	RRCA
	RRCA
	RRCA
	ADD A,L
	LD L,A
	RET
	db $C9
	db $FF
data_A019:
	db $00
data_A01A:
	db $00
function_A01B:
	LD HL,data_9C40	;[data_9C40]
	XOR A
	CP (HL)
	JR Z,label_A023	;[label_A023]
	DEC (HL)
label_A023:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(data_9C43)	;[data_9C43]
	LD (data_A019),A	;[data_A019]
label_A02C:
	LD A,(data_A019)	;[data_A019]
	RRCA
	RRCA
	RRCA
	LD HL,Sprite0	;[Sprite0]
	LD D,$00
	LD E,A
	ADD HL,DE
	PUSH HL
	POP IX
	XOR A
	CP (IX+$06)
	JP Z,label_A1A0	;[label_A1A0]
	LD A,$04
	LD (data_A01A),A	;[data_A01A]
label_A048:
	LD A,(IX+$15)
	CP $02
	JR Z,label_A094	;[label_A094]
	OR (IX+$16)
	JR Z,label_A094	;[label_A094]
	LD E,(IX+$05)
	LD D,(IX+$06)
	LD HL,(Sprite5_XPixelPos)	;[Sprite5_XPixelPos]
	LD A,E
	CP L
	JR C,label_A063	;[label_A063]
	LD E,L
	LD L,A
label_A063:
	LD A,L
	SUB E
	CP $0E
	JR NC,label_A08C	;[label_A08C]
	LD A,D
	CP H
	JR C,label_A06F	;[label_A06F]
	LD D,H
	LD H,A
label_A06F:
	LD A,H
	SUB D
	CP $0E
	JR NC,label_A08C	;[label_A08C]
	CALL function_A2E7	;[function_A2E7]
	LD (IX+$07),$C8
	LD (IX+$08),$BE
	LD (IX+$09),$07
	LD (IX+$15),$02
	LD (IX+$16),$00
label_A08C:
	LD A,(IX+$15)
	CP $01
	CALL Z,function_A305	;[function_A305]
label_A094:
	DEC (IX+$14)
	JP NZ,label_A199	;[label_A199]
	LD A,(IX+$13)
	LD (IX+$14),A
	LD A,(IX+$15)
	CP $00
	JP Z,label_A1A8	;[label_A1A8]
	CP $02
	JP Z,label_A1EB	;[label_A1EB]
label_A0AD:
	DEC (IX+$18)
	JR NZ,label_A0DE	;[label_A0DE]
	LD A,(IX+$17)
	LD (IX+$18),A
	LD A,(IX+$19)
	CP $00
	JP Z,label_A1FC	;[label_A1FC]
	CP $01
	JP Z,label_A216	;[label_A216]
	CP $02
	JP Z,label_A22D	;[label_A22D]
	CP $03
	JP Z,label_A236	;[label_A236]
	CP $04
	JP Z,label_A263	;[label_A263]
	CP $05
	JP Z,label_A285	;[label_A285]
	CP $06
	JP Z,label_A296	;[label_A296]
label_A0DE:
	LD C,(IX+$0D)
	LD A,(IX+$05)
	CP $03
	JR NC,label_A0F2	;[label_A0F2]
	LD A,C
	AND $FC
	OR $01
label_A0ED:
	LD (IX+$0D),A
	JR label_A110	;[label_A110]
label_A0F2:
	CP $EE
	JR C,label_A0FD	;[label_A0FD]
	LD A,C
	AND $FC
	OR $02
	JR label_A0ED	;[label_A0ED]
label_A0FD:
	CALL function_D2F0	;[function_D2F0]
	AND $03
	JR Z,label_A110	;[label_A110]
	XOR $03
	LD C,A
	LD A,(IX+$0D)
	AND $FC
	OR C
	LD (IX+$0D),A
label_A110:
	LD A,(IX+$05)
	LD C,(IX+$0D)
	BIT 0,C
	JR Z,label_A11D	;[label_A11D]
	ADD A,(IX+$11)
label_A11D:
	BIT 1,C
	JR Z,label_A124	;[label_A124]
	SUB (IX+$11)
label_A124:
	LD (IX+$05),A
	LD C,A
	LD B,(IX+$06)
	INC B
	OR B
	DEC D
	AND $07
	JR NZ,label_A149	;[label_A149]
	CALL function_9C4C	;[function_9C4C]
	LD A,$64
	LD DE,NEXT_CHAR	;[NEXT_CHAR]
	CP (HL)
	JR Z,label_A199	;[label_A199]
	INC HL
	CP (HL)
	JR Z,label_A199	;[label_A199]
	ADD HL,DE
	CP (HL)
	JR Z,label_A199	;[label_A199]
	DEC HL
	CP (HL)
	JR Z,label_A199	;[label_A199]
label_A149:
	LD C,(IX+$0D)
	LD A,(IX+$06)
	CP $12
	JR NC,label_A15D	;[label_A15D]
	LD A,C
	AND $F3
	OR $08
label_A158:
	LD (IX+$0D),A
	JR label_A182	;[label_A182]
label_A15D:
	CP $8D
	JR C,label_A168	;[label_A168]
	LD A,C
	AND $F3
	OR $04
	JR label_A158	;[label_A158]
label_A168:
	LD A,(IX+$19)
	CP $06
	JR Z,label_A182	;[label_A182]
	CALL function_D2F4	;[function_D2F4]
	AND $0C
	JR Z,label_A182	;[label_A182]
	XOR $0C
	LD C,A
	LD A,(IX+$0D)
	AND $F3
	OR C
	LD (IX+$0D),A
label_A182:
	LD C,(IX+$0D)
	LD A,(IX+$06)
	BIT 2,C
	JR Z,label_A18F	;[label_A18F]
	SUB (IX+$12)
label_A18F:
	BIT 3,C
	JR Z,label_A196	;[label_A196]
	ADD A,(IX+$12)
label_A196:
	LD (IX+$06),A
label_A199:
	LD HL,data_A01A	;[data_A01A]
	DEC (HL)
	JP NZ,label_A048	;[label_A048]
label_A1A0:
	LD HL,data_A019	;[data_A019]
	DEC (HL)
	JP NZ,label_A02C	;[label_A02C]
	RET
label_A1A8:
	LD A,(IX+$16)
	INC (IX+$16)
	CP $00
	JR NZ,label_A1CF	;[label_A1CF]
	LD (IX+$07),$48
	LD (IX+$08),$B1
	LD A,(IX+$0A)
	LD (IX+$05),A
	LD A,(IX+$0B)
	LD (IX+$06),A
	LD A,(RandomNo1)	;[RandomNo1]
	AND $03
	INC A
	LD (data_A41C),A	;[data_A41C]
label_A1CF:
	CP $10
	JP NZ,label_A0AD	;[label_A0AD]
	LD (IX+$15),$01
	LD (IX+$16),$00
	LD A,(IX+$0E)
	LD (IX+$07),A
	LD A,(IX+$0F)
	LD (IX+$08),A
	JP label_A0AD	;[label_A0AD]
label_A1EB:
	INC (IX+$16)
	LD A,$08
	CP (IX+$16)
	JP NZ,label_A0AD	;[label_A0AD]
	CALL function_9C49	;[function_9C49]
	JP label_A1A0	;[label_A1A0]
label_A1FC:
	CALL function_A2A5	;[function_A2A5]
	LD C,(IX+$0D)
	LD A,C
	AND $0B
	JR NZ,label_A209	;[label_A209]
	SET 1,C
label_A209:
	LD A,C
	AND $0C
	JR NZ,label_A210	;[label_A210]
	SET 3,C
label_A210:
	LD (IX+$0D),C
	JP label_A0DE	;[label_A0DE]
label_A216:
	CALL function_A2A5	;[function_A2A5]
	CALL function_A2AE	;[function_A2AE]
	AND $03
	RLCA
	LD E,A
	LD D,$00
	LD HL,$A2B9	;[label_A2B5 + 4]
	ADD HL,DE
	LD A,(HL)
	LD (IX+$0D),A
	JP label_A2D2	;[label_A2D2]
label_A22D:
	CALL function_A2A5	;[function_A2A5]
	CALL function_A2C1	;[function_A2C1]
	JP label_A2D2	;[label_A2D2]
label_A236:
	CALL function_A2A5	;[function_A2A5]
	CALL function_A2C1	;[function_A2C1]
	LD BC,$0400	;[BEEP + 8]
label_A23F:
	RRCA
	JR NC,label_A243	;[label_A243]
	INC C
label_A243:
	DJNZ label_A23F	;[label_A23F]
	LD A,C
	CP $01
	JP Z,label_A2D2	;[label_A2D2]
	CALL function_A2AE	;[function_A2AE]
	RLCA
	JP C,label_A2D2	;[label_A2D2]
	RLCA
	AND $01
	INC A
	LD (IX+$11),A
	DEC A
	XOR $01
	INC A
	LD (IX+$12),A
	JP label_A2D2	;[label_A2D2]
label_A263:
	LD B,$00
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,(IX+$05)
	SUB L
	RL B
	LD A,B
	XOR $01
	INC A
	LD D,A
	LD B,$00
	LD A,(IX+$06)
	SUB H
	RL B
	LD A,B
	INC A
	RLCA
	RLCA
	OR D
	LD (IX+$0D),A
	JR label_A2D2	;[label_A2D2]
label_A285:
	LD (IX+$0D),$00
	CALL function_A2AE	;[function_A2AE]
	RRCA
	JP C,label_A0DE	;[label_A0DE]
	CP $46
	JR C,label_A263	;[label_A263]
	JR label_A236	;[label_A236]
label_A296:
	LD A,(IX+$0D)
	AND $03
	JR NZ,label_A29F	;[label_A29F]
	LD A,$01
label_A29F:
	LD (IX+$0D),A
	JP C,label_A0DE	;[label_A0DE]
function_A2A5:
	LD (IX+$11),$02
	LD (IX+$12),$02
	RET
function_A2AE:
	LD A,(data_A019)	;[data_A019]
	LD B,A
	LD A,(RandomNo1)	;[RandomNo1]
label_A2B5:
	RLCA
	DJNZ label_A2B5	;[label_A2B5]
	RET
	db $08
	db $09
	db $01
	db $05
	db $04
	db $06
	db $02
	db $0A
function_A2C1:
	CALL function_A2AE	;[function_A2AE]
	AND $07
	LD E,A
	LD D,$00
	LD HL,$A2B9	;[label_A2B5 + 4]
	ADD HL,DE
	LD A,(HL)
	LD (IX+$0D),A
	RET
label_A2D2:
	LD A,(IX+$17)
	CP $64
	JP NZ,label_A0DE	;[label_A0DE]
	LD A,(data_DAC1)	;[data_DAC1]
	AND $03
	INC A
	RLCA
	LD (IX+$18),A
	JP label_A0DE	;[label_A0DE]
function_A2E7:
	PUSH IX
	LD A,(IX+$08)
	SUB $AE
	RLCA
	LD (data_D41D),A	;[data_D41D]
	CALL function_D422	;[function_D422]
	CALL function_C546	;[function_C546]
	LD A,$12
	CALL function_D7C0	;[function_D7C0]
	LD A,$0B
	LD (data_A41C),A	;[data_A41C]
	POP IX
	RET
function_A305:
	LD E,(IX+$05)
	LD D,(IX+$06)
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,E
	CP L
	JR C,label_A314	;[label_A314]
	LD E,L
	LD L,A
label_A314:
	LD A,L
	SUB E
	CP $0E
	RET NC
	LD A,D
	CP H
	JR C,label_A31F	;[label_A31F]
	LD D,H
	LD H,A
label_A31F:
	LD A,H
	SUB D
	CP $0B
	RET NC
	LD A,(IX+$08)
	CP $B4
	JR NC,label_A345	;[label_A345]
	LD H,A
	LD L,(IX+$07)
	LD (Sprite0_SpriteAddr2),HL	;[Sprite0_SpriteAddr2]
	LD A,(IX+$09)
	LD (Sprite0_Col),A	;[Sprite0_Col]
	LD A,L
	CP $C8
	LD A,$01
	JR NZ,label_A341	;[label_A341]
	LD A,$11
label_A341:
	POP HL
	JP label_C350	;[label_C350]
label_A345:
	LD HL,$DD30	;[data_DD2E + 2]
	LD A,(HL)
	ADD A,$0A
	LD (HL),A
	RET
	db $32
	db $43
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $7F
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $7F
	db $FF
	db $7F
	db $FF
	db $7F
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
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
	db $00
	db $00
function_A410:
	JR DrawHUDPanel	;[DrawHUDPanel]
ClearGameplayArea_Thunk:
	JP ClearGameplayArea	;[ClearGameplayArea]
UpdateElectricalHazardGraphics:
	JP UpdateElectricHazardGraphicsImpl	;[UpdateElectricHazardGraphicsImpl]
function_A418:
	JP label_A57B	;[label_A57B]
data_A41B:
	db $00
data_A41C:
	db $00
data_A41D:
	db $00
	db $93
data_A41F:
	db $03
	db $09
data_A421:
	db $02
	db $02
function_A423:
	JP label_A7D5	;[label_A7D5]
DrawHUDPanel:
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,H
	INC A
	AND $F8
	DEC A
	LD H,A
	LD A,L
	AND $F8
	LD L,A
	LD (Sprite0_XPixelPos),HL	;[Sprite0_XPixelPos]
	LD HL,START
	LD (AttributeDrawListPtr),HL	;[AttributeDrawListPtr]
	LD BC,$0001	;[START + 1]	;coords 1,0
	CALL DrawHUDPanelOutline	;[DrawHUDPanelOutline]
	LD BC,$0002	;[START + 2]	;coords 2,0
	LD L,$94	;Score Panel
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	INC L	;Lives panel
	LD C,$0A	;X = 10
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	INC L	; Next Panel
	LD C,$12	;X = 18
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	INC L	;Last HUD panel
	LD C,$1A	;X=26
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	CALL function_D425	;[function_D425]
	CALL ClearGameplayArea	;[ClearGameplayArea]
	LD HL,AttributeDrawList	;[AttributeDrawList]
	LD (AttributeDrawListPtr),HL	;[AttributeDrawListPtr]
	LD B,$A0
	XOR A
label_A46B:
	LD (HL),A
	INC HL
	DJNZ label_A46B	;[label_A46B]
	CALL ResetScreen	;[ResetScreen]
	LD HL,$A390	;[label_A345 + 75]
	CALL function_A804	;[function_A804]
	DEC A
	JR NZ,label_A486	;[label_A486]
	CALL function_A801	;[function_A801]
	LD A,$19
	LD (data_D41D),A	;[data_D41D]
	CALL function_D422	;[function_D422]
label_A486:
	LD HL,(FRAMES)	;[FRAMES]
	LD (RandomNo1),HL	;[RandomNo1]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD HL,Sprite0	;[Sprite0]
	LD B,$06
label_A494:
	PUSH BC
	EX DE,HL
	LD HL,$A4A7	;[label_A494 + 19]
	LD BC,$0005	;[START + 5]
	LDIR
	LD HL,$001B	;[GET_CHAR + 3]
	ADD HL,DE
	POP BC
	DJNZ label_A494	;[label_A494]
	JR label_A4B1	;[label_A4B1]
	db $00
	db $DE
	db $40
	db $DF
	db $00
	db $00
	db $00
	db $40
	db $DF
	db $07
label_A4B1:
	LD HL,PlatformStates	;[PlatformStates]
	LD B,$31
PlatformClearLoop:
	LD (HL),$00	;clear platform item
	INC HL
	DJNZ PlatformClearLoop	;[PlatformClearLoop]
	XOR A
	db $32
	db $29
	db $DD
	LD (DownHeld),A	;[DownHeld]
	LD A,$07
	LD (Sprite0_Col),A	;[Sprite0_Col]
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	XOR A
	CP H
	JR NZ,label_A4D4	;[label_A4D4]
	LD A,$C7
	CP L
	JP Z,label_A6C1	;[label_A6C1]
label_A4D4:
	LD A,(data_D2C4)	;[data_D2C4]
	CP $04
	JR NZ,label_A4DF	;[label_A4DF]
	LD A,$0D
	JR label_A4E5	;[label_A4E5]
label_A4DF:
	CP $05
	JR NZ,label_A4FA	;[label_A4FA]
	LD A,$0F
label_A4E5:
	LD HL,$96FE	;[data_96FA + 4]
	LD B,$14
label_A4EA:
	CP (HL)
	JR Z,label_A4F2	;[label_A4F2]
	INC HL
	INC HL
	INC HL
	DJNZ label_A4EA	;[label_A4EA]
label_A4F2:
	DEC HL
	LD D,(HL)
	DEC HL
	LD E,(HL)
	LD (Sprite0_XPixelPos),DE	;[Sprite0_XPixelPos]
label_A4FA:
	LD A,(data_D2C4)	;[data_D2C4]
	CP $01
	JR NZ,label_A50D	;[label_A50D]
	LD HL,(data_D2DC)	;[data_D2DC]
	LD (Sprite0_XPixelPos),HL	;[Sprite0_XPixelPos]
	LD A,(data_D2C5)	;[data_D2C5]
	LD (Sprite0_OnTransport),A	;[Sprite0_OnTransport]
label_A50D:
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD (data_D2DC),HL	;[data_D2DC]
	LD A,(Sprite0_OnTransport)	;[Sprite0_OnTransport]
	LD (data_D2C5),A	;[data_D2C5]
	LD A,(data_D2C4)	;[data_D2C4]
	CP $03
	JR Z,label_A523	;[label_A523]
	CALL function_9C47	;[function_9C47]
label_A523:
	CALL function_C544	;[function_C544]
	CP $64
	JR NC,label_A530	;[label_A530]
	LD (data_D2C4),A	;[data_D2C4]
	JP DrawHUDPanel	;[DrawHUDPanel]
label_A530:
	LD A,(data_D2CD)	;[data_D2CD]
	CP $00
	LD A,$02
	CALL Z,label_C350	;[label_C350]
	LD HL,ElecticHazardList	;[ElecticHazardList]
CheckPlayerElectricHazard:
	LD DE,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,(HL)	;get electric hazard X pos
	CP $00	;check if it's 0
	JR Z,label_A576	;[label_A576]	;jump out if it is
	INC HL
	RLCA	;mutliply by 8
	RLCA
	RLCA
	CP E	;compare to player X
	JR NC,label_A550	;[label_A550]
	LD C,A
	LD A,E
	LD E,C
label_A550:
	SUB E
	CP $0E
	JR NC,label_A570	;[label_A570]
	LD A,$1A
	SUB (HL)
	RLCA
	RLCA
	RLCA
	DEC A
	DEC A
	SUB D
	JR C,label_A570	;[label_A570]
	CP $17
	JR NC,label_A570	;[label_A570]
	LD BC,$0004	;[START + 4]
	ADD HL,BC
	XOR A
	CP (HL)
	CALL NZ,label_C350	;[label_C350]
	OR A
	SBC HL,BC
label_A570:
	LD BC,$0007	;[START + 7]	;advance to next hazard
	ADD HL,BC
	JR CheckPlayerElectricHazard	;[CheckPlayerElectricHazard]
label_A576:
	CALL function_A01B	;[function_A01B]
	JR label_A523	;[label_A523]
label_A57B:
	LD HL,data_A41B	;[data_A41B]
	XOR A
	CP (HL)
	JR Z,label_A586	;[label_A586]
	LD A,(HL)
	CALL function_A5DE	;[function_A5DE]
label_A586:
	INC HL
	INC HL
	XOR A
	CP (HL)
	JR NZ,label_A595	;[label_A595]
	DEC HL
	CP (HL)
	JR Z,label_A5CA	;[label_A5CA]
	LD A,(HL)
	CALL function_A5DE	;[function_A5DE]
	INC HL
label_A595:
	LD DE,data_A421	;[data_A421]
	EX DE,HL
	DEC (HL)
	JR NZ,label_A5DC	;[label_A5DC]
	INC HL
	LD A,(HL)
	DEC HL
	LD (HL),A
	EX DE,HL
	DEC (HL)
	INC HL
	INC HL
	LD A,(HL)
	DEC HL
	ADD A,(HL)
	LD (HL),A
	LD C,$00
	LD A,(FRAMES)	;[FRAMES]
	LD D,A
	LD HL,$A41E	;[data_A41D + 1]
	LD A,(HL)
	INC HL
	INC HL
	XOR (HL)
	RRCA
	AND $7F
	INC (HL)
	LD E,A
label_A5BA:
	LD A,C
	XOR $10
	LD C,A
	OUT ($FE),A
	LD B,E
label_A5C1:
	LD A,(FRAMES)	;[FRAMES]
	CP D
	RET NZ
	DJNZ label_A5C1	;[label_A5C1]
	JR label_A5BA	;[label_A5BA]
label_A5CA:
	LD A,(RandomNo1)	;[RandomNo1]
	CP $04
	JP NC,label_A5DC	;[label_A5DC]
	LD A,(data_DAC1)	;[data_DAC1]
	AND $03
	ADD A,$0C
	LD (data_A41C),A	;[data_A41C]
label_A5DC:
	HALT
	RET
function_A5DE:
	LD (HL),$00
	PUSH HL
	DEC A
	RLCA
	RLCA
	LD E,A
	LD D,$00
	LD HL,$A607	;[function_A5DE + 41]
	ADD HL,DE
	LD DE,data_A41D	;[data_A41D]
	PUSH DE
	LD BC,$0004	;[START + 4]
	LDIR
	POP HL
	LD A,(HL)
	LD E,A
	AND $3F
	LD (HL),A
	LD A,E
	RLCA
	RLCA
	AND $03
	INC A
	LD H,A
	LD L,A
	LD (data_A421),HL	;[data_A421]
	POP HL
	RET
	db $0A
	db $0C
	db $03
	db $64
	db $09
	db $0C
	db $03
	db $00
	db $0A
	db $5A
	db $00
	db $0D
	db $0E
	db $18
	db $0C
	db $F3
	db $06
	db $C8
	db $F7
	db $FA
	db $46
	db $C8
	db $EB
	db $FA
	db $01
	db $96
	db $00
	db $00
	db $03
	db $78
	db $0A
	db $0F
	db $3F
	db $00
	db $00
	db $1A
	db $05
	db $1D
	db $0C
	db $1A
	db $04
	db $3B
	db $05
	db $21
	db $42
	db $0C
	db $F4
	db $FF
	db $1E
	db $00
	db $00
	db $1E
	db $4A
	db $0C
	db $03
	db $C8
	db $49
	db $78
	db $03
	db $00
	db $00
	db $00
	db $00
	db $00
ClearGameplayArea:
	LD HL,$DE1E	;[ScanlineTable + 64]
	LD B,$90
ClearRowLoop:
	LD E,(HL)
	INC HL
	LD D,(HL)
	INC HL
	PUSH HL
	EX DE,HL
	LD C,$20
ClearColumnLoop:
	LD (HL),$00	;clear pixels
	INC HL
	DEC C
	JR NZ,ClearColumnLoop	;[ClearColumnLoop]
	POP HL
	DJNZ ClearRowLoop	;[ClearRowLoop]
	LD HL,$58C0	;[ScreenAttributes + 192]	;attribute clear start address
	LD DE,$0240	;[KEYTABLE_B + 20]	;attribute clear count
ClearAttrLoop:
	LD (HL),$47	;clear colour
	INC HL
	DEC DE
	LD A,D
	OR E
	JR NZ,ClearAttrLoop	;[ClearAttrLoop]
	RET
UpdateElectricHazardGraphicsImpl:
	LD A,(ElectricHazardUpdateCounter)	;[ElectricHazardUpdateCounter]
	INC A
	CP $04
	JR C,SkipClearElecTimer	;[SkipClearElecTimer]
	XOR A
SkipClearElecTimer:
	LD (ElectricHazardUpdateCounter),A	;[ElectricHazardUpdateCounter]
	RLCA	;multply by 8
	RLCA
	RLCA
	LD E,A	;offset into electric hazard list
	LD D,$00
	LD HL,ElecticHazardList	;[ElecticHazardList]
	ADD HL,DE
	LD C,(HL)	;X pos
	INC HL
	LD B,(HL)	;y pos
	XOR A	;checkif y pos is zero
	CP B
	RET Z	;return if it is
	INC HL
	INC HL
	DEC (HL)	;decrement counter
	LD E,(HL)
	INC HL
	LD A,E	;check if counter has gone below 0
	CP $FF
	JR NZ,SkipToggleElectric	;[SkipToggleElectric]
	LD A,(HL)
	DEC HL
	LD (HL),A
	INC HL
	INC HL
	LD A,(HL)
	XOR $01
	LD (HL),A
	LD L,$05	;Platform 5
	LD A,$47	;set attribute to bright white ink
	CALL DrawPlatform	;[DrawPlatform]
	RET
SkipToggleElectric:
	INC HL
	XOR A
	CP (HL)	;check active flag
	RET Z	;return if inactive
	LD A,E
	AND $03	;mask out bottom 2 bits of counter
	LD E,A
	LD D,$00
	LD HL,AltElectricPlatforms	;[AltElectricPlatforms]
	ADD HL,DE
	LD L,(HL)	;platform no
	LD A,(RandomNo1)	;[RandomNo1]	;random no 0-3
	AND $03
	ADD A,$44	;convert to random ink col with bright
	CALL DrawPlatform	;[DrawPlatform]
	RET
AltElectricPlatforms:
	db $06
	db $07
	db $07
	db $06
label_A6C1:
	CALL label_A7D5	;[label_A7D5]
	db $CD
	db $8D
	db $A7
	db $06
	db $02
	db $C5
	db $21
	db $D3
	db $D2
	db $06
	db $04
	db $C5
	db $AF
	db $BE
	db $2B
	db $CA
	db $49
	db $A7
	db $11
	db $DE
	db $D2
	db $06
	db $09
	db $1A
	db $D6
	db $80
	db $BE
	db $C2
	db $46
	db $A7
	db $E5
	db $C5
	db $D5
	db $3E
	db $01
	db $32
	db $1A
	db $D4
	db $CD
	db $22
	db $D4
	db $D1
	db $C1
	db $E1
	db $E5
	db $C5
	db $D5
	db $3E
	db $09
	db $90
	db $5F
	db $01
	db $BD
	db $D2
	db $B7
	db $ED
	db $42
	db $06
	db $01
	db $4D
	db $16
	db $07
	db $3E
	db $19
	db $F5
	db $7A
	db $EE
	db $05
	db $57
	db $CD
	db $55
	db $C3
	db $7A
	db $21
	db $40
	db $DF
	db $CD
	db $24
	db $DB
	db $3E
	db $03
	db $CD
	db $C0
	db $D7
	db $F1
	db $3D
	db $20
	db $E8
	db $6B
	db $E5
	db $CD
	db $8D
	db $A7
	db $E1
	db $D1
	db $7D
	db $12
	db $D5
	db $CD
	db $8D
	db $A7
	db $21
	db $E7
	db $D2
	db $35
	db $7E
	db $D1
	db $C1
	db $E1
	db $E5
	db $C5
	db $D5
	db $D6
	db $02
	db $30
	db $FC
	db $C6
	db $02
	db $CA
	db $A9
	db $A7
	db $CD
	db $94
	db $A7
	db $CD
	db $25
	db $D4
	db $D1
	db $C1
	db $E1
	db $13
	db $10
	db $92
	db $C1
	db $23
	db $23
	db $23
	db $10
	db $80
	db $C1
	db $05
	db $C2
	db $C9
	db $A6
	db $CD
	db $47
	db $9C
	db $06
	db $C8
	db $C5
	db $CD
	db $C8
	db $D9
	db $CD
	db $1B
	db $A0
	db $06
	db $02
	db $CD
	db $58
	db $C3
	db $10
	db $FB
	db $3A
	db $C0
	db $DA
	db $E6
	db $01
	db $C6
	db $14
	db $CD
	db $C0
	db $D7
	db $C1
	db $10
	db $E5
	db $21
	db $1D
	db $DD
	db $36
	db $F0
	db $23
	db $36
	db $27
	db $23
	db $36
	db $74
	db $23
	db $36
	db $E3
	db $21
	db $C8
	db $D2
	db $35
	db $AF
	db $32
	db $C4
	db $D2
	db $C3
	db $10
	db $A4
	db $01
	db $0D
	db $0C
	db $CD
	db $52
	db $C3
	db $C9
	db $AF
	db $77
	db $23
	db $77
	db $11
	db $CF
	db $D2
	db $ED
	db $52
	db $7D
	db $0F
	db $57
	db $CD
	db $4C
	db $C5
	db $36
	db $0A
	db $23
	db $36
	db $C7
	db $C9
	db $CD
	db $94
	db $A7
	db $CD
	db $25
	db $D4
	db $21
	db $E8
	db $D2
	db $34
	db $E5
	db $0E
	db $0A
	db $06
	db $08
	db $05
	db $CD
	db $5B
	db $C3
	db $04
	db $3E
	db $11
	db $CD
	db $C0
	db $D7
	db $10
	db $F4
	db $0D
	db $20
	db $EF
	db $E1
	db $7E
	db $FE
	db $05
	db $C2
	db $43
	db $A7
	db $E1
	db $CD
	db $29
	db $5E
	db $C3
	db $26
	db $5E
label_A7D5:
	LD HL,Sprite0	;[Sprite0]
	LD B,$06
label_A7DA:
	PUSH BC
	EX DE,HL
	LD HL,$A4A7	;[label_A494 + 19]
	LD BC,$0009	;[ERROR_1 + 1]
	LDIR
	LD HL,$0017	;[PRINT_A_1 + 7]
	ADD HL,DE
	POP BC
	DJNZ label_A7DA	;[label_A7DA]
	RET
	db $0D
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
PlatformColourLUT_Minus1:
	db $20
PlatformColourLUT:
	db $03
	db $02
	db $05
	db $06
ResetScreen:
	JR ResetScreenImpl	;[ResetScreenImpl]
function_A7FE:
	JP function_AA02	;[function_AA02]
function_A801:
	JP label_ABE1	;[label_ABE1]
function_A804:
	JP function_ABF0	;[function_ABF0]
function_A807:
	JP label_AB9F	;[label_AB9F]
ResetScreenImpl:	;Actual Implementation of ResetScreen
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	PUSH HL	;put current screen number on the stack
	LD HL,ScreenStateData	;[ScreenStateData]
	LD DE,label_0140	;[label_0140]	;number of bytes to clear
ScreenClearLoop:
	LD (HL),$00
	INC HL
	DEC DE
	LD A,D
	OR E
	JR NZ,ScreenClearLoop	;[ScreenClearLoop]
	LD HL,PatrollingEnemies	;[PatrollingEnemies]
	LD (PatrollingEnemyPtr),HL	;[PatrollingEnemyPtr]
	LD HL,ElecticHazardList	;[ElecticHazardList]	;reset electric hazard list
	LD (ElectricHazardPtr),HL	;[ElectricHazardPtr]
	LD HL,$6700	;[label_666D + 147]
	LD (TravelTubeCounter),HL	;[TravelTubeCounter]
	LD HL,CurrScreenItemPositions	;[CurrScreenItemPositions]
	LD (LastScreenItemPtr),HL	;[LastScreenItemPtr]
	LD HL,$96FC	;[data_96FA + 2]
	LD (data_96FA),HL	;[data_96FA]
	LD HL,START
	LD (FlyingTransportCoords),HL	;[FlyingTransportCoords]
	POP HL	;put screen number in HL
	LD DE,$000C	;[ERROR_1 + 4]
	CALL HL_HLxDE	;[HL_HLxDE]
	LD DE,Screens	;[Screens]	;seed RNG with screen address?
	ADD HL,DE
	LD (RandomNo1),HL	;[RandomNo1]
	LD A,(HL)
	LD (RandomNo2),A	;[RandomNo2]
	XOR $5F
	LD (data_DAC3),A	;[data_DAC3]
	LD (RandomNo3),HL	;[RandomNo3]
	PUSH HL
	LD HL,$0303	;[K_NEW + 18]
	LD (RNGCounter1),HL	;[RNGCounter1]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD HL,PlatformColourLUT	;[PlatformColourLUT]
	LD B,$04
PlatformColourLUTClearLoop:
	LD (HL),$00
	INC HL
	DJNZ PlatformColourLUTClearLoop	;[PlatformColourLUTClearLoop]
	LD B,$04	;set loop count
PlatformLUTSetupLoop:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,B
	CP $01	;last col
	JR Z,LastPlatformCol	;[LastPlatformCol]
	LD A,(RandomNo1)	;[RandomNo1]	;get random number 0-3
	AND $03
	CP $02	;less than 2
	JR C,PlatformColLessThan2	;[PlatformColLessThan2]
	INC A
PlatformColLessThan2:
	ADD A,$02
	JR label_A89A	;[label_A89A]
LastPlatformCol:
	LD A,(data_DAC1)	;[data_DAC1]	;get random number high byte
	AND $3F	;mask bottom 6 bits (0-63)
PlatformColSubLoop:
	CP $05
	JR C,label_A893	;[label_A893]	;jump if less than 5
	SUB $05	;otherwise subtract 5
	JR PlatformColSubLoop	;[PlatformColSubLoop]
label_A893:
	CP $02
	JR C,label_A898	;[label_A898]	;jump if less than 2
	INC A	;otherwise inc
label_A898:
	ADD A,$02
label_A89A:
	LD HL,PlatformColourLUT	;[PlatformColourLUT]
	LD C,$04	;loop counter
ExistingColCheckLoop:	;check if this colour has been used before
	CP (HL)	;same as before
	JR Z,PlatformLUTSetupLoop	;[PlatformLUTSetupLoop]	;generate again
	INC HL	;advance pointer
	DEC C
	JR NZ,ExistingColCheckLoop	;[ExistingColCheckLoop]
	LD C,A	;put col in C
	LD A,$04	;set write index to be 4 - count
	SUB B
	LD E,A
	LD D,$00
	LD HL,PlatformColourLUT	;[PlatformColourLUT]
	ADD HL,DE
	LD (HL),C	;write platform col
	DJNZ PlatformLUTSetupLoop	;[PlatformLUTSetupLoop]
	POP HL
	LD B,$06
	LD D,$03
label_A8B9:
	LD E,$04
	LD C,$00
label_A8BD:
	LD A,(HL)
	INC HL
	CALL DrawBigPlatform	;[DrawBigPlatform]
	LD A,C
	ADD A,$08
	LD C,A
	DEC E
	JR NZ,label_A8BD	;[label_A8BD]
	LD A,B
	ADD A,$06
	LD B,A
	DEC D
	JR NZ,label_A8B9	;[label_A8B9]
	JP label_AA30	;[label_AA30]
DrawBigPlatform:	;A - Platform No
	PUSH BC	;Big Platform is 2x2 Small Platforms that are 4x3 chars in size
	PUSH DE
	PUSH HL
	LD L,A	;platform no
	LD A,C	;add 4 to C and put back
	ADD A,$04
	LD C,A
	LD A,B	;add 3 to B & put back
	ADD A,$03
	LD B,A
	LD H,$00	;set page offset to 0
	ADD HL,HL	;multiply by 4 - each big platform is 4 bytes
	ADD HL,HL
	LD DE,Big_PlatformData	;[Big_PlatformData]
	ADD HL,DE	;offset into big platforms
	LD D,$02	;vertical count
DrawBigPlatform_YLoop:
	LD E,$02	;horizontal count
DrawBigPlatform_XLoop:
	LD A,(HL)	;load platform byte
	INC HL	;increment pointer
	PUSH HL
	LD L,A	;L contains small platform number
	CALL SetupSmallPlatformAttributes	;[SetupSmallPlatformAttributes]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	POP HL
	LD A,C	;subtract 4 from X pos
	SUB $04
	LD C,A
	DEC E	;dec horizontal count
	JR NZ,DrawBigPlatform_XLoop	;[DrawBigPlatform_XLoop]
	LD A,C	;reset X position
	ADD A,$08
	LD C,A
	LD A,B	;subtract 3 from Y pos
	SUB $03
	LD B,A
	DEC D	;dec vertical count
	JR NZ,DrawBigPlatform_YLoop	;[DrawBigPlatform_YLoop]
	POP HL
	POP DE
	POP BC
	RET
SetupSmallPlatformAttributes:	;L sml plt no, C - xpo, B - Y Pos
	PUSH HL
	PUSH BC
	PUSH DE
	LD H,$00
	LD DE,SmallPlatformTypeData	;[SmallPlatformTypeData]
	ADD HL,DE
	LD A,(HL)	;fetch value from platform info table
	CP $00
	JP Z,exit_func	;[exit_func]	;jump if zer0
	AND $F0	;mask out bottom 4 bits
	JR Z,label_A927	;[label_A927]	;top 4 bits are clear
	CP $50
	JP C,UsePlatformColour	;[UsePlatformColour]	;jump if less than
label_A927:
	PUSH AF	;store top 4 bits
	LD A,(HL)	;fetch value again
	LD D,A	;store in D
	AND $03	;bottom 2 bits
	ADD A,C	;add X coord
	LD C,A	;put back in C
	LD A,D	;put platform attrib in A
	AND $0C	;mask bits 2 & 3
	RRCA	;shift down 2
	RRCA
	ADD A,B	;add to Y
	LD B,A
	POP AF	;restore top 4 bits
	CP $00
	JP Z,SmlPlat_KillZone	;[SmlPlat_KillZone]
	CP $50	;check if upward transport tube
	JR NZ,SetupScreenElectricHazards	;[SetupScreenElectricHazards]	;jump if greater than
	PUSH HL	;code for upward transport tubes
	PUSH DE
	LD A,C	;put offset X in A
	DEC A	;decrement
	LD E,A	;put in E
	RRCA	;divide by 2
	ADD A,E	;add to E
	LD E,A
	LD A,$17	;set A to a count
	SUB B	;remove offset Y
	LD D,$FF
UpTubeDivCountLoop:
	INC D
	SUB $03
	JR NC,UpTubeDivCountLoop	;[UpTubeDivCountLoop]	;D should give us how many 3 char tube sections to reach the screen top
	LD A,E
	ADD A,D	;put no of sections in A
	RLCA	;mult by 2
	LD E,A	;use as offset into table
	LD D,$00
	LD HL,TransportTubeCoordTable	;[TransportTubeCoordTable]
	ADD HL,DE	;offset
	LD (HL),C	;store coords
	INC HL
	LD (HL),B
	POP DE
	POP HL
	JP exit_func	;[exit_func]
SetupScreenElectricHazards:
	CP $60	;check for kill zone
	JP Z,SmlPlat_KillZone	;[SmlPlat_KillZone]
	CP $70	;check for electric hazards
	JR NZ,SetupScreenPatrollers	;[SetupScreenPatrollers]	;not ray effect
	LD HL,(ElectricHazardPtr)	;[ElectricHazardPtr]
	LD (HL),C	;store xpos
	INC HL
	LD (HL),B	;store y pos
	INC HL
	LD A,(ElectricHazardUpdateCounter)	;[ElectricHazardUpdateCounter]	;increment value & store back
	INC A
	LD (ElectricHazardUpdateCounter),A	;[ElectricHazardUpdateCounter]
	INC HL
	LD A,(RandomNo1)	;[RandomNo1]	;get random number
	AND $0C	;mask bits 2 & 3
	ADD A,$08	;add 8
	LD (HL),A	;store in timer ?
	INC HL
	LD (HL),A	;timer reset val?
	INC HL
	LD (HL),$00	;set ray to off
	INC HL	;skip 3 bytes
	INC HL
	INC HL
	LD (ElectricHazardPtr),HL	;[ElectricHazardPtr]	;store pointer back
	JP exit_func	;[exit_func]
SetupScreenPatrollers:
	CP $80	;check for platform type : patrolling enemy placement
	JR NZ,SetupScreenItemPlacement	;[SetupScreenItemPlacement]
	LD HL,(PatrollingEnemyPtr)	;[PatrollingEnemyPtr]
	LD (HL),C	;put x coord
	INC HL
	LD (HL),B	;put y coord
	INC HL
	LD (PatrollingEnemyPtr),HL	;[PatrollingEnemyPtr]
	LD HL,NoPatrollingEnemies	;[NoPatrollingEnemies]
	INC (HL)
	JP exit_func	;[exit_func]
SetupScreenItemPlacement:
	CP $90	; check for platform type: item placement
	JR NZ,SetupScreenLockedDoor	;[SetupScreenLockedDoor]
	LD HL,(LastScreenItemPtr)	;[LastScreenItemPtr]
	LD (HL),C	;put X pos
	INC HL
	LD (HL),B	;put Y pos
	INC HL
	LD (LastScreenItemPtr),HL	;[LastScreenItemPtr]
	LD HL,NoScreenItemPositions	;[NoScreenItemPositions]
	INC (HL)	;increment number of screen item positions
	JP exit_func	;[exit_func]
SetupScreenLockedDoor:
	CP $B0	;check for platform type : locked door?
	JR NZ,label_A9E3	;[label_A9E3]
	PUSH HL
	PUSH BC
	LD (LockedDoorCoords),BC	;[LockedDoorCoords]	;set coords
	LD BC,(CurrentScreenNo)	;[CurrentScreenNo]
	LD HL,RoomNumbersOf???	;[RoomNumbersOf???]
ItemScreenCheck:
	LD E,(HL)	;low byte
	INC HL
	LD A,(HL)	;high byte
	RLCA	;check high bit
	AND $01
	CP B	;compare with current room high bit
	JR NZ,ScreenNo_NoMatch	;[ScreenNo_NoMatch]	;jump out if not equal
	LD A,E	;compare room number low byte
	CP C
	JR NZ,ScreenNo_NoMatch	;[ScreenNo_NoMatch]	;jump out if not equal
	LD (data_9605),HL	;[data_9605]	;store pointer at address
	POP BC	;restore coords
	POP HL	;restore screen item pointer
	JR SmlPlat_KillZone	;[SmlPlat_KillZone]
ScreenNo_NoMatch:
	INC HL	;advance to next one
	JR ItemScreenCheck	;[ItemScreenCheck]
label_A9E3:
	JR SmlPlat_KillZone	;[SmlPlat_KillZone]
UsePlatformColour:
	RRCA	;rotate right 4 - make the bottom 4 bits the top 4 bits
	RRCA
	RRCA
	RRCA
	LD HL,PlatformColourLUT_Minus1	;[PlatformColourLUT_Minus1]	;point HL to LUT
	LD D,$00	;offset by A
	LD E,A
	ADD HL,DE
	LD A,(HL)	;fetch colour
	LD (PlatformLUTColour),A	;[PlatformLUTColour]	;store in platform colour 2
	JR exit_func	;[exit_func]
SmlPlat_KillZone:
	LD A,(HL)	;get platform type
	RRCA	;shift down & mask top 4 bits - type
	RRCA
	RRCA
	RRCA
	AND $0F
	CALL function_AA02	;[function_AA02]
	JR exit_func	;[exit_func]
function_AA02:
	PUSH BC	;store char coords
	PUSH HL
	PUSH AF
	LD A,$18
	SUB B
	RLCA	;shift up 3
	RLCA
	RLCA
	DEC A
	LD B,A	;put back in B
	LD A,C	;Multiply X by 8
	RLCA
	RLCA
	RLCA
	LD C,A	;put back in C
	POP AF	;restore A & push back
	PUSH AF
	LD HL,(data_96FA)	;[data_96FA]	;get pointer
	LD (HL),C	;store X
	INC HL
	LD (HL),B	;store Y
	INC HL
	LD (HL),A	;Store type
	INC HL
	LD (data_96FA),HL	;[data_96FA]	;write pointer back
	CP $0C	;check for 'Flying transport'
	JR NZ,ExitFunc	;[ExitFunc]
	LD (FlyingTransportCoords),BC	;[FlyingTransportCoords]
ExitFunc:
	POP AF
	POP HL
	POP BC
	RET
exit_func:
	POP DE
	POP BC
	POP HL
	RET
label_AA30:
	XOR A
	LD (data_D2BE),A	;[data_D2BE]
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	XOR A
	CP H
	JR NZ,label_AA3F	;[label_AA3F]
	LD A,$C7
	CP L
	RET Z
label_AA3F:
	LD A,(NoScreenItemPositions)	;[NoScreenItemPositions]
	LD E,A
	LD A,(data_DAC1)	;[data_DAC1]
	LD D,A
	LD A,(data_D2C7)	;[data_D2C7]
	XOR D
	AND $7F
label_AA4D:
	SUB E
	CP E
	JR NC,label_AA4D	;[label_AA4D]
	LD (data_AA9F),A	;[data_AA9F]
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	LD BC,CollectableItemInfo	;[CollectableItemInfo]
	LD A,$2D
label_AA5C:
	PUSH AF
	INC BC
	LD A,(BC)
	LD D,A
	AND $7F
	INC BC
	JR NZ,label_AAA0	;[label_AAA0]
	LD A,(BC)
	LD E,A
	LD A,D
	RLCA
	AND $01
	CP H
	JR NZ,label_AAA0	;[label_AAA0]
	LD A,E
	CP L
	JR NZ,label_AAA0	;[label_AAA0]
	LD A,(data_AA9F)	;[data_AA9F]
	RLCA
	LD D,$00
	LD E,A
	LD HL,CurrScreenItemPositions	;[CurrScreenItemPositions]
	ADD HL,DE
	DEC BC
	DEC BC
	LD E,(HL)
	INC HL
	LD A,(RandomNo2)	;[RandomNo2]
	LD D,A
	LD A,(data_D2C6)	;[data_D2C6]
	XOR D
	AND $3F
label_AA8B:
	SUB $06
	CP $06
	JR NC,label_AA8B	;[label_AA8B]
	ADD A,$02
	RRCA
	RRCA
	RRCA
	OR E
	LD (BC),A
	INC BC
	LD A,(BC)
	OR (HL)
	LD (BC),A
	POP AF
	JR label_AAA6	;[label_AAA6]
data_AA9F:
	db $02
label_AAA0:
	INC BC
	INC BC
	POP AF
	DEC A
	JR NZ,label_AA5C	;[label_AA5C]
label_AAA6:
	LD HL,(data_D2C6)	;[data_D2C6]
	LD (RandomNo2),HL	;[RandomNo2]
	LD B,$04
	LD HL,ScreenItemPosition	;[ScreenItemPosition]
label_AAB1:
	LD (HL),$00
	INC HL
	DJNZ label_AAB1	;[label_AAB1]
	LD HL,$A350	;[label_A345 + 11]
	CALL function_ABF0	;[function_ABF0]
	CP $00
	JP Z,DrawCollectable	;[DrawCollectable]
	LD B,$14
label_AAC3:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	DJNZ label_AAC3	;[label_AAC3]
	LD A,(RandomNo1)	;[RandomNo1]
	CP $55
	JP C,DrawCollectable	;[DrawCollectable]
label_AAD0:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(NoScreenItemPositions)	;[NoScreenItemPositions]
	CP $01
	JP Z,DrawCollectable	;[DrawCollectable]
	LD E,A
	LD A,(RandomNo1)	;[RandomNo1]
	AND $7F
label_AAE1:
	SUB E
	CP E
	JR NC,label_AAE1	;[label_AAE1]
	LD E,A
	LD A,(data_AA9F)	;[data_AA9F]
	CP E
	JR Z,label_AAD0	;[label_AAD0]
	LD A,E
	RLCA	;mult by 2
	LD E,A	;setup offset
	LD D,$00
	LD HL,CurrScreenItemPositions	;[CurrScreenItemPositions]	;position table
	ADD HL,DE	;apply offset
	LD C,(HL)	;get x position
	INC HL
	LD B,(HL)	;get y position
	LD (ScreenItemPosition),BC	;[ScreenItemPosition]	;store position
label_AAFC:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
label_AB02:
	SUB $09
	CP $09
	JR NC,label_AB02	;[label_AB02]
	LD D,A
	CP $08
	JR NZ,label_AB15	;[label_AB15]
	LD A,(data_DAC1)	;[data_DAC1]
	CP $7F
	JR NC,label_AAFC	;[label_AAFC]
	LD A,D
label_AB15:
	ADD A,$11
	PUSH AF
	CALL GetItemImagePtr_Thunk	;[GetItemImagePtr_Thunk]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(data_DAC1)	;[data_DAC1]	;create random attribute col
	AND $3F	;bottom 6 bits
PickupSubLoop:
	SUB $06
	CP $06
	JR NC,PickupSubLoop	;[PickupSubLoop]	;keep looping while a > 6
	ADD A,$02
	OR $40	;or in bright bit
	CALL DrawItem	;[DrawItem]	;this draws the item in the level??
	LD (data_D2C3),A	;[data_D2C3]
	POP AF
	LD (ScreenItemType),A	;[ScreenItemType]
	LD A,$01
	CALL function_AA02	;[function_AA02]
	LD HL,data_D2BE	;[data_D2BE]
	INC (HL)
DrawCollectable:
	LD HL,CollectableItemInfo	;[CollectableItemInfo]
	LD A,$2D
CheckCollectableLoop:
	PUSH AF
	PUSH HL	;backup HL
	INC HL	;skip first byte
	LD A,(HL)
	RLCA
	AND $01
	LD B,A	;screen no high byte
	INC HL
	LD C,(HL)	;screen no low byte
	POP HL	;restore HL
	PUSH HL	;backup again
	LD DE,(CurrentScreenNo)	;[CurrentScreenNo]
	LD A,D	;check if screen number is correct for this item
	CP B
	JR NZ,SkipToNextCollectable	;[SkipToNextCollectable]	;not in this room
	LD A,E
	CP C
	JR NZ,SkipToNextCollectable	;[SkipToNextCollectable]	;not in this room
	LD A,(HL)	;read first byte
	LD E,A
	AND $1F	;mask off bottom 5 bits
	LD C,A	;x position
	INC HL
	LD A,(HL)	;read second byte
	AND $7F	;mask off bottom 7 bits
	CP $06	;less than 6 means it's in our inventory
	JR C,SkipToNextCollectable	;[SkipToNextCollectable]
	LD B,A	;ypos
	INC HL	;skip byte 3
	INC HL	;point to item no
	LD A,(HL)	;item number
	CALL GetItemImagePtr_Thunk	;[GetItemImagePtr_Thunk]
	LD A,E	;retrieve first byte
	AND $E0	;mask top 3 bits
	RLCA	;rotate them round to bottom 3 bits
	RLCA
	RLCA
	OR $40	;or in bright bit
	CALL DrawItem	;[DrawItem]
	POP HL
	POP DE
	PUSH DE
	PUSH HL
	LD A,$41
	SUB D
	CALL function_AA02	;[function_AA02]
	LD HL,data_D2BE	;[data_D2BE]
	INC (HL)
SkipToNextCollectable:
	POP HL
	LD DE,$0004	;[START + 4]	;add 4 to item pointer
	ADD HL,DE
	POP AF
	DEC A
	JR NZ,CheckCollectableLoop	;[CheckCollectableLoop]
	LD HL,(data_9605)	;[data_9605]
	XOR A
	CP H
	JR Z,label_ABBD	;[label_ABBD]
	LD A,(HL)
	AND $7F
	JR NZ,label_ABBD	;[label_ABBD]
label_AB9F:
	LD HL,(LockedDoorCoords)	;[LockedDoorCoords]
	LD A,L
	AND $FC
	OR $01
	LD (data_ABB8),A	;[data_ABB8]
	LD B,$03
	LD A,H
	INC H
	LD (data_ABB7),A	;[data_ABB7]
	CALL DrawText	;[DrawText]
	db $13
	db $01
	db $16
data_ABB7:
	db $00
data_ABB8:
	db $00
	db $20
	db $FF
	db $10
	db $EF
label_ABBD:
	RET
function_ABBE:
	PUSH DE
	LD C,A
	LD DE,(CurrentScreenNo)	;[CurrentScreenNo]
	LD A,D
	RRCA
	RRCA
	RRCA
	LD D,A
	LD A,E
	AND $07
	LD B,A
	LD A,E
	AND $F8
	RRCA
	RRCA
	RRCA
	OR D
	LD E,A
	LD D,$00
	ADD HL,DE
	LD A,(HL)
	INC B
	LD D,B
label_ABDB:
	RLCA
	DJNZ label_ABDB	;[label_ABDB]
	LD B,D
	POP DE
	RET
label_ABE1:
	PUSH HL
	PUSH BC
	CALL function_ABBE	;[function_ABBE]
	AND $FE
	OR C
label_ABE9:
	RRCA
	DJNZ label_ABE9	;[label_ABE9]
	LD (HL),A
	POP BC
	POP HL
	RET
function_ABF0:
	PUSH HL
	PUSH BC
	CALL function_ABBE	;[function_ABBE]
	AND $01
	POP BC
	POP HL
	RET
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $3E
	db $FF
	db $11
	db $00
	db $C0
	db $DD
	db $21
	db $00
	db $40
	db $C3
	db $C2
	db $04
	db $CD
	db $54
	db $AC
	db $D0
	db $3E
	db $16
	db $3D
	db $20
	db $FD
	db $A7
	db $04
	db $C8
	db $3E
	db $7F
	db $DB
	db $FE
	db $1F
	db $D0
	db $A9
	db $E6
	db $20
	db $28
	db $F3
	db $79
	db $2F
	db $4F
	db $E6
	db $07
	db $F6
	db $08
	db $D3
	db $FE
	db $37
	db $C9
	db $3E
	db $FF
	db $11
	db $00
	db $C0
	db $DD
	db $21
	db $00
	db $40
	db $14
	db $08
	db $15
	db $F3
	db $3E
	db $0B
	db $D3
	db $FE
	db $21
	db $00
	db $00
	db $E5
	db $DB
	db $FE
	db $1F
	db $E6
	db $20
	db $F6
	db $03
	db $4F
	db $BF
	db $C0
	db $CD
	db $54
	db $AC
	db $30
	db $FA
	db $21
	db $15
	db $04
	db $10
	db $FE
	db $2B
	db $7C
	db $B5
	db $20
	db $F9
	db $CD
	db $50
	db $AC
	db $30
	db $EB
	db $06
	db $9C
	db $CD
	db $50
	db $AC
	db $30
	db $E4
	db $3E
	db $C6
	db $B8
	db $30
	db $E0
	db $24
	db $20
	db $F1
	db $06
	db $C9
	db $CD
	db $54
	db $AC
	db $30
	db $D5
	db $78
	db $FE
	db $D4
	db $30
	db $F4
	db $CD
	db $54
	db $AC
	db $D0
	db $79
	db $EE
	db $03
	db $4F
	db $26
	db $00
	db $06
	db $B0
	db $18
	db $1E
	db $08
	db $20
	db $05
	db $DD
	db $75
	db $00
	db $18
	db $0A
	db $CB
	db $11
	db $AD
	db $C0
	db $79
	db $1F
	db $4F
	db $13
	db $18
	db $02
	db $DD
	db $23
	db $7D
	db $E6
	db $07
	db $A9
	db $4F
	db $AF
	db $1B
	db $08
	db $06
	db $B2
	db $2E
	db $01
	db $CD
	db $50
	db $AC
	db $D0
	db $3E
	db $CB
	db $B8
	db $CB
	db $15
	db $06
	db $B0
	db $D2
	db $EE
	db $AC
	db $7C
	db $AD
	db $67
	db $7A
	db $B3
	db $20
	db $CB
	db $7C
	db $FE
	db $01
	db $D2
	db $00
	db $00
	db $C3
	db $24
	db $5E
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
	db $80
	db $00
	db $80
	db $80
	db $00
	db $80
	db $00
	db $00
	db $C0
	db $00
	db $C0
	db $C0
	db $00
	db $C0
	db $00
	db $00
	db $E0
	db $00
	db $E0
	db $E0
	db $00
	db $E0
	db $00
	db $00
	db $F0
	db $00
	db $F0
	db $F0
	db $00
	db $F0
	db $00
	db $00
	db $F8
	db $00
	db $F8
	db $F8
	db $00
	db $F8
	db $00
	db $00
	db $FC
	db $00
	db $FC
	db $FC
	db $00
	db $FC
	db $00
	db $00
	db $FE
	db $00
	db $FE
	db $FE
	db $00
	db $FE
	db $00
	db $00
	db $FF
	db $00
	db $FE
	db $FE
	db $00
	db $FF
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
	db $1C
	db $00
	db $7F
	db $5D
	db $35
	db $77
	db $00
	db $00
	db $00
	db $0C
	db $0C
	db $0C
	db $0C
	db $0C
	db $0C
	db $0C
	db $7C
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $7F
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $30
	db $30
	db $00
	db $63
	db $63
	db $06
	db $3C
	db $78
	db $7B
	db $7B
	db $00
	db $7E
	db $6E
	db $6E
	db $6F
	db $6F
	db $6F
	db $7F
	db $00
	db $3C
	db $1C
	db $1C
	db $3E
	db $3E
	db $3E
	db $3E
	db $00
	db $7F
	db $67
	db $07
	db $7F
	db $60
	db $7F
	db $7F
	db $00
	db $7E
	db $0E
	db $0E
	db $7F
	db $0F
	db $7F
	db $7F
	db $00
	db $6E
	db $6E
	db $7F
	db $7F
	db $0F
	db $0F
	db $0F
	db $00
	db $7E
	db $66
	db $60
	db $7F
	db $0F
	db $7F
	db $7F
	db $00
	db $7E
	db $66
	db $60
	db $7F
	db $6F
	db $6F
	db $7F
	db $00
	db $7E
	db $6E
	db $0E
	db $0F
	db $0F
	db $0F
	db $0F
	db $00
	db $7E
	db $6E
	db $6F
	db $7F
	db $6F
	db $6F
	db $7F
	db $00
	db $7F
	db $6F
	db $6F
	db $7F
	db $0F
	db $0F
	db $0F
	db $00
	db $00
	db $18
	db $18
	db $00
	db $18
	db $18
	db $00
	db $00
	db $08
	db $18
	db $3F
	db $7F
	db $3F
	db $18
	db $08
	db $00
	db $08
	db $0C
	db $7E
	db $7F
	db $7E
	db $0C
	db $08
	db $00
	db $1C
	db $1C
	db $1C
	db $7F
	db $3E
	db $1C
	db $08
	db $00
	db $08
	db $1C
	db $3E
	db $7F
	db $1C
	db $1C
	db $1C
	db $00
	db $7F
	db $61
	db $67
	db $41
	db $47
	db $47
	db $7F
	db $00
	db $7E
	db $7F
	db $7F
	db $7E
	db $78
	db $78
	db $78
	db $00
	db $3F
	db $3B
	db $3B
	db $7F
	db $7B
	db $7B
	db $7B
	db $00
	db $7F
	db $7B
	db $7B
	db $7E
	db $7B
	db $7B
	db $7F
	db $00
	db $7F
	db $7B
	db $78
	db $78
	db $78
	db $7B
	db $7F
	db $00
	db $7F
	db $7B
	db $3B
	db $3B
	db $3B
	db $7B
	db $7F
	db $00
	db $7E
	db $70
	db $7E
	db $78
	db $78
	db $7F
	db $7F
	db $00
	db $7E
	db $70
	db $7E
	db $78
	db $78
	db $78
	db $78
	db $00
	db $7F
	db $73
	db $70
	db $77
	db $73
	db $73
	db $7F
	db $00
	db $7B
	db $7B
	db $7B
	db $7F
	db $7B
	db $7B
	db $7B
	db $00
	db $3F
	db $1E
	db $1E
	db $1E
	db $1E
	db $1E
	db $3F
	db $00
	db $1F
	db $0F
	db $0F
	db $0F
	db $0F
	db $6F
	db $7F
	db $00
	db $7B
	db $7B
	db $7A
	db $7E
	db $7B
	db $7B
	db $7B
	db $00
	db $78
	db $78
	db $78
	db $78
	db $78
	db $78
	db $7F
	db $00
	db $7F
	db $7F
	db $75
	db $75
	db $75
	db $75
	db $75
	db $00
	db $7B
	db $7B
	db $7B
	db $7F
	db $77
	db $73
	db $73
	db $00
	db $7F
	db $7B
	db $7B
	db $7B
	db $7B
	db $7B
	db $7F
	db $00
	db $7F
	db $7B
	db $7B
	db $7F
	db $78
	db $78
	db $78
	db $00
	db $7E
	db $76
	db $76
	db $76
	db $7E
	db $77
	db $7F
	db $00
	db $7E
	db $76
	db $76
	db $7F
	db $7B
	db $7B
	db $7B
	db $00
	db $7E
	db $76
	db $70
	db $7F
	db $03
	db $7F
	db $7F
	db $00
	db $7F
	db $1C
	db $1C
	db $1C
	db $1C
	db $1C
	db $1C
	db $00
	db $7B
	db $7B
	db $7B
	db $7B
	db $7B
	db $7F
	db $7F
	db $00
	db $7B
	db $7B
	db $7B
	db $7B
	db $7E
	db $7C
	db $78
	db $00
	db $75
	db $75
	db $75
	db $75
	db $75
	db $7F
	db $7F
	db $00
	db $7B
	db $7B
	db $7B
	db $3E
	db $7F
	db $7B
	db $7B
	db $00
	db $7B
	db $7B
	db $7B
	db $1E
	db $1E
	db $1E
	db $1E
	db $00
	db $3F
	db $33
	db $03
	db $7F
	db $78
	db $7F
	db $7F
	db $00
	db $7F
	db $57
	db $75
	db $00
	db $6F
	db $3A
	db $6A
	db $00
	db $00
	db $00
	db $6F
	db $7B
	db $6B
	db $00
	db $00
	db $00
	db $00
	db $7E
	db $57
	db $00
	db $2E
	db $3C
	db $00
	db $00
	db $00
	db $00
	db $00
Enemy_SpriteData:
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
	db $DD
	db $DD
	db $00
	db $02
	db $20
	db $00
	db $0F
	db $F0
	db $00
	db $72
	db $2E
	db $00
	db $DB
	db $D7
	db $00
	db $B6
	db $EB
	db $00
	db $F5
	db $EF
	db $00
	db $07
	db $E0
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
	db $2E
	db $EE
	db $C0
	db $01
	db $10
	db $00
	db $03
	db $FC
	db $00
	db $1C
	db $47
	db $80
	db $36
	db $F5
	db $C0
	db $2D
	db $BA
	db $C0
	db $3D
	db $7B
	db $C0
	db $01
	db $F8
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
	db $07
	db $77
	db $70
	db $00
	db $88
	db $80
	db $00
	db $FF
	db $00
	db $07
	db $88
	db $E0
	db $0D
	db $BD
	db $70
	db $0B
	db $6E
	db $B0
	db $0F
	db $5E
	db $F0
	db $00
	db $7E
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
	db $03
	db $BB
	db $B8
	db $00
	db $44
	db $40
	db $00
	db $3F
	db $C0
	db $01
	db $D1
	db $38
	db $03
	db $6F
	db $5C
	db $02
	db $DB
	db $AC
	db $03
	db $D7
	db $BC
	db $00
	db $1F
	db $80
	db $00
	db $00
	db $00
	db $00
	db $80
	db $00
	db $01
	db $C0
	db $00
	db $03
	db $E4
	db $00
	db $11
	db $C0
	db $00
	db $00
	db $80
	db $00
	db $00
	db $10
	db $00
	db $10
	db $84
	db $00
	db $39
	db $CE
	db $00
	db $10
	db $84
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $08
	db $08
	db $00
	db $00
	db $80
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
	db $02
	db $20
	db $00
	db $07
	db $00
	db $00
	db $02
	db $04
	db $00
	db $00
	db $0E
	db $00
	db $10
	db $9F
	db $00
	db $01
	db $CE
	db $00
	db $03
	db $E4
	db $00
	db $01
	db $C0
	db $00
	db $10
	db $80
	db $00
	db $00
	db $04
	db $00
	db $00
	db $20
	db $00
	db $02
	db $70
	db $00
	db $00
	db $20
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
	db $80
	db $00
	db $00
	db $08
	db $00
	db $08
	db $1C
	db $00
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $04
	db $80
	db $00
	db $21
	db $C1
	db $00
	db $00
	db $80
	db $00
	db $00
	db $08
	db $00
	db $08
	db $1C
	db $00
	db $1C
	db $3E
	db $00
	db $08
	db $1C
	db $00
	db $00
	db $08
	db $00
	db $00
	db $80
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
	db $02
	db $20
	db $00
	db $00
	db $00
	db $00
	db $10
	db $00
	db $00
	db $38
	db $04
	db $00
	db $10
	db $00
	db $00
	db $00
	db $80
	db $00
	db $00
	db $04
	db $00
	db $20
	db $0E
	db $00
	db $02
	db $04
	db $00
	db $07
	db $00
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $00
	db $00
	db $04
	db $00
	db $00
	db $09
	db $80
	db $00
	db $4B
	db $60
	db $00
	db $2A
	db $F0
	db $00
	db $02
	db $F0
	db $00
	db $C0
	db $60
	db $00
	db $00
	db $04
	db $00
	db $00
	db $00
	db $00
	db $40
	db $00
	db $00
	db $00
	db $03
	db $00
	db $06
	db $00
	db $00
	db $0F
	db $44
	db $00
	db $0F
	db $52
	db $00
	db $06
	db $D0
	db $00
	db $01
	db $90
	db $00
	db $00
	db $20
	db $00
	db $00
	db $80
	db $00
	db $00
	db $98
	db $00
	db $00
	db $30
	db $00
	db $02
	db $0E
	db $00
	db $00
	db $18
	db $00
	db $00
	db $16
	db $00
	db $00
	db $0F
	db $00
	db $06
	db $0F
	db $00
	db $0F
	db $06
	db $00
	db $0F
	db $00
	db $00
	db $06
	db $80
	db $00
	db $01
	db $80
	db $00
	db $07
	db $04
	db $00
	db $00
	db $C0
	db $00
	db $01
	db $90
	db $00
	db $00
	db $10
	db $00
	db $00
	db $01
	db $00
	db $00
	db $09
	db $20
	db $00
	db $00
	db $40
	db $00
	db $00
	db $00
	db $01
	db $80
	db $E0
	db $03
	db $C0
	db $10
	db $03
	db $C1
	db $C0
	db $05
	db $80
	db $60
	db $06
	db $01
	db $A0
	db $03
	db $83
	db $C0
	db $08
	db $03
	db $C0
	db $07
	db $01
	db $80
	db $00
	db $00
	db $00
	db $02
	db $00
	db $00
	db $04
	db $90
	db $00
	db $00
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $0C
	db $00
	db $00
	db $5E
	db $00
	db $00
	db $5E
	db $10
	db $01
	db $6C
	db $00
	db $01
	db $B0
	db $0C
	db $00
	db $80
	db $00
	db $00
	db $00
	db $10
	db $03
	db $00
	db $D8
	db $00
	db $03
	db $68
	db $00
	db $47
	db $A0
	db $00
	db $07
	db $A0
	db $00
	db $03
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $05
	db $80
	db $00
	db $15
	db $88
	db $00
	db $00
	db $00
	db $00
	db $40
	db $02
	db $00
	db $01
	db $E0
	db $00
	db $02
	db $73
	db $00
	db $02
	db $F0
	db $00
	db $C2
	db $F3
	db $00
	db $C2
	db $73
	db $00
	db $01
	db $E0
	db $00
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $40
	db $02
	db $00
	db $00
	db $00
	db $00
	db $11
	db $A8
	db $00
	db $01
	db $A0
	db $00
	db $00
	db $A0
	db $00
	db $02
	db $2C
	db $00
	db $00
	db $0C
	db $00
	db $00
	db $00
	db $00
	db $19
	db $E0
	db $80
	db $1A
	db $70
	db $00
	db $02
	db $F0
	db $40
	db $32
	db $F0
	db $00
	db $02
	db $70
	db $C0
	db $21
	db $E0
	db $00
	db $00
	db $01
	db $80
	db $10
	db $01
	db $80
	db $00
	db $00
	db $00
	db $03
	db $00
	db $00
	db $03
	db $44
	db $00
	db $00
	db $50
	db $00
	db $00
	db $29
	db $00
	db $00
	db $02
	db $00
	db $03
	db $00
	db $C0
	db $03
	db $00
	db $C0
	db $08
	db $00
	db $00
	db $04
	db $00
	db $00
	db $00
	db $78
	db $10
	db $08
	db $9C
	db $00
	db $00
	db $BC
	db $10
	db $08
	db $BC
	db $00
	db $00
	db $9C
	db $20
	db $00
	db $78
	db $10
	db $03
	db $00
	db $C0
	db $03
	db $00
	db $C0
	db $00
	db $40
	db $00
	db $00
	db $94
	db $00
	db $00
	db $01
	db $40
	db $00
	db $30
	db $00
	db $00
	db $B0
	db $30
	db $00
	db $80
	db $00
	db $00
	db $00
	db $18
	db $02
	db $00
	db $18
	db $00
	db $07
	db $80
	db $02
	db $09
	db $C0
	db $00
	db $0B
	db $C4
	db $00
	db $0B
	db $C0
	db $01
	db $89
	db $C4
	db $01
	db $87
	db $80
	db $00
	db $00
	db $10
	db $00
	db $C0
	db $D0
	db $00
	db $00
	db $C0
	db $00
	db $14
	db $00
	db $01
	db $10
	db $00
	db $1B
	db $B8
	db $00
	db $CE
	db $E3
	db $00
	db $84
	db $41
	db $00
	db $60
	db $06
	db $00
	db $30
	db $0C
	db $00
	db $1B
	db $D8
	db $00
	db $15
	db $E8
	db $00
	db $08
	db $10
	db $00
	db $05
	db $E0
	db $00
	db $55
	db $EE
	db $00
	db $F7
	db $EE
	db $00
	db $C8
	db $13
	db $00
	db $40
	db $03
	db $00
	db $FF
	db $FF
	db $00
	db $6E
	db $EE
	db $00
	db $02
	db $22
	db $00
	db $07
	db $76
	db $00
	db $35
	db $DC
	db $C0
	db $20
	db $88
	db $40
	db $18
	db $01
	db $80
	db $0C
	db $03
	db $00
	db $06
	db $F6
	db $00
	db $05
	db $7A
	db $00
	db $02
	db $04
	db $00
	db $01
	db $78
	db $00
	db $19
	db $7B
	db $80
	db $3D
	db $FB
	db $C0
	db $12
	db $04
	db $C0
	db $30
	db $00
	db $80
	db $3F
	db $FF
	db $C0
	db $0E
	db $EE
	db $80
	db $00
	db $44
	db $00
	db $00
	db $EE
	db $80
	db $0D
	db $BB
	db $B0
	db $08
	db $11
	db $10
	db $06
	db $00
	db $60
	db $03
	db $00
	db $C0
	db $01
	db $BD
	db $80
	db $01
	db $5E
	db $80
	db $00
	db $81
	db $00
	db $00
	db $5E
	db $00
	db $05
	db $5E
	db $E0
	db $0F
	db $7E
	db $E0
	db $0C
	db $81
	db $30
	db $04
	db $00
	db $30
	db $0F
	db $FF
	db $F0
	db $06
	db $EE
	db $E0
	db $00
	db $08
	db $80
	db $00
	db $1D
	db $C0
	db $03
	db $77
	db $6C
	db $02
	db $22
	db $04
	db $01
	db $80
	db $18
	db $00
	db $C0
	db $30
	db $00
	db $6F
	db $60
	db $00
	db $57
	db $A0
	db $00
	db $20
	db $40
	db $00
	db $17
	db $80
	db $01
	db $57
	db $B0
	db $03
	db $DF
	db $BC
	db $01
	db $20
	db $4C
	db $03
	db $00
	db $08
	db $03
	db $FF
	db $FC
	db $00
	db $EE
	db $E8
	db $03
	db $C0
	db $00
	db $04
	db $E0
	db $00
	db $05
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $BB
	db $DD
	db $00
	db $BB
	db $DD
	db $00
	db $BB
	db $DD
	db $00
	db $00
	db $00
	db $00
	db $05
	db $E0
	db $00
	db $05
	db $E0
	db $00
	db $05
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $1B
	db $F8
	db $00
	db $00
	db $00
	db $00
	db $02
	db $C0
	db $00
	db $01
	db $80
	db $00
	db $00
	db $F0
	db $00
	db $01
	db $38
	db $00
	db $01
	db $78
	db $00
	db $00
	db $00
	db $00
	db $2D
	db $EE
	db $C0
	db $2D
	db $EE
	db $C0
	db $2D
	db $EE
	db $C0
	db $00
	db $00
	db $00
	db $01
	db $78
	db $00
	db $01
	db $78
	db $00
	db $01
	db $78
	db $00
	db $00
	db $00
	db $00
	db $06
	db $FE
	db $00
	db $00
	db $00
	db $00
	db $00
	db $B0
	db $00
	db $00
	db $60
	db $00
	db $00
	db $3C
	db $00
	db $00
	db $4E
	db $00
	db $00
	db $5E
	db $00
	db $00
	db $00
	db $00
	db $0E
	db $F7
	db $B0
	db $0E
	db $F7
	db $B0
	db $0E
	db $F7
	db $B0
	db $00
	db $00
	db $00
	db $00
	db $5E
	db $00
	db $00
	db $5E
	db $00
	db $00
	db $5E
	db $00
	db $00
	db $00
	db $00
	db $01
	db $BF
	db $80
	db $00
	db $00
	db $00
	db $00
	db $2C
	db $00
	db $00
	db $18
	db $00
	db $00
	db $0F
	db $00
	db $00
	db $13
	db $80
	db $00
	db $17
	db $80
	db $00
	db $00
	db $00
	db $03
	db $7B
	db $DC
	db $03
	db $7B
	db $DC
	db $03
	db $7B
	db $DC
	db $00
	db $00
	db $00
	db $00
	db $17
	db $80
	db $00
	db $17
	db $80
	db $00
	db $17
	db $80
	db $00
	db $00
	db $00
	db $00
	db $6F
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $0B
	db $00
	db $00
	db $06
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $70
	db $00
	db $00
	db $D8
	db $00
	db $0A
	db $C8
	db $00
	db $6A
	db $F8
	db $00
	db $35
	db $B0
	db $00
	db $DE
	db $C0
	db $00
	db $75
	db $B0
	db $80
	db $AE
	db $F9
	db $C0
	db $7B
	db $BF
	db $40
	db $D5
	db $7F
	db $80
	db $3F
	db $DF
	db $00
	db $6A
	db $80
	db $00
	db $55
	db $60
	db $00
	db $15
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $1C
	db $00
	db $00
	db $36
	db $00
	db $05
	db $32
	db $00
	db $15
	db $BE
	db $00
	db $2A
	db $BC
	db $00
	db $0B
	db $A0
	db $00
	db $36
	db $FC
	db $00
	db $0D
	db $AE
	db $20
	db $37
	db $7F
	db $F0
	db $1D
	db $AF
	db $D0
	db $2B
	db $77
	db $E0
	db $1A
	db $A8
	db $00
	db $15
	db $50
	db $00
	db $05
	db $40
	db $00
	db $00
	db $00
	db $00
	db $00
	db $07
	db $00
	db $00
	db $0D
	db $80
	db $02
	db $AC
	db $80
	db $01
	db $6F
	db $80
	db $07
	db $AF
	db $00
	db $02
	db $F8
	db $00
	db $0D
	db $B7
	db $00
	db $02
	db $FF
	db $80
	db $0D
	db $5B
	db $C8
	db $03
	db $AF
	db $FC
	db $05
	db $7B
	db $F4
	db $02
	db $AC
	db $F8
	db $02
	db $B4
	db $00
	db $00
	db $A0
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
	db $C0
	db $00
	db $03
	db $60
	db $00
	db $A3
	db $20
	db $00
	db $AB
	db $E0
	db $03
	db $5B
	db $C0
	db $01
	db $EE
	db $00
	db $03
	db $5B
	db $C0
	db $00
	db $ED
	db $E2
	db $03
	db $5F
	db $FF
	db $00
	db $F5
	db $FD
	db $01
	db $DF
	db $7E
	db $03
	db $2B
	db $00
	db $00
	db $D5
	db $00
	db $00
	db $94
	db $00
	db $00
	db $00
	db $00
	db $07
	db $B6
	db $00
	db $0F
	db $6D
	db $00
	db $17
	db $6D
	db $00
	db $3F
	db $7F
	db $00
	db $3F
	db $7F
	db $00
	db $1F
	db $BA
	db $00
	db $7D
	db $C0
	db $00
	db $7E
	db $00
	db $00
	db $7E
	db $00
	db $00
	db $18
	db $78
	db $00
	db $7E
	db $54
	db $00
	db $7D
	db $1C
	db $00
	db $2F
	db $2C
	db $00
	db $1F
	db $F8
	db $00
	db $1B
	db $B8
	db $00
	db $03
	db $A0
	db $00
	db $01
	db $ED
	db $80
	db $03
	db $DB
	db $40
	db $05
	db $DB
	db $40
	db $0F
	db $DF
	db $C0
	db $0F
	db $DF
	db $C0
	db $07
	db $EE
	db $80
	db $1E
	db $F0
	db $00
	db $1F
	db $00
	db $00
	db $1F
	db $86
	db $00
	db $06
	db $03
	db $00
	db $1F
	db $82
	db $80
	db $1F
	db $43
	db $80
	db $0B
	db $C7
	db $00
	db $07
	db $FD
	db $00
	db $06
	db $EE
	db $00
	db $00
	db $EC
	db $00
	db $00
	db $7B
	db $60
	db $00
	db $F6
	db $D0
	db $03
	db $76
	db $D0
	db $07
	db $F7
	db $F0
	db $07
	db $F7
	db $F0
	db $03
	db $FB
	db $A0
	db $0F
	db $BC
	db $00
	db $0F
	db $C0
	db $00
	db $0F
	db $C0
	db $80
	db $03
	db $00
	db $C0
	db $0F
	db $C1
	db $C0
	db $0F
	db $A0
	db $C0
	db $05
	db $E3
	db $80
	db $03
	db $FE
	db $80
	db $03
	db $77
	db $00
	db $00
	db $74
	db $00
	db $00
	db $1E
	db $D8
	db $00
	db $3D
	db $B4
	db $00
	db $DD
	db $B4
	db $01
	db $FD
	db $FC
	db $01
	db $FD
	db $FC
	db $00
	db $FE
	db $E8
	db $03
	db $EF
	db $00
	db $03
	db $F0
	db $00
	db $03
	db $F0
	db $C0
	db $00
	db $C0
	db $60
	db $03
	db $F0
	db $50
	db $03
	db $E8
	db $70
	db $01
	db $78
	db $E0
	db $00
	db $FF
	db $A0
	db $00
	db $DD
	db $C0
	db $00
	db $1D
	db $80
	db $1F
	db $E0
	db $00
	db $71
	db $F8
	db $00
	db $EC
	db $DC
	db $00
	db $E8
	db $FE
	db $00
	db $F1
	db $EA
	db $00
	db $FF
	db $FF
	db $00
	db $6F
	db $85
	db $00
	db $7F
	db $7B
	db $00
	db $3E
	db $FC
	db $00
	db $00
	db $1C
	db $00
	db $B0
	db $1C
	db $00
	db $D8
	db $3D
	db $00
	db $0F
	db $E3
	db $00
	db $29
	db $2A
	db $00
	db $32
	db $98
	db $00
	db $1B
	db $B0
	db $00
	db $03
	db $FC
	db $00
	db $0E
	db $3F
	db $00
	db $1D
	db $9B
	db $80
	db $1D
	db $1F
	db $C0
	db $1E
	db $3D
	db $40
	db $1F
	db $FF
	db $E0
	db $0D
	db $F0
	db $A0
	db $0F
	db $EF
	db $60
	db $07
	db $DF
	db $80
	db $00
	db $07
	db $00
	db $2C
	db $07
	db $40
	db $06
	db $0F
	db $40
	db $13
	db $FC
	db $00
	db $19
	db $11
	db $80
	db $0A
	db $AB
	db $00
	db $03
	db $BA
	db $00
	db $00
	db $7F
	db $80
	db $01
	db $C7
	db $E0
	db $03
	db $B3
	db $70
	db $03
	db $A3
	db $F8
	db $03
	db $C7
	db $A8
	db $03
	db $FF
	db $FC
	db $01
	db $BE
	db $14
	db $01
	db $FD
	db $EC
	db $00
	db $FB
	db $E0
	db $00
	db $01
	db $C0
	db $0B
	db $01
	db $C0
	db $0D
	db $83
	db $D0
	db $00
	db $FE
	db $30
	db $02
	db $92
	db $A0
	db $03
	db $29
	db $80
	db $01
	db $BB
	db $00
	db $00
	db $3F
	db $C0
	db $00
	db $E3
	db $F0
	db $01
	db $D9
	db $B8
	db $01
	db $D1
	db $FC
	db $01
	db $E3
	db $D4
	db $01
	db $FF
	db $FE
	db $00
	db $DF
	db $0A
	db $00
	db $FE
	db $F6
	db $00
	db $7D
	db $F8
	db $00
	db $00
	db $70
	db $02
	db $C0
	db $74
	db $00
	db $60
	db $F4
	db $01
	db $7F
	db $C0
	db $01
	db $91
	db $18
	db $00
	db $AA
	db $B0
	db $00
	db $3B
	db $A0
	db $0E
	db $F8
	db $00
	db $77
	db $5E
	db $00
	db $FB
	db $53
	db $00
	db $F8
	db $7F
	db $00
	db $BB
	db $65
	db $80
	db $9B
	db $77
	db $80
	db $73
	db $AE
	db $80
	db $0D
	db $BB
	db $80
	db $7C
	db $4E
	db $80
	db $3E
	db $F3
	db $80
	db $7F
	db $34
	db $40
	db $7F
	db $C6
	db $80
	db $1F
	db $F8
	db $00
	db $1F
	db $FF
	db $00
	db $03
	db $FF
	db $80
	db $03
	db $33
	db $40
	db $03
	db $BE
	db $00
	db $1D
	db $D7
	db $80
	db $3E
	db $D4
	db $C0
	db $3E
	db $1F
	db $C0
	db $2E
	db $D9
	db $60
	db $26
	db $DD
	db $E0
	db $1C
	db $EB
	db $A0
	db $03
	db $6E
	db $E0
	db $0F
	db $13
	db $A0
	db $1F
	db $BC
	db $E0
	db $1F
	db $CD
	db $10
	db $07
	db $F1
	db $A0
	db $0F
	db $FE
	db $00
	db $0F
	db $FF
	db $C0
	db $03
	db $FF
	db $E0
	db $03
	db $33
	db $30
	db $00
	db $EF
	db $80
	db $07
	db $75
	db $E0
	db $0F
	db $B5
	db $30
	db $0F
	db $87
	db $F0
	db $0B
	db $B6
	db $58
	db $09
	db $B7
	db $78
	db $07
	db $3A
	db $E8
	db $00
	db $DB
	db $B8
	db $07
	db $C4
	db $E8
	db $03
	db $EF
	db $38
	db $07
	db $F3
	db $44
	db $07
	db $FC
	db $68
	db $01
	db $FF
	db $80
	db $01
	db $FF
	db $F0
	db $00
	db $3F
	db $F8
	db $00
	db $33
	db $34
	db $00
	db $3B
	db $E0
	db $01
	db $DD
	db $78
	db $03
	db $ED
	db $4C
	db $03
	db $E1
	db $FC
	db $02
	db $ED
	db $96
	db $02
	db $6D
	db $DE
	db $01
	db $CE
	db $BA
	db $00
	db $36
	db $EE
	db $00
	db $F1
	db $3A
	db $01
	db $FB
	db $CE
	db $01
	db $FC
	db $D1
	db $00
	db $7F
	db $1A
	db $00
	db $FF
	db $E0
	db $00
	db $FF
	db $FC
	db $00
	db $3F
	db $FE
	db $00
	db $33
	db $33
	db $00
	db $00
	db $00
	db $00
	db $EE
	db $00
	db $07
	db $DF
	db $00
	db $1F
	db $5D
	db $00
	db $7D
	db $DD
	db $00
	db $FF
	db $6E
	db $00
	db $FB
	db $D1
	db $00
	db $FF
	db $7F
	db $00
	db $7F
	db $FE
	db $00
	db $1F
	db $F0
	db $00
	db $07
	db $EC
	db $00
	db $00
	db $0E
	db $00
	db $10
	db $86
	db $00
	db $A9
	db $CE
	db $00
	db $47
	db $7C
	db $00
	db $02
	db $38
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3B
	db $80
	db $01
	db $F7
	db $C0
	db $07
	db $D7
	db $40
	db $1F
	db $77
	db $40
	db $3F
	db $DB
	db $80
	db $3E
	db $F4
	db $40
	db $3F
	db $DF
	db $C0
	db $1F
	db $FF
	db $80
	db $07
	db $FC
	db $00
	db $01
	db $FB
	db $00
	db $00
	db $03
	db $80
	db $08
	db $47
	db $80
	db $14
	db $EF
	db $00
	db $23
	db $BC
	db $00
	db $01
	db $18
	db $00
	db $00
	db $0E
	db $E0
	db $00
	db $7D
	db $F0
	db $01
	db $F5
	db $D0
	db $07
	db $DD
	db $D0
	db $0F
	db $F6
	db $E0
	db $0F
	db $BF
	db $10
	db $0F
	db $F7
	db $F0
	db $07
	db $FF
	db $E0
	db $01
	db $FF
	db $00
	db $00
	db $7E
	db $C0
	db $00
	db $00
	db $E0
	db $00
	db $00
	db $70
	db $04
	db $11
	db $F0
	db $0A
	db $3B
	db $E0
	db $01
	db $4E
	db $00
	db $00
	db $84
	db $00
	db $00
	db $01
	db $B8
	db $00
	db $0F
	db $7C
	db $00
	db $3D
	db $74
	db $00
	db $F7
	db $74
	db $01
	db $FD
	db $B8
	db $01
	db $EF
	db $C4
	db $01
	db $FD
	db $FC
	db $00
	db $FF
	db $F8
	db $00
	db $3F
	db $C0
	db $00
	db $0F
	db $B0
	db $00
	db $00
	db $38
	db $00
	db $00
	db $1C
	db $01
	db $10
	db $CC
	db $00
	db $A9
	db $FC
	db $00
	db $47
	db $38
	db $00
	db $02
	db $00
	db $0E
	db $70
	db $00
	db $1F
	db $F8
	db $00
	db $1D
	db $B8
	db $00
	db $59
	db $9A
	db $00
	db $EE
	db $77
	db $00
	db $F1
	db $8F
	db $00
	db $7F
	db $FE
	db $00
	db $7F
	db $FF
	db $00
	db $6F
	db $7A
	db $00
	db $06
	db $B0
	db $00
	db $09
	db $80
	db $00
	db $1F
	db $1C
	db $00
	db $1E
	db $3E
	db $00
	db $1F
	db $FF
	db $00
	db $0F
	db $F3
	db $00
	db $07
	db $C2
	db $00
	db $03
	db $9C
	db $00
	db $07
	db $FE
	db $00
	db $07
	db $6E
	db $00
	db $16
	db $66
	db $80
	db $3B
	db $9D
	db $80
	db $1C
	db $63
	db $C0
	db $3F
	db $FF
	db $C0
	db $3F
	db $FF
	db $80
	db $17
	db $BD
	db $80
	db $03
	db $58
	db $00
	db $00
	db $C0
	db $00
	db $03
	db $C3
	db $80
	db $03
	db $E7
	db $C0
	db $03
	db $FE
	db $E0
	db $01
	db $FC
	db $20
	db $00
	db $78
	db $00
	db $00
	db $E7
	db $00
	db $01
	db $FF
	db $80
	db $01
	db $DB
	db $80
	db $05
	db $99
	db $80
	db $06
	db $E7
	db $70
	db $0F
	db $18
	db $E0
	db $0F
	db $FF
	db $F0
	db $0F
	db $FF
	db $F0
	db $03
	db $BD
	db $C0
	db $03
	db $58
	db $C0
	db $00
	db $60
	db $00
	db $00
	db $78
	db $70
	db $00
	db $78
	db $F8
	db $00
	db $7F
	db $CC
	db $00
	db $3F
	db $80
	db $00
	db $1E
	db $00
	db $00
	db $39
	db $C0
	db $00
	db $7F
	db $E0
	db $00
	db $76
	db $E0
	db $00
	db $66
	db $60
	db $01
	db $B9
	db $D8
	db $03
	db $C6
	db $3C
	db $03
	db $FF
	db $F8
	db $00
	db $FF
	db $FC
	db $00
	db $DE
	db $FC
	db $00
	db $0C
	db $60
	db $00
	db $12
	db $00
	db $00
	db $3C
	db $38
	db $00
	db $3E
	db $7C
	db $00
	db $3F
	db $EE
	db $00
	db $1F
	db $C2
	db $00
	db $07
	db $80
	db $00
	db $00
	db $00
	db $04
	db $40
	db $00
	db $0A
	db $A4
	db $00
	db $0A
	db $AA
	db $40
	db $11
	db $11
	db $80
	db $60
	db $00
	db $00
	db $90
	db $00
	db $00
	db $B6
	db $B0
	db $00
	db $F6
	db $DA
	db $00
	db $6E
	db $DB
	db $00
	db $1E
	db $DB
	db $00
	db $3D
	db $D0
	db $40
	db $0B
	db $0E
	db $00
	db $E0
	db $EE
	db $C0
	db $EE
	db $E0
	db $C0
	db $0E
	db $00
	db $00
	db $00
	db $00
	db $00
	db $02
	db $20
	db $00
	db $05
	db $52
	db $00
	db $05
	db $55
	db $20
	db $18
	db $88
	db $C0
	db $24
	db $00
	db $00
	db $2D
	db $00
	db $00
	db $3D
	db $80
	db $00
	db $1B
	db $B6
	db $80
	db $07
	db $B6
	db $C0
	db $0F
	db $76
	db $D0
	db $06
	db $F6
	db $80
	db $30
	db $F0
	db $70
	db $37
	db $07
	db $70
	db $07
	db $77
	db $00
	db $00
	db $70
	db $00
	db $00
	db $00
	db $00
	db $00
	db $10
	db $00
	db $00
	db $29
	db $10
	db $00
	db $AA
	db $A8
	db $01
	db $44
	db $40
	db $06
	db $00
	db $00
	db $09
	db $40
	db $00
	db $0B
	db $60
	db $00
	db $0F
	db $68
	db $00
	db $06
	db $ED
	db $80
	db $01
	db $ED
	db $B0
	db $00
	db $5D
	db $B4
	db $03
	db $85
	db $80
	db $0B
	db $B8
	db $38
	db $08
	db $3B
	db $B8
	db $00
	db $03
	db $80
	db $00
	db $00
	db $00
	db $00
	db $08
	db $80
	db $00
	db $15
	db $48
	db $00
	db $15
	db $54
	db $00
	db $22
	db $22
	db $00
	db $40
	db $00
	db $01
	db $90
	db $00
	db $02
	db $5A
	db $00
	db $02
	db $DB
	db $00
	db $03
	db $DB
	db $60
	db $01
	db $BB
	db $68
	db $00
	db $62
	db $ED
	db $00
	db $1C
	db $1C
	db $01
	db $DD
	db $C1
	db $01
	db $C1
	db $DD
	db $00
	db $00
	db $1C
	db $00
	db $00
	db $00
	db $09
	db $80
	db $00
	db $6B
	db $40
	db $00
	db $7A
	db $40
	db $00
	db $2B
	db $DF
	db $00
	db $FD
	db $BF
	db $80
	db $AE
	db $7E
	db $C0
	db $7F
	db $CF
	db $C0
	db $2B
	db $70
	db $C0
	db $7F
	db $9F
	db $00
	db $6D
	db $80
	db $00
	db $0C
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
	db $09
	db $60
	db $00
	db $1E
	db $D0
	db $00
	db $0A
	db $91
	db $E0
	db $3E
	db $F7
	db $D0
	db $37
	db $6F
	db $F0
	db $1D
	db $9E
	db $30
	db $0F
	db $F0
	db $00
	db $3D
	db $5C
	db $00
	db $37
	db $E7
	db $00
	db $06
	db $C1
	db $E0
	db $06
	db $C0
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
	db $30
	db $03
	db $18
	db $68
	db $03
	db $BC
	db $F8
	db $0E
	db $B5
	db $E0
	db $0F
	db $A5
	db $C0
	db $06
	db $DB
	db $00
	db $07
	db $E6
	db $00
	db $0D
	db $7C
	db $00
	db $0F
	db $EF
	db $00
	db $03
	db $B9
	db $80
	db $03
	db $60
	db $C0
	db $00
	db $60
	db $70
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
	db $66
	db $00
	db $03
	db $6F
	db $00
	db $03
	db $ED
	db $1E
	db $01
	db $A9
	db $7D
	db $01
	db $F6
	db $FF
	db $07
	db $79
	db $E3
	db $07
	db $DF
	db $00
	db $01
	db $75
	db $C0
	db $03
	db $DE
	db $70
	db $03
	db $64
	db $1E
	db $00
	db $60
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
	db $03
	db $80
	db $00
	db $07
	db $40
	db $00
	db $07
	db $40
	db $01
	db $B6
	db $40
	db $07
	db $77
	db $C0
	db $2F
	db $53
	db $80
	db $6D
	db $74
	db $00
	db $6F
	db $57
	db $80
	db $EB
	db $76
	db $80
	db $EF
	db $BB
	db $80
	db $AE
	db $45
	db $00
	db $F5
	db $FE
	db $00
	db $03
	db $FE
	db $00
	db $FF
	db $E4
	db $00
	db $4E
	db $40
	db $00
	db $04
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $00
	db $01
	db $D0
	db $00
	db $61
	db $D0
	db $01
	db $ED
	db $90
	db $0B
	db $DD
	db $F0
	db $1B
	db $54
	db $E0
	db $1B
	db $DD
	db $00
	db $3A
	db $D5
	db $E0
	db $2B
	db $DD
	db $A0
	db $3B
	db $EE
	db $E0
	db $01
	db $11
	db $40
	db $3E
	db $FF
	db $80
	db $17
	db $FF
	db $00
	db $02
	db $72
	db $00
	db $00
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $38
	db $00
	db $18
	db $74
	db $03
	db $7B
	db $74
	db $06
	db $F7
	db $64
	db $06
	db $D5
	db $7C
	db $0E
	db $F7
	db $38
	db $0A
	db $B5
	db $40
	db $0E
	db $F7
	db $78
	db $00
	db $4B
	db $A8
	db $0F
	db $B0
	db $38
	db $03
	db $FF
	db $D0
	db $01
	db $3F
	db $E0
	db $00
	db $13
	db $80
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $0E
	db $00
	db $00
	db $1D
	db $00
	db $1E
	db $1D
	db $00
	db $3E
	db $D9
	db $00
	db $B5
	db $DF
	db $01
	db $BD
	db $4E
	db $01
	db $AD
	db $D0
	db $03
	db $BD
	db $5E
	db $02
	db $BD
	db $CA
	db $03
	db $80
	db $EE
	db $02
	db $7F
	db $14
	db $01
	db $FF
	db $F8
	db $03
	db $93
	db $F8
	db $01
	db $01
	db $38
	db $00
	db $00
	db $10
	db $03
	db $18
	db $00
	db $07
	db $3C
	db $00
	db $77
	db $FC
	db $00
	db $7F
	db $FC
	db $00
	db $7F
	db $FF
	db $00
	db $3C
	db $9F
	db $00
	db $FB
	db $6F
	db $00
	db $F6
	db $B6
	db $00
	db $F6
	db $B6
	db $00
	db $77
	db $F6
	db $00
	db $3B
	db $6F
	db $00
	db $7C
	db $9F
	db $00
	db $7F
	db $FF
	db $00
	db $77
	db $F8
	db $00
	db $07
	db $78
	db $00
	db $07
	db $38
	db $00
	db $03
	db $18
	db $00
	db $07
	db $3C
	db $00
	db $07
	db $FD
	db $80
	db $37
	db $FF
	db $C0
	db $3F
	db $27
	db $C0
	db $3E
	db $DB
	db $C0
	db $0D
	db $AD
	db $80
	db $05
	db $AD
	db $80
	db $3D
	db $FD
	db $80
	db $3E
	db $DB
	db $C0
	db $3F
	db $27
	db $C0
	db $07
	db $FF
	db $C0
	db $0F
	db $FE
	db $00
	db $0F
	db $EE
	db $00
	db $0E
	db $E6
	db $00
	db $00
	db $E0
	db $00
	db $01
	db $8C
	db $00
	db $03
	db $DE
	db $00
	db $03
	db $FF
	db $E0
	db $03
	db $FF
	db $E0
	db $0F
	db $C9
	db $E0
	db $0F
	db $B6
	db $E0
	db $0F
	db $6B
	db $40
	db $07
	db $6B
	db $60
	db $07
	db $7F
	db $70
	db $0F
	db $B6
	db $F0
	db $0F
	db $C9
	db $F0
	db $0F
	db $FF
	db $C0
	db $01
	db $FF
	db $C0
	db $03
	db $FD
	db $C0
	db $03
	db $DC
	db $00
	db $03
	db $9C
	db $00
	db $00
	db $06
	db $00
	db $00
	db $CF
	db $60
	db $01
	db $EF
	db $F0
	db $01
	db $FF
	db $F0
	db $00
	db $FF
	db $F0
	db $03
	db $F2
	db $78
	db $03
	db $ED
	db $BC
	db $03
	db $DA
	db $DC
	db $01
	db $DA
	db $DC
	db $00
	db $DF
	db $D8
	db $01
	db $ED
	db $B0
	db $01
	db $F2
	db $78
	db $01
	db $FF
	db $F8
	db $00
	db $3F
	db $F8
	db $00
	db $3B
	db $C0
	db $00
	db $39
	db $C0
	db $00
	db $00
	db $00
	db $36
	db $00
	db $00
	db $6B
	db $04
	db $00
	db $4B
	db $86
	db $00
	db $7B
	db $87
	db $00
	db $B7
	db $84
	db $00
	db $CF
	db $82
	db $00
	db $FF
	db $03
	db $00
	db $71
	db $01
	db $00
	db $2E
	db $07
	db $00
	db $1B
	db $DA
	db $00
	db $2E
	db $B4
	db $00
	db $0D
	db $07
	db $00
	db $70
	db $77
	db $00
	db $77
	db $70
	db $00
	db $07
	db $00
	db $00
	db $0D
	db $80
	db $00
	db $1A
	db $C0
	db $00
	db $12
	db $E0
	db $20
	db $1E
	db $E0
	db $30
	db $2D
	db $E0
	db $70
	db $33
	db $E0
	db $40
	db $3F
	db $C0
	db $20
	db $1C
	db $40
	db $20
	db $0B
	db $80
	db $20
	db $06
	db $C0
	db $60
	db $0B
	db $DF
	db $C0
	db $07
	db $76
	db $80
	db $38
	db $28
	db $00
	db $3B
	db $83
	db $80
	db $03
	db $BB
	db $80
	db $00
	db $38
	db $00
	db $03
	db $60
	db $00
	db $06
	db $B0
	db $00
	db $04
	db $B8
	db $02
	db $07
	db $B8
	db $06
	db $0B
	db $78
	db $0E
	db $0C
	db $F8
	db $00
	db $0F
	db $F0
	db $04
	db $07
	db $10
	db $04
	db $02
	db $E0
	db $04
	db $01
	db $90
	db $0C
	db $03
	db $F5
	db $B8
	db $02
	db $3F
	db $B8
	db $01
	db $C2
	db $E0
	db $0D
	db $DC
	db $10
	db $0C
	db $1D
	db $D0
	db $00
	db $01
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $D8
	db $00
	db $01
	db $AC
	db $00
	db $01
	db $2E
	db $02
	db $01
	db $EE
	db $03
	db $02
	db $DE
	db $07
	db $03
	db $3E
	db $04
	db $03
	db $FC
	db $02
	db $01
	db $C4
	db $02
	db $00
	db $BA
	db $C2
	db $00
	db $6F
	db $76
	db $00
	db $B1
	db $DC
	db $01
	db $0E
	db $18
	db $00
	db $EE
	db $E0
	db $02
	db $E0
	db $EC
	db $02
	db $00
	db $0C
	db $18
	db $C0
	db $00
	db $19
	db $C0
	db $00
	db $1D
	db $C0
	db $00
	db $0D
	db $C0
	db $00
	db $0D
	db $C0
	db $00
	db $05
	db $C0
	db $00
	db $01
	db $80
	db $00
	db $1D
	db $AF
	db $C0
	db $6E
	db $7F
	db $00
	db $97
	db $E8
	db $00
	db $B7
	db $C0
	db $00
	db $F7
	db $00
	db $00
	db $60
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
	db $07
	db $9B
	db $F0
	db $1B
	db $77
	db $C0
	db $25
	db $8A
	db $00
	db $2D
	db $F0
	db $00
	db $3D
	db $C0
	db $00
	db $18
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
	db $01
	db $FE
	db $FC
	db $06
	db $E7
	db $F0
	db $09
	db $5A
	db $80
	db $0B
	db $5C
	db $00
	db $0F
	db $5C
	db $00
	db $06
	db $1E
	db $00
	db $00
	db $6E
	db $00
	db $00
	db $6F
	db $00
	db $00
	db $67
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
	db $79
	db $BF
	db $01
	db $B7
	db $7C
	db $02
	db $58
	db $A0
	db $02
	db $DF
	db $00
	db $03
	db $DC
	db $00
	db $01
	db $80
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
	db $22
	db $11
	db $00
	db $55
	db $2A
	db $80
	db $09
	db $24
	db $00
	db $00
	db $00
	db $00
	db $03
	db $30
	db $00
	db $06
	db $D8
	db $00
	db $16
	db $DA
	db $00
	db $37
	db $FB
	db $00
	db $7B
	db $37
	db $80
	db $7C
	db $CF
	db $C0
	db $FF
	db $FF
	db $C0
	db $FF
	db $FF
	db $C0
	db $FF
	db $FF
	db $80
	db $BF
	db $FB
	db $80
	db $3B
	db $B8
	db $00
	db $03
	db $80
	db $00
	db $11
	db $80
	db $80
	db $2A
	db $4D
	db $40
	db $04
	db $4A
	db $20
	db $00
	db $00
	db $00
	db $00
	db $CC
	db $00
	db $01
	db $B6
	db $00
	db $05
	db $B6
	db $80
	db $0D
	db $FE
	db $C0
	db $1E
	db $CD
	db $E0
	db $1F
	db $33
	db $E0
	db $3F
	db $FF
	db $F0
	db $3F
	db $FF
	db $F0
	db $3F
	db $FF
	db $F0
	db $1D
	db $FF
	db $D0
	db $01
	db $DD
	db $C0
	db $00
	db $1C
	db $00
	db $00
	db $80
	db $40
	db $05
	db $52
	db $A8
	db $02
	db $33
	db $10
	db $00
	db $00
	db $00
	db $00
	db $33
	db $00
	db $00
	db $6D
	db $80
	db $00
	db $6D
	db $80
	db $01
	db $7F
	db $A0
	db $03
	db $B3
	db $70
	db $07
	db $CC
	db $F8
	db $07
	db $FF
	db $F8
	db $0F
	db $FF
	db $FC
	db $0F
	db $FF
	db $FC
	db $0E
	db $EF
	db $FC
	db $0E
	db $0E
	db $EC
	db $00
	db $00
	db $E0
	db $00
	db $40
	db $62
	db $00
	db $AC
	db $94
	db $01
	db $14
	db $88
	db $00
	db $00
	db $00
	db $00
	db $0C
	db $C0
	db $00
	db $1B
	db $60
	db $00
	db $1B
	db $60
	db $00
	db $5F
	db $E8
	db $00
	db $EC
	db $DC
	db $01
	db $F3
	db $3E
	db $01
	db $FF
	db $FE
	db $03
	db $FF
	db $FF
	db $03
	db $FF
	db $FF
	db $03
	db $77
	db $7F
	db $00
	db $70
	db $77
	db $00
	db $00
	db $07
	db $60
	db $06
	db $00
	db $F0
	db $0F
	db $00
	db $7C
	db $1E
	db $00
	db $1E
	db $78
	db $00
	db $06
	db $60
	db $00
	db $01
	db $80
	db $00
	db $05
	db $A0
	db $00
	db $09
	db $90
	db $00
	db $0C
	db $30
	db $00
	db $1F
	db $F8
	db $00
	db $39
	db $9C
	db $00
	db $32
	db $4C
	db $00
	db $72
	db $4E
	db $00
	db $79
	db $9E
	db $00
	db $7F
	db $FE
	db $00
	db $37
	db $74
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3C
	db $03
	db $C0
	db $3F
	db $9F
	db $C0
	db $00
	db $60
	db $00
	db $01
	db $68
	db $00
	db $02
	db $64
	db $00
	db $03
	db $0C
	db $00
	db $07
	db $FE
	db $00
	db $0E
	db $67
	db $00
	db $0C
	db $93
	db $00
	db $1C
	db $93
	db $80
	db $1E
	db $67
	db $80
	db $1F
	db $FF
	db $80
	db $0B
	db $BB
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
	db $DB
	db $80
	db $0F
	db $99
	db $F0
	db $0E
	db $18
	db $70
	db $0C
	db $C3
	db $30
	db $01
	db $FF
	db $80
	db $03
	db $99
	db $C0
	db $03
	db $24
	db $C0
	db $07
	db $24
	db $E0
	db $07
	db $99
	db $E0
	db $07
	db $FF
	db $E0
	db $01
	db $DD
	db $C0
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
	db $E0
	db $0F
	db $01
	db $FC
	db $FF
	db $00
	db $03
	db $00
	db $00
	db $0B
	db $40
	db $00
	db $13
	db $20
	db $00
	db $18
	db $60
	db $00
	db $3F
	db $F0
	db $00
	db $73
	db $38
	db $00
	db $64
	db $98
	db $00
	db $E4
	db $9C
	db $00
	db $F3
	db $3C
	db $00
	db $FF
	db $FC
	db $00
	db $77
	db $70
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $84
	db $00
	db $01
	db $CE
	db $00
	db $07
	db $E4
	db $00
	db $0D
	db $C0
	db $00
	db $04
	db $80
	db $00
	db $10
	db $10
	db $00
	db $38
	db $B9
	db $00
	db $10
	db $10
	db $00
	db $08
	db $80
	db $00
	db $1C
	db $00
	db $00
	db $3E
	db $04
	db $00
	db $1C
	db $0E
	db $00
	db $08
	db $84
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
	db $02
	db $20
	db $00
	db $07
	db $70
	db $00
	db $0F
	db $A0
	db $00
	db $07
	db $00
	db $00
	db $02
	db $0A
	db $00
	db $00
	db $07
	db $00
	db $08
	db $AF
	db $80
	db $00
	db $07
	db $00
	db $00
	db $2A
	db $00
	db $00
	db $70
	db $00
	db $00
	db $20
	db $00
	db $04
	db $00
	db $00
	db $00
	db $00
	db $80
	db $00
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $08
	db $00
	db $03
	db $80
	db $00
	db $01
	db $01
	db $00
	db $00
	db $03
	db $80
	db $00
	db $09
	db $00
	db $00
	db $40
	db $40
	db $04
	db $E8
	db $E0
	db $00
	db $40
	db $40
	db $00
	db $29
	db $00
	db $00
	db $1D
	db $80
	db $00
	db $3E
	db $00
	db $00
	db $1C
	db $00
	db $02
	db $08
	db $00
	db $00
	db $00
	db $10
	db $00
	db $08
	db $00
	db $00
	db $02
	db $00
	db $00
	db $80
	db $20
	db $00
	db $00
	db $70
	db $00
	db $02
	db $F8
	db $00
	db $07
	db $70
	db $00
	db $4A
	db $20
	db $00
	db $E0
	db $00
	db $01
	db $F2
	db $88
	db $00
	db $E0
	db $00
	db $00
	db $50
	db $20
	db $00
	db $38
	db $70
	db $00
	db $12
	db $F8
	db $00
	db $07
	db $70
	db $00
	db $02
	db $20
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
	db $80
	db $00
	db $03
	db $C0
	db $00
	db $06
	db $60
	db $00
	db $0C
	db $30
	db $00
	db $1E
	db $78
	db $00
	db $06
	db $60
	db $00
	db $06
	db $60
	db $00
	db $07
	db $E0
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
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $B0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $10
	db $20
	db $40
	db $F0
	db $10
	db $18
	db $2C
	db $3E
	db $07
	db $0C
	db $08
	db $18
	db $18
	db $2C
	db $2F
	db $BE
	db $C0
	db $60
	db $20
	db $20
	db $30
	db $7E
	db $E3
	db $C1
	db $7F
	db $5F
	db $5F
	db $DF
	db $9F
	db $9F
	db $9C
	db $83
	db $01
	db $02
	db $89
	db $2B
	db $75
	db $70
	db $0F
	db $B8
	db $1C
	db $C1
	db $F1
	db $E5
	db $CE
	db $01
	db $FF
	db $19
	db $C1
	db $41
	db $63
	db $FE
	db $F8
	db $40
	db $00
	db $3C
	db $9F
	db $C8
	db $66
	db $43
	db $33
	db $3C
	db $74
	db $CB
	db $7F
	db $72
	db $7A
	db $71
	db $7F
	db $60
	db $30
	db $9F
	db $F0
	db $B0
	db $30
	db $B0
	db $F3
	db $33
	db $19
	db $FE
	db $BC
	db $8C
	db $78
	db $F8
	db $50
	db $EC
	db $C2
	db $13
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $D5
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $0F
	db $FE
	db $54
	db $00
	db $00
	db $00
	db $0E
	db $FB
	db $F0
	db $A7
	db $1E
	db $00
	db $00
	db $00
	db $00
	db $80
	db $3C
	db $EF
	db $A4
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F9
	db $2F
	db $02
	db $00
	db $00
	db $00
	db $01
	db $07
	db $C0
	db $E0
	db $80
	db $00
	db $00
	db $00
	db $FF
	db $4F
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $AF
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $07
	db $1E
	db $00
	db $00
	db $00
	db $00
	db $00
	db $E0
	db $FC
	db $A0
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $70
	db $00
	db $00
	db $00
	db $00
	db $03
	db $0F
	db $01
	db $F8
	db $5F
	db $0A
	db $00
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $00
	db $F0
	db $7F
	db $2F
	db $05
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $5F
	db $08
	db $00
	db $00
	db $00
	db $00
	db $0E
	db $FF
	db $AF
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $E3
	db $FF
	db $AA
	db $00
	db $00
	db $00
	db $00
	db $00
	db $F8
	db $AE
	db $80
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $E0
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $A0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $7E
	db $D7
	db $02
	db $00
	db $00
	db $00
	db $3E
	db $E3
	db $1C
	db $FF
	db $A8
	db $00
	db $00
	db $00
	db $1C
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $80
	db $FF
	db $AA
	db $00
	db $00
	db $00
	db $00
	db $06
	db $01
	db $00
	db $00
	db $00
	db $01
	db $7F
	db $03
	db $7F
	db $D4
	db $00
	db $00
	db $39
	db $FF
	db $55
	db $FE
	db $FF
	db $AA
	db $00
	db $00
	db $80
	db $E0
	db $70
	db $00
	db $E0
	db $FF
	db $50
	db $00
	db $00
	db $00
	db $00
	db $D5
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $20
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $50
	db $A0
	db $90
	db $00
	db $00
	db $00
	db $00
	db $00
	db $08
	db $00
	db $00
	db $00
	db $00
	db $10
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $80
	db $00
	db $00
	db $00
	db $10
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $08
	db $1C
	db $08
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
	db $80
	db $00
	db $00
	db $00
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $50
	db $90
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $08
	db $1C
	db $08
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $04
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $40
	db $E0
	db $40
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $10
	db $00
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $36
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $E0
	db $F0
	db $70
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $38
	db $7C
	db $64
	db $68
	db $70
	db $00
	db $00
	db $01
	db $03
	db $07
	db $07
	db $07
	db $07
	db $00
	db $00
	db $98
	db $CC
	db $EE
	db $EE
	db $EE
	db $FE
	db $70
	db $38
	db $3E
	db $1F
	db $1F
	db $0F
	db $07
	db $01
	db $07
	db $03
	db $0C
	db $FB
	db $FB
	db $FB
	db $FB
	db $B5
	db $E6
	db $C2
	db $C1
	db $C7
	db $CE
	db $CC
	db $C1
	db $E1
	db $00
	db $00
	db $E0
	db $F8
	db $0C
	db $04
	db $04
	db $0A
	db $48
	db $30
	db $30
	db $38
	db $1C
	db $0E
	db $06
	db $06
	db $73
	db $FF
	db $FB
	db $FB
	db $FB
	db $70
	db $00
	db $00
	db $8E
	db $8E
	db $84
	db $84
	db $00
	db $00
	db $00
	db $00
	db $07
	db $07
	db $07
	db $07
	db $07
	db $07
	db $47
	db $00
	db $00
	db $07
	db $07
	db $00
	db $78
	db $FE
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $03
	db $07
	db $04
	db $00
	db $78
	db $07
	db $00
	db $F0
	db $7C
	db $BE
	db $DE
	db $00
	db $FC
	db $43
	db $7C
	db $DF
	db $47
	db $7E
	db $0F
	db $00
	db $00
	db $00
	db $F8
	db $04
	db $F4
	db $B4
	db $F4
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $02
	db $09
	db $0B
	db $0B
	db $0F
	db $00
	db $07
	db $60
	db $8F
	db $DF
	db $DF
	db $BF
	db $BF
	db $7C
	db $D3
	db $2F
	db $EC
	db $34
	db $5B
	db $5B
	db $19
	db $D9
	db $99
	db $1B
	db $32
	db $00
	db $00
	db $43
	db $A7
	db $B6
	db $B5
	db $24
	db $6E
	db $00
	db $00
	db $00
	db $00
	db $00
	db $C0
	db $77
	db $14
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $58
	db $34
	db $00
	db $36
	db $00
	db $00
	db $36
	db $00
	db $00
	db $00
	db $60
	db $E0
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $78
	db $FC
	db $FC
	db $FC
	db $FC
	db $58
	db $00
	db $00
	db $00
	db $00
	db $00
	db $78
	db $FC
	db $FC
	db $00
	db $3C
	db $7E
	db $7E
	db $7C
	db $7E
	db $3C
	db $00
	db $20
	db $31
	db $13
	db $1E
	db $CC
	db $6C
	db $3C
	db $19
	db $FC
	db $7C
	db $78
	db $00
	db $00
	db $78
	db $FC
	db $7C
	db $00
	db $1E
	db $3F
	db $3E
	db $3E
	db $3F
	db $1E
	db $00
	db $0F
	db $0E
	db $06
	db $E6
	db $FE
	db $1E
	db $06
	db $06
	db $7C
	db $FC
	db $78
	db $00
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $02
	db $00
	db $0C
	db $0C
	db $00
	db $00
	db $FF
	db $1F
	db $3F
	db $00
	db $7F
	db $7F
	db $18
	db $00
	db $FF
	db $FE
	db $FF
	db $00
	db $FF
	db $FF
	db $18
	db $00
	db $00
	db $80
	db $40
	db $00
	db $D0
	db $D0
	db $00
	db $07
	db $18
	db $2B
	db $3F
	db $0F
	db $37
	db $17
	db $07
	db $E7
	db $D9
	db $DA
	db $5F
	db $66
	db $F5
	db $F5
	db $BA
	db $E7
	db $9B
	db $5B
	db $FA
	db $66
	db $AF
	db $AF
	db $5D
	db $E0
	db $18
	db $D4
	db $FC
	db $F0
	db $EC
	db $E8
	db $E0
	db $00
	db $02
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
	db $41
	db $1C
	db $0F
	db $8F
	db $00
	db $8F
	db $8F
	db $8F
	db $C2
	db $1F
	db $FE
	db $FD
	db $00
	db $FD
	db $FD
	db $FD
	db $00
	db $40
	db $80
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
label_C350:
	JR label_C35E	;[label_C35E]
function_C352:
	JP label_C4AB	;[label_C4AB]
	db $C3
	db $E5
	db $C4
	db $C3
	db $06
	db $C5
	db $C3
	db $2D
	db $C5
label_C35E:
	LD HL,data_D2C4	;[data_D2C4]
	LD (HL),$00
	CP $10
	JR C,label_C368	;[label_C368]
	INC (HL)
label_C368:
	AND $07
	LD (data_C4A9),A	;[data_C4A9]
	CP $02
	JR NZ,label_C3B0	;[label_C3B0]
	CALL function_C475	;[function_C475]
	CALL function_C546	;[function_C546]
	LD B,$2D
label_C379:
	PUSH BC
	LD HL,Sprite0_Col	;[Sprite0_Col]
	LD A,(HL)
	XOR $05
	LD (HL),A
	EX AF,AF'
	INC HL
	LD A,(HL)
	CP $02
	JR NZ,label_C38C	;[label_C38C]
	EX AF,AF'
	LD (data_DDA1),A	;[data_DDA1]
label_C38C:
	LD A,(data_C3A2)	;[data_C3A2]
	XOR $05
	LD (data_C3A2),A	;[data_C3A2]
	CALL UpdateGame	;[UpdateGame]
	CALL DrawText	;[DrawText]
	db $13
	db $01
	db $16
	db $01
	db $0E
	db $15
	db $01
	db $10
data_C3A2:
	db $02
	db $20
	db $20
	db $15
	db $00
	db $FF
	LD A,$0F
	CALL function_D7C0	;[function_D7C0]
	POP BC
	DJNZ label_C379	;[label_C379]
label_C3B0:
	CALL function_C475	;[function_C475]
	CALL function_C546	;[function_C546]
	LD A,(Sprite0_OnTransport)	;[Sprite0_OnTransport]
	CP $02
	JR Z,label_C3C8	;[label_C3C8]
	LD HL,Sprite4_XPixelPos	;[Sprite4_XPixelPos]
	LD DE,Sprite5_XPixelPos	;[Sprite5_XPixelPos]
	LD BC,$0005	;[START + 5]
	LDIR
label_C3C8:
	LD A,$04
	LD (data_9C43),A	;[data_9C43]
	LD A,(data_C4A9)	;[data_C4A9]
	DEC A
	JR Z,label_C3E1	;[label_C3E1]
	LD HL,$DF40	;[ScanlineTable + 354]
	LD (Sprite0_SpriteAddr2),HL	;[Sprite0_SpriteAddr2]
	LD A,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	AND $F8
	LD (Sprite0_XPixelPos),A	;[Sprite0_XPixelPos]
label_C3E1:
	LD A,(data_D2CC)	;[data_D2CC]
	AND A
	JR NZ,label_C3EA	;[label_C3EA]
	CALL label_5E29	;[label_5E29]
label_C3EA:
	LD A,$13
	CALL function_D7C0	;[function_D7C0]
	LD HL,$DD3D	;[Sprite1 + 5]
	LD DE,$C498	;[label_C486 + 18]
	LD B,$04
label_C3F7:
	PUSH BC
	LD BC,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,C
	AND $FE
	LD (HL),A
	INC HL
	LD A,B
	OR $01
	LD (HL),A
	INC HL
	LD (HL),$C8
	INC HL
	LD (HL),$BE
	INC HL
	LD (HL),$07
	INC HL
	LD BC,$0004	;[START + 4]
	ADD HL,BC
	LD A,(DE)
	INC DE
	LD (HL),A
	INC HL
	INC HL
	PUSH DE
	LD DE,$C4A0	;[label_C486 + 26]
	LD B,$09
label_C41E:
	INC HL
	LD A,(DE)
	INC DE
	CP $FE
	JR NZ,label_C42B	;[label_C42B]
	EXX
	POP DE
	LD A,(DE)
	INC DE
	PUSH DE
	EXX
label_C42B:
	CP $FF
	JR Z,label_C430	;[label_C430]
	LD (HL),A
label_C430:
	DJNZ label_C41E	;[label_C41E]
	POP DE
	LD BC,$000C	;[ERROR_1 + 4]
	ADD HL,BC
	POP BC
	DJNZ label_C3F7	;[label_C3F7]
	LD A,$09
	LD (data_A41B),A	;[data_A41B]
	LD B,$50
label_C441:
	PUSH BC
	CALL UpdateGame	;[UpdateGame]
	CALL function_A01B	;[function_A01B]
	POP BC
	DJNZ label_C441	;[label_C441]
	CALL function_C475	;[function_C475]
	CALL UpdateGame	;[UpdateGame]
	LD B,$32
label_C453:
	HALT
	DJNZ label_C453	;[label_C453]
	CALL function_C546	;[function_C546]
	LD HL,data_D2CC	;[data_D2CC]
	XOR A
	CP (HL)
	JR NZ,label_C462	;[label_C462]
	POP HL
	RET
label_C462:
	DEC (HL)
	INC HL
	DEC A
	LD (HL),A
	LD A,$08
	INC HL
	OR (HL)
	LD (HL),A
	LD HL,PlayerSpriteData	;[PlayerSpriteData]
	LD (Sprite0_SpriteAddr2),HL	;[Sprite0_SpriteAddr2]
	POP HL
	JP function_A410	;[function_A410]
function_C475:
	LD A,(data_9C43)	;[data_9C43]
	CP $03
	JR NZ,label_C482	;[label_C482]
	LD HL,$0202	;[label_0140 + 194]
	LD (data_DDAB),HL	;[data_DDAB]
label_C482:
	LD B,A
	LD HL,$DD3D	;[Sprite1 + 5]
label_C486:
	XOR A
	LD (HL),A
	INC HL
	LD (HL),A
	INC HL
	LD DE,$DF40	;[ScanlineTable + 354]
	LD (HL),E
	INC HL
	LD (HL),D
	LD DE,$001D	;[TEST_CHAR + 1]
	ADD HL,DE
	DJNZ label_C486	;[label_C486]
	RET
	db $0A
	db $04
	db $06
	db $0C
	db $09
	db $14
	db $05
	db $1C
	db $02
	db $02
	db $FF
	db $FE
	db $02
	db $14
	db $08
	db $08
	db $00
data_C4A9:
	db $15
	db $0F
label_C4AB:
	PUSH DE
	LD (data_C4A9),BC	;[data_C4A9]
	LD HL,$D2DE	;[data_D2DC + 2]
	LD DE,$0303	;[K_NEW + 18]
label_C4B6:
	LD A,(HL)	;Item No
	INC HL
	PUSH HL
	LD H,$07
	CP $80
	JR C,label_C4C1	;[label_C4C1]
	LD H,$02
label_C4C1:
	PUSH HL
	AND $7F
	CALL GetItemImagePtr_Thunk	;[GetItemImagePtr_Thunk]
	POP AF
	CALL DrawItem	;[DrawItem]
	POP HL
	INC C
	INC C
	DEC E
	JR NZ,label_C4B6	;[label_C4B6]
	LD E,$03
	LD A,C
	SUB $06
	LD C,A
	INC B
	INC B
	DEC D
	JR NZ,label_C4B6	;[label_C4B6]
	LD B,$0D
label_C4DE:
	CALL function_C506	;[function_C506]
	DJNZ label_C4DE	;[label_C4DE]
	POP DE
	RET
label_C4E5:
	PUSH HL
	PUSH BC
	PUSH DE
	LD A,E
	LD B,$FE
label_C4EB:
	INC B
	INC B
	SUB $03
	JR NC,label_C4EB	;[label_C4EB]
	ADD A,$03
	RLCA
	LD C,A
	LD HL,(data_C4A9)	;[data_C4A9]
	ADD HL,BC
	LD B,H
	LD C,L
	LD HL,$DF40	;[ScanlineTable + 354]
	LD A,D
	CALL DrawItem	;[DrawItem]
	POP DE
	POP BC
	POP HL
	RET
function_C506:
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(RandomNo1)	;[RandomNo1]
	AND $3F
label_C50E:
	SUB $09
	JR NC,label_C50E	;[label_C50E]
	ADD A,$09
	LD E,A
	LD D,$00
	LD HL,$D2DE	;[data_D2DC + 2]
	ADD HL,DE
	LD A,(HL)
	CP $80
	RET C
	LD A,(data_DAC1)	;[data_DAC1]
	AND $3F
label_C524:
	SUB $05
	JR NC,label_C524	;[label_C524]
	ADD A,$07
	LD D,A
	JR label_C4E5	;[label_C4E5]
	db $C5
	db $E5
	db $D5
	db $50
	db $21
	db $00
	db $58
	db $01
	db $00
	db $03
	db $7E
	db $AA
	db $77
	db $23
	db $0B
	db $78
	db $B1
	db $20
	db $F7
	db $D1
	db $E1
	db $C1
	db $C9
function_C544:
	JR label_C54F	;[label_C54F]
function_C546:
	JP function_C8DD	;[function_C8DD]
GetItemImagePtr_Thunk:	;A - item no, ptr returned in HL
	JP GetItemImagePtr	;[GetItemImagePtr]
	db $C3
	db $A6
	db $D2
label_C54F:
	CALL UpdateGame	;[UpdateGame]
	LD A,$07
	LD (Sprite5_InkCol),A	;[Sprite5_InkCol]
	JP label_C5BD	;[label_C5BD]
Read_Game_Controls:
	LD BC,$7FFE	;[Screens + 2766]
	IN A,(C)
	BIT 0,A
	JR NZ,PauseNotPressed	;[PauseNotPressed]
	CALL Pause_Game	;[Pause_Game]
PauseNotPressed:
	LD A,$00
	CP $01
	JR NZ,Read_Key_Controls	;[Read_Key_Controls]
	LD BC,$001F	;[TEST_CHAR + 3]
	IN A,(C)
	CP $00
	JR Z,Read_Key_Controls	;[Read_Key_Controls]
	RET
Read_Key_Controls:
	LD A,$00
	LD BC,$FBFE	;[SmallPlatform3Data + 3670]
	IN D,(C)
	BIT 4,D
	JR NZ,label_C583	;[label_C583]
	ADD A,$02
label_C583:
	LD BC,$DFFE	;[SpriteEraseLoop + 109]
	IN D,(C)
	BIT 4,D
	JR NZ,label_C58E	;[label_C58E]
	ADD A,$04
label_C58E:
	LD BC,$DFFE	;[SpriteEraseLoop + 109]
	IN D,(C)
	BIT 3,D
	JR NZ,label_C599	;[label_C599]
	ADD A,$08
label_C599:
	LD BC,$DFFE	;[SpriteEraseLoop + 109]
	IN D,(C)
	BIT 2,D
	JR NZ,label_C5A4	;[label_C5A4]
	ADD A,$01
label_C5A4:
	LD BC,$DFFE	;[SpriteEraseLoop + 109]
	IN D,(C)
	BIT 0,D
	RET NZ
	ADD A,$10
	RET
Pause_Game:
	IN A,(C)	;Loop until key released
	BIT 0,A
	JR Z,Pause_Game	;[Pause_Game]
Pause_Loop:
	CALL PauseNotPressed	;[PauseNotPressed]
	CP $00
	JR Z,Pause_Loop	;[Pause_Loop]
	RET
label_C5BD:
	LD IX,Sprite0	;[Sprite0]
	XOR A
	LD (data_DD27),A	;[data_DD27]
	LD BC,$FDFE	;[SmallPlatform3Data + 4182]
	IN A,(C)
	AND $1F
	JR NZ,label_C5D2	;[label_C5D2]
	POP HL
	JP label_5E29	;[label_5E29]
label_C5D2:
	CALL Read_Game_Controls	;[Read_Game_Controls]
	BIT 4,A
	JR Z,label_C5DD	;[label_C5DD]
	LD HL,data_DD27	;[data_DD27]
	INC (HL)
label_C5DD:
	AND $0F
	LD C,A
	CPL
	AND $03
	JR NZ,label_C5E9	;[label_C5E9]
	LD A,C
	AND $FC
	LD C,A
label_C5E9:
	LD A,C
	CPL
	AND $0C
	JR NZ,label_C5F3	;[label_C5F3]
	LD A,C
	AND $F3
	LD C,A
label_C5F3:
	LD A,C
	CPL
	AND $09
	JR NZ,label_C5FB	;[label_C5FB]
	SET 6,C
label_C5FB:
	LD A,C
	CPL
	AND $0A
	JR NZ,label_C603	;[label_C603]
	SET 7,C
label_C603:
	LD A,C
	CPL
	AND $05
	JR NZ,label_C60B	;[label_C60B]
	SET 5,C
label_C60B:
	LD A,C
	CPL
	AND $06
	JR NZ,label_C613	;[label_C613]
	SET 4,C
label_C613:
	LD A,C
	LD (CurrPlayerControl),A	;[CurrPlayerControl]
	CP $00
	JR Z,label_C625	;[label_C625]
	LD (LastCtrlPlayerCtrl),A	;[LastCtrlPlayerCtrl]
	AND $03
	JR Z,label_C625	;[label_C625]
	LD (data_DD2B),A	;[data_DD2B]
label_C625:
	LD A,(Sprite0_OnTransport)	;[Sprite0_OnTransport]
	DEC A
	JP Z,label_C761	;[label_C761]
	DEC A
	JP Z,label_C967	;[label_C967]
	CALL function_D2F0	;[function_D2F0]
	LD C,A
	LD HL,data_DD28	;[data_DD28]
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	BIT 0,A
	JR Z,label_C682	;[label_C682]
	BIT 0,C
	JR NZ,label_C64A	;[label_C64A]
	LD A,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	ADD A,$02
	LD (Sprite0_XPixelPos),A	;[Sprite0_XPixelPos]
label_C64A:
	INC (HL)
	LD A,(HL)
	CP $03
	JP C,label_C6D6	;[label_C6D6]
	LD (HL),$00
	LD A,(data_DD26)	;[data_DD26]
	CP $00
	JR Z,label_C66D	;[label_C66D]
	DEC A
	LD (data_DD26),A	;[data_DD26]
	LD (IX+$0D),$00
	CP $00
	JR Z,label_C66D	;[label_C66D]
	LD HL,$E674	;[PlayerSpriteData + 1536]
	DEC A
	LD E,A
	JR label_C6C7	;[label_C6C7]
label_C66D:
	CALL function_C6EE	;[function_C6EE]
	LD A,(Sprite0_AnimFrame)	;[Sprite0_AnimFrame]
	INC A
	CP $04
	JR C,label_C679	;[label_C679]
	XOR A
label_C679:
	LD (Sprite0_AnimFrame),A	;[Sprite0_AnimFrame]
	LD HL,PlayerSpriteData	;[PlayerSpriteData]
	LD E,A
	JR label_C6C7	;[label_C6C7]
label_C682:
	BIT 1,A
	JR Z,label_C6D6	;[label_C6D6]
	BIT 1,C
	JR NZ,label_C692	;[label_C692]
	LD A,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	SUB $02
	LD (Sprite0_XPixelPos),A	;[Sprite0_XPixelPos]
label_C692:
	INC (HL)
	LD A,(HL)
	CP $03
	JR C,label_C6D6	;[label_C6D6]
	LD (HL),$00
	LD A,(data_DD26)	;[data_DD26]
	CP $04
	JR Z,label_C6B4	;[label_C6B4]
	INC A
	LD (data_DD26),A	;[data_DD26]
	LD (IX+$0D),$00
	CP $04
	JR Z,label_C6B4	;[label_C6B4]
	LD HL,$E674	;[PlayerSpriteData + 1536]
	DEC A
	LD E,A
	JR label_C6C7	;[label_C6C7]
label_C6B4:
	CALL function_C6EE	;[function_C6EE]
	LD A,(Sprite0_AnimFrame)	;[Sprite0_AnimFrame]
	INC A
	CP $04
	JR C,label_C6C0	;[label_C6C0]
	XOR A
label_C6C0:
	LD (Sprite0_AnimFrame),A	;[Sprite0_AnimFrame]
	LD HL,$E374	;[PlayerSpriteData + 768]
	LD E,A
label_C6C7:
	PUSH HL
	LD D,$00
	LD HL,$00C0	;[TOKENS + 43]
	CALL HL_HLxDE	;[HL_HLxDE]
	EX DE,HL
	POP HL
	ADD HL,DE
	LD (Sprite0_SpriteAddr2),HL	;[Sprite0_SpriteAddr2]
label_C6D6:
	LD HL,(FlyingTransportCoords)	;[FlyingTransportCoords]	;load transport coords
	LD DE,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]	;load player coords
	LD A,H
	CP D
	JR NZ,label_C700	;[label_C700]
	LD A,L
	CP E
	JP NZ,label_C700	;[label_C700]
	LD HL,CurrPlayerControl	;[CurrPlayerControl]
	RES 3,(HL)
	JP label_C85A	;[label_C85A]
function_C6EE:
	PUSH AF
	LD HL,data_C6FF	;[data_C6FF]
	LD A,(data_C6FF)	;[data_C6FF]
	XOR $01
	LD (data_C6FF),A	;[data_C6FF]
	CALL function_D7C0	;[function_D7C0]
	POP AF
	RET
data_C6FF:
	db $14
label_C700:
	CALL function_D2F4	;[function_D2F4]
	LD C,A
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,L
	SUB $08
	AND $1F
	JR NZ,label_C728	;[label_C728]
	LD A,H
label_C70F:
	SUB $03
	JR NC,label_C70F	;[label_C70F]
	CP $FD
	JR NZ,label_C728	;[label_C728]
	LD HL,(data_D3BF)	;[data_D3BF]
	INC HL
	ADD HL,DE
	LD A,(HL)
	CP $64
	JR NZ,label_C728	;[label_C728]
	LD A,$01
	LD (Sprite0_OnTransport),A	;[Sprite0_OnTransport]
	JR label_C761	;[label_C761]
label_C728:
	BIT 2,C
	JR NZ,label_C791	;[label_C791]
	LD HL,data_DD29	;[data_DD29]
	XOR A
	CP (HL)
	JR NZ,label_C738	;[label_C738]
	LD A,$06
	LD (data_A41B),A	;[data_A41B]
label_C738:
	LD A,(HL)
	CP $10
	JR Z,label_C73E	;[label_C73E]
	INC A
label_C73E:
	LD (HL),A
	LD E,A
	LD D,$00
	LD HL,$C751	;[label_C73E + 19]
	ADD HL,DE
	DEC HL
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	SUB (HL)
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
	JP label_C79F	;[label_C79F]
	db $01
	db $00
	db $01
	db $00
	db $01
	db $02
	db $01
	db $02
	db $01
	db $02
	db $02
	db $03
	db $02
	db $03
	db $03
	db $04
label_C761:
	LD HL,CurrPlayerControl	;[CurrPlayerControl]
	RES 3,(HL)
	CALL function_D2F4	;[function_D2F4]
	BIT 3,A
	JR NZ,label_C775	;[label_C775]
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	ADD A,$02
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
label_C775:
	LD HL,$BF88	;[Enemy_SpriteData + 4032]
	LD (Sprite0_SpriteAddr2),HL	;[Sprite0_SpriteAddr2]
	CALL function_D2F0	;[function_D2F0]
	AND $03
	CP $03
	JP Z,label_C85A	;[label_C85A]
	XOR A
	LD (Sprite0_OnTransport),A	;[Sprite0_OnTransport]
	LD A,$02
	LD (data_DD28),A	;[data_DD28]
	JP label_C85A	;[label_C85A]
label_C791:
	LD HL,data_DD29	;[data_DD29]
	XOR A
	CP (HL)
	JR Z,label_C79F	;[label_C79F]
	LD A,$07
	LD (data_A41B),A	;[data_A41B]
	XOR A
	LD (HL),A
label_C79F:
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	CP $04	;check for 'build platform' control
	JP NZ,label_C856	;[label_C856]
	LD A,(DownHeld)	;[DownHeld]
	CP $00
	JP NZ,label_C85A	;[label_C85A]	;still held - skip
	LD A,(data_D2CE)	;[data_D2CE]
	CP $00	;check if zero - then skip
	JP Z,label_C85A	;[label_C85A]
	LD A,$01	;set 'down held' so we don't hammer this loop
	LD (DownHeld),A	;[DownHeld]
	CALL function_D2F4	;[function_D2F4]
	LD HL,(data_D3BF)	;[data_D3BF]
	LD C,A
	LD DE,NEXT_CHAR	;[NEXT_CHAR]
	ADD HL,DE
	ADD HL,DE
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	INC A
	AND $07
	JR Z,label_C7D1	;[label_C7D1]
	ADD HL,DE
label_C7D1:
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	CP $17
	JR NC,label_C7DF	;[label_C7DF]
	LD A,$0F
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
	JR label_C7ED	;[label_C7ED]
label_C7DF:
	BIT 6,(HL)
	JR Z,label_C7ED	;[label_C7ED]
	INC HL
	BIT 6,(HL)
	JR Z,label_C7ED	;[label_C7ED]
	INC HL
	BIT 6,(HL)
	JR NZ,label_C7FD	;[label_C7FD]
label_C7ED:
	BIT 3,C
	JR NZ,label_C85A	;[label_C85A]
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	INC A
	AND $F8
	ADD A,$08
	DEC A
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
label_C7FD:
	LD A,(HL)
	AND $7F
	CP $64
	JR Z,label_C85A	;[label_C85A]
	LD HL,$DBBC	;[PlatformStates + 1]
	LD B,$0C
	XOR A
label_C80A:
	CP (HL)
	JR Z,label_C815	;[label_C815]
	INC HL
	INC HL
	INC HL
	INC HL
	DJNZ label_C80A	;[label_C80A]
	JR label_C85A	;[label_C85A]
label_C815:
	DEC HL
	LD A,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	ADD A,$04
	AND $F8
	RRCA
	RRCA
	RRCA
	LD C,A
	LD (HL),A
	INC HL
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	LD E,A
	LD A,$D6
	SUB E
	AND $F8
	RRCA
	RRCA
	RRCA
	LD B,A
	LD (HL),A
	INC HL
	INC HL
	LD A,(RandomNo1)	;[RandomNo1]
	AND $03
	ADD A,$05
	LD (HL),A
	LD L,$00
	LD A,$00
	LD D,$04
label_C841:
	CALL DrawPlatform	;[DrawPlatform]
	INC L
	DEC D
	JR NZ,label_C841	;[label_C841]
	LD A,$01
	LD C,$02
	CALL function_D41F	;[function_D41F]
	LD A,$08
	LD (data_A41B),A	;[data_A41B]
	JR label_C85A	;[label_C85A]
label_C856:
	XOR A
	LD (DownHeld),A	;[DownHeld]
label_C85A:
	LD A,(data_DD2A)	;[data_DD2A]
	CP $00
	JR NZ,label_C8A4	;[label_C8A4]
	LD A,(data_DD27)	;[data_DD27]
	CP $00
	JP Z,label_C8F4	;[label_C8F4]
	LD A,(data_D2CF)	;[data_D2CF]
	CP $00
	JP Z,label_C8F4	;[label_C8F4]
	LD A,(data_A41D)	;[data_A41D]
	CP $00
	JR Z,label_C87F	;[label_C87F]
	LD A,(data_A41F)	;[data_A41F]
	CP $F7
	JR Z,label_C8F4	;[label_C8F4]
label_C87F:
	LD A,$05
	LD (data_A41B),A	;[data_A41B]
	LD A,$02
	LD C,$01
	CALL function_D41F	;[function_D41F]
	LD A,(data_DD2B)	;[data_DD2B]
	LD (data_DD2A),A	;[data_DD2A]
	LD HL,$E8B4	;[PlayerSpriteData + 2112]
	BIT 0,A
	JR NZ,label_C89B	;[label_C89B]
	LD HL,$E974	;[PlayerSpriteData + 2304]
label_C89B:
	LD (Sprite5_SpriteAddr2),HL	;[Sprite5_SpriteAddr2]
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD (Sprite5_XPixelPos),HL	;[Sprite5_XPixelPos]
label_C8A4:
	LD IX,Sprite5	;[Sprite5]
	LD B,$03
	LD A,(data_DD2A)	;[data_DD2A]
	LD C,A
label_C8AE:
	PUSH BC
	LD A,(Sprite5_XPixelPos)	;[Sprite5_XPixelPos]
	AND $07
	LD A,$00
	CALL Z,function_D2F0	;[function_D2F0]
	POP BC
	AND C
	JR NZ,label_C8D8	;[label_C8D8]
	LD A,(data_DD2A)	;[data_DD2A]
	CP $01
	LD A,(Sprite5_XPixelPos)	;[Sprite5_XPixelPos]
	JR Z,label_C8CB	;[label_C8CB]
	SUB $02
	JR label_C8CD	;[label_C8CD]
label_C8CB:
	ADD A,$02
label_C8CD:
	LD (Sprite5_XPixelPos),A	;[Sprite5_XPixelPos]
	CP $F2
	JR NC,label_C8D8	;[label_C8D8]
	DJNZ label_C8AE	;[label_C8AE]
	JR label_C8F4	;[label_C8F4]
label_C8D8:
	CALL function_C8DD	;[function_C8DD]
	JR label_C8F4	;[label_C8F4]
function_C8DD:
	LD HL,$0F00	;[COPY_L_1 + 3]
	LD (Sprite5_XPixelPos),HL	;[Sprite5_XPixelPos]
	LD HL,$DF40	;[ScanlineTable + 354]
	LD (Sprite5_SpriteAddr2),HL	;[Sprite5_SpriteAddr2]
	XOR A
	LD (data_DD2A),A	;[data_DD2A]
	LD HL,START
	LD (data_DD2D),HL	;[data_DD2D]
	RET
label_C8F4:
	LD HL,Sprite0_XPixelPos	;[Sprite0_XPixelPos]
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	LD C,A
	LD A,(HL)
	SUB $F0
	CP $04
	JR NC,label_C90D	;[label_C90D]
	BIT 0,C
	JR Z,label_C90D	;[label_C90D]
	LD (HL),$00
	LD DE,$0001	;[START + 1]
	JR label_C936	;[label_C936]
label_C90D:
	LD A,(HL)
	ADD A,$02
	CP $04
	JR NC,label_C91F	;[label_C91F]
	BIT 1,C
	JR Z,label_C91F	;[label_C91F]
	LD (HL),$F0
	LD DE,RAMTOP	;[RAMTOP]
	JR label_C936	;[label_C936]
label_C91F:
	INC HL
	LD A,(HL)
	CP $0E
	JR NC,label_C92C	;[label_C92C]
	LD (HL),$8F
	LD DE,PRINT_A_1	;[PRINT_A_1]
	JR label_C936	;[label_C936]
label_C92C:
	CP $90
	JP C,label_CB58	;[label_CB58]
	LD (HL),$0F
	LD DE,$FFF0	;[SmallPlatform3Data + 4680]
label_C936:
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	ADD HL,DE
	LD (CurrentScreenNo),HL	;[CurrentScreenNo]
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	INC A
	AND $F8
	DEC A
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
	CALL function_C8DD	;[function_C8DD]
	LD A,$00
	RET
function_C94D:
	PUSH AF
	PUSH HL
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,H
	SUB $08
	LD H,A
	LD (Sprite4_XPixelPos),HL	;[Sprite4_XPixelPos]
	LD HL,Enemy_SpriteData	;[Enemy_SpriteData]
	LD (data_DD9F),HL	;[data_DD9F]
	LD A,$07
	LD (data_DDA1),A	;[data_DDA1]
	POP HL
	POP AF
	RET
label_C967:
	CALL function_D2F4	;[function_D2F4]
	PUSH AF
	LD HL,Sprite0_YPixelPos	;[Sprite0_YPixelPos]
	PUSH HL
	LD A,(HL)
	SUB $08
	LD (HL),A
	CALL function_D2F4	;[function_D2F4]
	LD C,A
	POP HL
	LD A,(HL)
	ADD A,$08
	LD (HL),A
	POP AF
	OR C
	LD E,A
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	LD D,A
	AND E
	XOR D
	LD E,A
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	BIT 3,E
	JR Z,label_C98F	;[label_C98F]
	ADD A,$02
label_C98F:
	BIT 2,E
	JR Z,label_C995	;[label_C995]
	SUB $02
label_C995:
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
	CALL function_C94D	;[function_C94D]
	CALL function_D2F0	;[function_D2F0]
	PUSH AF
	LD HL,Sprite0_YPixelPos	;[Sprite0_YPixelPos]
	PUSH HL
	LD A,(HL)
	SUB $08
	LD (HL),A
	CALL function_D2F0	;[function_D2F0]
	LD C,A
	POP HL
	LD A,(HL)
	ADD A,$08
	LD (HL),A
	POP AF
	OR C
	LD E,A
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	LD D,A
	AND E
	XOR D
	LD E,A
	LD A,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	BIT 0,E
	JR Z,label_C9C3	;[label_C9C3]
	ADD A,$02
label_C9C3:
	BIT 1,E
	JR Z,label_C9C9	;[label_C9C9]
	SUB $02
label_C9C9:
	LD (Sprite0_XPixelPos),A	;[Sprite0_XPixelPos]
	LD A,(data_DD28)	;[data_DD28]
	INC A
	LD (data_DD28),A	;[data_DD28]
	CP $03
	JR C,label_CA15	;[label_CA15]
	XOR A
	LD (data_DD28),A	;[data_DD28]
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	LD C,A
	LD A,(data_DD26)	;[data_DD26]
	BIT 0,C
	JR Z,label_C9EC	;[label_C9EC]
	DEC A
	CP $05
	JR C,label_C9EC	;[label_C9EC]
	XOR A
label_C9EC:
	BIT 1,C
	JR Z,label_C9F7	;[label_C9F7]
	INC A
	CP $05
	JR C,label_C9F7	;[label_C9F7]
	LD A,$04
label_C9F7:
	LD (data_DD26),A	;[data_DD26]
	RLCA
	LD E,A
	LD D,$00
	LD HL,$CA0B	;[label_C9F7 + 20]
	ADD HL,DE
	LD E,(HL)
	INC HL
	LD D,(HL)
	LD (Sprite0_SpriteAddr2),DE	;[Sprite0_SpriteAddr2]
	JR label_CA15	;[label_CA15]
	db $74
	db $E0
	db $74
	db $E6
	db $34
	db $E7
	db $F4
	db $E7
	db $74
	db $E3
label_CA15:
	LD A,(data_DD2D)	;[data_DD2D]
	CP $00
	JR NZ,label_CA5F	;[label_CA5F]
	LD A,(data_DD27)	;[data_DD27]
	CP $00
	JP Z,label_CB36	;[label_CB36]
	LD A,(data_D2CF)	;[data_D2CF]
	CP $00
	JP Z,label_CB36	;[label_CB36]
	LD A,(data_A41D)	;[data_A41D]
	CP $00
	JR Z,label_CA3B	;[label_CA3B]
	LD A,(data_A41F)	;[data_A41F]
	CP $F7
	JP Z,label_CB36	;[label_CB36]
label_CA3B:
	LD A,$05
	LD (data_A41B),A	;[data_A41B]
	LD A,$02
	LD C,$01
	CALL function_D41F	;[function_D41F]
	LD A,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	AND $F8
	LD (Sprite5_XPixelPos),A	;[Sprite5_XPixelPos]
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	INC A
	AND $F8
	DEC A
	LD (Sprite5_YPixelPos),A	;[Sprite5_YPixelPos]
	LD A,(LastCtrlPlayerCtrl)	;[LastCtrlPlayerCtrl]
	LD (data_DD2D),A	;[data_DD2D]
label_CA5F:
	LD IX,Sprite5	;[Sprite5]
	CALL function_D2F0	;[function_D2F0]
	LD C,A
	LD B,$00
	LD A,(data_DD2D)	;[data_DD2D]
	LD D,A
	AND $03
	AND C
	JR Z,label_CA8F	;[label_CA8F]
	LD B,$01
	LD A,D
	XOR $03
	LD D,A
	AND $0C
	JR NZ,label_CA89	;[label_CA89]
	LD A,(RandomNo1)	;[RandomNo1]
	SET 3,D
	AND $01
	JR Z,label_CA87	;[label_CA87]
	LD A,$0C
label_CA87:
	XOR D
	LD D,A
label_CA89:
	LD A,D
	LD (data_DD2D),A	;[data_DD2D]
	JR label_CAA0	;[label_CAA0]
label_CA8F:
	LD HL,Sprite5_XPixelPos	;[Sprite5_XPixelPos]
	LD A,(HL)
	BIT 0,D
	JR Z,label_CA99	;[label_CA99]
	ADD A,$08
label_CA99:
	BIT 1,D
	JR Z,label_CA9F	;[label_CA9F]
	SUB $08
label_CA9F:
	LD (HL),A
label_CAA0:
	PUSH BC
	CALL function_D2F4	;[function_D2F4]
	POP BC
	LD C,A
	LD A,(data_DD2D)	;[data_DD2D]
	LD D,A
	AND $0C
	AND C
	JR Z,label_CAE2	;[label_CAE2]
	INC B
	LD A,D
	XOR $0C
	LD D,A
	AND $03
	JR NZ,label_CAC5	;[label_CAC5]
	SET 0,D
	LD A,(data_DAC1)	;[data_DAC1]
	AND $01
	JR Z,label_CAC3	;[label_CAC3]
	LD A,$03
label_CAC3:
	XOR D
	LD D,A
label_CAC5:
	LD A,B
	CP $02
	JR NZ,label_CADC	;[label_CADC]
	DEC B
	LD A,(RandomNo1)	;[RandomNo1]
	AND $20
	JR Z,label_CAD8	;[label_CAD8]
	LD A,D
	AND $03
	LD D,A
	JR label_CADC	;[label_CADC]
label_CAD8:
	LD A,D
	AND $0C
	LD D,A
label_CADC:
	LD A,D
	LD (data_DD2D),A	;[data_DD2D]
	JR label_CAF3	;[label_CAF3]
label_CAE2:
	LD HL,Sprite5_YPixelPos	;[Sprite5_YPixelPos]
	LD A,(HL)
	BIT 2,D
	JR Z,label_CAEC	;[label_CAEC]
	SUB $08
label_CAEC:
	BIT 3,D
	JR Z,label_CAF2	;[label_CAF2]
	ADD A,$08
label_CAF2:
	LD (HL),A
label_CAF3:
	LD A,(data_DD2E)	;[data_DD2E]
	ADD A,B
	CP $02
	JR NC,label_CB33	;[label_CB33]
	LD (data_DD2E),A	;[data_DD2E]
	LD HL,(Sprite5_XPixelPos)	;[Sprite5_XPixelPos]
	LD A,L
	CP $F2
	JR NC,label_CB33	;[label_CB33]
	LD A,H
	CP $0F
	JR C,label_CB33	;[label_CB33]
	CP $91
	JR NC,label_CB33	;[label_CB33]
	LD HL,$DD2F	;[data_DD2E + 1]
	INC (HL)
	LD A,(HL)
	CP $04
	JR C,label_CB19	;[label_CB19]
	XOR A
label_CB19:
	LD (HL),A
	RLCA
	LD E,A
	LD D,$00
	LD HL,$CB2B	;[label_CB19 + 18]
	ADD HL,DE
	LD E,(HL)
	INC HL
	LD D,(HL)
	LD (Sprite5_SpriteAddr2),DE	;[Sprite5_SpriteAddr2]
	JR label_CB36	;[label_CB36]
	db $88
	db $B0
	db $B8
	db $B0
	db $E8
	db $B0
	db $18
	db $B1
label_CB33:
	CALL function_C8DD	;[function_C8DD]
label_CB36:
	LD HL,CurrPlayerControl	;[CurrPlayerControl]
	RES 3,(HL)
	LD HL,Sprite0_YPixelPos	;[Sprite0_YPixelPos]
	LD A,(HL)
	CP $16
	JR NC,label_CB4B	;[label_CB4B]
	LD (HL),$8F
	LD DE,PRINT_A_1	;[PRINT_A_1]
	JP label_C936	;[label_C936]
label_CB4B:
	CP $90
	JP C,label_C8F4	;[label_C8F4]
	LD (HL),$17
	LD DE,$FFF0	;[SmallPlatform3Data + 4680]
	JP label_C936	;[label_C936]
label_CB58:
	LD HL,$DD30	;[data_DD2E + 2]
	INC (HL)
	LD A,(HL)
	CP $78
	JR C,label_CB68	;[label_CB68]
	XOR A
	LD (HL),A
	LD C,$04
	CALL function_D41F	;[function_D41F]
label_CB68:
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	CP $08	;has 'up' been pressed
	JR NZ,label_CB7E	;[label_CB7E]
	LD A,(data_DD32)	;[data_DD32]
	CP $00
	JR NZ,label_CB86	;[label_CB86]
	LD HL,$0101	;[TOKENS + 108]
	LD (data_DD31),HL	;[data_DD31]
	JR label_CB8A	;[label_CB8A]
label_CB7E:
	LD HL,START
	LD (data_DD31),HL	;[data_DD31]
	JR label_CB8A	;[label_CB8A]
label_CB86:
	XOR A
	LD (data_DD31),A	;[data_DD31]
label_CB8A:
	LD HL,$96FC	;[data_96FA + 2]
	LD B,$16
label_CB8F:
	PUSH BC
	LD C,(HL)	;get X pix coord
	INC HL
	LD B,(HL)	;get y pix coord
	INC HL
	LD D,(HL)	;get ??
	PUSH HL	;store pointer
	LD HL,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,D
	CP $01
	JR C,label_CBAA	;[label_CBAA]	;is val less than 1?
	CP $0C	;less than c (12)
	JR C,label_CBB6	;[label_CBB6]
	CP $14	;greater than 14h
	JR NC,label_CBB6	;[label_CBB6]
	CP $0E	;e ?
	JR Z,label_CBB6	;[label_CBB6]
label_CBAA:
	LD A,L
	CP C
	JP NZ,NoPlayerIntersection	;[NoPlayerIntersection]
	LD A,H
	CP B
	JP NZ,NoPlayerIntersection	;[NoPlayerIntersection]
	JR PlayerItemIntersection	;[PlayerItemIntersection]
label_CBB6:
	LD A,B	;item y
	CP H	;compare to player y
	JR C,label_CBC2	;[label_CBC2]	;player height is greater?
	SUB H	;subtract player y
	CP $0F	;less than 15
	JP NC,NoPlayerIntersection	;[NoPlayerIntersection]
	JR label_CBC9	;[label_CBC9]
label_CBC2:
	LD A,H	;player y in A
	SUB B	;subtract item y
	CP $0F
	JP NC,NoPlayerIntersection	;[NoPlayerIntersection]
label_CBC9:
	LD A,C	;item x in a
	CP L	;compare to player x
	JR C,label_CBD5	;[label_CBD5]
	SUB L
	CP $0F
	JP NC,NoPlayerIntersection	;[NoPlayerIntersection]
	JR PlayerItemIntersection	;[PlayerItemIntersection]
label_CBD5:
	LD A,L	;put player x in a
	SUB C	;subtract item x
	CP $0F
	JP NC,NoPlayerIntersection	;[NoPlayerIntersection]
PlayerItemIntersection:
	LD A,D
	CP $00
	JP NZ,label_CC5A	;[label_CC5A]
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	AND $03
	JP Z,NoPlayerIntersection	;[NoPlayerIntersection]
	CALL ClearGameplayArea_Thunk	;[ClearGameplayArea_Thunk]
	db $CD
	db $5F
	db $D5
	db $CD
	db $C1
	db $D3
	db $16
	db $08
	db $09
	db $53
	db $45
	db $43
	db $55
	db $52
	db $49
	db $54
	db $59
	db $20
	db $20
	db $44
	db $4F
	db $4F
	db $52
	db $16
	db $0F
	db $0A
	db $41
	db $43
	db $43
	db $45
	db $53
	db $53
	db $20
	db $20
	db $43
	db $4F
	db $44
	db $45
	db $FF
	db $01
	db $0C
	db $0A
	db $2E
	db $25
	db $CD
	db $65
	db $EA
	db $2C
	db $0E
	db $10
	db $CD
	db $65
	db $EA
	db $3E
	db $08
	db $CD
	db $C0
	db $D7
	db $01
	db $0B
	db $11
	db $3E
	db $03
	db $CD
	db $FD
	db $D5
	db $FE
	db $00
	db $20
	db $18
	db $3E
	db $0A
	db $CD
	db $C0
	db $D7
	db $3A
	db $23
	db $DD
	db $CB
	db $47
	db $3A
	db $1D
	db $DD
	db $28
	db $04
	db $C6
	db $30
	db $18
	db $02
	db $D6
	db $30
	db $32
	db $1D
	db $DD
label_CC4B:
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	INC A
	AND $F8
	DEC A
	LD (Sprite0_YPixelPos),A	;[Sprite0_YPixelPos]
	LD A,$03
	POP DE
	POP DE
	RET
label_CC5A:
	CP $01	;check for type 1
	JP NZ,HandleKillZoneIntersection	;[HandleKillZoneIntersection]
	LD A,(ScreenItemType)	;[ScreenItemType]	;check for screen item type 19
	CP $19
	JR Z,label_CC82	;[label_CC82]
	LD BC,(ScreenItemPosition)	;[ScreenItemPosition]
	CALL GetItemImagePtr	;[GetItemImagePtr]
	LD A,$47
	CALL DrawItem	;[DrawItem]
	LD A,(data_D2C3)	;[data_D2C3]
	OR $40
	CALL function_D97B	;[function_D97B]
	POP HL
	LD (HL),$05
	PUSH HL
	LD HL,data_D2BE	;[data_D2BE]
	DEC (HL)
label_CC82:
	LD A,(ScreenItemType)	;[ScreenItemType]	;check for screen item type 19
	CP $19
	JP Z,label_CCF1	;[label_CCF1]
	XOR A
	LD HL,$A350	;[label_A345 + 11]
	CALL function_A801	;[function_A801]
	LD A,(ScreenItemType)	;[ScreenItemType]
	CALL function_CC9A	;[function_CC9A]
	JP NoPlayerIntersection	;[NoPlayerIntersection]
function_CC9A:
	CP $17
	CALL Z,function_CCCC	;[function_CCCC]
	SUB $11
	RLCA
	LD E,A
	LD D,$00
	LD HL,$CCBC	;[function_CC9A + 34]
	ADD HL,DE
	LD A,(HL)
	CALL function_D7C0	;[function_D7C0]
	LD E,(HL)
	LD D,$00
	INC HL
	LD A,(HL)
	LD HL,data_D2CC	;[data_D2CC]
	ADD HL,DE
	ADD A,(HL)
	LD (HL),A
	CALL function_D425	;[function_D425]
	RET
	db $01
	db $20
	db $01
	db $60
	db $01
	db $40
	db $02
	db $32
	db $03
	db $20
	db $03
	db $3C
	db $00
	db $00
	db $00
	db $01
function_CCCC:
	LD HL,data_D2CC	;[data_D2CC]
	XOR A
	CP (HL)
	JR NZ,label_CCD6	;[label_CCD6]
	LD A,$18
	RET
label_CCD6:
	LD B,$03
	LD A,$FF
label_CCDA:
	INC HL
	CP (HL)
	JR C,label_CCE4	;[label_CCE4]
	LD A,$03
	SUB B
	RLCA
	LD E,A
	LD A,(HL)
label_CCE4:
	DJNZ label_CCDA	;[label_CCDA]
	LD A,E
	ADD A,$12
	RET
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
label_CCF1:
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]	;check if up has been pressed
	BIT 3,A
	JP Z,NoPlayerIntersection	;[NoPlayerIntersection]
	CALL ClearGameplayArea_Thunk	;[ClearGameplayArea_Thunk]
	CALL function_CDFE	;[function_CDFE]
	CALL DrawText	;[DrawText]
	ascii '	CHEOPS KEY CODE'
	db $FF
	LD A,$0B
	CALL function_D7C0	;[function_D7C0]
	LD A,$02
	LD BC,$0F0D	;[COPY_L_2 + 1]
	CALL function_D5FD	;[function_D5FD]
	CP $00
	JP NZ,label_CC4B	;[label_CC4B]
	db $CD
	db $12
	db $A4
	db $CD
	db $FE
	db $CD
	db $21
	db $D2
	db $D2
	db $06
	db $04
	db $7E
	db $FE
	db $00
	db $28
	db $08
	db $FE
	db $09
	db $38
	db $12
	db $FE
	db $1A
	db $30
	db $0E
	db $23
	db $23
	db $10
	db $EF
	db $06
	db $04
	db $2B
	db $2B
	db $7E
	db $A7
	db $20
	db $02
	db $10
	db $F8
	db $22
	db $EF
	db $CC
	db $21
	db $EE
	db $CC
	db $77
	db $06
	db $04
	db $2B
	db $CD
	db $C6
	db $DA
	db $3A
	db $C0
	db $DA
	db $D6
	db $09
	db $30
	db $FC
	db $C6
	db $0A
	db $11
	db $DD
	db $D2
	db $13
	db $3D
	db $20
	db $FC
	db $1A
	db $FE
	db $80
	db $38
	db $E8
	db $E6
	db $3F
	db $77
	db $10
	db $E2
	db $CD
	db $C1
	db $D3
	db $16
	db $0D
	db $08
	db $45
	db $58
	db $43
	db $48
	db $41
	db $4E
	db $47
	db $45
	db $20
	db $20
	db $20
	db $20
	db $46
	db $4F
	db $52
	db $FF
	db $CD
	db $5F
	db $D5
	db $CD
	db $C1
	db $D3
	db $16
	db $15
	db $04
	db $48
	db $49
	db $54
	db $20
	db $41
	db $4E
	db $59
	db $20
	db $4B
	db $45
	db $59
	db $20
	db $46
	db $52
	db $4F
	db $4D
	db $20
	db $31
	db $20
	db $54
	db $4F
	db $20
	db $35
	db $FF
	db $3A
	db $EE
	db $CC
	db $CD
	db $68
	db $CE
	db $01
	db $11
	db $0C
	db $3A
	db $C0
	db $DA
	db $D6
	db $06
	db $30
	db $FC
	db $C6
	db $08
	db $CD
	db $24
	db $DB
	db $06
	db $05
	db $3E
	db $05
	db $90
	db $CD
	db $2C
	db $CE
	db $10
	db $F8
label_CDCA:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $31
	JR C,label_CDCA	;[label_CDCA]
	CP $36
	JR NC,label_CDCA	;[label_CDCA]
	SUB $31
	LD B,$23
	PUSH AF
	LD A,$10
	CALL function_D7C0	;[function_D7C0]
	db $F1
	db $CD
	db $2C
	db $CE
	db $10
	db $F4
	db $5F
	db $16
	db $00
	db $21
	db $EA
	db $CC
	db $19
	db $7E
	db $2A
	db $EF
	db $CC
	db $77
	db $CD
	db $25
	db $D4
	db $21
	db $50
	db $A3
	db $AF
	db $CD
	db $01
	db $A8
	db $C3
	db $4B
	db $CC
function_CDFE:
	CALL function_D55F	;[function_D55F]
	CALL DrawText	;[DrawText]
	db $16
	db $09
	db $06
	db $43
	db $48
	db $45
	db $4F
	db $50
	db $53
	db $20
	db $50
	db $59
	db $52
	db $41
	db $4D
	db $49
	db $44
	db $FF
	LD A,(RandomNo1)	;[RandomNo1]
label_CE19:
	SUB $06
	JR NC,label_CE19	;[label_CE19]
	ADD A,$08
	LD BC,$0816	;[LD_CONTRL + 14]
	LD HL,$93A8	;[ItemImgData + 800]
	CALL DrawItem	;[DrawItem]
	CALL function_D55F	;[function_D55F]
	RET
	db $C5
	db $F5
	db $CD
	db $5F
	db $D5
	db $5F
	db $16
	db $00
	db $21
	db $EA
	db $CC
	db $19
	db $7E
	db $CD
	db $68
	db $CE
	db $7B
	db $07
	db $83
	db $07
	db $C6
	db $04
	db $4F
	db $06
	db $10
	db $3D
	db $3D
	db $32
	db $61
	db $CE
	db $3A
	db $C0
	db $DA
	db $D6
	db $06
	db $30
	db $FC
	db $C6
	db $08
	db $CD
	db $24
	db $DB
	db $7B
	db $C6
	db $31
	db $32
	db $62
	db $CE
	db $CD
	db $C1
	db $D3
	db $16
	db $0F
	db $05
	db $31
	db $2E
	db $FF
	db $F1
	db $C1
	db $C9
GetItemImagePtr:	;A - Item no, ptr in HL
	PUSH DE
	LD L,A	;item no
	LD H,$00
	ADD HL,HL	;mul HL by 32
	ADD HL,HL
	ADD HL,HL
	ADD HL,HL
	ADD HL,HL
	LD DE,ItemImgData	;[ItemImgData]
	ADD HL,DE
	POP DE
	RET
HandleKillZoneIntersection:
	CP $06	;check for type 6 - Kill zone
	JR NZ,HandleLockedDoorIntersection	;[HandleLockedDoorIntersection]
	POP HL
	POP HL
	LD A,$10
	JP label_C350	;[label_C350]
HandleLockedDoorIntersection:
	CP $0B	;check for type B - locked door
	JR NZ,HandleTransporterStationIntersection	;[HandleTransporterStationIntersection]
	LD HL,PlayerInventory??	;[PlayerInventory??]
	LD B,$04	;loop count
label_CE8B:
	LD A,(HL)
	CP $10
	JR Z,label_CE96	;[label_CE96]
	INC HL
	INC HL
	DJNZ label_CE8B	;[label_CE8B]
	JR label_CEAA	;[label_CEAA]
label_CE96:
	LD HL,(data_9605)	;[data_9605]
	LD A,(HL)
	AND $7F
	JR Z,label_CEAA	;[label_CEAA]
	LD A,(HL)
	AND $80
	LD (HL),A
	CALL function_A807	;[function_A807]
	db $3E
	db $08
	db $CD
	db $C0
	db $D7
label_CEAA:
	JP NoPlayerIntersection	;[NoPlayerIntersection]
HandleTransporterStationIntersection:
	CP $0C	;check for type C - Transporter Base
	JR NZ,HandleTeleporterIntersection	;[HandleTeleporterIntersection]
	CALL function_C8DD	;[function_C8DD]
	LD HL,LastCtrlPlayerCtrl	;[LastCtrlPlayerCtrl]
	XOR A
	BIT 3,(HL)
	JR Z,label_CEBE	;[label_CEBE]
	LD A,$02
label_CEBE:
	LD (Sprite0_OnTransport),A	;[Sprite0_OnTransport]	;set player on transport
	JP NoPlayerIntersection	;[NoPlayerIntersection]
HandleTeleporterIntersection:
	CP $0D	;Check for type D - Teleporter
	JP NZ,label_D09F	;[label_D09F]
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	AND $03
	JP Z,NoPlayerIntersection	;[NoPlayerIntersection]
	CALL ClearGameplayArea_Thunk	;[ClearGameplayArea_Thunk]
	LD HL,START
	LD (AttributeDrawListPtr),HL	;[AttributeDrawListPtr]
	CALL function_D55F	;[function_D55F]
	CALL DrawText	;[DrawText]
	ascii 'YOU HAVE ENTERED<cr>
TELEPORTCODE : '
	db $FF
	LD BC,$0917	;[ME_OLD_V3 + 5]	;coords
	LD L,$24	;teleporter
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD HL,$D03B	;[TeleportCodes + 5]
	LD B,$0F	;loop count 15
TeleportCodeLoop:
	LD DE,(CurrentScreenNo)	;[CurrentScreenNo]
	LD A,(HL)	;first byte of teleport screen no
	INC HL
	CP E	;compare to first byte of current screen
	JR NZ,SkipCode	;[SkipCode]
	LD A,(HL)
	CP D	;compare second byte
	JR Z,ThisRoomsTeleportCode	;[ThisRoomsTeleportCode]
SkipCode:
	LD DE,$0006	;[START + 6]	;skip to next code
	ADD HL,DE
	DJNZ TeleportCodeLoop	;[TeleportCodeLoop]
ThisRoomsTeleportCode:
	LD DE,$0006	;[START + 6]
	OR A
	SBC HL,DE	;point HL at code name
	LD DE,TeleportCodeToDraw	;[TeleportCodeToDraw]
	LD BC,$0005	;[START + 5]
	LDIR	;copy teleport code string
	CALL function_D55F	;[function_D55F]
	CALL DrawText	;[DrawText]
TeleportCodeToDraw:
	ascii 'VEROX'
	db $FF
	CALL function_D55F	;[function_D55F]
	CALL DrawText	;[DrawText]
	ascii 'ENTER TELEPORTALDESTINATION CODE'
	db $FF
	CALL function_D55F	;[function_D55F]
	CALL DrawText	;[DrawText]
	ascii '- - - - -'
	db $FF
	LD A,$07
	CALL function_D7C0	;[function_D7C0]
	CALL function_D55F	;[function_D55F]
	LD HL,$D031	;[label_D00A + 39]
	LD B,$05
label_CF93:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $00
	JR NZ,label_CF93	;[label_CF93]
label_CF9A:
	CALL GetKeyPressedAscii	;[GetKeyPressedAscii]
	CP $0A
	JR C,label_CF9A	;[label_CF9A]
	LD (HL),A
	INC HL
	LD (data_CFA9),A	;[data_CFA9]
	CALL DrawText	;[DrawText]
data_CFA9:
	db $4C
	db $20
	db $FF
	LD A,$11
	CALL function_D7C0	;[function_D7C0]
	DJNZ label_CF93	;[label_CF93]
	LD HL,TeleportCodes	;[TeleportCodes]
	LD C,$0F
label_CFB8:
	LD B,$05
	LD DE,$D031	;[label_D00A + 39]
	PUSH HL
label_CFBE:
	LD A,(DE)
	CP (HL)
	JR NZ,label_CFFD	;[label_CFFD]
	INC HL
	INC DE
	DJNZ label_CFBE	;[label_CFBE]
	POP DE
	POP DE
	POP DE
	LD E,(HL)
	INC HL
	LD D,(HL)
	LD (CurrentScreenNo),DE	;[CurrentScreenNo]
	LD B,$14
	CALL function_D55F	;[function_D55F]
label_CFD5:
	CALL function_D58A	;[function_D58A]
	CALL DrawText	;[DrawText]
	db $16
	db $15
	db $09
	db $4E
	db $4F
	db $57
	db $20
	db $54
	db $45
	db $4C
	db $45
	db $50
	db $4F
	db $52
	db $54
	db $49
	db $4E
	db $47
	db $FF
	LD A,$10
	CALL function_D7C0	;[function_D7C0]
	DJNZ label_CFD5	;[label_CFD5]
	LD A,$09
	CALL function_D7C0	;[function_D7C0]
	LD A,$04
	RET
label_CFFD:
	POP HL
	LD DE,$0007	;[START + 7]
	ADD HL,DE
	DEC C
	JR NZ,label_CFB8	;[label_CFB8]
	CALL function_D55F	;[function_D55F]
	LD B,$28
label_D00A:
	CALL function_D58A	;[function_D58A]
	CALL DrawText	;[DrawText]
	db $16
	db $15
	db $06
	db $43
	db $4F
	db $44
	db $45
	db $20
	db $4E
	db $4F
	db $54
	db $20
	db $52
	db $45
	db $43
	db $4F
	db $47
	db $4E
	db $49
	db $53
	db $45
	db $44
	db $FF
	LD A,$0F
	CALL function_D7C0	;[function_D7C0]
	DJNZ label_D00A	;[label_D00A]
	JP label_CC4B	;[label_CC4B]
	db $45
	db $58
	db $49
	db $41
	db $4C
TeleportCodes:
	db $56
	db $45
	db $52
	db $4F
	db $58
	dw $0028
	db $52
	db $41
	db $4D
	db $49
	db $58
	dw $001F
	db $54
	db $55
	db $4C
	db $53
	db $41
	dw $0042
	db $41
	db $53
	db $4F
	db $49
	db $43
	dw $0096
	db $44
	db $45
	db $4C
	db $54
	db $41
	dw $00A2
	db $51
	db $55
	db $41
	db $4B
	db $45
	dw $00D5
	db $41
	db $4C
	db $47
	db $4F
	db $4C
	dw $0121
	db $45
	db $58
	db $49
	db $41
	db $4C
	dw $0157
	db $4B
	db $59
	db $5A
	db $49
	db $41
	dw $017C
	db $55
	db $4C
	db $54
	db $52
	db $41
	dw $01B1
	db $49
	db $52
	db $41
	db $47
	db $45
	dw $01C9
	db $4F
	db $4B
	db $54
	db $55
	db $50
	dw $01CD
	db $53
	db $4F
	db $4E
	db $49
	db $51
	dw $01D6
	db $41
	db $4D
	db $49
	db $47
	db $41
	dw $01F3
	db $41
	db $4D
	db $41
	db $48
	db $41
	db $FA
	db $01
label_D09F:
	CP $0E	;check for type E - ???
	JR NZ,label_D117	;[label_D117]
	LD A,(data_DD29)	;[data_DD29]
	CP $10
	JP NZ,NoPlayerIntersection	;[NoPlayerIntersection]
	LD A,(Sprite0_YPixelPos)	;[Sprite0_YPixelPos]
	CP B
	JP NZ,NoPlayerIntersection	;[NoPlayerIntersection]
	POP HL
	LD (HL),$05
	PUSH HL
	LD A,C
	RRCA
	RRCA
	RRCA
	DEC A
	AND $1F
	LD (data_D0D4),A	;[data_D0D4]
	LD C,A
	LD A,$BF
	SUB B
	RRCA
	RRCA
	RRCA
	ADD A,$02
	AND $1F
	LD (data_D0D3),A	;[data_D0D3]
	LD B,A
	CALL DrawText	;[DrawText]
	db $16
data_D0D3:
	db $0F
data_D0D4:
	db $14
	db $13
	db $01
	db $10
	db $07
	db $20
	db $20
	db $20
	db $20
	db $FF
	db $3E
	db $10
	db $CD
	db $C0
	db $D7
	db $1E
	db $02
	db $21
	db $BC
	db $DB
	db $16
	db $0C
	db $AF
	db $BE
	db $28
	db $0A
	db $23
	db $23
	db $23
	db $23
	db $15
	db $20
	db $F6
	db $C3
	db $A6
	db $D1
	db $2B
	db $79
	db $F6
	db $40
	db $77
	db $23
	db $70
	db $23
	db $23
	db $36
	db $02
	db $2E
	db $03
	db $16
	db $02
	db $AF
	db $CD
	db $88
	db $DB
	db $2D
	db $15
	db $20
	db $F8
	db $0C
	db $0C
	db $1D
	db $20
	db $D1
	db $C3
	db $A6
	db $D1
label_D117:
	CP $0F	;check for type F - ???
	JR NZ,label_D13B	;[label_D13B]
	LD A,(CurrPlayerControl)	;[CurrPlayerControl]
	AND $03
	JP Z,NoPlayerIntersection	;[NoPlayerIntersection]
	LD HL,(CurrentScreenNo)	;[CurrentScreenNo]
	AND $01
	JR NZ,label_D12D	;[label_D12D]
	DEC HL
	JR label_D12E	;[label_D12E]
label_D12D:
	INC HL
label_D12E:
	LD (CurrentScreenNo),HL	;[CurrentScreenNo]
	LD A,$04
	CALL function_D7C0	;[function_D7C0]
	db $3E
	db $05
	db $E1
	db $C1
	db $C9
label_D13B:
	CP $14	;check for ???
	JR C,NoPlayerIntersection	;[NoPlayerIntersection]
	LD E,A
	LD HL,data_DD31	;[data_DD31]
	LD A,(HL)
	CP $01
	JR NZ,NoPlayerIntersection	;[NoPlayerIntersection]
	INC (HL)
	LD HL,data_D2BE	;[data_D2BE]
	DEC (HL)
	LD A,E
	SUB $14
	RLCA
	RLCA
	LD E,A
	LD D,$00
	LD HL,CollectableItemInfo	;[CollectableItemInfo]
	ADD HL,DE
	LD A,(HL)
	AND $1F
	LD C,A
	LD A,(HL)
	RLCA
	RLCA
	RLCA
	AND $07
	LD (data_D2D1),A	;[data_D2D1]
	INC HL
	LD A,(HL)
	AND $1F
	LD B,(HL)
	LD (HL),$01
	INC HL
	INC HL
	LD A,(HL)
	LD (data_D2D0),A	;[data_D2D0]
	CALL GetItemImagePtr	;[GetItemImagePtr]
	LD A,$47
	CALL DrawItem	;[DrawItem]
	LD A,(data_D2D1)	;[data_D2D1]
	OR $40
	CALL function_D97B	;[function_D97B]
	POP HL
	PUSH HL
	DEC HL
	DEC HL
	EX DE,HL
	LD HL,$973A	;[data_96FA + 64]
	OR A
	SBC HL,DE
	PUSH HL
	POP BC
	LD HL,$0003	;[START + 3]
	ADD HL,DE
	LDIR
	LD DE,$0003	;[START + 3]
	OR A
	POP HL
	SBC HL,DE
	PUSH HL
	LD HL,(data_96FA)	;[data_96FA]
	SBC HL,DE
	LD (data_96FA),HL	;[data_96FA]
NoPlayerIntersection:
	POP HL
	POP BC
	XOR A
	INC HL
	INC HL
	CP (HL)
	JR Z,label_D1B3	;[label_D1B3]
	DEC HL
	DEC B
	JP NZ,label_CB8F	;[label_CB8F]
label_D1B3:
	LD A,(data_DD31)	;[data_DD31]
	CP $00
	JP Z,label_D2B9	;[label_D2B9]
	LD A,(data_D2DB)	;[data_D2DB]
	CP $00
	JR Z,label_D1CA	;[label_D1CA]
	LD A,(data_D2BE)	;[data_D2BE]
	CP $04
	JP NC,label_D2B9	;[label_D2B9]
label_D1CA:
	LD A,$0C
	CALL function_D7C0	;[function_D7C0]
	LD DE,data_D2DB	;[data_D2DB]
	LD HL,$D2D9	;[PlayerInventory?? + 7]
	LD BC,$000A	;[ERROR_1 + 2]
	LDDR
	XOR A
	INC HL
	LD (HL),A
	INC HL
	LD (HL),A
	LD D,$05
label_D1E1:
	CALL function_D2A6	;[function_D2A6]
	INC D
	CP $00
	JR Z,label_D1F1	;[label_D1F1]
	LD (HL),D
	LD A,D
	CP $06
	JR NZ,label_D1F1	;[label_D1F1]
	LD (HL),$32
label_D1F1:
	DEC D
	DEC D
	JR NZ,label_D1E1	;[label_D1E1]
	CALL function_D425	;[function_D425]
	LD A,(data_D2DB)	;[data_D2DB]
	CP $00
	JP Z,label_D2B9	;[label_D2B9]
	LD HL,data_D2BE	;[data_D2BE]
	INC (HL)
	LD BC,(Sprite0_XPixelPos)	;[Sprite0_XPixelPos]
	LD A,C
	RRCA
	RRCA
	RRCA
	AND $1F
	LD C,A
	LD A,$BF
	SUB B
	RRCA
	RRCA
	RRCA
	AND $1F
	LD B,A
	LD A,C
	CP $01
	JR C,label_D226	;[label_D226]
	DEC C
	CALL function_D267	;[function_D267]
	CP $00
	JR Z,label_D236	;[label_D236]
	db $0C
label_D226:
	LD A,C
	CP $1D
	JR NC,label_D236	;[label_D236]
	INC C
	INC C
	CALL function_D267	;[function_D267]
	db $FE
	db $00
	db $28
	db $02
	db $0D
	db $0D
label_D236:
	LD D,$32
	CALL function_D2A6	;[function_D2A6]
	LD A,E
	PUSH AF
	DEC HL
	LD A,(HL)
	AND $E0
	OR C
	LD (HL),A
	INC HL
	LD DE,(CurrentScreenNo)	;[CurrentScreenNo]
	LD A,D
	RRCA
	OR B
	LD (HL),A
	INC HL
	LD (HL),E
	INC HL
	LD A,(data_D2DA)	;[data_D2DA]
	LD (HL),A
	CALL GetItemImagePtr	;[GetItemImagePtr]
	LD A,(data_D2DB)	;[data_D2DB]
	ADD A,$40
	CALL DrawItem	;[DrawItem]
	POP DE
	LD A,$41
	SUB D
	CALL function_A7FE	;[function_A7FE]
	JR label_D2B9	;[label_D2B9]
function_D267:
	PUSH BC
	LD A,B
	RRCA
	RRCA
	RRCA
	LD D,A
	AND $E0
	OR C
	LD E,A
	LD A,D
	AND $03
	LD D,A
	LD HL,ScreenAttributes	;[ScreenAttributes]
	ADD HL,DE
	LD DE,NEXT_CHAR	;[NEXT_CHAR]
	LD A,$64
	LD C,$00
	BIT 6,(HL)
	JR Z,label_D2A2	;[label_D2A2]
	CP (HL)
	JR Z,label_D2A2	;[label_D2A2]
	INC HL
	BIT 6,(HL)
	JR Z,label_D2A2	;[label_D2A2]
	CP (HL)
	JR Z,label_D2A2	;[label_D2A2]
	ADD HL,DE
	BIT 6,(HL)
	JR Z,label_D2A2	;[label_D2A2]
	CP (HL)
	JR Z,label_D2A2	;[label_D2A2]
	DEC HL
	BIT 6,(HL)
	JR Z,label_D2A2	;[label_D2A2]
	CP (HL)
	JR Z,label_D2A2	;[label_D2A2]
	XOR A
	POP BC
	RET
label_D2A2:
	LD A,$01
	POP BC
	RET
function_D2A6:
	LD E,$2D
	LD HL,$94E9	;[CollectableItemInfo + 1]
label_D2AB:
	LD A,(HL)
	AND $7F
	CP D
	RET Z
	INC HL
	INC HL
	INC HL
	INC HL
	DEC E
	JR NZ,label_D2AB	;[label_D2AB]
	XOR A
	RET
label_D2B9:
	XOR A
	OUT ($FE),A
	DEC A
	RET
data_D2BE:
	db $00
	db $00
ScreenItemPosition:
	db $11
	db $13
ScreenItemType:
	db $16
data_D2C3:
	db $46
data_D2C4:
	db $00
data_D2C5:
	db $00
data_D2C6:
	db $78
data_D2C7:
	db $7B
CurrentScreenNo:
	dw $0200
FlyingTransportCoords:
	dw $0000
data_D2CC:
	db $04
data_D2CD:
	db $17
data_D2CE:
	db $30
data_D2CF:
	db $7E
data_D2D0:
	db $00
data_D2D1:
	db $00
PlayerInventory??:
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
data_D2DA:
	db $00
data_D2DB:
	db $00
data_D2DC:
	db $48
	db $8F
	db $80
	db $8B
	db $89
	db $8A
	db $84
	db $85
	db $A1
	db $8C
	db $88
data_D2E7:
	db $09
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
function_D2F0:
	XOR A
	PUSH AF
	JR label_D2F7	;[label_D2F7]
function_D2F4:
	LD A,$01
	PUSH AF
label_D2F7:
	XOR A
	LD (data_D3BE),A	;[data_D3BE]
	LD H,A
	LD A,$BF
	SUB (IX+$06)
	AND $F8
	RLA
	RL H
	RLA
	RL H
	LD L,A
	LD A,H
	OR $58
	LD H,A
	LD A,(IX+$05)
	AND $F8
	RRCA
	RRCA
	RRCA
	ADD A,L
	LD L,A
	LD (data_D3BF),HL	;[data_D3BF]
	LD D,$00
	LD E,$20
	LD C,D
	LD B,$40
	POP AF
	CP $00
	JR NZ,label_D373	;[label_D373]
	LD A,(IX+$05)
	AND $07
	JR NZ,label_D371	;[label_D371]
	PUSH HL
	DEC HL
	LD A,(IX+$06)
	INC A
	AND $07
	JR Z,label_D33D	;[label_D33D]
	LD A,(HL)
	CP B
	JR C,label_D346	;[label_D346]
	ADD HL,DE
label_D33D:
	LD A,(HL)
	CP B
	JR C,label_D346	;[label_D346]
	ADD HL,DE
	LD A,(HL)
	CP B
	JR NC,label_D34E	;[label_D34E]
label_D346:
	LD A,(data_D3BE)	;[data_D3BE]
	SET 1,A
	LD (data_D3BE),A	;[data_D3BE]
label_D34E:
	POP HL
	INC HL
	INC HL
	LD A,(IX+$06)
	INC A
	AND $07
	JR Z,label_D35E	;[label_D35E]
	LD A,(HL)
	CP B
	JR C,label_D36B	;[label_D36B]
	ADD HL,DE
label_D35E:
	LD A,(HL)
	CP B
	JR C,label_D36B	;[label_D36B]
	ADD HL,DE
	LD A,(HL)
	CP B
	JR C,label_D36B	;[label_D36B]
	LD A,(data_D3BE)	;[data_D3BE]
	RET
label_D36B:
	LD A,(data_D3BE)	;[data_D3BE]
	SET 0,A
	RET
label_D371:
	XOR A
	RET
label_D373:
	LD A,(IX+$06)
	INC A
	AND $07
	JR NZ,label_D371	;[label_D371]
	PUSH HL
	OR A
	SBC HL,DE
	LD A,(IX+$05)
	AND $07
	JR Z,label_D38B	;[label_D38B]
	LD A,(HL)
	CP B
	JR C,label_D394	;[label_D394]
	INC HL
label_D38B:
	LD A,(HL)
	CP B
	JR C,label_D394	;[label_D394]
	INC HL
	LD A,(HL)
	CP B
	JR NC,label_D39C	;[label_D39C]
label_D394:
	LD A,(data_D3BE)	;[data_D3BE]
	SET 3,A
	LD (data_D3BE),A	;[data_D3BE]
label_D39C:
	POP HL
	ADD HL,DE
	ADD HL,DE
	LD A,(IX+$05)
	AND $07
	JR Z,label_D3AB	;[label_D3AB]
	LD A,(HL)
	CP B
	JR C,label_D3B8	;[label_D3B8]
	INC HL
label_D3AB:
	LD A,(HL)
	CP B
	JR C,label_D3B8	;[label_D3B8]
	INC HL
	LD A,(HL)
	CP B
	JR C,label_D3B8	;[label_D3B8]
	LD A,(data_D3BE)	;[data_D3BE]
	RET
label_D3B8:
	LD A,(data_D3BE)	;[data_D3BE]
	SET 2,A
	RET
data_D3BE:
	db $00
data_D3BF:
	dw $5964	;Attribute address - of last sprite rendered?
DrawText:
	LD (data_D3DD),HL	;[data_D3DD]
	EX (SP),HL	;put return address in HL (address of text)
	PUSH AF
	PUSH DE
	PUSH BC
PrintCharLoop:
	LD A,(HL)
	INC HL
	CP $FF	;FF is terminator
	JR Z,ExitFunc_1	;[ExitFunc_1]
	PUSH HL
	CALL PRINT_A_2	;[PRINT_A_2]
	POP HL
	JR PrintCharLoop	;[PrintCharLoop]
ExitFunc_1:
	POP BC
	POP DE
	POP AF
	EX (SP),HL	;make function return address after text string
	LD HL,(data_D3DD)	;[data_D3DD]
	RET
data_D3DD:
	db $70
	db $5E
DrawHUDPanelOutline:
	LD L,$91	;right side of HUD panel
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	LD A,C	;add 3 to X
	ADD A,$03
	LD C,A
	LD E,$05	;count
DrawPanelEdgeLoop:
	LD L,$93	;top of HUD panel
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	PUSH BC	;store coords
	LD A,B	;add 5 to Y
	ADD A,$05
	LD B,A
	CALL DrawSmallPlatform	;[DrawSmallPlatform]	;draw bottom panel
	POP BC	;restore coords
	LD A,C	;add 4 to X
	ADD A,$04
	LD C,A
	LD A,E
	CP $04	;compare count to 4
	JR Z,label_D405	;[label_D405]
	CP $03
	JR NZ,label_D406	;[label_D406]
label_D405:
	INC C
label_D406:
	DEC E
	JR NZ,DrawPanelEdgeLoop	;[DrawPanelEdgeLoop]
	LD A,C	;add 2 to X
	ADD A,$02
	LD C,A
	LD L,$92	;draw right end of HUD panel
	CALL DrawSmallPlatform	;[DrawSmallPlatform]
	RET
	db $00
	db $00
	db $02
	db $09
	db $05
	db $00
	db $00
	db $00
data_D41B:
	db $00
	db $00
data_D41D:
	db $00
	db $00
function_D41F:
	JP label_D4E9	;[label_D4E9]
function_D422:
	JP function_D521	;[function_D521]
function_D425:
	CALL function_D521	;[function_D521]
	LD A,(data_D2CC)	;[data_D2CC]
	LD C,$2F
label_D42D:
	INC C
	SUB $0A
	JR NC,label_D42D	;[label_D42D]
	ADD A,$3A
	LD B,A
	LD (data_D441),BC	;[data_D441]
	CALL DrawText	;[DrawText]
	db $16
	db $03
	db $0B
	db $10
	db $06
data_D441:
	db $30
	db $34
	db $16
	db $01
	db $10
	db $10
	db $02
	db $20
	db $10
	db $04
	db $20
	db $20
	db $20
	db $16
	db $02
	db $10
	db $10
	db $07
	db $20
	db $20
	db $20
	db $20
	db $16
	db $03
	db $10
	db $10
	db $06
	db $20
	db $20
	db $20
	db $20
	db $10
	db $08
	db $FF
	LD B,$03
	LD HL,data_D2CF	;[data_D2CF]
label_D468:
	LD A,(HL)
	CP $7F
	JR C,label_D46F	;[label_D46F]
	LD (HL),$7F
label_D46F:
	LD A,B
	LD (data_D477),A	;[data_D477]
	CALL DrawText	;[DrawText]
	db $16
data_D477:
	db $01
	db $10
	db $FF
	LD A,(HL)
	RLCA
	RLCA
	RLCA
	AND $03
	JR Z,label_D48A	;[label_D48A]
label_D482:
	CALL DrawText	;[DrawText]
	db $28
	db $FF
	DEC A
	JR NZ,label_D482	;[label_D482]
label_D48A:
	LD A,(HL)
	CP $7F
	JR NZ,label_D493	;[label_D493]
	LD A,$28
	JR label_D499	;[label_D499]
label_D493:
	RRCA
	RRCA
	AND $07
	ADD A,$20
label_D499:
	LD (data_D49F),A	;[data_D49F]
	CALL DrawText	;[DrawText]
data_D49F:
	db $25
	db $FF
	DEC HL
	DJNZ label_D468	;[label_D468]
	CALL DrawText	;[DrawText]
	db $16
	db $01
	db $15
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $16
	db $02
	db $15
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $20
	db $FF
	LD C,$15
	LD D,$04
	LD HL,PlayerInventory??	;[PlayerInventory??]
label_D4C5:
	PUSH DE
	LD E,(HL)
	INC HL
	LD B,(HL)
	INC HL
	INC B
	DEC B
	JR Z,label_D4E2	;[label_D4E2]
	PUSH HL
	LD D,$00
	LD HL,NEXT_CHAR	;[NEXT_CHAR]
	CALL HL_HLxDE	;[HL_HLxDE]
	LD A,B
	LD DE,ItemImgData	;[ItemImgData]
	ADD HL,DE
	LD B,$01
	CALL DrawItem	;[DrawItem]
	POP HL
label_D4E2:
	POP DE
	INC C
	INC C
	DEC D
	JR NZ,label_D4C5	;[label_D4C5]
	RET
label_D4E9:
	RET
	LD D,$00
	LD HL,data_D2CD	;[data_D2CD]
	ADD HL,DE
	INC A
	LD (data_D51C),A	;[data_D51C]
	LD A,(HL)
	SUB C
	JR NC,label_D4F9	;[label_D4F9]
	XOR A
label_D4F9:
	LD (HL),A
	RLCA
	RLCA
	RLCA
	AND $03
	ADD A,$10
	LD (data_D51D),A	;[data_D51D]
	LD A,(HL)
	CP $04
	JR NC,label_D50B	;[label_D50B]
	ADD A,$03
label_D50B:
	RRCA
	RRCA
	AND $07
	ADD A,$20
	LD (data_D51E),A	;[data_D51E]
	CALL DrawText	;[DrawText]
	db $10
	db $08
	db $13
	db $01
	db $16
data_D51C:
	db $01
data_D51D:
	db $10
data_D51E:
	db $25
	db $FF
	RET
function_D521:
	LD B,$06
	LD HL,$D41E	;[data_D41D + 1]
	LD DE,$D418	;[label_D406 + 18]
	PUSH DE
	LD C,$00
label_D52C:
	LD A,(DE)
	ADD A,C
	ADD A,(HL)
	LD (HL),$00
	LD C,$FF
label_D533:
	INC C
	SUB $0A
	JR NC,label_D533	;[label_D533]
	ADD A,$0A
	LD (DE),A
	DEC DE
	DEC HL
	DJNZ label_D52C	;[label_D52C]
	POP HL
	LD DE,$D55C	;[label_D545 + 23]
	LD B,$06
label_D545:
	LD A,(HL)
	ADD A,$30
	LD (DE),A
	DEC HL
	DEC DE
	DJNZ label_D545	;[label_D545]
	CALL DrawText	;[DrawText]
	db $16
	db $02
	db $03
	db $13
	db $01
	db $10
	db $07
	db $30
	db $30
	db $32
	db $39
	db $35
	db $30
	db $FF
	RET
function_D55F:
	PUSH AF
	PUSH BC
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	LD A,(data_DAC1)	;[data_DAC1]
	AND $3F
label_D569:
	SUB $06
	JR NC,label_D569	;[label_D569]
	ADD A,$08
	LD B,A
	LD A,(data_D589)	;[data_D589]
	LD C,A
	LD A,B
	CP C
	JR NZ,label_D57A	;[label_D57A]
	XOR $01
label_D57A:
	LD (data_D589),A	;[data_D589]
label_D57D:
	LD (data_D584),A	;[data_D584]
	CALL DrawText	;[DrawText]
	db $10
data_D584:
	db $07
	db $FF
	POP BC
	POP AF
	RET
data_D589:
	db $07
function_D58A:
	PUSH AF
	PUSH BC
	LD A,(data_D59F)	;[data_D59F]
	XOR $01
	LD (data_D59F),A	;[data_D59F]
	LD A,(data_D589)	;[data_D589]
	JR Z,label_D59D	;[label_D59D]
	XOR $07
	OR $02
label_D59D:
	JR label_D57D	;[label_D57D]
data_D59F:
	db $00
KeyAsciiLookup:
	db $01
	db $5A
	db $58
	db $43
	db $56
	db $41
	db $53
	db $44
	db $46
	db $47
	db $51
	db $57
	db $45
	db $52
	db $54
	db $31
	db $32
	db $33
	db $34
	db $35
	db $30
	db $39
	db $38
	db $37
	db $36
	db $50
	db $4F
	db $49
	db $55
	db $59
	db $02
	db $4C
	db $4B
	db $4A
	db $48
	db $20
	db $03
	db $4D
	db $4E
	db $42
GetKeyPressedAscii:	;Ascii value of keypress returned in A
	PUSH HL
	PUSH BC
	PUSH DE
	LD DE,START
	LD HL,KeyAsciiLookup	;[KeyAsciiLookup]
	LD BC,$FEFE	;[SmallPlatform3Data + 4438]
label_D5D4:
	IN A,(C)	;read key
	CPL
	AND $1F
	PUSH BC
	LD B,$05
label_D5DC:
	RRCA
	JR NC,label_D5E1	;[label_D5E1]
	LD E,(HL)
	INC D
label_D5E1:
	INC HL
	DJNZ label_D5DC	;[label_D5DC]
	POP BC
	RLC B
	JR C,label_D5D4	;[label_D5D4]
	DEC D
	JR Z,label_D5EF	;[label_D5EF]
	XOR A
	JR label_D5F0	;[label_D5F0]
label_D5EF:
	LD A,E
label_D5F0:
	POP DE
	POP BC
	POP HL
	RET
data_D5F4:
	db $00
data_D5F5:
	db $00
data_D5F6:
	db $00
data_D5F7:
	db $00
	db $00
data_D5F9:
	db $00
	db $00
data_D5FB:
	db $00
	db $00
function_D5FD:
	LD (data_D5F6),A	;[data_D5F6]
	LD (data_D5F4),BC	;[data_D5F4]
	LD HL,data_D5F7	;[data_D5F7]
	LD A,$06
label_D609:
	LD (HL),$03
	INC HL
	DEC A
	JR NZ,label_D609	;[label_D609]
	PUSH BC
	LD B,$0F
label_D612:
	HALT
	DJNZ label_D612	;[label_D612]
	POP BC
	LD HL,(data_D2C6)	;[data_D2C6]
	LD DE,(CurrentScreenNo)	;[CurrentScreenNo]
	LD A,B
	XOR H
	XOR E
	LD (data_D5F7),A	;[data_D5F7]
	XOR L
	XOR C
	LD (data_D5F9),A	;[data_D5F9]
	XOR H
	XOR B
	LD (data_D5FB),A	;[data_D5FB]
	LD HL,data_D5F7	;[data_D5F7]
	LD B,$03
label_D632:
	LD A,(HL)
	AND $3F
label_D635:
	SUB $05
	JR NC,label_D635	;[label_D635]
	ADD A,$0E
	LD (HL),A
	INC HL
	INC HL
	DJNZ label_D632	;[label_D632]
	CALL function_D78B	;[function_D78B]
	LD DE,data_D5F7	;[data_D5F7]
	LD BC,(data_D5F5)	;[data_D5F5]
	LD C,$00
label_D64C:
	LD L,$19
label_D64E:
	PUSH HL
	PUSH BC
	PUSH DE
	CALL function_D55F	;[function_D55F]
	LD A,(data_D5F6)	;[data_D5F6]
	LD B,A
	LD A,(RandomNo1)	;[RandomNo1]
	AND $1F
label_D65D:
	SUB B
	JR NC,label_D65D	;[label_D65D]
	ADD A,B
	RLCA
	RLCA
	LD BC,(data_D5F4)	;[data_D5F4]
	ADD A,C
	LD C,A
	LD HL,$DF40	;[ScanlineTable + 354]
	LD A,(data_D589)	;[data_D589]
	CALL DrawItem	;[DrawItem]
	LD A,(data_DAC1)	;[data_DAC1]
	AND $03
	ADD A,$0C
	CALL function_D7C0	;[function_D7C0]
	POP DE
	POP BC
	POP HL
	DEC L
	JR NZ,label_D64E	;[label_D64E]
	CALL function_D78B	;[function_D78B]
	CALL function_D78B	;[function_D78B]
	PUSH BC
	LD HL,PlayerInventory??	;[PlayerInventory??]
	LD B,$08
label_D68E:
	LD A,B
	AND C
	JR NZ,label_D69B	;[label_D69B]
	LD A,(HL)
	CP $0F
	JR Z,label_D6BF	;[label_D6BF]
	LD A,(DE)
	CP (HL)
	JR Z,label_D6BA	;[label_D6BA]
label_D69B:
	INC HL
	INC HL
	RRC B
	JR NC,label_D68E	;[label_D68E]
	LD HL,PlayerInventory??	;[PlayerInventory??]
	LD B,$08
label_D6A6:
	LD A,B
	AND C
	JR NZ,label_D6AF	;[label_D6AF]
	LD A,(HL)
	CP $0E
	JR Z,label_D6BA	;[label_D6BA]
label_D6AF:
	INC HL
	INC HL
	RRC B
	JR NC,label_D6A6	;[label_D6A6]
	JR label_D71A	;[label_D71A]
	db $00
	db $00
	db $00
label_D6BA:
	db $78
	db $C1
	db $B1
	db $4F
	db $C5
label_D6BF:
	db $D5
	db $CD
	db $5F
	db $D5
	db $3A
	db $89
	db $D5
	db $32
	db $B7
	db $D6
	db $01
	db $BD
	db $D2
	db $B7
	db $ED
	db $42
	db $7D
	db $32
	db $B8
	db $D6
	db $D1
	db $D5
	db $21
	db $F7
	db $D5
	db $EB
	db $B7
	db $ED
	db $52
	db $7D
	db $07
	db $4F
	db $3A
	db $F4
	db $D5
	db $81
	db $32
	db $B9
	db $D6
	db $06
	db $0A
	db $C5
	db $21
	db $40
	db $DF
	db $3A
	db $B7
	db $D6
	db $EE
	db $07
	db $F6
	db $02
	db $32
	db $B7
	db $D6
	db $ED
	db $4B
	db $B8
	db $D6
	db $06
	db $01
	db $CD
	db $24
	db $DB
	db $F5
	db $ED
	db $4B
	db $B9
	db $D6
	db $3A
	db $F5
	db $D5
	db $47
	db $F1
	db $CD
	db $24
	db $DB
	db $3E
	db $03
	db $CD
	db $C0
	db $D7
	db $C1
	db $10
	db $D4
	db $D1
	db $13
	db $3E
	db $07
	db $12
	db $1B
label_D71A:
	PUSH DE
	CALL function_D425	;[function_D425]
	POP DE
	POP BC
	INC DE
	INC DE
	DEC B
	JP NZ,label_D64C	;[label_D64C]
	LD B,$14
label_D728:
	HALT
	DJNZ label_D728	;[label_D728]
	LD HL,data_D5F7	;[data_D5F7]
	LD A,(data_D5F6)	;[data_D5F6]
	LD B,A
	INC HL
	LD A,$07
	CP (HL)
	JP NZ,label_D762	;[label_D762]
	db $23
	db $10
	db $F6
	db $06
	db $23
	db $CD
	db $8A
	db $D5
	db $CD
	db $C1
	db $D3
	db $16
	db $15
	db $07
	db $41
	db $43
	db $43
	db $45
	db $53
	db $53
	db $20
	db $41
	db $55
	db $54
	db $48
	db $4F
	db $52
	db $49
	db $53
	db $45
	db $44
	db $FF
	db $3E
	db $0F
	db $CD
	db $C0
	db $D7
	db $10
	db $DE
	db $AF
	db $C9
label_D762:
	LD B,$28
label_D764:
	CALL function_D58A	;[function_D58A]
	CALL DrawText	;[DrawText]
	db $16
	db $15
	db $06
	db $41
	db $43
	db $43
	db $45
	db $53
	db $53
	db $20
	db $43
	db $4F
	db $44
	db $45
	db $20
	db $49
	db $4E
	db $56
	db $41
	db $4C
	db $49
	db $44
	db $FF
	LD A,$0F
	CALL function_D7C0	;[function_D7C0]
	DJNZ label_D764	;[label_D764]
	LD A,$01
	RET
function_D78B:
	PUSH AF
	PUSH HL
	PUSH BC
	PUSH DE
	LD BC,(data_D5F4)	;[data_D5F4]
	LD DE,data_D5F7	;[data_D5F7]
	LD A,(data_D5F6)	;[data_D5F6]
label_D799:
	PUSH AF
	LD A,(DE)
	INC DE
	PUSH DE
	LD HL,NEXT_CHAR	;[NEXT_CHAR]
	LD E,A
	LD D,$00
	CALL HL_HLxDE	;[HL_HLxDE]
	LD DE,ItemImgData	;[ItemImgData]
	ADD HL,DE
	POP DE
	LD A,(DE)
	INC DE
	CALL DrawItem	;[DrawItem]
	INC C
	INC C
	INC C
	INC C
	POP AF
	DEC A
	JR NZ,label_D799	;[label_D799]
	POP DE
	POP BC
	POP HL
	POP AF
	RET
	db $00
	db $00
	db $00
function_D7C0:
	PUSH HL
	PUSH BC
	PUSH DE
	PUSH IX
	LD D,$00
	LD E,A
	RLCA
	RLCA
	ADD A,E
	LD E,A
	LD HL,$D839	;[label_D82F + 10]
	ADD HL,DE
	PUSH HL
	POP IX
	LD A,(IX+$04)
	AND $1F
label_D7D8:
	PUSH AF
	LD H,(IX+$00)
	LD L,(IX+$02)
	BIT 6,(IX+$04)
	JR Z,label_D7F7	;[label_D7F7]
	LD E,A
	LD A,L
	BIT 5,(IX+$04)
	JR Z,label_D7F0	;[label_D7F0]
	SUB E
	JR label_D7F1	;[label_D7F1]
label_D7F0:
	ADD A,E
label_D7F1:
	CP $00
	LD L,A
	JR NZ,label_D7F7	;[label_D7F7]
	INC L
label_D7F7:
	LD C,$00
label_D7F9:
	LD B,H
label_D7FA:
	LD A,C
	OUT ($FE),A
	XOR $10
	LD C,A
	LD A,H
	AND B
	XOR (IX+$03)
	LD D,A
	BIT 7,(IX+$04)
	JR Z,label_D813	;[label_D813]
	LD A,D
	SRL A
	SUB H
	AND $3F
	LD D,A
label_D813:
	PUSH IX
	POP IX
	DEC D
	JR NZ,label_D813	;[label_D813]
	LD A,B
	SUB L
	JR C,label_D821	;[label_D821]
	LD B,A
	JR label_D7FA	;[label_D7FA]
label_D821:
	LD A,H
	CP (IX+$01)
	JR Z,label_D82F	;[label_D82F]
	JR C,label_D82C	;[label_D82C]
	DEC H
	JR label_D7F9	;[label_D7F9]
label_D82C:
	INC H
	JR label_D7F9	;[label_D7F9]
label_D82F:
	POP AF
	DEC A
	JR NZ,label_D7D8	;[label_D7D8]
	POP IX
	POP DE
	POP BC
	POP HL
	RET
	db $3F
	db $30
	db $01
	db $00
	db $81
	db $00
	db $7F
	db $FE
	db $00
	db $01
	db $C5
	db $C4
	db $03
	db $01
	db $C1
	db $01
	db $7F
	db $7F
	db $01
	db $41
	db $01
	db $14
	db $FF
	db $01
	db $41
	db $28
	db $22
	db $01
	db $7F
	db $FF
	db $32
	db $38
	db $FE
	db $05
	db $C3
	db $F0
	db $F1
	db $28
	db $01
	db $DE
	db $1E
	db $00
	db $01
	db $01
	db $C3
	db $8C
	db $80
	db $01
	db $7F
	db $C3
	db $00
	db $22
	db $01
	db $7F
	db $DF
	db $22
	db $00
	db $28
	db $7F
	db $DF
	db $20
	db $00
	db $14
	db $00
	db $81
	db $C8
	db $C9
	db $FE
	db $05
	db $03
	db $00
	db $0A
	db $FE
	db $0F
	db $07
	db $00
	db $03
	db $04
	db $17
	db $FF
	db $1E
	db $00
	db $07
	db $00
	db $41
	db $14
	db $0A
	db $FE
	db $00
	db $6A
	db $40
	db $00
	db $FF
	db $01
	db $81
	db $FF
	db $FE
	db $FF
	db $FF
	db $C1
	db $0A
	db $01
	db $FF
	db $00
	db $01
	db $04
	db $00
	db $FF
	db $14
	db $01
	db $07
	db $0A
	db $FF
	db $00
	db $01
	db $00
	db $00
	db $00
	db $00
	db $00
function_D8B1:
	LD B,$06	;sprite counter
	LD HL,Sprite0_XPixelPos	;[Sprite0_XPixelPos]	;point at x pos of sprite
SpriteCheckLoop:
	PUSH BC
	PUSH HL
	LD C,(HL)	;get X pixel pos
	INC HL
	LD B,(HL)	;get y pixel pos
	INC HL	;skip image ptr 2
	INC HL
	INC HL	;skip byte
	LD E,(HL)	;read colour and put in E
	LD A,B	;put y coord in a
	OR C	;or with x
	CP $10	;compare to 16
	JR C,label_D934	;[label_D934]
	LD A,$BF
	SUB B
	RLCA
	RLCA
	LD L,A
	AND $03
	OR $58
	LD H,A
	LD A,L
	AND $E0
	LD L,A
	LD A,C
	AND $F8
	RRCA
	RRCA
	RRCA
	OR L
	LD L,A
	LD D,$00
	LD A,C
	AND $07
	JR NZ,label_D8E4	;[label_D8E4]
	INC D
label_D8E4:
	LD A,B
	INC A
	AND $07
	JR NZ,label_D8EC	;[label_D8EC]
	SET 1,D
label_D8EC:
	LD B,$F8
	LD C,$20
	LD A,(HL)
	AND C
	JR NZ,label_D8F8	;[label_D8F8]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D8F8:
	INC HL
	LD A,(HL)
	AND C
	JR NZ,label_D901	;[label_D901]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D901:
	INC HL
	BIT 0,D
	JR NZ,label_D90E	;[label_D90E]
	LD A,(HL)
	AND C
	JR NZ,label_D90E	;[label_D90E]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D90E:
	PUSH DE
	LD DE,NEXT_CHAR	;[NEXT_CHAR]
	ADD HL,DE
	POP DE
	BIT 0,D
	JR NZ,label_D920	;[label_D920]
	LD A,(HL)
	AND C
	JR NZ,label_D920	;[label_D920]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D920:
	DEC HL
	LD A,(HL)
	AND C
	JR NZ,label_D929	;[label_D929]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A	;Write attribute
label_D929:
	DEC HL
	LD A,(HL)
	AND C
	JR NZ,label_D932	;[label_D932]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D932:
	BIT 1,D
label_D934:
	JR NZ,label_D95A	;[label_D95A]
	PUSH DE
	LD DE,NEXT_CHAR	;[NEXT_CHAR]
	ADD HL,DE
	POP DE
	LD A,(HL)
	AND C
	JR NZ,label_D944	;[label_D944]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D944:
	INC HL
	LD A,(HL)
	AND C
	JR NZ,label_D94D	;[label_D94D]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D94D:
	BIT 0,D
	JR NZ,label_D95A	;[label_D95A]
	INC HL
	LD A,(HL)
	AND C
	JR NZ,label_D95A	;[label_D95A]
	LD A,(HL)
	AND B
	OR E
	LD (HL),A
label_D95A:
	POP HL
	POP BC
	LD DE,NEXT_CHAR	;[NEXT_CHAR]	;make HL point to next sprite
	ADD HL,DE
	DEC B	;decrement sprite counter
	JP NZ,SpriteCheckLoop	;[SpriteCheckLoop]
	LD HL,AttributeDrawList	;[AttributeDrawList]
	LD B,$3F	;mask - bottom 6 bits
label_D969:
	LD E,(HL)	;read DE from where HL is pointing
	INC HL
	LD D,(HL)
	INC HL
	XOR A	;clear A
	OR D
	RET Z	;return if D is zero
	LD A,(HL)	;value
	INC HL
	AND B	;mask bottom 6 bits
	LD C,A
	LD A,(DE)	;read in attribute byte
	AND $C0	;mask top 2 bits - flash & bright
	OR C	;or in attribute
	LD (DE),A	;write item attr
	JR label_D969	;[label_D969]
function_D97B:
	LD H,A
	LD A,B
	RRCA
	RRCA
	RRCA
	LD B,A
	AND $E0
	OR C
	LD E,A
	LD A,B
	AND $03
	OR $58
	LD D,A
	LD C,H
	LD B,$35
	LD HL,$5BBE	;[AttributeDrawList + 158]
label_D991:
	LD A,(HL)
	DEC HL
	CP C
	JR NZ,label_D9C3	;[label_D9C3]
	LD A,(HL)
	DEC HL
	CP D
	JR NZ,label_D9C4	;[label_D9C4]
	LD A,(HL)
	CP E
	JR NZ,label_D9C4	;[label_D9C4]
	EX DE,HL
	LD HL,$5BDF	;[AttributeDrawList + 191]
	OR A
	SBC HL,DE
	LD B,H
	LD C,L
	LD HL,$000C	;[ERROR_1 + 4]
	ADD HL,DE
	LDIR
	LD HL,(AttributeDrawListPtr)	;[AttributeDrawListPtr]
	LD DE,GET_CHAR	;[GET_CHAR]
	OR A
	SBC HL,DE
	LD (AttributeDrawListPtr),HL	;[AttributeDrawListPtr]
label_D9BA:
	LD (HL),$00
	INC HL
	LD A,L
	CP $FF
	JR C,label_D9BA	;[label_D9BA]
	RET
label_D9C3:
	DEC HL
label_D9C4:
	DEC HL
	DJNZ label_D991	;[label_D991]
	RET
UpdateGame:
	CALL function_A418	;[function_A418]
	CALL DrawSpriteList	;[DrawSpriteList]
	CALL function_D8B1	;[function_D8B1]
	CALL UpdatePlayerPlatforms	;[UpdatePlayerPlatforms]
	CALL GenerateRandomNumbers	;[GenerateRandomNumbers]
	CALL GetTravelTubeScreenOffset	;[GetTravelTubeScreenOffset]
	CALL UpdateElectricalHazardGraphics	;[UpdateElectricalHazardGraphics]
	RET
label_D9DE:
	DI
	LD (data_DA03),HL	;[data_DA03]
	INC HL
label_D9E3:
	LD A,(HL)
	INC HL
	PUSH HL
	CP $00
	JR Z,label_DA35	;[label_DA35]
	LD C,A
	AND $1F
	LD H,$00
	LD L,A
	ADD HL,HL
	LD DE,$DA70	;[Audio_Byte + 1]
	ADD HL,DE
	LD E,(HL)
	INC HL
	LD D,(HL)
	PUSH DE
	PUSH DE
	PUSH DE
	LD H,$00
	LD A,C
	OR $1F
	LD L,A
	LD DE,(data_6A34)	;[data_6A34]
data_DA03:
	LD D,H
	CALL HL_HLxDE	;[HL_HLxDE]
	POP DE
	LD BC,START
label_DA0D:
	INC BC
	INC BC
	INC BC
	INC BC
	OR A
	SBC HL,DE
	JR NC,label_DA0D	;[label_DA0D]
	POP HL
	POP DE
	EXX
	LD BC,$FEFE	;[SmallPlatform3Data + 4438]
	EXX
label_DA1D:
	LD A,(Audio_Byte)	;[Audio_Byte]
	OUT ($FE),A
	ADD A,$10
	AND $30
	LD (Audio_Byte),A	;[Audio_Byte]
	EX AF,AF'
	EXX
	IN A,(C)
	RLC B
	EXX
	CPL
	AND $1F
	JR Z,label_DA38	;[label_DA38]
label_DA35:
	POP HL
	EI
	RET
label_DA38:
	EX DE,HL
	PUSH BC
	PUSH HL
label_DA3B:
	DEC HL
	LD A,H
	OR L
	JR NZ,label_DA3B	;[label_DA3B]
	POP HL
data_DA41:
	DEC HL
	LD BC,$0002	;[START + 2]
	LD A,H
	CP B
	JR NZ,label_DA4B	;[label_DA4B]
	LD A,L
	CP C
label_DA4B:
	JR NZ,label_DA53	;[label_DA53]
	CALL function_DA64	;[function_DA64]
	INC DE
	JR label_DA57	;[label_DA57]
label_DA53:
	LD B,$06
label_DA55:
	DJNZ label_DA55	;[label_DA55]
label_DA57:
	CALL function_DA64	;[function_DA64]
	POP BC
	DEC BC
	LD A,B
	OR C
	JR NZ,label_DA1D	;[label_DA1D]
	POP HL
	JP label_D9E3	;[label_D9E3]
function_DA64:
	LD A,(data_DA41)	;[data_DA41]
	LD C,A
	LD A,$4E
	SUB C
	LD (data_DA41),A	;[data_DA41]
	RET
Audio_Byte:
	db $10
	db $5D
	db $01
	db $49
	db $01
	db $36
	db $01
	db $24
	db $01
	db $12
	db $01
	db $02
	db $01
	db $F3
	db $00
	db $E4
	db $00
	db $D7
	db $00
	db $CA
	db $00
	db $BE
	db $00
	db $B2
	db $00
	db $A7
	db $00
	db $9D
	db $00
	db $94
	db $00
	db $8B
	db $00
	db $82
	db $00
	db $7A
	db $00
	db $72
	db $00
	db $6B
	db $00
	db $64
	db $00
	db $5E
	db $00
	db $58
	db $00
	db $52
	db $00
	db $4D
	db $00
	db $47
	db $00
	db $43
	db $00
	db $3E
	db $00
	db $3A
	db $00
	db $36
	db $00
	db $32
	db $00
	db $20
	db $4C
	db $44
	db $20
	db $20
	db $20
	db $44
	db $45
	db $2C
	db $4D
	db $55
	db $53
	db $4E
	db $4F
	db $54
	db $45
	db $53
	db $0D
RandomNo1:	;high frequency
	dw $8538
data_DAC1:
RandomNo2:	;mid frequency
	dw $A781
data_DAC3:
RandomNo3:	;low frequency
	dw $D0A7
	;This generates several random number and is called all over the code base
GenerateRandomNumbers:
	PUSH AF
	PUSH HL
	PUSH BC
	LD HL,(RandomNo1)	;[RandomNo1]	;read first 2 random numbers
	PUSH HL	;backup old number
	LD A,L	;swap low & high bytes
	LD L,H
	LD H,A
	POP BC	;restore old no to BC
	ADD HL,BC	;add to new number
	LD BC,$0029	;[FP_CALC + 1]
	ADD HL,BC	;add 29h
	LD BC,(RandomNo2)	;[RandomNo2]	;get second number
	ADD HL,BC	;add to first
	LD (RandomNo1),HL	;[RandomNo1]	;write back first number
	LD HL,RNGCounter1	;[RNGCounter1]	;read address
	DEC (HL)	;decrement contents
	JR NZ,ExitRNG	;[ExitRNG]	;exit if not 0
	LD (HL),$05	;reset to 5
	LD HL,(RandomNo2)	;[RandomNo2]	;load random no 2
	PUSH HL	;backup
	ADD HL,HL	;mult by 16
	ADD HL,HL
	ADD HL,HL
	ADD HL,HL
	POP BC	;restore old
	ADD HL,BC	;add original
	LD BC,$00C5	;[TOKENS + 48]
	ADD HL,BC	;add C5h
	LD BC,(RandomNo3)	;[RandomNo3]	;get random number 3
	ADD HL,BC	;add
	LD (RandomNo2),HL	;[RandomNo2]	;write back
	LD HL,RNGCounter2	;[RNGCounter2]	;load counter
	DEC (HL)	;decrement counter
	JR NZ,ExitRNG	;[ExitRNG]	;exit if not zero
	LD (HL),$0B	;reset to 11
	LD HL,(RandomNo3)	;[RandomNo3]
	LD BC,(RandomNo1)	;[RandomNo1]
	ADD HL,HL	;double random number 3 
	ADD HL,BC	;add random number 1
	ADD HL,HL	;double again
	LD BC,$4BBB	;[label_4053 + 2920]
	ADD HL,BC	;add 4bbbh
	LD (RandomNo3),HL	;[RandomNo3]
ExitRNG:
	POP BC
	POP HL
	POP AF
	RET
RNGCounter1:
	db $05
RNGCounter2:
	db $03
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
DrawItem:	;HL - src image C - X, B- Y, A attribute
	PUSH HL
	PUSH DE
	PUSH BC
	CALL DrawItemChar	;[DrawItemChar]
	INC C
	CALL DrawItemChar	;[DrawItemChar]
	INC B
	DEC C
	CALL DrawItemChar	;[DrawItemChar]
	INC C
	CALL DrawItemChar	;[DrawItemChar]
	POP BC
	POP DE
	POP HL
	RET
	;Draw an 8x8 item character square
	;HL points to the image data
	;C - X Position
	;B - Y Position
DrawItemChar:	;HL image data, C - x pos B - y pos
	PUSH AF
	LD A,B
	RRCA	;div A by 8
	RRCA
	RRCA
	AND $E0	;mask off top 3 bits
	ADD A,C
	LD E,A	;low byte of screen address
	LD A,B
	AND $18	;mask bits 3&4
	OR $40	;or in bit 6
	LD D,A	;high byte of screen address
	LD A,$08	;set line count 8
	EX DE,HL
ItemDrawLoop:
	PUSH AF
	LD A,(DE)	;read pixel line
	INC DE	;inc src
	XOR (HL)	;XOR in
	LD (HL),A	;write pixel line
	INC H	;inc pixel line
	POP AF
	DEC A	;dec count
	JR NZ,ItemDrawLoop	;[ItemDrawLoop]
	DEC H
	LD A,H
	RRCA	;shift down 3 - div 8
	RRCA
	RRCA
	AND $03	;mask bottom 2 bits
	OR $58	;or into high bit of attribute address
	LD H,A
	POP AF	;pop attrib
	PUSH AF	;backup
	AND $80	;check high bit (flash)
	JR NZ,DrawItemAttribute	;[DrawItemAttribute]
	POP AF	;restore attrib
	LD (HL),A	;write to screen
	EX DE,HL	;put address in DE
	BIT 6,A	;check attrib for blue
	JR Z,label_DB7C	;[label_DB7C]
	PUSH HL
	LD HL,(AttributeDrawListPtr)	;[AttributeDrawListPtr]
	LD (HL),E	;write attribute address (DE) to list
	INC HL
	LD (HL),D
	INC HL
	LD (HL),A	;write attribute value to list
	INC HL
	LD (AttributeDrawListPtr),HL	;[AttributeDrawListPtr]	;update attribute draw list pointer
	POP HL
label_DB7C:
	RET
DrawPlatformWithDelay:	;L is platform no
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
DrawPlatform:	;L is platform number, A - attribute
	OR $80
	PUSH HL
	PUSH DE
	PUSH BC
	LD H,A
	LD A,L	;put platform number in A
	RLCA	;mult by 16
	RLCA
	RLCA
	RLCA
	LD E,A
	LD A,H
	LD D,$00
	LD HL,PlatformSpriteData	;[PlatformSpriteData]
	ADD HL,DE
	CALL DrawItemChar	;[DrawItemChar]
	INC C
	CALL DrawItemChar	;[DrawItemChar]
	POP BC
	POP DE
	POP HL
	RET
DrawItemAttribute:
	POP AF
	PUSH AF
	AND $7F
	JR Z,label_DBB0	;[label_DBB0]
	CP $40	;check bright bit
	JR NZ,ItemAtribSet	;[ItemAtribSet]
label_DBB0:
	AND $40
	RES 6,(HL)
	OR (HL)
ItemAtribSet:
	LD (HL),A
	EX DE,HL
	POP AF
	RET
	db $00
PlatformTimer:
	db $0A
PlatformStates:	;list of platform states - 4 bytes each
	db $00	;lower bits contain X, higher bits contain platform no
	db $00
	db $00
	db $00	;platform timeout
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
UpdatePlayerPlatforms:
	LD A,(PlatformTimer)	;[PlatformTimer]
	INC A	;count timer up to 12 and then reset
	CP $0C
	JR C,label_DBF5	;[label_DBF5]
	XOR A	;reset timer
label_DBF5:
	LD (PlatformTimer),A	;[PlatformTimer]
	RLCA	;mult timer by 4
	RLCA
	LD E,A	;put in E
	LD D,$00
	LD HL,PlatformStates	;[PlatformStates]
	ADD HL,DE
	LD A,(HL)	;load first byte
	AND $1F	;mask bottom 5 bits - x value
	LD C,A	;put result in C
	INC HL	;inc ptr
	LD A,(HL)	;load next byte - height
	CP $00	;check if 0
	RET Z	;exit if it is
	PUSH AF
	RLCA
	AND $01
	LD D,A
	POP AF
	AND $7F
	LD B,A
	INC HL
	LD E,(HL)	;load next byte
	INC HL
	LD A,(HL)	;decrement timer
	DEC A
	LD (HL),A
	DEC HL	;wind pointer back to start of platform state
	DEC HL
	DEC HL
	CP $04	;check timer
	JR NC,label_DC2D	;[label_DC2D]
	LD A,(HL)	;increment platform number
	ADD A,$20
	LD (HL),A
	AND $E0	;mask top 3 bits
	XOR $80	;check high bit
	JR NZ,label_DC2D	;[label_DC2D]
	INC HL
	LD (HL),A	;store 0 in ypos, retiring platform
	DEC HL	;point back at X
label_DC2D:
	NOP
	LD A,(HL)	;get platform number
	RLCA
	RLCA
	RLCA
	AND $07
	RET Z	;return if platform 0
	LD L,A	;platform number in L
	CP $04	;platform 4?
	LD A,$00
	JR NZ,label_DC3E	;[label_DC3E]
	LD A,$40	;set A to 40 if it is 4
label_DC3E:
	DEC L
	CALL DrawPlatformWithDelay	;[DrawPlatformWithDelay]
	RET
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
PlatformSpriteData:
	db $60
	db $C0
	db $07
	db $1C
	db $0A
	db $02
	db $03
	db $00
	db $0C
	db $06
	db $00
	db $00
	db $00
	db $04
	db $58
	db $00
	db $D8
	db $D5
	db $43
	db $0C
	db $00
	db $00
	db $18
	db $00
	db $0A
	db $B5
	db $E8
	db $38
	db $50
	db $44
	db $F0
	db $10
	db $C8
	db $F2
	db $41
	db $08
	db $09
	db $05
	db $00
	db $08
	db $0B
	db $1C
	db $00
	db $20
	db $A0
	db $20
	db $50
	db $20
	db $80
	db $10
	db $02
	db $00
	db $08
	db $01
	db $00
	db $04
	db $02
	db $40
	db $08
	db $00
	db $20
	db $00
	db $20
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
	db $02
	db $02
	db $47
	db $67
	db $3D
	db $18
	db $10
	db $00
	db $08
	db $1C
	db $14
	db $36
	db $A2
	db $E0
	db $C0
	db $40
	db $02
	db $12
	db $56
	db $5E
	db $56
	db $16
	db $16
	db $04
	db $88
	db $DC
	db $D0
	db $50
	db $8C
	db $D8
	db $D8
	db $50
	db $08
	db $1C
	db $0D
	db $6F
	db $19
	db $2D
	db $27
	db $05
	db $80
	db $E0
	db $A6
	db $10
	db $FA
	db $A4
	db $10
	db $10
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
GetTravelTubeScreenOffset:	;Screen offset in BC
	LD DE,(TravelTubeCounter)	;[TravelTubeCounter]
	INC E
	LD A,E
	ADD A,$D0
	JR NC,label_DCF5	;[label_DCF5]
	LD E,A
	LD A,$07
	XOR D
	LD D,A
label_DCF5:
	LD (TravelTubeCounter),DE	;[TravelTubeCounter]
	LD A,E	;counter low byte
	RLCA	;mult by 2
	LD C,A	;put in C
	XOR A	;clear A
	LD B,A	;put in B
	LD HL,TransportTubeCoordTable	;[TransportTubeCoordTable]
	ADD HL,BC	;offset by counter based offset
	LD C,(HL)	;x coord in C
	INC HL
	LD A,(HL)	;y coord in A
	RRCA	;rotate bottom 3 bits to top
	RRCA
	RRCA
	LD B,A	;store in B
	AND $E0	;mask top 3 bits
	OR C	;or top 3 bits with X coord
	RET Z
	LD L,A
	LD A,B
	AND $03
	OR $58
	LD H,A
	LD (HL),D
	INC HL
	LD (HL),D
	RET
Sprite0:	;32 byte struct
	dw $DEEE	;0-1 - scanline address
Sprite0_SpriteAddr:
	dw $E614	;1-2
Sprite0_XCharPos:
	db $04	;3
Sprite0_XPixelPos:
	db $24	;4
Sprite0_YPixelPos:
	db $27	;5
Sprite0_SpriteAddr2:
	dw $E5B4	;6-7
Sprite0_Col:
	db $07	;8
Sprite0_OnTransport:
	db $00	;9
CurrPlayerControl:
	db $00	;10
LastCtrlPlayerCtrl:
	db $02	;11
Sprite0_AnimFrame:
	db $03
data_DD26:
	db $04
data_DD27:
	db $00
data_DD28:
	db $02
data_DD29:
	db $00
data_DD2A:
	db $00
data_DD2B:
	db $02
DownHeld:
	db $00
data_DD2D:
	db $00
data_DD2E:
	db $00
	db $00
	db $51
data_DD31:
	db $00
data_DD32:
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
Sprite1:
	dw $DE72
	dw $BDD8
	db $04
	db $26
	db $65
	dw $BD48
	db $02
	db $02
	db $67
	db $00
	db $01
	db $48
	db $BD
	db $00
	db $02
	db $02
	db $04
	db $04
	db $01
	db $00
	db $64
	db $02
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
Sprite2:
	db $EA
	db $DE
	dw $BDA8
	db $05
	db $2C
	db $29
	dw $BD48
	db $02
	db $02
	db $37
	db $00
	db $09
	db $48
	db $BD
	db $00
	db $02
	db $02
	db $04
	db $03
	db $01
	db $00
	db $64
	db $01
	db $04
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
Sprite3:
	dw $DEB6
	dw $BD78
	db $04
	db $23
	db $43
	dw $BD48
	db $02
	db $50
	db $8D
	db $00
	db $09
	db $48
	db $BD
	db $00
	db $01
	db $02
	db $04
	db $01
	db $01
	db $00
	db $64
	db $02
	db $03
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
Sprite4:
	dw $DE46
	dw $BD48
Sprite4_XCharPos:
	db $09
Sprite4_XPixelPos:
	db $48
Sprite4_YPixelPos:
	db $7B
data_DD9F:
	dw $BD48
data_DDA1:
	db $02
	db $28
	db $8D
	db $00
	db $01
	db $48
	db $BD
	db $00
	db $02
	db $02
data_DDAB:
	db $04
	db $03
	db $01
	db $00
	db $64
	db $02
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
Sprite5:
	dw $DF1E
	dw $DF40
	db $00
Sprite5_XPixelPos:
	db $00
Sprite5_YPixelPos:
	db $0F
Sprite5_SpriteAddr2:
	dw $DF40
Sprite5_InkCol:
	db $07
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
ScanlineTable:
	dw $4040	;Scanline addr table
	dw $4140
	dw $4240
	dw $4340
	dw $4440
	dw $4540
	dw $4640
	dw $4740
	dw $4060
	dw $4160
	dw $4260
	dw $4360
	dw $4460
	dw $4560
	dw $4660
	dw $4760
	dw $4080
	dw $4180
	dw $4280
	dw $4380
	dw $4480
	dw $4580
	dw $4680
	dw $4780
	dw $40A0
	dw $41A0
	dw $42A0
	dw $43A0
	dw $44A0
	dw $45A0
	dw $46A0
	dw $47A0
	dw $40C0
	dw $41C0
	dw $42C0
	dw $43C0
	dw $44C0
	dw $45C0
	dw $46C0
	dw $47C0
	dw $40E0
	dw $41E0
	dw $42E0
	dw $43E0
	dw $44E0
	dw $45E0
	dw $46E0
	dw $47E0
	dw $4800
	dw $4900
	dw $4A00
	db $00
	db $4B
	db $00
	db $4C
	db $00
	db $4D
	db $00
	db $4E
	db $00
	db $4F
	db $20
	db $48
	db $20
	db $49
	db $20
	db $4A
	db $20
	db $4B
	db $20
	db $4C
	db $20
	db $4D
	db $20
	db $4E
	db $20
	db $4F
	db $40
	db $48
	db $40
	db $49
	db $40
	db $4A
	db $40
	db $4B
	db $40
	db $4C
	db $40
	db $4D
	db $40
	db $4E
	db $40
	db $4F
	db $60
	db $48
	db $60
	db $49
	db $60
	db $4A
	db $60
	db $4B
	db $60
	db $4C
	db $60
	db $4D
	db $60
	db $4E
	db $60
	db $4F
	db $80
	db $48
	db $80
	db $49
	db $80
	db $4A
	db $80
	db $4B
	db $80
	db $4C
	db $80
	db $4D
	db $80
	db $4E
	db $80
	db $4F
	db $A0
	db $48
	db $A0
	db $49
	db $A0
	db $4A
	db $A0
	db $4B
	db $A0
	db $4C
	db $A0
	db $4D
	db $A0
	db $4E
	db $A0
	db $4F
	db $C0
	db $48
	db $C0
	db $49
	db $C0
	db $4A
	db $C0
	db $4B
	db $C0
	db $4C
	db $C0
	db $4D
	db $C0
	db $4E
	db $C0
	db $4F
	db $E0
	db $48
	db $E0
	db $49
	db $E0
	db $4A
	db $E0
	db $4B
	db $E0
	db $4C
	db $E0
	db $4D
	db $E0
	db $4E
	db $E0
	db $4F
	db $00
	db $50
	db $00
	db $51
	db $00
	db $52
	db $00
	db $53
	db $00
	db $54
	db $00
	db $55
	db $00
	db $56
	db $00
	db $57
	db $20
	db $50
	db $20
	db $51
	db $20
	db $52
	db $20
	db $53
	db $20
	db $54
	db $20
	db $55
	db $20
	db $56
	db $20
	db $57
	db $40
	db $50
	db $40
	db $51
	db $40
	db $52
	db $40
	db $53
	db $40
	db $54
	db $40
	db $55
	db $40
	db $56
	db $40
	db $57
	dw $5060
	dw $5160
	dw $5260
	dw $5360
	dw $5460
	dw $5560
	dw $5660
	db $60
	db $57
	db $80
	db $50
	db $80
	db $51
	db $80
	db $52
	db $80
	db $53
	db $80
	db $54
	db $80
	db $55
	db $80
	db $56
	db $80
	db $57
	db $A0
	db $50
	db $A0
	db $51
	db $A0
	db $52
	db $A0
	db $53
	db $A0
	db $54
	db $A0
	db $55
	db $A0
	db $56
	db $A0
	db $57
	db $C0
	db $50
	db $C0
	db $51
	db $C0
	db $52
	db $C0
	db $53
	db $C0
	db $54
	db $C0
	db $55
	db $C0
	db $56
	db $C0
	db $57
	db $E0
	db $50
	db $E0
	db $51
	db $E0
	db $52
	db $E0
	db $53
	db $E0
	db $54
	db $E0
	db $55
	db $E0
	db $56
	db $E0
	db $57
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
DrawSpriteList:	;Draw the sprite list
	NOP
	XOR A	;A = 0
label_DF72:
	PUSH AF
	RRCA	;Point HL at DD18 + A * 32
	RRCA
	RRCA
	ADD A,$18
	LD L,A
	LD H,$DD
	PUSH HL
	LD C,(HL)	;put first 16 bytes into BC
	INC L
	LD B,(HL)
	INC L
	LD E,(HL)	;put second 16 bytes into DE - src image
	INC L
	LD D,(HL)
	INC L
	LD A,(HL)	;put x char pos in A
	INC L
	PUSH HL	;put HL onto stack
	LD (BlobStackBkp),SP	;[BlobStackBkp]	;backup stack
	LD H,B	;put BC into HL and make it the stack pointer
	LD L,C
	LD SP,HL
	LD B,$04	;set loop count to 4 because loop is unrolled - 16 scanlines
	LD C,A	;X Coord?
SpriteEraseLoop:
	POP HL	;pop scanline address 1
	LD A,C
	ADD A,L	;add X char pos
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	POP HL	;pop scanline address 2
	LD A,C
	ADD A,L	;add x char offset
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	POP HL	;pop scanline address 3
	LD A,C
	ADD A,L
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	POP HL	;pop scanline address 4
	LD A,C
	ADD A,L
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	DJNZ SpriteEraseLoop	;[SpriteEraseLoop]
	LD SP,(BlobStackBkp)	;[BlobStackBkp]	;restore stack bkp
	POP HL	;HL points to X screen pos
	LD A,(HL)	;x screen pos
	EX AF,AF'
	LD A,(HL)	; x screen pos
	EX AF,AF'
	INC L
	LD D,$00
	LD E,(HL)	;y screen pos
	INC L
	LD C,(HL)	;BC points to sprite frame
	INC L
	LD B,(HL)
	AND $06	;mask bits 1 & 2 of x screen pos
	RLCA	;mult by 8
	RLCA
	RLCA
	LD L,A	;store A * 8
	RLCA
	ADD A,L	;add A * 8 to A * 16 = A * 24
	LD L,A
	LD H,D
	ADD HL,BC	;add to blob frame
	EX DE,HL
	LD A,$AF
	SUB L
	LD L,A
	ADD HL,HL
	LD BC,ScanlineTable	;[ScanlineTable]
	ADD HL,BC
	LD B,H
	LD C,L
	EX AF,AF'
	AND $F8
	RRCA
	RRCA
	RRCA
	POP HL
	LD (HL),C	;put new scanline table pointer
	INC L
	LD (HL),B
	INC L
	LD (HL),E	;put new sprite frame
	INC L
	LD (HL),D
	INC L
	LD (HL),A
	LD H,B	;BC->HL
	LD L,C
	LD SP,HL	;set stack pointer to scanline table entry
	LD B,$04
	LD C,A	;x char pos
SpriteDrawLoop:
	POP HL	;pop screen address off stack
	LD A,C	;put x val in A
	ADD A,L	;add low address
	LD L,A	;put result in L
	LD A,(DE)	;read pixel line
	XOR (HL)	;XOR with BG
	LD (HL),A	;Write blob pixel line A
	INC L	;Increment screen X pos by 1 byte - 8px
	INC DE	;Increment src ptr
	LD A,(DE)	;Read pixel Line B
	XOR (HL)	;XOR with BG
	LD (HL),A	;Write blob pixel line B
	INC L	;inc screen X by 8px
	INC DE	;inc src ptr
	LD A,(DE)	;read pixel line C
	XOR (HL)	;XOR with BG
	LD (HL),A	;Write blob pixel line C
	INC DE
	POP HL	;pop screen address off stack
	LD A,C
	ADD A,L
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	POP HL	;pop screen address off stack
	LD A,C
	ADD A,L
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	POP HL	;pop address off stack
	LD A,C
	ADD A,L
	LD L,A
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC L
	INC DE
	LD A,(DE)	;read pixel line
	XOR (HL)
	LD (HL),A	;write pixel line
	INC DE
	DJNZ SpriteDrawLoop	;[SpriteDrawLoop]
	LD SP,(BlobStackBkp)	;[BlobStackBkp]
	POP HL
	POP HL
	POP AF
	INC A
	CP $06
	JP NZ,label_DF72	;[label_DF72]
	RET
BlobStackBkp:
	dw $5DFF
	db $C9
PlayerSpriteData:
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
	3 Bytes
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
	db $00
	db $00
	db $00
	db $06
	db $0C
	db $0E
	db $1B
	db $2E
	db $25
	db $C3
	db $65
	db $EA
	db $00
AttributeDrawListPtr:	;Current attribute draw list pointer
	dw $0316
data_EA61:
PlatformRandColour:
	db $06
PlatformLUTColour:
	db $05
	db $00
DrawSmallPlatform:	;L - platform number, B - YPos, C - XPos
	PUSH IX
	PUSH BC
	PUSH DE
	PUSH HL
	LD A,(RandomNo1)	;[RandomNo1]
	AND $07	;mask out ink bits
	CP $02	;check for 2 (red)
	JR NC,SmlPltWriteColour	;[SmlPltWriteColour]	;jump if greater
	LD A,(RandomNo1)	;[RandomNo1]	;try again - why?
	AND $07	;mask out ink vals
	CP $02
	JR NC,SmlPltWriteColour	;[SmlPltWriteColour]	;jump if greater
	LD A,B	;put B(?) into A
	AND $07	;mask ink bits
	OR $02	;ensure >= red
SmlPltWriteColour:
	LD (PlatformRandColour),A	;[PlatformRandColour]	;set col
	LD DE,SmallPlatformsDataLUT	;[SmallPlatformsDataLUT]
	LD H,$00	;HL contains platform number
	ADD HL,HL	;multiply by 2 to make offset into address LUT
	ADD HL,DE	;Add to base offset
	LD E,(HL)	;put platform address in DE
	INC HL
	LD D,(HL)
	PUSH DE	;push platform address
	POP IX	;pop into IX
	LD HL,$0006	;[START + 6]
	ADD HL,DE	;add 6 to platform address to point to colour data
	DEC DE	;dec 1 from platform address to get col data
	PUSH DE
	EXX
	POP DE
	EXX
	LD D,$06	;vertical loop count
SmPltVerticalCharLoop:
	LD A,(IX+$00)	;load mask
	LD E,$08	;horizontal loop
SmPltHorizontalCharLoop:
	RLCA	;shift mask
	CALL C,DrawSmallPlatformChar	;[DrawSmallPlatformChar]	;draw if bit set
	INC C	;increment X
	DEC E	;dec counter
	JR NZ,SmPltHorizontalCharLoop	;[SmPltHorizontalCharLoop]
	INC B	;increment B
	LD A,C	;subtract 8 from X to put it back at line start
	SUB $08
	LD C,A
	INC IX	;increment mask pointer
	DEC D	;dec vertical counter
	JR NZ,SmPltVerticalCharLoop	;[SmPltVerticalCharLoop]
	POP HL
	POP DE
	POP BC
	POP IX
	RET
DrawSmallPlatformChar:	;C - X pos, B - Y pos, HL - src data, DE - colour ptr
	PUSH BC
	PUSH DE
	EX AF,AF'
	LD A,B	;y pos
	RRCA
	RRCA
	RRCA
	AND $E0
	ADD A,C
	LD C,A
	LD A,B
	AND $18
	OR $40
	LD B,A
	LD D,$08	;set counter to 8
SmlPltPixelRowLoop:
	LD A,(HL)	;read pixel row
	LD (BC),A	;write pixel row
	INC HL
	INC B
	DEC D
	JR NZ,SmlPltPixelRowLoop	;[SmlPltPixelRowLoop]
	DEC B
	LD A,B
	RRCA
	RRCA
	RRCA
	AND $03
	OR $58
	LD B,A
	EXX
	LD A,(DE)	;read colour value
	DEC DE	;dec col pointer
	EXX
	LD D,A	;backup attrib value
	AND $3F	;mask out bottom 6 bits (ink/paper)
	CP $36	;check for 36
	JR Z,SmlPltProcessAttrib36h	;[SmlPltProcessAttrib36h]
	CP $00
	LD A,D	;put old attrib val back in
	JR NZ,SmlPltWritePlatformAttr	;[SmlPltWritePlatformAttr]
	LD A,D	;get original val
	AND $F8	;mask out bottom 3 bits (ink)
	LD D,A	;put back in D
	LD A,(PlatformRandColour)	;[PlatformRandColour]
	OR D
	JR SmlPltWritePlatformAttr	;[SmlPltWritePlatformAttr]
SmlPltProcessAttrib36h:
	LD A,D	;restore attrib value
	AND $C0	;mask top 2 bits (bright & flash)
	LD D,A	;put back into d
	LD A,(PlatformLUTColour)	;[PlatformLUTColour]
	OR D	;or in bright & flash bits
SmlPltWritePlatformAttr:
	LD (BC),A	;write screen attribute
	BIT 6,A	;check bit 6 (bright)
	JR Z,SmlPltCharExitFunc	;[SmlPltCharExitFunc]	;exit function if not set
	LD D,A	;store written attrib in backup attrib in D
	AND $38	;mask paper colour
	CP $20	;check for blue
	JR Z,SmlPltCharExitFunc	;[SmlPltCharExitFunc]	;exit if blue
	LD A,D	;restore attrib from D
	PUSH HL
	LD HL,(AttributeDrawListPtr)	;[AttributeDrawListPtr]
	LD (HL),C	;write address
	INC HL
	LD (HL),B
	INC HL
	LD (HL),A	;write attrib
	INC HL
	LD (AttributeDrawListPtr),HL	;[AttributeDrawListPtr]
	POP HL
SmlPltCharExitFunc:
	EX AF,AF'
	POP DE
	POP BC
	RET
	db $00
	db $00
	db $00
	db $00
	db $00
SmallPlatformsDataLUT:
	dw Small_Platform1_Data
	dw Small_Platform2_Data
	dw SmallPlatform3Data
	dw $EE1A
	dw $EE8C
	dw $EEFE
	dw $EF70
	dw $EFE0
	dw $F042
	dw $F0B4
	dw $F126
	dw $F197
	dw $F200
	dw $F268
	dw $F2C8
	dw $F329
	dw $F393
	dw $F405
	dw $F477
	dw $F4E9
	dw $F54F
	dw $F561
	dw $F5D3
	dw $F644
	dw $F6AC
	dw $F708
	dw $F744
	dw $F780
	dw $F7BF
	dw $F816
	dw $F86A
	dw $F8A9
	dw $F901
	dw $F958
	dw $F967
	dw $F977
	dw $FD57
	dw $FDB7
	dw $FE17
	dw $FE71
	dw $FE9D
	dw $FED3
	dw $FED9
	dw $FEE5
	dw $FF21
	dw $FF5A
	dw $FF7C
	dw $FFA9
	dw $FFE7
	dw $BFC3
	db $2B
	db $C0
	db $8C
	db $C0
	db $F4
	db $C0
	db $50
	db $C1
	db $8C
	db $C1
	db $E7
	db $FF
	db $E7
	db $FF
	db $CC
	db $C1
	db $2D
	db $C2
	db $93
	db $C2
	db $E5
	db $C2
	db $3C
	db $8D
	db $AE
	db $8D
	db $20
	db $8E
	db $92
	db $8E
	db $03
	db $8F
	db $6D
	db $8F
	db $DC
	db $8F
	db $33
	db $90
	db $1E
	db $6C
	db $90
	db $6C
	db $00
	db $6D
	db $60
	db $6D
	db $C2
	db $6D
	db $34
	db $6E
	db $A4
	db $6E
	db $04
	db $6F
	db $66
	db $6F
	db $D8
	db $6F
	db $4A
	db $70
	db $BC
	db $70
	db $2E
	db $71
	db $98
	db $71
	db $C6
	db $71
	db $14
	db $72
	db $5E
	db $72
	db $AB
	db $72
	db $29
	db $F3
	db $20
	db $8E
	db $D3
	db $FE
	db $D3
	db $FE
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
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
	db $00
	db $F9
	db $73
	db $8B
	db $74
	db $AC
	db $6A
	db $FA
	db $6A
	db $44
	db $6B
	db $6E
	db $6B
	db $98
	db $6B
	db $C2
	db $6B
	db $EC
	db $6B
	db $97
	db $F9
	db $F7
	db $F9
	db $53
	db $FA
	db $A3
	db $FA
	db $91
	db $FB
	db $67
	db $FC
	db $F7
	db $FC
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
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $04	;12 bytes of attrib data
	db $64
	db $64
	db $04
	db $04
	db $64
	db $64
	db $04
	db $04
	db $60
	db $60
	db $04
Small_Platform1_Data:
	db $F0	;3 rows of 4
	db $F0
	db $F0
	db $00	;3 rows of empty
	db $00
	db $00
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ECD2:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ECDA:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ECE2:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ECEA:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ECF2:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ECFA:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED02:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED0A:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED12:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED1A:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED22:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $44	;12 bytes of attrib data
	db $64
	db $64
	db $44
	db $44
	db $64
	db $64
	db $44
	db $04
	db $60
	db $60
	db $04
Small_Platform2_Data:
	db $F0	;character masking
	db $F0
	db $F0
	db $00
	db $00
	db $00
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED44:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED4C:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED54:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED5C:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED64:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED6C:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED74:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED7C:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED84:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED8C:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
data_ED94:
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $04
	db $64
	db $64
	db $44
	db $04
	db $64
	db $64
	db $44
	db $04
	db $60
	db $60
	db $04
SmallPlatform3Data:
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $01
	db $03
	db $06
	db $0C
	db $1E
	db $06
	db $06
	db $07
	db $80
	db $C0
	db $60
	db $30
	db $78
	db $60
	db $60
	db $E0
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $A3
	db $D1
	db $D1
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $D1
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $44
	db $64
	db $64
	db $04
	db $44
	db $64
	db $64
	db $04
	db $04
	db $60
	db $60
	db $04
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $01
	db $03
	db $06
	db $0C
	db $1E
	db $06
	db $06
	db $07
	db $80
	db $C0
	db $60
	db $30
	db $78
	db $60
	db $60
	db $E0
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $C7
	db $8B
	db $8B
	db $05
	db $05
	db $05
	db $05
	db $05
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $8B
	db $44
	db $64
	db $64
	db $44
	db $44
	db $64
	db $64
	db $44
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $4B
	db $97
	db $97
	db $97
	db $97
	db $97
	db $4B
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $F6
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $F6
	db $00
	db $A3
	db $D1
	db $D1
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $C7
	db $8B
	db $8B
	db $05
	db $05
	db $05
	db $05
	db $05
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $D1
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $8B
	db $04
	db $64
	db $64
	db $44
	db $04
	db $64
	db $64
	db $44
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $4B
	db $97
	db $97
	db $97
	db $97
	db $97
	db $4B
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $F6
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $F6
	db $00
	db $A3
	db $D1
	db $D1
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $A0
	db $D1
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $44
	db $64
	db $64
	db $04
	db $44
	db $64
	db $64
	db $04
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $4B
	db $97
	db $97
	db $97
	db $97
	db $97
	db $4B
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $F6
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $F6
	db $00
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $C7
	db $8B
	db $8B
	db $05
	db $05
	db $05
	db $05
	db $05
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $97
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $8B
	db $45
	db $03
	db $03
	db $45
	db $45
	db $45
	db $45
	db $45
	db $40
	db $40
	db $90
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $78
	db $FC
	db $BC
	db $9C
	db $FC
	db $60
	db $00
	db $00
	db $1E
	db $3F
	db $2F
	db $27
	db $3F
	db $02
	db $18
	db $18
	db $1C
	db $0C
	db $0E
	db $07
	db $05
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $80
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $0E
	db $18
	db $18
	db $38
	db $30
	db $70
	db $A0
	db $60
	db $C0
	db $02
	db $01
	db $01
	db $01
	db $00
	db $00
	db $00
	db $00
	db $0F
	db $39
	db $67
	db $47
	db $CF
	db $87
	db $7F
	db $00
	db $F0
	db $FC
	db $FE
	db $FE
	db $FF
	db $FF
	db $FE
	db $00
	db $C0
	db $80
	db $80
	db $80
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
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $01
	db $02
	db $05
	db $0A
	db $15
	db $2A
	db $75
	db $3F
	db $DF
	db $E7
	db $F9
	db $FC
	db $FB
	db $F7
	db $EF
	db $FE
	db $FD
	db $EB
	db $95
	db $6B
	db $B5
	db $5B
	db $AD
	db $00
	db $80
	db $C0
	db $E0
	db $F0
	db $F8
	db $FC
	db $FE
	db $DA
	db $8D
	db $87
	db $83
	db $86
	db $8C
	db $98
	db $B0
	db $DF
	db $BF
	db $7F
	db $FF
	db $FF
	db $BF
	db $DF
	db $AF
	db $57
	db $AB
	db $55
	db $AB
	db $57
	db $AD
	db $59
	db $B1
	db $FD
	db $F3
	db $CF
	db $3F
	db $7F
	db $BF
	db $5F
	db $AF
	db $E0
	db $C0
	db $C0
	db $A0
	db $90
	db $40
	db $7F
	db $00
	db $D7
	db $AB
	db $D5
	db $AA
	db $D4
	db $00
	db $FF
	db $00
	db $61
	db $C1
	db $81
	db $41
	db $21
	db $00
	db $FF
	db $00
	db $57
	db $AB
	db $57
	db $AD
	db $51
	db $22
	db $FE
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $31
	db $79
	db $6B
	db $EF
	db $67
	db $77
	db $27
	db $6F
	db $C6
	db $CF
	db $86
	db $CE
	db $1E
	db $39
	db $39
	db $31
	db $FC
	db $FB
	db $C3
	db $C3
	db $E3
	db $87
	db $C3
	db $87
	db $EE
	db $C6
	db $C6
	db $C6
	db $8C
	db $0C
	db $9C
	db $C8
	db $6F
	db $6A
	db $6A
	db $3A
	db $3B
	db $33
	db $73
	db $73
	db $B1
	db $33
	db $33
	db $31
	db $30
	db $79
	db $38
	db $39
	db $8F
	db $87
	db $83
	db $C3
	db $E7
	db $E7
	db $EF
	db $2F
	db $3C
	db $38
	db $1C
	db $10
	db $18
	db $18
	db $BC
	db $0C
	db $FB
	db $C3
	db $F3
	db $E3
	db $73
	db $E3
	db $79
	db $11
	db $49
	db $88
	db $98
	db $3C
	db $38
	db $B0
	db $F0
	db $E3
	db $EF
	db $E7
	db $73
	db $73
	db $78
	db $71
	db $F0
	db $FC
	db $9E
	db $9A
	db $1D
	db $CD
	db $C7
	db $EF
	db $C6
	db $E4
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $31
	db $3B
	db $73
	db $71
	db $33
	db $77
	db $7F
	db $E3
	db $C6
	db $C2
	db $C2
	db $E6
	db $F7
	db $C7
	db $8B
	db $C7
	db $FC
	db $FC
	db $3E
	db $7C
	db $3E
	db $BC
	db $30
	db $38
	db $EE
	db $64
	db $24
	db $6C
	db $4C
	db $CC
	db $98
	db $DC
	db $C1
	db $67
	db $63
	db $71
	db $3B
	db $F0
	db $F9
	db $F0
	db $86
	db $07
	db $86
	db $E4
	db $EC
	db $7E
	db $D8
	db $CE
	db $19
	db $0C
	db $18
	db $38
	db $71
	db $7C
	db $38
	db $1E
	db $DC
	db $CE
	db $63
	db $67
	db $F7
	db $3E
	db $78
	db $30
	db $F8
	db $7C
	db $38
	db $78
	db $3C
	db $78
	db $70
	db $31
	db $47
	db $77
	db $31
	db $38
	db $78
	db $38
	db $71
	db $E3
	db $07
	db $C1
	db $F3
	db $70
	db $F8
	db $30
	db $FC
	db $FC
	db $1C
	db $9E
	db $CF
	db $CE
	db $EF
	db $77
	db $67
	db $E7
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $E0
	db $F0
	db $00
	db $00
	db $00
	db $73
	db $D1
	db $7C
	db $FF
	db $72
	db $73
	db $7A
	db $5B
	db $C6
	db $62
	db $72
	db $76
	db $66
	db $6E
	db $76
	db $F6
	db $FC
	db $DD
	db $EF
	db $6E
	db $6E
	db $6E
	db $CF
	db $CF
	db $EE
	db $CE
	db $EC
	db $C8
	db $D8
	db $D8
	db $70
	db $60
	db $59
	db $5C
	db $C4
	db $E6
	db $E6
	db $C6
	db $78
	db $74
	db $83
	db $03
	db $02
	db $00
	db $20
	db $38
	db $38
	db $68
	db $0F
	db $02
	db $02
	db $00
	db $00
	db $00
	db $00
	db $00
	db $36
	db $76
	db $66
	db $E6
	db $F7
	db $E7
	db $73
	db $71
	db $68
	db $68
	db $6E
	db $8E
	db $87
	db $B7
	db $E3
	db $E3
	db $00
	db $00
	db $70
	db $50
	db $68
	db $A8
	db $FE
	db $FC
	db $00
	db $10
	db $38
	db $38
	db $68
	db $6E
	db $66
	db $E7
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $70
	db $F0
	db $00
	db $00
	db $00
	db $31
	db $18
	db $19
	db $1B
	db $1B
	db $0E
	db $0E
	db $04
	db $C6
	db $C2
	db $E6
	db $82
	db $CF
	db $6F
	db $6C
	db $68
	db $FE
	db $FA
	db $E2
	db $F6
	db $C4
	db $E4
	db $EC
	db $EE
	db $EE
	db $66
	db $E7
	db $C7
	db $CE
	db $CE
	db $CE
	db $CE
	db $38
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $08
	db $EA
	db $5A
	db $5A
	db $73
	db $76
	db $26
	db $07
	db $0F
	db $DE
	db $BC
	db $BC
	db $98
	db $10
	db $30
	db $18
	db $18
	db $00
	db $0C
	db $0E
	db $1F
	db $39
	db $33
	db $39
	db $31
	db $1E
	db $3E
	db $77
	db $73
	db $61
	db $FB
	db $61
	db $E3
	db $0F
	db $08
	db $1D
	db $39
	db $B9
	db $FC
	db $E0
	db $FC
	db $1C
	db $8E
	db $8F
	db $8F
	db $8F
	db $47
	db $43
	db $E7
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $A0
	db $00
	db $00
	db $00
	db $33
	db $23
	db $61
	db $F7
	db $E1
	db $E1
	db $71
	db $63
	db $EC
	db $87
	db $CF
	db $C6
	db $C6
	db $CE
	db $C7
	db $A7
	db $39
	db $71
	db $61
	db $78
	db $E3
	db $E1
	db $F7
	db $61
	db $9C
	db $DE
	db $E6
	db $EE
	db $E3
	db $E7
	db $C3
	db $E7
	db $67
	db $27
	db $26
	db $36
	db $36
	db $36
	db $36
	db $16
	db $A7
	db $27
	db $3B
	db $0E
	db $00
	db $00
	db $00
	db $00
	db $60
	db $73
	db $61
	db $63
	db $2F
	db $27
	db $27
	db $37
	db $E6
	db $A6
	db $A4
	db $AC
	db $B8
	db $18
	db $18
	db $10
	db $16
	db $14
	db $1C
	db $1C
	db $1C
	db $1C
	db $18
	db $08
	db $27
	db $27
	db $33
	db $3F
	db $0E
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $60
	db $00
	db $00
	db $00
	db $33
	db $7B
	db $E1
	db $C3
	db $F1
	db $C1
	db $73
	db $79
	db $6C
	db $CE
	db $C9
	db $98
	db $BD
	db $F8
	db $70
	db $FC
	db $3B
	db $79
	db $FB
	db $7B
	db $FF
	db $9F
	db $DF
	db $77
	db $BC
	db $F6
	db $B3
	db $9B
	db $1B
	db $93
	db $99
	db $3B
	db $21
	db $73
	db $72
	db $66
	db $CC
	db $6C
	db $7C
	db $18
	db $70
	db $79
	db $70
	db $70
	db $30
	db $18
	db $19
	db $18
	db $77
	db $FB
	db $72
	db $71
	db $F1
	db $31
	db $F0
	db $F0
	db $3B
	db $3B
	db $3A
	db $2E
	db $2C
	db $E4
	db $C0
	db $C0
	db $1B
	db $1B
	db $08
	db $0D
	db $0D
	db $0D
	db $0D
	db $07
	db $F0
	db $E0
	db $C0
	db $80
	db $80
	db $80
	db $80
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $E0
	db $00
	db $00
	db $00
	db $21
	db $7B
	db $7B
	db $E1
	db $C1
	db $C7
	db $E3
	db $C7
	db $EC
	db $EF
	db $C8
	db $A8
	db $CC
	db $8C
	db $1C
	db $18
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $1C
	db $B6
	db $B2
	db $F3
	db $F7
	db $F7
	db $E6
	db $B6
	db $C7
	db $C3
	db $E3
	db $C1
	db $81
	db $C3
	db $F1
	db $F1
	db $38
	db $30
	db $38
	db $7C
	db $F8
	db $F0
	db $F8
	db $71
	db $F1
	db $73
	db $69
	db $ED
	db $6F
	db $67
	db $E0
	db $C0
	db $B6
	db $BC
	db $98
	db $88
	db $00
	db $00
	db $00
	db $00
	db $43
	db $67
	db $42
	db $4E
	db $48
	db $78
	db $30
	db $30
	db $31
	db $3C
	db $19
	db $1B
	db $1F
	db $0E
	db $00
	1 Bytes
	db $C0
	1 Bytes
	db $C0
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	1 Bytes
	db $61
	db $67
	db $14
	db $7A
	db $FF
	db $07
	db $C6
	db $03
	db $CF
	db $67
	db $0C
	db $3F
	db $3B
	db $F3
	db $7B
	db $79
	db $33
	db $32
	db $E3
	db $E7
	db $E6
	db $72
	db $6A
	db $7E
	db $78
	db $76
	db $67
	db $66
	db $66
	db $67
	db $67
	db $67
	db $73
	db $B3
	db $EF
	db $CF
	db $E7
	db $73
	db $67
	db $3F
	db $1C
	db $62
	db $A6
	db $B6
	db $F3
	db $F3
	db $77
	db $37
	db $3E
	db $5E
	db $2C
	db $0C
	db $0C
	db $0F
	db $07
	db $02
	db $00
	db $00
	db $B3
	db $B3
	db $B3
	db $3E
	db $1E
	db $0E
	db $0C
	db $0C
	db $7E
	db $70
	db $3C
	db $0F
	db $00
	db $00
	db $00
	db $00
	db $58
	db $E0
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $38
	db $7F
	db $6E
	db $7D
	db $F1
	db $FE
	db $74
	db $78
	db $07
	db $9E
	db $3E
	db $B2
	db $DA
	db $D6
	db $BB
	db $BE
	db $87
	db $E7
	db $EF
	db $7C
	db $0C
	db $0C
	db $8C
	db $1C
	db $80
	db $E6
	db $3D
	db $E7
	db $E7
	db $E6
	db $CE
	db $CE
	db $74
	db $E0
	db $FC
	db $EE
	db $E6
	db $6E
	db $66
	db $3D
	db $DE
	db $E6
	db $C7
	db $E3
	db $71
	db $60
	db $33
	db $36
	db $38
	db $38
	db $3C
	db $DC
	db $D8
	db $DE
	db $1E
	db $AF
	db $8E
	db $86
	db $4E
	db $56
	db $5E
	db $6C
	db $1C
	db $20
	db $01
	db $07
	db $03
	db $03
	db $01
	db $00
	db $00
	db $00
	db $9E
	db $21
	db $2B
	db $FB
	db $E3
	db $03
	db $01
	db $01
	db $AE
	db $A6
	db $26
	db $27
	db $27
	db $A3
	db $E3
	db $C0
	db $30
	db $70
	db $30
	db $60
	db $20
	db $60
	db $E0
	db $C0
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $1C
	db $7F
	db $FF
	db $DF
	db $DF
	db $5E
	db $4C
	db $7B
	db $0E
	db $1F
	db $1F
	db $07
	db $07
	db $03
	db $99
	db $9B
	db $6C
	db $FE
	db $FF
	db $7F
	db $1F
	db $07
	db $8F
	db $CF
	db $0E
	db $1F
	db $33
	db $F6
	db $E4
	db $8C
	db $CC
	db $98
	db $3F
	db $37
	db $16
	db $35
	db $77
	db $76
	db $66
	db $6E
	db $1F
	db $17
	db $D7
	db $D7
	db $F7
	db $F7
	db $EF
	db $CF
	db $C3
	db $C1
	db $F1
	db $F1
	db $81
	db $E3
	db $E3
	db $E1
	db $1C
	db $8C
	db $8E
	db $C6
	db $CE
	db $C6
	db $E7
	db $E7
	db $63
	db $E7
	db $C7
	db $E3
	db $F7
	db $71
	db $70
	db $31
	db $DC
	db $8F
	db $CF
	db $8E
	db $C7
	db $E7
	db $E7
	db $E3
	db $C3
	db $43
	db $80
	db $03
	db $41
	db $60
	db $F0
	db $FC
	db $E3
	db $C3
	db $E3
	db $E3
	db $E6
	db $E6
	db $C6
	db $E6
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $38
	db $7F
	db $E3
	db $E7
	db $B7
	db $B5
	db $B8
	db $1E
	db $77
	db $FB
	db $99
	db $39
	db $B9
	db $D9
	db $91
	db $00
	db $06
	db $0F
	db $8E
	db $8F
	db $87
	db $C6
	db $E6
	db $38
	db $68
	db $FC
	db $5E
	db $BE
	db $B6
	db $B6
	db $92
	db $BD
	db $DD
	db $DF
	db $C9
	db $8C
	db $8C
	db $CE
	db $C6
	db $F8
	db $33
	db $39
	db $0D
	db $C4
	db $6F
	db $67
	db $4F
	db $E7
	db $C7
	db $E7
	db $E7
	db $C7
	db $8A
	db $9E
	db $9E
	db $A3
	db $33
	db $33
	db $13
	db $1A
	db $32
	db $73
	db $63
	db $CE
	db $48
	db $58
	db $5B
	db $50
	db $71
	db $33
	db $31
	db $CF
	db $E3
	db $F1
	db $EB
	db $79
	db $E3
	db $C1
	db $E3
	db $9E
	db $BF
	db $BC
	db $BE
	db $D8
	db $FC
	db $F8
	db $FC
	db $27
	db $37
	db $17
	db $36
	db $76
	db $E6
	db $CE
	db $E7
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $3C
	db $7E
	db $7D
	db $7D
	db $7E
	db $3C
	db $00
	db $03
	db $18
	db $24
	db $BE
	db $BE
	db $7E
	db $1C
	db $D0
	db $CC
	db $0C
	db $0C
	db $01
	db $1F
	db $1F
	db $1F
	db $1F
	db $0F
	db $00
	db $00
	db $00
	db $80
	db $6E
	db $6F
	db $8F
	db $3F
	db $3B
	db $78
	db $7E
	db $7E
	db $7E
	db $24
	db $18
	db $18
	db $0E
	db $06
	db $01
	db $07
	db $07
	db $07
	db $07
	db $03
	db $00
	db $00
	db $C1
	db $E3
	db $DD
	db $DD
	db $E3
	db $01
	db $0F
	db $36
	db $B0
	db $80
	db $F0
	db $F0
	db $F0
	db $E0
	db $18
	db $24
	db $7D
	db $BD
	db $BC
	db $7E
	db $3C
	db $00
	db $3C
	db $7E
	db $BE
	db $3E
	db $7E
	db $24
	db $18
	db $18
	db $60
	db $60
	db $1E
	db $3F
	db $3E
	db $3E
	db $3F
	db $12
	db $00
	db $00
	db $1E
	db $3E
	db $DE
	db $DF
	db $3F
	db $1E
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $3C
	db $7E
	db $7E
	db $7E
	db $7E
	db $30
	db $18
	db $18
	db $24
	db $7E
	db $7E
	db $7D
	db $7D
	db $1C
	db $0C
	db $0C
	db $0C
	db $12
	db $3F
	db $DE
	db $DE
	db $3F
	db $00
	db $1E
	db $3F
	db $3F
	db $1F
	db $DF
	db $DC
	db $00
	db $0C
	db $0C
	db $01
	db $07
	db $07
	db $07
	db $04
	db $03
	db $60
	db $60
	db $83
	db $E7
	db $E7
	db $E7
	db $61
	db $46
	db $0E
	db $30
	db $B0
	db $80
	db $E0
	db $E0
	db $E0
	db $C0
	db $00
	db $78
	db $FC
	db $FC
	db $FC
	db $FC
	db $48
	db $30
	db $03
	db $36
	db $78
	db $BE
	db $BE
	db $7E
	db $3C
	db $00
	db $06
	db $38
	db $7E
	db $7D
	db $7D
	db $7E
	db $24
	db $18
	db $00
	db $00
	db $1E
	db $DF
	db $DF
	db $3F
	db $3F
	db $12
	db $30
	db $0C
	db $7E
	db $7D
	db $7D
	db $7E
	db $3C
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $E0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $01
	db $3B
	db $7A
	db $18
	db $60
	db $FC
	db $FB
	db $7B
	db $7C
	db $78
	db $00
	db $0C
	db $12
	db $3F
	db $DF
	db $DE
	db $3E
	db $06
	db $30
	db $00
	db $00
	db $1E
	db $3F
	db $DF
	db $DF
	db $3F
	db $06
	db $78
	db $7E
	db $78
	db $0B
	db $33
	db $30
	db $49
	db $FD
	db $00
	db $00
	db $01
	db $71
	db $79
	db $79
	db $F8
	db $F8
	db $30
	db $C0
	db $F8
	db $F8
	db $F8
	db $C0
	db $D8
	db $1A
	db $18
	db $18
	db $24
	db $7E
	db $7E
	db $7E
	db $7E
	db $3C
	db $FC
	db $FC
	db $FC
	db $79
	db $01
	db $01
	db $01
	db $00
	db $90
	db $60
	db $90
	db $F8
	db $F8
	db $F8
	db $F8
	db $F0
	db $07
	db $3F
	db $3F
	db $3F
	db $1E
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $F0
	db $60
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
	db $18
	db $18
	db $60
	db $FC
	db $FC
	db $FC
	db $30
	db $B6
	db $0C
	db $12
	db $3F
	db $3E
	db $3E
	db $3F
	db $06
	db $18
	db $00
	db $1E
	db $3F
	db $DF
	db $DF
	db $3F
	db $12
	db $0C
	db $1D
	db $3C
	db $3F
	db $3F
	db $3F
	db $1E
	db $00
	db $00
	db $87
	db $03
	db $00
	db $01
	db $01
	db $00
	db $03
	db $07
	db $18
	db $60
	db $78
	db $F8
	db $F8
	db $78
	db $40
	db $30
	db $0C
	db $30
	db $7E
	db $7E
	db $7E
	db $7E
	db $3C
	db $00
	db $76
	db $F0
	db $FC
	db $FC
	db $FC
	db $78
	db $00
	db $00
	db $30
	db $30
	db $48
	db $FC
	db $FC
	db $FC
	db $FC
	db $78
	db $02
	db $00
	db $00
	db $00
	db $02
	db $02
	db $C0
	db $C0
	db $C0
	db $00
	db $00
	db $00
	db $00
	db $03
	db $0F
	db $19
	db $39
	db $4F
	db $49
	db $3F
	db $00
	db $E0
	db $78
	db $BC
	db $EE
	db $EF
	db $7D
	db $DE
	db $00
	db $05
	db $05
	db $18
	db $0F
	db $00
	db $0D
	db $1B
	db $00
	db $E0
	db $E0
	db $18
	db $F0
	db $00
	db $C0
	db $80
	db $1B
	db $F6
	db $6E
	db $DC
	db $7A
	db $F7
	db $EF
	db $5C
	db $38
	db $5C
	db $F4
	db $D8
	db $78
	db $50
	db $60
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $90
	db $90
	db $90
	db $00
	db $00
	db $00
	db $78
	db $CC
	db $DA
	db $FB
	db $F3
	db $F1
	db $F9
	db $7C
	db $1E
	db $33
	db $5B
	db $DF
	db $CF
	db $8F
	db $9F
	db $3E
	db $1E
	db $4F
	db $5F
	db $5F
	db $1E
	db $01
	db $5F
	db $5F
	db $7C
	db $B2
	db $BE
	db $BE
	db $3C
	db $82
	db $BE
	db $BE
	db $1E
	db $41
	db $DF
	db $3E
	db $F8
	db $E0
	db $C0
	db $80
	db $3C
	db $82
	db $DF
	db $6F
	db $1F
	db $07
	db $03
	db $01
	db $02
	db $02
	db $40
	db $40
	db $40
	db $40
	db $30
	db $30
	db $30
	db $00
	db $00
	db $00
	db $01
	db $41
	db $61
	db $31
	db $39
	db $1D
	db $0E
	db $FB
	db $00
	db $02
	db $06
	db $8C
	db $9C
	db $B8
	db $70
	db $DE
	db $3F
	db $05
	db $0E
	db $1A
	db $1A
	db $33
	db $23
	db $03
	db $B8
	db $E0
	db $B0
	db $B8
	db $98
	db $4C
	db $44
	db $C0
	db $04
	db $07
	db $06
	db $03
	db $01
	db $00
	db $00
	db $00
	db $20
	db $F8
	db $FF
	db $7F
	db $9F
	db $63
	db $1F
	db $00
	db $04
	db $36
	db $36
	db $36
	db $36
	db $04
	db $04
	db $36
	db $04
	db $E0
	db $E0
	db $E0
	db $00
	db $00
	db $00
	db $02
	db $33
	db $1B
	db $0D
	db $01
	db $3E
	db $00
	db $00
	db $06
	db $0C
	db $0E
	db $0F
	db $1C
	db $FE
	db $07
	db $03
	db $1C
	db $38
	db $60
	db $DF
	db $84
	db $78
	db $1E
	db $00
	db $00
	db $70
	db $1C
	db $07
	db $38
	db $F7
	db $0E
	db $38
	db $03
	db $03
	db $03
	db $0F
	db $FF
	db $19
	db $30
	db $60
	db $00
	db $1C
	db $3E
	db $26
	db $96
	db $C6
	db $FE
	db $7C
	db $00
	db $03
	db $07
	db $0E
	db $0C
	db $0D
	db $07
	db $00
	db $60
	db $E0
	db $F0
	db $3F
	db $1C
	db $0E
	db $06
	db $06
	db $00
	db $3E
	db $E0
	db $1C
	db $EE
	db $70
	db $38
	db $00
	db $04
	db $36
	db $36
	db $04
	db $04
	db $36
	db $04
	db $36
	db $36
	db $60
	db $70
	db $F0
	db $00
	db $00
	db $00
	db $06
	db $06
	db $1F
	db $3F
	db $61
	db $60
	db $30
	db $00
	db $00
	db $0E
	db $1F
	db $9B
	db $C3
	db $FF
	db $FE
	db $C0
	db $00
	db $1E
	db $07
	db $FA
	db $3C
	db $03
	db $0E
	db $3C
	db $E6
	db $67
	db $62
	db $F0
	db $B0
	db $30
	db $30
	db $30
	db $0F
	db $7C
	db $00
	db $B8
	db $CE
	db $60
	db $20
	db $00
	db $70
	db $18
	db $E6
	db $78
	db $06
	db $1C
	db $70
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $7D
	db $0F
	db $07
	db $06
	db $70
	db $E0
	db $F8
	db $DC
	db $C7
	db $80
	db $00
	db $00
	db $00
	db $30
	db $60
	db $C0
	db $38
	db $C0
	db $70
	db $00
	db $00
	db $00
	db $00
	db $00
	db $04
	db $04
	db $60
	db $60
	db $60
	db $00
	db $00
	db $00
	db $09
	db $1B
	db $3D
	db $2D
	db $4D
	db $2C
	db $6A
	db $52
	db $00
	db $40
	db $B0
	db $B8
	db $DC
	db $CF
	db $D0
	db $88
	db $70
	db $FB
	db $FB
	db $F8
	db $77
	db $8F
	db $6F
	db $67
	db $2C
	db $8C
	db $B0
	db $7A
	db $7A
	db $B0
	db $AC
	db $6C
	db $18
	db $3A
	db $38
	db $06
	db $0F
	db $06
	db $00
	db $00
	db $00
	db $E0
	db $E0
	db $40
	db $00
	db $00
	db $00
	db $00
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $70
	db $70
	db $70
	db $00
	db $00
	db $00
	db $0E
	db $1C
	db $38
	db $31
	db $73
	db $66
	db $69
	db $28
	db $03
	db $1F
	db $7A
	db $D6
	db $B2
	db $90
	db $80
	db $80
	db $80
	db $40
	db $C0
	db $40
	db $00
	db $00
	db $10
	db $10
	db $2A
	db $0A
	db $1E
	db $33
	db $2C
	db $27
	db $39
	db $25
	db $00
	db $44
	db $64
	db $EE
	db $FF
	db $00
	db $FF
	db $DF
	db $58
	db $58
	db $F8
	db $EC
	db $9C
	db $7C
	db $EC
	db $FC
	db $37
	db $19
	db $1B
	db $0E
	db $07
	db $01
	db $00
	db $00
	db $7E
	db $FF
	db $AF
	db $7F
	db $4D
	db $D7
	db $7E
	db $00
	db $FC
	db $F8
	db $F8
	db $B0
	db $E0
	db $80
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $05
	db $05
	db $00
	db $00
	db $00
	db $90
	db $F0
	db $F0
	db $00
	db $00
	db $00
	db $FE
	db $DA
	db $FE
	db $EE
	db $C6
	db $82
	db $EE
	db $EE
	db $7F
	db $5D
	db $7F
	db $77
	db $63
	db $41
	db $77
	db $77
	db $7E
	db $6E
	db $6E
	db $6E
	db $6F
	db $6B
	db $6F
	db $6F
	db $78
	db $5F
	db $5C
	db $5C
	db $6F
	db $2D
	db $37
	db $0F
	db $1E
	db $FE
	db $BE
	db $FE
	db $3E
	db $3C
	db $FC
	db $F0
	db $7E
	db $76
	db $76
	db $76
	db $F6
	db $D6
	db $F6
	db $F6
	db $6B
	db $7F
	db $7E
	db $3E
	db $3F
	db $1F
	db $0F
	db $03
	db $10
	db $77
	db $F8
	db $E0
	db $0F
	db $F8
	db $BF
	db $FF
	db $08
	db $EE
	db $1F
	db $07
	db $F0
	db $3F
	db $FD
	db $FF
	db $D6
	db $F6
	db $7E
	db $7C
	db $FC
	db $F8
	db $F0
	db $C0
	db $00
	db $00
	db $00
	db $10
	db $00
	db $00
	db $00
	db $7E
	db $E7
	db $F7
	db $F7
	db $E3
	db $E3
	db $FF
	db $80
	db $00
	db $00
	db $00
	db $80
	db $00
	db $00
	db $00
	db $7E
	db $C3
	db $FB
	db $C3
	db $CF
	db $C3
	db $FF
	db $01
	db $00
	db $00
	db $00
	db $00
	db $90
	db $00
	db $00
	db $00
	db $7E
	db $C7
	db $F7
	db $C3
	db $F3
	db $C3
	db $FF
	db $01
	db $7E
	db $C7
	db $F7
	db $C3
	db $F3
	db $C3
	db $FF
	db $80
	db $42
	db $46
	db $46
	db $46
	db $42
	db $42
	db $46
	db $42
	db $46
	db $46
	db $E0
	db $80
	db $80
	db $80
	db $80
	db $E0
	db $00
	db $17
	db $3B
	db $6B
	db $3D
	db $4D
	db $72
	db $7C
	db $00
	db $FF
	db $62
	db $FA
	db $FA
	db $B2
	db $FF
	db $00
	db $00
	db $00
	db $FF
	db $C7
	db $DF
	db $FF
	db $00
	db $00
	db $5E
	db $7E
	db $7A
	db $5E
	db $5A
	db $42
	db $7E
	db $42
	db $3C
	db $24
	db $34
	db $34
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $2C
	db $2C
	db $24
	db $3C
	db $42
	db $7E
	db $42
	db $5A
	db $5E
	db $7E
	db $7A
	db $5E
	db $7C
	db $72
	db $4D
	db $3D
	db $6B
	db $3B
	db $17
	db $00
	db $00
	db $FF
	db $B2
	db $FA
	db $FA
	db $62
	db $FF
	db $00
	db $00
	db $00
	db $FF
	db $C7
	db $DF
	db $FF
	db $00
	db $00
	db $46
	db $46
	db $42
	db $46
	db $42
	db $42
	db $46
	db $46
	db $46
	db $42
	db $E0
	db $20
	db $20
	db $20
	db $20
	db $E0
	db $00
	db $00
	db $FF
	db $FB
	db $E3
	db $FF
	db $00
	db $00
	db $00
	db $FF
	db $46
	db $5F
	db $5F
	db $4B
	db $FF
	db $00
	db $00
	db $E8
	db $DC
	db $D6
	db $BC
	db $B2
	db $4E
	db $3E
	db $7A
	db $5E
	db $7E
	db $7A
	db $5A
	db $42
	db $7E
	db $42
	db $3C
	db $24
	db $34
	db $34
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $3C
	db $2C
	db $2C
	db $24
	db $3C
	db $42
	db $7E
	db $42
	db $5A
	db $7A
	db $7E
	db $5E
	db $7A
	db $00
	db $00
	db $FF
	db $FB
	db $E3
	db $FF
	db $00
	db $00
	db $00
	db $FF
	db $4D
	db $5F
	db $5F
	db $46
	db $FF
	db $00
	db $3E
	db $4E
	db $B2
	db $BC
	db $D6
	db $DC
	db $E8
	db $00
	db $42
	db $42
	db $42
	db $42
	db $42
	db $42
	db $FC
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $FE
	db $F6
	db $C6
	db $FE
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $C7
	db $DF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $00
	db $05
	db $05
	db $0D
	db $0D
	db $0D
	db $0D
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $00
	db $FF
	db $81
	db $FF
	db $FF
	db $00
	db $00
	db $7F
	db $7F
	db $7F
	db $70
	db $77
	db $77
	db $76
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $0F
	db $CF
	db $CF
	db $4F
	db $F6
	db $F6
	db $F6
	db $F6
	db $F6
	db $F6
	db $F6
	db $F6
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $F6
	db $F7
	db $F0
	db $F0
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $EF
	db $EF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $F7
	db $F7
	db $4F
	db $CF
	db $0F
	db $0F
	db $FF
	db $FF
	db $FF
	db $FF
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $7F
	db $00
	db $EF
	db $EF
	db $E1
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $C3
	db $C3
	db $C3
	db $C3
	db $C3
	db $FF
	db $FF
	db $FF
	db $FF
	db $0F
	db $0C
	db $0C
	db $0C
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $3F
	db $30
	db $30
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $C3
	db $FF
	db $FF
	db $FF
	db $F7
	db $F7
	db $07
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $47
	db $47
	db $05
	db $05
	db $07
	db $07
	db $05
	db $47
	db $47
	db $05
	db $43
	db $43
	db $05
	db $47
	db $47
	db $05
	db $00
	db $FC
	db $FC
	db $9C
	db $FF
	db $00
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $00
	db $00
	db $07
	db $1B
	db $0D
	db $3C
	db $58
	db $60
	db $D0
	db $D8
	db $40
	db $68
	db $AC
	db $DC
	db $7E
	db $FE
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $00
	db $7F
	db $77
	db $63
	db $77
	db $7F
	db $7F
	db $00
	db $00
	db $FC
	db $04
	db $76
	db $06
	db $04
	db $FC
	db $00
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $71
	db $7B
	db $3F
	db $3F
	db $DF
	db $E7
	db $78
	db $3C
	db $FE
	db $FE
	db $9C
	db $60
	db $BE
	db $0E
	db $FC
	db $F8
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $00
	db $78
	db $7B
	db $03
	db $0C
	db $05
	db $1A
	db $00
	db $00
	db $1E
	db $DE
	db $C0
	db $30
	db $A0
	db $58
	db $00
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $EE
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $00
	db $04
	db $0E
	db $9B
	db $71
	db $20
	db $00
	db $00
	db $00
	db $18
	db $3C
	db $7E
	db $CE
	db $86
	db $00
	db $00
	db $EE
	db $EF
	db $E0
	db $E0
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $4E
	db $CF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $00
	db $20
	db $20
	db $3F
	db $FF
	db $00
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4F
	db $40
	db $40
	db $4F
	db $4F
	db $4F
	db $4F
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $E0
	db $E7
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $07
	db $E7
	db $E7
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $4F
	db $CF
	db $0F
	db $0F
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $EF
	db $C7
	db $83
	db $01
	db $01
	db $01
	db $FF
	db $FF
	db $E7
	db $E7
	db $E3
	db $E3
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $66
	db $66
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $30
	db $30
	db $FF
	db $FF
	db $FF
	db $FF
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $05
	db $00
	db $10
	db $10
	db $F0
	db $F0
	db $00
	db $00
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4E
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $4F
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $C0
	db $CF
	db $CF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $0F
	db $CF
	db $FF
	db $00
	db $FF
	db $00
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $4F
	db $CF
	db $0F
	db $0F
	db $FF
	db $FF
	db $FF
	db $FF
	db $CF
	db $CF
	db $8F
	db $8F
	db $FF
	db $FF
	db $FF
	db $FF
	db $01
	db $01
	db $01
	db $83
	db $C7
	db $EF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FF
	db $FE
	db $FE
	db $FE
	db $FE
	db $FE
	db $FE
	db $FE
	db $00
	db $00
	db $05
	db $47
	db $00
	db $05
	db $47
	db $00
	db $00
	db $00
	db $00
	db $F0
	db $B0
	db $B0
	db $00
	db $00
	db $00
	db $03
	db $1F
	db $3C
	db $73
	db $6F
	db $EF
	db $FF
	db $80
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $03
	db $C0
	db $F8
	db $3C
	db $CE
	db $F6
	db $F7
	db $FF
	db $C3
	db $68
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $FC
	db $7E
	db $56
	db $7E
	db $56
	db $7E
	db $66
	db $7E
	db $BD
	db $99
	db $99
	db $99
	db $C3
	db $FF
	db $FB
	db $FB
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $E8
	db $7E
	db $7E
	db $42
	db $52
	db $52
	db $42
	db $42
	db $FE
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $FB
	db $06
	db $05
	db $47
	db $0E
	db $05
	db $47
	db $03
	db $03
	db $03
	db $03
	db $F0
	db $B0
	db $B0
	db $00
	db $00
	db $00
	db $1F
	db $3F
	db $5F
	db $EF
	db $F0
	db $F7
	db $F7
	db $80
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $F0
	db $F8
	db $F5
	db $EE
	db $1E
	db $DD
	db $DB
	db $00
	db $00
	db $00
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $00
	db $68
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $FF
	db $7E
	db $56
	db $7E
	db $56
	db $7E
	db $66
	db $7E
	db $7F
	db $F0
	db $E1
	db $C3
	db $81
	db $C0
	db $E1
	db $F3
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $D0
	db $E8
	db $7E
	db $7E
	db $42
	db $52
	db $52
	db $42
	db $42
	db $FE
	db $FF
	db $D1
	db $FF
	db $C0
	db $DE
	db $C0
	db $FF
	db $FF
	db $47
	db $05
	db $0E
	db $47
	db $05
	db $0E
	db $03
	db $03
	db $03
	db $03
	db $F0
	db $D0
	db $D0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FF
	db $00
	db $FF
	db $FF
	db $FF
	db $00
	db $0F
	db $1F
	db $AF
	db $77
	db $78
	db $BB
	db $DB
	db $00
	db $FF
	db $FF
	db $FF
	db $FF
	db $00
	db $FF
	db $FF
	db $00
	db $F8
	db $FC
	db $FA
	db $F7
	db $0F
	db $EF
	db $EF
	db $00
	db $FE
	db $CF
	db $87
	db $03
	db $81
	db $C3
	db $87
	db $0F
	db $FF
	db $7E
	db $6A
	db $7E
	db $6A
	db $7E
	db $66
	db $7E
	db $17
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $FF
	db $FF
	db $8B
	db $FF
	db $03
	db $EB
	db $03
	db $FF
	db $7E
	db $7E
	db $42
	db $52
	db $52
	db $42
	db $42
	db $7F
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $0B
	db $17
	db $07
	db $07
	db $07
	db $07
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $00
	db $FE
	db $FE
	db $00
	db $FB
	db $ED
	db $FE
	db $B0
	db $E0
	db $00
	db $FE
	db $FE
	db $01
	db $FF
	db $F0
	db $00
	db $00
	db $00
	db $FE
	db $FE
	db $01
	db $FF
	db $0F
	db $00
	db $00
	db $FE
	db $FE
	db $00
	db $DF
	db $B7
	db $7F
	db $0D
	db $07
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $90
	db $90
	db $90
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $78
	db $FC
	db $DE
	db $FE
	db $F6
	db $00
	db $00
	db $00
	db $1E
	db $3F
	db $7B
	db $7F
	db $6F
	db $F2
	db $F4
	db $86
	db $BF
	db $86
	db $F4
	db $F2
	db $F6
	db $4F
	db $2F
	db $61
	db $FD
	db $61
	db $2F
	db $4F
	db $6F
	db $FE
	db $DE
	db $FC
	db $78
	db $00
	db $00
	db $00
	db $00
	db $7F
	db $7B
	db $3F
	db $1E
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
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $10
	db $10
	db $00
	db $00
	db $00
	db $31
	db $1B
	db $1A
	db $0C
	db $07
	db $00
	db $00
	db $00
	db $E3
	db $63
	db $F3
	db $D5
	db $9D
	db $0C
	db $08
	db $00
	db $7C
	db $7E
	db $6C
	db $6C
	db $CC
	db $C4
	db $83
	db $80
	db $CE
	db $64
	db $4C
	db $CE
	db $E7
	db $EB
	db $4F
	db $6E
	db $7A
	db $32
	db $32
	db $12
	db $36
	db $27
	db $27
	db $27
	db $6F
	db $66
	db $72
	db $76
	db $76
	db $63
	db $61
	db $E7
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $F0
	db $80
	db $80
	db $00
	db $00
	db $00
	db $31
	db $3B
	db $3A
	db $3E
	db $77
	db $77
	db $F7
	db $C6
	db $E3
	db $63
	db $F3
	db $DD
	db $8D
	db $00
	db $00
	db $00
	db $7C
	db $7E
	db $2C
	db $AC
	db $A4
	db $E5
	db $43
	db $40
	db $CE
	db $64
	db $4C
	db $CC
	db $E8
	db $A8
	db $38
	db $10
	db $D2
	db $72
	db $F3
	db $E7
	db $E6
	db $E3
	db $73
	db $53
	db $72
	db $F6
	db $C6
	db $CE
	db $CE
	db $E3
	db $63
	db $36
	db $00
	db $00
	db $00
	db $40
	db $40
	db $40
	db $00
	db $00
	db $00
	db $7E
	db $7E
	db $FF
	db $7E
	db $3C
	db $5A
	db $66
	db $3C
	db $5A
	db $66
	db $66
	db $66
	db $66
	db $66
	db $66
	db $5A
	db $3C
	db $66
	db $5A
	db $3C
	db $7E
	db $FF
	db $7E
	db $7E
	db $40
	db $40
	db $40
	db $40
	db $00
	db $60
	db $60
	db $00
	db $00
	db $00
	db $01
	db $41
	db $61
	db $31
	db $39
	db $1D
	db $0F
	db $FB
	db $00
	db $02
	db $86
	db $8C
	db $9C
	db $B8
	db $70
	db $EF
	db $3F
	db $05
	db $0E
	db $1A
	db $1A
	db $33
	db $23
	db $03
	db $5C
	db $E0
	db $B0
	db $B8
	db $98
	db $4C
	db $44
	db $C0
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $36
	db $40
	db $60
	db $F0
	db $00
	db $00
	db $00
	db $00
	db $04
	db $0E
	db $0B
	db $1B
	db $1F
	db $1B
	db $33
	db $33
	db $3B
	db $31
	db $31
	db $39
	db $D9
	db $F9
	db $B9
	db $06
	db $0E
	db $1B
	db $7B
	db $FB
	db $B3
	db $33
	db $33
	db $00
	db $0F
	db $0B
	db $3B
	db $71
	db $73
	db $63
	db $71
	db $B9
	db $39
	db $31
	db $B0
	db $90
	db $B8
	db $B8
	db $F1
	db $B1
	db $99
	db $98
	db $DC
	db $D8
	db $D8
	db $7C
	db $FC
	db $00
	db $9C
	db $76
	db $E6
	db $E6
	db $F2
	db $E2
	db $E7
	db $00
	db $00
	db $00
	db $00
	db $00
	db $00
	db $42
	db $3C
	db $00
	db $00
	db $FE
	db $10
	db $10
	db $10
	db $10
	db $10
	db $00
	db $00
	db $42
	db $42
	db $42
	db $42
	db $42
	db $3C
RAMTOP:
	db $00
