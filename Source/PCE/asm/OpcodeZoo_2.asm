; ============================================================
; HuC6280 opcode reference
; Grouped by opcode page
; ============================================================

	.cpu 6280
	.list
	.org $E000

; there should be 234 opcodes

; to get working:
; 0f 10 e2
; BBR0 < $10,$E000 should be BBR0 <$10,Label
; ...
; BBR7

; 8f 10 e0
; BBS0 < $10,$E000 should be BBS0 <$10,Label
; ...
; BBS7

; 93 0f f0 ff
; Addr. mode : 18 'immediate absolute'
; TST #$0F,$FFF0 should be TST #$0F,data_FFF0
; 
; b3 0f f0 ff
; Addr. mode : 20 'immediate absolute indexed'
; TST #$0F,$FFF0,X should be TST #$0F,data_FFF0,X


; ------------------------------------------------------------
; Startup code
; ------------------------------------------------------------

EntryPoint:

    SEI
    CSH
    CLD
    LDA #$FF    ;MAP IN I/O
    TAM #0
    TAX
    LDA #$F8    ;MAP IN RAM
    TAM #1
.loop
    BRA .loop

; ------------------------------------------------------------
; 00–0F
; ------------------------------------------------------------

    BRK                         ; 00
    ORA [$10,X]                 ; 01
    SXY                         ; 02
    ST0 #$10                    ; 03
    TSB <$10                    ; 04
    ORA <$10                    ; 05
    ASL <$10                    ; 06
    RMB0 <$10                   ; 07
    PHP                         ; 08
    ORA #$10                    ; 09
    ASL                         ; 0A
    BRK                         ; 0B missing
    TSB data_FFF0               ; 0C
    ORA data_FFF0               ; 0D
    ASL data_FFF0               ; 0E
    BBR0 <$10,EntryPoint        ; 0F

; ------------------------------------------------------------
; 10–1F
; ------------------------------------------------------------

.label_10_1F:

    BPL .label_10_1F            ; 10
    ORA [$10],Y                 ; 11
    ORA [<$10]                  ; 12
    ST1 #$10                    ; 13
    TRB <$10                    ; 14
    ORA <$10,X                  ; 15
    ASL <$10,X                  ; 16
    RMB1 <$10                   ; 17
    CLC                         ; 18
    ORA data_FFF0,Y             ; 19
    INC                         ; 1A
    BRK                         ; 1B missing
    TRB data_FFF0               ; 1C
    ORA data_FFF0,X             ; 1D
    ASL data_FFF0,X             ; 1E
    BBR1 <$10,.label_10_1F      ; 1F

; ------------------------------------------------------------
; 20–2F
; ------------------------------------------------------------

.label_20_2f:

    JSR data_FFF0               ; 20
    AND [$10,X]                 ; 21
    SAX                         ; 22
    ST2 #$10                    ; 23
    BIT <$10                    ; 24
    AND <$10                    ; 25
    ROL <$10                    ; 26
    RMB2 <$10                   ; 27
    PLP                         ; 28
    AND #$10                    ; 29
    ROL                         ; 2A
    BRK                         ; 2B missing
    BIT data_FFF0               ; 2C
    AND data_FFF0               ; 2D
    ROL data_FFF0               ; 2E
    BBR2 <$10,.label_20_2f      ; 1F

; ------------------------------------------------------------
; 30–3F
; ------------------------------------------------------------

.label_30_3f:

    BMI .label_30_3f            ; 30
    AND [$10],Y                 ; 31
    AND [<$10]                  ; 32
    BRK                         ; 33 missing
    BIT <$10,X                  ; 34
    AND <$10,X                  ; 35
    ROL <$10,X                  ; 36
    RMB3 <$10                   ; 37
    SEC                         ; 38
    AND data_FFF0,Y             ; 39
    DEC                         ; 3A
    BRK                         ; 3B missing
    BIT data_FFF0,X             ; 3C
    AND data_FFF0,X             ; 3D
    ROL data_FFF0,X             ; 3E
    BBR3 <$10,.label_30_3f      ; 3F

; ------------------------------------------------------------
; 40–4F
; ------------------------------------------------------------

.label_40_4f:

    RTI                         ; 40
    EOR [$10,X]                 ; 41
    SAY                         ; 42
    TMA #$02                    ; 43
    BSR .label_40_4f            ; 44
    EOR <$10                    ; 45
    LSR <$10                    ; 46
    RMB4 <$10                   ; 47
    PHA                         ; 48
    EOR #$10                    ; 49
    LSR                         ; 4A
    BRK                         ; 4B missing
    JMP data_FFF0               ; 4C
    EOR data_FFF0               ; 4D
    LSR data_FFF0               ; 4E
    BBR4 <$10,.label_40_4f      ; 4F

; ------------------------------------------------------------
; 50–5F
; ------------------------------------------------------------

.label_50_5f:

    BVC .label_50_5f            ; 50
    EOR [$10],Y                 ; 51
    EOR [<$10]                  ; 52
    TAM #$01                    ; 53
    CSL                         ; 54
    EOR <$10,X                  ; 55
    LSR <$10,X                  ; 56
    RMB5 <$10                   ; 57
    CLI                         ; 58
    EOR data_FFF0,Y             ; 59
    PHY                         ; 5A
    BRK                         ; 5B missing
    BRK                         ; 5C missing
    EOR data_FFF0,X             ; 5D
    LSR data_FFF0,X             ; 5E
    BBR5 <$10,.label_50_5f      ; 5F

; ------------------------------------------------------------
; 60–6F
; ------------------------------------------------------------

.label_60_6f:

    RTS                         ; 60
    ADC [$10,X]                 ; 61
    CLA                         ; 62
    BRK                         ; 63 missing
    STZ <$10                    ; 64
    ADC <$10                    ; 65
    ROR <$10                    ; 66
    RMB6 <$10                   ; 67
    PLA                         ; 68
    ADC #$10                    ; 69
    ROR                         ; 6A
    BRK                         ; 6B missing
    JMP [data_FFF0]             ; 6C
    ADC data_FFF0               ; 6D
    ROR data_FFF0               ; 6E
    BBR6 <$10,.label_60_6f      ; 6F

; ------------------------------------------------------------
; 70–7F
; ------------------------------------------------------------

.label_70_7f:

    BVS .label_70_7f            ; 70
    ADC [$10],Y                 ; 71
    ADC [<$10]                  ; 72
    TII $1000,$2000,$0010       ; 73
    STZ <$10,X                  ; 74
    ADC <$10,X                  ; 75
    ROR <$10,X                  ; 76
    RMB7 <$10                   ; 77
    SEI                         ; 78
    ADC data_FFF0,Y             ; 79
    PLY                         ; 7A
    BRK                         ; 7B missing
    JMP [data_FFF0,X]           ; 7C
    ADC data_FFF0,X             ; 7D
    ROR data_FFF0,X             ; 7E
    BBR7 <$10,.label_70_7f      ; 7F

; ------------------------------------------------------------
; 80–8F
; ------------------------------------------------------------

.label_80_8f:

    BRA .label_80_8f            ; 80
    STA [$10,X]                 ; 81
    CLX                         ; 82
    TST #$0F,<$10               ; 83
    STY <$10                    ; 84
    STA <$10                    ; 85
    STX <$10                    ; 86
    SMB0 <$10                   ; 87
    DEY                         ; 88
    BIT #$10                    ; 89
    TXA                         ; 8A
    BRK                         ; 8B missing
    STY data_FFF0               ; 8C
    STA data_FFF0               ; 8D
    STX data_FFF0               ; 8E
    BBS0 <$10,.label_80_8f      ; 8F

; ------------------------------------------------------------
; 90–9F
; ------------------------------------------------------------

.label_90_9f:

    BCC .label_90_9f            ; 90
    STA [$10],Y                 ; 91
    STA [<$10]                  ; 92
    TST #$0F,data_FFF0          ; 93
    STY <$10,X                  ; 94
    STA <$10,X                  ; 95
    STX <$10,Y                  ; 96
    SMB1 <$10                   ; 97
    TYA                         ; 98
    STA data_FFF0,Y             ; 99
    TXS                         ; 9A
    BRK                         ; 9B missing
    STZ data_FFF0               ; 9C
    STA data_FFF0,X             ; 9D
    STZ data_FFF0,X             ; 9E
    BBS1 <$10,.label_90_9f      ; 9F

; ------------------------------------------------------------
; A0–AF
; ------------------------------------------------------------

.label_a0_af:

    LDY #$10                    ; A0
    LDA [$10,X]                 ; A1
    LDX #$10                    ; A2
    TST #$10,<$10,X             ; A3
    LDY <$10                    ; A4
    LDA <$10                    ; A5
    LDX <$10                    ; A6
    SMB2 <$10                   ; A7
    TAY                         ; A8
    LDA #$10                    ; A9
    TAX                         ; AA
    BRK                         ; AB missing
    LDY data_FFF0               ; AC
    LDA data_FFF0               ; AD
    LDX data_FFF0               ; AE
    BBS2 <$10,.label_a0_af      ; AF

; ------------------------------------------------------------
; B0–BF
; ------------------------------------------------------------

.label_b0_bf:

    BCS .label_b0_bf            ; B0
    LDA [$10],Y                 ; B1
    LDA [<$10]                  ; B2
    TST #$0F,data_FFF0,X        ; B3
    LDY <$10,X                  ; B4
    LDA <$10,X                  ; B5
    LDX <$10,Y                  ; B6
    SMB3 <$10                   ; B7
    CLV                         ; B8
    LDA data_FFF0,Y             ; B9
    TSX                         ; BA
    BRK                         ; BB missing
    LDY data_FFF0,X             ; BC
    LDA data_FFF0,X             ; BD
    LDX data_FFF0,Y             ; BE
    BBS3 <$10,.label_b0_bf      ; BF

; ------------------------------------------------------------
; C0–CF
; ------------------------------------------------------------

.label_c0_cf:

    CPY #$10                    ; C0
    CMP [$10,X]                 ; C1
    CLY                         ; C2
    TDD $1000,$2000,$0010       ; C3
    CPY <$10                    ; C4
    CMP <$10                    ; C5
    DEC <$10                    ; C6
    SMB4 <$10                   ; C7
    INY                         ; C8
    CMP #$10                    ; C9
    DEX                         ; CA
    BRK                         ; CB missing
    CPY data_FFF0               ; CC
    CMP data_FFF0               ; CD
    DEC data_FFF0               ; CE
    BBS4 <$10,.label_c0_cf      ; CF

; ------------------------------------------------------------
; D0–DF
; ------------------------------------------------------------

.label_d0_df:

    BNE .label_d0_df            ; D0
    CMP [$10],Y                 ; D1
    CMP [<$10]                  ; D2
    TIN $1000,$2000,$0010       ; D3
    CSH                         ; D4
    CMP <$10,X                  ; D5
    DEC <$10,X                  ; D6
    SMB5 <$10                   ; D7
    CLD                         ; D8
    CMP data_FFF0,Y             ; D9
    PHX                         ; DA
    BRK                         ; DB missing
    BRK                         ; DC missing
    CMP data_FFF0,X             ; DD
    DEC data_FFF0,X             ; DE
    BBS5 <$10,.label_d0_df      ; DF

; ------------------------------------------------------------
; E0–EF
; ------------------------------------------------------------

.label_e0_ef:

    CPX #$10                    ; E0
    SBC [$10,X]                 ; E1
    BRK                         ; E2 missing
    TIA $1000,$2000,$0010       ; E3
    CPX <$10                    ; E4
    SBC <$10                    ; E5
    INC <$10                    ; E6
    SMB6 <$10                   ; E7
    INX                         ; E8
    SBC #$10                    ; E9
    NOP                         ; EA
    BRK                         ; EB missing
    CPX data_FFF0               ; EC
    SBC data_FFF0               ; ED
    INC data_FFF0               ; EE
    BBS6 <$10,.label_e0_ef      ; EF

; ------------------------------------------------------------
; F0–FF
; ------------------------------------------------------------

.label_f0_ff:

    BEQ .label_f0_ff            ; F0
    SBC [$10],Y                 ; F1
    SBC [<$10]                  ; F2
    TAI $1000,$2000,$0010       ; F3
    SET                         ; F4
    SBC <$10,X                  ; F5
    INC <$10,X                  ; F6
    SMB7 <$10                   ; F7
    SED                         ; F8
    SBC data_FFF0,Y             ; F9
    PLX                         ; FA
    BRK                         ; FB missing
    BRK                         ; FC missing
    SBC data_FFF0,X             ; FD
    INC data_FFF0,X             ; FE
    BBS7 <$10,.label_f0_ff      ; FF

; ------------------------------------------------------------
; End of opcode sequence
; ------------------------------------------------------------

    NOP
    NOP
    NOP
    NOP
    RTS

    .org $fff0
    data_FFF0:

	.org $fffe
	.dw $e000
;	.end