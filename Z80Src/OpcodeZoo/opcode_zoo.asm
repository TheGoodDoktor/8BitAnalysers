	DEVICE ZXSPECTRUM48

START:
	; 8-bit load group
	;--------------------------------------
	ORG $8000

	; LD R, R' 
	LD A, B
	LD A, C
	LD A, D
	LD A, E
	LD A, H
	LD A, L

	LD B, B
	LD B, C
	LD B, D
	LD B, E
	LD B, H
	LD B, L

	LD C, B
	LD C, C
	LD C, D
	LD C, E
	LD C, H
	LD C, L

	LD D, B
	LD D, C
	LD D, D
	LD D, E
	LD D, H
	LD D, L

	LD E, B
	LD E, C
	LD E, D
	LD E, E
	LD E, H
	LD E, L

	LD H, B
	LD H, C
	LD H, D
	LD H, E
	LD H, H
	LD H, L

	LD L, B
	LD L, C
	LD L, D
	LD L, E
	LD L, H
	LD L, L

	; LD R, N 
	LD A, $99
	LD B, $99
	LD C, $99
	LD D, $99
	LD E, $99
	LD H, $99
	LD L, $99

	; LD R, (HL) 
	LD A, (HL)
	LD B, (HL)
	LD C, (HL)
	LD D, (HL)
	LD E, (HL)
	LD H, (HL)
	LD L, (HL)

	; LD R, (IX+D) 
	LD A, (IX+8)
	LD A, (IX-8)
	LD B, (IX+8)
	LD C, (IX+8)
	LD D, (IX+8)
	LD E, (IX+8)
	LD H, (IX+8)
	LD L, (IX+8)

	; LD R, (IY+D) 
	LD A, (IY+8)
	LD A, (IY-8)
	LD B, (IY+8)
	LD C, (IY+8)
	LD D, (IY+8)
	LD E, (IY+8)
	LD H, (IY+8)
	LD L, (IY+8)

	; LD (HL), R
	LD (HL), A
	LD (HL), B
	LD (HL), C
	LD (HL), D
	LD (HL), E
	LD (HL), H
	LD (HL), L

	; LD (IX+D), R 
	LD (IX+8), A
	LD (IX-8), A
	LD (IX+8), B
	LD (IX+8), C
	LD (IX+8), D
	LD (IX+8), E
	LD (IX+8), H
	LD (IX+8), L

	; LD (IY+D), R 
	LD (IY+8), A
	LD (IY-8), A
	LD (IY+8), B
	LD (IY+8), C
	LD (IY+8), D
	LD (IY+8), E
	LD (IY+8), H
	LD (IY+8), L

	; LD (HL), N 
	LD (HL), $99

	; LD (IX+D), N 
	LD (IX), $99

	; LD (IY+D), N 
	LD (IY), $99

	LD A, (BC) 
	LD A, (DE) 

	; LD A, (NN) 
	LD A, ($f123) 

	LD (BC), A 
	LD (DE), A 

	; LD (NN), A 
	LD ($f123), A 

	LD A, I 
	LD A, R 
	LD I,A 
	LD R, A 

	; 16-bit load group
	;--------------------------------------
	; LD dd, nn 
	LD BC, $1234
	LD DE, $1234
	LD HL, $1234
	LD SP, $1234

	; LD IX, nn 
	LD IX, $1234

	; LD IY, nn 
	LD IY, $1234

	; LD HL, (nn) 
	LD HL, ($F123)

	; LD dd, (nn)
	LD BC, ($F123)
	LD DE, ($F123)
	LD HL, ($F123)
	LD SP, ($F123)

	; LD IX, (nn) 
	LD IX, ($F123)

	; LD IY, (nn) 
	LD IY, ($F123)

	; LD (nn), HL 
	LD ($E123), HL 

	; LD (nn), dd 
	LD ($E123), BC 
	LD ($E123), DE 
	LD ($E123), HL 
	LD ($E123), SP 

	; LD (nn), IX 
	LD ($E123), IX

	; LD (nn), IY 
	LD ($E123), IY 

	LD SP, HL 
	LD SP, IX 
	LD SP, IY 

	; PUSH qq 
	PUSH BC
	PUSH DE
	PUSH HL
	PUSH AF

	PUSH IX 
	PUSH IY 

	; POP qq 
	POP BC
	POP DE
	POP HL
	POP AF

	POP IX
	POP IY

	; Exchange, Block Transfer, And Search Group
	;--------------------------------------
	; EX DE, HL 
	EX DE, HL 

	; EX AF, AF′ 
	EX AF, AF'

	EXX 

	; EX (SP), HL 
	EX (SP), IX 

	; EX (SP), IX 
	EX (SP), IX 

	; EX (SP), IY 
	EX (SP), IY 

	LDI 
	LDIR
	LDD 
	LDDR 
	CPI 
	CPIR 
	CPD 
	CPDR 

	; 8-bit arithmetic group
	;--------------------------------------
	; ADD A, r 
	ADD A, A
	ADD A, B
	ADD A, C
	ADD A, D
	ADD A, E
	ADD A, H
	ADD A, L

	; ADD A, n 
	ADD A, $99

	; ADD A, (HL) 
	ADD A, (HL) 

	; ADD A, (IX + d) 
	ADD A, (IX + 8) 
	ADD A, (IX - 8) 

	; ADD A, (IY + d) 
	ADD A, (IY + 8) 
	ADD A, (IY - 8) 

	; ADC A, s 
	ADC A, B
	ADC A, C
	ADC A, D
	ADC A, E
	ADC A, H
	ADC A, L
	ADC A, A
	ADC A, $99
	ADC A, (HL)
	ADC A, (IX + 8)
	ADC A, (IX - 8)
	ADC A, (IY + 8)
	ADC A, (IY - 8)


	; XOR s 
	XOR B
	XOR C
	XOR D
	XOR E
	XOR H
	XOR L
	XOR A
	XOR $99
	XOR (HL)
	XOR (IX + 8)
	XOR (IX - 8)
	XOR (IY + 8)
	XOR (IY - 8)

	; SBC A, s 
	SBC A, B
	SBC A, C
	SBC A, D
	SBC A, E
	SBC A, H
	SBC A, L
	SBC A, A
	SBC A, $99
	SBC A, (HL)
	SBC A, (IX + 8)
	SBC A, (IX - 8)
	SBC A, (IY + 8)
	SBC A, (IY - 8)

	; AND s 
	AND B
	AND C
	AND D
	AND E
	AND H
	AND L
	AND A
	AND $99
	AND (HL)
	AND (IX + 8)
	AND (IX - 8)
	AND (IY + 8)
	AND (IY - 8)

	; OR s 
	OR B
	OR C
	OR D
	OR E
	OR H
	OR L
	OR A
	OR $99
	OR (HL)
	OR (IX + 8)
	OR (IX - 8)
	OR (IY + 8)
	OR (IY - 8)

	; XOR s 
	XOR B
	XOR C
	XOR D
	XOR E
	XOR H
	XOR L
	XOR A
	XOR $99
	XOR (HL)
	XOR (IX + 8)
	XOR (IX - 8)
	XOR (IY + 8)
	XOR (IY - 8)

	; CP s 
	CP B
	CP C
	CP D
	CP E
	CP H
	CP L
	CP A
	CP $99
	CP (HL)
	CP (IX + 8)
	CP (IX - 8)
	CP (IY + 8)
	CP (IY - 8)

	; INC r 
	INC A
	INC B
	INC C
	INC D
	INC E
	INC H
	INC L

	INC (HL) 

	; INC (IX+d) 
	INC (IX + 8) 
	INC (IX - 8) 

	; INC (IY+d) 
	INC (IY + 8) 
	INC (IY - 8) 

	; DEC m 
	DEC B
	DEC C
	DEC D
	DEC E
	DEC H
	DEC L
	DEC A
	DEC (HL)
	DEC (IX + 8)
	DEC (IX - 8)
	DEC (IY + 8)
	DEC (IY - 8)

	; General-purpose Arithmetic And Cpu Control Group
	;--------------------------------------

	DAA 
	CPL 
	NEG 
	CCF 
	SCF 
	NOP 
	HALT 
	DI 
	EI 
	IM 0 
	IM 1 
	IM 2 

	; 16-bit arithmetic group
	;--------------------------------------
	; ADD HL, ss 
	ADD HL, BC
	ADD HL, DE
	ADD HL, HL
	ADD HL, SP

	; ADC HL, ss 
	ADC HL, BC
	ADC HL, DE
	ADC HL, HL
	ADC HL, SP

	; SBC HL, ss 
	SBC HL, BC
	SBC HL, DE
	SBC HL, HL
	SBC HL, SP

	; ADD IX, pp 
	ADD IX, BC
	ADD IX, DE
	ADD IX, IX
	ADD IX, SP

	; ADD IY, rr 
	ADD IY, BC
	ADD IY, DE
	ADD IY, IY
	ADD IY, SP

	; INC ss 
	INC BC
	INC DE
	INC HL
	INC SP

	INC IX 
	INC IY 

	; DEC ss 
	DEC BC
	DEC DE
	DEC HL
	DEC SP

	DEC IX 
	DEC IY 

	; Rotate And Shift Group
	;--------------------------------------
	RLCA 
	RLA 
	RRCA 
	RRA 

	; RLC r 
	RLC B
	RLC C
	RLC D
	RLC E
	RLC H
	RLC L
	RLC A

	RLC (HL) 

	; RLC (IX+d) 
	RLC (IX + 8) 
	RLC (IX - 8) 

	; RLC (IY+d) 
	RLC (IY + 8) 
	RLC (IY - 8) 

	; RL m 
	RL B
	RL C
	RL D
	RL E
	RL H
	RL L
	RL A
	RL (HL)
	RL (IX + 8)
	RL (IX - 8)
	RL (IY + 8)
	RL (IY - 8)

	; RRC m
	RRC B
	RRC C
	RRC D
	RRC E
	RRC H
	RRC L
	RRC A
	RRC (HL)
	RRC (IX + 8)
	RRC (IX - 8)
	RRC (IY + 8)
	RRC (IY - 8)

	; RR m 
	RR B
	RR C
	RR D
	RR E
	RR H
	RR L
	RR A
	RR (HL)
	RR (IX + 8)
	RR (IX - 8)
	RR (IY + 8)
	RR (IY - 8)

	; SLA m 
	SLA B
	SLA C
	SLA D
	SLA E
	SLA H
	SLA L
	SLA A
	SLA (HL)
	SLA (IX + 8)
	SLA (IX - 8)
	SLA (IY + 8)
	SLA (IY - 8)

	; SRA m 
	SRA B
	SRA C
	SRA D
	SRA E
	SRA H
	SRA L
	SRA A
	SRA (HL)
	SRA (IX + 8)
	SRA (IX - 8)
	SRA (IY + 8)
	SRA (IY - 8)

	; SRL m 
	SRL B
	SRL C
	SRL D
	SRL E
	SRL H
	SRL L
	SRL A
	SRL (HL)
	SRL (IX + 8)
	SRL (IX - 8)
	SRL (IY + 8)
	SRL (IY - 8)

	RLD 
	RRD 

	; Bit Set, Reset, And Test Group
	;--------------------------------------
	; BIT b, r 

	BIT 3, B
	BIT 3, C
	BIT 3, D
	BIT 3, E
	BIT 3, H
	BIT 3, L
	BIT 3, A

	; BIT b, (HL) 
	BIT 0, (HL)
	BIT 1, (HL)
	BIT 2, (HL)
	BIT 3, (HL)
	BIT 4, (HL)
	BIT 5, (HL)
	BIT 6, (HL)
	BIT 7, (HL)

	; BIT b, (IX+d) 
	BIT 3, (IX + 8)

	; BIT b, (IY+d) 
	BIT 3, (IY + 8)

	; SET b, r 
	SET 3, B
	SET 3, C
	SET 3, D
	SET 3, E
	SET 3, H
	SET 3, L
	SET 3, A

	; SET b, (HL) 
	BIT 3, (HL)
	

	; SET b, (IX+d) 
	BIT 3, (IX + 8)
	

	; SET b, (IY+d) 
	BIT 3, (IY + 8)

	; RES b, m 
	RES 3, B
	RES 3, C
	RES 3, D
	RES 3, E
	RES 3, H
	RES 3, L
	RES 3, A
	RES 3, (HL)
	RES 3, (IX+$8)
	RES 3, (IY+$8)

	; Jump Group
	;--------------------------------------
	; JP nn 
	JP $D123

	; JP cc, nn 
	JP NZ, $D123
	JP Z, $D123
	JP NC, $D123
	JP C, $D123
	JP PO, $D123
	JP P, $D123
	JP M, $D123

BRANCH_BACK:
	; JR e 
	JR BRANCH_BACK
	JR BRANCH_FWD

	; JR C, e 
	JR C, BRANCH_BACK
	JR C, BRANCH_FWD

	; JR NC, e 
	JR NC, BRANCH_BACK
	JR NC, BRANCH_FWD

	; JR Z, e 
	JR Z, BRANCH_BACK
	JR Z, BRANCH_FWD

	; JR NZ, e 
	JR NZ, BRANCH_BACK
	JR NZ, BRANCH_FWD

	JP (HL) 
	JP (IX) 
	JP (IY) 

LOOP_BACK:
	; DJNZ, e 
	DJNZ LOOP_BACK 
	DJNZ LOOP_FWD 


	; Call And Return Group
	;--------------------------------------
	; CALL nn 
	CALL $C123

	; CALL cc, nn 
	CALL NZ, $C123
	CALL Z, $C123
	CALL NC, $C123
	CALL C, $C123
	CALL PO, $C123
	CALL P, $C123
	CALL M, $C123

BRANCH_FWD:

	RET 

	; RET cc 
	RET NZ
	RET Z 
	RET NC
	RET C 
	RET PO
	RET P 
	RET M 

	RETI 
	RETN 

LOOP_FWD:

	; RST p 
	RST $0
	RST $8
	RST $10
	RST $18
	RST $20
	RST $28
	RST $30
	RST $38


	; Input And Output Group 
	;--------------------------------------
	; IN A, (n) 
	IN A, ($FE)

	; IN r (C) 
	IN B, (C)
	IN C, (C)
	IN D, (C)
	IN E, (C)
	IN H, (C)
	IN L, (C)
	IN A, (C)

	INI 
	INIR 
	IND 
	INDR 

	; OUT (n), A 
	OUT ($FE), A

	; OUT (C), r 
	OUT (C), B 
	OUT (C), C 
	OUT (C), D 
	OUT (C), E 
	OUT (C), H 
	OUT (C), L 
	OUT (C), A 

	OUTI 
	OTIR 
	OUTD 
	OTDR 


	DEFS 32

	; Bit Set, Reset, And Test Group Full
	;--------------------------------------
	; BIT b, r 
	BIT 0, B
	BIT 0, C
	BIT 0, D
	BIT 0, E
	BIT 0, H
	BIT 0, L
	BIT 0, A

	BIT 1, B
	BIT 1, C
	BIT 1, D
	BIT 1, E
	BIT 1, H
	BIT 1, L
	BIT 1, A

	BIT 2, B
	BIT 2, C
	BIT 2, D
	BIT 2, E
	BIT 2, H
	BIT 2, L
	BIT 2, A

	BIT 3, B
	BIT 3, C
	BIT 3, D
	BIT 3, E
	BIT 3, H
	BIT 3, L
	BIT 3, A

	BIT 4, B
	BIT 4, C
	BIT 4, D
	BIT 4, E
	BIT 4, H
	BIT 4, L
	BIT 4, A

	BIT 5, B
	BIT 5, C
	BIT 5, D
	BIT 5, E
	BIT 5, H
	BIT 5, L
	BIT 5, A

	BIT 6, B
	BIT 6, C
	BIT 6, D
	BIT 6, E
	BIT 6, H
	BIT 6, L
	BIT 6, A

	BIT 7, B
	BIT 7, C
	BIT 7, D
	BIT 7, E
	BIT 7, H
	BIT 7, L
	BIT 7, A

	; BIT b, (HL) 
	BIT 0, (HL)
	BIT 1, (HL)
	BIT 2, (HL)
	BIT 3, (HL)
	BIT 4, (HL)
	BIT 5, (HL)
	BIT 6, (HL)
	BIT 7, (HL)

	; BIT b, (IX+d) 
	BIT 0, (IX + 8)
	BIT 0, (IX - 8)
	BIT 1, (IX + 8)
	BIT 2, (IX + 8)
	BIT 3, (IX + 8)
	BIT 4, (IX + 8)
	BIT 5, (IX + 8)
	BIT 6, (IX + 8)
	BIT 7, (IX + 8)

	; BIT b, (IY+d) 
	BIT 0, (IY + 8)
	BIT 0, (IY - 8)
	BIT 1, (IY + 8)
	BIT 2, (IY + 8)
	BIT 3, (IY + 8)
	BIT 4, (IY + 8)
	BIT 5, (IY + 8)
	BIT 6, (IY + 8)
	BIT 7, (IY + 8)

	; SET b, r 
	SET 0, B
	SET 0, C
	SET 0, D
	SET 0, E
	SET 0, H
	SET 0, L
	SET 0, A

	SET 1, B
	SET 1, C
	SET 1, D
	SET 1, E
	SET 1, H
	SET 1, L
	SET 1, A

	SET 2, B
	SET 2, C
	SET 2, D
	SET 2, E
	SET 2, H
	SET 2, L
	SET 2, A

	SET 3, B
	SET 3, C
	SET 3, D
	SET 3, E
	SET 3, H
	SET 3, L
	SET 3, A

	SET 4, B
	SET 4, C
	SET 4, D
	SET 4, E
	SET 4, H
	SET 4, L
	SET 4, A

	SET 5, B
	SET 5, C
	SET 5, D
	SET 5, E
	SET 5, H
	SET 5, L
	SET 5, A

	SET 6, B
	SET 6, C
	SET 6, D
	SET 6, E
	SET 6, H
	SET 6, L
	SET 6, A

	SET 7, B
	SET 7, C
	SET 7, D
	SET 7, E
	SET 7, H
	SET 7, L
	SET 7, A

	; SET b, (HL) 
	BIT 0, (HL)
	BIT 1, (HL)
	BIT 2, (HL)
	BIT 3, (HL)
	BIT 4, (HL)
	BIT 5, (HL)
	BIT 6, (HL)
	BIT 7, (HL)

	; SET b, (IX+d) 
	BIT 0, (IX + 8)
	BIT 0, (IX - 8)
	BIT 1, (IX + 8)
	BIT 2, (IX + 8)
	BIT 3, (IX + 8)
	BIT 4, (IX + 8)
	BIT 5, (IX + 8)
	BIT 6, (IX + 8)
	BIT 7, (IX + 8)

	; SET b, (IY+d) 
	BIT 0, (IY + 8)
	BIT 0, (IY - 8)
	BIT 1, (IY + 8)
	BIT 2, (IY + 8)
	BIT 3, (IY + 8)
	BIT 4, (IY + 8)
	BIT 5, (IY + 8)
	BIT 6, (IY + 8)
	BIT 7, (IY + 8)

	; RES b, m 
	RES 0, B
	RES 0, C
	RES 0, D
	RES 0, E
	RES 0, H
	RES 0, L
	RES 0, A
	RES 0, (HL)
	RES 0, (IX + 8)
	RES 0, (IX - 8)
	RES 0, (IY + 8)
	RES 0, (IY - 8)

	RES 1, B
	RES 1, C
	RES 1, D
	RES 1, E
	RES 1, H
	RES 1, L
	RES 1, A
	RES 1, (HL)
	RES 1, (IX+$8)
	RES 1, (IY+$8)

	RES 2, B
	RES 2, C
	RES 2, D
	RES 2, E
	RES 2, H
	RES 2, L
	RES 2, A
	RES 2, (HL)
	RES 2, (IX+$8)
	RES 2, (IY+$8)

	RES 3, B
	RES 3, C
	RES 3, D
	RES 3, E
	RES 3, H
	RES 3, L
	RES 3, A
	RES 3, (HL)
	RES 3, (IX+$8)
	RES 3, (IY+$8)

	RES 4, B
	RES 4, C
	RES 4, D
	RES 4, E
	RES 4, H
	RES 4, L
	RES 4, A
	RES 4, (HL)
	RES 4, (IX+$8)
	RES 4, (IY+$8)

	RES 5, B
	RES 5, C
	RES 5, D
	RES 5, E
	RES 5, H
	RES 5, L
	RES 5, A
	RES 5, (HL)
	RES 5, (IX+$8)
	RES 5, (IY+$8)

	RES 6, B
	RES 6, C
	RES 6, D
	RES 6, E
	RES 6, H
	RES 6, L
	RES 6, A
	RES 6, (HL)
	RES 6, (IX+$8)
	RES 6, (IY+$8)

	RES 7, B
	RES 7, C
	RES 7, D
	RES 7, E
	RES 7, H
	RES 7, L
	RES 7, A
	RES 7, (HL)
	RES 7, (IX+$8)
	RES 7, (IY+$8)


	
	SAVESNA "Opcode_Zoo.sna", START

