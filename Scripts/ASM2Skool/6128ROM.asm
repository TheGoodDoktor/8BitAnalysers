; Disassembly of the CPC6128 operating system ROM

;; START OF LOW KERNEL JUMPBLOCK AND ROM START
;;
;; firmware register assignments:
;; B' = 0x07f - Gate Array I/O port address (upper 8 bits)
;; C': upper/lower rom enabled state and current mode. Bit 7 = 1, Bit 6 = 0.

;----------------------------------------------------------------
; RST 0 - LOW: RESET ENTRY
0000 01897f    ld      bc,$7f89		; select mode 1, disable upper rom, enable lower rom		
0003 ed49      out     (c),c		; select mode and rom configuration
0005 c39105    jp      $0591
;----------------------------------------------------------------
0008 c38ab9    jp      $b98a			; RST 1 - LOW: LOW JUMP
;----------------------------------------------------------------
000b c384b9    jp      $b984			; LOW: KL LOW PCHL
;----------------------------------------------------------------
000e c5        push    bc				; LOW: PCBC INSTRUCTION
000f c9        ret     
;----------------------------------------------------------------
0010 c31dba    jp      $ba1d			; RST 2 - LOW: SIDE CALL
;----------------------------------------------------------------
0013 c317ba    jp      $ba17			; LOW: KL SIDE PCHL
;----------------------------------------------------------------
0016 d5        push    de				; LOW: PCDE INSTRUCTION
0017 c9        ret     
;----------------------------------------------------------------
0018 c3c7b9    jp      $b9c7			; RST 3 - LOW: FAR CALL
;----------------------------------------------------------------
001b c3b9b9    jp      $b9b9			; LOW: KL FAR PCHL
;----------------------------------------------------------------
001e e9        jp      (hl)				; LOW: PCHL INSTRUCTION
;----------------------------------------------------------------
001f 00        nop     
;----------------------------------------------------------------
0020 c3c6ba    jp      $bac6			; RST 4 - LOW: RAM LAM
;----------------------------------------------------------------
0023 c3c1b9    jp      $b9c1			; LOW: KL FAR ICALL
;----------------------------------------------------------------
0026 00        nop     
0027 00        nop     
;----------------------------------------------------------------
0028 c335ba    jp      $ba35			; RST 5 - LOW: FIRM JUMP
002b 00        nop     
002c ed49      out     (c),c
002e d9        exx     
002f fb        ei      
;----------------------------------------------------------------
0030 f3        di						; RST 6 - LOW: USER RESTART
0031 d9        exx     
0032 212b00    ld      hl,$002b
0035 71        ld      (hl),c
0036 1808      jr      $0040            
;----------------------------------------------------------------
0038 c341b9    jp      $b941			; RST 7 - LOW: INTERRUPT ENTRY
;----------------------------------------------------------------
;; This is the default handler in the ROM. The user can patch the RAM version of this
;; handler.
003b c9        ret						; LOW: EXT INTERRUPT

;----------------------------------------------------------------

003c 00        nop     
003d 00        nop     
003e 00        nop     
003f 00        nop     
;; END OF LOW KERNEL JUMPBLOCK
;;----------------------------------------------------------------------------------------

0040 cbd1      set     2,c
0042 18e8      jr      $002c            ; (-$18)

;;----------------------------------------------------------------------------------------
;; Setup LOW KERNEL jumpblock

;; Copy RSTs to RAM
0044 214000    ld      hl,$0040			; copy first &40 bytes of this rom to &0000
										; in RAM, and therefore initialise low kernel jumpblock
0047 2d        dec     l
0048 7e        ld      a,(hl)			; get byte from rom
0049 77        ld      (hl),a			; write byte to ram
004a 20fb      jr      nz,$0047         ; 

;; initialise USER RESTART in LOW KERNEL jumpblcok
004c 3ec7      ld      a,$c7
004e 323000    ld      ($0030),a

;; Setup HIGH KERNEL jumpblock

0051 21a603    ld      hl,$03a6			; copy high kernel jumpblock
0054 1100b9    ld      de,$b900
0057 01e401    ld      bc,$01e4
005a edb0      ldir    

;;==========================================================================
;; KL CHOKE OFF

005c f3        di      
005d 3ad9b8    ld      a,($b8d9)
0060 ed5bd7b8  ld      de,($b8d7)
0064 06cd      ld      b,$cd
0066 212db8    ld      hl,$b82d
0069 3600      ld      (hl),$00
006b 23        inc     hl
006c 10fb      djnz    $0069            ; (-$05)
006e 47        ld      b,a
006f 0eff      ld      c,$ff
0071 a9        xor     c
0072 c0        ret     nz
0073 47        ld      b,a
0074 5f        ld      e,a
0075 57        ld      d,a
0076 c9        ret     

;;==========================================================================
;; this is called at the very end just before BASIC is started 
;;
;; HL = address to start
;; C = rom select 
;;
;; if HL=0, then BASIC is started.

0077 7c        ld      a,h
0078 b5        or      l
0079 79        ld      a,c
007a 2004      jr      nz,$0080          ; HL=0?

;; yes, HL = 0
007c 7d        ld      a,l				; A = 0 (BASIC)
007d 2106c0    ld      hl,$c006			; execution address for BASIC

;; A = rom select 
;; HL = address to start
0080 32d6b8    ld      ($b8d6),a
;; initialise three byte far address
0083 32d9b8    ld      ($b8d9),a		; rom select byte
0086 22d7b8    ld      ($b8d7),hl		; address

0089 21ffab    ld      hl,$abff
008c 114000    ld      de,$0040
008f 01ffb0    ld      bc,$b0ff
0092 3100c0    ld      sp,$c000
0095 df        rst     $18				; RST 3 - LOW: FAR CALL
0096
defw &b8d7
0098 c7        rst     $00				; RST 0 - LOW: RESET ENTRY

;;==========================================================================
;; KL TIME PLEASE

0099 f3        di      
009a ed5bb6b8  ld      de,($b8b6)
009e 2ab4b8    ld      hl,($b8b4)
00a1 fb        ei      
00a2 c9        ret     

;;==========================================================================
;; KL TIME SET

00a3 f3        di      
00a4 af        xor     a
00a5 32b8b8    ld      ($b8b8),a
00a8 ed53b6b8  ld      ($b8b6),de
00ac 22b4b8    ld      ($b8b4),hl
00af fb        ei      
00b0 c9        ret     

;;==========================================================================

;; update TIME
00b1 21b4b8    ld      hl,$b8b4
00b4 34        inc     (hl)
00b5 23        inc     hl
00b6 28fc      jr      z,$00b4          ; (-$04)

;; test VSYNC state
00b8 06f5      ld      b,$f5
00ba ed78      in      a,(c)
00bc 1f        rra     
00bd 3008      jr      nc,$00c7         

;; VSYNC is set
00bf 2ab9b8    ld      hl,($b8b9)		;; FRAME FLY events
00c2 7c        ld      a,h
00c3 b7        or      a
00c4 c45301    call    nz,$0153

00c7 2abbb8    ld      hl,($b8bb)		;; FAST TICKER events
00ca 7c        ld      a,h
00cb b7        or      a
00cc c45301    call    nz,$0153

00cf cdd720    call    $20d7			;; process sound

00d2 21bfb8    ld      hl,$b8bf			;; keyboard scan interrupt counter
00d5 35        dec     (hl)
00d6 c0        ret     nz

00d7 3606      ld      (hl),$06			;; reset keyboard scan interrupt counter

00d9 cdf4bd    call    $bdf4			; IND: KM SCAN KEYS

00dc 2abdb8    ld      hl,($b8bd)		; ticker list
00df 7c        ld      a,h
00e0 b7        or      a
00e1 c8        ret     z

00e2 2131b8    ld      hl,$b831			; indicate there are some ticker events to process?
00e5 cbc6      set     0,(hl)
00e7 c9        ret     

;;------------------------------------------------------------------------------
;; these two are for queuing up normal Asynchronous events to be processed after all others

;; normal event 
00e8 2b        dec     hl
00e9 3600      ld      (hl),$00
00eb 2b        dec     hl
;; has list been setup?
00ec 3a2eb8    ld      a,($b82e)
00ef b7        or      a
00f0 200c      jr      nz,$00fe         ; (+$0c)
;; add to start of list
00f2 222db8    ld      ($b82d),hl
00f5 222fb8    ld      ($b82f),hl
;; signal normal event list setup
00f8 2131b8    ld      hl,$b831
00fb cbf6      set     6,(hl)
00fd c9        ret     

;; add another event to 
00fe ed5b2fb8  ld      de,($b82f)
0102 222fb8    ld      ($b82f),hl
0105 eb        ex      de,hl
0106 73        ld      (hl),e
0107 23        inc     hl
0108 72        ld      (hl),d
0109 c9        ret     

;;---------------------------------------------------
;; synchronous??
010a ed7332b8  ld      ($b832),sp
010e 31b4b8    ld      sp,$b8b4
0111 e5        push    hl
0112 d5        push    de
0113 c5        push    bc
;; normal event has been setup?
0114 2131b8    ld      hl,$b831
0117 cb76      bit     6,(hl)
0119 281e      jr      z,$0139          ; (+$1e)

011b cbfe      set     7,(hl)
011d 2a2db8    ld      hl,($b82d)
0120 7c        ld      a,h
0121 b7        or      a
0122 280e      jr      z,$0132          ; (+$0e)
0124 5e        ld      e,(hl)
0125 23        inc     hl
0126 56        ld      d,(hl)
0127 ed532db8  ld      ($b82d),de
012b 23        inc     hl
012c cd0902    call    $0209			; execute event function
012f f3        di      
0130 18eb      jr      $011d            ; (-$15)

;;---------------------------------------------------
0132 2131b8    ld      hl,$b831
0135 cb46      bit     0,(hl)
0137 2810      jr      z,$0149          ; (+$10)
0139 3600      ld      (hl),$00
013b 37        scf     
013c 08        ex      af,af'
013d cd8901    call    $0189			;; execute ticker
0140 b7        or      a
0141 08        ex      af,af'
0142 2131b8    ld      hl,$b831
0145 7e        ld      a,(hl)
0146 b7        or      a
0147 20d2      jr      nz,$011b         ; (-$2e)
0149 3600      ld      (hl),$00
014b c1        pop     bc
014c d1        pop     de
014d e1        pop     hl
014e ed7b32b8  ld      sp,($b832)
0152 c9        ret     

;;---------------------------------------------------------------------
;; loop over events
;;
;; HL = address of event list
0153 5e        ld      e,(hl)			
0154 23        inc     hl
0155 7e        ld      a,(hl)			
0156 23        inc     hl
0157 b7        or      a
0158 cae201    jp      z,$01e2			; KL EVENT

015b 57        ld      d,a
015c d5        push    de
015d cde201    call    $01e2			; KL EVENT
0160 e1        pop     hl
0161 18f0      jr      $0153            ; (-$10)

;;==========================================================================
;; KL NEW FRAME FLY

0163 e5        push    hl
0164 23        inc     hl
0165 23        inc     hl
0166 cdd201    call    $01d2			; KL INIT EVENT
0169 e1        pop     hl

;;==========================================================================
;; KL ADD FRAME FLY

016a 11b9b8    ld      de,$b8b9
016d c37903    jp      $0379 ;; add event to list

;;==========================================================================
;; KL DEL FRAME FLY

0170 11b9b8    ld      de,$b8b9
0173 c38803    jp      $0388 ; remove event from list

;;==========================================================================
;; KL NEW FAST TICKER

0176 e5        push    hl
0177 23        inc     hl
0178 23        inc     hl
0179 cdd201    call    $01d2			; KL INIT EVENT
017c e1        pop     hl

;;==========================================================================
;; KL ADD FAST TICKER

;; HL = address of event block
017d 11bbb8    ld      de,$b8bb
0180 c37903    jp      $0379 ;; add event to list

;;==========================================================================
;; KL DEL FAST TICKER

;; HL = address of event block
0183 11bbb8    ld      de,$b8bb
0186 c38803    jp      $0388 ; remove event from list

;;==========================================================================

0189 2abdb8    ld      hl,($b8bd)			; ticker list
018c 7c        ld      a,h
018d b7        or      a
018e c8        ret     z

018f 5e        ld      e,(hl)
0190 23        inc     hl
0191 56        ld      d,(hl)
0192 23        inc     hl
0193 4e        ld      c,(hl)
0194 23        inc     hl
0195 46        ld      b,(hl)
0196 78        ld      a,b
0197 b1        or      c
0198 2816      jr      z,$01b0          ; (+$16)
019a 0b        dec     bc
019b 78        ld      a,b
019c b1        or      c
019d 200e      jr      nz,$01ad         ; (+$0e)
019f d5        push    de
01a0 23        inc     hl
01a1 23        inc     hl
01a2 e5        push    hl
01a3 23        inc     hl
01a4 cde201    call    $01e2			; KL EVENT
01a7 e1        pop     hl
01a8 46        ld      b,(hl)
01a9 2b        dec     hl
01aa 4e        ld      c,(hl)
01ab 2b        dec     hl
01ac d1        pop     de
01ad 70        ld      (hl),b
01ae 2b        dec     hl
01af 71        ld      (hl),c
01b0 eb        ex      de,hl
01b1 18d9      jr      $018c            ; (-$27)

;;==========================================================================
;; KL ADD TICKER
;; HL = event b lock
;; DE = initial value for counter
;; BC = reset count

01b3 e5        push    hl
01b4 23        inc     hl
01b5 23        inc     hl
01b6 f3        di      
01b7 73        ld      (hl),e			;; initial counter
01b8 23        inc     hl
01b9 72        ld      (hl),d
01ba 23        inc     hl
01bb 71        ld      (hl),c			;; reset count
01bc 23        inc     hl
01bd 70        ld      (hl),b
01be e1        pop     hl
01bf 11bdb8    ld      de,$b8bd			;; ticker list
01c2 c37903    jp      $0379 ;; add event to list

;;==========================================================================
;; KL DEL TICKER

01c5 11bdb8    ld      de,$b8bd
01c8 cd8803    call    $0388 ; remove event from list
01cb d0        ret     nc

01cc eb        ex      de,hl
01cd 23        inc     hl
01ce 5e        ld      e,(hl)
01cf 23        inc     hl
01d0 56        ld      d,(hl)
01d1 c9        ret     

;;==========================================================================
;; KL INIT EVENT

01d2 f3        di      
01d3 23        inc     hl
01d4 23        inc     hl
01d5 3600      ld      (hl),$00		;; tick count
01d7 23        inc     hl
01d8 70        ld      (hl),b		;; class
01d9 23        inc     hl
01da 73        ld      (hl),e		;; routine
01db 23        inc     hl
01dc 72        ld      (hl),d
01dd 23        inc     hl
01de 71        ld      (hl),c		;; rom
01df 23        inc     hl
01e0 fb        ei      
01e1 c9        ret     

;;==========================================================================
;; KL EVENT
;;
;; perform event
;; DE = address of next in chain
;; HL = address of current event

01e2 23        inc     hl
01e3 23        inc     hl
01e4 f3        di      
01e5 7e        ld      a,(hl)			;; count
01e6 34        inc     (hl)
01e7 fa0102    jp      m,$0201			;; update count 

01ea b7        or      a
01eb 2015      jr      nz,$0202         ; (+$15)

01ed 23        inc     hl
01ee 7e        ld      a,(hl)			; class
01ef 2b        dec     hl
01f0 b7        or      a
01f1 f22e02    jp      p,$022e			; -ve (bit = 1) = Asynchronous, +ve (bit = 0) = synchronous

;; Asynchronous
01f4 08        ex      af,af'
01f5 3011      jr      nc,$0208         
01f7 08        ex      af,af'

01f8 87        add     a,a				; express = -ve (bit = 1), normal = +ve (bit = 0)
01f9 f2e800    jp      p,$00e8			; add to normal list

;; Asynchronous Express
01fc 35        dec     (hl)				; indicate it needs processing
01fd 23        inc     hl
01fe 23        inc     hl
										; HL = routine address
01ff 1821      jr      $0222            ; execute event

;; update count 
0201 35        dec     (hl)

;; done processing

0202 08        ex      af,af'
0203 3801      jr      c,$0206          ; (+$01)
0205 fb        ei      
0206 08        ex      af,af'
0207 c9        ret     

0208 08        ex      af,af'

;;--------------------------
;; execute event func
0209 fb        ei      					; enable ints
020a 7e        ld      a,(hl)
020b 3d        dec     a
020c f8        ret     m

020d e5        push    hl
020e cd1b02    call    $021b				; part of KL DO SYNC
0211 e1        pop     hl
0212 35        dec     (hl)
0213 c8        ret     z

0214 f20d02    jp      p,$020d
0217 34        inc     (hl)
0218 c9        ret     

;;==========================================================================
;; KL DO SYNC

;; HL = event block
;; DE = address of event
0219 23        inc     hl
021a 23        inc     hl
021b 23        inc     hl

;; near or far address?
021c 7e        ld      a,(hl)
021d 23        inc     hl
021e 1f        rra     
021f d2c1b9    jp      nc,$b9c1			;	 LOW: KL FAR ICALL

;; event uses near address
;; execute it.
;; note that lower rom is enabled at this point so the function can't sit under the lower rom
0222 5e        ld      e,(hl)
0223 23        inc     hl
0224 56        ld      d,(hl)
0225 eb        ex      de,hl
0226 e9        jp      (hl)

;;==========================================================================
;; KL SYNC RESET

0227 210000    ld      hl,$0000
022a 22c1b8    ld      ($b8c1),hl
022d c9        ret     

;;---------------------------------------------------------------------------

;; Synchronous Event
022e e5        push    hl
022f 47        ld      b,a
0230 11c3b8    ld      de,$b8c3
0233 eb        ex      de,hl

0234 2b        dec     hl
0235 2b        dec     hl
0236 56        ld      d,(hl)
0237 2b        dec     hl
0238 5e        ld      e,(hl)
0239 7a        ld      a,d
023a b7        or      a
023b 2807      jr      z,$0244          ; (+$07)

023d 13        inc     de				; count
023e 13        inc     de				; class
023f 13        inc     de
0240 1a        ld      a,(de)
0241 b8        cp      b
0242 30ef      jr      nc,$0233         ; (-$11)

0244 d1        pop     de
0245 1b        dec     de
0246 23        inc     hl
0247 7e        ld      a,(hl)
0248 12        ld      (de),a
0249 1b        dec     de
024a 72        ld      (hl),d
024b 2b        dec     hl
024c 7e        ld      a,(hl)
024d 12        ld      (de),a
024e 73        ld      (hl),e
024f 08        ex      af,af'
0250 3801      jr      c,$0253          ; (+$01)

0252 fb        ei      
0253 08        ex      af,af'
0254 c9        ret     

;;==========================================================================
;; KL NEXT SYNC

0255 f3        di      
0256 2ac0b8    ld      hl,($b8c0)			; synchronous event list
0259 7c        ld      a,h
025a b7        or      a
025b 2817      jr      z,$0274          ; (+$17)
025d e5        push    hl
025e 5e        ld      e,(hl)
025f 23        inc     hl
0260 56        ld      d,(hl)
0261 23        inc     hl
0262 23        inc     hl
0263 3ac2b8    ld      a,($b8c2)
0266 be        cp      (hl)
0267 300a      jr      nc,$0273         ; (+$0a)
0269 f5        push    af
026a 7e        ld      a,(hl)
026b 32c2b8    ld      ($b8c2),a
026e ed53c0b8  ld      ($b8c0),de			; synchronous event list
0272 f1        pop     af
0273 e1        pop     hl
0274 fb        ei      
0275 c9        ret     

;;==========================================================================
;; KL DONE SYNC

0276 32c2b8    ld      ($b8c2),a
0279 23        inc     hl
027a 23        inc     hl
027b 35        dec     (hl)
027c c8        ret     z

027d f3        di      
027e f22e02    jp      p,$022e				;; Synchronous event
0281 34        inc     (hl)
0282 fb        ei      
0283 c9        ret     

;;==========================================================================
;; KL DEL SYNCHRONOUS

0284 cd8d02    call    $028d			; KL DISARM EVENT
0287 11c0b8    ld      de,$b8c0			; synchronouse event list
028a c38803    jp      $0388 ; remove event from list

;;==========================================================================
;; KL DISARM EVENT

028d 23        inc     hl
028e 23        inc     hl
028f 36c0      ld      (hl),$c0
0291 2b        dec     hl
0292 2b        dec     hl
0293 c9        ret     

;;==========================================================================
;; KL EVENT DISABLE

0294 21c2b8    ld      hl,$b8c2
0297 cbee      set     5,(hl)
0299 c9        ret     

;;==========================================================================
;; KL EVENT ENABLE

029a 21c2b8    ld      hl,$b8c2
029d cbae      res     5,(hl)
029f c9        ret     

;;==========================================================================
;; KL LOG EXT
;;
;; BC contains the address of the RSX's command table
;; HL contains the address of four bytes exclusively for use by the firmware 
;; 
;; NOTES: Most recent command is added to the start of the list. The next oldest
;; is next and so on until we get to the command that was registered first and the 
;; end of the list.
;; 
;; HL can't be in the range &0000-&3fff because the OS rom will be active in this range. 
;; Sensible range is &4000-&c000. (&c000-&ffff is normally where upper ROM is located, so it
;; is unwise to locate it here if you want to access the command from BASIC because BASIC
;; will be active in this range)
02a0 e5        push    hl
02a1 ed5bd3b8  ld      de,($b8d3)   ;; get head of the list
02a5 22d3b8    ld      ($b8d3),hl   ;; set new head of the list
02a8 73        ld      (hl),e       ;; previous | command registered with KL LOG EXT or 0 if end of list
02a9 23        inc     hl
02aa 72        ld      (hl),d
02ab 23        inc     hl
02ac 71        ld      (hl),c       ;; address of RSX's command table
02ad 23        inc     hl
02ae 70        ld      (hl),b
02af e1        pop     hl
02b0 c9        ret     

;;==========================================================================
;; KL FIND COMMAND
;; HL = address of command name to be found.

;; NOTES: 
;; - last char must have bit 7 set to indicate the end of the string.
;; - up to 16 characters is compared. Name can be any length but first 16 characters must be unique.

02b1 11c3b8    ld      de,$b8c3 ;; destination
02b4 011000    ld      bc,$0010 ;; length
02b7 cda1ba    call    $baa1			;; HI: KL LDIR (disable upper and lower roms and perform LDIR)

;; ensure last character has bit 7 set (indicates end of string, where length of name is longer
;; than 16 characters). If name is less than 16 characters the last char will have bit 7 set anyway.
02ba eb        ex      de,hl
02bb 2b        dec     hl
02bc cbfe      set     7,(hl)

02be 2ad3b8    ld      hl,($b8d3)   ; points to commands registered with KL LOG EXT
02c1 7d        ld      a,l          ; preload lower byte of address into A for comparison
02c2 1810      jr      $02d4           

;; search for more | commands registered with KL LOG EXT
02c4 e5        push    hl
02c5 23        inc     hl         ; skip pointer to next registered RSX
02c6 23        inc     hl
02c7 4e        ld      c,(hl)     ; fetch address of RSX table
02c8 23        inc     hl
02c9 46        ld      b,(hl)
02ca cdf102    call    $02f1      ; search for command
02cd d1        pop     de
02ce d8        ret     c

02cf eb        ex      de,hl
02d0 7e        ld      a,(hl)     ; get address of next registered RSX
02d1 23        inc     hl
02d2 66        ld      h,(hl)
02d3 6f        ld      l,a

02d4 b4        or      h              ; if HL is zero, then this is the end of the list.
02d5 20ed      jr      nz,$02c4       ; loop if we didn't get to the end of the list


02d7 0eff      ld      c,$ff
02d9 0c        inc     c
;; C = ROM select address of ROM to probe
02da cd7eba    call    $ba7e			;; HI: KL PROBE ROM
;; A = ROM's class.
;; 0 = Foreground
;; 1 = Background
;; 2 = Extension foreground ROM
02dd f5        push    af
02de e603      and     $03
02e0 47        ld      b,a
02e1 ccf102    call    z,$02f1    ; search for command

02e4 dc1c06    call    c,$061c			; MC START PROGRAM
02e7 f1        pop     af
02e8 87        add     a,a
02e9 30ee      jr      nc,$02d9         ; (-$12)
02eb 79        ld      a,c
02ec fe10      cp      $10          ; maximum rom selection scanned by firmware
02ee 38e9      jr      c,$02d9          ; (-$17)
02f0 c9        ret     

;;-------------------------------------------------------------------------------------------------------------
;; perform search of RSX in command-table.
;; EIther RSX in RAM or RSX in ROM.

;; HL = address of command-table in ROM
02f1 2104c0    ld      hl,$c004

;;B=0 for RSX in ROM, B!=0 for RSX in RAM
;; This also means that ROM class must be foreground.
02f4 78        ld      a,b
02f5 b7        or      a
02f6 2804      jr      z,$02fc          

;; HL = address of RSX table
02f8 60        ld      h,b
02f9 69        ld      l,c
;; "ROM select" for RAM 
02fa 0eff      ld      c,$ff

;; C = ROM select address
02fc cd79ba    call    $ba79			;; HI: KL ROM SELECT
;; C contains the ROM select address of the previously selected ROM.
;; B contains the previous ROM state
;; preserve previous rom selection and rom state
02ff c5        push    bc

;; get address of strings from table.
0300 5e        ld      e,(hl)
0301 23        inc     hl
0302 56        ld      d,(hl)
0303 23        inc     hl
;; DE = jumpblock for RSX commands
0304 eb        ex      de,hl
0305 1817      jr      $031e            ; (+$17)

;; B8C3 = RSX command to look for stored in RAM
0307 01c3b8    ld      bc,$b8c3
030a 0a        ld      a,(bc)
030b be        cp      (hl)
030c 2008      jr      nz,$0316         ; (+$08)
030e 23        inc     hl
030f 03        inc     bc
0310 87        add     a,a
0311 30f7      jr      nc,$030a         ; (-$09)
;; if we get to here, then we found the name
0313 eb        ex      de,hl
0314 180c      jr      $0322            ; (+$0c)

;; char didn't match in name
;; look for end of string, it has bit 7 set
0316 7e        ld      a,(hl)
0317 23        inc     hl
;; transfer bit 7 into carry flag
0318 87        add     a,a
0319 30fb      jr      nc,$0316         ; (-$05)

;; update jumpblock pointer
031b 13        inc     de
031c 13        inc     de
031d 13        inc     de

;; 0 indicates end of list.
031e 7e        ld      a,(hl)
031f b7        or      a
0320 20e5      jr      nz,$0307         ; (-$1b)

;; we got to the end of the RSX command-table and we didn't find the command

0322 c1        pop     bc
;; restore previous rom selection
0323 c387ba    jp      $ba87			;; HI: KL ROM DESELECT

;;==========================================================================
;; KL ROM WALK

0326 0e0f      ld      c,$0f      ;; maximum number of roms that firmware supports -1
0328 cd3003    call    $0330			; KL INIT BACK
032b 0d        dec     c
032c f22803    jp      p,$0328
032f c9        ret     

;;==========================================================================
;; KL INIT BACK

0330 3ad9b8    ld      a,($b8d9)
0333 b9        cp      c
0334 c8        ret     z

0335 79        ld      a,c
0336 fe10      cp      $10        ;; maximum rom selection supported by firmware
0338 d0        ret     nc

0339 cd79ba    call    $ba79		;; HI: KL ROM SELECT
033c 3a00c0    ld      a,($c000)
033f e603      and     $03
0341 3d        dec     a
0342 2022      jr      nz,$0366         ; (+$22)
0344 c5        push    bc
0345 37        scf     
0346 cd06c0    call    $c006
0349 301a      jr      nc,$0365         ; (+$1a)
034b d5        push    de
034c 23        inc     hl
034d eb        ex      de,hl
034e 21dab8    ld      hl,$b8da
0351 ed4bd6b8  ld      bc,($b8d6)
0355 0600      ld      b,$00
0357 09        add     hl,bc
0358 09        add     hl,bc
0359 73        ld      (hl),e
035a 23        inc     hl
035b 72        ld      (hl),d
035c 21fcff    ld      hl,$fffc
035f 19        add     hl,de
0360 cda002    call    $02a0			; KL LOG EXT
0363 2b        dec     hl
0364 d1        pop     de
0365 c1        pop     bc
0366 c387ba    jp      $ba87			;; HI: KL ROM DESELECT

;;====================================================================
;; DE = address of event block
;; HL = address of event list

;; find event in list
0369 7e        ld      a,(hl)
036a bb        cp      e
036b 23        inc     hl
036c 7e        ld      a,(hl)
036d 2b        dec     hl
036e 2003      jr      nz,$0373         ; (+$03)
0370 ba        cp      d
0371 37        scf     
0372 c8        ret     z

0373 b7        or      a
0374 c8        ret     z

0375 6e        ld      l,(hl)
0376 67        ld      h,a
0377 18f0      jr      $0369 ;; find event in list            ; (-$10)

;;====================================================================
;; add event to an event list
;; HL = address of event block
;; DE = address of event list
0379 eb        ex      de,hl
037a f3        di      
037b cd6903    call    $0369 ;; find event in list
037e 3806      jr      c,$0386          ; event found
;; add to head of list
0380 73        ld      (hl),e
0381 23        inc     hl
0382 72        ld      (hl),d
0383 13        inc     de
0384 af        xor     a
0385 12        ld      (de),a

0386 fb        ei      
0387 c9        ret     

;;====================================================================
;; delete event from list
;; HL = address of event block
;; DE = address of event list
0388 eb        ex      de,hl
0389 f3        di      
038a cd6903    call    $0369 ;; find event in list
038d 3006      jr      nc,$0395         ; (+$06)
038f 1a        ld      a,(de)
0390 77        ld      (hl),a
0391 13        inc     de
0392 23        inc     hl
0393 1a        ld      a,(de)
0394 77        ld      (hl),a
0395 fb        ei      
0396 c9        ret     

;;====================================================================
;; KL BANK SWITCH 
;;
;; A = new configuration (0-31)
;;
;; Allows any configuration to be used, so compatible with ALL Dk'Tronics RAM sizes.

0397 f3        di      
0398 d9        exx     
0399 21d5b8    ld      hl,$b8d5   ; current bank selection
039c 56        ld      d,(hl)     ; get previous
039d 77        ld      (hl),a     ; set new
039e f6c0      or      $c0        ; bit 7 = 1, bit 6 = 1, selection in lower bits.
03a0 ed79      out     (c),a
03a2 7a        ld      a,d        ; previous bank selection
03a3 d9        exx     
03a4 fb        ei      
03a5 c9        ret     

;;--------------------------------------------------------------------
;; HIGH KERNEL JUMPBLOCK
03a6 c35fba    jp      $ba5f		;; HI: KL U ROM ENABLE
03a9 c366ba    jp      $ba66		;; HI: KL U ROM DISABLE
03ac c351ba    jp      $ba51		;; HI: KL L ROM ENABLE
03af c358ba    jp      $ba58		;; HI: KL L ROM DISABLE
03b2 c370ba    jp      $ba70		;; HI: KL L ROM RESTORE
03b5 c379ba    jp      $ba79		;; HI: KL ROM SELECT
03b8 c39dba    jp      $ba9d		;; HI: KL CURR SELECTION
03bb c37eba    jp      $ba7e		;; HI: KL PROBE ROM
03be c387ba    jp      $ba87		;; HI: KL ROM DESELECT
03c1 c3a1ba    jp      $baa1		;; HI: KL LDIR
03c4 c3a7ba    jp      $baa7		;; HI: KL LDDR

;;--------------------------------------------------------------------
03c7 3ac1b8    ld      a,($b8c1)	;; HI: KL POLL SYNCRONOUS
03ca b7        or      a
03cb c8        ret     z

03cc e5        push    hl
03cd f3        di      
03ce 1806      jr      $03d6            ; (+$06)

03d0 21bfb8    ld      hl,$b8bf		;; HI: KL SCAN NEEDED
03d3 3601      ld      (hl),$01
03d5 c9        ret     

03d6 2ac0b8    ld      hl,($b8c0)		; synchronouse event list
03d9 7c        ld      a,h
03da b7        or      a
03db 2807      jr      z,$03e4          ; (+$07)
03dd 23        inc     hl
03de 23        inc     hl
03df 23        inc     hl
03e0 3ac2b8    ld      a,($b8c2)
03e3 be        cp      (hl)
03e4 e1        pop     hl
03e5 fb        ei      
03e6 c9        ret     

;;============================================================================================
; RST 7 - LOW: INTERRUPT ENTRY handler

03e7 f3        di      
03e8 08        ex      af,af'
03e9 3833      jr      c,$041e          ; detect external interrupt
03eb d9        exx     
03ec 79        ld      a,c
03ed 37        scf     
03ee fb        ei      
03ef 08        ex      af,af'			; allow interrupt function to be re-entered. This will happen if there is an external interrupt
										; source that continues to assert INT. Internal raster interrupts are acknowledged automatically and cleared.
03f0 f3        di      
03f1 f5        push    af
03f2 cb91      res     2,c				; ensure lower rom is active in range &0000-&3fff
03f4 ed49      out     (c),c
03f6 cdb100    call    $00b1			; update time, execute FRAME FLY, FAST TICKER and SOUND events
										; also scan keyboard
03f9 b7        or      a
03fa 08        ex      af,af'
03fb 4f        ld      c,a
03fc 067f      ld      b,$7f

03fe 3a31b8    ld      a,($b831)
0401 b7        or      a
0402 2814      jr      z,$0418          ; quit...
0404 fa72b9    jp      m,$b972			; quit... (same as 0418, but in RAM)

0407 79        ld      a,c
0408 e60c      and     $0c				; %00001100
040a f5        push    af
040b cb91      res     2,c				; ensure lower rom is active in range &0000-&3fff
040d d9        exx     
040e cd0a01    call    $010a
0411 d9        exx     
0412 e1        pop     hl
0413 79        ld      a,c
0414 e6f3      and     $f3				; %11110011
0416 b4        or      h
0417 4f        ld      c,a

;;
0418 ed49      out     (c),c			set rom config/mode etc
041a d9        exx     
041b f1        pop     af
041c fb        ei      
041d c9        ret     

;; handle external interrupt
041e 08        ex      af,af'
041f e1        pop     hl
0420 f5        push    af
0421 cbd1      set     2,c				; disable lower rom 
0423 ed49      out     (c),c			; set rom config/mode etc
0425 cd3b00    call    $003b			; LOW: EXT INTERRUPT. Patchable by the user
0428 18cf      jr      $03f9            ; return to interrupt processing.

;;============================================================================================
; LOW: KL LOW PCHL
042a f3        di      
042b e5        push    hl				; store HL onto stack
042c d9        exx     
042d d1        pop     de				; get it back from stack
042e 1806      jr      $0436            ; 

;;============================================================================================
; RST 1 - LOW: LOW JUMP

0430 f3        di      
0431 d9        exx     
0432 e1        pop     hl				; get return address from stack
0433 5e        ld      e,(hl)			; DE = address to call
0434 23        inc     hl
0435 56        ld      d,(hl)

;;--------------------------------------------------------------------------------------------
0436 08        ex      af,af'
0437 7a        ld      a,d
0438 cbba      res     7,d
043a cbb2      res     6,d
043c 07        rlca    
043d 07        rlca    

;;---------------------------------------------------------------------------------------------
043e 07        rlca    
043f 07        rlca    
0440 a9        xor     c
0441 e60c      and     $0c
0443 a9        xor     c
0444 c5        push    bc
0445 cdb0b9    call    $b9b0
0448 f3        di      
0449 d9        exx     
044a 08        ex      af,af'
044b 79        ld      a,c
044c c1        pop     bc
044d e603      and     $03
044f cb89      res     1,c
0451 cb81      res     0,c
0453 b1        or      c
0454 1801      jr      $0457            ; (+$01)

;;============================================================================================
;; copied to $b9b0 in RAM

0456 d5        push    de
0457 4f        ld      c,a
0458 ed49      out     (c),c
045a b7        or      a
045b 08        ex      af,af'
045c d9        exx     
045d fb        ei      
045e c9        ret     

;;============================================================================================
; LOW: KL FAR PCHL
045f f3        di      
0460 08        ex      af,af'
0461 79        ld      a,c
0462 e5        push    hl
0463 d9        exx     
0464 d1        pop     de
0465 1815      jr      $047c            ; (+$15)

;;============================================================================================
; LOW: KL FAR ICALL
0467 f3        di      
0468 e5        push    hl
0469 d9        exx     
046a e1        pop     hl
046b 1809      jr      $0476            ; (+$09)

;;============================================================================================
;; RST 3 - LOW: FAR CALL
;;
;; far call limits rom select to 251. So firmware can call functions in ROMs up to 251.
;; If you want to access ROMs above this use KL ROM SELECT.
;;
046d f3        di      
046e d9        exx     
046f e1        pop     hl
0470 5e        ld      e,(hl)
0471 23        inc     hl
0472 56        ld      d,(hl)
0473 23        inc     hl
0474 e5        push    hl
0475 eb        ex      de,hl
0476 5e        ld      e,(hl)
0477 23        inc     hl
0478 56        ld      d,(hl)
0479 23        inc     hl
047a 08        ex      af,af'
047b 7e        ld      a,(hl)
;; $fc - no change to rom select, enable upper and lower roms
;; $fd - no change to rom select, enable upper disable lower
;; $fe - no change to rom select, disable upper and enable lower
;; $ff - no change to rom select, disable upper and lower roms
047c fefc      cp      $fc
047e 30be      jr      nc,$043e 

;; allow rom select to change
0480 06df      ld      b,$df			; ROM select I/O port
0482 ed79      out     (c),a			; select upper rom

0484 21d6b8    ld      hl,$b8d6
0487 46        ld      b,(hl)
0488 77        ld      (hl),a
0489 c5        push    bc
048a fde5      push    iy

;; rom select below 16 (max for firmware 1.1)?
048c fe10      cp      $10         
048e 300f      jr      nc,$049f   

;; 16-bit table at &b8da
0490 87        add     a,a
0491 c6da      add     a,$da
0493 6f        ld      l,a
0494 ceb8      adc     a,$b8
0496 95        sub     l
0497 67        ld      h,a

;; get 16-bit value from this address
0498 7e        ld      a,(hl)
0499 23        inc     hl
049a 66        ld      h,(hl)
049b 6f        ld      l,a
049c e5        push    hl
049d fde1      pop     iy

049f 067f      ld      b,$7f
04a1 79        ld      a,c
04a2 cbd7      set     2,a          
04a4 cb9f      res     3,a          
04a6 cdb0b9    call    $b9b0
04a9 fde1      pop     iy
04ab f3        di      
04ac d9        exx     
04ad 08        ex      af,af'
04ae 59        ld      e,c
04af c1        pop     bc
04b0 78        ld      a,b
;; restore rom select
04b1 06df      ld      b,$df			; ROM select I/O port
04b3 ed79      out     (c),a			; restore upper rom selection

04b5 32d6b8    ld      ($b8d6),a
04b8 067f      ld      b,$7f
04ba 7b        ld      a,e
04bb 1890      jr      $044d            ; (-$70)

;;============================================================================================
; LOW: KL SIDE PCHL
04bd f3        di      
04be e5        push    hl
04bf d9        exx     
04c0 d1        pop     de
04c1 1808      jr      $04cb            ; (+$08)

;;============================================================================================
;; RST 2 - LOW: SIDE CALL

04c3 f3        di      
04c4 d9        exx     
04c5 e1        pop     hl
04c6 5e        ld      e,(hl)
04c7 23        inc     hl
04c8 56        ld      d,(hl)
04c9 23        inc     hl
04ca e5        push    hl
04cb 08        ex      af,af'
04cc 7a        ld      a,d
04cd cbfa      set     7,d
04cf cbf2      set     6,d
04d1 e6c0      and     $c0
04d3 07        rlca    
04d4 07        rlca    
04d5 21d9b8    ld      hl,$b8d9
04d8 86        add     a,(hl)
04d9 18a5      jr      $0480            ; (-$5b)

;;============================================================================================
; RST 5 - LOW: FIRM JUMP
04db f3        di      
04dc d9        exx     
04dd e1        pop     hl
04de 5e        ld      e,(hl)
04df 23        inc     hl
04e0 56        ld      d,(hl)
04e1 cb91      res     2,c				; enable lower rom
04e3 ed49      out     (c),c
04e5 ed5346ba  ld      ($ba46),de
04e9 d9        exx     
04ea fb        ei      
04eb cd45ba    call    $ba45
04ee f3        di      
04ef d9        exx     
04f0 cbd1      set     2,c				; disable lower rom
04f2 ed49      out     (c),c
04f4 d9        exx     
04f5 fb        ei      
04f6 c9        ret     

;;============================================================================================
;; HI: KL L ROM ENABLE
04f7 f3        di      
04f8 d9        exx     
04f9 79        ld      a,c        ; current mode/rom state
04fa cb91      res     2,c				; enable lower rom
04fc 1813      jr      $0511            ; enable/disable rom common code

;;============================================================================================
;; HI: KL L ROM DISABLE
04fe f3        di      
04ff d9        exx     
0500 79        ld      a,c        ; current mode/rom state
0501 cbd1      set     2,c				; disable upper rom
0503 180c      jr      $0511            ; enable/disable rom common code

;;============================================================================================
;; HI: KL U ROM ENABLE
0505 f3        di      
0506 d9        exx     
0507 79        ld      a,c        ; current mode/rom state
0508 cb99      res     3,c				; enable upper rom
050a 1805      jr      $0511            ; enable/disable rom common code

;;============================================================================================
;; HI: KL U ROM DISABLE
050c f3        di      
050d d9        exx     
050e 79        ld      a,c        ; current mode/rom state
050f cbd9      set     3,c				; disable upper rom

;;--------------------------------------------------------------------------------------------
;; enable/disable rom common code
0511 ed49      out     (c),c
0513 d9        exx     
0514 fb        ei      
0515 c9        ret     

;;============================================================================================
;; HI: KL L ROM RESTORE
0516 f3        di      
0517 d9        exx     
0518 a9        xor     c
0519 e60c      and     $0c				; %1100
051b a9        xor     c
051c 4f        ld      c,a
051d 18f2      jr      $0511             ; enable/disable rom common code

;;============================================================================================
;; HI: KL ROM SELECT
;; Any value can be used from 0-255.

051f cd5fba    call    $ba5f			;; HI: KL U ROM ENABLE
0522 180f      jr      $0533      ;; common upper rom selection code      

;;============================================================================================
;; HI: KL PROBE ROM
0524 cd79ba    call    $ba79			;; HI: KL ROM SELECT

;; read rom version etc
0527 3a00c0    ld      a,($c000)			
052a 2a01c0    ld      hl,($c001)
;; drop through to HI: KL ROM DESELECT
;;============================================================================================
;; HI: KL ROM DESELECT
052d f5        push    af
052e 78        ld      a,b
052f cd70ba    call    $ba70			;; HI: KL L ROM RESTORE
0532 f1        pop     af

;;--------------------------------------------------------------------------------------------
;; common upper rom selection code
0533 e5        push    hl
0534 f3        di      
0535 06df      ld      b,$df			;; ROM select I/O port
0537 ed49      out     (c),c			;; select upper rom
0539 21d6b8    ld      hl,$b8d6   ;; previous upper rom selection
053c 46        ld      b,(hl)     ;; get previous upper rom selection
053d 71        ld      (hl),c     ;; store new rom selection
053e 48        ld      c,b        ;; C = previous rom select
053f 47        ld      b,a        ;; B = previous rom state
0540 fb        ei      
0541 e1        pop     hl
0542 c9        ret     

;;============================================================================================
;; HI: KL CURR SELECTION
0543 3ad6b8    ld      a,($b8d6)
0546 c9        ret     

;;============================================================================================
;; HI: KL LDIR
0547 cdadba    call    $baad			;; disable upper/lower rom.. execute code below and then restore rom state

;; called via $baad
054a edb0      ldir    
;; returns back to code after call in $baad   
054c c9        ret     

;;============================================================================================
;; HI: KL LDDR
054d cdadba    call    $baad			;; disable upper/lower rom.. execute code below and then restore rom state

;; called via $baad
0550 edb8      lddr 
;; returns back to code after call in $baad   
0552 c9        ret     
;;============================================================================================
;; used by HI: KL LDIR and HI: KL LDDR
;; copied to $baad in RAM
;;
;; - disables upper and lower rom
;; - continues execution from function that called it allowing it to return back
;; - restores upper and lower rom state

0553 f3        di      
0554 d9        exx     
0555 e1        pop     hl					; return address
0556 c5        push    bc					; store rom state
0557 cbd1      set     2,c					; disable lower rom
0559 cbd9      set     3,c					; disable upper rom
055b ed49      out     (c),c				; set rom state

;; jump to function on the stack, allow it to return back here
055d cdc2ba    call    $bac2				; jump to function in HL


0560 f3        di      
0561 d9        exx     
0562 c1        pop     bc					; get previous rom state
0563 ed49      out     (c),c				; restore previous rom state
0565 d9        exx     
0566 fb        ei      
0567 c9        ret     

;;============================================================================================
;; copied to $bac2 into RAM
0568 e5        push    hl
0569 d9        exx     
056a fb        ei      
056b c9        ret     

;;============================================================================================
; RST 4 - LOW: RAM LAM
;; HL = address to read
056c f3        di      
056d d9        exx     
056e 59        ld      e,c				;; E = current rom configuration
056f cbd3      set     2,e				;; disable lower rom
0571 cbdb      set     3,e				;; disable upper rom
0573 ed59      out     (c),e			;; set rom configuration
0575 d9        exx     
0576 7e        ld      a,(hl)			;; read byte from RAM
0577 d9        exx     
0578 ed49      out     (c),c			;; restore rom configuration
057a d9        exx     
057b fb        ei      
057c c9        ret     

;;============================================================================================
;; read byte from address pointed to IX with roms disabled
;;
;; (used by cassette functions to read/write to RAM)
;;
;; IX = address of byte to read
;; C' = current rom selection and mode

057d d9        exx						;; switch to alternative register set

057e 79        ld      a,c				;; get rom configuration
057f f60c      or      $0c				;; %00001100 (disable upper and lower rom)
0581 ed79      out     (c),a			;; set the new rom configuration

0583 dd7e00    ld      a,(ix+$00)		;; read byte from RAM

0586 ed49      out     (c),c			;; restore original rom configuration
0588 d9        exx						;; switch back from alternative register set
0589 c9        ret     

;;============================================================================================

058a 26c7      ld      h,$c7
058c c7        rst     $00
058d c7        rst     $00
058e c7        rst     $00
058f c7        rst     $00
0590 c7        rst     $00

;;============================================================================================
0591 f3        di      
0592 0182f7    ld      bc,$f782
0595 ed49      out     (c),c

0597 0100f4    ld      bc,$f400			;; initialise PPI port A data
059a ed49      out     (c),c

059c 0100f6    ld      bc,$f600			;; initialise PPI port C data 
										;; - select keyboard line 0
										;; - PSG control inactive
										;; - cassette motor off
										;; - cassette write data "0"
059f ed49      out     (c),c			;; set PPI port C data

05a1 017fef    ld      bc,$ef7f
05a4 ed49      out     (c),c

05a6 06f5      ld      b,$f5			;; PPI port B inputs
05a8 ed78      in      a,(c)
05aa e610      and     $10				
05ac 21d505    ld      hl,$05d5			;; end of CRTC data for 50Hz display
05af 2003      jr      nz,$05b4         
05b1 21e505    ld      hl,$05e5			;; end of CRTC data for 60Hz display

;; initialise display
;; starting with register 15, then down to 0
05b4 010fbc    ld      bc,$bc0f
05b7 ed49      out     (c),c			; select CRTC register
05b9 2b        dec     hl
05ba 7e        ld      a,(hl)			; get data from table 
05bb 04        inc     b
05bc ed79      out     (c),a			; write data to selected CRTC register
05be 05        dec     b
05bf 0d        dec     c
05c0 f2b705    jp      p,$05b7

;; continue with setup...
05c3 1820      jr      $05e5            ; (+$20)

;; CRTC data for 50Hz display
05c5 
defb &3f, &28, &2e, &8e, &26, &00, &19, &1e, &00, &07, &00,&00,&30,&00,&c0,&00
;; CRTC data for 60Hz display
defb &3f, &28, &2e, &8e, &1f, &06, &19, &1b, &00, &07, &00,&00,&30,&00,&c0,&00

;;========================================================
;; continue with setup...

05e5 117706    ld      de,$0677			; this is executed by execution address
05e8 210000    ld      hl,$0000			; this will force MC START PROGRAM to start BASIC
05eb 1832      jr      $061f            ; mc start program

;;========================================================
;; MC BOOT PROGRAM
;; 
;; HL = execute address

05ed 3100c0    ld      sp,$c000
05f0 e5        push    hl
05f1 cde91f    call    $1fe9			;; SOUND RESET
05f4 f3        di      

05f5 01fff8    ld      bc,$f8ff			;; reset all peripherals
05f8 ed49      out     (c),c

05fa cd5c00    call    $005c			;; KL CHOKE OFF
05fd e1        pop     hl
05fe d5        push    de
05ff c5        push    bc
0600 e5        push    hl
0601 cd981b    call    $1b98			;; KM RESET
0604 cd8410    call    $1084			;; TXT RESET
0607 cdd00a    call    $0ad0			;; SCR RESET
060a cd5fba    call    $ba5f			;; HI: KL U ROM ENABLE
060d e1        pop     hl
060e cd1e00    call    $001e			;; LOW: PCHL INSTRUCTION
0611 c1        pop     bc
0612 d1        pop     de
0613 3807      jr      c,$061c          ; MC START PROGRAM


;; display program load failed message
0615 eb        ex      de,hl
0616 48        ld      c,b
0617 11f906    ld      de,$06f9			; program load failed
061a 1803      jr      $061f            ; 

;;=========================================================
;; MC START PROGRAM
;; HL = entry address
;; C = rom select 

061c 113707    ld      de,$0737			; RET (no message)
										; this is executed by: LOW: PCHL INSTRUCTION

;;---------------------------------------------------------

061f f3        di						; disable interrupts
0620 ed56      im      1				; Z80 interrupt mode 1
0622 d9        exx     

0623 0100df    ld      bc,$df00			; select upper ROM 0
0626 ed49      out     (c),c

0628 01fff8    ld      bc,$f8ff			; reset all peripherals
062b ed49      out     (c),c

062d 01c07f    ld      bc,$7fc0			; select ram configuration 0
0630 ed49      out     (c),c

0632 017efa    ld      bc,$fa7e			; stop disc motor
0635 af        xor     a
0636 ed79      out     (c),a

0638 2100b1    ld      hl,$b100			; clear memory block which will hold 
063b 1101b1    ld      de,$b101			; firmware jumpblock
063e 01f907    ld      bc,$07f9
0641 77        ld      (hl),a
0642 edb0      ldir    

0644 01897f    ld      bc,$7f89			; select mode 1, lower rom on, upper rom off
0647 ed49      out     (c),c

0649 d9        exx     
064a af        xor     a
064b 08        ex      af,af'
064c 3100c0    ld      sp,$c000				;; initial stack location
064f e5        push    hl
0650 c5        push    bc
0651 d5        push    de

0652 cd4400    call    $0044				;; initialise LOW KERNEL and HIGH KERNEL jumpblocks
0655 cdbd08    call    $08bd				;; JUMP RESTORE
0658 cd5c1b    call    $1b5c				;; KM INITIALISE
065b cde91f    call    $1fe9				;; SOUND RESET
065e cdbf0a    call    $0abf				;; SCR INITIALISE
0661 cd7410    call    $1074				;; TXT INITIALISE
0664 cda815    call    $15a8				;; GRA INITIALISE
0667 cdbc24    call    $24bc				;; CAS INITIALISE
066a cde007    call    $07e0				;; MC RESET PRINTER
066d fb        ei      
066e e1        pop     hl
066f cd1e00    call    $001e				;; LOW: PCHL INSTRUCTION
0672 c1        pop     bc
0673 e1        pop     hl
0674 c37700    jp      $0077				;; start BASIC or program

;;======================================================================

0677 210202    ld      hl,$0202
067a cd7011    call    $1170			; TXT SET CURSOR

067d cd2307    call    $0723			; get pointer to machine name (based on LK1-LK3 on PCB)

0680 cdfc06    call    $06fc			; display message

0683 218806    ld      hl,$0688			; "128K Microcomputer.." message
0686 1874      jr      $06fc            ; 

0688 
defb " 128K Microcomputer  (v3)"
defb &1f,&02,&04
defb "Copyright"
defb &1f,&02,&04
defb &a4								;; copyright symbol
defb "1985 Amstrad Consumer Electronics plc"
defb &1f,&0c,&05
defb "and Locomotive Software Ltd."
defb &1f,&01,&07
defb 0

;;-----------------------------------------------------------------------
06f9 210507    ld      hl,$0705			; "*** PROGRAM LOAD FAILED ***" message

;;-----------------------------------------------------------------------
;; display a null terminated string
06fc 7e        ld      a,(hl)			; get message character
06fd 23        inc     hl
06fe b7        or      a
06ff c8        ret     z

0700 cdfe13    call    $13fe			; TXT OUTPUT
0703 18f7      jr      $06fc            

0705
defb "*** PROGRAM LOAD FAILED ***",13,10,0
;;-----------------------------------------------------------------------
;; get a pointer to the machine name
;; HL = machine name
0723 06f5      ld      b,$f5			;; PPI port B input
0725 ed78      in      a,(c)
0727 2f        cpl     
0728 e60e      and     $0e				;; isolate LK1-LK3 (defines machine name on startup)
072a 0f        rrca    
;; A = machine name number
072b 213807    ld      hl,$0738			; table of names
072e 3c        inc     a
072f 47        ld      b,a

;; B = index of string wanted

;; keep getting bytes until end of string marker (0) is found
;; decrement string count and continue until we have got string
;; wanted
0730 7e        ld      a,(hl)			; get byte
0731 23        inc     hl
0732 b7        or      a				; end of string?
0733 20fb      jr      nz,$0730         ; 

0735 10f9      djnz    $0730            ; (-$07)
0737 c9        ret     

;; start-up names
0738 
defb "Arnold",0							;; this name can't be chosen
defb "Amstrad",0
defb "Orion",0
defb "Schneider",0
defb "Awa",0
defb "Solavox",0
defb "Saisho",0
defb "Triumph",0
defb "Isp",0


;;====================================================================
;; MC SET MODE
;; 
;; A = mode index
;;
;; C' = Gate Array rom and mode configuration register

;; test mode index is in range
0776 fe03      cp      $03
0778 d0        ret     nc

;; mode index is in range: A = 0,1 or 2.

0779 f3        di      
077a d9        exx     
077b cb89      res     1,c				;; clear mode bits (bit 1 and bit 0)
077d cb81      res     0,c

077f b1        or      c				;; set mode bits to new mode value
0780 4f        ld      c,a
0781 ed49      out     (c),c			;; set mode
0783 fb        ei      
0784 d9        exx     
0785 c9        ret     

;;====================================================================
;; MC CLEAR INKS

0786 e5        push    hl
0787 210000    ld      hl,$0000
078a 1804      jr      $0790            

;;====================================================================
;; MC SET INKS

078c e5        push    hl
078d 210100    ld      hl,$0001

;;--------------------------------------------------------------------
;; HL = 0 for clear, 1 for set
0790 d5        push    de
0791 c5        push    bc
0792 eb        ex      de,hl

0793 01107f    ld      bc,$7f10			; set border colour
0796 cdaa07    call    $07aa			; set colour for PEN/border direct to hardware
0799 23        inc     hl
079a 0e00      ld      c,$00

079c cdaa07    call    $07aa			; set colour for PEN/border direct to hardware
079f 19        add     hl,de
07a0 0c        inc     c
07a1 79        ld      a,c
07a2 fe10      cp      $10        ; maximum number of colours (mode 0 has 16 colours)
07a4 20f6      jr      nz,$079c         ; (-$0a)

07a6 c1        pop     bc
07a7 d1        pop     de
07a8 e1        pop     hl
07a9 c9        ret     

;;====================================================================
;; set colour for a pen
;;
;; HL = address of colour for pen
;; C = pen index

07aa ed49      out     (c),c			; select pen 
07ac 7e        ld      a,(hl)
07ad e61f      and     $1f
07af f640      or      $40
07b1 ed79      out     (c),a			; set colour for pen
07b3 c9        ret     


;;====================================================================
;; MC WAIT FLYBACK

07b4 f5        push    af
07b5 c5        push    bc

07b6 06f5      ld      b,$f5			; PPI port B I/O address
07b8 ed78      in      a,(c)			; read PPI port B input
07ba 1f        rra						; transfer bit 0 (VSYNC signal from CRTC) into carry flag
07bb 30fb      jr      nc,$07b8			; wait until VSYNC=1

07bd c1        pop     bc
07be f1        pop     af
07bf c9        ret     

;;====================================================================
;; MC SCREEN OFFSET
;;
;; HL = offset
;; A = base

07c0 c5        push    bc
07c1 0f        rrca    
07c2 0f        rrca    
07c3 e630      and     $30
07c5 4f        ld      c,a
07c6 7c        ld      a,h
07c7 1f        rra     
07c8 e603      and     $03
07ca b1        or      c

;; CRTC register 12 and 13 define screen base and offset

07cb 010cbc    ld      bc,$bc0c			
07ce ed49      out     (c),c			; select CRTC register 12
07d0 04        inc     b				; BC = bd0c
07d1 ed79      out     (c),a			; set CRTC register 12 data
07d3 05        dec     b				; BC = bc0c
07d4 0c        inc     c				; BC = bc0d
07d5 ed49      out     (c),c			; select CRTC register 13
07d7 04        inc     b				; BC = bd0d

07d8 7c        ld      a,h
07d9 1f        rra     
07da 7d        ld      a,l
07db 1f        rra     

07dc ed79      out     (c),a			; set CRTC register 13 data
07de c1        pop     bc
07df c9        ret     


;;====================================================================
;; MC RESET PRINTER

07e0 21f707    ld      hl,$07f7
07e3 1104b8    ld      de,$b804
07e6 011500    ld      bc,$0015
07e9 edb0      ldir    

07eb 21f107    ld      hl,$07f1				;; table used to initialise printer indirections
07ee c3b40a    jp      $0ab4				;; initialise printer indirections

07f1 
defb &03
defw &bdf1									
07f4 c3b40a jp      $0835								;; IND: MC WAIT PRINTER

07f7 0a        ld      a,(bc)
07f8 a0        and     b
07f9 5e        ld      e,(hl)
07fa a1        and     c
07fb 5c        ld      e,h
07fc a2        and     d
07fd 7b        ld      a,e
07fe a3        and     e
07ff 23        inc     hl
0800 a6        and     (hl)
0801 40        ld      b,b
0802 ab        xor     e
0803 7c        ld      a,h
0804 ac        xor     h
0805 7d        ld      a,l
0806 ad        xor     l
0807 7e        ld      a,(hl)
0808 ae        xor     (hl)
0809 5d        ld      e,l
080a af        xor     a
080b 5b        ld      e,e

;;===========================================================================
;; MC PRINT TRANSLATION

080c e7        rst     $20				; RST 4 - LOW: RAM LAM
080d 87        add     a,a
080e 3c        inc     a
080f 4f        ld      c,a
0810 0600      ld      b,$00
0812 1104b8    ld      de,$b804
0815 fe2a      cp      $2a
0817 dca1ba    call    c,$baa1			;; HI: KL LDIR
081a c9        ret     

;;===========================================================================
;; MC PRINT CHAR

081b c5        push    bc
081c e5        push    hl
081d 2104b8    ld      hl,$b804
0820 46        ld      b,(hl)
0821 04        inc     b
0822 05        dec     b
0823 280a      jr      z,$082f          ; (+$0a)
0825 23        inc     hl
0826 be        cp      (hl)
0827 23        inc     hl
0828 20f8      jr      nz,$0822         ; (-$08)
082a 7e        ld      a,(hl)
082b feff      cp      $ff
082d 2803      jr      z,$0832          ; (+$03)
082f cdf1bd    call    $bdf1			; IND: MC WAIT PRINTER
0832 e1        pop     hl
0833 c1        pop     bc
0834 c9        ret     

;;====================================================================
;; IND: MC WAIT PRINTER

0835 013200    ld      bc,$0032
0838 cd5808    call    $0858			; MC BUSY PRINTER
083b 3007      jr      nc,$0844         ; MC SEND PRINTER
083d 10f9      djnz    $0838            
083f 0d        dec     c
0840 20f6      jr      nz,$0838         
0842 b7        or      a
0843 c9        ret     

;;====================================================================
;; MC SEND PRINTER
;; 
;; NOTES: 
;; - bits 6..0 of A contain the data
;; - bit 7 of data is /STROBE signal
;; - /STROBE signal is inverted by hardware; therefore 0->1 and 1->0
;; - data is written with /STROBE pulsed low 
0844 c5        push    bc
0845 06ef      ld      b,$ef			; printer I/O address
0847 e67f      and     $7f				; clear bit 7 (/STROBE)
0849 ed79      out     (c),a			; write data with /STROBE=1
084b f680      or      $80				; set bit 7 (/STROBE)
084d f3        di						
084e ed79      out     (c),a			; write data with /STROBE=0
0850 e67f      and     $7f				; clear bit 7 (/STROBE)
0852 fb        ei						
0853 ed79      out     (c),a			; write data with /STROBE=1
0855 c1        pop     bc
0856 37        scf     
0857 c9        ret     

;;====================================================================
;; MC BUSY PRINTER
;; 
;; exit:
;; carry = state of BUSY input from printer

0858 c5        push    bc
0859 4f        ld      c,a
085a 06f5      ld      b,$f5			; PPI port B I/O address
085c ed78      in      a,(c)			; read PPI port B input
085e 17        rla						; transfer bit 6 into carry (BUSY input from printer)						
085f 17        rla
0860 79        ld      a,c
0861 c1        pop     bc
0862 c9        ret     

;;====================================================================
;; MC SOUND REGISTER
;; 
;; entry:
;; A = register index
;; C = register data
;; 

0863 f3        di      

0864 06f4      ld      b,$f4			; PPI port A I/O address
0866 ed79      out     (c),a			; write register index

0868 06f6      ld      b,$f6			; PPI port C I/O address
086a ed78      in      a,(c)			; get current outputs of PPI port C I/O port
086c f6c0      or      $c0				; bit 7,6: PSG register select
086e ed79      out     (c),a			; write control to PSG. PSG will select register
										; referenced by data at PPI port A output
0870 e63f      and     $3f				; bit 7,6: PSG inactive
0872 ed79      out     (c),a			; write control to PSG.

0874 06f4      ld      b,$f4			; PPI port A I/O address
0876 ed49      out     (c),c			; write register data

0878 06f6      ld      b,$f6			; PPI port C I/O address
087a 4f        ld      c,a
087b f680      or      $80				; bit 7,6: PSG write data to selected register
087d ed79      out     (c),a			; write control to PSG. PSG will write the data
										; at PPI port A into the currently selected register
; bit 7,6: PSG inactive
087f ed49      out     (c),c			; write control to PSG
0881 fb        ei      
0882 c9        ret     

;;--------------------------------------------------------------
;; scan keyboard

;;---------------------------------------------------------------------------------
;; select PSG port A register
0883 010ef4    ld      bc,$f40e			; B = I/O address for PPI port A
										; C = 14 (index of PSG I/O port A register)
0886 ed49      out     (c),c			; write PSG register index to PPI port A

0888 06f6      ld      b,$f6			; B = I/O address for PPI port C
088a ed78      in      a,(c)			; get current port C data
088c e630      and     $30
088e 4f        ld      c,a

088f f6c0      or      $c0				; PSG operation: select register
0891 ed79      out     (c),a			; write to PPI port C 
										; PSG will use data from PPI port A
										; to select a register
0893 ed49      out     (c),c			

;;---------------------------------------------------------------------------------
;; set PPI port A to input
0895 04        inc     b				; B = &f7 (I/O address for PPI control)
0896 3e92      ld      a,$92			; PPI port A: input
										; PPI port B: input
										; PPI port C (upper and lower): output
0898 ed79      out     (c),a			; write to PPI control register

;;---------------------------------------------------------------------------------

089a c5        push    bc
089b cbf1      set     6,c				; PSG: operation: read data from selected register


089d 06f6      ld      b,$f6			; B = I/O address for PPI port C
089f ed49      out     (c),c
08a1 06f4      ld      b,$f4			; B = I/O address for PPI port A
08a3 ed78      in      a,(c)			; read selected keyboard line
										; (keyboard data->PSG port A->PPI port A)

08a5 46        ld      b,(hl)			; get previous keyboard line state
										; "0" indicates a pressed key
										; "1" indicates a released key
08a6 77        ld      (hl),a			; store new keyboard line state

08a7 a0        and     b				; a bit will be 1 where a key was not pressed
										; in the previous keyboard scan and the current keyboard scan.
										; a bit will be 0 where a key has been:
										; - pressed in previous keyboard scan, released in this keyboard scan
										; - not pressed in previous keyboard scan, pressed in this keyboard scan
										; - key has been held for previous and this keyboard scan.
08a8 2f        cpl						; change so a '1' now indicates held/pressed key
										; '0' indicates a key that has not been pressed/held
08a9 12        ld      (de),a			; store keybaord line data

08aa 23        inc     hl
08ab 13        inc     de
08ac 0c        inc     c

08ad 79        ld      a,c
08ae e60f      and     $0f				; current keyboard line
08b0 fe0a      cp      $0a				; 10 keyboard lines
08b2 20e9      jr      nz,$089d         

08b4 c1        pop     bc
;; B = I/O address of PPI control register
08b5 3e82      ld      a,$82			; PPI port A: output
										; PPI port B: input
										; PPI port C (upper and lower): output
08b7 ed79      out     (c),a
;; B = I/O address of PPI port C lower

08b9 05        dec     b
08ba ed49      out     (c),c
08bc c9        ret     

;;--------------------------------------------------------------
;; JUMP RESTORE
;;
;; (restore all the firmware jump routines)

;; main firmware jumpblock
08bd 21de08    ld      hl,$08de			; table of addressess for firmware functions
08c0 1100bb    ld      de,$bb00			; start of firmware jumpblock
08c3 01cfcb    ld      bc,$cbcf			; B = 203 entries, C = 0x0cf -> RST 1 -> LOW: LOW JUMP
08c6 cdcc08    call    $08cc

08c9 01ef20    ld      bc,$20ef			; B = number of entries: 32 entries
										; C=  0x0ef -> RST 5 -> LOW: FIRM JUMP
;;-------------------------------------------------------------------------------------
; C = 0x0cf -> RST 1 -> LOW: LOW JUMP
; OR
; C=  0x0ef -> RST 5 -> LOW: FIRM JUMP

08cc 79        ld      a,c				; write RST instruction 			
08cd 12        ld      (de),a
08ce 13        inc     de
08cf eda0      ldi     					; write low byte of address in ROM
08d1 03        inc     bc
08d2 2f        cpl     
08d3 07        rlca    
08d4 07        rlca    
08d5 e680      and     $80
08d7 b6        or      (hl)
08d8 12        ld      (de),a			; write high byte of address in ROM
08d9 13        inc     de
08da 23        inc     hl
08db 10ef      djnz    $08cc            
08dd c9        ret     

;; each entry is an address (within this ROM) which will perform
;; the associated firmware function
08de
defw &1b5c		;; 0 firmware function: KM INITIALISE
defw &1b98		;; 1 firmware function: KM RESET 
defw &1bbf		;; 2 firmware function: KM WAIT CHAR
defw &1bc5		;; 3 firmware function: KM READ CHAR 
defw &1bfa		;; 4 firmware function: KM CHAR RETURN
defw &1c46		;; 5 firmware function: KM SET EXPAND
defw &1cb3		;; 6 firmware function: KM GET EXPAND
defw &1c04		;; 7 firmware function: KM EXP BUFFER
defw &1cdb		;; 8 firmware function: KM WAIT KEY
defw &1ce1		;; 9 firmware function: KM READ KEY
defw &1e45		;; 10 firmware function: KM TEST KEY
defw &1d38		;; 11 firmware function: KM GET STATE
defw &1de5		;; 12 firmware function: KM GET JOYSTICK
defw &1ed8		;; 13 firmware function: KM SET TRANSLATE
defw &1ec4		;; 14 firmware function: KM GET TRANSLATE
defw &1edd		;; 15 firmware function: KM SET SHIFT
defw &1ec9		;; 16 firmware function: KM GET SHIFT
defw &1ee2		;; 17 firmware function: KM SET CONTROL 
defw &1ece		;; 18 firmware function: KM GET CONTROL 
defw &1e34		;; 19 firmware function: KM SET REPEAT
defw &1e2f		;; 20 firmware function: KM GET REPEAT
defw &1df6		;; 21 firmware function: KM SET DELAY
defw &1df2		;; 22 firmware function: KM GET DELAY
defw &1dfa		;; 23 firmware function: KM ARM BREAK
defw &1e0b		;; 24 firmware function: KM DISARM BREAK
defw &1e19		;; 25 firmware function: KM BREAK EVENT 
defw &1074		;; 26 firmware function: TXT INITIALISE
defw &1084		;; 27 firmware function: TXT RESET
defw &1459		;; 28 firmware function: TXT VDU ENABLE
defw &1452		;; 29 firmware function: TXT VDU DISABLE
defw &13fe		;; 30 firmware function: TXT OUTPUT
defw &1335		;; 31 firmware function: TXT WR CHAR
defw &13ac		;; 32 firmware function: TXT RD CHAR
defw &13a8		;; 33 firmware function: TXT SET GRAPHIC
defw &1208		;; 34 firmware function: TXT WIN ENABLE
defw &1252		;; 35 firmware function: TXT GET WINDOW
defw &154f		;; 36 firmware function: TXT CLEAR WINDOW
defw &115a		;; 37 firmware function: TXT SET COLUMN
defw &1165		;; 38 firmware function: TXT SET ROW
defw &1170		;; 39 firmware function: TXT SET CURSOR
defw &117c		;; 40 firmware function: TXT GET CURSOR
defw &1286		;; 41 firmware function: TXT CUR ENABLE
defw &1297		;; 42 firmware function: TXT CUR DISABLE
defw &1276		;; 43 firmware function: TXT CUR ON
defw &127e		;; 44 firmware function: TXT CUR OFF
defw &11ca		;; 45 firmware function: TXT VALIDATE
defw &1265		;; 46 firmware function: TXT PLACE CURSOR
defw &1265		;; 47 firmware function: TXT REMOVE CURSOR
defw &12a6		;; 48 firmware function: TXT SET PEN 
defw &12ba		;; 49 firmware function: TXT GET PEN
defw &12ab		;; 50 firmware function: TXT SET PAPER
defw &12c0		;; 51 firmware function: TXT GET PAPER
defw &12c6		;; 52 firmware function: TXT INVERSE
defw &137b		;; 53 firmware function: TXT SET BACK
defw &1388		;; 54 firmware function: TXT GET BACK
defw &12d4		;; 55 firmware function: TXT GET MATRIX
defw &12f2		;; 56 firmware function: TXT SET MATRIX
defw &12fe		;; 57 firmware function: TXT SET M TABLE
defw &132b		;; 58 firmware function: TXT GET M TABLE
defw &14d4		;; 59 firmware function: TXT GET CONTROLS
defw &10e4		;; 60 firmware function: TXT STR SELECT
defw &1103		;; 61 firmware function: TXT SWAP STREAMS
defw &15a8		;; 62 firmware function: GRA INITIALISE
defw &15d7		;; 63 firmware function: GRA RESET
defw &15fe		;; 64 firmware function: GRA MOVE ABSOLUTE
defw &15fb		;; 65 firmware function: GRA MOVE RELATIVE
defw &1606		;; 66 firmware function: GRA ASK CURSOR
defw &160e		;; 67 firmware function: GRA SET ORIGIN
defw &161c		;; 68 firmware function: GRA GET ORIGIN
defw &16a5		;; 69 firmware function: GRA WIN WIDTH
defw &16ea		;; 70 firmware function: GRA WIN HEIGHT
defw &1717		;; 71 firmware function: GRA GET W WIDTH
defw &172d		;; 72 firmware function: GRA GET W HEIGHT
defw &1736		;; 73 firmware function: GRA CLEAR WINDOW
defw &1767		;; 74 firmware function: GRA SET PEN
defw &1775		;; 75 firmware function: GRA GET PEN
defw &176e		;; 76 firmware function: GRA SET PAPER
defw &177a		;; 77 firmware function: GRA GET PAPER
defw &1783		;; 78 firmware function: GRA PLOT ABSOLUTE
defw &1780		;; 79 firmware function: GRA PLOT RELATIVE
defw &1797		;; 80 firmware function: GRA TEST ABSOLUTE
defw &1794		;; 81 firmware function: GRA TEST RELATIVE
defw &17a9		;; 82 firmware function: GRA LINE ABSOLUTE
defw &17a6		;; 83 firmware function: GRA LINE RELATIVE
defw &1940		;; 84 firmware function: GRA WR CHAR
defw &0abf		;; 85 firmware function: SCR INITIALIZE
defw &0ad0		;; 86 firmware function: SCR RESET
defw &0b37		;; 87 firmware function: SCR OFFSET
defw &0b3c		;; 88 firmware function: SCR SET BASE
defw &0b56		;; 89 firmware function: SCR GET LOCATION
defw &0ae9		;; 90 firmware function: SCR SET MODE
defw &0b0c		;; 91 firmware function: SCR GET MODE
defw &0b17		;; 92 firmware function: SCR CLEAR
defw &0b5d		;; 93 firmware function: SCR CHAR LIMITS
defw &0b6a		;; 94 firmware function: SCR CHAR POSITION
defw &0baf		;; 95 firmware function: SCR DOT POSITION
defw &0c05		;; 96 firmware function: SCR NEXT BYTE
defw &0c11		;; 97 firmware function: SCR PREV BYTE
defw &0c1f		;; 98 firmware function: SCR NEXT LINE
defw &0c39		;; 99 firmware function: SCR PREV LINE
defw &0c8e		;; 100 firmware function: SCR INK ENCODE
defw &0ca7		;; 101 firmware function: SCR INK DECODE
defw &0cf2		;; 102 firmware function: SCR SET INK
defw &0d1a		;; 103 firmware function: SCR GET INK
defw &0cf7		;; 104 firmware function: SCR SET BORDER
defw &0d1f		;; 105 firmware function: SCR GET BORDER
defw &0cea		;; 106 firmware function: SCR SET FLASHING
defw &0cee		;; 107 firmware function: SCR GET FLASHING
defw &0db9		;; 108 firmware function: SCR FILL BOX
defw &0dbd		;; 109 firmware function: SCR FLOOD BOX
defw &0de5		;; 110 firmware function: SCR CHAR INVERT
defw &0e00		;; 111 firmware function: SCR HW ROLL
defw &0e44		;; 112 firmware function: SCR SW ROLL
defw &0ef9		;; 113 firmware function: SCR UNPACK
defw &0f2a		;; 114 firmware function: SCR REPACK
defw &0c55		;; 115 firmware function: SCR ACCESS
defw &0c74		;; 116 firmware function: SCR PIXELS
defw &0f93		;; 117 firmware function: SCR HORIZONTAL
defw &0f9b		;; 118 firmware function: SCR VERTICAL
defw &24bc		;; 119 firmware function: CAS INITIALISE
defw &24ce		;; 120 firmware function: CAS SET SPEED
defw &24e1		;; 121 firmware function: CAS NOISY
defw &2bbb		;; 122 firmware function: CAS START MOTOR
defw &2bbf		;; 123 firmware function: CAS STOP MOTOR
defw &2bc1		;; 124 firmware function: CAS RESTORE MOTOR
defw &24e5		;; 125 firmware function: CAS IN OPEN
defw &2550		;; 126 firmware function: CAS IN CLOSE
defw &2557		;; 127 firmware function: CAS IN ABANDON
defw &25a0		;; 128 firmware function: CAS IN CHAR
defw &2618		;; 129 firmware function: CAS IN DIRECT
defw &2607		;; 130 firmware function: CAS RETURN
defw &2603		;; 131 firmware function: CAS TEST EOF
defw &24fe		;; 132 firmware function: CAS OUT OPEN
defw &257f		;; 133 firmware function: CAS OUT CLOSE
defw &2599		;; 134 firmware function: CAS OUT ABANDON
defw &25c6		;; 135 firmware function: CAS OUT CHAR
defw &2653		;; 136 firmware function: CAS OUT DIRECT
defw &2692		;; 137 firmware function: CAS CATALOG
defw &29af		;; 138 firmware function: CAS WRITE
defw &29a6		;; 139 firmware function: CAS READ
defw &29c1		;; 140 firmware function: CAS CHECK
defw &1fe9		;; 141 firmware function: SOUND RESET
defw &2114		;; 142 firmware function: SOUND QUEUE
defw &21ce		;; 143 firmware function: SOUND CHECK
defw &21eb		;; 144 firmware function: SOUND ARM EVENT
defw &21ac		;; 145 firmware function: SOUND RELEASE
defw &2050		;; 146 firmware function: SOUND HOLD
defw &206b		;; 147 firmware function: SOUND CONTINUE
defw &2495		;; 148 firmware function: SOUND AMPL ENVELOPE
defw &249a		;; 149 firmware function: SOUND TONE ENVELOPE
defw &24a6		;; 150 firmware function: SOUND A ADDRESS
defw &24ab		;; 151 firmware function: SOUND T ADDRESS
defw &005c		;; 152 firmware function: KL CHOKE OFF
defw &0326		;; 153 firmware function: KL ROM WALK
defw &0330		;; 154 firmware function: KL INIT BACK
defw &02a0		;; 155 firmware function: KL LOG EXT
defw &02b1		;; 156 firmware function: KL FIND COMMAND
defw &0163		;; 157 firmware function: KL NEW FRAME FLY
defw &016a		;; 158 firmware function: KL ADD FRAME FLY
defw &0170		;; 159 firmware function: KL DEL FRAME FLY
defw &0176		;; 160 firmware function: KL NEW FAST TICKER
defw &017d		;; 161 firmware function: KL ADD FAST TICKER
defw &0183		;; 162 firmware function: KL DEL FAST TICKER
defw &01b3		;; 163 firmware function: KL ADD TICKER
defw &01c5		;; 164 firmware function: KL DEL TICKER
defw &01d2		;; 165 firmware function: KL INIT EVENT
defw &01e2		;; 166 firmware function: KL EVENT
defw &0227		;; 167 firmware function: KL SYNC RESET
defw &0284		;; 168 firmware function: KL DEL SYNCHRONOUS
defw &0255		;; 169 firmware function: KL NEXT SYNC
defw &0219		;; 170 firmware function: KL DO SYNC
defw &0276		;; 171 firmware function: KL DONE SYNC
defw &0294		;; 172 firmware function: KL EVENT DISABLE
defw &029a		;; 173 firmware function: KL EVENT ENABLE
defw &028d		;; 174 firmware function: KL DISARM EVENT
defw &0099		;; 175 firmware function: KL TIME PLEASE
defw &00a3		;; 176 firmware function: KL TIME SET
defw &05ed		;; 177 firmware function: MC BOOT PROGRAM
defw &061c		;; 178 firmware function: MC START PROGRAM
defw &07b4		;; 179 firmware function: MC WAIT FLYBACK
defw &0776		;; 180 firmware function: MC SET MODE 
defw &07c0		;; 181 firmware function: MC SCREEN OFFSET
defw &0786		;; 182 firmware function: MC CLEAR INKS
defw &078c		;; 183 firmware function: MC SET INKS
defw &07e0		;; 184 firmware function: MC RESET PRINTER
defw &081b		;; 185 firmware function: MC PRINT CHAR
defw &0858		;; 186 firmware function: MC BUSY PRINTER
defw &0844		;; 187 firmware function: MC SEND PRINTER
defw &0863		;; 188 firmware function: MC SOUND REGISTER
defw &08bd		;; 189 firmware function: JUMP RESTORE
defw &1d3c		;; 190 firmware function: KM SET LOCKS
defw &1bfe		;; 191 firmware function: KM FLUSH
defw &1460		;; 192 firmware function: TXT ASK STATE
defw &15ec		;; 193 firmware function: GRA DEFAULT
defw &19d5		;; 194 firmware function: GRA SET BACK
defw &17b0		;; 195 firmware function: GRA SET FIRST
defw &17ac		;; 196 firmware function: GRA SET LINE MASK
defw &162a		;; 197 firmware function: GRA FROM USER
defw &19d9		;; 198 firmware function: GRA FILL
defw &0b45		;; 199 firmware function: SCR SET POSITION
defw &080c		;; 200 firmware function: MC PRINT TRANSLATION
defw &0397		;; 201 firmware function: KL BANK SWITCH
defw &2c02		;; 202 BD5E
defw &2f91		;; 0 BD61
defw &2f9f		;; 1 BD64
defw &2fc8		;; 2 BD67
defw &2fd9		;; 3 BD6A
defw &3001		;; 4 BD6D
defw &3014		;; 5 BD70
defw &3055		;; 6 BD73
defw &305f		;; 7 BD76
defw &30c6		;; 8 BD79
defw &34a2		;; 9 BD7C
defw &3159		;; 10 BD7F
defw &349e		;; 11 BD82
defw &3577		;; 12 BD85
defw &3604		;; 13 BD88
defw &3188		;; 14 BD8B
defw &36df		;; 15 BD8E
defw &3731		;; 16 BD91
defw &3727		;; 17 BD94
defw &3345		;; 18 BD97
defw &2f73		;; 19 BD9A
defw &32ac		;; 20 BD9D
defw &32af		;; 21 BDA0
defw &31b6		;; 22 BDA3
defw &31b1		;; 23 BDA6
defw &322f		;; 24 BDA9
defw &3353		;; 25 BDAC
defw &3349		;; 26 BDAF
defw &33c8		;; 27 BDB2
defw &33d8		;; 28 BDB5
defw &2fd1		;; 29 BDB8
defw &3136		;; 30 BDBB
defw &3143		;; 31 BDBE

;;==========================================================================
;; used to initialise the firmware indirections
;; this routine is called by each of the firmware "units"
;; i.e. screen pack, graphics pack etc.

;; HL = pointer to start of a table

;; table format:
;;
;; 0 = length of data 
;; 1,2 = destination to copy data
;; 3.. = data

0ab4 4e        ld      c,(hl)
0ab5 06 00     ld      b,$00
0ab7 23        inc     hl
0ab8 5e        ld      e,(hl)
0ab9 23        inc     hl
0aba 56        ld      d,(hl)
0abb 23        inc     hl
0abc edb0      ldir    
0abe c9        ret     

;;===========================================================================
;; SCR INITIALISE

0abf 115210    ld      de,$1052			;; default colour palette
0ac2 cd8607    call    $0786			;; MC CLEAR INKS
0ac5 3ec0      ld      a,$c0
0ac7 32c6b7    ld      ($b7c6),a
0aca cdd00a    call    $0ad0			;; SCR RESET
0acd c3120b    jp      $0b12

;;===========================================================================
;; SCR RESET

0ad0 af        xor     a
0ad1 cd550c    call    $0c55			;; SCR ACCESS
0ad4 21dd0a    ld      hl,$0add			;; table used to initialise screen indirections
0ad7 cdb40a    call    $0ab4			;; initialise screen pack indirections
0ada c3d80c    jp      $0cd8			;; restore colours and set default flashing

0add 
defb &09
defw &bde5
0ae0 c38a0c    jp      $0c8a			;; IND: SCR READ
0ae3 c3710c    jp      $0c71			;; IND: SCR WRITE
0ae6 c3170b    jp      $0b17			;; IND: SCR MODE CLEAR

;;===========================================================================
;; SCR SET MODE

0ae9 e603      and     $03
0aeb fe03      cp      $03
0aed d0        ret     nc

0aee f5        push    af
0aef cd550d    call    $0d55
0af2 d1        pop     de
0af3 cdb310    call    $10b3
0af6 f5        push    af
0af7 cdce15    call    $15ce
0afa e5        push    hl
0afb 7a        ld      a,d
0afc cd310b    call    $0b31
0aff cdebbd    call    $bdeb			; IND: SCR MODE CLEAR
0b02 e1        pop     hl
0b03 cdae15    call    $15ae
0b06 f1        pop     af
0b07 cdd110    call    $10d1
0b0a 1822      jr      $0b2e            ; (+$22)

;;===========================================================================
;; SCR GET MODE

0b0c 3ac3b7    ld      a,($b7c3)
0b0f fe01      cp      $01
0b11 c9        ret     

0b12 3e01      ld      a,$01
0b14 cd310b    call    $0b31

;;===========================================================================
;; SCR CLEAR

0b17 cd550d    call    $0d55
0b1a 210000    ld      hl,$0000
0b1d cd370b    call    $0b37			;; SCR OFFSET
0b20 2ac5b7    ld      hl,($b7c5)
0b23 2e00      ld      l,$00
0b25 54        ld      d,h
0b26 1e01      ld      e,$01
0b28 01ff3f    ld      bc,$3fff
0b2b 75        ld      (hl),l
0b2c edb0      ldir    
0b2e c3420d    jp      $0d42

;;===========================================================================
0b31 32c3b7    ld      ($b7c3),a
0b34 c37607    jp      $0776			; MC SET MODE

;;===========================================================================
;; SCR OFFSET

0b37 3ac6b7    ld      a,($b7c6)
0b3a 1803      jr      $0b3f            ; (+$03)

;;===========================================================================
;; SCR SET BASE

0b3c 2ac4b7    ld      hl,($b7c4)
0b3f cd450b    call    $0b45			; SCR SET POSITION
0b42 c3c007    jp      $07c0			; MC SCREEN OFFSET

;;===========================================================================
;; SCR SET POSITION

0b45 e6c0      and     $c0
0b47 32c6b7    ld      ($b7c6),a
0b4a f5        push    af
0b4b 7c        ld      a,h
0b4c e607      and     $07
0b4e 67        ld      h,a
0b4f cb85      res     0,l
0b51 22c4b7    ld      ($b7c4),hl
0b54 f1        pop     af
0b55 c9        ret     

;;===========================================================================
;; SCR GET LOCATION

0b56 2ac4b7    ld      hl,($b7c4)
0b59 3ac6b7    ld      a,($b7c6)
0b5c c9        ret     

;;======================================================================================
;; SCR CHAR LIMITS
0b5d cd0c0b    call    $0b0c			;; SCR GET MODE
0b60 011813    ld      bc,$1318			;; B = 19, C = 24
0b63 d8        ret     c

0b64 0627      ld      b,$27			;; 39
0b66 c8        ret     z

0b67 064f      ld      b,$4f			;; 79
;; B = x limit-1
;; C = y limit-1
0b69 c9        ret     

;;======================================================================================
;; SCR CHAR POSITION

0b6a d5        push    de
0b6b cd0c0b    call    $0b0c			;; SCR GET MODE
0b6e 0604      ld      b,$04
0b70 3805      jr      c,$0b77          ; (+$05)
0b72 0602      ld      b,$02
0b74 2801      jr      z,$0b77          ; (+$01)
0b76 05        dec     b
0b77 c5        push    bc
0b78 5c        ld      e,h
0b79 1600      ld      d,$00
0b7b 62        ld      h,d
0b7c d5        push    de
0b7d 54        ld      d,h
0b7e 5d        ld      e,l
0b7f 29        add     hl,hl
0b80 29        add     hl,hl
0b81 19        add     hl,de
0b82 29        add     hl,hl
0b83 29        add     hl,hl
0b84 29        add     hl,hl
0b85 29        add     hl,hl
0b86 d1        pop     de
0b87 19        add     hl,de
0b88 10fd      djnz    $0b87            ; (-$03)
0b8a ed5bc4b7  ld      de,($b7c4)
0b8e 19        add     hl,de
0b8f 7c        ld      a,h
0b90 e607      and     $07
0b92 67        ld      h,a
0b93 3ac6b7    ld      a,($b7c6)
0b96 84        add     a,h
0b97 67        ld      h,a
0b98 c1        pop     bc
0b99 d1        pop     de
0b9a c9        ret     

0b9b 7b        ld      a,e
0b9c 95        sub     l
0b9d 3c        inc     a
0b9e 87        add     a,a
0b9f 87        add     a,a
0ba0 87        add     a,a
0ba1 5f        ld      e,a
0ba2 7a        ld      a,d
0ba3 94        sub     h
0ba4 3c        inc     a
0ba5 57        ld      d,a
0ba6 cd6a0b    call    $0b6a			; SCR CHAR POSITION
0ba9 af        xor     a
0baa 82        add     a,d
0bab 10fd      djnz    $0baa            ; (-$03)
0bad 57        ld      d,a
0bae c9        ret     

;;======================================================================================
;; SCR DOT POSITION

0baf d5        push    de
0bb0 eb        ex      de,hl
0bb1 21c700    ld      hl,$00c7
0bb4 b7        or      a
0bb5 ed52      sbc     hl,de
0bb7 7d        ld      a,l
0bb8 e607      and     $07
0bba 87        add     a,a
0bbb 87        add     a,a
0bbc 87        add     a,a
0bbd 4f        ld      c,a
0bbe 7d        ld      a,l
0bbf e6f8      and     $f8
0bc1 6f        ld      l,a
0bc2 54        ld      d,h
0bc3 5d        ld      e,l
0bc4 29        add     hl,hl
0bc5 29        add     hl,hl
0bc6 19        add     hl,de
0bc7 29        add     hl,hl
0bc8 d1        pop     de
0bc9 c5        push    bc
0bca cdf60b    call    $0bf6
0bcd 78        ld      a,b
0bce a3        and     e
0bcf 2805      jr      z,$0bd6          ; (+$05)
0bd1 cb09      rrc     c
0bd3 3d        dec     a
0bd4 20fb      jr      nz,$0bd1         ; (-$05)
0bd6 e3        ex      (sp),hl
0bd7 61        ld      h,c
0bd8 4d        ld      c,l
0bd9 e3        ex      (sp),hl
0bda 78        ld      a,b
0bdb 0f        rrca    
0bdc cb3a      srl     d
0bde cb1b      rr      e
0be0 0f        rrca    
0be1 38f9      jr      c,$0bdc          ; (-$07)
0be3 19        add     hl,de
0be4 ed5bc4b7  ld      de,($b7c4)
0be8 19        add     hl,de
0be9 7c        ld      a,h
0bea e607      and     $07
0bec 67        ld      h,a
0bed 3ac6b7    ld      a,($b7c6)
0bf0 84        add     a,h
0bf1 81        add     a,c
0bf2 67        ld      h,a
0bf3 d1        pop     de
0bf4 4a        ld      c,d
0bf5 c9        ret     

;;---------------------------------------------------------------------
0bf6 cd0c0b    call    $0b0c			;; SCR GET MODE
0bf9 01aa01    ld      bc,$01aa
0bfc d8        ret     c

0bfd 018803    ld      bc,$0388 ; remove event from list
0c00 c8        ret     z

0c01 018007    ld      bc,$0780
0c04 c9        ret     


;;==========================================================================
;; SCR NEXT BYTE
;;
;; Entry conditions:
;; HL = screen address
;; Exit conditions:
;; HL = updated screen address
;; AF corrupt
;;
;; Assumes:
;; - 16k screen

0c05 2c        inc     l
0c06 c0        ret     nz

0c07 24        inc     h
0c08 7c        ld      a,h
0c09 e607      and     $07
0c0b c0        ret     nz

;; at this point the address has incremented over a 2048
;; byte boundary.
;;
;; At this point, the next byte on screen is *not* previous byte plus 1.
;;
;; The following is true:
;; 07FF->0000
;; 0FFF->0800
;; 17FF->1000
;; 1FFF->1800
;; 27FF->2000
;; 2FFF->2800
;; 37FF->3000
;; 3FFF->3800
;;
;; The following code adjusts for this case.

0c0c 7c        ld      a,h
0c0d d608      sub     $08
0c0f 67        ld      h,a
0c10 c9        ret     

;;==========================================================================
;; SCR PREV BYTE
;;
;; Entry conditions:
;; HL = screen address
;; Exit conditions:
;; HL = updated screen address
;; AF corrupt
;;
;; Assumes:
;; - 16k screen

0c11 7d        ld      a,l
0c12 2d        dec     l
0c13 b7        or      a
0c14 c0        ret     nz

0c15 7c        ld      a,h
0c16 25        dec     h
0c17 e607      and     $07
0c19 c0        ret     nz

0c1a 7c        ld      a,h
0c1b c608      add     a,$08
0c1d 67        ld      h,a
0c1e c9        ret     

;;==========================================================================
;; SCR NEXT LINE
;;
;; Entry conditions:
;; HL = screen address
;; Exit conditions:
;; HL = updated screen address
;; AF corrupt
;;
;; Assumes:
;; - 16k screen
;; - 80 bytes per line (40 CRTC characters per line)

0c1f 7c        ld      a,h
0c20 c608      add     a,$08
0c22 67        ld      h,a


0c23 e638      and     $38
0c25 c0        ret     nz

;; 

0c26 7c        ld      a,h
0c27 d640      sub     $40
0c29 67        ld      h,a
0c2a 7d        ld      a,l
0c2b c650      add     a,$50			;; number of bytes per line
0c2d 6f        ld      l,a
0c2e d0        ret     nc

0c2f 24        inc     h
0c30 7c        ld      a,h
0c31 e607      and     $07
0c33 c0        ret     nz

0c34 7c        ld      a,h
0c35 d608      sub     $08
0c37 67        ld      h,a
0c38 c9        ret     

;;==========================================================================
;; SCR PREV LINE
;;
;; Entry conditions:
;; HL = screen address
;; Exit conditions:
;; HL = updated screen address
;; AF corrupt
;;
;; Assumes:
;; - 16k screen
;; - 80 bytes per line (40 CRTC characters per line)

0c39 7c        ld      a,h
0c3a d608      sub     $08
0c3c 67        ld      h,a
0c3d e638      and     $38
0c3f fe38      cp      $38
0c41 c0        ret     nz

0c42 7c        ld      a,h
0c43 c640      add     a,$40
0c45 67        ld      h,a

0c46 7d        ld      a,l
0c47 d650      sub     $50				;; number of bytes per line
0c49 6f        ld      l,a
0c4a d0        ret     nc

0c4b 7c        ld      a,h
0c4c 25        dec     h
0c4d e607      and     $07
0c4f c0        ret     nz

0c50 7c        ld      a,h
0c51 c608      add     a,$08
0c53 67        ld      h,a
0c54 c9        ret     


;;============================================================================
;; SCR ACCESS
;;
;; A = write mode:
;; 0 -> fill
;; 1 -> XOR
;; 2 -> AND
;; 3 -> OR 
0c55 e603      and     $03
0c57 21740c    ld      hl,$0c74			; SCR PIXELS
0c5a 280c      jr      z,$0c68          ; (+$0c)
0c5c fe02      cp      $02
0c5e 2e7a      ld      l,$7a
0c60 3806      jr      c,$0c68          ; (+$06)
0c62 2e7f      ld      l,$7f
0c64 2802      jr      z,$0c68          ; (+$02)
0c66 2e85      ld      l,$85
;; HL = address of screen write function 
;; initialise jump for IND: SCR WRITE
0c68 3ec3      ld      a,$c3
0c6a 32c7b7    ld      ($b7c7),a
0c6d 22c8b7    ld      ($b7c8),hl
0c70 c9        ret     

;;==================================================================================
;; IND: SCR WRITE

;; jump initialised by SCR ACCESS
0c71 c3c7b7    jp      $b7c7


;;============================================================================
;; SCR PIXELS
;; (write mode fill)
0c74 78        ld      a,b
0c75 ae        xor     (hl)
0c76 a1        and     c
0c77 ae        xor     (hl)
0c78 77        ld      (hl),a
0c79 c9        ret     

;;----------------------------------------------------------------------------
;; screen write access mode

;; (write mode XOR)
0c7a 78        ld      a,b
0c7b a1        and     c
0c7c ae        xor     (hl)
0c7d 77        ld      (hl),a
0c7e c9        ret     

;;----------------------------------------------------------------------------
;; screen write access mode
;;
;; (write mode AND)
0c7f 79        ld      a,c
0c80 2f        cpl     
0c81 b0        or      b
0c82 a6        and     (hl)
0c83 77        ld      (hl),a
0c84 c9        ret     

;;----------------------------------------------------------------------------
;; screen write access mode
;;
;; (write mode OR)
0c85 78        ld      a,b
0c86 a1        and     c
0c87 b6        or      (hl)
0c88 77        ld      (hl),a
0c89 c9        ret     

;;==================================================================================
;; IND: SCR READ
0c8a 7e        ld      a,(hl)
0c8b c3b20c    jp      $0cb2

;;==================================================================================
;; SCR INK ENCODE
0c8e c5        push    bc
0c8f d5        push    de
0c90 cdc80c    call    $0cc8
0c93 5f        ld      e,a
0c94 cdf60b    call    $0bf6
0c97 0608      ld      b,$08
0c99 cb0b      rrc     e
0c9b 17        rla     
0c9c cb09      rrc     c
0c9e 3802      jr      c,$0ca2          ; (+$02)
0ca0 cb03      rlc     e
0ca2 10f5      djnz    $0c99            ; (-$0b)
0ca4 d1        pop     de
0ca5 c1        pop     bc
0ca6 c9        ret     

;;============================================================================
;; SCR INK DECODE

0ca7 c5        push    bc
0ca8 f5        push    af
0ca9 cdf60b    call    $0bf6
0cac f1        pop     af
0cad cdb20c    call    $0cb2
0cb0 c1        pop     bc
0cb1 c9        ret     

;;-----------------------------------------------------------------------------

0cb2 d5        push    de
0cb3 110800    ld      de,$0008
0cb6 0f        rrca    
0cb7 cb12      rl      d
0cb9 cb09      rrc     c
0cbb 3802      jr      c,$0cbf          ; (+$02)
0cbd cb1a      rr      d
0cbf 1d        dec     e
0cc0 20f4      jr      nz,$0cb6         ; (-$0c)
0cc2 7a        ld      a,d
0cc3 cdc80c    call    $0cc8
0cc6 d1        pop     de
0cc7 c9        ret     

;;-----------------------------------------------------------------------------
0cc8 57        ld      d,a
0cc9 cd0c0b    call    $0b0c			;; SCR GET MODE
0ccc 7a        ld      a,d
0ccd d0        ret     nc
0cce 0f        rrca    
0ccf 0f        rrca    
0cd0 ce00      adc     a,$00
0cd2 0f        rrca    
0cd3 9f        sbc     a,a
0cd4 e606      and     $06
0cd6 aa        xor     d
0cd7 c9        ret     

;;-----------------------------------------------------------------------------

;; restore colours and set default flashing
0cd8 215210    ld      hl,$1052					;; default colour palette
0cdb 11d4b7    ld      de,$b7d4
0cde 012200    ld      bc,$0022
0ce1 edb0      ldir    
0ce3 af        xor     a
0ce4 32f6b7    ld      ($b7f6),a
0ce7 210a0a    ld      hl,$0a0a

;;============================================================================
;; SCR SET FLASHING

0cea 22d2b7    ld      ($b7d2),hl
0ced c9        ret     


;;============================================================================
;; SCR GET FLASHING

0cee 2ad2b7    ld      hl,($b7d2)
0cf1 c9        ret     

;;============================================================================
;; SCR SET INK
0cf2 e60f      and     $0f              ; keep pen within 0-15 range
0cf4 3c        inc     a                
0cf5 1801      jr      $0cf8

;;============================================================================
;; SCR SET BORDER
0cf7 af        xor     a
;;----------------------------------------------------------------------------
;; SCR SET INK/SCR SET BORDER
;;
;; A = internal pen number
;; B = ink 1 (firmware colour number)
;; C = ink 2 (firmware colour number)
;; 0 = border
;; 1 = colour 0
;; 2 = colour 1
;; ...
;; 16 = colour 15
0cf8 5f        ld      e,a

0cf9 78        ld      a,b
0cfa cd100d    call    $0d10		; lookup address of hardware colour number in conversion
									; table using software colour number
									
0cfd 46        ld      b,(hl)		; get hardware colour number for ink 1

0cfe 79        ld      a,c
0cff cd100d    call    $0d10		; lookup address of hardware colour number in conversion
									; table using software colour number
									
0d02 4e        ld      c,(hl)		; get hardware colour number for ink 2

0d03 7b        ld      a,e
0d04 cd350d    call    $0d35		; get address of pen in both palette's in RAM

0d07 71        ld      (hl),c		; write ink 2
0d08 eb        ex      de,hl
0d09 70        ld      (hl),b		; write ink 1

0d0a 3eff      ld      a,$ff
0d0c 32f7b7    ld      ($b7f7),a
0d0f c9        ret     

;;============================================================================
;; input:
;; A = software colour number
;; output:
;; HL = address of element in table. Element is corresponding hardware colour number.
0d10 e61f      and     $1f
0d12 c699      add     a,$99
0d14 6f        ld      l,a
0d15 ce0d      adc     a,$0d
0d17 95        sub     l
0d18 67        ld      h,a
0d19 c9        ret     

;;============================================================================
;; SCR GET INK
0d1a e60f      and     $0f            ; keep pen within range 0-15.
0d1c 3c        inc     a
0d1d 1801      jr      $0d20           

;;============================================================================
;; SCR GET BORDER

0d1f af        xor     a
;;----------------------------------------------------------------------------
;; SCR GET INK/SCR GET BORDER
;; entry:
;; A = internal pen number
;; 0 = border
;; 1 = colour 0
;; 2 = colour 1
;; ...
;; 16 = colour 15
;; exit:
;; B = ink 1 (software colour number)
;; C = ink 2 (software colour number)
0d20 cd350d    call    $0d35			; get address of pen in both palette's in RAM
0d23 1a        ld      a,(de)			; ink 2

0d24 5e        ld      e,(hl)			; ink 1

0d25 cd2a0d    call    $0d2a			; lookup hardware colour number for ink 2
0d28 41        ld      b,c

;; lookup hardware colour number for ink 1
0d29 7b        ld      a,e

;;----------------------------------------------------------------------------
;; lookup software colour number which corresponds to the hardware colour number

;; entry:
;; A = hardware colour number
;; exit:
;; C = index in table (same as software colour number)
0d2a 0e00      ld      c,$00
0d2c 21990d    ld      hl,$0d99			; table to convert from software colour
										; number to hardware colour number
;;----------
0d2f be        cp      (hl)				; same as this entry in the table?
0d30 c8        ret     z				; zero set if entry is the same, zero clear if entry is different
0d31 23        inc     hl
0d32 0c        inc     c
0d33 18fa      jr      $0d2f

;;============================================================================
;;
;; The firmware stores two palette's in RAM, this allows a pen to have a flashing ink.
;;
;; get address of palette entry for corresponding ink for both palettes in RAM.
;;
;; entry:
;; A = pen number
;; 0 = border
;; 1 = colour 0
;; 2 = colour 1
;; ...
;; 16 = colour 15
;; 
;; exit:
;; HL = address of element in palette 2
;; DE = address of element in palette 1
0d35 5f        ld      e,a
0d36 1600      ld      d,$00
0d38 21e5b7    ld      hl,$b7e5			; palette 2 start
0d3b 19        add     hl,de
0d3c eb        ex      de,hl
0d3d 21efff    ld      hl,$ffef			; palette 1 start (B7D4)
0d40 19        add     hl,de
0d41 c9        ret     
;;============================================================================

0d42 21f9b7    ld      hl,$b7f9
0d45 e5        push    hl
0d46 cd7001    call    $0170			; KL DEL FRAME FLY
0d49 cd730d    call    $0d73
0d4c 11610d    ld      de,$0d61
0d4f 0681      ld      b,$81
0d51 e1        pop     hl
0d52 c36301    jp      $0163			; KL NEW FRAME FLY

;;==================================================================================
0d55 21f9b7    ld      hl,$b7f9
0d58 cd7001    call    $0170			; KL DEL FRAME FLY
0d5b cd870d    call    $0d87
0d5e c38607    jp      $0786			; MC CLEAR INKS

;;---------------------------------------------------------
;; frame flyback routine for changing colours
0d61 21f8b7    ld      hl,$b7f8
0d64 35        dec     (hl)
0d65 280c      jr      z,$0d73          ; (+$0c)
0d67 2b        dec     hl
0d68 7e        ld      a,(hl)
0d69 b7        or      a
0d6a c8        ret     z

0d6b cd870d    call    $0d87
0d6e cd8c07    call    $078c			; MC SET INKS
0d71 180f      jr      $0d82            ; (+$0f)
;;==================================================================================

0d73 cd870d    call    $0d87
0d76 32f8b7    ld      ($b7f8),a
0d79 cd8c07    call    $078c			; MC SET INKS
0d7c 21f6b7    ld      hl,$b7f6
0d7f 7e        ld      a,(hl)
0d80 2f        cpl     
0d81 77        ld      (hl),a
0d82 af        xor     a
0d83 32f7b7    ld      ($b7f7),a
0d86 c9        ret     

;;===========================================================================

0d87 11e5b7    ld      de,$b7e5
0d8a 3af6b7    ld      a,($b7f6)
0d8d b7        or      a
0d8e 3ad3b7    ld      a,($b7d3)
0d91 c8        ret     z

;;===========================================================================

0d92 11d4b7    ld      de,$b7d4
0d95 3ad2b7    ld      a,($b7d2)
0d98 c9        ret     

;;---------------------------------------------------------------------------
;; table to convert from software colour number to hardware colour number
0d99 
defb &14,&04,&15,&1c,&18,&1d,&0c,&05,&0d,&16,&06,&17,&1e,&00,&1f,&0e,&07,&0f
defb &12,&02,&13,&1a,&19,&1b,&0a,&03,&0b,&01,&08,&09,&10,&11

;;============================================================================
;; SCR FILL BOX

0db9 4f        ld      c,a
0dba cd9b0b    call    $0b9b


;;============================================================================
;; SCR FLOOD BOX

0dbd e5        push    hl
0dbe 7a        ld      a,d
0dbf cdee0e    call    $0eee
0dc2 3009      jr      nc,$0dcd         ; (+$09)
0dc4 42        ld      b,d
0dc5 71        ld      (hl),c
0dc6 cd050c    call    $0c05			; SCR NEXT BYTE
0dc9 10fa      djnz    $0dc5            ; (-$06)
0dcb 1810      jr      $0ddd            ; (+$10)
0dcd c5        push    bc
0dce d5        push    de
0dcf 71        ld      (hl),c
0dd0 15        dec     d
0dd1 2808      jr      z,$0ddb          ; (+$08)
0dd3 4a        ld      c,d
0dd4 0600      ld      b,$00
0dd6 54        ld      d,h
0dd7 5d        ld      e,l
0dd8 13        inc     de
0dd9 edb0      ldir    
0ddb d1        pop     de
0ddc c1        pop     bc
0ddd e1        pop     hl
0dde cd1f0c    call    $0c1f			; SCR NEXT LINE
0de1 1d        dec     e
0de2 20d9      jr      nz,$0dbd         ; (-$27)
0de4 c9        ret     


;;============================================================================
;; SCR CHAR INVERT

0de5 78        ld      a,b
0de6 a9        xor     c
0de7 4f        ld      c,a
0de8 cd6a0b    call    $0b6a			; SCR CHAR POSITION
0deb 1608      ld      d,$08
0ded e5        push    hl
0dee c5        push    bc
0def 7e        ld      a,(hl)
0df0 a9        xor     c
0df1 77        ld      (hl),a
0df2 cd050c    call    $0c05			; SCR NEXT BYTE
0df5 10f8      djnz    $0def            ; (-$08)
0df7 c1        pop     bc
0df8 e1        pop     hl
0df9 cd1f0c    call    $0c1f			; SCR NEXT LINE
0dfc 15        dec     d
0dfd 20ee      jr      nz,$0ded         ; (-$12)
0dff c9        ret     


;;============================================================================
;; SCR HW ROLL
0e00 4f        ld      c,a
0e01 c5        push    bc
0e02 11d0ff    ld      de,$ffd0
0e05 0630      ld      b,$30
0e07 cd2a0e    call    $0e2a
0e0a c1        pop     bc
0e0b cdb407    call    $07b4			; MC WAIT FLYBACK
0e0e 78        ld      a,b
0e0f b7        or      a
0e10 200d      jr      nz,$0e1f         ; (+$0d)
0e12 11b0ff    ld      de,$ffb0
0e15 cd3d0e    call    $0e3d
0e18 110000    ld      de,$0000
0e1b 0620      ld      b,$20
0e1d 180b      jr      $0e2a            ; (+$0b)
0e1f 115000    ld      de,$0050
0e22 cd3d0e    call    $0e3d
0e25 11b0ff    ld      de,$ffb0
0e28 0620      ld      b,$20
0e2a 2ac4b7    ld      hl,($b7c4)
0e2d 19        add     hl,de
0e2e 7c        ld      a,h
0e2f e607      and     $07
0e31 67        ld      h,a
0e32 3ac6b7    ld      a,($b7c6)
0e35 84        add     a,h
0e36 67        ld      h,a
0e37 50        ld      d,b
0e38 1e08      ld      e,$08
0e3a c3bd0d    jp      $0dbd			;; SCR FLOOD BOX
0e3d 2ac4b7    ld      hl,($b7c4)
0e40 19        add     hl,de
0e41 c3370b    jp      $0b37			;; SCR OFFSET


;;============================================================================
;; SCR SW ROLL

0e44 f5        push    af
0e45 78        ld      a,b
0e46 b7        or      a
0e47 2830      jr      z,$0e79          ; (+$30)
0e49 e5        push    hl
0e4a cd9b0b    call    $0b9b
0e4d e3        ex      (sp),hl
0e4e 2c        inc     l
0e4f cd6a0b    call    $0b6a			; SCR CHAR POSITION
0e52 4a        ld      c,d
0e53 7b        ld      a,e
0e54 d608      sub     $08
0e56 47        ld      b,a
0e57 2817      jr      z,$0e70          ; (+$17)
0e59 d1        pop     de
0e5a cdb407    call    $07b4			; MC WAIT FLYBACK
0e5d c5        push    bc
0e5e e5        push    hl
0e5f d5        push    de
0e60 cdaa0e    call    $0eaa
0e63 e1        pop     hl
0e64 cd1f0c    call    $0c1f			; SCR NEXT LINE
0e67 eb        ex      de,hl
0e68 e1        pop     hl
0e69 cd1f0c    call    $0c1f			; SCR NEXT LINE
0e6c c1        pop     bc
0e6d 10ee      djnz    $0e5d            ; (-$12)
0e6f d5        push    de
0e70 e1        pop     hl
0e71 51        ld      d,c
0e72 1e08      ld      e,$08
0e74 f1        pop     af
0e75 4f        ld      c,a
0e76 c3bd0d    jp      $0dbd			;; SCR FLOOD BOX
0e79 e5        push    hl
0e7a d5        push    de
0e7b cd9b0b    call    $0b9b
0e7e 4a        ld      c,d
0e7f 7b        ld      a,e
0e80 d608      sub     $08
0e82 47        ld      b,a
0e83 d1        pop     de
0e84 e3        ex      (sp),hl
0e85 28e9      jr      z,$0e70          ; (-$17)
0e87 c5        push    bc
0e88 6b        ld      l,e
0e89 54        ld      d,h
0e8a 1c        inc     e
0e8b cd6a0b    call    $0b6a			; SCR CHAR POSITION
0e8e eb        ex      de,hl
0e8f cd6a0b    call    $0b6a			; SCR CHAR POSITION
0e92 c1        pop     bc
0e93 cdb407    call    $07b4			; MC WAIT FLYBACK
0e96 cd390c    call    $0c39			; SCR PREV LINE
0e99 e5        push    hl
0e9a eb        ex      de,hl
0e9b cd390c    call    $0c39			; SCR PREV LINE
0e9e e5        push    hl
0e9f c5        push    bc
0ea0 cdaa0e    call    $0eaa
0ea3 c1        pop     bc
0ea4 d1        pop     de
0ea5 e1        pop     hl
0ea6 10ee      djnz    $0e96            ; (-$12)
0ea8 18c6      jr      $0e70            ; (-$3a)
0eaa 0600      ld      b,$00
0eac cdec0e    call    $0eec
0eaf 3816      jr      c,$0ec7          ; (+$16)
0eb1 cdec0e    call    $0eec
0eb4 3025      jr      nc,$0edb         ; (+$25)
0eb6 c5        push    bc
0eb7 af        xor     a
0eb8 95        sub     l
0eb9 4f        ld      c,a
0eba edb0      ldir    
0ebc c1        pop     bc
0ebd 2f        cpl     
0ebe 3c        inc     a
0ebf 81        add     a,c
0ec0 4f        ld      c,a
0ec1 7c        ld      a,h
0ec2 d608      sub     $08
0ec4 67        ld      h,a
0ec5 1814      jr      $0edb            ; (+$14)
0ec7 cdec0e    call    $0eec
0eca 3812      jr      c,$0ede          ; (+$12)
0ecc c5        push    bc
0ecd af        xor     a
0ece 93        sub     e
0ecf 4f        ld      c,a
0ed0 edb0      ldir    
0ed2 c1        pop     bc
0ed3 2f        cpl     
0ed4 3c        inc     a
0ed5 81        add     a,c
0ed6 4f        ld      c,a
0ed7 7a        ld      a,d
0ed8 d608      sub     $08
0eda 57        ld      d,a
0edb edb0      ldir    
0edd c9        ret     

0ede 41        ld      b,c
0edf 7e        ld      a,(hl)
0ee0 12        ld      (de),a
0ee1 cd050c    call    $0c05			; SCR NEXT BYTE
0ee4 eb        ex      de,hl
0ee5 cd050c    call    $0c05			; SCR NEXT BYTE
0ee8 eb        ex      de,hl
0ee9 10f4      djnz    $0edf            
0eeb c9        ret     

;;============================================================================
0eec 79        ld      a,c
0eed eb        ex      de,hl
0eee 3d        dec     a
0eef 85        add     a,l
0ef0 d0        ret     nc

0ef1 7c        ld      a,h
0ef2 e607      and     $07
0ef4 ee07      xor     $07
0ef6 c0        ret     nz

0ef7 37        scf     
0ef8 c9        ret     


;;============================================================================
;; SCR UNPACK

0ef9 cd0c0b    call    $0b0c			;; SCR GET MODE 
0efc 380d      jr      c,$0f0b          ; mode 0
0efe 2806      jr      z,$0f06          ; mode 1
0f00 010800    ld      bc,$0008
0f03 edb0      ldir    
0f05 c9        ret     

;;-----------------------------------------------------------------------------
;; SCR UNPACK: mode 1
0f06 018802    ld      bc,$0288
0f09 1803      jr      $0f0e            ; 0x088 is the pixel mask

;;-----------------------------------------------------------------------------
;; SCR UNPACK: mode 0
0f0b 01aa04    ld      bc,$04aa         ;; 0x0aa is the pixel mask

;;-----------------------------------------------------------------------------
;; routine used by mode 0 and mode 1 for SCR UNPACK
0f0e 3e08      ld      a,$08
0f10 f5        push    af
0f11 e5        push    hl
0f12 6e        ld      l,(hl)
0f13 60        ld      h,b
0f14 af        xor     a
0f15 cb05      rlc     l
0f17 3001      jr      nc,$0f1a         ; (+$01)
0f19 b1        or      c
0f1a cb09      rrc     c
0f1c 30f7      jr      nc,$0f15         ; (-$09)
0f1e 12        ld      (de),a
0f1f 13        inc     de
0f20 10f2      djnz    $0f14            ; (-$0e)
0f22 44        ld      b,h
0f23 e1        pop     hl
0f24 23        inc     hl
0f25 f1        pop     af
0f26 3d        dec     a
0f27 20e7      jr      nz,$0f10         ; (-$19)
0f29 c9        ret     


;;============================================================================
;; SCR REPACK

0f2a 4f        ld      c,a
0f2b cd6a0b    call    $0b6a			; SCR CHAR POSITION
0f2e cd0c0b    call    $0b0c			; SCR GET MODE
0f31 0608      ld      b,$08
0f33 3836      jr      c,$0f6b          ; mode 0
0f35 280b      jr      z,$0f42          ; mode 1

;;----------------------------------------------------------------------------------------
;; SCR REPACK: mode 2
0f37 7e        ld      a,(hl)
0f38 a9        xor     c
0f39 2f        cpl     
0f3a 12        ld      (de),a
0f3b 13        inc     de
0f3c cd1f0c    call    $0c1f			; SCR NEXT LINE
0f3f 10f6      djnz    $0f37            
0f41 c9        ret     

;;----------------------------------------------------------------------------------------
;; SCR REPACK: mode 1
0f42 c5        push    bc
0f43 e5        push    hl
0f44 d5        push    de
0f45 cd5a0f    call    $0f5a      ; mode 1
0f48 cd050c    call    $0c05			; SCR NEXT BYTE
0f4b cd5a0f    call    $0f5a      ; mode 1
0f4e 7b        ld      a,e
0f4f d1        pop     de
0f50 12        ld      (de),a
0f51 13        inc     de
0f52 e1        pop     hl
0f53 cd1f0c    call    $0c1f			; SCR NEXT LINE
0f56 c1        pop     bc
0f57 10e9      djnz    $0f42            
0f59 c9        ret     

;;----------------------------------------------------------------------------------------
;; SCR REPACK: mode 1 (part)
0f5a 1688      ld      d,$88        ; pixel mask
0f5c 0604      ld      b,$04
0f5e 7e        ld      a,(hl)
0f5f a9        xor     c
0f60 a2        and     d
0f61 2001      jr      nz,$0f64         ; (+$01)
0f63 37        scf     
0f64 cb13      rl      e
0f66 cb0a      rrc     d
0f68 10f4      djnz    $0f5e            
0f6a c9        ret     

;;----------------------------------------------------------------------------------------
;; SCR REPACK: mode 0
0f6b c5        push    bc
0f6c e5        push    hl
0f6d d5        push    de

0f6e 0604      ld      b,$04
0f70 7e        ld      a,(hl)
0f71 a9        xor     c
0f72 e6aa      and     $aa            ; left pixel mask
0f74 2001      jr      nz,$0f77        
0f76 37        scf     
0f77 cb13      rl      e
0f79 7e        ld      a,(hl)
0f7a a9        xor     c
0f7b e655      and     $55            ; right pixel mask
0f7d 2001      jr      nz,$0f80        
0f7f 37        scf     
0f80 cb13      rl      e
0f82 cd050c    call    $0c05			; SCR NEXT BYTE
0f85 10e9      djnz    $0f70            

0f87 7b        ld      a,e
0f88 d1        pop     de
0f89 12        ld      (de),a
0f8a 13        inc     de
0f8b e1        pop     hl
0f8c cd1f0c    call    $0c1f			; SCR NEXT LINE
0f8f c1        pop     bc
0f90 10d9      djnz    $0f6b           
0f92 c9        ret     


;;============================================================================
;; SOUND ARM EVENT
0f93 cdad0f    call    $0fad
0f96 cdc20f    call    $0fc2
0f99 1806      jr      $0fa1            ; (+$06)


;;============================================================================
;; SCR VERTICAL

0f9b cdad0f    call    $0fad
0f9e cd1610    call    $1016
0fa1 2a02b8    ld      hl,($b802)
0fa4 7d        ld      a,l
0fa5 32a3b6    ld      ($b6a3),a		; graphics pen
0fa8 7c        ld      a,h
0fa9 32b3b6    ld      ($b6b3),a		; graphics line mask
0fac c9        ret     

;;============================================================================

0fad e5        push    hl
0fae 2aa3b6    ld      hl,($b6a3)		; L = graphics pen, H = graphics paper
0fb1 32a3b6    ld      ($b6a3),a		; graphics pen
0fb4 3ab3b6    ld      a,($b6b3)		; graphics line mask
0fb7 67        ld      h,a
0fb8 3eff      ld      a,$ff
0fba 32b3b6    ld      ($b6b3),a		; graphics line mask
0fbd 2202b8    ld      ($b802),hl
0fc0 e1        pop     hl
0fc1 c9        ret     

0fc2 37        scf     
0fc3 cd3b10    call    $103b
0fc6 cb00      rlc     b
0fc8 79        ld      a,c
0fc9 3013      jr      nc,$0fde         ; (+$13)
0fcb 1d        dec     e
0fcc 2003      jr      nz,$0fd1         ; (+$03)
0fce 15        dec     d
0fcf 282c      jr      z,$0ffd          ; (+$2c)
0fd1 cb09      rrc     c
0fd3 3828      jr      c,$0ffd          ; (+$28)
0fd5 cb78      bit     7,b
0fd7 2824      jr      z,$0ffd          ; (+$24)
0fd9 b1        or      c
0fda cb00      rlc     b
0fdc 18ed      jr      $0fcb            ; (-$13)
0fde 1d        dec     e
0fdf 2003      jr      nz,$0fe4         ; (+$03)
0fe1 15        dec     d
0fe2 280d      jr      z,$0ff1          ; (+$0d)
0fe4 cb09      rrc     c
0fe6 3809      jr      c,$0ff1          ; (+$09)
0fe8 cb78      bit     7,b
0fea 2005      jr      nz,$0ff1         ; (+$05)
0fec b1        or      c
0fed cb00      rlc     b
0fef 18ed      jr      $0fde            ; (-$13)
0ff1 c5        push    bc
0ff2 4f        ld      c,a
0ff3 3aa4b6    ld      a,($b6a4)		; graphics paper
0ff6 47        ld      b,a
0ff7 3ab4b6    ld      a,($b6b4)
0ffa b7        or      a
0ffb 1807      jr      $1004            ; (+$07)
0ffd c5        push    bc
0ffe 4f        ld      c,a
0fff 3aa3b6    ld      a,($b6a3)		; graphics pen
1002 47        ld      b,a
1003 af        xor     a
1004 cce8bd    call    z,$bde8			; IND: SCR WRITE
1007 c1        pop     bc
1008 cb79      bit     7,c
100a c4050c    call    nz,$0c05			; SCR NEXT BYTE
100d 7a        ld      a,d
100e b3        or      e
100f 20b5      jr      nz,$0fc6         ; (-$4b)
1011 78        ld      a,b
1012 32b3b6    ld      ($b6b3),a		; graphics line mask
1015 c9        ret     

1016 b7        or      a
1017 cd3b10    call    $103b
101a cb00      rlc     b
101c 3aa3b6    ld      a,($b6a3)		; graphics pen
101f 3809      jr      c,$102a          ; (+$09)
1021 3ab4b6    ld      a,($b6b4)
1024 b7        or      a
1025 2009      jr      nz,$1030         ; (+$09)
1027 3aa4b6    ld      a,($b6a4)		; graphics paper
102a c5        push    bc
102b 47        ld      b,a
102c cde8bd    call    $bde8			; IND: SCR WRITE
102f c1        pop     bc
1030 cd390c    call    $0c39			; SCR PREV LINE
1033 1d        dec     e
1034 20e4      jr      nz,$101a         ; (-$1c)
1036 15        dec     d
1037 20e1      jr      nz,$101a         ; (-$1f)
1039 18d6      jr      $1011            ; (-$2a)
103b e5        push    hl
103c 3002      jr      nc,$1040         ; (+$02)
103e 62        ld      h,d
103f 6b        ld      l,e
1040 b7        or      a
1041 ed42      sbc     hl,bc
1043 cd3919    call    $1939			; HL = -HL
1046 24        inc     h
1047 2c        inc     l
1048 e3        ex      (sp),hl
1049 cdaf0b    call    $0baf			; SCR DOT POSITION
104c 3ab3b6    ld      a,($b6b3)		; graphics line mask
104f 47        ld      b,a
1050 d1        pop     de
1051 c9        ret     

;;---------------------------------------------------------------------------
;; default colour palette
;; uses hardware colour numbers
;; 
;; There are two palettes here; so that flashing colours can be defined.
1052 
defb &04,&04,&0a,&13,&0c,&0b,&14,&15,&0d,&06,&1e,&1f,&07,&12,&19,&04,&17
defb &04,&04,&0a,&13,&0c,&0b,&14,&15,&0d,&06,&1e,&1f,&07,&12,&19,&0a,&07

;;===========================================================================
;; TXT INITIALISE

1074 cd8410    call    $1084			;; TXT RESET
1077 af        xor     a
1078 3235b7    ld      ($b735),a
107b 210100    ld      hl,$0001
107e cd3911    call    $1139
1081 c39f10    jp      $109f

;;===========================================================================
;; TXT RESET

1084 218d10    ld      hl,$108d			;; table used to initialise text vdu indirections
1087 cdb40a    call    $0ab4			;; initialise text vdu indirections
108a c36414    jp      $1464			;; initialise control code handler functions

108d 
defb &f
defw &bdcd
1090 c35f12 jp		$125f							;; IND: TXT DRAW CURSOR
1093 c35f12 jp      $125f							;; IND: TXT UNDRAW CURSOR
1096 c34b13 jp      $134b							;; IND: TXT WRITE CHAR
1099 c3be13 jp      $13be							;; IND: TXT UNWRITE
109c c30a14 jp      $140a							;; IND: TXT OUT ACTION


;;===========================================================================

109f 3e08      ld      a,$08
10a1 11b6b6    ld      de,$b6b6
10a4 2126b7    ld      hl,$b726
10a7 010e00    ld      bc,$000e
10aa edb0      ldir    
10ac 3d        dec     a
10ad 20f5      jr      nz,$10a4         ; (-$0b)
10af 32b5b6    ld      ($b6b5),a
10b2 c9        ret     
;;==================================================================================

10b3 3ab5b6    ld      a,($b6b5)
10b6 4f        ld      c,a
10b7 0608      ld      b,$08

10b9 78        ld      a,b
10ba 3d        dec     a
10bb cde410    call    $10e4			; TXT STR SELECT
10be cdd0bd    call    $bdd0			; IND: TXT UNDRAW CURSOR
10c1 cdc012    call    $12c0			; TXT GET PAPER
10c4 3230b7    ld      ($b730),a
10c7 cdba12    call    $12ba			; TXT GET PEN
10ca 322fb7    ld      ($b72f),a
10cd 10ea      djnz    $10b9            ; (-$16)
10cf 79        ld      a,c
10d0 c9        ret     

;;==================================================================================
10d1 4f        ld      c,a
10d2 0608      ld      b,$08
10d4 78        ld      a,b
10d5 3d        dec     a
10d6 cde410    call    $10e4			; TXT STR SELECT
10d9 c5        push    bc
10da 2a2fb7    ld      hl,($b72f)
10dd cd3911    call    $1139
10e0 c1        pop     bc
10e1 10f1      djnz    $10d4            ; (-$0f)
10e3 79        ld      a,c

;;==================================================================================
;; TXT STR SELECT
10e4 e607      and     $07
10e6 21b5b6    ld      hl,$b6b5
10e9 be        cp      (hl)
10ea c8        ret     z

10eb c5        push    bc
10ec d5        push    de
10ed 4e        ld      c,(hl)
10ee 77        ld      (hl),a
10ef 47        ld      b,a
10f0 79        ld      a,c
10f1 cd2611    call    $1126
10f4 cd1e11    call    $111e
10f7 78        ld      a,b
10f8 cd2611    call    $1126
10fb eb        ex      de,hl
10fc cd1e11    call    $111e
10ff 79        ld      a,c
1100 d1        pop     de
1101 c1        pop     bc
1102 c9        ret     

;;===========================================================================
;; TXT SWAP STREAMS
1103 3ab5b6    ld      a,($b6b5)
1106 f5        push    af
1107 79        ld      a,c
1108 cde410    call    $10e4
110b 78        ld      a,b
110c 32b5b6    ld      ($b6b5),a
110f cd2611    call    $1126
1112 d5        push    de
1113 79        ld      a,c
1114 cd2611    call    $1126
1117 e1        pop     hl
1118 cd1e11    call    $111e
111b f1        pop     af
111c 18c6      jr      $10e4            ; (-$3a)
;;===========================================================================
111e c5        push    bc
111f 010e00    ld      bc,$000e
1122 edb0      ldir    
1124 c1        pop     bc
1125 c9        ret     

;;===========================================================================
1126 e607      and     $07
1128 5f        ld      e,a
1129 87        add     a,a
112a 83        add     a,e
112b 87        add     a,a
112c 83        add     a,e
112d 87        add     a,a
112e c6b6      add     a,$b6
1130 5f        ld      e,a
1131 ceb6      adc     a,$b6
1133 93        sub     e
1134 57        ld      d,a
1135 2126b7    ld      hl,$b726
1138 c9        ret     

;;===========================================================================
1139 eb        ex      de,hl
113a 3e83      ld      a,$83
113c 322eb7    ld      ($b72e),a
113f 7a        ld      a,d
1140 cdab12    call    $12ab			; TXT SET PAPER
1143 7b        ld      a,e
1144 cda612    call    $12a6			; TXT SET PEN
1147 af        xor     a
1148 cda813    call    $13a8			; TXT SET GRAPHIC
114b cd7b13    call    $137b			; TXT SET BACK
114e 210000    ld      hl,$0000
1151 117f7f    ld      de,$7f7f
1154 cd0812    call    $1208			; TXT WIN ENABLE
1157 c35914    jp      $1459			; TXT VDU ENABLE

;;===========================================================================
;; TXT SET COLUMN

115a 3d        dec     a
115b 212ab7    ld      hl,$b72a
115e 86        add     a,(hl)
115f 2a26b7    ld      hl,($b726)
1162 67        ld      h,a
1163 180e      jr      $1173            ;; undraw cursor, set cursor position and draw it

;;===========================================================================
;; TXT SET ROW

1165 3d        dec     a
1166 2129b7    ld      hl,$b729
1169 86        add     a,(hl)
116a 2a26b7    ld      hl,($b726)
116d 6f        ld      l,a
116e 1803      jr      $1173            ;; undraw cursor, set cursor position and draw it

;;===========================================================================
;; TXT SET CURSOR

1170 cd8611    call    $1186

;; undraw cursor, set cursor position and draw it
1173 cdd0bd    call    $bdd0			; IND: TXT UNDRAW CURSOR

;; set cursor position and draw it
1176 2226b7    ld      ($b726),hl
1179 c3cdbd    jp      $bdcd			; IND: TXT DRAW CURSOR

;;===========================================================================
;; TXT GET CURSOR

117c 2a26b7    ld      hl,($b726)
117f cd9311    call    $1193
1182 3a2db7    ld      a,($b72d)
1185 c9        ret     

;;===========================================================================
1186 3a29b7    ld      a,($b729)
1189 3d        dec     a
118a 85        add     a,l
118b 6f        ld      l,a
118c 3a2ab7    ld      a,($b72a)
118f 3d        dec     a
1190 84        add     a,h
1191 67        ld      h,a
1192 c9        ret     

;;====================================================================
1193 3a29b7    ld      a,($b729)
1196 95        sub     l
1197 2f        cpl     
1198 3c        inc     a
1199 3c        inc     a
119a 6f        ld      l,a
119b 3a2ab7    ld      a,($b72a)
119e 94        sub     h
119f 2f        cpl     
11a0 3c        inc     a
11a1 3c        inc     a
11a2 67        ld      h,a
11a3 c9        ret     

;;====================================================================
11a4 cdd0bd    call    $bdd0				;; IND: TXT UNDRAW CURSOR

;;--------------------------------------------------------------------
11a7 2a26b7    ld      hl,($b726)
11aa cdd611    call    $11d6
11ad 2226b7    ld      ($b726),hl
11b0 d8        ret     c

11b1 e5        push    hl
11b2 212db7    ld      hl,$b72d
11b5 78        ld      a,b
11b6 87        add     a,a
11b7 3c        inc     a
11b8 86        add     a,(hl)
11b9 77        ld      (hl),a
11ba cd5212    call    $1252				;; TXT GET WINDOW
11bd 3a30b7    ld      a,($b730)
11c0 f5        push    af
11c1 dc440e    call    c,$0e44				;; SCR SW ROLL
11c4 f1        pop     af
11c5 d4000e    call    nc,$0e00				;; SCR HW ROLL
11c8 e1        pop     hl
11c9 c9        ret     


;;===========================================================================
;; TXT VALIDATE

11ca cd8611    call    $1186
11cd cdd611    call    $11d6
11d0 f5        push    af
11d1 cd9311    call    $1193
11d4 f1        pop     af
11d5 c9        ret     
;;===========================================================================
11d6 3a2cb7    ld      a,($b72c)
11d9 bc        cp      h
11da f2e211    jp      p,$11e2
11dd 3a2ab7    ld      a,($b72a)
11e0 67        ld      h,a
11e1 2c        inc     l
11e2 3a2ab7    ld      a,($b72a)
11e5 3d        dec     a
11e6 bc        cp      h
11e7 faef11    jp      m,$11ef
11ea 3a2cb7    ld      a,($b72c)
11ed 67        ld      h,a
11ee 2d        dec     l
11ef 3a29b7    ld      a,($b729)
11f2 3d        dec     a
11f3 bd        cp      l
11f4 f20212    jp      p,$1202
11f7 3a2bb7    ld      a,($b72b)
11fa bd        cp      l
11fb 37        scf     
11fc f0        ret     p

11fd 6f        ld      l,a
11fe 06ff      ld      b,$ff
1200 b7        or      a
1201 c9        ret     

;;===========================================================================
1202 3c        inc     a
1203 6f        ld      l,a
1204 0600      ld      b,$00
1206 b7        or      a
1207 c9        ret     

;;===========================================================================
;; TXT WIN ENABLE

1208 cd5d0b    call    $0b5d			;; SCR CHAR LIMITS
120b 7c        ld      a,h
120c cd4012    call    $1240
120f 67        ld      h,a
1210 7a        ld      a,d
1211 cd4012    call    $1240
1214 57        ld      d,a
1215 bc        cp      h
1216 3002      jr      nc,$121a         ; (+$02)
1218 54        ld      d,h
1219 67        ld      h,a
121a 7d        ld      a,l
121b cd4912    call    $1249
121e 6f        ld      l,a
121f 7b        ld      a,e
1220 cd4912    call    $1249
1223 5f        ld      e,a
1224 bd        cp      l
1225 3002      jr      nc,$1229         ; (+$02)
1227 5d        ld      e,l
1228 6f        ld      l,a
1229 2229b7    ld      ($b729),hl
122c ed532bb7  ld      ($b72b),de
1230 7c        ld      a,h
1231 b5        or      l
1232 2006      jr      nz,$123a         ; (+$06)
1234 7a        ld      a,d
1235 a8        xor     b
1236 2002      jr      nz,$123a         ; (+$02)
1238 7b        ld      a,e
1239 a9        xor     c
123a 3228b7    ld      ($b728),a
123d c37311    jp      $1173			;; undraw cursor, set cursor position and draw it

;;===========================================================================
1240 b7        or      a
1241 f24512    jp      p,$1245
1244 af        xor     a
1245 b8        cp      b
1246 d8        ret     c

1247 78        ld      a,b
1248 c9        ret     

1249 b7        or      a
124a f24e12    jp      p,$124e
124d af        xor     a
124e b9        cp      c
124f d8        ret     c

1250 79        ld      a,c
1251 c9        ret     

;;===========================================================================
;; TXT GET WINDOW

1252 2a29b7    ld      hl,($b729)
1255 ed5b2bb7  ld      de,($b72b)
1259 3a28b7    ld      a,($b728)
125c c6ff      add     a,$ff
125e c9        ret     

;;===========================================================================
;; IND: TXT UNDRAW CURSOR
125f 3a2eb7    ld      a,($b72e)
1262 e603      and     $03
1264 c0        ret     nz

;;===========================================================================
;; TXT PLACE CURSOR
;; TXT REMOVE CURSOR

1265 c5        push    bc
1266 d5        push    de
1267 e5        push    hl
1268 cda711    call    $11a7
126b ed4b2fb7  ld      bc,($b72f)
126f cde50d    call    $0de5				;; SCR CHAR INVERT
1272 e1        pop     hl
1273 d1        pop     de
1274 c1        pop     bc
1275 c9        ret     

;;===========================================================================
;; TXT CUR ON

1276 f5        push    af
1277 3efd      ld      a,$fd
1279 cd8812    call    $1288
127c f1        pop     af
127d c9        ret     

;;===========================================================================
;; TXT CUR OFF

127e f5        push    af
127f 3e02      ld      a,$02
1281 cd9912    call    $1299
1284 f1        pop     af
1285 c9        ret     

;;===========================================================================
;; TXT CUR ENABLE

1286 3efe      ld      a,$fe
;;---------------------------------------------------------------------------
1288 f5        push    af
1289 cdd0bd    call    $bdd0				;; IND: TXT UNDRAW CURSOR
128c f1        pop     af
128d e5        push    hl
128e 212eb7    ld      hl,$b72e
1291 a6        and     (hl)
1292 77        ld      (hl),a
1293 e1        pop     hl
1294 c3cdbd    jp      $bdcd				;; IND: TXT DRAW CURSOR

;;===========================================================================
;; TXT CUR DISABLE

1297 3e01      ld      a,$01
;;---------------------------------------------------------------------------
1299 f5        push    af
129a cdd0bd    call    $bdd0				;; IND: TXT UNDRAW CURSOR
129d f1        pop     af
129e e5        push    hl
129f 212eb7    ld      hl,$b72e
12a2 b6        or      (hl)
12a3 77        ld      (hl),a
12a4 e1        pop     hl
12a5 c9        ret     

;;===========================================================================
;; TXT SET PEN 
12a6 212fb7    ld      hl,$b72f
12a9 1803      jr      $12ae            ; (+$03)

;;===========================================================================
;; TXT SET PAPER
12ab 2130b7    ld      hl,$b730
;;---------------------------------------------------------------------------
12ae f5        push    af
12af cdd0bd    call    $bdd0				;; IND: TXT UNDRAW CURSOR
12b2 f1        pop     af
12b3 cd8e0c    call    $0c8e				;; SCR INK ENCODE
12b6 77        ld      (hl),a
12b7 c3cdbd    jp      $bdcd				;; IND: TXT DRAW CURSOR

;;===========================================================================
;; TXT GET PEN
12ba 3a2fb7    ld      a,($b72f)
12bd c3a70c    jp      $0ca7			; SCR INK DECODE

;;===========================================================================
;; TXT GET PAPER
12c0 3a30b7    ld      a,($b730)
12c3 c3a70c    jp      $0ca7			; SCR INK DECODE

;;===========================================================================
;; TXT INVERSE
12c6 cdd0bd    call    $bdd0				;; IND: TXT UNDRAW CURSOR
12c9 2a2fb7    ld      hl,($b72f)
12cc 7c        ld      a,h
12cd 65        ld      h,l
12ce 6f        ld      l,a
12cf 222fb7    ld      ($b72f),hl
12d2 18e3      jr      $12b7            ; (-$1d)

;;===========================================================================
;; TXT GET MATRIX
12d4 d5        push    de
12d5 5f        ld      e,a
12d6 cd2b13    call    $132b			; TXT GET M TABLE
12d9 3009      jr      nc,$12e4         ; get pointer to character graphics
12db 57        ld      d,a
12dc 7b        ld      a,e
12dd 92        sub     d
12de 3f        ccf     
12df 3003      jr      nc,$12e4         ; get pointer to character graphics
12e1 5f        ld      e,a
12e2 1803      jr      $12e7            ; (+$03)

;;-------------------------------------------------------------------
;; get pointer to graphics for character in font
;;
;; Entry conditions:
;; A = character code
;; Exit conditions:
;; HL = pointer to graphics for character

12e4 210038    ld      hl,$3800			; font graphics
12e7 f5        push    af
12e8 1600      ld      d,$00
12ea eb        ex      de,hl
12eb 29        add     hl,hl			; x2
12ec 29        add     hl,hl			; x4
12ed 29        add     hl,hl			; x8
12ee 19        add     hl,de
12ef f1        pop     af
12f0 d1        pop     de
12f1 c9        ret     

;;===========================================================================
;; TXT SET MATRIX
12f2 eb        ex      de,hl
12f3 cdd412    call    $12d4			; TXT GET MATRIX
12f6 d0        ret     nc

12f7 eb        ex      de,hl

;;---------------------------------------------------------------------------
12f8 010800    ld      bc,$0008
12fb edb0      ldir    
12fd c9        ret     

;;===========================================================================
;; TXT SET M TABLE
12fe e5        push    hl
12ff 7a        ld      a,d
1300 b7        or      a
1301 1600      ld      d,$00
1303 2019      jr      nz,$131e         ; (+$19)
1305 15        dec     d
1306 d5        push    de
1307 4b        ld      c,e
1308 eb        ex      de,hl
1309 79        ld      a,c
130a cdd412    call    $12d4			; TXT GET MATRIX
130d 7c        ld      a,h
130e aa        xor     d
130f 2004      jr      nz,$1315         ; (+$04)
1311 7d        ld      a,l
1312 ab        xor     e
1313 2808      jr      z,$131d          ; (+$08)
1315 c5        push    bc
1316 cdf812    call    $12f8
1319 c1        pop     bc
131a 0c        inc     c
131b 20ec      jr      nz,$1309         ; (-$14)
131d d1        pop     de
131e cd2b13    call    $132b			; TXT GET M TABLE
1321 ed5334b7  ld      ($b734),de
1325 d1        pop     de
1326 ed5336b7  ld      ($b736),de
132a c9        ret     

;;===========================================================================
;; TXT GET M TABLE
132b 2a34b7    ld      hl,($b734)
132e 7c        ld      a,h
132f 0f        rrca    
1330 7d        ld      a,l
1331 2a36b7    ld      hl,($b736)
1334 c9        ret     

;;===========================================================================
;; TXT WR CHAR

1335 47        ld      b,a
1336 3a2eb7    ld      a,($b72e)
1339 07        rlca    
133a d8        ret     c

133b c5        push    bc
133c cda411    call    $11a4
133f 24        inc     h
1340 2226b7    ld      ($b726),hl
1343 25        dec     h
1344 f1        pop     af
1345 cdd3bd    call    $bdd3				;; IND: TXT WRITE CURSOR
1348 c3cdbd    jp      $bdcd				;; IND: TXT DRAW CURSOR

;;===========================================================================
;; IND: TXT WRITE CHAR
134b e5        push    hl
134c cdd412    call    $12d4			; TXT GET MATRIX
134f 1138b7    ld      de,$b738
1352 d5        push    de
1353 cdf90e    call    $0ef9			; SCR UNPACK
1356 d1        pop     de
1357 e1        pop     hl
1358 cd6a0b    call    $0b6a			; SCR CHAR POSITION
135b 0e08      ld      c,$08
135d c5        push    bc
135e e5        push    hl
135f c5        push    bc
1360 d5        push    de
1361 eb        ex      de,hl
1362 4e        ld      c,(hl)
1363 cd7713    call    $1377
1366 cd050c    call    $0c05			; SCR NEXT BYTE
1369 d1        pop     de
136a 13        inc     de
136b c1        pop     bc
136c 10f1      djnz    $135f            ; (-$0f)
136e e1        pop     hl
136f cd1f0c    call    $0c1f			; SCR NEXT LINE
1372 c1        pop     bc
1373 0d        dec     c
1374 20e7      jr      nz,$135d         ; (-$19)
1376 c9        ret     

;;===========================================================================
1377 2a31b7    ld      hl,($b731)
137a e9        jp      (hl)
;;===========================================================================
;; TXT SET BACK
137b 219213    ld      hl,$1392
137e b7        or      a
137f 2803      jr      z,$1384          ; (+$03)
1381 21a013    ld      hl,$13a0
1384 2231b7    ld      ($b731),hl
1387 c9        ret     

;;===========================================================================
;; TXT GET BACK
1388 2a31b7    ld      hl,($b731)
138b 116eec    ld      de,$ec6e
138e 19        add     hl,de
138f 7c        ld      a,h
1390 b5        or      l
1391 c9        ret     
;;===========================================================================

1392 2a2fb7    ld      hl,($b72f)
1395 79        ld      a,c
1396 2f        cpl     
1397 a4        and     h
1398 47        ld      b,a
1399 79        ld      a,c
139a a5        and     l
139b b0        or      b
139c 0eff      ld      c,$ff
139e 1803      jr      $13a3            ; (+$03)

;;===========================================================================
13a0 3a2fb7    ld      a,($b72f)
;;---------------------------------------------------------------------------
13a3 47        ld      b,a
13a4 eb        ex      de,hl
13a5 c3740c    jp      $0c74			; SCR PIXELS

;;===========================================================================
;; TXT SET GRAPHIC

13a8 3233b7    ld      ($b733),a
13ab c9        ret     

;;===========================================================================
;; TXT RD CHAR

13ac e5        push    hl
13ad d5        push    de
13ae c5        push    bc
13af cda411    call    $11a4
13b2 cdd6bd    call    $bdd6			; IND: TXT UNWRITE
13b5 f5        push    af
13b6 cdcdbd    call    $bdcd			; IND: TXT DRAW CURSOR
13b9 f1        pop     af
13ba c1        pop     bc
13bb d1        pop     de
13bc e1        pop     hl
13bd c9        ret     

;;===========================================================================
;; IND: TXT UNWRITE

13be 3a30b7    ld      a,($b730)
13c1 1138b7    ld      de,$b738
13c4 e5        push    hl
13c5 d5        push    de
13c6 cd2a0f    call    $0f2a			; SCR REPACK
13c9 d1        pop     de
13ca d5        push    de
13cb 0608      ld      b,$08
13cd 1a        ld      a,(de)
13ce 2f        cpl     
13cf 12        ld      (de),a
13d0 13        inc     de
13d1 10fa      djnz    $13cd            ; (-$06)
13d3 cde113    call    $13e1
13d6 d1        pop     de
13d7 e1        pop     hl
13d8 3001      jr      nc,$13db         ; (+$01)
13da c0        ret     nz

13db 3a2fb7    ld      a,($b72f)
13de cd2a0f    call    $0f2a			; SCR REPACK
13e1 0e00      ld      c,$00
13e3 79        ld      a,c
13e4 cdd412    call    $12d4			; TXT GET MATRIX
13e7 1138b7    ld      de,$b738
13ea 0608      ld      b,$08
13ec 1a        ld      a,(de)
13ed be        cp      (hl)
13ee 2009      jr      nz,$13f9         ; (+$09)
13f0 23        inc     hl
13f1 13        inc     de
13f2 10f8      djnz    $13ec            ; (-$08)
13f4 79        ld      a,c
13f5 fe8f      cp      $8f
13f7 37        scf     
13f8 c9        ret     

13f9 0c        inc     c
13fa 20e7      jr      nz,$13e3         ; (-$19)
13fc af        xor     a
13fd c9        ret     

;;===========================================================================
;; TXT OUTPUT

13fe f5        push    af
13ff c5        push    bc
1400 d5        push    de
1401 e5        push    hl
1402 cdd9bd    call    $bdd9			; IND: TXT OUT ACTION
1405 e1        pop     hl
1406 d1        pop     de
1407 c1        pop     bc
1408 f1        pop     af
1409 c9        ret     

;;===========================================================================
;; IND: TXT OUT ACTION

140a 4f        ld      c,a
140b 3a33b7    ld      a,($b733)
140e b7        or      a
140f 79        ld      a,c
1410 c24019    jp      nz,$1940			; GRA WR CHAR

1413 2158b7    ld      hl,$b758
1416 46        ld      b,(hl)
1417 78        ld      a,b
1418 fe0a      cp      $0a
141a 3031      jr      nc,$144d         ; (+$31)
141c b7        or      a
141d 2006      jr      nz,$1425         ; (+$06)
141f 79        ld      a,c
1420 fe20      cp      $20
1422 d23513    jp      nc,$1335			; TXT WR CHAR
1425 04        inc     b
1426 70        ld      (hl),b
1427 58        ld      e,b
1428 1600      ld      d,$00
142a 19        add     hl,de
142b 71        ld      (hl),c


;; b759 = control code character
142c 3a59b7    ld      a,($b759)
142f 5f        ld      e,a

;; start of control code table in RAM
;; each entry is 3 bytes
1430 2163b7    ld      hl,$b763
;; this effectively multiplies E by 3
;; and adds it onto the base address of the table

1433 19        add     hl,de
1434 19        add     hl,de
1435 19        add     hl,de		;; 3 bytes per entry

1436 7e        ld      a,(hl)
1437 e60f      and     $0f
1439 b8        cp      b
143a d0        ret     nc

143b 3a2eb7    ld      a,($b72e)
143e a6        and     (hl)
143f 07        rlca    
1440 380b      jr      c,$144d          ; (+$0b)

1442 23        inc     hl
1443 5e        ld      e,(hl)			;; function to execute
1444 23        inc     hl
1445 56        ld      d,(hl)
1446 2159b7    ld      hl,$b759
1449 79        ld      a,c
144a cd1600    call    $0016			; LOW: PCDE INSTRUCTION
144d af        xor     a
144e 3258b7    ld      ($b758),a
1451 c9        ret     

;;===========================================================================
;; TXT VDU DISABLE

1452 3e81      ld      a,$81
1454 cd9912    call    $1299
1457 18f4      jr      $144d            ; (-$0c)

;;===========================================================================
;; TXT VDU ENABLE

1459 3e7e      ld      a,$7e
145b cd8812    call    $1288
145e 18ed      jr      $144d            ; (-$13)

;;===========================================================================
;; TXT ASK STATE

1460 3a2eb7    ld      a,($b72e)
1463 c9        ret     

;;===========================================================================
;; initialise control code functions
1464 af        xor     a
1465 3258b7    ld      ($b758),a

1468 217414    ld      hl,$1474
146b 1163b7    ld      de,$b763
146e 016000    ld      bc,$0060
1471 edb0      ldir    
1473 c9        ret     
;;===========================================================================

;; control code handler functions
;; (see SOFT968	for a description of the control character operations)

;; byte 0: bits 3..0: number of parameters expected
;; byte 1,2: handler function

1474
defb &80
defw &1513				;; NUL:
defb &81
defw &1335				;; SOH: firmware function: TXT WR CHAR
defb &80
defw &1297				;; STX: firmware function: TXT CUR DISABLE
defb &80
defw &1286				;; ETX: firmware function: TXT CUR ENABLE
defb &81
defw &0ae9				;; EOT: firmware function: SCR SET MODE
defb &81
defw &1940				;; ENQ: firmware function: GRA WR CHAR
defb &00
defw &1459				;; ACK: firmware function: TXT VDU ENABLE
defb &80
defw &14e1				;; BEL:
defb &80
defw &1519				;; BS:
defb &80
defw &151e				;; TAB:
defb &80
defw &1523				;; LF:
defb &80
defw &1528				;; VT:
defb &80
defw &154f				;; FF: firmware function: TXT CLEAR WINDOW
defb &80
defw &153f				;; CR:
defb &81
defw &12ab				;; SO: firmware function: TXT SET PAPER
defb &81
defw &12a6				;; SI: firmware function: TXT SET PEN
defb &80
defw &155e				;; DLE:
defb &80
defw &1599				;; DC1:
defb &80
defw &158f				;; DC2:
defb &80
defw &1578				;; DC3:
defb &80
defw &1565				;; DC4:
defb &80
defw &1452				;; NAK: firmware function: TXT VDU DISABLE
defb &81
defw &14ec				;; SYN:
defb &81
defw &0c55				;; ETB: firmware function: SCR ACCESS
defb &80
defw &12c6				;; CAN: firmware function: TXT INVERSE
defb &89
defw &150d				;; EM:
defb &84
defw &1501				;; SUB:
defb &00
defw &14eb				;; ESC
defb &83
defw &14f1				;; FS:
defb &82
defw &14fa				;; GS:
defb &80
defw &1539				;; RS:
defb &82
defw &1547				;; US:

;; =============================================================================
;; TXT GET CONTROLS
14d4 2163b7    ld      hl,$b763
14d7 c9        ret     

;; =============================================================================
;; data for control character 'BEL' sound
14d8 
defb &87 ;; channel status byte
defb &00 ;; volume envelope to use
defb &00 ;; tone envelope to use
defb &5a ;; tone period low
defb &00 ;; tone period high
defb &00 ;; noise period
defb &0b ;; start volume
defb &14 ;; envelope repeat count low
defb &00 ;; envelope repeat count high

;; =============================================================================
;; performs control character 'BEL' function
14e1 dde5      push    ix
14e3 21d814    ld      hl,$14d8			; 
14e6 cd1421    call    $2114			; SOUND QUEUE
14e9 dde1      pop     ix

;; performs control character 'ESC' function
14eb c9        ret     
;; =============================================================================
;; performs control character 'SYN' function
14ec 0f        rrca    
14ed 9f        sbc     a,a
14ee c37b13    jp      $137b			; TXT SET BACK
;; =============================================================================
;; performs control character 'FS' function
14f1 23        inc     hl
14f2 7e        ld      a,(hl)			; pen number
14f3 23        inc     hl
14f4 46        ld      b,(hl)			; ink 1
14f5 23        inc     hl
14f6 4e        ld      c,(hl)			; ink 2
14f7 c3f20c    jp      $0cf2			; SCR SET INK
;;====================================================================
;; performs control character 'GS' instruction
14fa 23        inc     hl
14fb 46        ld      b,(hl)			; ink 1
14fc 23        inc     hl
14fd 4e        ld      c,(hl)			; ink 2
14fe c3f70c    jp      $0cf7			; SCR SET BORDER
;;====================================================================
;; performs control character 'SUB' function
1501 23        inc     hl
1502 56        ld      d,(hl)			; left column
1503 23        inc     hl
1504 7e        ld      a,(hl)			; right column
1505 23        inc     hl
1506 5e        ld      e,(hl)			; top row
1507 23        inc     hl
1508 6e        ld      l,(hl)			; bottom row
1509 67        ld      h,a
150a c30812    jp      $1208			; TXT WIN ENABLE
;;====================================================================
;; performs control character 'EM' function
150d 23        inc     hl
150e 7e        ld      a,(hl)			; character index
150f 23        inc     hl
1510 c3f212    jp      $12f2			; TXT SET MATRIX
;;====================================================================
;; performs control character 'NUL' function
1513 cda411    call    $11a4
1516 c3cdbd    jp      $bdcd			; IND: TXT DRAW CURSOR
;;====================================================================
;; performs control character 'BS' function
1519 1100ff    ld      de,$ff00
151c 180d      jr      $152b            ; (+$0d)
;;====================================================================
;; performs control character 'TAB' function
151e 110001    ld      de,$0100
1521 1808      jr      $152b            ; (+$08)
;;====================================================================
;; performs control character 'LF' function
1523 110100    ld      de,$0001
1526 1803      jr      $152b            ; (+$03)
;;====================================================================
;; performs control character 'VT' function
1528 11ff00    ld      de,$00ff
;;--------------------------------------------------------------------
;; D = column adjustment
;; E = row adjustment
152b d5        push    de
152c cda411    call    $11a4
152f d1        pop     de

;; adjust row 
1530 7d        ld      a,l
1531 83        add     a,e
1532 6f        ld      l,a

;; adjust column
1533 7c        ld      a,h
1534 82        add     a,d
1535 67        ld      h,a

1536 c37611    jp      $1176			; set cursor position and draw it
;;====================================================================
;; performs control character 'RS' function
1539 2a29b7    ld      hl,($b729)
153c c37311    jp      $1173			;; undraw cursor, set cursor position and draw it
;;===========================================================================
;; performs control character 'CR' function
153f cda411    call    $11a4
1542 3a2ab7    ld      a,($b72a)
1545 18ee      jr      $1535            ; (-$12)

;;===========================================================================
;; performs control character 'US' function
1547 23        inc     hl
1548 56        ld      d,(hl)			; column
1549 23        inc     hl
154a 5e        ld      e,(hl)			; row
154b eb        ex      de,hl
154c c37011    jp      $1170			; TXT SET CURSOR

;;===========================================================================
;; TXT CLEAR WINDOW

154f cdd0bd    call    $bdd0			; IND: TXT UNDRAW CURSOR
1552 2a29b7    ld      hl,($b729)
1555 2226b7    ld      ($b726),hl
1558 ed5b2bb7  ld      de,($b72b)
155c 1844      jr      $15a2            ; (+$44)

;;===========================================================================
;; performs control character 'DLE' function
155e cda411    call    $11a4
1561 54        ld      d,h
1562 5d        ld      e,l
1563 183d      jr      $15a2            ; (+$3d)

;;===========================================================================
;; performs control character 'DC4' function
1565 cd8f15    call    $158f			; control character 'DC2'
1568 2a29b7    ld      hl,($b729)
156b ed5b2bb7  ld      de,($b72b)
156f 3a26b7    ld      a,($b726)
1572 6f        ld      l,a
1573 2c        inc     l
1574 bb        cp      e
1575 d0        ret     nc

1576 1811      jr      $1589            ; (+$11)

;;===========================================================================
;; performs control character 'DC3' function
1578 cd9915    call    $1599			; control character 'DC1' function
157b 2a29b7    ld      hl,($b729)
157e 3a2cb7    ld      a,($b72c)
1581 57        ld      d,a
1582 3a26b7    ld      a,($b726)
1585 3d        dec     a
1586 5f        ld      e,a
1587 bd        cp      l
1588 d8        ret     c

1589 3a30b7    ld      a,($b730)
158c c3b90d    jp      $0db9			; SCR FILL BOX

;;===========================================================================
;; performs control character 'DC2' function
158f cda411    call    $11a4
1592 5d        ld      e,l
1593 3a2cb7    ld      a,($b72c)
1596 57        ld      d,a
1597 1809      jr      $15a2            ; (+$09)

;;===========================================================================
;; performs control character 'DC1' function
1599 cda411    call    $11a4
159c eb        ex      de,hl
159d 6b        ld      l,e
159e 3a2ab7    ld      a,($b72a)
15a1 67        ld      h,a

;;---------------------------------------------------------------------------
15a2 cd8915    call    $1589
15a5 c3cdbd    jp      $bdcd			; IND: TXT DRAW CURSOR

;;===========================================================================
;; GRA INITIALISE
15a8 cdd715    call    $15d7			; GRA RESET
15ab 210100    ld      hl,$0001
15ae 7c        ld      a,h
15af cd6e17    call    $176e			; GRA SET PAPER
15b2 7d        ld      a,l
15b3 cd6717    call    $1767			; GRA SET PEN
15b6 210000    ld      hl,$0000
15b9 54        ld      d,h
15ba 5d        ld      e,l
15bb cd0e16    call    $160e			; GRA SET ORIGIN
15be 110080    ld      de,$8000
15c1 21ff7f    ld      hl,$7fff
15c4 e5        push    hl
15c5 d5        push    de
15c6 cda516    call    $16a5			; GRA WIN WIDTH
15c9 e1        pop     hl
15ca d1        pop     de
15cb c3ea16    jp      $16ea			; GRA WIN HEIGHT
;;===========================================================================

15ce cd7a17    call    $177a			; GRA GET PAPER
15d1 67        ld      h,a
15d2 cd7517    call    $1775			; GRA GET PEN
15d5 6f        ld      l,a
15d6 c9        ret     

;;===========================================================================
;; GRA RESET
15d7 cdf015    call    $15f0			
15da 21e015    ld      hl,$15e0			;; table used to initialise graphics pack indirections
15dd c3b40a    jp      $0ab4			;; initialise graphics pack indirections

15e0 
defb &09
defw &bddc
15e3 c38617 jp      $1786							;; IND: GRA PLOT
15e6 c39a17 jp      $179a							;; IND: GRA TEXT
15e9 c3b417 jp      $17b4							;; IND: GRA LINE

;;===========================================================================
;; GRA DEFAULT

15ec af        xor     a
15ed cd550c    call    $0c55			; SCR ACCESS

15f0 af        xor     a
15f1 cdd519    call    $19d5			; GRA SET BACK
15f4 2f        cpl     
15f5 cdb017    call    $17b0			; GRA SET FIRST
15f8 c3ac17    jp      $17ac			; GRA SET LINE MASK

;;===========================================================================
;; GRA MOVE RELATIVE
15fb cd5d16    call    $165d			; convert relative graphics coordinate to
										; absolute graphics coordinate


;;---------------------------------------------------------------------------
;; GRA MOVE ABSOLUTE
15fe ed5397b6  ld      ($b697),de		; absolute x
1602 2299b6    ld      ($b699),hl		; absolute y
1605 c9        ret     

;;===========================================================================
;; GRA ASK CURSOR
1606 ed5b97b6  ld      de,($b697)		; absolute x
160a 2a99b6    ld      hl,($b699)		; absolute y
160d c9        ret     

;;===========================================================================
;; GRA SET ORIGIN
160e ed5393b6  ld      ($b693),de		; origin x
1612 2295b6    ld      ($b695),hl		; origin y


;;===========================================================================
;; set absolute position to origin
1615 110000    ld      de,$0000			; x = 0
1618 62        ld      h,d
1619 6b        ld      l,e				; y = 0
161a 18e2      jr      $15fe            ; GRA MOVE ABSOLUTE

;;===========================================================================
;; GRA GET ORIGIN
161c ed5b93b6  ld      de,($b693)		; origin x	
1620 2a95b6    ld      hl,($b695)		; origin y
1623 c9        ret     

;;===========================================================================
;; get cursor absolute user coordinate
1624 cd0616    call    $1606			; GRA ASK CURSOR

;;----------------------------------------------------------------------------
;; get absolute user coordinate
1627 cdfe15    call    $15fe			; GRA MOVE ABSOLUTE

;;===========================================================================
;; GRA FROM USER
;; DE = X user coordinate
;; HL = Y user coordinate
;; out:
;; DE = x base coordinate
;; HL = y base coordinate
162a e5        push    hl
162b cd0c0b    call    $0b0c			; SCR GET MODE
162e ed44      neg     
1630 defd      sbc     a,$fd
1632 2600      ld      h,$00
1634 6f        ld      l,a
1635 cb7a      bit     7,d
1637 2803      jr      z,$163c          ; (+$03)
1639 eb        ex      de,hl
163a 19        add     hl,de
163b eb        ex      de,hl
163c 2f        cpl     
163d a3        and     e
163e 5f        ld      e,a
163f 7d        ld      a,l
1640 2a93b6    ld      hl,($b693)		; origin x
1643 19        add     hl,de
1644 0f        rrca    
1645 dce516    call    c,$16e5			; HL = HL/2
1648 0f        rrca    
1649 dce516    call    c,$16e5			; HL = HL/2
164c d1        pop     de
164d e5        push    hl
164e 7a        ld      a,d
164f 07        rlca    
1650 3001      jr      nc,$1653         
1652 13        inc     de
1653 cb83      res     0,e
1655 2a95b6    ld      hl,($b695)		; origin y
1658 19        add     hl,de
1659 d1        pop     de
165a c3e516    jp      $16e5			; HL = HL/2

;;==================================================================================
;; convert relative graphics coordinate to absolute graphics coordinate
;; DE = relative X
;; HL = relative Y
165d e5        push    hl
165e 2a97b6    ld      hl,($b697)		; absolute x		
1661 19        add     hl,de
1662 d1        pop     de
1663 e5        push    hl
1664 2a99b6    ld      hl,($b699)		; absolute y
1667 19        add     hl,de
1668 d1        pop     de
1669 c9        ret     

;;==================================================================================
;; X graphics coordinate within window

;; DE = x coordinate
166a 2a9bb6    ld      hl,($b69b)		; graphics window left edge
166d 37        scf     
166e ed52      sbc     hl,de
1670 f27e16    jp      p,$167e

1673 2a9db6    ld      hl,($b69d)		; graphics window right edge
1676 b7        or      a
1677 ed52      sbc     hl,de
1679 37        scf     
167a f0        ret     p

167b f6ff      or      $ff
167d c9        ret     

167e af        xor     a
167f c9        ret     

;;==================================================================================
;; y graphics coordinate within window
;; DE = y coordinate
1680 2a9fb6    ld      hl,($b69f)		; graphics window top edge
1683 b7        or      a
1684 ed52      sbc     hl,de
1686 fa7b16    jp      m,$167b
1689 2aa1b6    ld      hl,($b6a1)		; graphics window bottom edge
168c 37        scf     
168d ed52      sbc     hl,de
168f f27e16    jp      p,$167e
1692 37        scf     
1693 c9        ret     

;;==================================================================================

;; current point within graphics window
1694 cd2716    call    $1627			; get absolute user coordinate

;; point in graphics window?
;; HL = x coordinate
;; DE = y coordinate
1697 e5        push    hl
1698 cd6a16    call    $166a			; X graphics coordinate within window
169b e1        pop     hl
169c d0        ret     nc

169d d5        push    de
169e eb        ex      de,hl
169f cd8016    call    $1680			; Y graphics coordinate within window
16a2 eb        ex      de,hl
16a3 d1        pop     de
16a4 c9        ret     

;;==================================================================================
;; GRA WIN WIDTH
;; DE = left edge
;; HL = right edge
16a5 e5        push    hl
16a6 cdd116    call    $16d1			;; Make X coordinate within range 0-639
16a9 d1        pop     de
16aa e5        push    hl
16ab cdd116    call    $16d1			;; Make X coordinate within range 0-639
16ae d1        pop     de
16af 7b        ld      a,e
16b0 95        sub     l
16b1 7a        ld      a,d
16b2 9c        sbc     a,h
16b3 3801      jr      c,$16b6         

16b5 eb        ex      de,hl
16b6 7b        ld      a,e
16b7 e6f8      and     $f8
16b9 5f        ld      e,a
16ba 7d        ld      a,l
16bb f607      or      $07
16bd 6f        ld      l,a
16be cd0c0b    call    $0b0c			; SCR GET MODE
16c1 3d        dec     a
16c2 fce116    call    m,$16e1			; DE = DE/2 and HL = HL/2
16c5 3d        dec     a
16c6 fce116    call    m,$16e1			; DE = DE/2 and HL = HL/2
16c9 ed539bb6  ld      ($b69b),de		; graphics window left edge
16cd 229db6    ld      ($b69d),hl		; graphics window right edge
16d0 c9        ret     

;;==================================================================================
;; Make X coordinate within range 0-639
16d1 7a        ld      a,d
16d2 b7        or      a
16d3 210000    ld      hl,$0000
16d6 f8        ret     m

16d7 217f02    ld      hl,$027f			; 639
16da 7b        ld      a,e
16db 95        sub     l
16dc 7a        ld      a,d
16dd 9c        sbc     a,h
16de d0        ret     nc

16df eb        ex      de,hl
16e0 c9        ret     

;;==================================================================================
;; DE = DE/2
;; HL = HL/2
16e1 cb2a      sra     d
16e3 cb1b      rr      e

;;----------------------------------------------------------------------------------
;; HL = HL/2
16e5 cb2c      sra     h
16e7 cb1d      rr      l
16e9 c9        ret     

;;==================================================================================
;; GRA WIN HEIGHT
16ea e5        push    hl
16eb cd0317    call    $1703			;; make Y coordinate in range 0-199
16ee d1        pop     de
16ef e5        push    hl
16f0 cd0317    call    $1703			;; make Y coordinate in range 0-199
16f3 d1        pop     de
16f4 7d        ld      a,l
16f5 93        sub     e
16f6 7c        ld      a,h
16f7 9a        sbc     a,d
16f8 3801      jr      c,$16fb          ; (+$01)
16fa eb        ex      de,hl
16fb ed539fb6  ld      ($b69f),de		; graphics window top edge
16ff 22a1b6    ld      ($b6a1),hl		; graphics window bottom edge
1702 c9        ret     

;;==================================================================================
;; make Y coordinate in range 0-199

1703 7a        ld      a,d
1704 b7        or      a
1705 210000    ld      hl,$0000
1708 f8        ret     m

1709 cb3a      srl     d
170b cb1b      rr      e
170d 21c700    ld      hl,$00c7		; 199
1710 7b        ld      a,e
1711 95        sub     l
1712 7a        ld      a,d
1713 9c        sbc     a,h
1714 d0        ret     nc

1715 eb        ex      de,hl
1716 c9        ret     

;;==================================================================================
;; GRA GET W WIDTH
1717 ed5b9bb6  ld      de,($b69b)		; graphics window left edge
171b 2a9db6    ld      hl,($b69d)		; graphics window right edge
171e cd0c0b    call    $0b0c			; SCR GET MODE
1721 3d        dec     a
1722 fc2717    call    m,$1727
1725 3d        dec     a
1726 f0        ret     p

;; HL = (HL*2)+1
1727 29        add     hl,hl			
1728 23        inc     hl				

;; DE = DE * 2
1729 eb        ex      de,hl
172a 29        add     hl,hl
172b eb        ex      de,hl			

172c c9        ret     
;;==================================================================================
;; GRA GET W HEIGHT
172d ed5b9fb6  ld      de,($b69f)		; graphics window top edge
1731 2aa1b6    ld      hl,($b6a1)		; graphics window bottom edge
1734 18f1      jr      $1727
;;==================================================================================
;; GRA CLEAR WINDOW
1736 cd1717    call    $1717			; GRA GET W WIDTH
1739 b7        or      a
173a ed52      sbc     hl,de
173c 23        inc     hl
173d cde516    call    $16e5			; HL = HL/2
1740 cde516    call    $16e5			; HL = HL/2
1743 cb3d      srl     l
1745 45        ld      b,l
1746 ed5ba1b6  ld      de,($b6a1)		; graphics window bottom edge
174a 2a9fb6    ld      hl,($b69f)		; graphics window top edge
174d e5        push    hl
174e b7        or      a
174f ed52      sbc     hl,de
1751 23        inc     hl
1752 4d        ld      c,l
1753 ed5b9bb6  ld      de,($b69b)		; graphics window left edge
1757 e1        pop     hl
1758 c5        push    bc
1759 cdaf0b    call    $0baf			;; SCR DOT POSITION
175c d1        pop     de
175d 3aa4b6    ld      a,($b6a4)		; graphics paper
1760 4f        ld      c,a
1761 cdbd0d    call    $0dbd			;; SCR FLOOD BOX
1764 c31516    jp      $1615			;; set absolute position to origin

;;==================================================================================
;; GRA SET PEN
1767 cd8e0c    call    $0c8e				;; SCR INK ENCODE
176a 32a3b6    ld      ($b6a3),a		; graphics pen
176d c9        ret     

;;==================================================================================
;; GRA SET PAPER
176e cd8e0c    call    $0c8e				;; SCR INK ENCODE
1771 32a4b6    ld      ($b6a4),a		; graphics paper
1774 c9        ret     
;;==================================================================================
;; GRA GET PEN
1775 3aa3b6    ld      a,($b6a3)		; graphics pen
1778 1803      jr      $177d            ; do SCR INK ENCODE
;;==================================================================================
;; GRA GET PAPER
177a 3aa4b6    ld      a,($b6a4)		; graphics paper
177d c3a70c    jp      $0ca7			;; SCR INK DECODE

;;==================================================================================
;; GRA PLOT RELATIVE
1780 cd5d16    call    $165d			; convert relative graphics coordinate to
										; absolute graphics coordinate

;;----------------------------------------------------------------------------------
;; GRA PLOT ABSOLUTE
1783 c3dcbd    jp      $bddc			; IND: GRA PLOT

;;============================================================================
;; IND: GRA PLOT
1786 cd9416    call    $1694			; test if current coordinate within graphics window
1789 d0        ret     nc

178a cdaf0b    call    $0baf			;; SCR DOT POSITION
178d 3aa3b6    ld      a,($b6a3)		; graphics pen
1790 47        ld      b,a
1791 c3e8bd    jp      $bde8			; IND: SCR WRITE

;;===========================================================================
;; GRA TEST RELATIVE
1794 cd5d16    call    $165d			; convert relative graphics coordinate to
										; absolute graphics coordinate

;;---------------------------------------------------------------------------
;; GRA TEST ABSOLUTE
1797 c3dfbd    jp      $bddf			; IND: GRA TEST

;;===========================================================================
;; IND: GRA TEXT
179a cd9416    call    $1694			; test if current coordinate within graphics window
179d d27a17    jp      nc,$177a			; GRA GET PAPER
17a0 cdaf0b    call    $0baf			; SCR DOT POSITION
17a3 c3e5bd    jp      $bde5			; IND: SCR READ

;;===========================================================================
;; GRA LINE RELATIVE
17a6 cd5d16    call    $165d			; convert relative graphics coordinate to
										; absolute graphics coordinate

;;---------------------------------------------------------------------------
;; GRA LINE ABSOLUTE
17a9 c3e2bd    jp      $bde2			; IND: GRA LINE

;;===========================================================================
;; GRA SET LINE MASK

17ac 32b3b6    ld      ($b6b3),a		; gra line mask
17af c9        ret     

;;===========================================================================
;; GRA SET FIRST

17b0 32b2b6    ld      ($b6b2),a
17b3 c9        ret     

;;===========================================================================
;; IND: GRA LINE
17b4 e5        push    hl
17b5 cd8b18    call    $188b			; get cursor absolute position
17b8 e1        pop     hl
17b9 cd2716    call    $1627			; get absolute user coordinate

;; remember Y coordinate
17bc e5        push    hl

;; DE = X coordinate

;;-------------------------------------------

;; calculate dx
17bd 2aa5b6    ld      hl,($b6a5)		; absolute user X coordinate
17c0 b7        or      a
17c1 ed52      sbc     hl,de

;; this will record the fact of dx is +ve or negative
17c3 7c        ld      a,h
17c4 32adb6    ld      ($b6ad),a

;; if dx is negative, make it positive
17c7 fc3919    call    m,$1939			; HL = -HL

;; HL = abs(dx)

;;-------------------------------------------

;; calculate dy
17ca d1        pop     de
;; DE = Y coordinate
17cb e5        push    hl
17cc 2aa7b6    ld      hl,($b6a7)		; absolute user Y coordinate
17cf b7        or      a
17d0 ed52      sbc     hl,de

;; this stores the fact of dy is +ve or negative
17d2 7c        ld      a,h
17d3 32aeb6    ld      ($b6ae),a

;; if dy is negative, make it positive
17d6 fc3919    call    m,$1939			; HL = -HL

;; HL = abs(dy)


17d9 d1        pop     de
;; DE = abs(dx)
;; HL = abs(dy)

;;-------------------------------------------

;; is dx or dy largest?
17da b7        or      a
17db ed52      sbc     hl,de			; dy-dx
17dd 19        add     hl,de			; and return it back to their original values

17de 9f        sbc     a,a
17df 32afb6    ld      ($b6af),a		; remembers which of dy/dx was largest

17e2 3aaeb6    ld      a,($b6ae)		; dy is negative
17e5 2804      jr      z,$17eb          ; depends on result of dy-dx

;; if yes, then swap dx/dy
17e7 eb        ex      de,hl
;; DE = abs(dy)
;; HL = abs(dx)

17e8 3aadb6    ld      a,($b6ad)		; dx is negative

;;-------------------------------------------

17eb f5        push    af
17ec ed53abb6  ld      ($b6ab),de
17f0 44        ld      b,h
17f1 4d        ld      c,l
17f2 3ab2b6    ld      a,($b6b2)
17f5 b7        or      a
17f6 2801      jr      z,$17f9          ; (+$01)
17f8 03        inc     bc
17f9 ed43b0b6  ld      ($b6b0),bc
17fd cd3919    call    $1939			; HL = -HL
1800 e5        push    hl
1801 19        add     hl,de
1802 22a9b6    ld      ($b6a9),hl
1805 e1        pop     hl
1806 cb2c      sra     h				;; /2 for y coordinate (0-400 GRA coordinates, 0-200 actual number of lines)
1808 cb1d      rr      l
180a f1        pop     af
180b 07        rlca    
180c 3812      jr      c,$1820          ; (+$12)
180e e5        push    hl
180f cd8b18    call    $188b			; get cursor absolute position
1812 2aadb6    ld      hl,($b6ad)
1815 7c        ld      a,h
1816 2f        cpl     
1817 67        ld      h,a
1818 7d        ld      a,l
1819 2f        cpl     
181a 6f        ld      l,a
181b 22adb6    ld      ($b6ad),hl
181e 1812      jr      $1832            ; (+$12)


1820 3ab2b6    ld      a,($b6b2)
1823 b7        or      a
1824 200d      jr      nz,$1833         ; (+$0d)
1826 19        add     hl,de
1827 e5        push    hl

1828 3aafb6    ld      a,($b6af)		; dy or dx was biggest?
182b 07        rlca    
182c dcda18    call    c,$18da			; plot a pixel moving up
182f d42819    call    nc,$1928			; plot a pixel moving right

1832 e1        pop     hl
1833 7a        ld      a,d
1834 b3        or      e
1835 ca9818    jp      z,$1898
1838 dde5      push    ix
183a 010000    ld      bc,$0000
183d c5        push    bc
183e dde1      pop     ix
1840 dde5      push    ix
1842 d1        pop     de
1843 b7        or      a
1844 ed5a      adc     hl,de
1846 ed5babb6  ld      de,($b6ab)
184a f25318    jp      p,$1853
184d 03        inc     bc
184e dd19      add     ix,de
1850 19        add     hl,de
1851 30fa      jr      nc,$184d         ; (-$06)

; DE = -DE
1853 af        xor     a
1854 93        sub     e
1855 5f        ld      e,a
1856 9f        sbc     a,a
1857 92        sub     d
1858 57        ld      d,a

1859 19        add     hl,de
185a 3005      jr      nc,$1861         ; (+$05)
185c dd19      add     ix,de
185e 0b        dec     bc
185f 18f8      jr      $1859            ; (-$08)


1861 ed5ba9b6  ld      de,($b6a9)
1865 19        add     hl,de
1866 c5        push    bc
1867 e5        push    hl
1868 2ab0b6    ld      hl,($b6b0)
186b b7        or      a
186c ed42      sbc     hl,bc
186e 3006      jr      nc,$1876         ; (+$06)

1870 09        add     hl,bc
1871 44        ld      b,h
1872 4d        ld      c,l
1873 210000    ld      hl,$0000

1876 22b0b6    ld      ($b6b0),hl
1879 cd9818    call    $1898			; plot with clip
187c e1        pop     hl
187d c1        pop     bc
187e 3008      jr      nc,$1888         ; (+$08)
1880 ed5bb0b6  ld      de,($b6b0)
1884 7a        ld      a,d
1885 b3        or      e
1886 20b8      jr      nz,$1840         ; (-$48)
1888 dde1      pop     ix
188a c9        ret     
;;==================================================================================

188b d5        push    de
188c cd2416    call    $1624			;; get cursor absolute user coordinate
188f ed53a5b6  ld      ($b6a5),de		
1893 22a7b6    ld      ($b6a7),hl
1896 d1        pop     de
1897 c9        ret     
;;==================================================================================

1898 3aafb6    ld      a,($b6af)
189b 07        rlca    
189c 384d      jr      c,$18eb          ; (+$4d)
189e 78        ld      a,b
189f b1        or      c
18a0 2838      jr      z,$18da          ; (+$38)
18a2 2aa7b6    ld      hl,($b6a7)
18a5 09        add     hl,bc
18a6 2b        dec     hl
18a7 44        ld      b,h
18a8 4d        ld      c,l
18a9 eb        ex      de,hl
18aa cd8016    call    $1680			; Y graphics coordinate within window
18ad 2aa7b6    ld      hl,($b6a7)
18b0 eb        ex      de,hl
18b1 23        inc     hl
18b2 22a7b6    ld      ($b6a7),hl
18b5 3806      jr      c,$18bd          ; 
18b7 2821      jr      z,$18da          ; 
18b9 ed4b9fb6  ld      bc,($b69f)		; graphics window top edge
18bd cd8016    call    $1680			; Y graphics coordinate within window
18c0 3805      jr      c,$18c7          ; (+$05)
18c2 c0        ret     nz

18c3 ed5ba1b6  ld      de,($b6a1)		; graphics window bottom edge
18c7 d5        push    de
18c8 ed5ba5b6  ld      de,($b6a5)		
18cc cd6a16    call    $166a			; graphics x coordinate within window
18cf e1        pop     hl
18d0 3805      jr      c,$18d7          ; (+$05)
18d2 21adb6    ld      hl,$b6ad
18d5 ae        xor     (hl)
18d6 f0        ret     p

18d7 dc1610    call    c,$1016			; plot a pixel, going up a line


18da 2aa5b6    ld      hl,($b6a5)
18dd 3aadb6    ld      a,($b6ad)
18e0 07        rlca    
18e1 23        inc     hl
18e2 3802      jr      c,$18e6          ; (+$02)
18e4 2b        dec     hl
18e5 2b        dec     hl
18e6 22a5b6    ld      ($b6a5),hl
18e9 37        scf     
18ea c9        ret     

;; we work with coordinates...

;; this performs the clipping to find if the coordinates are within rnage

18eb 78        ld      a,b
18ec b1        or      c
18ed 2839      jr      z,$1928          ; (+$39)
18ef 2aa5b6    ld      hl,($b6a5)
18f2 09        add     hl,bc
18f3 2b        dec     hl
18f4 44        ld      b,h
18f5 4d        ld      c,l
18f6 eb        ex      de,hl
18f7 cd6a16    call    $166a			; x graphics coordinate within window
18fa 2aa5b6    ld      hl,($b6a5)
18fd eb        ex      de,hl
18fe 23        inc     hl
18ff 22a5b6    ld      ($b6a5),hl
1902 3806      jr      c,$190a          
1904 2822      jr      z,$1928          
1906 ed4b9db6  ld      bc,($b69d)		; graphics window right edge
190a cd6a16    call    $166a			; x graphics coordinate within window
190d 3805      jr      c,$1914          
190f c0        ret     nz

1910 ed5b9bb6  ld      de,($b69b)		; graphics window left edge
1914 d5        push    de
1915 ed5ba7b6  ld      de,($b6a7)
1919 cd8016    call    $1680			; Y graphics coordinate within window
191c e1        pop     hl
191d 3805      jr      c,$1924          ; (+$05)

191f 21aeb6    ld      hl,$b6ae
1922 ae        xor     (hl)
1923 f0        ret     p

1924 eb        ex      de,hl
1925 dcc20f    call    c,$0fc2			; plot a pixel moving right

1928 2aa7b6    ld      hl,($b6a7)
192b 3aaeb6    ld      a,($b6ae)
192e 07        rlca    
192f 23        inc     hl
1930 3802      jr      c,$1934          ; (+$02)
1932 2b        dec     hl
1933 2b        dec     hl
1934 22a7b6    ld      ($b6a7),hl
1937 37        scf     
1938 c9        ret     

;;==================================================================================
; HL = -HL
1939 af        xor     a
193a 95        sub     l
193b 6f        ld      l,a
193c 9f        sbc     a,a
193d 94        sub     h
193e 67        ld      h,a
193f c9        ret     

;;===========================================================================
;; GRA WR CHAR

1940 dde5      push    ix
1942 cdd412    call    $12d4			; TXT GET MATRIX
1945 e5        push    hl
1946 dde1      pop     ix
1948 cd2416    call    $1624			;; get cursor absolute user coordinate
194b cd9716    call    $1697			;; point in graphics window
194e 304b      jr      nc,$199b         ; (+$4b)
1950 e5        push    hl
1951 d5        push    de
1952 010700    ld      bc,$0007
1955 eb        ex      de,hl
1956 09        add     hl,bc
1957 eb        ex      de,hl
1958 b7        or      a
1959 ed42      sbc     hl,bc
195b cd9716    call    $1697			;; point in graphics window
195e d1        pop     de
195f e1        pop     hl
1960 3039      jr      nc,$199b         ; (+$39)
1962 cdaf0b    call    $0baf			;; SCR DOT POSITION
1965 1608      ld      d,$08
1967 e5        push    hl
1968 dd5e00    ld      e,(ix+$00)
196b 37        scf     
196c cb13      rl      e
196e cdc419    call    $19c4
1971 cb09      rrc     c
1973 dc050c    call    c,$0c05			; SCR NEXT BYTE
1976 cb23      sla     e
1978 20f4      jr      nz,$196e         ; (-$0c)
197a e1        pop     hl
197b cd1f0c    call    $0c1f			; SCR NEXT LINE
197e dd23      inc     ix
1980 15        dec     d
1981 20e4      jr      nz,$1967         ; (-$1c)
1983 dde1      pop     ix
1985 cd0616    call    $1606			; GRA ASK CURSOR
1988 eb        ex      de,hl
1989 cd0c0b    call    $0b0c			; SCR GET MODE
198c 010800    ld      bc,$0008
198f 2804      jr      z,$1995          ; (+$04)
1991 3003      jr      nc,$1996         ; (+$03)
1993 09        add     hl,bc
1994 09        add     hl,bc
1995 09        add     hl,bc
1996 09        add     hl,bc
1997 eb        ex      de,hl
1998 c3fe15    jp      $15fe			; GRA MOVE ABSOLUTE

;;==================================================================================
199b 0608      ld      b,$08
199d c5        push    bc
199e d5        push    de
199f dd7e00    ld      a,(ix+$00)
19a2 37        scf     
19a3 8f        adc     a,a
19a4 e5        push    hl
19a5 d5        push    de
19a6 f5        push    af
19a7 cd9716    call    $1697			;; point in graphics window
19aa 3008      jr      nc,$19b4         ; (+$08)
19ac cdaf0b    call    $0baf			;; SCR DOT POSITION
19af f1        pop     af
19b0 f5        push    af
19b1 cdc419    call    $19c4
19b4 f1        pop     af
19b5 d1        pop     de
19b6 e1        pop     hl
19b7 13        inc     de
19b8 87        add     a,a
19b9 20e9      jr      nz,$19a4         ; (-$17)
19bb d1        pop     de
19bc 2b        dec     hl
19bd dd23      inc     ix
19bf c1        pop     bc
19c0 10db      djnz    $199d            ; (-$25)
19c2 18bf      jr      $1983            ; (-$41)

;;==================================================================================

19c4 3aa3b6    ld      a,($b6a3)		; graphics pen
19c7 3808      jr      c,$19d1          ; (+$08)
19c9 3ab4b6    ld      a,($b6b4)
19cc b7        or      a
19cd c0        ret     nz

19ce 3aa4b6    ld      a,($b6a4)		; graphics paper
19d1 47        ld      b,a
19d2 c3e8bd    jp      $bde8			; IND: SCR WRITE

;;===========================================================================
;; GRA SET BACK

19d5 32b4b6    ld      ($b6b4),a
19d8 c9        ret     

;;===========================================================================
;; GRA FILL
;; HL = buffer
;; A = pen to fill
;; DE = length of buffer

19d9 22a5b6    ld      ($b6a5),hl
19dc 3601      ld      (hl),$01
19de 1b        dec     de
19df ed53a7b6  ld      ($b6a7),de
19e3 cd8e0c    call    $0c8e				;; SCR INK ENCODE
19e6 32aab6    ld      ($b6aa),a
19e9 cd2416    call    $1624			;; get cursor absolute user coordinate
19ec cd9716    call    $1697			;; point in graphics window
19ef dc421b    call    c,$1b42
19f2 d0        ret     nc

19f3 e5        push    hl
19f4 cde71a    call    $1ae7
19f7 e3        ex      (sp),hl
19f8 cd151b    call    $1b15
19fb c1        pop     bc
19fc 3eff      ld      a,$ff
19fe 32a9b6    ld      ($b6a9),a
1a01 e5        push    hl
1a02 d5        push    de
1a03 c5        push    bc
1a04 cd0b1a    call    $1a0b
1a07 c1        pop     bc
1a08 d1        pop     de
1a09 e1        pop     hl
1a0a af        xor     a
1a0b 32abb6    ld      ($b6ab),a
1a0e cdde1a    call    $1ade
1a11 cd9716    call    $1697			;; point in graphics window
1a14 dc501a    call    c,$1a50
1a17 38f5      jr      c,$1a0e          ; (-$0b)
1a19 2aa5b6    ld      hl,($b6a5)		; graphics fill buffer
1a1c e7        rst     $20				; RST 4 - LOW: RAM LAM
1a1d fe01      cp      $01
1a1f 282a      jr      z,$1a4b          ; (+$2a)
1a21 32abb6    ld      ($b6ab),a
1a24 eb        ex      de,hl
1a25 2aa7b6    ld      hl,($b6a7)
1a28 010700    ld      bc,$0007
1a2b 09        add     hl,bc
1a2c 22a7b6    ld      ($b6a7),hl
1a2f eb        ex      de,hl
1a30 2b        dec     hl
1a31 e7        rst     $20				; RST 4 - LOW: RAM LAM
1a32 47        ld      b,a
1a33 2b        dec     hl
1a34 e7        rst     $20				; RST 4 - LOW: RAM LAM
1a35 4f        ld      c,a
1a36 2b        dec     hl
1a37 e7        rst     $20				; RST 4 - LOW: RAM LAM
1a38 57        ld      d,a
1a39 2b        dec     hl
1a3a e7        rst     $20				; RST 4 - LOW: RAM LAM
1a3b 5f        ld      e,a
1a3c d5        push    de
1a3d 2b        dec     hl
1a3e e7        rst     $20				; RST 4 - LOW: RAM LAM
1a3f 57        ld      d,a
1a40 2b        dec     hl
1a41 e7        rst     $20				; RST 4 - LOW: RAM LAM
1a42 5f        ld      e,a
1a43 2b        dec     hl
1a44 22a5b6    ld      ($b6a5),hl		; graphics fill buffer
1a47 eb        ex      de,hl
1a48 d1        pop     de
1a49 18c6      jr      $1a11            ; (-$3a)
1a4b 3aa9b6    ld      a,($b6a9)
1a4e 0f        rrca    
1a4f c9        ret     

;;==================================================================================

1a50 ed43acb6  ld      ($b6ac),bc
1a54 cd421b    call    $1b42
1a57 3809      jr      c,$1a62          ; (+$09)
1a59 cdf11a    call    $1af1
1a5c d0        ret     nc

1a5d 22aeb6    ld      ($b6ae),hl
1a60 1811      jr      $1a73            ; (+$11)
1a62 e5        push    hl
1a63 cd151b    call    $1b15
1a66 22aeb6    ld      ($b6ae),hl
1a69 c1        pop     bc
1a6a 7d        ld      a,l
1a6b 91        sub     c
1a6c 7c        ld      a,h
1a6d 98        sbc     a,b
1a6e dccb1a    call    c,$1acb
1a71 60        ld      h,b
1a72 69        ld      l,c
1a73 cde71a    call    $1ae7
1a76 22b0b6    ld      ($b6b0),hl
1a79 ed4bacb6  ld      bc,($b6ac)
1a7d b7        or      a
1a7e ed42      sbc     hl,bc
1a80 09        add     hl,bc
1a81 2811      jr      z,$1a94          ; (+$11)
1a83 3008      jr      nc,$1a8d         ; (+$08)
1a85 cdf11a    call    $1af1
1a88 dc9d1a    call    c,$1a9d
1a8b 1807      jr      $1a94            ; (+$07)
1a8d e5        push    hl
1a8e 60        ld      h,b
1a8f 69        ld      l,c
1a90 c1        pop     bc
1a91 cdcb1a    call    $1acb
1a94 2aaeb6    ld      hl,($b6ae)
1a97 ed4bb0b6  ld      bc,($b6b0)
1a9b 37        scf     
1a9c c9        ret     

1a9d d5        push    de
1a9e e5        push    hl
1a9f 2aa7b6    ld      hl,($b6a7)
1aa2 11f9ff    ld      de,$fff9
1aa5 19        add     hl,de
1aa6 d1        pop     de
1aa7 301c      jr      nc,$1ac5         ; (+$1c)
1aa9 22a7b6    ld      ($b6a7),hl
1aac 2aa5b6    ld      hl,($b6a5)		; graphics fill buffer
1aaf 23        inc     hl
1ab0 73        ld      (hl),e
1ab1 23        inc     hl
1ab2 72        ld      (hl),d
1ab3 23        inc     hl
1ab4 d1        pop     de
1ab5 73        ld      (hl),e
1ab6 23        inc     hl
1ab7 72        ld      (hl),d
1ab8 23        inc     hl
1ab9 71        ld      (hl),c
1aba 23        inc     hl
1abb 70        ld      (hl),b
1abc 23        inc     hl
1abd 3aabb6    ld      a,($b6ab)
1ac0 77        ld      (hl),a
1ac1 22a5b6    ld      ($b6a5),hl				; graphics fill buffer
1ac4 c9        ret     

1ac5 af        xor     a
1ac6 32a9b6    ld      ($b6a9),a
1ac9 d1        pop     de
1aca c9        ret     

1acb cdd71a    call    $1ad7
1ace cd421b    call    $1b42
1ad1 d4f11a    call    nc,$1af1
1ad4 dc9d1a    call    c,$1a9d
1ad7 3aabb6    ld      a,($b6ab)
1ada 2f        cpl     
1adb 32abb6    ld      ($b6ab),a
1ade 1b        dec     de
1adf 3aabb6    ld      a,($b6ab)
1ae2 b7        or      a
1ae3 c8        ret     z

1ae4 13        inc     de
1ae5 13        inc     de
1ae6 c9        ret     

1ae7 af        xor     a
1ae8 ed4b9fb6  ld      bc,($b69f)		; graphics window top edge
1aec cdf31a    call    $1af3
1aef 2b        dec     hl
1af0 c9        ret     

;;==================================================================================

1af1 3eff      ld      a,$ff
1af3 c5        push    bc
1af4 d5        push    de
1af5 e5        push    hl
1af6 f5        push    af
1af7 cd4f1b    call    $1b4f
1afa f1        pop     af
1afb 47        ld      b,a
1afc cd341b    call    $1b34
1aff 04        inc     b
1b00 1004      djnz    $1b06            ; (+$04)
1b02 3047      jr      nc,$1b4b         ; (+$47)
1b04 ae        xor     (hl)
1b05 77        ld      (hl),a
1b06 3843      jr      c,$1b4b          ; (+$43)
1b08 e3        ex      (sp),hl
1b09 23        inc     hl
1b0a e3        ex      (sp),hl
1b0b ed52      sbc     hl,de
1b0d 283c      jr      z,$1b4b          ; (+$3c)
1b0f 19        add     hl,de
1b10 cd390c    call    $0c39			; SCR PREV LINE
1b13 18e7      jr      $1afc            ; (-$19)
1b15 c5        push    bc
1b16 d5        push    de
1b17 e5        push    hl
1b18 ed4ba1b6  ld      bc,($b6a1)		; graphics window bottom edge
1b1c cd4f1b    call    $1b4f
1b1f b7        or      a
1b20 ed52      sbc     hl,de
1b22 2827      jr      z,$1b4b          ; (+$27)
1b24 19        add     hl,de
1b25 cd1f0c    call    $0c1f			; SCR NEXT LINE
1b28 cd341b    call    $1b34
1b2b 281e      jr      z,$1b4b          ; (+$1e)
1b2d ae        xor     (hl)
1b2e 77        ld      (hl),a
1b2f e3        ex      (sp),hl
1b30 2b        dec     hl
1b31 e3        ex      (sp),hl
1b32 18eb      jr      $1b1f            ; (-$15)

;;==================================================================================

1b34 3aa3b6    ld      a,($b6a3)		; graphics pen
1b37 ae        xor     (hl)
1b38 a1        and     c
1b39 c8        ret     z

1b3a 3aaab6    ld      a,($b6aa)
1b3d ae        xor     (hl)
1b3e a1        and     c
1b3f c8        ret     z

1b40 37        scf     
1b41 c9        ret     

;;==================================================================================

1b42 c5        push    bc
1b43 d5        push    de
1b44 e5        push    hl
1b45 cdaf0b    call    $0baf			;; SCR DOT POSITION
1b48 cd341b    call    $1b34
1b4b e1        pop     hl
1b4c d1        pop     de
1b4d c1        pop     bc
1b4e c9        ret     

;;==================================================================================

1b4f c5        push    bc
1b50 d5        push    de
1b51 cdaf0b    call    $0baf			;; SCR DOT POSITION
1b54 d1        pop     de
1b55 e3        ex      (sp),hl
1b56 cdaf0b    call    $0baf			;; SCR DOT POSITION
1b59 eb        ex      de,hl
1b5a e1        pop     hl
1b5b c9        ret     

;;===========================================================================
;; KM INITIALISE

1b5c 21021e    ld      hl,$1e02
1b5f cdf61d    call    $1df6			; KM SET DELAY
1b62 af        xor     a
1b63 3255b6    ld      ($b655),a
1b66 67        ld      h,a
1b67 6f        ld      l,a
1b68 2231b6    ld      ($b631),hl
1b6b 01b0ff    ld      bc,$ffb0
1b6e 11d6b5    ld      de,$b5d6
1b71 2192b6    ld      hl,$b692
1b74 3e04      ld      a,$04
1b76 eb        ex      de,hl
1b77 09        add     hl,bc
1b78 eb        ex      de,hl
1b79 72        ld      (hl),d
1b7a 2b        dec     hl
1b7b 73        ld      (hl),e
1b7c 2b        dec     hl
1b7d 3d        dec     a
1b7e 20f6      jr      nz,$1b76         ; (-$0a)

;;-------------------------------------------
;; copy keyboard translation table
1b80 21ef1e    ld      hl,$1eef
1b83 01fa00    ld      bc,$00fa
1b86 edb0      ldir    

;;-------------------------------------------
1b88 060a      ld      b,$0a
1b8a 1135b6    ld      de,$b635
1b8d 213fb6    ld      hl,$b63f
1b90 af        xor     a
1b91 12        ld      (de),a
1b92 13        inc     de
1b93 36ff      ld      (hl),$ff
1b95 23        inc     hl
1b96 10f9      djnz    $1b91            ; (-$07)
;;-------------------------------------------

;;===========================================================================
;; KM RESET

1b98 cd751e    call    $1e75
1b9b cdf81b    call    $1bf8			; reset returned key (KM CHAR RETURN)
1b9e 1190b5    ld      de,$b590
1ba1 219800    ld      hl,$0098
1ba4 cd0a1c    call    $1c0a

1ba7 21b31b    ld      hl,$1bb3			; table used to initialise keyboard manager indirections
1baa cdb40a    call    $0ab4			; initialise keyboard manager indirections (KM TEST BREAK)
1bad cdb40a    call    $0ab4			; initialise keyboard manager indirections (KM SCAN KEYS)
1bb0 c30b1e    jp      $1e0b			; KM DISARM BREAK

1bb3
defb &3
defw &bdee								; IND: KM TEST BREAK
1bb6 c3b81d jp      $1db8
1bb9
defb &3
defw &bdf4								; IND: KM SCAN KEYS
1bbc c3401d jp		$1d40

;;===========================================================================
;; KM WAIT CHAR

1bbf cdc51b    call    $1bc5			; KM READ CHAR
1bc2 30fb      jr      nc,$1bbf         
1bc4 c9        ret     

;;===========================================================================
;; KM READ CHAR

1bc5 e5        push    hl
1bc6 212ab6    ld      hl,$b62a			; returned char
1bc9 7e        ld      a,(hl)			; get char
1bca 36ff      ld      (hl),$ff			; reset state
1bcc be        cp      (hl)				; was a char returned?
1bcd 3827      jr      c,$1bf6          ; a key was put back into buffer, return without expanding it

;; are we expanding?
1bcf 2a28b6    ld      hl,($b628)
1bd2 7c        ld      a,h
1bd3 b7        or      a
1bd4 2011      jr      nz,$1be7			; continue expansion

1bd6 cde11c    call    $1ce1			; KM READ KEY
1bd9 301b      jr      nc,$1bf6         ; (+$1b)
1bdb fe80      cp      $80
1bdd 3817      jr      c,$1bf6          ; (+$17)
1bdf fea0      cp      $a0
1be1 3f        ccf     
1be2 3812      jr      c,$1bf6          ; (+$12)

;; begin expansion
1be4 67        ld      h,a
1be5 2e00      ld      l,$00

;; continue expansion
1be7 d5        push    de
1be8 cdb31c    call    $1cb3			; KM GET EXPAND
1beb 3802      jr      c,$1bef          

;; write expansion pointer
1bed 2600      ld      h,$00
1bef 2c        inc     l
1bf0 2228b6    ld      ($b628),hl
1bf3 d1        pop     de
1bf4 30e0      jr      nc,$1bd6         
1bf6 e1        pop     hl
1bf7 c9        ret     

;===========================================================================
;; reset returned key
1bf8 3eff      ld      a,$ff

;;===========================================================================
;; KM CHAR RETURN

1bfa 322ab6    ld      ($b62a),a
1bfd c9        ret     

;;===========================================================================
;; KM FLUSH

1bfe cdc51b    call    $1bc5			; KM READ CHAR
1c01 38fb      jr      c,$1bfe          
1c03 c9        ret     

;;===========================================================================
;; KM EXP BUFFER

1c04 cd0a1c    call    $1c0a
1c07 3f        ccf     
1c08 fb        ei      
1c09 c9        ret     

;;===========================================================================

1c0a f3        di      
1c0b 7d        ld      a,l
1c0c d631      sub     $31
1c0e 7c        ld      a,h
1c0f de00      sbc     a,$00
1c11 d8        ret     c

1c12 19        add     hl,de
1c13 222db6    ld      ($b62d),hl
1c16 eb        ex      de,hl
1c17 222bb6    ld      ($b62b),hl
1c1a 01300a    ld      bc,$0a30
1c1d 3601      ld      (hl),$01
1c1f 23        inc     hl
1c20 71        ld      (hl),c
1c21 23        inc     hl
1c22 0c        inc     c
1c23 10f8      djnz    $1c1d            ; (-$08)
1c25 eb        ex      de,hl

1c26 213c1c    ld      hl,$1c3c					;; default expansion values
1c29 0e0a      ld      c,$0a
1c2b edb0      ldir    

1c2d eb        ex      de,hl
1c2e 0613      ld      b,$13
1c30 af        xor     a
1c31 77        ld      (hl),a
1c32 23        inc     hl
1c33 10fc      djnz    $1c31            ; (-$04)
1c35 222fb6    ld      ($b62f),hl
1c38 3229b6    ld      ($b629),a
1c3b c9        ret     

1c3c 
defb &01
defb "."
defb &01
defb 13
defb &5
defb "RUN""",13

;;===========================================================================
;; KM SET EXPAND
1c46 78    	   ld 		a,b
1c47 cdc31c    call    $1cc3
1c4a d0        ret     nc

1c4b c5        push    bc
1c4c d5        push    de
1c4d e5        push    hl
1c4e cd6a1c    call    $1c6a
1c51 3f        ccf     
1c52 e1        pop     hl
1c53 d1        pop     de
1c54 c1        pop     bc
1c55 d0        ret     nc

1c56 1b        dec     de
1c57 79        ld      a,c
1c58 0c        inc     c
1c59 12        ld      (de),a
1c5a 13        inc     de
1c5b e7        rst     $20				; RST 4 - LOW: RAM LAM
1c5c 23        inc     hl
1c5d 0d        dec     c
1c5e 20f9      jr      nz,$1c59         ; (-$07)
1c60 2129b6    ld      hl,$b629
1c63 78        ld      a,b
1c64 ae        xor     (hl)
1c65 2001      jr      nz,$1c68         ; (+$01)
1c67 77        ld      (hl),a
1c68 37        scf     
1c69 c9        ret     

;;===========================================================================
1c6a 0600      ld      b,$00
1c6c 60        ld      h,b
1c6d 6f        ld      l,a
1c6e 79        ld      a,c
1c6f 95        sub     l
1c70 c8        ret     z

1c71 300f      jr      nc,$1c82         ; (+$0f)
1c73 7d        ld      a,l
1c74 69        ld      l,c
1c75 4f        ld      c,a
1c76 19        add     hl,de
1c77 eb        ex      de,hl
1c78 09        add     hl,bc
1c79 cda71c    call    $1ca7						
1c7c 2823      jr      z,$1ca1          ; (+$23)
1c7e edb0      ldir    
1c80 181f      jr      $1ca1            ; (+$1f)
1c82 4f        ld      c,a
1c83 19        add     hl,de
1c84 e5        push    hl
1c85 2a2fb6    ld      hl,($b62f)
1c88 09        add     hl,bc
1c89 eb        ex      de,hl
1c8a 2a2db6    ld      hl,($b62d)
1c8d 7d        ld      a,l
1c8e 93        sub     e
1c8f 7c        ld      a,h
1c90 9a        sbc     a,d
1c91 e1        pop     hl
1c92 d8        ret     c

1c93 cda71c    call    $1ca7			
1c96 2a2fb6    ld      hl,($b62f)
1c99 2806      jr      z,$1ca1          ; (+$06)
1c9b d5        push    de
1c9c 1b        dec     de
1c9d 2b        dec     hl
1c9e edb8      lddr    
1ca0 d1        pop     de
1ca1 ed532fb6  ld      ($b62f),de
1ca5 b7        or      a
1ca6 c9        ret     

;;===========================================================================

1ca7 3a2fb6    ld      a,($b62f)
1caa 95        sub     l
1cab 4f        ld      c,a
1cac 3a30b6    ld      a,($b630)
1caf 9c        sbc     a,h
1cb0 47        ld      b,a
1cb1 b1        or      c
1cb2 c9        ret     

;;===========================================================================
;; KM GET EXPAND

1cb3 cdc31c    call    $1cc3
1cb6 d0        ret     nc

1cb7 bd        cp      l
1cb8 c8        ret     z

1cb9 3f        ccf     
1cba d0        ret     nc

1cbb e5        push    hl
1cbc 2600      ld      h,$00
1cbe 19        add     hl,de
1cbf 7e        ld      a,(hl)
1cc0 e1        pop     hl
1cc1 37        scf     
1cc2 c9        ret     

;;===========================================================================

;; keycode above &7f not defineable?
1cc3 e67f      and     $7f
;; keys between &20-&7f are not defineable?
1cc5 fe20      cp      $20
1cc7 d0        ret     nc

1cc8 e5        push    hl
1cc9 2a2bb6    ld      hl,($b62b)
1ccc 110000    ld      de,$0000
1ccf 3c        inc     a
1cd0 19        add     hl,de
1cd1 5e        ld      e,(hl)
1cd2 23        inc     hl
1cd3 3d        dec     a
1cd4 20fa      jr      nz,$1cd0         ; (-$06)
1cd6 7b        ld      a,e
1cd7 eb        ex      de,hl
1cd8 e1        pop     hl
1cd9 37        scf     
1cda c9        ret     

;;===========================================================================
;; KM WAIT KEY

1cdb cde11c    call    $1ce1			; KM READ KEY
1cde 30fb      jr      nc,$1cdb         
1ce0 c9        ret     

;;===========================================================================
;; KM READ KEY

1ce1 e5        push    hl
1ce2 c5        push    bc
1ce3 cd9d1e    call    $1e9d
1ce6 303a      jr      nc,$1d22         ; (+$3a)
1ce8 79        ld      a,c
1ce9 feef      cp      $ef
1ceb 2834      jr      z,$1d21          ; (+$34)
1ced e60f      and     $0f
1cef 87        add     a,a
1cf0 87        add     a,a
1cf1 87        add     a,a
1cf2 3d        dec     a
1cf3 3c        inc     a
1cf4 cb08      rrc     b
1cf6 30fb      jr      nc,$1cf3         ; (-$05)
1cf8 cd251d    call    $1d25
1cfb 2132b6    ld      hl,$b632
1cfe cb7e      bit     7,(hl)
1d00 280a      jr      z,$1d0c          ; (+$0a)
1d02 fe61      cp      $61
1d04 3806      jr      c,$1d0c          ; (+$06)
1d06 fe7b      cp      $7b
1d08 3002      jr      nc,$1d0c         ; (+$02)
1d0a c6e0      add     a,$e0
1d0c feff      cp      $ff
1d0e 28d3      jr      z,$1ce3          ; (-$2d)
1d10 fefe      cp      $fe
1d12 2131b6    ld      hl,$b631
1d15 2805      jr      z,$1d1c          ; (+$05)
1d17 fefd      cp      $fd
1d19 23        inc     hl
1d1a 2005      jr      nz,$1d21         ; (+$05)
1d1c 7e        ld      a,(hl)
1d1d 2f        cpl     
1d1e 77        ld      (hl),a
1d1f 18c2      jr      $1ce3            ; (-$3e)
1d21 37        scf     
1d22 c1        pop     bc
1d23 e1        pop     hl
1d24 c9        ret     

;;===========================================================================

1d25 cb11      rl      c
1d27 dace1e    jp      c,$1ece			; KM GET CONTROL
1d2a 47        ld      b,a
1d2b 3a31b6    ld      a,($b631)
1d2e b1        or      c
1d2f e640      and     $40
1d31 78        ld      a,b
1d32 c2c91e    jp      nz,$1ec9			; KM GET SHIFT
1d35 c3c41e    jp      $1ec4			; KM GET TRANSLATE

;;===========================================================================
;; KM GET STATE

1d38 2a31b6    ld      hl,($b631)
1d3b c9        ret     

;;===========================================================================
;; KM SET LOCKS

1d3c 2231b6    ld      ($b631),hl
1d3f c9        ret     

;;===========================================================================
;; IND: KM SCAN KEYS

1d40 1149b6    ld      de,$b649			; buffer for keys that have changed
1d43 213fb6    ld      hl,$b63f			; buffer for current state of key matrix
										; if a bit is '0' then key is pressed,
										; if a bit is '1' then key is released.
1d46 cd8308    call    $0883			; scan keyboard

;;b635-b63e
;;b63f-b648
;;b649-b652 (keyboard line 0-10 inclusive)

1d49 3a4bb6    ld      a,($b64b)		; keyboard line 2
1d4c e6a0      and     $a0				; isolate change state of CTRL and SHIFT keys
1d4e 4f        ld      c,a

1d4f 2137b6    ld      hl,$b637
1d52 b6        or      (hl)
1d53 77        ld      (hl),a

;;----------------------------------------------------------------------
1d54 2149b6    ld      hl,$b649
1d57 1135b6    ld      de,$b635
1d5a 0600      ld      b,$00

1d5c 1a        ld      a,(de)
1d5d ae        xor     (hl)
1d5e a6        and     (hl)
1d5f c4d11d    call    nz,$1dd1
1d62 7e        ld      a,(hl)
1d63 12        ld      (de),a
1d64 23        inc     hl
1d65 13        inc     de
1d66 0c        inc     c
1d67 79        ld      a,c
1d68 e60f      and     $0f
1d6a fe0a      cp      $0a
1d6c 20ee      jr      nz,$1d5c         ; (-$12)

;;---------------------------------------------------------------------

1d6e 79        ld      a,c
1d6f e6a0      and     $a0
1d71 cb71      bit     6,c
1d73 4f        ld      c,a
1d74 c4eebd    call    nz,$bdee			; IND: KM TEST BREAK
1d77 78        ld      a,b
1d78 b7        or      a
1d79 c0        ret     nz

1d7a 2153b6    ld      hl,$b653
1d7d 35        dec     (hl)
1d7e c0        ret     nz

1d7f 2a54b6    ld      hl,($b654)
1d82 eb        ex      de,hl
1d83 42        ld      b,d
1d84 1600      ld      d,$00
1d86 2135b6    ld      hl,$b635
1d89 19        add     hl,de
1d8a 7e        ld      a,(hl)
1d8b 2a91b6    ld      hl,($b691)
1d8e 19        add     hl,de
1d8f a6        and     (hl)
1d90 a0        and     b
1d91 c8        ret     z

1d92 2153b6    ld      hl,$b653
1d95 34        inc     (hl)
1d96 3a8ab6    ld      a,($b68a)
1d99 b7        or      a
1d9a c0        ret     nz

1d9b 79        ld      a,c
1d9c b3        or      e
1d9d 4f        ld      c,a
1d9e 3a33b6    ld      a,($b633)

1da1 3253b6    ld      ($b653),a
1da4 cd861e    call    $1e86
1da7 79        ld      a,c
1da8 e60f      and     $0f
1daa 6f        ld      l,a
1dab 60        ld      h,b
1dac 2254b6    ld      ($b654),hl

1daf fe08      cp      $08
1db1 c0        ret     nz

1db2 cb60      bit     4,b
1db4 c0        ret     nz

1db5 cbf1      set     6,c
1db7 c9        ret     

;;=====================================================================================
;; IND: KM TEST BREAK

1db8 213db6    ld      hl,$b63d
1dbb cb56      bit     2,(hl)
1dbd c8        ret     z

1dbe 79        ld      a,c
1dbf eea0      xor     $a0
1dc1 2056      jr      nz,$1e19         ; KM BREAK EVENT
1dc3 c5        push    bc
1dc4 23        inc     hl
1dc5 060a      ld      b,$0a
1dc7 8e        adc     a,(hl)
1dc8 2b        dec     hl
1dc9 10fc      djnz    $1dc7            ; (-$04)
1dcb c1        pop     bc
1dcc fea4      cp      $a4
1dce 2049      jr      nz,$1e19         ; KM BREAK EVENT

;; do reset
1dd0 c7        rst     $00

;;====================================================================

1dd1 e5        push    hl
1dd2 d5        push    de
1dd3 5f        ld      e,a
1dd4 2f        cpl     
1dd5 3c        inc     a
1dd6 a3        and     e
1dd7 47        ld      b,a
1dd8 3a34b6    ld      a,($b634)
1ddb cda11d    call    $1da1
1dde 78        ld      a,b
1ddf ab        xor     e
1de0 20f1      jr      nz,$1dd3         ; (-$0f)
1de2 d1        pop     de
1de3 e1        pop     hl
1de4 c9        ret     

;;===========================================================================
;; KM GET JOYSTICK

1de5 3a3bb6    ld      a,($b63b)
1de8 e67f      and     $7f
1dea 6f        ld      l,a
1deb 3a3eb6    ld      a,($b63e)
1dee e67f      and     $7f
1df0 67        ld      h,a
1df1 c9        ret     

;;===========================================================================
;; KM GET DELAY

1df2 2a33b6    ld      hl,($b633)
1df5 c9        ret     

;;===========================================================================
;; KM SET DELAY

1df6 2233b6    ld      ($b633),hl
1df9 c9        ret     

;;===========================================================================
;; KM ARM BREAK

1dfa cd0b1e    call    $1e0b			; KM DISARM BREAK
1dfd 2157b6    ld      hl,$b657
1e00 0640      ld      b,$40
1e02 cdd201    call    $01d2			; KL INIT EVENT
1e05 3eff      ld      a,$ff
1e07 3256b6    ld      ($b656),a
1e0a c9        ret     

;;===========================================================================
;; KM DISARM BREAK

1e0b c5        push    bc
1e0c d5        push    de
1e0d 2156b6    ld      hl,$b656
1e10 3600      ld      (hl),$00
1e12 23        inc     hl
1e13 cd8402    call    $0284			; KL DEL SYNCHRONOUS
1e16 d1        pop     de
1e17 c1        pop     bc
1e18 c9        ret     

;;===========================================================================
;; KM BREAK EVENT

1e19 2156b6    ld      hl,$b656
1e1c 7e        ld      a,(hl)
1e1d 3600      ld      (hl),$00
1e1f be        cp      (hl)
1e20 c8        ret     z

1e21 c5        push    bc
1e22 d5        push    de
1e23 23        inc     hl
1e24 cde201    call    $01e2			; KL EVENT
1e27 0eef      ld      c,$ef
1e29 cd861e    call    $1e86
1e2c d1        pop     de
1e2d c1        pop     bc
1e2e c9        ret     

;;===========================================================================
;; KM GET REPEAT

1e2f 2a91b6    ld      hl,($b691)
1e32 181c      jr      $1e50            ; (+$1c)

;;===========================================================================
;; KM SET REPEAT

1e34 fe50      cp      $50
1e36 d0        ret     nc

1e37 2a91b6    ld      hl,($b691)
1e3a cd551e    call    $1e55
1e3d 2f        cpl     
1e3e 4f        ld      c,a
1e3f 7e        ld      a,(hl)
1e40 a8        xor     b
1e41 a1        and     c
1e42 a8        xor     b
1e43 77        ld      (hl),a
1e44 c9        ret     

;;===========================================================================
;; KM TEST KEY

1e45 f5        push    af
1e46 3a37b6    ld      a,($b637)
1e49 e6a0      and     $a0
1e4b 4f        ld      c,a
1e4c f1        pop     af
1e4d 2135b6    ld      hl,$b635
1e50 cd551e    call    $1e55
1e53 a6        and     (hl)
1e54 c9        ret     

;;===========================================================================

1e55 d5        push    de
1e56 f5        push    af
1e57 e6f8      and     $f8
1e59 0f        rrca    
1e5a 0f        rrca    
1e5b 0f        rrca    
1e5c 5f        ld      e,a
1e5d 1600      ld      d,$00
1e5f 19        add     hl,de
1e60 f1        pop     af

1e61 e5        push    hl
1e62 216d1e    ld      hl,$1e6d
1e65 e607      and     $07
1e67 5f        ld      e,a
1e68 19        add     hl,de
1e69 7e        ld      a,(hl)
1e6a e1        pop     hl
1e6b d1        pop     de
1e6c c9        ret     

;;===========================================================================
;; table to convert from bit index (0-7) to bit OR mask (1<<bit index)
1e6d 
defb &01,&02,&04,&08,&10,&20,&40,&80
;;===========================================================================

1e75 f3        di      
1e76 2186b6    ld      hl,$b686
1e79 3615      ld      (hl),$15
1e7b 23        inc     hl
1e7c af        xor     a
1e7d 77        ld      (hl),a
1e7e 23        inc     hl
1e7f 3601      ld      (hl),$01
1e81 23        inc     hl
1e82 77        ld      (hl),a
1e83 23        inc     hl
1e84 77        ld      (hl),a
1e85 c9        ret     

;;===========================================================================

1e86 2186b6    ld      hl,$b686
1e89 b7        or      a
1e8a 35        dec     (hl)
1e8b 280e      jr      z,$1e9b          ; (+$0e)
1e8d cdb41e    call    $1eb4
1e90 71        ld      (hl),c
1e91 23        inc     hl
1e92 70        ld      (hl),b
1e93 218ab6    ld      hl,$b68a
1e96 34        inc     (hl)
1e97 2188b6    ld      hl,$b688
1e9a 37        scf     
1e9b 34        inc     (hl)
1e9c c9        ret     

;;===========================================================================
1e9d 2188b6    ld      hl,$b688
1ea0 b7        or      a
1ea1 35        dec     (hl)
1ea2 280e      jr      z,$1eb2          ; (+$0e)
1ea4 cdb41e    call    $1eb4
1ea7 4e        ld      c,(hl)
1ea8 23        inc     hl
1ea9 46        ld      b,(hl)
1eaa 218ab6    ld      hl,$b68a
1ead 35        dec     (hl)
1eae 2186b6    ld      hl,$b686
1eb1 37        scf     
1eb2 34        inc     (hl)
1eb3 c9        ret     

;;===========================================================================
1eb4 23        inc     hl
1eb5 34        inc     (hl)
1eb6 7e        ld      a,(hl)
1eb7 fe14      cp      $14
1eb9 2002      jr      nz,$1ebd         ; (+$02)

1ebb af        xor     a
1ebc 77        ld      (hl),a

1ebd 87        add     a,a
1ebe c65e      add     a,$5e
1ec0 6f        ld      l,a
1ec1 26b6      ld      h,$b6
1ec3 c9        ret     

;;===========================================================================
;; KM GET TRANSLATE

1ec4 2a8bb6    ld      hl,($b68b)
1ec7 1808      jr      $1ed1            ; (+$08)

;;===========================================================================
;; KM GET SHIFT

1ec9 2a8db6    ld      hl,($b68d)
1ecc 1803      jr      $1ed1            ; (+$03)

;;===========================================================================
;; KM GET CONTROL 

1ece 2a8fb6    ld      hl,($b68f)

1ed1 85        add     a,l
1ed2 6f        ld      l,a
1ed3 8c        adc     a,h
1ed4 95        sub     l
1ed5 67        ld      h,a
1ed6 7e        ld      a,(hl)
1ed7 c9        ret     

;;===========================================================================
;; KM SET TRANSLATE

1ed8 2a8bb6    ld      hl,($b68b)
1edb 1808      jr      $1ee5            ; (+$08)

;;===========================================================================
;; KM SET SHIFT

1edd 2a8db6    ld      hl,($b68d)
1ee0 1803      jr      $1ee5            ; (+$03)

;;===========================================================================
;; KM SET CONTROL

1ee2 2a8fb6    ld      hl,($b68f)
1ee5 fe50      cp      $50
1ee7 d0        ret     nc

1ee8 85        add     a,l
1ee9 6f        ld      l,a
1eea 8c        adc     a,h
1eeb 95        sub     l
1eec 67        ld      h,a
1eed 70        ld      (hl),b
1eee c9        ret     

;;------------------------------------------------------
;; keyboard translation table

1eef f0        ret     p
1ef0 f3        di      
1ef1 f1        pop     af
1ef2 89        adc     a,c
1ef3 86        add     a,(hl)
1ef4 83        add     a,e
1ef5 8b        adc     a,e
1ef6 8a        adc     a,d
1ef7 f2e087    jp      p,$87e0
1efa 88        adc     a,b
1efb 85        add     a,l
1efc 81        add     a,c
1efd 82        add     a,d
1efe 80        add     a,b
1eff 105b      djnz    $1f5c            ; (+$5b)
1f01 0d        dec     c
1f02 5d        ld      e,l
1f03 84        add     a,h
1f04 ff        rst     $38
1f05 5c        ld      e,h
1f06 ff        rst     $38
1f07 5e        ld      e,(hl)
1f08 2d        dec     l
1f09 40        ld      b,b
1f0a 70        ld      (hl),b
1f0b 3b        dec     sp
1f0c 3a2f2e    ld      a,($2e2f)
1f0f 3039      jr      nc,$1f4a         ; (+$39)
1f11 6f        ld      l,a
1f12 69        ld      l,c
1f13 6c        ld      l,h
1f14 6b        ld      l,e
1f15 6d        ld      l,l
1f16 2c        inc     l
1f17 3837      jr      c,$1f50          ; (+$37)
1f19 75        ld      (hl),l
1f1a 79        ld      a,c
1f1b 68        ld      l,b
1f1c 6a        ld      l,d
1f1d 6e        ld      l,(hl)
1f1e 2036      jr      nz,$1f56         ; (+$36)
1f20 35        dec     (hl)
1f21 72        ld      (hl),d
1f22 74        ld      (hl),h
1f23 67        ld      h,a
1f24 66        ld      h,(hl)
1f25 62        ld      h,d
1f26 76        halt    
1f27 34        inc     (hl)
1f28 33        inc     sp
1f29 65        ld      h,l
1f2a 77        ld      (hl),a
1f2b 73        ld      (hl),e
1f2c 64        ld      h,h
1f2d 63        ld      h,e
1f2e 78        ld      a,b
1f2f 3132fc    ld      sp,$fc32
1f32 71        ld      (hl),c
1f33 09        add     hl,bc
1f34 61        ld      h,c
1f35 fd7a      ld      a,d
1f37 0b        dec     bc
1f38 0a        ld      a,(bc)
1f39 08        ex      af,af'
1f3a 09        add     hl,bc
1f3b 58        ld      e,b
1f3c 5a        ld      e,d
1f3d ff        rst     $38
1f3e 7f        ld      a,a
1f3f f4f7f5    call    p,$f5f7
1f42 89        adc     a,c
1f43 86        add     a,(hl)
1f44 83        add     a,e
1f45 8b        adc     a,e
1f46 8a        adc     a,d
1f47 f6e0      or      $e0
1f49 87        add     a,a
1f4a 88        adc     a,b
1f4b 85        add     a,l
1f4c 81        add     a,c
1f4d 82        add     a,d
1f4e 80        add     a,b
1f4f 107b      djnz    $1fcc            ; (+$7b)
1f51 0d        dec     c
1f52 7d        ld      a,l
1f53 84        add     a,h
1f54 ff        rst     $38
1f55 60        ld      h,b
1f56 ff        rst     $38
1f57 a3        and     e
1f58 3d        dec     a
1f59 7c        ld      a,h
1f5a 50        ld      d,b
1f5b 2b        dec     hl
1f5c 2a3f3e    ld      hl,($3e3f)
1f5f 5f        ld      e,a
1f60 29        add     hl,hl
1f61 4f        ld      c,a
1f62 49        ld      c,c
1f63 4c        ld      c,h
1f64 4b        ld      c,e
1f65 4d        ld      c,l
1f66 3c        inc     a
1f67 2827      jr      z,$1f90          ; (+$27)
1f69 55        ld      d,l
1f6a 59        ld      e,c
1f6b 48        ld      c,b
1f6c 4a        ld      c,d
1f6d 4e        ld      c,(hl)
1f6e 2026      jr      nz,$1f96         ; (+$26)
1f70 25        dec     h
1f71 52        ld      d,d
1f72 54        ld      d,h
1f73 47        ld      b,a
1f74 46        ld      b,(hl)
1f75 42        ld      b,d
1f76 56        ld      d,(hl)
1f77 24        inc     h
1f78 23        inc     hl
1f79 45        ld      b,l
1f7a 57        ld      d,a
1f7b 53        ld      d,e
1f7c 44        ld      b,h
1f7d 43        ld      b,e
1f7e 58        ld      e,b
1f7f 2122fc    ld      hl,$fc22
1f82 51        ld      d,c
1f83 09        add     hl,bc
1f84 41        ld      b,c
1f85 fd5a      ld      e,d
1f87 0b        dec     bc
1f88 0a        ld      a,(bc)
1f89 08        ex      af,af'
1f8a 09        add     hl,bc
1f8b 58        ld      e,b
1f8c 5a        ld      e,d
1f8d ff        rst     $38
1f8e 7f        ld      a,a
1f8f f8        ret     m

1f90 fb        ei      
1f91 f9        ld      sp,hl
1f92 89        adc     a,c
1f93 86        add     a,(hl)
1f94 83        add     a,e
1f95 8c        adc     a,h
1f96 8a        adc     a,d
1f97 fae087    jp      m,$87e0
1f9a 88        adc     a,b
1f9b 85        add     a,l
1f9c 81        add     a,c
1f9d 82        add     a,d
1f9e 80        add     a,b
1f9f 101b      djnz    $1fbc            ; (+$1b)
1fa1 0d        dec     c
1fa2 1d        dec     e
1fa3 84        add     a,h
1fa4 ff        rst     $38
1fa5 1c        inc     e
1fa6 ff        rst     $38
1fa7 1eff      ld      e,$ff
1fa9 00        nop     
1faa 10ff      djnz    $1fab            ; (-$01)
1fac ff        rst     $38
1fad ff        rst     $38
1fae ff        rst     $38
1faf 1f        rra     
1fb0 ff        rst     $38
1fb1 0f        rrca    
1fb2 09        add     hl,bc
1fb3 0c        inc     c
1fb4 0b        dec     bc
1fb5 0d        dec     c
1fb6 ff        rst     $38
1fb7 ff        rst     $38
1fb8 ff        rst     $38
1fb9 15        dec     d
1fba 19        add     hl,de
1fbb 08        ex      af,af'
1fbc 0a        ld      a,(bc)
1fbd 0eff      ld      c,$ff
1fbf ff        rst     $38
1fc0 ff        rst     $38
1fc1 12        ld      (de),a
1fc2 14        inc     d
1fc3 07        rlca    
1fc4 0602      ld      b,$02
1fc6 16ff      ld      d,$ff
1fc8 ff        rst     $38
1fc9 05        dec     b
1fca 17        rla     
1fcb 13        inc     de
1fcc 04        inc     b
1fcd 03        inc     bc
1fce 18ff      jr      $1fcf            ; (-$01)
1fd0 7e        ld      a,(hl)
1fd1 fc11e1    call    m,$e111
1fd4 01fe1a    ld      bc,$1afe
1fd7 ff        rst     $38
1fd8 ff        rst     $38
1fd9 ff        rst     $38
1fda ff        rst     $38
1fdb ff        rst     $38
1fdc ff        rst     $38
1fdd ff        rst     $38
1fde 7f        ld      a,a
1fdf 07        rlca    
1fe0 03        inc     bc
1fe1 4b        ld      c,e
1fe2 ff        rst     $38
1fe3 ff        rst     $38
1fe4 ff        rst     $38
1fe5 ff        rst     $38
1fe6 ff        rst     $38
1fe7 ab        xor     e
1fe8 8f        adc     a,a


;;============================================================================
;; SOUND RESET

;; for each channel:
;; $00 - channel number (0,1,2)
;; $01 - mixer value for tone (also used for active mask)
;; $02 - mixer value for noise
;; $03 - status
;; status bit 0=rendezvous channel A
;; status bit 1=rendezvous channel B
;; status bit 2=rendezvous channel C
;; status bit 3=hold

;; $04 - bit 0 = tone envelope active
;; $07 - bit 0 = volume envelope active

;; $08,$09 - duration of sound or envelope repeat count
;; $0a,$0b - volume envelope pointer reload
;; $0c - volume envelope step down count
;; $0d,$0e - current volume envelope pointer
;; $0f - current volume for channel (bit 7 set if has noise)
;; $10 - volume envelope current step down count

;; $11,$12 - tone envelope pointer reload
;; $13 - number of sections in tone remaining
;; $14,$15 - current tone pointer
;; $16 - low byte tone for channel
;; $17 - high byte tone for channel
;; $18 - tone envelope current step down count

;; $19 - read position in queue
;; $1a - number of items in the queue
;; $1b - write position in queue
;; $1c - number of items free in queue
;; $1d - low byte event 
;; $1e - high byte event (set to 0 to disarm event)



1fe9 21edb1    ld      hl,$b1ed			; channels active at SOUND HOLD

;; clear flags
;; b1ed - channels active at SOUND HOLD
;; b1ee - sound channels active
;; b1ef - sound timer?
;; b1f0 - ??
1fec 0604      ld      b,$04
1fee 3600      ld      (hl),$00		
1ff0 23        inc     hl
1ff1 10fb      djnz    $1fee            

;; HL  = event block (b1f1)
1ff3 118b20    ld      de,$208b			;; sound event function
1ff6 0681      ld      b,$81			;; asynchronous event, near address
										;; C = rom select, but unused because it's a near address
1ff8 cdd201    call    $01d2			; KL INIT EVENT

1ffb 3e3f      ld      a,$3f			; default mixer value (noise/tone off + I/O)
1ffd 32b5b2    ld      ($b2b5),a

2000 21f8b1    ld      hl,$b1f8			;; data for channel A
2003 013d00    ld      bc,$003d			;; size of data for each channel
2006 110801    ld      de,$0108			;; D = mixer value for tone (channel A)
										;; E = mixer value for noise (channel A)

;; initialise channel data
2009 af        xor     a

200a 77        ld      (hl),a 			;; channel number
200b 23        inc     hl
200c 72        ld      (hl),d			;; mixer tone for channel
200d 23        inc     hl
200e 73        ld      (hl),e			;; mixer noise for channel
200f 09        add     hl,bc			;; update channel data pointer

2010 3c        inc     a				;; increment channel number

2011 eb        ex      de,hl			;; update tone/noise mixer for next channel shifting it left once
2012 29        add     hl,hl
2013 eb        ex      de,hl

2014 fe03      cp      $03				;; setup all channels?
2016 20f2      jr      nz,$200a         

2018 0e07      ld      c,$07			; all channels active
201a dde5      push    ix
201c e5        push    hl
201d 21f0b1    ld      hl,$b1f0
2020 34        inc     (hl)
2021 e5        push    hl
2022 dd21b9b1  ld      ix,$b1b9
2026 79        ld      a,c				; channels active

2027 cd0922    call    $2209			;; get next active channel
202a f5        push    af
202b c5        push    bc
202c cd8622    call    $2286			;; update channels that are active
202f cde723    call    $23e7			;; disable channel
2032 dde5      push    ix
2034 d1        pop     de
2035 13        inc     de
2036 13        inc     de
2037 13        inc     de
2038 6b        ld      l,e
2039 62        ld      h,d
203a 13        inc     de
203b 013b00    ld      bc,$003b
203e 3600      ld      (hl),$00
2040 edb0      ldir    
2042 dd361c04  ld      (ix+$1c),$04		;; number of spaces in queue
2046 c1        pop     bc
2047 f1        pop     af
2048 20dd      jr      nz,$2027         ; (-$23)


204a e1        pop     hl
204b 35        dec     (hl)
204c e1        pop     hl
204d dde1      pop     ix
204f c9        ret     

;;==========================================================================
;; SOUND HOLD
;;
;; - Stop firmware handling sound
;; - turn off all volume registers
;;
;; carry false - already stopped
;; carry true - sound has been held
2050 21eeb1    ld      hl,$b1ee				;; sound channels active
2053 f3        di      
2054 7e        ld      a,(hl)				;; get channels that were active
2055 3600      ld      (hl),$00 			;; no channels active
2057 fb        ei      
2058 b7        or      a					;; already stopped?
2059 c8        ret     z

205a 2b        dec     hl					
205b 77        ld      (hl),a				;; channels held

;; set all AY volume registers to zero to silence sound
205c 2e03      ld      l,$03
205e 0e00      ld      c,$00			; set zero volume

2060 3e07      ld      a,$07			; AY Mixer register
2062 85        add     a,l				; add on value to get volume register
										; A = AY volume register (10,9,8)
2063 cd6308    call    $0863			; MC SOUND REGISTER
2066 2d        dec     l
2067 20f7      jr      nz,$2060         
 
2069 37        scf     
206a c9        ret     


;;==========================================================================
;; SOUND CONTINUE

206b 11edb1    ld      de,$b1ed	;; channels active at SOUND HELD
206e 1a        ld      a,(de)
206f b7        or      a
2070 c8        ret     z

;; at least one channel was held

2071 d5        push    de
2072 dd21b9b1  ld      ix,$b1b9
2076 cd0922    call    $2209			; get next active channel
2079 f5        push    af
207a dd7e0f    ld      a,(ix+$0f)		; volume for channel
207d dcde23    call    c,$23de			; set channel volume
2080 f1        pop     af
2081 20f3      jr      nz,$2076         ;repeat next held channel

2083 e3        ex      (sp),hl
2084 7e        ld      a,(hl)
2085 3600      ld      (hl),$00
2087 23        inc     hl
2088 77        ld      (hl),a
2089 e1        pop     hl
208a c9        ret     

;;===============================================================================
;; sound processing function

208b dde5      push    ix
208d 3aeeb1    ld      a,($b1ee)		; sound channels active
2090 b7        or      a
2091 283d      jr      z,$20d0          

;; A = channel to process
2093 f5        push    af
2094 dd21b9b1  ld      ix,$b1b9
2098 013f00    ld      bc,$003f
209b dd09      add     ix,bc
209d cb3f      srl     a
209f 30fa      jr      nc,$209b         

20a1 f5        push    af
20a2 dd7e04    ld      a,(ix+$04)
20a5 1f        rra     
20a6 dc1f24    call    c,$241f			; update tone envelope

20a9 dd7e07    ld      a,(ix+$07)
20ac 1f        rra     
20ad dc1f23    call    c,$231f			; update volume envelope

20b0 dc1322    call    c,$2213			; process queue
20b3 f1        pop     af
20b4 20e2      jr      nz,$2098         ;; process next..?

20b6 c1        pop     bc
20b7 3aeeb1    ld      a,($b1ee)		; sound channels active
20ba 2f        cpl     
20bb a0        and     b
20bc 2812      jr      z,$20d0          ; (+$12)

20be dd21b9b1  ld      ix,$b1b9
20c2 113f00    ld      de,$003f
20c5 dd19      add     ix,de
20c7 cb3f      srl     a
20c9 f5        push    af
20ca dce723    call    c,$23e7			; mixer
20cd f1        pop     af
20ce 20f5      jr      nz,$20c5         ; (-$0b)

;; ???
20d0 af        xor     a
20d1 32f0b1    ld      ($b1f0),a
20d4 dde1      pop     ix
20d6 c9        ret     

;;------------------------------------------------------------------
;; process sound

20d7 21eeb1    ld      hl,$b1ee		;; sound active flag?
20da 7e        ld      a,(hl)
20db b7        or      a
20dc c8        ret     z
;; sound is active

20dd 23        inc     hl			;; sound timer?
20de 35        dec     (hl)
20df c0        ret     nz

20e0 47        ld      b,a
20e1 34        inc     (hl)
20e2 23        inc     hl

20e3 7e        ld      a,(hl)		;; b1f0
20e4 b7        or      a
20e5 c0        ret     nz

20e6 2b        dec     hl
20e7 3603      ld      (hl),$03

20e9 21beb1    ld      hl,$b1be
20ec 113f00    ld      de,$003f
20ef af        xor     a
20f0 19        add     hl,de
20f1 cb38      srl     b
20f3 30fb      jr      nc,$20f0         ; (-$05)

20f5 35        dec     (hl)
20f6 2005      jr      nz,$20fd         ; (+$05)
20f8 2b        dec     hl
20f9 cb06      rlc     (hl)
20fb 8a        adc     a,d
20fc 23        inc     hl
20fd 23        inc     hl
20fe 35        dec     (hl)
20ff 2005      jr      nz,$2106         ; (+$05)
2101 23        inc     hl
2102 cb06      rlc     (hl)
2104 8a        adc     a,d
2105 2b        dec     hl
2106 2b        dec     hl
2107 04        inc     b
2108 10e6      djnz    $20f0            ; (-$1a)
210a b7        or      a
210b c8        ret     z

210c 21f0b1    ld      hl,$b1f0
210f 77        ld      (hl),a
2110 23        inc     hl
;; HL = event block
;; kick off event
2111 c3e201    jp      $01e2			; KL EVENT


;;============================================================================
;; SOUND QUEUE
;; HL = sound data
;;byte 0 - channel status byte 
;; bit 0 = send sound to channel A
;; bit 1 = send sound to channel B
;; bit 2 = send sound to channel C
;; bit 3 = rendezvous with channel A
;; bit 4 = rendezvous with channel B
;; bit 5 = rendezvous with channel C
;; bit 6 = hold sound channel
;; bit 7 = flush sound channel

;;byte 1 - volume envelope to use 
;;byte 2 - tone envelope to use 
;;bytes 3&4 - tone period (0 = no tone)
;;byte 5 - noise period (0 = no noise)
;;byte 6 - start volume 
;;bytes 7&8 - duration of the sound, or envelope repeat count 


2114 cd6b20    call    $206b			; SOUND CONTINUE
2117 7e        ld      a,(hl)			; channel status byte
2118 e607      and     $07
211a 37        scf     
211b c8        ret     z

211c 4f        ld      c,a
211d b6        or      (hl)
211e fc1a20    call    m,$201a
2121 41        ld      b,c
2122 dd21b9b1  ld      ix,$b1b9
;; get channel address
2126 113f00    ld      de,$003f
2129 af        xor     a

212a dd19      add     ix,de
212c cb38      srl     b
212e 30fa      jr      nc,$212a         ; (-$06)

2130 dd721e    ld      (ix+$1e),d		;; disarm event
2133 ddbe1c    cp      (ix+$1c)			;; number of spaces in queue
2136 3f        ccf     
2137 9f        sbc     a,a
2138 04        inc     b
2139 10ef      djnz    $212a            

213b b7        or      a
213c c0        ret     nz

213d 41        ld      b,c
213e 7e        ld      a,(hl)			;; channel status
213f 1f        rra     
2140 1f        rra     
2141 1f        rra     
2142 b0        or      b
2143 e60f      and     $0f
2145 4f        ld      c,a
2146 e5        push    hl
2147 21f0b1    ld      hl,$b1f0
214a 34        inc     (hl)
214b e3        ex      (sp),hl
214c 23        inc     hl
214d dd21b9b1  ld      ix,$b1b9

2151 113f00    ld      de,$003f
2154 dd19      add     ix,de
2156 cb38      srl     b
2158 30fa      jr      nc,$2154         ; (-$06)

215a e5        push    hl
215b c5        push    bc
215c dd7e1b    ld      a,(ix+$1b)		; write pointer in queue
215f dd341b    inc     (ix+$1b)			; increment for next item
2162 dd351c    dec     (ix+$1c)			;; number of spaces in queue
2165 eb        ex      de,hl
2166 cd9c21    call    $219c		;; get sound queue slot
2169 e5        push    hl
216a eb        ex      de,hl
216b dd7e01    ld      a,(ix+$01)		;; channel's active flag
216e 2f        cpl     
216f a1        and     c
2170 12        ld      (de),a
2171 13        inc     de
2172 7e        ld      a,(hl)
2173 23        inc     hl
2174 87        add     a,a
2175 87        add     a,a
2176 87        add     a,a
2177 87        add     a,a
2178 47        ld      b,a
2179 7e        ld      a,(hl)
217a 23        inc     hl
217b e60f      and     $0f
217d b0        or      b
217e 12        ld      (de),a
217f 13        inc     de
2180 010600    ld      bc,$0006
2183 edb0      ldir    
2185 e1        pop     hl
2186 dd7e1a    ld      a,(ix+$1a)		;; number of items in the queue
2189 dd341a    inc     (ix+$1a)			
218c ddb603    or      (ix+$03)			;; status
218f cc1f22    call    z,$221f
2192 c1        pop     bc
2193 e1        pop     hl
2194 04        inc     b
2195 10ba      djnz    $2151            

2197 e3        ex      (sp),hl
2198 35        dec     (hl)
2199 e1        pop     hl
219a 37        scf     
219b c9        ret     

;; A = index in queue
219c e603      and     $03
219e 87        add     a,a		
219f 87        add     a,a
21a0 87        add     a,a
21a1 c61f      add     a,$1f
21a3 dde5      push    ix
21a5 e1        pop     hl
21a6 85        add     a,l
21a7 6f        ld      l,a
21a8 8c        adc     a,h
21a9 95        sub     l
21aa 67        ld      h,a
21ab c9        ret     

;;==========================================================================
;; SOUND RELEASE

21ac 6f        ld      l,a
21ad cd6b20    call    $206b			; SOUND CONTINUE
21b0 7d        ld      a,l
21b1 e607      and     $07
21b3 c8        ret     z

21b4 21f0b1    ld      hl,$b1f0
21b7 34        inc     (hl)
21b8 e5        push    hl
21b9 dd21b9b1  ld      ix,$b1b9
21bd cd0922    call    $2209			; get next active channel
21c0 f5        push    af
21c1 ddcb035e  bit     3,(ix+$03)		 ; held?
21c5 c41922    call    nz,$2219			 ; process queue item
21c8 f1        pop     af
21c9 20f2      jr      nz,$21bd         ; (-$0e)
21cb e1        pop     hl
21cc 35        dec     (hl)
21cd c9        ret     


;;============================================================================
;; SOUND CHECK
;; in:
;; bit 0 = channel 0
;; bit 1 = channel 1
;; bit 2 = channel 2
;;
;; result:
;; xxxxx000 - not allowed
;; xxxxx001 - 0
;; xxxxx010 - 1
;; xxxxx011 - 0
;; xxxxx100 - 2
;; xxxxx101 - 0
;; xxxxx110 - 1
;; xxxxx111 - 2
;; out:
;;bits 0 to 2 - the number of free spaces in the sound queue 
;;bit 3 - trying to rendezvous with channel A 
;;bit 4 - trying to rendezvous with channel B 
;;bit 5 - trying to rendezvous with channel C 
;;bit 6 - holding the channel 
;;bit 7 - producing a sound 

21ce e607      and     $07
21d0 c8        ret     z

21d1 21bcb1    ld      hl,$b1bc			;; sound data - 63
21d4 113f00    ld      de,$003f			;; 63

21d7 19        add     hl,de
21d8 1f        rra     
21d9 30fc      jr      nc,$21d7        ;; bit a zero?

21db f3        di      
21dc 7e        ld      a,(hl)
21dd 87        add     a,a		;; x2
21de 87        add     a,a		;; x4
21df 87        add     a,a		;; x8
21e0 111900    ld      de,$0019
21e3 19        add     hl,de
21e4 b6        or      (hl)
21e5 23        inc     hl
21e6 23        inc     hl
21e7 3600      ld      (hl),$00
21e9 fb        ei      
21ea c9        ret     

;;============================================================================
;; SOUND ARM EVENT
;; 
;; Sets up an event which will be activated when a space occurs in a sound queue.
;; if there is space the event is kicked immediately.
;;
;;
;; A:
;; bit 0 = channel 0
;; bit 1 = channel 1
;; bit 2 = channel 2
;; 
;; result:
;; xxxxx000 - not allowed
;; xxxxx001 - 0
;; xxxxx010 - 1
;; xxxxx011 - 0
;; xxxxx100 - 2
;; xxxxx101 - 0
;; xxxxx110 - 1
;; xxxxx111 - 2
;;
;; HL = event function
21eb e607      and     $07
21ed c8        ret     z

21ee eb        ex      de,hl			;; DE = event function

;; get address of data
21ef 21d5b1    ld      hl,$b1d5
21f2 013f00    ld      bc,$003f
21f5 09        add     hl,bc
21f6 1f        rra     
21f7 30fc      jr      nc,$21f5         

21f9 af        xor     a				;; 0=no space in queue. !=0  space in the queue
21fa f3        di      					;; stop event processing changing the value (this is a data fence)
21fb be        cp      (hl)				;; +$1c -> number of events remaining in queue
21fc 2001      jr      nz,$21ff         ;; if it has space, disarm and call

;; no space in the queue, arm the event
21fe 7a        ld      a,d

;; write function
21ff 23        inc     hl
2200 73        ld      (hl),e			;; +$1d
2201 23        inc     hl
2202 77        ld      (hl),a			;; +$1e if zero means event is disarmed
2203 fb        ei      
2204 c8        ret     z				;; queue is full
;; queue has space
2205 eb        ex      de,hl			 
2206 c3e201    jp      $01e2			; KL EVENT

;;==================================================================================
;; get next active channel
;; A = channel mask (updated)
;; IX = channel pointer
2209 113f00    ld      de,$003f			; 63
220c dd19      add     ix,de
220e cb3f      srl     a
2210 d8        ret     c
2211 18f9      jr      $220c            ; (-$07)

;;==================================================================================

2213 dd7e1a    ld      a,(ix+$1a)		; has items in the queue
2216 b7        or      a
2217 286d      jr      z,$2286          

;; process queue item
2219 dd7e19    ld      a,(ix+$19)		; read pointer in queue
221c cd9c21    call    $219c			; get sound queue slot

;;----------------------------
221f 7e        ld      a,(hl)			; channel status byte
;; bit 0=rendezvous channel A
;; bit 1=rendezvous channel B
;; bit 2=rendezvous channel C
;; bit 3=hold
2220 b7        or      a
2221 280d      jr      z,$2230          

2223 cb5f      bit     3,a				; hold channel?
2225 2059      jr      nz,$2280         ; 

2227 e5        push    hl
2228 3600      ld      (hl),$00
222a cd9022    call    $2290			; process rendezvous
222d e1        pop     hl
222e 3056      jr      nc,$2286         

2230 dd360310  ld      (ix+$03),$10		; playing

2234 23        inc     hl
2235 7e        ld      a,(hl)			; 	
2236 e6f0      and     $f0
2238 f5        push    af
2239 ae        xor     (hl)
223a 5f        ld      e,a				; tone envelope number
223b 23        inc     hl
223c 4e        ld      c,(hl)			; tone low
223d 23        inc     hl
223e 56        ld      d,(hl)			; tone period high
223f 23        inc     hl

2240 b2        or      d				; tone period set?
2241 b1        or      c
2242 2808      jr      z,$224c          
;; 
2244 e5        push    hl
2245 cd0824    call    $2408			; set tone and tone envelope	
2248 dd5601    ld      d,(ix+$01)		; tone mixer value
224b e1        pop     hl

224c 4e        ld      c,(hl)			; noise
224d 23        inc     hl
224e 5e        ld      e,(hl)			; start volume
224f 23        inc     hl
2250 7e        ld      a,(hl)			; duration of sound or envelope repeat count
2251 23        inc     hl
2252 66        ld      h,(hl)
2253 6f        ld      l,a
2254 f1        pop     af
2255 cdde22    call    $22de			;; set noise

2258 21eeb1    ld      hl,$b1ee			;; channel active flag
225b dd4601    ld      b,(ix+$01)		;; channels' active flag
225e 7e        ld      a,(hl)
225f b0        or      b
2260 77        ld      (hl),a
2261 a8        xor     b
2262 2003      jr      nz,$2267         ; (+$03)

2264 23        inc     hl
2265 3603      ld      (hl),$03

2267 dd3419    inc     (ix+$19)			;; increment read position in queue
226a dd351a    dec     (ix+$1a)			;; number of items in the queue
;; 
226d dd341c    inc     (ix+$1c)			;; increase space in the queue

;; there is a space in the queue...
2270 dd7e1e    ld      a,(ix+$1e)		;; high byte of event (0=disarmed)
2273 dd361e00  ld      (ix+$1e),$00		;; disarm event
2277 b7        or      a
2278 c8        ret     z

;; event is armed, kick it off.
2279 67        ld      h,a
227a dd6e1d    ld      l,(ix+$1d)
227d c3e201    jp      $01e2			; KL EVENT

;;=============================================================================

;; ?
2280 cb9e      res     3,(hl)
2282 dd360308  ld      (ix+$03),$08		;; held

;; stop sound?
2286 21eeb1    ld      hl,$b1ee			;; sound channels active flag
2289 dd7e01    ld      a,(ix+$01)		;; channels' active flag
228c 2f        cpl     
228d a6        and     (hl)
228e 77        ld      (hl),a
228f c9        ret     

;;==============================================================
;; process rendezvous
2290 dde5      push    ix
2292 47        ld      b,a
2293 dd4e01    ld      c,(ix+$01)		;; channels' active flag
2296 dd21f8b1  ld      ix,$b1f8			;; channel A's data
229a cb47      bit     0,a
229c 200c      jr      nz,$22aa         

229e dd2137b2  ld      ix,$b237			;; channel B's data
22a2 cb4f      bit     1,a
22a4 2004      jr      nz,$22aa        
22a6 dd2176b2  ld      ix,$b276			;; channel C's data

22aa dd7e03    ld      a,(ix+$03)		; channels' rendezvous flags
22ad a1        and     c				; ignore rendezvous with self.
22ae 2827      jr      z,$22d7
          
22b0 78        ld      a,b
22b1 ddbe01    cp      (ix+$01)			; channels' active flag
22b4 2819      jr      z,$22cf          ; ignore rendezvous with self (process own queue)

22b6 dde5      push    ix
22b8 dd2176b2  ld      ix,$b276			; channel C's data
22bc cb57      bit     2,a				; rendezvous channel C
22be 2004      jr      nz,$22c4         
22c0 dd2137b2  ld      ix,$b237			; channel B's data

22c4 dd7e03    ld      a,(ix+$03)		; channels' rendezvous flags
22c7 a1        and     c				; ignore rendezvous with self.
22c8 280c      jr      z,$22d6          
;; process us/other

22ca cd1922    call    $2219			; process queue item
22cd dde1      pop     ix
22cf cd1922    call    $2219			; process queue item
22d2 dde1      pop     ix
22d4 37        scf     
22d5 c9        ret     

22d6 e1        pop     hl
22d7 dde1      pop     ix
22d9 dd7003    ld      (ix+$03),b		; status
22dc b7        or      a
22dd c9        ret     


;;=================================================================================

;; set initial values
;; C = noise value
;; E = initial volume
;; HL = duration of sound or envelope repeat count
22de cbfb      set     7,e
22e0 dd730f    ld      (ix+$0f),e		;; volume for channel?
22e3 5f        ld      e,a

;; duration of sound or envelope repeat count
22e4 7d        ld      a,l
22e5 b4        or      h
22e6 2001      jr      nz,$22e9        

22e8 2b        dec     hl
22e9 dd7508    ld      (ix+$08),l		; duration of sound or envelope repeat count
22ec dd7409    ld      (ix+$09),h

22ef 79        ld      a,c				; if zero do not set noise
22f0 b7        or      a
22f1 2808      jr      z,$22fb          

22f3 3e06      ld      a,$06			; PSG noise register
22f5 cd6308    call    $0863			; MC SOUND REGISTER
22f8 dd7e02    ld      a,(ix+$02)

22fb b2        or      d
22fc cde823    call    $23e8			; mixer for channel
22ff 7b        ld      a,e
2300 b7        or      a
2301 280a      jr      z,$230d          

2303 21a6b2    ld      hl,$b2a6
2306 1600      ld      d,$00
2308 19        add     hl,de
2309 7e        ld      a,(hl)			
230a b7        or      a
230b 2003      jr      nz,$2310         

230d 211b23    ld      hl,$231b			; default volume envelope	
2310 dd750a    ld      (ix+$0a),l
2313 dd740b    ld      (ix+$0b),h
2316 cdcd23    call    $23cd			; set volume envelope?
2319 180d      jr      $2328            ; (+$0d)

;;=================================================================================
;; default volume envelope
231b 
defb 1 ;; step count
defb 1 ;; step size
defb 0 ;; pause time

;; unused?
defb &c8

;;=================================================================================
;; update volume envelope
231f dd6e0d    ld      l,(ix+$0d)		; volume envelope pointer
2322 dd660e    ld      h,(ix+$0e)
2325 dd5e10    ld      e,(ix+$10)		; step count	

2328 7b        ld      a,e
2329 feff      cp      $ff
232b 2875      jr      z,$23a2          ; no tone/volume envelopes active


232d 87        add     a,a				
232e 7e        ld      a,(hl)			; reload envelope shape/step count
232f 23        inc     hl
2330 3849      jr      c,$237b          ; set hardware envelope (HL) = hardware envelope value
2332 280c      jr      z,$2340          ; set volume

2334 1d        dec     e				; decrease step count

2335 dd4e0f    ld      c,(ix+$0f)		;; 
2338 b7        or      a
2339 2004      jr      nz,$233f         ;

233b cb79      bit     7,c				; has noise
233d 2806      jr      z,$2345    		;       

;; 
233f 81        add     a,c


2340 e60f      and     $0f
2342 cddb23    call    $23db			; write volume for channel and store

2345 4e        ld      c,(hl)
2346 dd7e09    ld      a,(ix+$09)
2349 47        ld      b,a
234a 87        add     a,a
234b 381b      jr      c,$2368          ; (+$1b)
234d af        xor     a
234e 91        sub     c
234f dd8608    add     a,(ix+$08)
2352 380c      jr      c,$2360          ; (+$0c)
2354 05        dec     b
2355 f25d23    jp      p,$235d
2358 dd4e08    ld      c,(ix+$08)
235b af        xor     a
235c 47        ld      b,a
235d dd7009    ld      (ix+$09),b
2360 dd7708    ld      (ix+$08),a
2363 b0        or      b
2364 2002      jr      nz,$2368         ; (+$02)
2366 1eff      ld      e,$ff
2368 7b        ld      a,e
2369 b7        or      a
236a ccae23    call    z,$23ae
236d dd7310    ld      (ix+$10),e
2370 f3        di      
2371 dd7106    ld      (ix+$06),c
2374 dd360780  ld      (ix+$07),$80		; has tone envelope
2378 fb        ei      
2379 b7        or      a
237a c9        ret     

;; E = hardware envelope shape
;; D = hardware envelope period low
;; (HL) = hardware envelope period high

;; DE = hardware envelope period
237b 57        ld      d,a
237c 4b        ld      c,e
237d 3e0d      ld      a,$0d			; PSG hardware volume shape register
237f cd6308    call    $0863			; MC SOUND REGISTER
2382 4a        ld      c,d
2383 3e0b      ld      a,$0b			; PSG hardware volume period low
2385 cd6308    call    $0863			; MC SOUND REGISTER
2388 4e        ld      c,(hl)
2389 3e0c      ld      a,$0c			; PSG hardware volume period high
238b cd6308    call    $0863			; MC SOUND REGISTER
238e 3e10      ld      a,$10			; use hardware envelope
2390 cddb23    call    $23db			; write volume for channel and store

2393 cdae23    call    $23ae
2396 7b        ld      a,e
2397 3c        inc     a
2398 208e      jr      nz,$2328         

239a 211b23    ld      hl,$231b			; default volume envelope
239d cdcd23    call    $23cd			; set volume envelope
23a0 1886      jr      $2328            ;

;;=======================================================================

23a2 af        xor     a
23a3 dd7703    ld      (ix+$03),a		; no rendezvous/hold and not playing
23a6 dd7707    ld      (ix+$07),a		; no tone envelope active
23a9 dd7704    ld      (ix+$04),a		; no volume envelope active
23ac 37        scf     
23ad c9        ret     

;;=======================================================================

23ae dd350c    dec     (ix+$0c)
23b1 201e      jr      nz,$23d1         ; (+$1e)

23b3 dd7e09    ld      a,(ix+$09)
23b6 87        add     a,a
23b7 211b23    ld      hl,$231b			; 
23ba 3011      jr      nc,$23cd         ; set volume envelope

23bc dd3408    inc     (ix+$08)
23bf 2006      jr      nz,$23c7         ; (+$06)
23c1 dd3409    inc     (ix+$09)
23c4 1eff      ld      e,$ff
23c6 c8        ret     z

;; reload?
23c7 dd6e0a    ld      l,(ix+$0a)
23ca dd660b    ld      h,(ix+$0b)

;; set volume envelope
23cd 7e        ld      a,(hl)
23ce dd770c    ld      (ix+$0c),a		;; step count
23d1 23        inc     hl
23d2 5e        ld      e,(hl)			;; step size
23d3 23        inc     hl
23d4 dd750d    ld      (ix+$0d),l		;; current volume envelope pointer
23d7 dd740e    ld      (ix+$0e),h
23da c9        ret     

;;----------------------------

;; write volume 0 = channel, 15 = value
23db dd770f    ld      (ix+$0f),a

;;----------------------------
;; set volume for channel
;; IX = pointer to channel data
;;
;; A = volume
23de 4f        ld      c,a
23df dd7e00    ld      a,(ix+$00)
23e2 c608      add     a,$08			; PSG volume register for channel A
23e4 c36308    jp      $0863			; MC SOUND REGISTER

;;----------------------------
;; disable channel
23e7 af        xor     a

;;-------------------------
;; update mixer for channel
23e8 47        ld      b,a
23e9 dd7e01    ld      a,(ix+$01)		; tone mixer value
23ec ddb602    or      (ix+$02)			; noise mixer value

23ef 21b5b2    ld      hl,$b2b5			 ; mixer value
23f2 f3        di      
23f3 b6        or      (hl)				 ; combine with current
23f4 a8        xor     b
23f5 be        cp      (hl)
23f6 77        ld      (hl),a
23f7 fb        ei      
23f8 2003      jr      nz,$23fd         ; this means tone and noise disabled

23fa 78        ld      a,b
23fb b7        or      a
23fc c0        ret     nz

23fd af        xor     a				; silence sound
23fe cdde23    call    $23de			; set channel volume
2401 f3        di      
2402 4e        ld      c,(hl)
2403 3e07      ld      a,$07			; PSG mixer register
2405 c36308    jp      $0863			; MC SOUND REGISTER

;;------------------------------------------------------------------------

;; set tone and get tone envelope
;; E = tone envelope number
2408 cd8124    call    $2481			; write tone to psg registers
240b 7b        ld      a,e
240c cdab24    call    $24ab			; SOUND T ADDRESS
240f d0        ret     nc

2410 7e        ld      a,(hl)			; number of sections in tone
2411 e67f      and     $7f				
2413 c8        ret     z

2414 dd7511    ld      (ix+$11),l		; set tone envelope pointer reload
2417 dd7412    ld      (ix+$12),h
241a cd7024    call    $2470
241d 1809      jr      $2428			; initial update tone envelope            

;;====================================================================================

241f dd6e14    ld      l,(ix+$14)			; current tone pointer?
2422 dd6615    ld      h,(ix+$15)

2425 dd5e18    ld      e,(ix+$18)			; step count

;; update tone envelope
2428 4e        ld      c,(hl)				; step size
2429 23        inc     hl
242a 7b        ld      a,e
242b d6f0      sub     $f0
242d 3804      jr      c,$2433          ; increase/decrease tone

242f 1e00      ld      e,$00
2431 180e      jr      $2441             

;;-------------------------------------

2433 1d        dec     e				; decrease step count
2434 79        ld      a,c
2435 87        add     a,a
2436 9f        sbc     a,a
2437 57        ld      d,a
2438 dd7e16    ld      a,(ix+$16)		;; low byte tone
243b 81        add     a,c
243c 4f        ld      c,a
243d dd7e17    ld      a,(ix+$17)		;; high byte tone
2440 8a        adc     a,d

2441 57        ld      d,a
2442 cd8124    call    $2481			; write tone to psg registers
2445 4e        ld      c,(hl)			; pause time
2446 7b        ld      a,e
2447 b7        or      a
2448 2019      jr      nz,$2463         ; (+$19)

;; step count done..

244a dd7e13    ld      a,(ix+$13)		; number of tone sections remaining..
244d 3d        dec     a
244e 2010      jr      nz,$2460         

;; reload
2450 dd6e11    ld      l,(ix+$11)
2453 dd6612    ld      h,(ix+$12)

2456 7e        ld      a,(hl)			; number of sections.
2457 c680      add     a,$80
2459 3805      jr      c,$2460          

245b dd360400  ld      (ix+$04),$00		; no volume envelope
245f c9        ret     

;;====================================================

2460 cd7024    call    $2470
2463 dd7318    ld      (ix+$18),e
2466 f3        di      
2467 dd7105    ld      (ix+$05),c			; pause
246a dd360480  ld      (ix+$04),$80			; has volume envelope
246e fb        ei      
246f c9        ret     

;;=====================================================================

2470 dd7713    ld      (ix+$13),a 	;; number of sections remaining in envelope
2473 23        inc     hl
2474 5e        ld      e,(hl)		;; step count
2475 23        inc     hl
2476 dd7514    ld      (ix+$14),l
2479 dd7415    ld      (ix+$15),h
247c 7b        ld      a,e
247d b7        or      a
247e c0        ret     nz

247f 1c        inc     e
2480 c9        ret     

;;===========================================================================
;; write tone to PSG
;; C = tone low byte
;; D = tone high byte
2481 dd7e00    ld      a,(ix+$00)		;; sound channel 0 = A, 1 = B, 2 =C 
2484 87        add     a,a				
										;; A = 0/2/4
2485 f5        push    af
2486 dd7116    ld      (ix+$16),c
2489 cd6308    call    $0863			; MC SOUND REGISTER
248c f1        pop     af
248d 3c        inc     a				
										;; A = 1/3/5
248e 4a        ld      c,d
248f dd7117    ld      (ix+$17),c
2492 c36308    jp      $0863			; MC SOUND REGISTER


;;==========================================================================
;; SOUND AMPL ENVELOPE
;; sets up a volume envelope
;; A = envelope 1-15
2495 11a6b2    ld      de,$b2a6
2498 1803      jr      $249d            ; (+$03)


;;==========================================================================
;; SOUND TONE ENVELOPE
;; sets up a tone envelope
;; A = envelope 1-15

249a 1196b3    ld      de,$b396
249d eb        ex      de,hl
249e cdae24    call    $24ae			;; get envelope
24a1 eb        ex      de,hl
24a2 d0        ret     nc

;; +0 - number of sections in the envelope 
;; +1..3 - first section of the envelope 
;; +4..6 - second section of the envelope 
;; +7..9 - third section of the envelope 
;; +10..12 - fourth section of the envelope 
;; +13..15 = fifth section of the envelope 
;;
;; Each section of the envelope has three bytes set out as follows 

;; non-hardware envelope:
;;byte 0 - step count (with bit 7 set) 
;;byte 1 - step size 
;;byte 2 - pause time 
;; hardware-envelope:
;;byte 0 - envelope shape (with bit 7 not set)
;;bytes 1 and 2 - envelope period 
24a3 edb0      ldir    
24a5 c9        ret     

;;==========================================================================
;; SOUND A ADDRESS
;; Gets the address of the data block associated with the amplitude/volume envelope
;; A = envelope number (1-15)

24a6 21a6b2    ld      hl,$b2a6			; first amplitude envelope - &10
24a9 1803      jr      $24ae            ; get envelope

;;==========================================================================
;; SOUND T ADDRESS
;; Gets the address of the data block associated with the tone envelope
;; A = envelope number (1-15)
 
24ab 2196b3    ld      hl,$b396		;; first tone envelope - &10

;; get envelope
24ae b7        or      a			;; 0 = invalid envelope number
24af c8        ret     z

24b0 fe10      cp      $10			;; >=16 invalid envelope number
24b2 d0        ret     nc

24b3 011000    ld      bc,$0010		;; 16 bytes per envelope (5 sections + count)
24b6 09        add     hl,bc
24b7 3d        dec     a
24b8 20fc      jr      nz,$24b6         ; (-$04)
24ba 37        scf     
24bb c9        ret     

;;============================================================================
;; CAS INITIALISE

24bc cd5725    call    $2557			; CAS IN ABANDON
24bf cd9925    call    $2599			; CAS OUT ABANDON

;; enable cassette messages
24c2 af        xor     a
24c3 cde124    call    $24e1			; CAS NOISY

;; stop cassette motor
24c6 cdbf2b    call    $2bbf			; CAS STOP MOTOR

;; set default speed for writing
24c9 214d01    ld      hl,$014d
24cc 3e19      ld      a,$19

;;============================================================================
;; CAS SET SPEED

24ce 29        add     hl,hl			; x2
24cf 29        add     hl,hl			; x4
24d0 29        add     hl,hl			; x8
24d1 29        add     hl,hl			; x32
24d2 29        add     hl,hl			; x64
24d3 29        add     hl,hl			; x128
24d4 0f        rrca    
24d5 0f        rrca    
24d6 e63f      and     $3f
24d8 6f        ld      l,a
24d9 22e9b1    ld      ($b1e9),hl
24dc 3ae7b1    ld      a,($b1e7)
24df 37        scf     
24e0 c9        ret     

;;============================================================================
;; CAS NOISY

24e1 3218b1    ld      ($b118),a
24e4 c9        ret     

;;============================================================================
;; CAS IN OPEN
;; 
;; B = length of filename
;; HL = filename
;; DE = address of 2K buffer
;;
;; NOTES:
;; - first block of file *must* be 2K long

24e5 dd211ab1  ld      ix,$b11a			;; input header

24e9 cd0225    call    $2502			;; initialise header
24ec e5        push    hl
24ed dcac26    call    c,$26ac			;; read a block
24f0 e1        pop     hl
24f1 d0        ret     nc

24f2 ed5b34b1  ld      de,($b134)		;; load address
24f6 ed4b37b1  ld      bc,($b137)		;; execution address
24fa 3a31b1    ld      a,($b131)		;; file type from header
24fd c9        ret     

;;============================================================================
;; CAS OUT OPEN

24fe dd215fb1  ld      ix,$b15f

;;----------------------------------------------------------------------------
;; DE = address of 2k buffer
;; HL = address of filename
;; B = length of filename

2502 dd7e00    ld      a,(ix+$00)
2505 b7        or      a
2506 3e0e      ld      a,$0e
2508 c0        ret     nz

2509 dde5      push    ix
250b e3        ex      (sp),hl			
250c 34        inc     (hl)
250d 23        inc     hl
250e 73        ld      (hl),e
250f 23        inc     hl
2510 72        ld      (hl),d
2511 23        inc     hl
2512 73        ld      (hl),e
2513 23        inc     hl
2514 72        ld      (hl),d
2515 23        inc     hl
2516 eb        ex      de,hl
2517 e1        pop     hl
2518 d5        push    de

;; length of header
2519 0e40      ld      c,$40

;; clear header
251b af        xor     a
251c 12        ld      (de),a
251d 13        inc     de
251e 0d        dec     c
251f 20fb      jr      nz,$251c         ; (-$05)

;; write filename
2521 d1        pop     de
2522 d5        push    de

;;-----------------------------------------------------
;; copy filename into buffer

2523 78        ld      a,b
2524 fe10      cp      $10
2526 3802      jr      c,$252a          ; (+$02)

2528 0610      ld      b,$10			

252a 04        inc     b
252b 48        ld      c,b
252c 1807      jr      $2535            ; (+$07)

;; read character from RAM
252e e7        rst     $20				; RST 4 - LOW: RAM LAM
252f 23        inc     hl
2530 cd2629    call    $2926			; convert character to upper case
2533 12        ld      (de),a			; store character
2534 13        inc     de
2535 10f7      djnz    $252e            

;; pad with spaces
2537 0d        dec     c
2538 2809      jr      z,$2543          ; (+$09)
253a 1b        dec     de
253b 1a        ld      a,(de)
253c ee20      xor     $20
253e 2003      jr      nz,$2543         ; 

2540 12        ld      (de),a			; write character
2541 18f4      jr      $2537            ; 

;;------------------------------------------------------

2543 e1        pop     hl
2544 dd3415    inc     (ix+$15)			; set block index
2547 dd361716  ld      (ix+$17),$16		; set initial file type
254b dd351c    dec     (ix+$1c)			; set first block flag
254e 37        scf     
254f c9        ret     

;;============================================================================
;; CAS IN CLOSE

2550 3a1ab1    ld      a,($b11a)		; get current read function
2553 b7        or      a
2554 3e0e      ld      a,$0e
2556 c8        ret     z

;;============================================================================
;; CAS IN ABANDON

2557 211ab1    ld      hl,$b11a			; get current read function
255a 0601      ld      b,$01
255c 7e        ld      a,(hl)
255d 3600      ld      (hl),$00			; clear function allowing other functions to proceed
255f c5        push    bc
2560 cd6d25    call    $256d		
2563 f1        pop     af

2564 21e4b1    ld      hl,$b1e4
2567 ae        xor     (hl)
2568 37        scf     
2569 c0        ret     nz
256a 77        ld      (hl),a
256b 9f        sbc     a,a
256c c9        ret     

;;============================================================================
;; A = function code
;; HL = ?
256d fe04      cp      $04				
256f d8        ret     c

;; clear
2570 23        inc     hl
2571 5e        ld      e,(hl)
2572 23        inc     hl
2573 56        ld      d,(hl)
2574 6b        ld      l,e
2575 62        ld      h,d
2576 13        inc     de
2577 3600      ld      (hl),$00
2579 01ff07    ld      bc,$07ff
257c c3a1ba    jp      $baa1				;; HI: KL LDIR			

;;============================================================================
;; CAS OUT CLOSE

257f 3a5fb1    ld      a,($b15f)
2582 fe03      cp      $03
2584 2813      jr      z,$2599          ; (+$13)
2586 c6ff      add     a,$ff
2588 3e0e      ld      a,$0e
258a d0        ret     nc

258b 2175b1    ld      hl,$b175
258e 35        dec     (hl)
258f 23        inc     hl
2590 23        inc     hl
2591 7e        ld      a,(hl)
2592 23        inc     hl
2593 b6        or      (hl)
2594 37        scf     
2595 c48627    call    nz,$2786			;; write a block
2598 d0        ret     nc

;;============================================================================
;; CAS OUT ABANDON

2599 215fb1    ld      hl,$b15f
259c 0602      ld      b,$02
259e 18bc      jr      $255c            ; (-$44)

;;============================================================================
;; CAS IN CHAR

25a0 e5        push    hl
25a1 d5        push    de
25a2 c5        push    bc
25a3 0605      ld      b,$05
25a5 cdf625    call    $25f6			;; set cassette input function
25a8 201a      jr      nz,$25c4         ; (+$1a)
25aa 2a32b1    ld      hl,($b132)
25ad 7c        ld      a,h
25ae b5        or      l
25af 37        scf     
25b0 ccac26    call    z,$26ac			;; read a block
25b3 300f      jr      nc,$25c4         ; (+$0f)
25b5 2a32b1    ld      hl,($b132)
25b8 2b        dec     hl
25b9 2232b1    ld      ($b132),hl
25bc 2a1db1    ld      hl,($b11d)
25bf e7        rst     $20				; RST 4 - LOW: RAM LAM
25c0 23        inc     hl
25c1 221db1    ld      ($b11d),hl
25c4 182c      jr      $25f2            ; (+$2c)

;;============================================================================
;; CAS OUT CHAR

25c6 e5        push    hl
25c7 d5        push    de
25c8 c5        push    bc
25c9 4f        ld      c,a
25ca 215fb1    ld      hl,$b15f
25cd 0605      ld      b,$05
25cf cdf925    call    $25f9
25d2 201e      jr      nz,$25f2         ; (+$1e)
25d4 2a77b1    ld      hl,($b177)
25d7 110008    ld      de,$0800
25da ed52      sbc     hl,de
25dc c5        push    bc
25dd d48627    call    nc,$2786			;; write a block
25e0 c1        pop     bc
25e1 300f      jr      nc,$25f2         ; (+$0f)
25e3 2a77b1    ld      hl,($b177)
25e6 23        inc     hl
25e7 2277b1    ld      ($b177),hl
25ea 2a62b1    ld      hl,($b162)
25ed 71        ld      (hl),c
25ee 23        inc     hl
25ef 2262b1    ld      ($b162),hl
25f2 c1        pop     bc
25f3 d1        pop     de
25f4 e1        pop     hl
25f5 c9        ret     


;;============================================================================
;; attempt to set cassette input function

;; entry:
;; B = function code
;;
;; 0 = no function active
;; 1 = opened using CAS IN OPEN or CAS OUT OPEN
;; 2 = reading with CAS IN DIRECT
;; 3 = broken into with ESC
;; 4 = catalog
;; 5 = reading with CAS IN CHAR
;;
;; exit:
;; zero set = no error. function has been set or function is already set
;; zero clear = error. A = error code
;;
25f6 211ab1    ld      hl,$b11a			

25f9 7e        ld      a,(hl)			;; get current function code
25fa b8        cp      b				;; same as existing code?
25fb c8        ret     z
;; function codes are different
25fc ee01      xor     $01				;; just opened?
25fe 3e0e      ld      a,$0e
2600 c0        ret     nz
;; must be just opened for this to succeed
;;
;; set new function

2601 70        ld      (hl),b
2602 c9        ret     

;;============================================================================
;; CAS TEST EOF

2603 cda025    call    $25a0
2606 d0        ret     nc

;;============================================================================
;; CAS RETURN

2607 e5        push    hl
2608 2a32b1    ld      hl,($b132)
260b 23        inc     hl
260c 2232b1    ld      ($b132),hl
260f 2a1db1    ld      hl,($b11d)
2612 2b        dec     hl
2613 221db1    ld      ($b11d),hl
2616 e1        pop     hl
2617 c9        ret     

;;============================================================================
;; CAS IN DIRECT
;; 
;; HL = load address
;;
;; Notes:
;; - file must be contiguous;
;; - load address of first block is important, load address of subsequent blocks 
;;   is ignored and can be any value
;; - first block of file must be 2k long; subsequent blocks can be any length
;; - execution address is taken from header of last block
;; - filename of each block must be the same
;; - block numbers are consecutive and increment
;; - first block number is *not* important; it can be any value!

2618 eb        ex      de,hl
2619 0602      ld      b,$02			;; IN direct
261b cdf625    call    $25f6			;; set cassette input function
261e c0        ret     nz

;; set initial load address
261f ed5334b1  ld      ($b134),de

;; transfer first block to destination
2623 cd3c26    call    $263c			;; transfer loaded block to destination location


;; update load address
2626 2a34b1    ld      hl,($b134)		;; load address from in memory header
2629 ed5b32b1  ld      de,($b132)		;; length from loaded header
262d 19        add     hl,de
262e 2234b1    ld      ($b134),hl

2631 cdac26    call    $26ac			;; read a block
2634 38f0      jr      c,$2626          ; (-$10)

2636 c8        ret     z
2637 2abeb1    ld      hl,($b1be)		;; execution address
263a 37        scf     
263b c9        ret     

;;============================================================================
;; transfer loaded block to destination location

263c 2a1bb1    ld      hl,($b11b)
263f ed4b32b1  ld      bc,($b132)
2643 7b        ld      a,e
2644 95        sub     l
2645 7a        ld      a,d
2646 9c        sbc     a,h
2647 daa1ba    jp      c,$baa1				;; HI: KL LDIR
264a 09        add     hl,bc
264b 2b        dec     hl
264c eb        ex      de,hl
264d 09        add     hl,bc
264e 2b        dec     hl
264f eb        ex      de,hl
2650 c3a7ba    jp      $baa7				;; HI: KL LDDR

;;============================================================================
;; CAS OUT DIRECT
;; 
;; HL = load address
;; DE = length
;; BC = execution address
;; A = file type

2653 e5        push    hl
2654 c5        push    bc
2655 4f        ld      c,a
2656 215fb1    ld      hl,$b15f
2659 0602      ld      b,$02
265b cdf925    call    $25f9
265e 202d      jr      nz,$268d         ; (+$2d)

2660 79        ld      a,c
2661 c1        pop     bc
2662 e1        pop     hl

;; setup header
2663 3276b1    ld      ($b176),a		
2666 ed537cb1  ld      ($b17c),de		; length
266a ed437eb1  ld      ($b17e),bc		; execution address

266e 2260b1    ld      ($b160),hl		; load address
2671 ed5377b1  ld      ($b177),de		; length
2675 21fff7    ld      hl,$f7ff			; $f7ff = -$800
2678 19        add     hl,de
2679 3f        ccf     
267a d8        ret     c

267b 210008    ld      hl,$0800
267e 2277b1    ld      ($b177),hl		; length of this block

2681 eb        ex      de,hl
2682 ed52      sbc     hl,de
2684 e5        push    hl
2685 2a60b1    ld      hl,($b160)
2688 19        add     hl,de
2689 e5        push    hl
268a cd8627    call    $2786			; write block
	
268d e1        pop     hl
268e d1        pop     de
268f d0        ret     nc

2690 18dc      jr      $266e            ; (-$24)

;;============================================================================
;; CAS CATALOG
;;
;; DE = address of 2k buffer

2692 211ab1    ld      hl,$b11a
2695 7e        ld      a,(hl)
2696 b7        or      a
2697 3e0e      ld      a,$0e
2699 c0        ret     nz

269a 3604      ld      (hl),$04			; set catalog function

269c ed531bb1  ld      ($b11b),de		; buffer to load blocks to
26a0 af        xor     a
26a1 cde124    call    $24e1			;; CAS NOISY
26a4 cdb326    call    $26b3			; read block
26a7 38fb      jr      c,$26a4          ; loop if cassette not pressed

26a9 c35725    jp      $2557			;; CAS IN ABANDON


;;=================================================================================
;; read a block
;; 
;; 
;; notes:
;;

26ac 3a30b1    ld      a,($b130)		; last block flag
26af b7        or      a
26b0 3e0f      ld      a,$0f			; "hard end of file"
26b2 c0        ret     nz

26b3 010183    ld      bc,$8301			; Press PLAY then any key
26b6 cde527    call    $27e5			; display message if required
26b9 305f      jr      nc,$271a         

26bb 21a4b1    ld      hl,$b1a4			; location to load header
26be 114000    ld      de,$0040			; header length
26c1 3e2c      ld      a,$2c			; header marker byte
26c3 cda629    call    $29a6			; cas read: read header
26c6 3052      jr      nc,$271a         

26c8 068b      ld      b,$8b			; no message
26ca cd2f29    call    $292f			; catalog?
26cd 2807      jr      z,$26d6          

;; not catalog, so compare filenames
26cf cd3727    call    $2737			; compare filenames
26d2 2053      jr      nz,$2727         ; if nz, display "Found xxx block x"

26d4 0689      ld      b,$89			; "Loading"
26d6 cd0428    call    $2804			; display "Loading xxx block x"

26d9 ed5bb7b1  ld      de,($b1b7)		; length from loaded header
26dd 2a34b1    ld      hl,($b134)		; location from in-memory header

26e0 3a1ab1    ld      a,($b11a)		; 
26e3 fe02      cp      $02				; in direct?
26e5 280e      jr      z,$26f5          ; 

;; not in direct, so is:
;; 1. catalog
;; 2. opening file for read
;; 3. reading file char by char
;;
;; check the block is no longer than $800 bytes
;; if it is report a "read error d"
26e7 21fff7    ld      hl,$f7ff			; $f7ff = -$800
26ea 19        add     hl,de			; add length from header

26eb 3e04      ld      a,$04			; code for 'read error d'
26ed 382b      jr      c,$271a          ; (+$2b)

26ef 2a1bb1    ld      hl,($b11b)		; 2k buffer
26f2 221db1    ld      ($b11d),hl

26f5 3e16      ld      a,$16			; data marker
26f7 cda629    call    $29a6			; cas read: read data

26fa 301e      jr      nc,$271a         ;

;; increment block number in internal header
26fc 212fb1    ld      hl,$b12f			; block number
26ff 34        inc     (hl)				; increment block number

;; get last block flag from loaded header and store into
;; internal header
2700 3ab5b1    ld      a,($b1b5)		
2703 23        inc     hl
2704 77        ld      (hl),a			

;; clear first block flag
2705 af        xor     a				
2706 3236b1    ld      ($b136),a

2709 2ab7b1    ld      hl,($b1b7)		; get length from loaded header
270c 2232b1    ld      ($b132),hl		; store in internal header

270f cd2f29    call    $292f			; catalog?

;; if catalog display OK message
2712 3e8c      ld      a,$8c			; "OK"
2714 cc7e28    call    z,$287e			; display message

;; 
2717 37        scf     
2718 1865      jr      $277f            ; (+$65)

;;===========================================================================
;; A = code (A=0: no error; A<>0: error)

271a b7        or      a
271b 211ab1    ld      hl,$b11a
271e 2858      jr      z,$2778          ; 

;; A = error code
2720 0685      ld      b,$85			; "Read error"
2722 cd8528    call    $2885			; display message with code
;; .. retry
2725 1894      jr      $26bb            

;;===========================================================================

2727 f5        push    af
2728 0688      ld      b,$88			; "Found "
272a cd0428    call    $2804			; "Found xxx block x"
272d f1        pop     af
272e 308b      jr      nc,$26bb         ; (-$75)

2730 0687      ld      b,$87			; "Rewind tape"
2732 cd8328    call    $2883
2735 1884      jr      $26bb            ; (-$7c)

;;========================================================================
;; compare filenames
;;
;; if not first block:
;; compare names
;; if first block:
;; - compare filenames if a filename was specified
;; - copy loaded header into ram

2737 3a36b1    ld      a,($b136)		; first block flag in internal header?
273a b7        or      a
273b 281b      jr      z,$2758          

273d 3abbb1    ld      a,($b1bb)		; first block flag in loaded header?
2740 2f        cpl     
2741 b7        or      a
2742 c0        ret     nz

;; if user specified a filename, compare it against the filename in the loaded
;; header, otherwise accept the file

2743 3a1fb1    ld      a,($b11f)		; did user specify a filename?
										; e.g. LOAD"bob
2746 b7        or      a

2747 c46027    call    nz,$2760			; compare filenames and block number
274a c0        ret     nz				; if filenames do not match, quit

;; gets here if:

;; 1. if a filename was specified by user and filename matches with 
;; filename in loaded header
;;
;; 2. no filename was specified by user

;; copy loaded header to in-memory header
274b 21a4b1    ld      hl,$b1a4
274e 111fb1    ld      de,$b11f
2751 014000    ld      bc,$0040
2754 edb0      ldir    

2756 af        xor     a
2757 c9        ret     

;;=========================================================================
;; compare name and block number

2758 cd6027    call    $2760			; compare filenames
275b c0        ret     nz

;; compare block number
275c eb        ex      de,hl
275d 1a        ld      a,(de)
275e be        cp      (hl)
275f c9        ret     

;;============================================================================
;; compare two filenames; one filename is in the loaded header
;; the second filename is in the in-memory header
;;
;; nz = filenames are different
;; z = filenames are identical

2760 211fb1    ld      hl,$b11f			; in-memory header
2763 11a4b1    ld      de,$b1a4			; loaded header

;; compare filenames
2766 0610      ld      b,$10			; 16 characters

2768 1a        ld      a,(de)			; get character from loaded header
2769 cd2629    call    $2926			; convert character to upper case
276c 4f        ld      c,a
276d 7e        ld      a,(hl)			; get character from in-memory header
276e cd2629    call    $2926			; convert character to upper case

2771 a9        xor     c				; result will be 0 if the characters are identical.
										; will be <>0 if the characters are different

2772 c0        ret     nz				; quit if characters are not the same

2773 23        inc     hl				; increment pointer
2774 13        inc     de				; increment pointer
2775 10f1      djnz    $2768            

;; if control gets to here, then the filenames are identical
2777 c9        ret     
;;============================================================================

2778 7e        ld      a,(hl)
2779 3603      ld      (hl),$03			; 
277b cd6d25    call    $256d		
277e b7        or      a

;;----------------------------------------------------------------------------
;; quit loading block
277f 9f        sbc     a,a
2780 f5        push    af
2781 cdbf2b    call    $2bbf			; CAS STOP MOTOR
2784 f1        pop     af
2785 c9        ret     

;;============================================================================
;; write a block

2786 010284    ld      bc,$8402			; press rec
2789 cde527    call    $27e5			;  display message if required
278c 304a      jr      nc,$27d8         ; (+$4a)
278e 068a      ld      b,$8a
2790 1164b1    ld      de,$b164
2793 cd0728    call    $2807
2796 217bb1    ld      hl,$b17b
2799 cdfa27    call    $27fa
279c 303a      jr      nc,$27d8         ; (+$3a)
279e 2a60b1    ld      hl,($b160)
27a1 2262b1    ld      ($b162),hl
27a4 2279b1    ld      ($b179),hl
27a7 e5        push    hl

;; write header for this block
27a8 2164b1    ld      hl,$b164
27ab 114000    ld      de,$0040
27ae 3e2c      ld      a,$2c			; header marker
27b0 cdaf29    call    $29af			; cas write: write header

27b3 e1        pop     hl
27b4 3022      jr      nc,$27d8         ; (+$22)

;; write data for this block
27b6 ed5b77b1  ld      de,($b177)
27ba 3e16      ld      a,$16			; data marker
27bc cdaf29    call    $29af			; cas write: write data block
27bf 2175b1    ld      hl,$b175
27c2 dcfa27    call    c,$27fa
27c5 3011      jr      nc,$27d8         ; (+$11)
27c7 210000    ld      hl,$0000
27ca 2277b1    ld      ($b177),hl
27cd 2174b1    ld      hl,$b174
27d0 34        inc     (hl)
27d1 af        xor     a
27d2 327bb1    ld      ($b17b),a
27d5 37        scf     
27d6 18a7      jr      $277f            ; (-$59)

;;=======================================================================
;; A = code (A=0: no error; A<>0: error)
27d8 b7        or      a
27d9 215fb1    ld      hl,$b15f
27dc 289a      jr      z,$2778          ; (-$66)

;; a = code
27de 0686      ld      b,$86			; "Write error"
27e0 cd8528    call    $2885			; display message with code
27e3 18b9      jr      $279e            ; (-$47)

;;========================================================================
;; C = message code
;; exit:
;; A = 0: no error
;; A <>0: error

27e5 21e4b1    ld      hl,$b1e4
27e8 79        ld      a,c
27e9 be        cp      (hl)
27ea 71        ld      (hl),c
27eb 37        scf     

27ec e5        push    hl
27ed c5        push    bc
27ee c4d228    call    nz,$28d2			; Press play then any key
27f1 c1        pop     bc
27f2 e1        pop     hl

27f3 9f        sbc     a,a
27f4 d0        ret     nc

27f5 cdbb2b    call    $2bbb			; CAS START MOTOR
27f8 9f        sbc     a,a
27f9 c9        ret     

;;========================================================================

27fa 7e        ld      a,(hl)
27fb b7        or      a
27fc 37        scf     
27fd c8        ret     z

27fe 012c01    ld      bc,$012c			; delay in 1/100ths of a second
2801 c3e22b    jp      $2be2			; delay for 3 seconds

;;===================================================================================

2804 11a4b1    ld      de,$b1a4

2807 3a18b1    ld      a,($b118)		; cassette messages enabled?
280a b7        or      a
280b c0        ret     nz

280c 3219b1    ld      ($b119),a
280f cdf328    call    $28f3

2812 cd9828    call    $2898			; display message

2815 1a        ld      a,(de)			; is first character of filename = 0?
2816 b7        or      a
2817 200a      jr      nz,$2823         ; 

;; unnamed file

2819 3e8e      ld      a,$8e			; "Unnamed file"
281b cd9928    call    $2899			; display message

281e 011000    ld      bc,$0010
2821 182e      jr      $2851            ; (+$2e)

;;-----------------------------
;; named file
2823 cd2f29    call    $292f

2826 010010    ld      bc,$1000
2829 280d      jr      z,$2838          ; (+$0d)
282b 6b        ld      l,e
282c 62        ld      h,d
282d 7e        ld      a,(hl)
282e b7        or      a
282f 2804      jr      z,$2835          ; (+$04)
2831 0c        inc     c
2832 23        inc     hl
2833 10f8      djnz    $282d            ; (-$08)
2835 78        ld      a,b
2836 41        ld      b,c
2837 4f        ld      c,a

2838 cdfd28    call    $28fd			; insert new-line if word
										; can't fit onto current-line

283b 1a        ld      a,(de)			; get character from filename
283c cd2629    call    $2926			; convert character to upper case

283f b7        or      a				; zero?
2840 2002      jr      nz,$2844         

;; display a space if a zero is found

2842 3e20      ld      a,$20			; display a space

2844 c5        push    bc
2845 d5        push    de
2846 cd3513    call    $1335			; TXT WR CHAR
2849 d1        pop     de
284a c1        pop     bc
284b 13        inc     de
284c 10ed      djnz    $283b            ; (-$13)

284e cdce28    call    $28ce			; display space

2851 eb        ex      de,hl
2852 09        add     hl,bc
2853 eb        ex      de,hl

2854 3e8d      ld      a,$8d			; "block "
2856 cd9928    call    $2899			; display message

2859 0602      ld      b,$02			; length of word
285b cdfd28    call    $28fd			; insert new-line if word
										; can't fit onto current-line

285e 1a        ld      a,(de)
285f cd1429    call    $2914			; display decimal number

2862 cdce28    call    $28ce			; display space

2865 13        inc     de
2866 cd2f29    call    $292f
2869 200b      jr      nz,$2876         ; (+$0b)
286b 13        inc     de
286c 1a        ld      a,(de)
286d e60f      and     $0f
286f c624      add     a,$24
2871 cdf028    call    $28f0

2874 1858      jr      $28ce            ; display space

;;=========================================================================

2876 1a        ld      a,(de)
2877 2119b1    ld      hl,$b119
287a b6        or      (hl)
287b c8        ret     z
287c 186d      jr      $28eb            ; (+$6d)

;;=========================================================================
;; A = message code

287e cd9928    call    $2899			; display message
2881 1868      jr      $28eb            ; (+$68)

;;=========================================================================

2883 3eff      ld      a,$ff

;; display message with code on end (e.g. "Read error x" or "Write error x"
;; A = code (1,2,3)
2885 f5        push    af
2886 cd9128    call    $2891			
2889 f1        pop     af
288a c660      add     a,$60			; 'a'-1
288c d4f028    call    nc,$28f0			; display character
288f 185a      jr      $28eb            

;;=========================================================================

2891 cd7c11    call    $117c			; TXT GET CURSOR
2894 25        dec     h
2895 c4eb28    call    nz,$28eb

2898 78        ld      a,b

;;=========================================================================
;; display message
;;
;; - message is displayed using word-wrap
;;
;; a = message number (&80-&FF)
2899 e5        push    hl

289a e67f      and     $7f				; get message index (0-127)
289c 47        ld      b,a

289d 213529    ld      hl,$2935			; start of message list (points to first message)

;; first message in list? (message 0?)
28a0 2807      jr      z,$28a9          

;; not first. 
;; 
;; - each message is terminated by a zero byte
;; - keep fetching bytes until a zero is found.
;; - if a zero is found, decrement count. If count reaches zero, then 
;; the first byte following the zero, is the start of the message we want

28a2 7e        ld      a,(hl)			; get byte
28a3 23        inc     hl				; increment pointer

28a4 b7        or      a				; is it zero (0) ?
28a5 20fb      jr      nz,$28a2         ; if zero, it is the end of this string

;; got a zero byte, so at end of the current string

28a7 10f9      djnz    $28a2            ; decrement message count

;; HL = start of message to display

;; this part is looped; message may contain multiple strings

;; end of message?
28a9 7e        ld      a,(hl)
28aa b7        or      a
28ab 2805      jr      z,$28b2          ; (+$05)

;; display message
28ad cdb528    call    $28b5			; display message with word-wrap

;; at this point there might be a end of string marker (0), the start
;; of another string (next byte will have bit 7=0) or a continuation string
;; (next byte will have bit 7=1)
28b0 18f7      jr      $28a9           ; continue displaying string 

;; finished displaying complete string , or displayed part of string sequence
28b2 e1        pop     hl

28b3 23        inc     hl				; if part of a complete message, go to next sub-string or word
28b4 c9        ret     

;;=========================================================================
;; display message with word wrap

;; HL = address of message
;; A = first character in message

;; if -ve, then bit 7 is set. Bit 6..0 define the ID of the message to display
;; if +ve, then this is the first character in the message
28b5 fa9928    jp      m,$2899			


;;-------------------------------------
;; count number of letters in word

28b8 e5        push    hl			;; store start of word

;; count number of letters in world
28b9 0600      ld      b,$00
28bb 04        inc     b

28bc 7e        ld      a,(hl)		;; get character
28bd 23        inc     hl			;; increment pointer
28be 07        rlca					;; if bit 7 is set, then this is the last character of the current word
28bf 30fa      jr      nc,$28bb        

;; B = number of letters

;; if word will not fit onto end of current line, insert
;; a line break, and display on next line
28c1 cdfd28    call    $28fd

28c4 e1        pop     hl			;; restore start of word 

;;------------------------------------
;; display word

;; HL = location of characters
;; B = number of characters 
28c5 7e        ld      a,(hl)			; get byte
28c6 23        inc     hl				; increment counter
28c7 e67f      and     $7f				; isolate byte
28c9 cdf028    call    $28f0			; display char (txt output?)
28cc 10f7      djnz    $28c5            

;; display space
28ce 3e20      ld      a,$20			; " " (space) character
28d0 181e      jr      $28f0            ; display character

;;=========================================================================

28d2 3a18b1    ld      a,($b118)		; cassette messages enabled?
28d5 b7        or      a
28d6 37        scf     
28d7 c0        ret     nz

28d8 cd9128    call    $2891			; display message

28db cdfe1b    call    $1bfe			; KM FLUSH
28de cd7612    call    $1276			; TXT CUR ON
28e1 cddb1c    call    $1cdb			; KM WAIT KEY
28e4 cd7e12    call    $127e			; TXT CUR OFF
28e7 fefc      cp      $fc
28e9 c8        ret     z

28ea 37        scf     

;;-----------------------------------------------------------------------

28eb cdf328    call    $28f3

;; display cr
28ee 3e0a      ld      a,$0a
28f0 c3fe13    jp      $13fe			; TXT OUTPUT

;;==========================================================================

28f3 f5        push    af
28f4 e5        push    hl
28f5 3e01      ld      a,$01
28f7 cd5a11    call    $115a			; TXT SET COLUMN
28fa e1        pop     hl
28fb f1        pop     af
28fc c9        ret     

;;==========================================================================
;; determine if word can be displayed on this line
28fd d5        push    de
28fe cd5212    call    $1252			; TXT GET WINDOW
2901 5c        ld      e,h
2902 cd7c11    call    $117c			; TXT GET CURSOR
2905 7c        ld      a,h
2906 3d        dec     a
2907 83        add     a,e
2908 80        add     a,b
2909 3d        dec     a
290a ba        cp      d
290b d1        pop     de
290c d8        ret     c

290d 3eff      ld      a,$ff
290f 3219b1    ld      ($b119),a
2912 18d7      jr      $28eb            ; (-$29)


;;============================================================================

;; divide by 10
2914 06ff      ld      b,$ff
2916 04        inc     b
2917 d60a      sub     $0a				
2919 30fb      jr      nc,$2916         ; (-$05)
;; B = result of division by 10
;; A = <10

291b c63a      add     a,$3a			; convert to ASCII digit
			
291d f5        push    af
291e 78        ld      a,b
291f b7        or      a
2920 c41429    call    nz,$2914			; continue with division

2923 f1        pop     af
2924 18ca      jr      $28f0            ; display character

;;============================================================================
;; convert character to upper case
2926 fe61      cp      $61				; "a"
2928 d8        ret     c

2929 fe7b      cp      $7b				; "z"
292b d0        ret     nc

292c c6e0      add     a,$e0
292e c9        ret     

;;============================================================================
;; test if read function is CATALOG
;;
;; zero set = catalog
;; zero clear = not catalog
292f 3a1ab1    ld      a,($b11a)		; get current read function
2932 fe04      cp      $04				; catalog function?
2934 c9        ret     

2935
;;============================================================================
;; cassette messages
;; - a zero (0) byte indicates end of complete message
;; - a byte with bit 7 set indicates:
;;	 end of a word, the id of another continuing string
;; 0: "Press"
;; 1: "PLAY then any key:"
;; 2: "error"
;; 3: "Press PLAY then any key:"
;; 4: "Press REC and PLAY then any key:"
;; 5: "Read error"
;; 6: "Write error"
;; 7: "Rewind tape"
;; 8: "Found  "
;; 9: "Loading"
;; 10: "Saving"
;; 11: <blank>
;; 12: "Ok"
;; 13: "block"
;; 14: "Unnamed file"

defb "Pres","s"+&80,0
defb "PLA","Y"+&80,"the","n"+&80,"an","y"+&80,"key",":"+&80,0
defb "erro","r"+&80,0
defb 0+&80,1+&80,0
defb &80,"RE","C"+&80,"an","d"+&80,&81,defb 0
defb "Rea","d"+&80,&82
defb "Writ","e"+&80,&82
defb "Rewin","d"+&80,"tap","e"+&80,0
defb "Found "," "+&80,0
defb "Loadin","g"+&80,0
defb "Savin","g"+&80,0
defb 0
defb "O","k"+&80,0
defb "bloc","k"+&80,0
defb "Unname"d"+&80,"file   "," "+&80,0


;;=========================================================================
;; CAS READ

;; A = sync byte
;; HL = location of data
;; DE = length of data

29a6 cde329    call    $29e3			; enable key checking and start the cassette motor
29a9 f5        push    af
29aa 21282a    ld      hl,$2a28			; read block of data
29ad 1819      jr      $29c8            ; do read

;;=========================================================================
;; CAS WRITE

;; A = sync byte
;; HL = destination location for data
;; DE = length of data

29af cde329    call    $29e3			; enable key checking and start the cassette motor
29b2 f5        push    af
29b3 cdd42a    call    $2ad4			;; write start of block (pilot and syncs)
29b6 21672a    ld      hl,$2a67			;; write block of data
29b9 dc0d2a    call    c,$2a0d			;; read/write 256 byte blocks
29bc dce92a    call    c,$2ae9			;; write trailer
29bf 180f      jr      $29d0            ;; 

;;=========================================================================
;; CAS CHECK

29c1 cde329    call    $29e3			; enable key checking and start the cassette motor
29c4 f5        push    af
29c5 21372a    ld      hl,$2a37			;; check stored block with block in memory

;;------------------------------------------------------
;; do read
;; cas check or cas read
29c8 e5        push    hl
29c9 cd892a    call    $2a89			;; read pilot and sync
29cc e1        pop     hl
29cd dc0d2a    call    c,$2a0d			;; read/write 256 byte blocks


;;----------------------------------------------------------------
;; cas check, cas read or cas write
29d0 d1        pop     de
29d1 f5        push    af

29d2 0182f7    ld      bc,$f782			;; set PPI port A to output
29d5 ed49      out     (c),c

29d7 0110f6    ld      bc,$f610			;; cassette motor on
29da ed49      out     (c),c

;; if cassette motor is stopped, then it will stop immediatly
;; if cassette motor is running, then there will not be any pause.

29dc fb        ei						;; enable interrupts

29dd 7a        ld      a,d
29de cdc12b    call    $2bc1			;; CAS RESTORE MOTOR
29e1 f1        pop     af
29e2 c9        ret     

;;=========================================================================
;; enable key checking and start the cassette motor

;; store marker
29e3 32e5b1    ld      ($b1e5),a

29e6 1b        dec     de
29e7 1c        inc     e

29e8 e5        push    hl
29e9 d5        push    de
29ea cde91f    call    $1fe9			; SOUND RESET
29ed d1        pop     de
29ee dde1      pop     ix

29f0 cdbb2b    call    $2bbb			; CAS START MOTOR


29f3 f3        di					;; disable interrupts

;; select PSG register 14 (PSG port A)
;; (keyboard data is connected to PSG port A)
29f4 010ef4    ld      bc,$f40e		;; select keyboard line 14
29f7 ed49      out     (c),c

29f9 01d0f6    ld      bc,$f6d0		;; cassette motor on + PSG select register operation
29fc ed49      out     (c),c

29fe 0e10      ld      c,$10
2a00 ed49      out     (c),c		;; cassette motor on + PSG inactive operation

2a02 0192f7    ld      bc,$f792		;; set PPI port A to input
2a05 ed49      out     (c),c		
									;; PSG port A data can be read through PPI port A now

2a07 0158f6    ld      bc,$f658		;; cassette motor on + PSG read data operation + select keyboard line 8
2a0a ed49      out     (c),c
2a0c c9        ret     

;;========================================================================================
;; read/write blocks

;; DE = number of bytes to read/write

;; D = number of 256 blocks to read/write 
;; if D = 0, then there is a single block to write, which has E bytes
;; in it.
;; if D!=0, then there is more than one block to write, write 256 bytes
;; for each block except the last. Then write final block with remaining
;; bytes.

2a0d 7a        ld      a,d
2a0e b7        or      a
2a0f 280d      jr      z,$2a1e          ; (+$0d)

;; do each complete 256 byte block
2a11 e5        push    hl
2a12 d5        push    de
2a13 1e00      ld      e,$00			; number of bytes
2a15 cd1e2a    call    $2a1e			; read/write block
2a18 d1        pop     de
2a19 e1        pop     hl
2a1a d0        ret     nc

2a1b 15        dec     d
2a1c 20f3      jr      nz,$2a11         ; (-$0d)

;; E = number of bytes in last block to write

;;------------------------------------
;; initialise crc
2a1e 01ffff    ld      bc,$ffff
2a21 ed43ebb1  ld      ($b1eb),bc		; crc 

;; do function
2a25 1601      ld      d,$01
2a27 e9        jp      (hl)

;;========================================================================================
;; IX = address to load data to 
;; read data
;; input:
;; D = block size
;; E = actual data size
;; output:
;; D = bytes remaining in block (block size - actual data size)

2a28 cd202b    call    $2b20			; read byte from cassette
2a2b d0        ret     nc

2a2c dd7700    ld      (ix+$00),a		; store byte
2a2f dd23      inc     ix				; increment pointer

2a31 15        dec     d				; decrement block count

2a32 1d        dec     e
2a33 20f3      jr      nz,$2a28         ; decrement actual data count

;; D = number of bytes remaining in block

;; read remaining bytes in block; but ignore
2a35 1812      jr      $2a49            ; (+$12)

;;========================================================================================
;; check stored block with block in memory
2a37 cd202b    call    $2b20			; read byte from cassette
2a3a d0        ret     nc

2a3b 47        ld      b,a
2a3c cdd7ba    call    $bad7			; get byte from IX with roms disabled
2a3f a8        xor     b				


2a40 3e03      ld      a,$03			; 
2a42 c0        ret     nz

2a43 dd23      inc     ix
2a45 15        dec     d
2a46 1d        dec     e
2a47 20ee      jr      nz,$2a37         ; (-$12)

;; any more bytes remaining in block??
2a49 15        dec     d
2a4a 2806      jr      z,$2a52          ; 

;; bytes remaining
;; read the remaining bytes but ignore

2a4c cd202b    call    $2b20			; read byte from cassette	
2a4f d0        ret     nc

2a50 18f7      jr      $2a49            ; 

;;-----------------------------------------------------

2a52 cd162b    call    $2b16			; get 1's complemented crc

2a55 cd202b    call    $2b20			; read crc byte1 from cassette
2a58 d0        ret     nc

2a59 aa        xor     d
2a5a 2007      jr      nz,$2a63         ;

2a5c cd202b    call    $2b20			; read crc byte2 from cassette
2a5f d0        ret     nc

2a60 ab        xor     e
2a61 37        scf     
2a62 c8        ret     z

2a63 3e02      ld      a,$02
2a65 b7        or      a
2a66 c9        ret     

;;========================================================================================
;; write block of data (pad with 0's if less than block size)
;; IX = address of data
;; E = actual byte count
;; D = block size count
 
2a67 cdd7ba    call    $bad7			; get byte from IX with roms disabled
2a6a cd682b    call    $2b68			; write data byte
2a6d d0        ret     nc

2a6e dd23      inc     ix				; increment pointer

2a70 15        dec     d				; decrement block size count
2a71 1d        dec     e				; decrement actual count
2a72 20f3      jr      nz,$2a67         ; (-$0d)

;; actual byte count = block size count?
2a74 15        dec     d
2a75 2807      jr      z,$2a7e          

;; no, actual byte count was less than block size
;; pad up to block size with zeros

2a77 af        xor     a
2a78 cd682b    call    $2b68			; write data byte
2a7b d0        ret     nc

2a7c 18f6      jr      $2a74            ; (-$0a)


;; get 1's complemented crc
2a7e cd162b    call    $2b16

;; write crc 1
2a81 cd682b    call    $2b68			; write data byte
2a84 d0        ret     nc

;; write crc 2
2a85 7b        ld      a,e
2a86 c3682b    jp      $2b68			; write data byte

;;========================================================================================
;; read pilot and sync

2a89 d5        push    de
2a8a cd932a    call    $2a93			; read pilot and sync
2a8d d1        pop     de

2a8e d8        ret     c			

2a8f b7        or      a
2a90 c8        ret     z

2a91 18f6      jr      $2a89            ; (-$0a)

;;==========================================================================
;; read pilot and sync

;;---------------------------------
;; wait for start of leader/pilot

2a93 2e55      ld      l,$55			; %01010101
										; this is used to generate the cassette input data comparison 
										; used in the edge detection

2a95 cd3d2b    call    $2b3d			; sample edge
2a98 d0        ret     nc

;;------------------------------------------
;; get 256 pulses of leader/pilot
2a99 110000    ld      de,$0000			; initial total

2a9c 62        ld      h,d

2a9d cd3d2b    call    $2b3d			; sample edge
2aa0 d0        ret     nc

2aa1 eb        ex      de,hl
;; C = measured time
;; add measured time to total
2aa2 0600      ld      b,$00
2aa4 09        add     hl,bc
2aa5 eb        ex      de,hl

2aa6 25        dec     h
2aa7 20f4      jr      nz,$2a9d         ; (-$0c)


;; C = duration of last pulse read

;; look for sync bit
;; and adjust the average for every non-sync

;; DE = sum of 256 edges
;; D:E forms a 8.8 fixed point number
;; D = integer part of number (integer average of 256 pulses)
;; E = fractional part of number

2aa9 61        ld      h,c				; time of last pulse

2aaa 79        ld      a,c
2aab 92        sub     d				; subtract initial average 
2aac 4f        ld      c,a
2aad 9f        sbc     a,a
2aae 47        ld      b,a

;; if C>D then BC is +ve; BC = +ve delta
;; if C<D then BC is -ve; BC = -ve delta

;; adjust average
2aaf eb        ex      de,hl
2ab0 09        add     hl,bc			; DE = DE + BC
2ab1 eb        ex      de,hl

2ab2 cd3d2b    call    $2b3d			; sample edge
2ab5 d0        ret     nc

; A = D * 5/4
2ab6 7a        ld      a,d				; average so far			
2ab7 cb3f      srl     a				; /2
2ab9 cb3f      srl     a				; /4
										; A = D * 1/4
2abb 8a        adc     a,d				; A = D + (D*1/4)

;; sync pulse will have a duration which is half that of a pulse in a 1 bit
;; average<previous 


2abc 94        sub     h				; time of last pulse
2abd 38ea      jr      c,$2aa9          ; carry set if H>A

;; average>=previous (possibly read first pulse of sync or second of sync)

2abf 91        sub     c				; time of current pulse
2ac0 38e7      jr      c,$2aa9          ; carry set if C>(A-H)

;; to get here average>=(previous*2)
;; and this means we have just read the second pulse of the sync bit


;; calculate bit 1 timing
2ac2 7a        ld      a,d				; average
2ac3 1f        rra						; /2
										; A = D/2
2ac4 8a        adc     a,d				; A = D + (D/2)
										; A = D * (3/2)
2ac5 67        ld      h,a
										; this is the middle time
										; to calculate difference between 0 and 1 bit

;; if pulse measured is > this time, then we have a 1 bit
;; if pulse measured is < this time, then we have a 0 bit

;; H = timing constant
;; L = initial cassette data input state
2ac6 22e6b1    ld      ($b1e6),hl

;; read marker
2ac9 cd202b    call    $2b20			; read data-byte
2acc d0        ret     nc

2acd 21e5b1    ld      hl,$b1e5			; marker
2ad0 ae        xor     (hl)
2ad1 c0        ret     nz

2ad2 37        scf     
2ad3 c9        ret     

;;========================================================================================
;; write start of block
2ad4 cdf92b    call    $2bf9		;; 1/100th of a second delay

;; write leader
2ad7 210108    ld      hl,$0801		;; 2049
2ada cdec2a    call    $2aec		;; write leader (2049 1 bits; 4096 pulses)
2add d0        ret     nc

;; write sync bit
2ade b7        or      a
2adf cd782b    call    $2b78		;; write data-bit
2ae2 d0        ret     nc

;; write marker
2ae3 3ae5b1    ld      a,($b1e5)
2ae6 c3682b    jp      $2b68		;; write data byte

;;=============================================================================
;; write trailer = 33 "1" bits
;;
;; carry set = trailer written successfully
;; zero set = escape was pressed

2ae9 212100    ld      hl,$0021		;; 33

;; check for escape
2aec 06f4      ld      b,$f4		;; PPI port A
2aee ed78      in      a,(c)		;; read keyboard data through PPI port A (connected to PSG port A)
2af0 e604      and     $04			;; escape key pressed?
									;; bit 2 is 0 if escape key pressed
2af2 c8        ret     z			

;; write trailer bit
2af3 e5        push    hl
2af4 37        scf					;; a "1" bit   
2af5 cd782b    call    $2b78		;; write data-bit
2af8 e1        pop     hl
2af9 2b        dec     hl			;; decrement trailer bit count

2afa 7c        ld      a,h
2afb b5        or      l
2afc 20ee      jr      nz,$2aec     ;;

2afe 37        scf     
2aff c9        ret     
;;=============================================================================

;; update crc
2b00 2aebb1    ld      hl,($b1eb)		;; get crc
2b03 ac        xor     h
2b04 f2102b    jp      p,$2b10

2b07 7c        ld      a,h
2b08 ee08      xor     $08
2b0a 67        ld      h,a
2b0b 7d        ld      a,l
2b0c ee10      xor     $10
2b0e 6f        ld      l,a
2b0f 37        scf     

2b10 ed6a      adc     hl,hl
2b12 22ebb1    ld      ($b1eb),hl		;; store crc
2b15 c9        ret     

;;========================================================================================
;; get stored data crc and 1's complement it
;; initialise ready to write to cassette or to compare against crc from cassette

2b16 2aebb1    ld      hl,($b1eb)		;; block crc

;; 1's complement crc
2b19 7d        ld      a,l
2b1a 2f        cpl     
2b1b 5f        ld      e,a
2b1c 7c        ld      a,h
2b1d 2f        cpl     
2b1e 57        ld      d,a
2b1f c9        ret     

;;========================================================================================
;; read data-byte

2b20 d5        push    de
2b21 1e08      ld      e,$08			;; number of data-bits

2b23 2ae6b1    ld      hl,($b1e6)	
;; H = timing constant
;; L = initial cassette data input state

2b26 cd442b    call    $2b44			;; get edge

2b29 dc4d2b    call    c,$2b4d			;; get edge
2b2c 300d      jr      nc,$2b3b         

2b2e 7c        ld      a,h				;; ideal time
2b2f 91        sub     c				;; subtract measured time
										;; -ve (1 pulse) or +ve (0 pulse)
2b30 9f        sbc     a,a				
										;; if -ve, set carry
										;; if +ve, clear carry

;; carry flag = bit state: carry set = 1 bit, carry clear = 0 bit

2b31 cb12      rl      d				;; shift carry state into bit 0
										;; updating data-byte
										
2b33 cd002b    call    $2b00			;; update crc
2b36 1d        dec     e
2b37 20ea      jr      nz,$2b23         ; 

2b39 7a        ld      a,d
2b3a 37        scf     
2b3b d1        pop     de
2b3c c9        ret     

;;========================================================================================
;; sample edge and check for escape
;; L = bit-sequence which is shifted after each edge detected
;; starts of as &55 (%01010101)

;; check for escape
2b3d 06f4      ld      b,$f4		;; PPI port A
2b3f ed78      in      a,(c)		;; read keyboard data through PPI port A (connected to PSG port A)
2b41 e604      and     $04			;; escape key pressed?
									;; bit 2 is 0 if escape key pressed
2b43 c8        ret     z			


;; precompensation?
2b44 ed5f      ld      a,r

;; round up to divisible by 4
;; i.e.
;; 0->0, 
;; 1->4, 
;; 2->4, 
;; 3->4, 
;; 4->8, 
;; 5->8
;; etc

2b46 c603      add     a,$03
2b48 0f        rrca					;; /2
2b49 0f        rrca					;; /4

2b4a e61f      and     $1f			;; 

2b4c 4f        ld      c,a

2b4d 06f5      ld      b,$f5		; PPI port B input (includes cassette data input)

;; -----------------------------------------------------
;; loop to count time between edges
;; C = time in 17us units (68T states)
;; carry set = edge arrived within time
;; carry clear = edge arrived too late

2b4f 79        ld      a,c		; [1] update edge timer
2b50 c602      add     a,$02		; [2]
2b52 4f        ld      c,a		; [1]
2b53 380e      jr      c,$2b63          ; [3] overflow?

2b55 ed78      in      a,(c)		; [4] read cassette input data
2b57 ad        xor     l		; [1]
2b58 e680      and     $80		; [2] isolate cassette input in bit 7
2b5a 20f3      jr      nz,$2b4f         ; [3] has bit 7 (cassette data input) changed state?

;; pulse successfully read

2b5c af        xor     a
2b5d ed4f      ld      r,a

2b5f cb0d      rrc     l		; toggles between 0 and 1 

2b61 37        scf     
2b62 c9        ret     

;; time-out
2b63 af        xor     a
2b64 ed4f      ld      r,a
2b66 3c        inc     a			; "read error a"
2b67 c9        ret     

;;========================================================================================
;; write data byte to cassette
;; A = data byte
2b68 d5        push    de
2b69 1e08      ld      e,$08			;; number of bits
2b6b 57        ld      d,a

2b6c cb02      rlc     d				;; shift bit state into carry
2b6e cd782b    call    $2b78			;; write bit to cassette
2b71 3003      jr      nc,$2b76         

2b73 1d        dec     e
2b74 20f6      jr      nz,$2b6c         ;; loop for next bit

2b76 d1        pop     de
2b77 c9        ret     

;;========================================================================================
;; write bit to cassette
;;
;; carry flag = state of bit
;; carry set = 1 data bit
;; carry clear = 0 data bit

2b78 ed4be8b1  ld      bc,($b1e8)
2b7c 2aeab1    ld      hl,($b1ea)
2b7f 9f        sbc     a,a
2b80 67        ld      h,a
2b81 2807      jr      z,$2b8a          ; (+$07)
2b83 7d        ld      a,l
2b84 87        add     a,a
2b85 80        add     a,b
2b86 6f        ld      l,a
2b87 79        ld      a,c
2b88 90        sub     b
2b89 4f        ld      c,a
2b8a 7d        ld      a,l
2b8b 32e8b1    ld      ($b1e8),a

;; write a low level
2b8e 2e0a      ld      l,$0a			; %00001010 = clear bit 5 (cassette write data)
2b90 cda72b    call    $2ba7

2b93 3806      jr      c,$2b9b          ; (+$06)
2b95 91        sub     c
2b96 300c      jr      nc,$2ba4         ; (+$0c)
2b98 2f        cpl     
2b99 3c        inc     a
2b9a 4f        ld      c,a
2b9b 7c        ld      a,h
2b9c cd002b    call    $2b00			; update crc

;; write a high level
2b9f 2e0b      ld      l,$0b			; %00001011 = set bit 5 (cassette write data)
2ba1 cda72b    call    $2ba7

2ba4 3e01      ld      a,$01			
2ba6 c9        ret     


;;=====================================================================
;; write level to cassette
;; uses PPI control bit set/clear function
;; L = PPI Control byte 
;;   bit 7 = 0
;;   bit 3,2,1 = bit index
;;   bit 0: 1=bit set, 0=bit clear

2ba7 ed5f      ld      a,r
2ba9 cb3f      srl     a
2bab 91        sub     c
2bac 3003      jr      nc,$2bb1         ; 

;; delay in 4us (16T-state) units
;; total delay = ((A-1)*4) + 3

2bae 3c        inc     a				; [1]
2baf 20fd      jr      nz,$2bae         ; [3] 

;; set low/high level
2bb1 06f7      ld      b,$f7			; PPI control 
2bb3 ed69      out     (c),l			; set control

2bb5 f5        push    af
2bb6 af        xor     a
2bb7 ed4f      ld      r,a
2bb9 f1        pop     af
2bba c9        ret     

;;=====================================================================
;; CAS START MOTOR
;;
;; start cassette motor (if cassette motor was previously off
;; allow to to achieve full rotational speed)
2bbb 3e10      ld      a,$10			; start cassette motor
2bbd 1802      jr      $2bc1            ; CAS RESTORE MOTOR 

;;=====================================================================
;; CAS STOP MOTOR

2bbf 3eef      ld      a,$ef			; stop cassette motor

;;=====================================================================
;; CAS RESTORE MOTOR
;;
;; - if motor was switched from off->on, delay for a time to allow
;; cassette motor to achieve full rotational speed
;; - if motor was switched from on->off, do nothing

;; bit 4 of register A = cassette motor state
2bc1 c5        push    bc

2bc2 06f6      ld      b,$f6		; B = I/O address for PPI port C 
2bc4 ed48      in      c,(c)		; read current inputs (includes cassette input data)
2bc6 04        inc     b			; B = I/O address for PPI control		

2bc7 e610      and     $10			; isolate cassette motor state from requested
									; cassette motor status
									
2bc9 3e08      ld      a,$08		; %00001000	= cassette motor off
2bcb 2801      jr      z,$2bce

2bcd 3c        inc     a			; %00001001 = cassette motor on

2bce ed79      out     (c),a		; set the requested motor state
									; (uses PPI Control bit set/reset feature)

2bd0 37        scf     
2bd1 280c      jr      z,$2bdf          

2bd3 79        ld      a,c
2bd4 e610      and     $10			; previous state

2bd6 c5        push    bc
2bd7 01c800    ld      bc,$00c8		; delay in 1/100ths of a second
2bda 37        scf     
2bdb cce22b    call    z,$2be2		; delay for 2 seconds
2bde c1        pop     bc

2bdf 79        ld      a,c
2be0 c1        pop     bc
2be1 c9        ret     

;;=================================================================
;; delay & check for escape; allows cassette motor to achieve full
;; rotational speed

;; entry conditions:
;; B = delay factor in 1/100ths of a second

;; exit conditions:
;; c = delay completed and escape was not pressed
;; nc = escape was pressed

2be2 c5        push    bc
2be3 e5        push    hl
2be4 cdf92b    call    $2bf9		;; 1/100th of a second delay

2be7 3e42      ld      a,$42		;; keycode for escape key 
2be9 cd451e    call    $1e45		;; check for escape pressed (km test key)
									;; if non-zero then escape key has been pressed
									;; if zero, then escape key is not pressed
2bec e1        pop     hl
2bed c1        pop     bc
2bee 2007      jr      nz,$2bf7		;; escape key pressed?

;; continue delay
2bf0 0b        dec     bc
2bf1 78        ld      a,b
2bf2 b1        or      c
2bf3 20ed      jr      nz,$2be2 

;; delay completed successfully and escape was not pressed
2bf5 37        scf     
2bf6 c9        ret     

;; escape was pressed
2bf7 af        xor     a
2bf8 c9        ret     

;;========================================================================================
;; 1/10th of a second delay

2bf9 018206    ld      bc,$0682			; [3]

;; total delay is ((BC-1)*(2+1+1+3)) + (2+1+1+2) + 3 + 3 = 11667 microseconds
;; there are 1000000 microseconds in a second
;; therefore delay is 11667/1000000 = 0.01 seconds or 1/100th of a second

2bfc 0b        dec     bc				; [2]
2bfd 78        ld      a,b				; [1]
2bfe b1        or      c				; [1]
2bff 20fb      jr      nz,$2bfc         ; [3]

2c01 c9        ret						; [3]     
;;========================================================================================
;; EDIT
;; HL = address of buffer

2c02 c5        push    bc
2c03 d5        push    de
2c04 e5        push    hl
2c05 cdf22d    call    $2df2			; reset relative cursor pos
2c08 01ff00    ld      bc,$00ff			
; B = position in edit buffer
; C = number of characters remaining in buffer

;; if there is a number at the start of the line then skip it
2c0b 7e        ld      a,(hl)
2c0c fe30      cp      $30				; '0'
2c0e 3807      jr      c,$2c17          ; (+$07)
2c10 fe3a      cp      $3a				; '9'+1
2c12 dc422c    call    c,$2c42
2c15 38f4      jr      c,$2c0b          

;;--------------------------------------------------------------------
;; all other characters
2c17 78        ld      a,b
2c18 b7        or      a
;; zero flag set if start of buffer, zero flag clear if not start of buffer

2c19 7e        ld      a,(hl)
2c1a c4422c    call    nz,$2c42

2c1d e5        push    hl
2c1e 0c        inc     c
2c1f 7e        ld      a,(hl)
2c20 23        inc     hl
2c21 b7        or      a
2c22 20fa      jr      nz,$2c1e         ; (-$06)

2c24 3215b1    ld      ($b115),a		; insert/overwrite mode
2c27 e1        pop     hl
2c28 cde42e    call    $2ee4


2c2b c5        push    bc
2c2c e5        push    hl
2c2d cd562f    call    $2f56
2c30 e1        pop     hl
2c31 c1        pop     bc
2c32 cd482c    call    $2c48			; process key
2c35 30f4      jr      nc,$2c2b         ; (-$0c)

2c37 f5        push    af
2c38 cd4f2e    call    $2e4f
2c3b f1        pop     af
2c3c e1        pop     hl
2c3d d1        pop     de
2c3e c1        pop     bc
2c3f fefc      cp      $fc
2c41 c9        ret     

;;--------------------------------------------------------------------
;; used to skip characters in input buffer

2c42 0c        inc     c
2c43 04        inc     b		; increment pos
2c44 23        inc     hl		; increment position in buffer
2c45 c3252f    jp      $2f25

;;--------------------------------------------------------------------

2c48 e5        push    hl
2c49 21722c    ld      hl,$2c72
2c4c 5f        ld      e,a
2c4d 78        ld      a,b
2c4e b1        or      c
2c4f 7b        ld      a,e
2c50 200b      jr      nz,$2c5d         ; (+$0b)

2c52 fef0      cp      $f0				;
2c54 3807      jr      c,$2c5d          ; (+$07)
2c56 fef4      cp      $f4
2c58 3003      jr      nc,$2c5d         ; (+$03)

;; cursor keys
2c5a 21ae2c    ld      hl,$2cae

;;--------------------------------------------------------------------
2c5d 56        ld      d,(hl)
2c5e 23        inc     hl
2c5f e5        push    hl
2c60 23        inc     hl
2c61 23        inc     hl
2c62 be        cp      (hl)
2c63 23        inc     hl
2c64 2804      jr      z,$2c6a          ; (+$04)
2c66 15        dec     d
2c67 20f7      jr      nz,$2c60         ; (-$09)
2c69 e3        ex      (sp),hl
2c6a f1        pop     af
2c6b 7e        ld      a,(hl)
2c6c 23        inc     hl
2c6d 66        ld      h,(hl)
2c6e 6f        ld      l,a
2c6f 7b        ld      a,e
2c70 e3        ex      (sp),hl
2c71 c9        ret     

;; keys for editing an existing line
2c72 
defb &13
defw &2d8a
defb &fc								; ESC key
defw &2cd0								
defb &ef
defw &2cce
defb &0d								; RETURN key
defw &2cf2
defb &f0								; up cursor key
defw &2d3c
defb &f1								; down cursor key
defw &2d0a
defb &f2								; left cursor key
defw &2d34
defb &f3								; right cursor key
defw &2d02
defb &f8								; CTRL key + up cursor key
defw &2d4f
defb &f9								; CTRL key + down cursor key
defw &2d1d
defb &fa								; CTRL key + left cursor key
defw &2d45
defb &fb								; CTRL key + right cursor key
defw &2d14
defb &f4								; SHIFT key + up cursor key
defw &2e21
defb &f5								; SHIFT key + down cursor key
defw &2e26
defb &f6								; SHIFT key + left cursor key
defw &2e1c
defb &f7								; SHIFT key + right cursor key
defw &2e17								
defb &e0								; COPY key
defw &2e65
defb &7f								; ESC key
defw &2dc3
defb &10								; CLR key
defw &2dcd
defb &e1								; CTRL key+TAB key (toggle insert/overwrite)
defw &2d81

;;--------------------------------------------------------------------

;; keys for 
2cae
defb &04
defw &2cfe								; Sound bleeper
defb &f0								; up cursor key
defw &2cbd								; Move cursor up a line
defb &f1								; down cursor key
defw &2cc1								; Move cursor down a line
defb &f2								; left cursor key
defw &2cc9								; Move cursor back one character
defb &f3								; right cursor key
defw &2cc5								; Move cursor forward one character

;;--------------------------------------------------------------------
;; up cursor key pressed
2cbd 3e0b      ld      a,$0b			; VT (Move cursor up a line)
2cbf 180a      jr      $2ccb            ; 

;;--------------------------------------------------------------------
;; down cursor key pressed
2cc1 3e0a      ld      a,$0a			; LF (Move cursor down a line)
2cc3 1806      jr      $2ccb           

;;--------------------------------------------------------------------
;; right cursor key pressed
2cc5 3e09      ld      a,$09			; TAB (Move cursor forward one character)
2cc7 1802      jr      $2ccb            ; 

;;--------------------------------------------------------------------
;; left cursor key pressed
2cc9 3e08      ld      a,$08			; BS (Move character back one character)

;;--------------------------------------------------------------------

2ccb cdfe13    call    $13fe			; TXT OUTPUT

;;--------------------------------------------------------------------
2cce b7        or      a
2ccf c9        ret     

;;--------------------------------------------------------------------

2cd0 cdf22c    call    $2cf2			; display message
2cd3 f5        push    af
2cd4 21ea2c    ld      hl,$2cea			; "*Break*"
2cd7 cdf22c    call    $2cf2			; display message

2cda cd7c11    call    $117c			; TXT GET CURSOR
2cdd 25        dec     h
2cde 2808      jr      z,$2ce8          

;; go to next line
2ce0 3e0d      ld      a,$0d			; CR (Move cursor to left edge of window on current line)
2ce2 cdfe13    call    $13fe			; TXT OUTPUT
2ce5 cdc12c    call    $2cc1			; Move cursor down a line

2ce8 f1        pop     af
2ce9 c9        ret     

;;--------------------------------------------------------------------
2cea 
defb "*Break*",0

;;--------------------------------------------------------------------
;; display 0 terminated string

2cf2 f5        push af
2cf3 7e        ld      a,(hl)			; get character
2cf4 23        inc     hl
2cf5 b7        or      a				; end of string marker?
2cf6 c4252f    call    nz,$2f25			; display character
2cf9 20f8      jr      nz,$2cf3         ; loop for next character
2cfb f1        pop     af
2cfc 37        scf     
2cfd c9        ret     

;;===========================================================================
2cfe 3e07      ld      a,$07			; BEL (Sound bleeper)
2d00 18c9      jr      $2ccb

;;===========================================================================
;; right cursor key pressed
2d02 1601      ld      d,$01
2d04 cd1e2d    call    $2d1e
2d07 28f5      jr      z,$2cfe          ; (-$0b)
2d09 c9        ret     

;;===========================================================================
;; down cursor key pressed

2d0a cd732d    call    $2d73
2d0d 79        ld      a,c
2d0e 90        sub     b
2d0f ba        cp      d
2d10 38ec      jr      c,$2cfe          ; (-$14)
2d12 180a      jr      $2d1e            ; (+$0a)

;;--------------------------------------------------------------------
;; CTRL key + right cursor key pressed
;; 
;; go to end of current line
2d14 cd732d    call    $2d73
2d17 7a        ld      a,d
2d18 93        sub     e
2d19 c8        ret     z

2d1a 57        ld      d,a
2d1b 1801      jr      $2d1e            ; (+$01)

;;--------------------------------------------------------------------
;; CTRL key + down cursor key pressed
;;
;; go to end of text 

2d1d 51        ld      d,c

;;--------------------------------------------------------------------

2d1e 78        ld      a,b
2d1f b9        cp      c
2d20 c8        ret     z

2d21 d5        push    de
2d22 cdcd2e    call    $2ecd
2d25 7e        ld      a,(hl)
2d26 d4252f    call    nc,$2f25
2d29 04        inc     b
2d2a 23        inc     hl
2d2b d4e42e    call    nc,$2ee4
2d2e d1        pop     de
2d2f 15        dec     d
2d30 20ec      jr      nz,$2d1e         ; (-$14)
2d32 183c      jr      $2d70            ; (+$3c)

;;===========================================================================
;; left cursor key pressed
2d34 1601      ld      d,$01
2d36 cd502d    call    $2d50
2d39 28c3      jr      z,$2cfe          ; (-$3d)
2d3b c9        ret     


;;===========================================================================
;; up cursor key pressed
2d3c cd732d    call    $2d73
2d3f 78        ld      a,b
2d40 ba        cp      d
2d41 38bb      jr      c,$2cfe          ; (-$45)
2d43 180b      jr      $2d50            ; (+$0b)


;;===========================================================================
;; CTRL key + left cursor key pressed
;;
;; go to start of current line

2d45 cd732d    call    $2d73
2d48 7b        ld      a,e
2d49 d601      sub     $01
2d4b c8        ret     z

2d4c 57        ld      d,a
2d4d 1801      jr      $2d50            ; (+$01)

;;===========================================================================
;; CTRL key + up cursor key pressed

;; go to start of text

2d4f 51        ld      d,c

2d50 78        ld      a,b
2d51 b7        or      a
2d52 c8        ret     z

2d53 cdc72e    call    $2ec7
2d56 3007      jr      nc,$2d5f         ; (+$07)
2d58 05        dec     b
2d59 2b        dec     hl
2d5a 15        dec     d
2d5b 20f3      jr      nz,$2d50         ; (-$0d)
2d5d 1811      jr      $2d70            ; (+$11)

;;===========================================================================
2d5f 78        ld      a,b
2d60 b7        or      a
2d61 280a      jr      z,$2d6d          ; (+$0a)
2d63 05        dec     b
2d64 2b        dec     hl
2d65 d5        push    de
2d66 cda22e    call    $2ea2
2d69 d1        pop     de
2d6a 15        dec     d
2d6b 20f2      jr      nz,$2d5f         ; (-$0e)
2d6d cde42e    call    $2ee4
2d70 f6ff      or      $ff
2d72 c9        ret     

;;--------------------------------------------------------------------
2d73 e5        push    hl
2d74 cd5212    call    $1252			; TXT GET WINDOW
2d77 7a        ld      a,d
2d78 94        sub     h
2d79 3c        inc     a
2d7a 57        ld      d,a
2d7b cd7c11    call    $117c			; TXT GET CURSOR
2d7e 5c        ld      e,h
2d7f e1        pop     hl
2d80 c9        ret     
;;--------------------------------------------------------------------
;; CTRL key + TAB key
;; 
;; toggle insert/overwrite mode
2d81 3a15b1    ld      a,($b115)		; insert/overwrite mode
2d84 2f        cpl     
2d85 3215b1    ld      ($b115),a
2d88 b7        or      a
2d89 c9        ret     

;;--------------------------------------------------------------------
2d8a b7        or      a
2d8b c8        ret     z

2d8c 5f        ld      e,a
2d8d 3a15b1    ld      a,($b115)		; insert/overwrite mode
2d90 b7        or      a
2d91 79        ld      a,c
2d92 280b      jr      z,$2d9f          ; (+$0b)
2d94 b8        cp      b
2d95 2808      jr      z,$2d9f          ; (+$08)
2d97 73        ld      (hl),e
2d98 23        inc     hl
2d99 04        inc     b
2d9a b7        or      a
2d9b 7b        ld      a,e
2d9c c3252f    jp      $2f25

2d9f feff      cp      $ff
2da1 cafe2c    jp      z,$2cfe
2da4 af        xor     a
2da5 3214b1    ld      ($b114),a
2da8 cd9b2d    call    $2d9b
2dab 0c        inc     c
2dac e5        push    hl
2dad 7e        ld      a,(hl)
2dae 73        ld      (hl),e
2daf 5f        ld      e,a
2db0 23        inc     hl
2db1 b7        or      a
2db2 20f9      jr      nz,$2dad         ; (-$07)
2db4 77        ld      (hl),a
2db5 e1        pop     hl
2db6 04        inc     b
2db7 23        inc     hl
2db8 cde42e    call    $2ee4
2dbb 3a14b1    ld      a,($b114)
2dbe b7        or      a
2dbf c4a22e    call    nz,$2ea2
2dc2 c9        ret     

;; ESC key pressed
2dc3 78        ld      a,b
2dc4 b7        or      a
2dc5 c4c72e    call    nz,$2ec7
2dc8 d2fe2c    jp      nc,$2cfe
2dcb 05        dec     b
2dcc 2b        dec     hl

;; CLR key pressed
2dcd 78        ld      a,b
2dce b9        cp      c
2dcf cafe2c    jp      z,$2cfe
2dd2 e5        push    hl
2dd3 23        inc     hl
2dd4 7e        ld      a,(hl)
2dd5 2b        dec     hl
2dd6 77        ld      (hl),a
2dd7 23        inc     hl
2dd8 b7        or      a
2dd9 20f8      jr      nz,$2dd3         ; (-$08)
2ddb 2b        dec     hl
2ddc 3620      ld      (hl),$20
2dde 3214b1    ld      ($b114),a
2de1 e3        ex      (sp),hl
2de2 cde42e    call    $2ee4
2de5 e3        ex      (sp),hl
2de6 3600      ld      (hl),$00
2de8 e1        pop     hl
2de9 0d        dec     c
2dea 3a14b1    ld      a,($b114)
2ded b7        or      a
2dee c4a62e    call    nz,$2ea6
2df1 c9        ret     


;;--------------------------------------------------------------------
;; initialise relative copy cursor position to origin
2df2 af        xor     a
2df3 3216b1    ld      ($b116),a
2df6 3217b1    ld      ($b117),a
2df9 c9        ret     

;;--------------------------------------------------------------------
;; compare copy cursor relative position
;; HL = cursor position
2dfa ed5b16b1  ld      de,($b116)
2dfe 7c        ld      a,h
2dff aa        xor     d
2e00 c0        ret     nz
2e01 7d        ld      a,l
2e02 ab        xor     e
2e03 c0        ret     nz
2e04 37        scf     
2e05 c9        ret     
;;--------------------------------------------------------------------

2e06 4f        ld      c,a
2e07 cdc12e    call    $2ec1			; get copy cursor position
2e0a c8        ret     z				; quit if not active

;; adjust y position
2e0b 7d        ld      a,l
2e0c 81        add     a,c
2e0d 6f        ld      l,a

;; validate new position
2e0e cdca11    call    $11ca			; TXT VALIDATE
2e11 30df      jr      nc,$2df2         ; reset relative cursor pos

;; set cursor position
2e13 2216b1    ld      ($b116),hl
2e16 c9        ret     

;;--------------------------------------------------------------------
;; SHIFT key + left cursor key
;; 
;; move copy cursor left
2e17 110001    ld      de,$0100
2e1a 180d      jr      $2e29            ; (+$0d)
;;--------------------------------------------------------------------
;; SHIFT key + right cursor pressed
;; 
;; move copy cursor right
2e1c 1100ff    ld      de,$ff00
2e1f 1808      jr      $2e29            ; (+$08)
;;--------------------------------------------------------------------
;; SHIFT key + up cursor pressed
;;
;; move copy cursor up
2e21 11ff00    ld      de,$00ff
2e24 1803      jr      $2e29            ; (+$03)
;;--------------------------------------------------------------------
;; SHIFT key + left cursor pressed
;;
;; move copy cursor down
2e26 110100    ld      de,$0001

;;--------------------------------------------------------------------
;; D = column increment
;; E = row increment
2e29 c5        push    bc
2e2a e5        push    hl
2e2b cdc12e    call    $2ec1			; get copy cursor position

;; get cursor position
2e2e cc7c11    call    z,$117c			; TXT GET CURSOR

;; adjust cursor position

;; adjust column
2e31 7c        ld      a,h
2e32 82        add     a,d
2e33 67        ld      h,a

;; adjust row
2e34 7d        ld      a,l
2e35 83        add     a,e
2e36 6f        ld      l,a
;; validate the position
2e37 cdca11    call    $11ca			; TXT VALIDATE
2e3a 300b      jr      nc,$2e47         ; position invalid?

;; position is valid

2e3c e5        push    hl
2e3d cd4f2e    call    $2e4f
2e40 e1        pop     hl

;; store new position
2e41 2216b1    ld      ($b116),hl

2e44 cd4a2e    call    $2e4a

;;----------------

2e47 e1        pop     hl
2e48 c1        pop     bc
2e49 c9        ret     

;;--------------------------------------------------------------------

2e4a 116512    ld      de,$1265			; TXT PLACE CURSOR/TXT REMOVE CURSOR
2e4d 1803      jr      $2e52            

;;--------------------------------------------------------------------
2e4f 116512    ld      de,$1265			; TXT PLACE CURSOR/TXT REMOVE CURSOR

;;--------------------------------------------------------------------
2e52 cdc12e    call    $2ec1			; get copy cursor position
2e55 c8        ret     z

2e56 e5        push    hl
2e57 cd7c11    call    $117c			; TXT GET CURSOR
2e5a e3        ex      (sp),hl
2e5b cd7011    call    $1170			; TXT SET CURSOR
2e5e cd1600    call    $0016			; LOW: PCDE INSTRUCTION
2e61 e1        pop     hl
2e62 c37011    jp      $1170			; TXT SET CURSOR
;;--------------------------------------------------------------------
;; COPY key pressed
2e65 c5        push    bc
2e66 e5        push    hl
2e67 cd7c11    call    $117c			; TXT GET CURSOR
2e6a eb        ex      de,hl
2e6b cdc12e    call    $2ec1
2e6e 200c      jr      nz,$2e7c         ; perform copy
2e70 78        ld      a,b
2e71 b1        or      c
2e72 2026      jr      nz,$2e9a         ; (+$26)
2e74 cd7c11    call    $117c			; TXT GET CURSOR
2e77 2216b1    ld      ($b116),hl
2e7a 1806      jr      $2e82            ; (+$06)

;;--------------------------------------------------------------------

2e7c cd7011    call    $1170			; TXT SET CURSOR
2e7f cd6512    call    $1265			; TXT PLACE CURSOR/TXT REMOVE CURSOR

2e82 cdac13    call    $13ac			; TXT RD CHAR
2e85 f5        push    af
2e86 eb        ex      de,hl
2e87 cd7011    call    $1170			; TXT SET CURSOR
2e8a 2a16b1    ld      hl,($b116)
2e8d 24        inc     h
2e8e cdca11    call    $11ca			; TXT VALIDATE
2e91 3003      jr      nc,$2e96         ; (+$03)
2e93 2216b1    ld      ($b116),hl
2e96 cd4a2e    call    $2e4a
2e99 f1        pop     af
2e9a e1        pop     hl
2e9b c1        pop     bc
2e9c da8a2d    jp      c,$2d8a
2e9f c3fe2c    jp      $2cfe

;;--------------------------------------------------------------------

2ea2 1601      ld      d,$01
2ea4 1802      jr      $2ea8            ; (+$02)

;;--------------------------------------------------------------------

2ea6 16ff      ld      d,$ff
;;--------------------------------------------------------------------
2ea8 c5        push    bc
2ea9 e5        push    hl
2eaa d5        push    de
2eab cd4f2e    call    $2e4f
2eae d1        pop     de
2eaf cdc12e    call    $2ec1
2eb2 2809      jr      z,$2ebd          ; (+$09)
2eb4 7c        ld      a,h
2eb5 82        add     a,d
2eb6 67        ld      h,a
2eb7 cd0e2e    call    $2e0e
2eba cd4a2e    call    $2e4a
2ebd e1        pop     hl
2ebe c1        pop     bc
2ebf b7        or      a
2ec0 c9        ret     

;;--------------------------------------------------------------------
;; get copy cursor position
;; this is relative to the actual cursor pos
;;
;; zero flag set if cursor is not active
2ec1 2a16b1    ld      hl,($b116)
2ec4 7c        ld      a,h
2ec5 b5        or      l
2ec6 c9        ret     
;;--------------------------------------------------------------------
;; try to move cursor left?
2ec7 d5        push    de
2ec8 1108ff    ld      de,$ff08
2ecb 1804      jr      $2ed1            ; (+$04)

;;--------------------------------------------------------------------
;; try to move cursor right?
2ecd d5        push    de
2ece 110901    ld      de,$0109
;;--------------------------------------------------------------------
;; D = column increment
;; E = character to plot
2ed1 c5        push    bc
2ed2 e5        push    hl

;; get current cursor position
2ed3 cd7c11    call    $117c			; TXT GET CURSOR

;; adjust cursor position
2ed6 7a        ld      a,d				; column increment
2ed7 84        add     a,h				; add on column
2ed8 67        ld      h,a				; final column

;; validate this new position
2ed9 cdca11    call    $11ca			; TXT VALIDATE

;; if valid then output character, otherwise report error
2edc 7b        ld      a,e
2edd dcfe13    call    c,$13fe			; TXT OUTPUT

2ee0 e1        pop     hl
2ee1 c1        pop     bc
2ee2 d1        pop     de
2ee3 c9        ret     

;;--------------------------------------------------------------------
2ee4 c5        push    bc
2ee5 e5        push    hl
2ee6 eb        ex      de,hl
2ee7 cd7c11    call    $117c			; TXT GET CURSOR
2eea 4f        ld      c,a
2eeb eb        ex      de,hl
2eec 7e        ld      a,(hl)
2eed 23        inc     hl
2eee b7        or      a
2eef c4022f    call    nz,$2f02
2ef2 20f8      jr      nz,$2eec         ; (-$08)
2ef4 cd7c11    call    $117c			; TXT GET CURSOR
2ef7 91        sub     c
2ef8 eb        ex      de,hl
2ef9 85        add     a,l
2efa 6f        ld      l,a
2efb cd7011    call    $1170			; TXT SET CURSOR
2efe e1        pop     hl
2eff c1        pop     bc
2f00 b7        or      a
2f01 c9        ret     

2f02 f5        push    af
2f03 c5        push    bc
2f04 d5        push    de
2f05 e5        push    hl
2f06 47        ld      b,a
2f07 cd7c11    call    $117c			; TXT GET CURSOR
2f0a 91        sub     c
2f0b 83        add     a,e
2f0c 5f        ld      e,a
2f0d 48        ld      c,b
2f0e cdca11    call    $11ca			; TXT VALIDATE
2f11 3805      jr      c,$2f18          ; (+$05)
2f13 78        ld      a,b
2f14 87        add     a,a
2f15 3c        inc     a
2f16 83        add     a,e
2f17 5f        ld      e,a
2f18 eb        ex      de,hl
2f19 cdca11    call    $11ca			; TXT VALIDATE
2f1c 79        ld      a,c
2f1d dc252f    call    c,$2f25
2f20 e1        pop     hl
2f21 d1        pop     de
2f22 c1        pop     bc
2f23 f1        pop     af
2f24 c9        ret     

2f25 f5        push    af
2f26 c5        push    bc
2f27 d5        push    de
2f28 e5        push    hl
2f29 47        ld      b,a
2f2a cd7c11    call    $117c			; TXT GET CURSOR
2f2d 4f        ld      c,a
2f2e c5        push    bc
2f2f cdca11    call    $11ca			; TXT VALIDATE
2f32 c1        pop     bc
2f33 dcfa2d    call    c,$2dfa
2f36 f5        push    af
2f37 dc4f2e    call    c,$2e4f
2f3a 78        ld      a,b
2f3b c5        push    bc
2f3c cd3513    call    $1335			; TXT WR CHAR
2f3f c1        pop     bc
2f40 cd7c11    call    $117c			; TXT GET CURSOR
2f43 91        sub     c
2f44 c4062e    call    nz,$2e06
2f47 f1        pop     af
2f48 3007      jr      nc,$2f51         ; (+$07)
2f4a 9f        sbc     a,a
2f4b 3214b1    ld      ($b114),a
2f4e cd4a2e    call    $2e4a
2f51 e1        pop     hl
2f52 d1        pop     de
2f53 c1        pop     bc
2f54 f1        pop     af
2f55 c9        ret     

2f56 cd7c11    call    $117c			; TXT GET CURSOR
2f59 4f        ld      c,a
2f5a cdca11    call    $11ca			; TXT VALIDATE
2f5d cdfa2d    call    $2dfa
2f60 dabf1b    jp      c,$1bbf			; KM WAIT CHAR
2f63 cd7612    call    $1276			; TXT CUR ON
2f66 cd7c11    call    $117c			; TXT GET CURSOR
2f69 91        sub     c
2f6a c4062e    call    nz,$2e06
2f6d cdbf1b    call    $1bbf			; KM WAIT CHAR
2f70 c37e12    jp      $127e			; TXT CUR OFF

;;===========================================================================================
;; MATHS FUNCTIONS

;; REAL: PI
2f73 11782f    ld      de,$2f78
2f76 1819      jr      $2f91           

2f78 
defb &a2,&da,&0f,&49,&82				;; PI in floating point format

;;===========================================================================================
;; REAL: ONE
2f7d 11822f    ld      de,$2f82
2f80 180f      jr      $2f91            ; (+$0f)

2f82 
defb &00,&00,&00,&00,&81			;; 1 in floating point format

;;===========================================================================================
2f87 eb        ex      de,hl
2f88 210eb1    ld      hl,$b10e
2f8b 1804      jr      $2f91            ; (+$04)

;;===========================================================================================
2f8d 1104b1    ld      de,$b104
2f90 eb        ex      de,hl

;; REAL move
;; HL = points to address to write floating point number to
;; DE = points to address of a floating point number

2f91 e5        push    hl
2f92 d5        push    de
2f93 c5        push    bc
2f94 eb        ex      de,hl
2f95 010500    ld      bc,$0005
2f98 edb0      ldir    
2f9a c1        pop     bc
2f9b d1        pop     de
2f9c e1        pop     hl
2f9d 37        scf     
2f9e c9        ret     

;; INT to real
2f9f d5        push    de
2fa0 c5        push    bc
2fa1 f67f      or      $7f
2fa3 47        ld      b,a
2fa4 af        xor     a
2fa5 12        ld      (de),a
2fa6 13        inc     de
2fa7 12        ld      (de),a
2fa8 13        inc     de
2fa9 0e90      ld      c,$90
2fab b4        or      h
2fac 200d      jr      nz,$2fbb         ; (+$0d)
2fae 4f        ld      c,a
2faf b5        or      l
2fb0 280d      jr      z,$2fbf          ; (+$0d)
2fb2 6c        ld      l,h
2fb3 0e88      ld      c,$88
2fb5 1804      jr      $2fbb            ; (+$04)
2fb7 0d        dec     c
2fb8 cb25      sla     l
2fba 8f        adc     a,a
2fbb f2b72f    jp      p,$2fb7
2fbe a0        and     b
2fbf eb        ex      de,hl
2fc0 73        ld      (hl),e
2fc1 23        inc     hl
2fc2 77        ld      (hl),a
2fc3 23        inc     hl
2fc4 71        ld      (hl),c
2fc5 c1        pop     bc
2fc6 e1        pop     hl
2fc7 c9        ret     

;; BIN to real
2fc8 c5        push    bc
2fc9 0100a0    ld      bc,$a000
2fcc cdd32f    call    $2fd3
2fcf c1        pop     bc
2fd0 c9        ret     

2fd1 06a8      ld      b,$a8
2fd3 d5        push    de
2fd4 cd9c37    call    $379c
2fd7 d1        pop     de
2fd8 c9        ret     

;; REAL to int
2fd9 e5        push    hl
2fda dde1      pop     ix
2fdc af        xor     a
2fdd dd9604    sub     (ix+$04)
2fe0 281b      jr      z,$2ffd          ; (+$1b)
2fe2 c690      add     a,$90
2fe4 d0        ret     nc

2fe5 d5        push    de
2fe6 c5        push    bc
2fe7 c610      add     a,$10
2fe9 cd3d37    call    $373d
2fec cb21      sla     c
2fee ed5a      adc     hl,de
2ff0 2808      jr      z,$2ffa          ; (+$08)
2ff2 dd7e03    ld      a,(ix+$03)
2ff5 b7        or      a
2ff6 3f        ccf     
2ff7 c1        pop     bc
2ff8 d1        pop     de
2ff9 c9        ret     

2ffa 9f        sbc     a,a
2ffb 18f9      jr      $2ff6            ; (-$07)
2ffd 6f        ld      l,a
2ffe 67        ld      h,a
2fff 37        scf     
3000 c9        ret     

;; REAL to bin
3001 cd1430    call    $3014
3004 d0        ret     nc

3005 f0        ret     p

3006 e5        push    hl
3007 79        ld      a,c
3008 34        inc     (hl)
3009 2006      jr      nz,$3011         ; (+$06)
300b 23        inc     hl
300c 3d        dec     a
300d 20f9      jr      nz,$3008         ; (-$07)
300f 34        inc     (hl)
3010 0c        inc     c
3011 e1        pop     hl
3012 37        scf     
3013 c9        ret     

;; REAL fix

3014 e5        push    hl
3015 d5        push    de
3016 e5        push    hl
3017 dde1      pop     ix
3019 af        xor     a
301a dd9604    sub     (ix+$04)
301d 200a      jr      nz,$3029         ; (+$0a)
301f 0604      ld      b,$04
3021 77        ld      (hl),a
3022 23        inc     hl
3023 10fc      djnz    $3021            ; (-$04)
3025 0e01      ld      c,$01
3027 1828      jr      $3051            ; (+$28)

3029 c6a0      add     a,$a0
302b 3025      jr      nc,$3052         ; (+$25)
302d e5        push    hl
302e cd3d37    call    $373d
3031 af        xor     a
3032 b8        cp      b
3033 8f        adc     a,a
3034 b1        or      c
3035 4d        ld      c,l
3036 44        ld      b,h
3037 e1        pop     hl
3038 71        ld      (hl),c
3039 23        inc     hl
303a 70        ld      (hl),b
303b 23        inc     hl
303c 73        ld      (hl),e
303d 23        inc     hl
303e 5f        ld      e,a
303f 7e        ld      a,(hl)
3040 72        ld      (hl),d
3041 e680      and     $80
3043 47        ld      b,a
3044 0e04      ld      c,$04
3046 af        xor     a
3047 b6        or      (hl)
3048 2005      jr      nz,$304f         ; (+$05)
304a 2b        dec     hl
304b 0d        dec     c
304c 20f9      jr      nz,$3047         ; (-$07)
304e 0c        inc     c
304f 7b        ld      a,e
3050 b7        or      a
3051 37        scf     
3052 d1        pop     de
3053 e1        pop     hl
3054 c9        ret     

;; REAL int

3055 cd1430    call    $3014
3058 d0        ret     nc

3059 c8        ret     z

305a cb78      bit     7,b
305c c8        ret     z

305d 18a7      jr      $3006            ; (-$59)
305f cd2737    call    $3727
3062 47        ld      b,a
3063 2852      jr      z,$30b7          ; (+$52)
3065 fc3437    call    m,$3734
3068 e5        push    hl
3069 dd7e04    ld      a,(ix+$04)
306c d680      sub     $80
306e 5f        ld      e,a
306f 9f        sbc     a,a
3070 57        ld      d,a
3071 6b        ld      l,e
3072 62        ld      h,d
3073 29        add     hl,hl
3074 29        add     hl,hl
3075 29        add     hl,hl
3076 19        add     hl,de
3077 29        add     hl,hl
3078 19        add     hl,de
3079 29        add     hl,hl
307a 29        add     hl,hl
307b 19        add     hl,de
307c 7c        ld      a,h
307d d609      sub     $09
307f 4f        ld      c,a
3080 e1        pop     hl
3081 c5        push    bc
3082 c4c830    call    nz,$30c8
3085 11bc30    ld      de,$30bc
3088 cde236    call    $36e2
308b 300b      jr      nc,$3098         ; (+$0b)
308d 11f530    ld      de,$30f5			; start of power's of ten
3090 cd7735    call    $3577
3093 d1        pop     de
3094 1d        dec     e
3095 d5        push    de
3096 18ed      jr      $3085            ; (-$13)
3098 11c130    ld      de,$30c1
309b cde236    call    $36e2
309e 380b      jr      c,$30ab          ; (+$0b)
30a0 11f530    ld      de,$30f5			; start of power's of ten
30a3 cd0436    call    $3604
30a6 d1        pop     de
30a7 1c        inc     e
30a8 d5        push    de
30a9 18ed      jr      $3098            ; (-$13)
30ab cd0130    call    $3001
30ae 79        ld      a,c
30af d1        pop     de
30b0 42        ld      b,d
30b1 3d        dec     a
30b2 85        add     a,l
30b3 6f        ld      l,a
30b4 d0        ret     nc

30b5 24        inc     h
30b6 c9        ret     

30b7 5f        ld      e,a
30b8 77        ld      (hl),a
30b9 0e01      ld      c,$01
30bb c9        ret     

30bc 
defb &f0,&1f,&bc,&3e,&96

30c1 
defb &fe,&27,&7b,&6e,&9e

;; REAL exp A
30c6 2f        cpl     
30c7 3c        inc     a
30c8 b7        or      a
30c9 37        scf     
30ca c8        ret     z

30cb 4f        ld      c,a
30cc f2d130    jp      p,$30d1
30cf 2f        cpl     
30d0 3c        inc     a
30d1 113131    ld      de,$3131
30d4 d60d      sub     $0d
30d6 2815      jr      z,$30ed          ; (+$15)
30d8 3809      jr      c,$30e3          ; (+$09)
30da c5        push    bc
30db f5        push    af
30dc cded30    call    $30ed
30df f1        pop     af
30e0 c1        pop     bc
30e1 18ee      jr      $30d1            ; (-$12)
30e3 47        ld      b,a
30e4 87        add     a,a
30e5 87        add     a,a
30e6 80        add     a,b
30e7 83        add     a,e
30e8 5f        ld      e,a
30e9 3eff      ld      a,$ff
30eb 8a        adc     a,d
30ec 57        ld      d,a
30ed 79        ld      a,c
30ee b7        or      a
30ef f20436    jp      p,$3604
30f2 c37735    jp      $3577

;;===========================================================================================
;; power's of 10 in internal floating point representation
;;
30f5 
defb &00,&00,&00,&00,&84			;; 10 (10^1)
defb &00,&00,&00,&48,&87			;; 100 (10^2)
defb &00,&00,&00,&7A,&8A			;; 1000 (10^3)
defb &00,&00,&40,&1c,&8e			;; 10000 (10^4) (1E+4)
defb &00,&00,&50,&43,&91			;; 100000 (10^5) (1E+5)
defb &00,&00,&24,&74,&94			;; 1000000 (10^6) (1E+6)
defb &00,&80,&96,&18,&98			;; 10000000 (10^7) (1E+7)
defb &00,&20,&bc,&3e,&9b			;; 100000000 (10^8) (1E+8)
defb &00,&28,&6b,&6e,&9e			;; 1000000000 (10^9) (1E+9)
defb &00,&f9,&02,&15,&a2			;; 10000000000 (10^10) (1E+10)
defb &40,&b7,&43,&3a,&a5			;; 100000000000 (10^11) (1E+11)
defb &10,&a5,&d4,&68,&a8			;; 1000000000000 (10^12) (1E+12)
defb &2a,&e7,&84,&11,&ac			;; 10000000000000 (10^13) (1E+13)
;;===========================================================================================

3136 216589    ld      hl,&8965
3139 2202b1    ld      ($b102),hl
313c 21076c    ld      hl,$6c07
313f 2200b1    ld      ($b100),hl
3142 c9        ret     

;; RANDOMIZE seed
3143 eb        ex      de,hl
3144 cd3631    call    $3136
3147 eb        ex      de,hl
3148 cd2737    call    $3727
314b c8        ret     z

314c 1100b1    ld      de,$b100
314f 0604      ld      b,$04
3151 1a        ld      a,(de)
3152 ae        xor     (hl)
3153 12        ld      (de),a
3154 13        inc     de
3155 23        inc     hl
3156 10f9      djnz    $3151            ; (-$07)
3158 c9        ret     

;; REAL rnd
3159 e5        push    hl
315a 2a02b1    ld      hl,($b102)
315d 01076c    ld      bc,$6c07
3160 cd9c31    call    $319c
3163 e5        push    hl
3164 2a00b1    ld      hl,($b100)
3167 016589    ld      bc,$8965
316a cd9c31    call    $319c
316d d5        push    de
316e e5        push    hl
316f 2a02b1    ld      hl,($b102)
3172 cd9c31    call    $319c
3175 e3        ex      (sp),hl
3176 09        add     hl,bc
3177 2200b1    ld      ($b100),hl
317a e1        pop     hl
317b 01076c    ld      bc,$6c07
317e ed4a      adc     hl,bc
3180 c1        pop     bc
3181 09        add     hl,bc
3182 c1        pop     bc
3183 09        add     hl,bc
3184 2202b1    ld      ($b102),hl
3187 e1        pop     hl
;; REAL rnd0
3188 e5        push    hl
3189 dde1      pop     ix
318b 2a00b1    ld      hl,($b100)
318e ed5b02b1  ld      de,($b102)
3192 010000    ld      bc,$0000
3195 dd360480  ld      (ix+$04),$80
3199 c3ac37    jp      $37ac
319c eb        ex      de,hl
319d 210000    ld      hl,$0000
31a0 3e11      ld      a,$11
31a2 3d        dec     a
31a3 c8        ret     z

31a4 29        add     hl,hl
31a5 cb13      rl      e
31a7 cb12      rl      d
31a9 30f7      jr      nc,$31a2         ; (-$09)
31ab 09        add     hl,bc
31ac 30f4      jr      nc,$31a2         ; (-$0c)
31ae 13        inc     de
31af 18f1      jr      $31a2            ; (-$0f)

;; REAL log10
31b1 112a32    ld      de,$322a
31b4 1803      jr      $31b9            ; (+$03)

;; REAL log
31b6 112532    ld      de,$3225
31b9 cd2737    call    $3727
31bc 3d        dec     a
31bd fe01      cp      $01
31bf d0        ret     nc

31c0 d5        push    de
31c1 cdd336    call    $36d3
31c4 f5        push    af
31c5 dd360480  ld      (ix+$04),$80
31c9 112032    ld      de,$3220
31cc cddf36    call    $36df
31cf 3006      jr      nc,$31d7         ; (+$06)
31d1 dd3404    inc     (ix+$04)
31d4 f1        pop     af
31d5 3d        dec     a
31d6 f5        push    af
31d7 cd872f    call    $2f87
31da d5        push    de
31db 11822f    ld      de,$2f82
31de d5        push    de
31df cda234    call    $34a2
31e2 d1        pop     de
31e3 e3        ex      (sp),hl
31e4 cd9a34    call    $349a
31e7 d1        pop     de
31e8 cd0436    call    $3604
31eb cd4034    call    $3440

31ee 04        inc     b
31ef 4c        ld      c,h
31f0 4b        ld      c,e
31f1 57        ld      d,a
31f2 5e        ld      e,(hl)
31f3 7f        ld      a,a
31f4 0d        dec     c
31f5 08        ex      af,af'
31f6 9b        sbc     a,e
31f7 13        inc     de
31f8 80        add     a,b
31f9 23        inc     hl
31fa 93        sub     e
31fb 3876      jr      c,$3273          ; (+$76)
31fd 80        add     a,b
31fe 203b      jr      nz,$323b         ; (+$3b)
3200 aa        xor     d
3201 3882      jr      c,$3185          ; (-$7e)
3203 d5        push    de
3204 cd7735    call    $3577
3207 d1        pop     de
3208 e3        ex      (sp),hl
3209 7c        ld      a,h
320a b7        or      a
320b f21032    jp      p,$3210
320e 2f        cpl     
320f 3c        inc     a
3210 6f        ld      l,a
3211 7c        ld      a,h
3212 2600      ld      h,$00
3214 cd9f2f    call    $2f9f
3217 eb        ex      de,hl
3218 e1        pop     hl
3219 cda234    call    $34a2
321c d1        pop     de
321d c37735    jp      $3577

3220 
defb &34,&f3,&04,&35,&80			;; 0.707106781
defb &f8,&17,&72,&31,&80			;; 0.693147181
defb &85,&9a,&20,&1a,&7f			;; 0.301029996

;; REAL exp
322f 06e1      ld      b,$e1
3231 cd9234    call    $3492
3234 d27d2f    jp      nc,$2f7d
3237 11a232    ld      de,$32a2
323a cddf36    call    $36df
323d f2e837    jp      p,$37e8
3240 11a732    ld      de,$32a7
3243 cddf36    call    $36df
3246 fae237    jp      m,$37e2
3249 119d32    ld      de,$329d
324c cd6934    call    $3469
324f 7b        ld      a,e
3250 f25532    jp      p,$3255
3253 ed44      neg     
3255 f5        push    af
3256 cd7035    call    $3570
3259 cd8d2f    call    $2f8d
325c d5        push    de
325d cd4334    call    $3443
3260 03        inc     bc
3261 f432eb    call    p,$eb32
3264 0f        rrca    
3265 73        ld      (hl),e
3266 08        ex      af,af'
3267 b8        cp      b
3268 d5        push    de
3269 52        ld      d,d
326a 7b        ld      a,e
326b 00        nop     
326c 00        nop     
326d 00        nop     
326e 00        nop     
326f 80        add     a,b
3270 e3        ex      (sp),hl
3271 cd4334    call    $3443
3274 02        ld      (bc),a
3275 09        add     hl,bc
3276 60        ld      h,b
3277 de01      sbc     a,$01
3279 78        ld      a,b
327a f8        ret     m

327b 17        rla     
327c 72        ld      (hl),d
327d 317ecd    ld      sp,$cd7e
3280 77        ld      (hl),a
3281 35        dec     (hl)
3282 d1        pop     de
3283 e5        push    hl
3284 eb        ex      de,hl
3285 cd9a34    call    $349a
3288 eb        ex      de,hl
3289 e1        pop     hl
328a cd0436    call    $3604
328d 116b32    ld      de,$326b
3290 cda234    call    $34a2
3293 f1        pop     af
3294 37        scf     
3295 dd8e04    adc     a,(ix+$04)
3298 dd7704    ld      (ix+$04),a
329b 37        scf     
329c c9        ret     

329d 29        add     hl,hl
329e 3b        dec     sp
329f aa        xor     d
32a0 3881      jr      c,$3223          ; (-$7f)
32a2 c7        rst     $00
32a3 33        inc     sp
32a4 0f        rrca    
32a5 3087      jr      nc,$322e         ; (-$79)
32a7 f8        ret     m

32a8 17        rla     
32a9 72        ld      (hl),d
32aa b1        or      c
32ab 87        add     a,a

;; REAL sqr
32ac 116b32    ld      de,$326b

;; REAL power
32af eb        ex      de,hl
32b0 cd2737    call    $3727
32b3 eb        ex      de,hl
32b4 ca7d2f    jp      z,$2f7d
32b7 f5        push    af
32b8 cd2737    call    $3727
32bb 2825      jr      z,$32e2          ; (+$25)
32bd 47        ld      b,a
32be fc3437    call    m,$3734
32c1 e5        push    hl
32c2 cd2433    call    $3324
32c5 e1        pop     hl
32c6 3825      jr      c,$32ed          ; (+$25)
32c8 e3        ex      (sp),hl
32c9 e1        pop     hl
32ca faea32    jp      m,$32ea
32cd c5        push    bc
32ce d5        push    de
32cf cdb631    call    $31b6
32d2 d1        pop     de
32d3 dc7735    call    c,$3577
32d6 dc2f32    call    c,$322f
32d9 c1        pop     bc
32da d0        ret     nc

32db 78        ld      a,b
32dc b7        or      a
32dd fc3137    call    m,$3731
32e0 37        scf     
32e1 c9        ret     

32e2 f1        pop     af
32e3 37        scf     
32e4 f0        ret     p

32e5 cde837    call    $37e8
32e8 af        xor     a
32e9 c9        ret     

32ea af        xor     a
32eb 3c        inc     a
32ec c9        ret     

32ed 4f        ld      c,a
32ee f1        pop     af
32ef c5        push    bc
32f0 f5        push    af
32f1 79        ld      a,c
32f2 37        scf     
32f3 8f        adc     a,a
32f4 30fd      jr      nc,$32f3         ; (-$03)
32f6 47        ld      b,a
32f7 cd8d2f    call    $2f8d
32fa eb        ex      de,hl
32fb 78        ld      a,b
32fc 87        add     a,a
32fd 2815      jr      z,$3314          ; (+$15)
32ff f5        push    af
3300 cd7035    call    $3570
3303 3016      jr      nc,$331b         ; (+$16)
3305 f1        pop     af
3306 30f4      jr      nc,$32fc         ; (-$0c)
3308 f5        push    af
3309 1104b1    ld      de,$b104
330c cd7735    call    $3577
330f 300a      jr      nc,$331b         ; (+$0a)
3311 f1        pop     af
3312 18e8      jr      $32fc            ; (-$18)
3314 f1        pop     af
3315 37        scf     
3316 fcfb35    call    m,$35fb
3319 18be      jr      $32d9            ; (-$42)
331b f1        pop     af
331c f1        pop     af
331d c1        pop     bc
331e fae237    jp      m,$37e2
3321 c3ea37    jp      $37ea
3324 c5        push    bc
3325 cd882f    call    $2f88
3328 cd1430    call    $3014
332b 79        ld      a,c
332c c1        pop     bc
332d 3002      jr      nc,$3331         ; (+$02)
332f 2803      jr      z,$3334          ; (+$03)
3331 78        ld      a,b
3332 b7        or      a
3333 c9        ret     

3334 4f        ld      c,a
3335 7e        ld      a,(hl)
3336 1f        rra     
3337 9f        sbc     a,a
3338 a0        and     b
3339 47        ld      b,a
333a 79        ld      a,c
333b fe02      cp      $02
333d 9f        sbc     a,a
333e d0        ret     nc

333f 7e        ld      a,(hl)
3340 fe27      cp      $27
3342 d8        ret     c

3343 af        xor     a
3344 c9        ret     

3345 3213b1    ld      ($b113),a
3348 c9        ret     

;; REAL cosine
3349 cd2737    call    $3727
334c fc3437    call    m,$3734
334f f601      or      $01
3351 1801      jr      $3354            ; (+$01)

;; REAL sin
3353 af        xor     a
3354 f5        push    af
3355 11b433    ld      de,$33b4
3358 06f0      ld      b,$f0
335a 3a13b1    ld      a,($b113)
335d b7        or      a
335e 2805      jr      z,$3365          ; (+$05)
3360 11b933    ld      de,$33b9
3363 06f6      ld      b,$f6
3365 cd9234    call    $3492
3368 303a      jr      nc,$33a4         ; (+$3a)
336a f1        pop     af
336b cd6a34    call    $346a
336e d0        ret     nc

336f 7b        ld      a,e
3370 1f        rra     
3371 dc3437    call    c,$3734
3374 06e8      ld      b,$e8
3376 cd9234    call    $3492
3379 d2e237    jp      nc,$37e2
337c dd3404    inc     (ix+$04)
337f cd4034    call    $3440
3382 061b      ld      b,$1b
3384 2d        dec     l
3385 1a        ld      a,(de)
3386 e66e      and     $6e
3388 f8        ret     m

3389 fb        ei      
338a 07        rlca    
338b 2874      jr      z,$3401          ; (+$74)
338d 018968    ld      bc,$6889
3390 99        sbc     a,c
3391 79        ld      a,c
3392 e1        pop     hl
3393 df        rst     $18
3394 35        dec     (hl)
3395 23        inc     hl
3396 7d        ld      a,l
3397 28e7      jr      z,$3380          ; (-$19)
3399 5d        ld      e,l
339a a5        and     l
339b 80        add     a,b
339c a2        and     d
339d da0f49    jp      c,$490f
33a0 81        add     a,c
33a1 c37735    jp      $3577
33a4 f1        pop     af
33a5 c27d2f    jp      nz,$2f7d
33a8 3a13b1    ld      a,($b113)
33ab fe01      cp      $01
33ad d8        ret     c

33ae 11be33    ld      de,$33be
33b1 c37735    jp      $3577
33b4 6e        ld      l,(hl)
33b5 83        add     a,e
33b6 f9        ld      sp,hl
33b7 227fb6    ld      ($b67f),hl
33ba 60        ld      h,b
33bb 0b        dec     bc
33bc 3679      ld      (hl),$79
33be 13        inc     de
33bf 35        dec     (hl)
33c0 fa0e7b    jp      m,$7b0e
33c3 d3e0      out     ($e0),a
33c5 2e65      ld      l,$65
33c7 86        add     a,(hl)

;; REAL tan
33c8 cd8d2f    call    $2f8d
33cb d5        push    de
33cc cd4933    call    $3349
33cf e3        ex      (sp),hl
33d0 dc5333    call    c,$3353
33d3 d1        pop     de
33d4 da0436    jp      c,$3604
33d7 c9        ret     

;; REAL arctan
33d8 cd2737    call    $3727
33db f5        push    af
33dc fc3437    call    m,$3734
33df 06f0      ld      b,$f0
33e1 cd9234    call    $3492
33e4 304a      jr      nc,$3430         ; (+$4a)
33e6 3d        dec     a
33e7 f5        push    af
33e8 f4fb35    call    p,$35fb
33eb cd4034    call    $3440
33ee 0b        dec     bc
33ef ff        rst     $38
33f0 c1        pop     bc
33f1 03        inc     bc
33f2 0f        rrca    
33f3 77        ld      (hl),a
33f4 83        add     a,e
33f5 fce8eb    call    m,$ebe8
33f8 79        ld      a,c
33f9 6f        ld      l,a
33fa ca7836    jp      z,$3678
33fd 7b        ld      a,e
33fe d5        push    de
33ff 3eb0      ld      a,$b0
3401 b5        or      l
3402 7c        ld      a,h
3403 b0        or      b
3404 c1        pop     bc
3405 8b        adc     a,e
3406 09        add     hl,bc
3407 7d        ld      a,l
3408 af        xor     a
3409 e8        ret     pe

340a 32b47d    ld      ($7db4),a
340d 74        ld      (hl),h
340e 6c        ld      l,h
340f 65        ld      h,l
3410 62        ld      h,d
3411 7d        ld      a,l
3412 d1        pop     de
3413 f5        push    af
3414 37        scf     
3415 92        sub     d
3416 7e        ld      a,(hl)
3417 7a        ld      a,d
3418 c3cb4c    jp      $4ccb
341b 7e        ld      a,(hl)
341c 83        add     a,e
341d a7        and     a
341e aa        xor     d
341f aa        xor     d
3420 7f        ld      a,a
3421 feff      cp      $ff
3423 ff        rst     $38
3424 7f        ld      a,a
3425 80        add     a,b
3426 cd7735    call    $3577
3429 f1        pop     af
342a 119c33    ld      de,$339c
342d f49e34    call    p,$349e
3430 3a13b1    ld      a,($b113)
3433 b7        or      a
3434 11c333    ld      de,$33c3
3437 c47735    call    nz,$3577
343a f1        pop     af
343b fc3437    call    m,$3734
343e 37        scf     
343f c9        ret     

3440 cd7035    call    $3570
3443 cd872f    call    $2f87
3446 e1        pop     hl
3447 46        ld      b,(hl)
3448 23        inc     hl
3449 cd902f    call    $2f90
344c 13        inc     de
344d 13        inc     de
344e 13        inc     de
344f 13        inc     de
3450 13        inc     de
3451 d5        push    de
3452 1109b1    ld      de,$b109
3455 05        dec     b
3456 c8        ret     z

3457 c5        push    bc
3458 110eb1    ld      de,$b10e
345b cd7735    call    $3577
345e c1        pop     bc
345f d1        pop     de
3460 d5        push    de
3461 c5        push    bc
3462 cda234    call    $34a2
3465 c1        pop     bc
3466 d1        pop     de
3467 18e3      jr      $344c            ; (-$1d)
3469 af        xor     a
346a f5        push    af
346b cd7735    call    $3577
346e f1        pop     af
346f 116b32    ld      de,$326b
3472 c4a234    call    nz,$34a2
3475 e5        push    hl
3476 cdd92f    call    $2fd9
3479 3013      jr      nc,$348e         ; (+$13)
347b d1        pop     de
347c e5        push    hl
347d f5        push    af
347e d5        push    de
347f 1109b1    ld      de,$b109
3482 cd9f2f    call    $2f9f
3485 eb        ex      de,hl
3486 e1        pop     hl
3487 cd9a34    call    $349a
348a f1        pop     af
348b d1        pop     de
348c 37        scf     
348d c9        ret     

348e e1        pop     hl
348f af        xor     a
3490 3c        inc     a
3491 c9        ret     

3492 cdd336    call    $36d3
3495 f0        ret     p

3496 b8        cp      b
3497 c8        ret     z

3498 3f        ccf     
3499 c9        ret     

349a 3e01      ld      a,$01
349c 1805      jr      $34a3            ; (+$05)

;; REAL reverse subtract
349e 3e80      ld      a,$80
34a0 1801      jr      $34a3            ; (+$01)

;; REAL addition
34a2 af        xor     a
34a3 e5        push    hl
34a4 dde1      pop     ix
34a6 d5        push    de
34a7 fde1      pop     iy
34a9 dd4603    ld      b,(ix+$03)
34ac fd4e03    ld      c,(iy+$03)
34af b7        or      a
34b0 280a      jr      z,$34bc          ; (+$0a)
34b2 faba34    jp      m,$34ba
34b5 0f        rrca    
34b6 a9        xor     c
34b7 4f        ld      c,a
34b8 1802      jr      $34bc            ; (+$02)
34ba a8        xor     b
34bb 47        ld      b,a
34bc dd7e04    ld      a,(ix+$04)
34bf fdbe04    cp      (iy+$04)
34c2 3014      jr      nc,$34d8         ; (+$14)
34c4 50        ld      d,b
34c5 41        ld      b,c
34c6 4a        ld      c,d
34c7 b7        or      a
34c8 57        ld      d,a
34c9 fd7e04    ld      a,(iy+$04)
34cc dd7704    ld      (ix+$04),a
34cf 2854      jr      z,$3525          ; (+$54)
34d1 92        sub     d
34d2 fe21      cp      $21
34d4 304f      jr      nc,$3525         ; (+$4f)
34d6 1811      jr      $34e9            ; (+$11)
34d8 af        xor     a
34d9 fd9604    sub     (iy+$04)
34dc 2859      jr      z,$3537          ; (+$59)
34de dd8604    add     a,(ix+$04)
34e1 fe21      cp      $21
34e3 3052      jr      nc,$3537         ; (+$52)
34e5 e5        push    hl
34e6 fde1      pop     iy
34e8 eb        ex      de,hl
34e9 5f        ld      e,a
34ea 78        ld      a,b
34eb a9        xor     c
34ec f5        push    af
34ed c5        push    bc
34ee 7b        ld      a,e
34ef cd4337    call    $3743
34f2 79        ld      a,c
34f3 c1        pop     bc
34f4 4f        ld      c,a
34f5 f1        pop     af
34f6 fa3c35    jp      m,$353c
34f9 fd7e00    ld      a,(iy+$00)
34fc 85        add     a,l
34fd 6f        ld      l,a
34fe fd7e01    ld      a,(iy+$01)
3501 8c        adc     a,h
3502 67        ld      h,a
3503 fd7e02    ld      a,(iy+$02)
3506 8b        adc     a,e
3507 5f        ld      e,a
3508 fd7e03    ld      a,(iy+$03)
350b cbff      set     7,a
350d 8a        adc     a,d
350e 57        ld      d,a
350f d2b737    jp      nc,$37b7
3512 cb1a      rr      d
3514 cb1b      rr      e
3516 cb1c      rr      h
3518 cb1d      rr      l
351a cb19      rr      c
351c dd3404    inc     (ix+$04)
351f c2b737    jp      nz,$37b7
3522 c3ea37    jp      $37ea
3525 fd7e02    ld      a,(iy+$02)
3528 dd7702    ld      (ix+$02),a
352b fd7e01    ld      a,(iy+$01)
352e dd7701    ld      (ix+$01),a
3531 fd7e00    ld      a,(iy+$00)
3534 dd7700    ld      (ix+$00),a
3537 dd7003    ld      (ix+$03),b
353a 37        scf     
353b c9        ret     

353c af        xor     a
353d 91        sub     c
353e 4f        ld      c,a
353f fd7e00    ld      a,(iy+$00)
3542 9d        sbc     a,l
3543 6f        ld      l,a
3544 fd7e01    ld      a,(iy+$01)
3547 9c        sbc     a,h
3548 67        ld      h,a
3549 fd7e02    ld      a,(iy+$02)
354c 9b        sbc     a,e
354d 5f        ld      e,a
354e fd7e03    ld      a,(iy+$03)
3551 cbff      set     7,a
3553 9a        sbc     a,d
3554 57        ld      d,a
3555 3016      jr      nc,$356d         ; (+$16)
3557 78        ld      a,b
3558 2f        cpl     
3559 47        ld      b,a
355a af        xor     a
355b 91        sub     c
355c 4f        ld      c,a
355d 3e00      ld      a,$00
355f 9d        sbc     a,l
3560 6f        ld      l,a
3561 3e00      ld      a,$00
3563 9c        sbc     a,h
3564 67        ld      h,a
3565 3e00      ld      a,$00
3567 9b        sbc     a,e
3568 5f        ld      e,a
3569 3e00      ld      a,$00
356b 9a        sbc     a,d
356c 57        ld      d,a
356d c3ac37    jp      $37ac
3570 1109b1    ld      de,$b109
3573 cd902f    call    $2f90
3576 eb        ex      de,hl

;; REAL multiplication
3577 d5        push    de
3578 fde1      pop     iy
357a e5        push    hl
357b dde1      pop     ix
357d fd7e04    ld      a,(iy+$04)
3580 b7        or      a
3581 282a      jr      z,$35ad          ; (+$2a)
3583 3d        dec     a
3584 cdaf36    call    $36af
3587 2824      jr      z,$35ad          ; (+$24)
3589 301f      jr      nc,$35aa         ; (+$1f)
358b f5        push    af
358c c5        push    bc
358d cdb035    call    $35b0
3590 79        ld      a,c
3591 c1        pop     bc
3592 4f        ld      c,a
3593 f1        pop     af
3594 cb7a      bit     7,d
3596 200b      jr      nz,$35a3         ; (+$0b)
3598 3d        dec     a
3599 2812      jr      z,$35ad          ; (+$12)
359b cb21      sla     c
359d ed6a      adc     hl,hl
359f cb13      rl      e
35a1 cb12      rl      d
35a3 dd7704    ld      (ix+$04),a
35a6 b7        or      a
35a7 c2b737    jp      nz,$37b7
35aa c3ea37    jp      $37ea
35ad c3e237    jp      $37e2
35b0 210000    ld      hl,$0000
35b3 5d        ld      e,l
35b4 54        ld      d,h
35b5 fd7e00    ld      a,(iy+$00)
35b8 cdf335    call    $35f3
35bb fd7e01    ld      a,(iy+$01)
35be cdf335    call    $35f3
35c1 fd7e02    ld      a,(iy+$02)
35c4 cdf335    call    $35f3
35c7 fd7e03    ld      a,(iy+$03)
35ca f680      or      $80
35cc 0608      ld      b,$08
35ce 1f        rra     
35cf 4f        ld      c,a
35d0 3014      jr      nc,$35e6         ; (+$14)
35d2 7d        ld      a,l
35d3 dd8600    add     a,(ix+$00)
35d6 6f        ld      l,a
35d7 7c        ld      a,h
35d8 dd8e01    adc     a,(ix+$01)
35db 67        ld      h,a
35dc 7b        ld      a,e
35dd dd8e02    adc     a,(ix+$02)
35e0 5f        ld      e,a
35e1 7a        ld      a,d
35e2 dd8e03    adc     a,(ix+$03)
35e5 57        ld      d,a
35e6 cb1a      rr      d
35e8 cb1b      rr      e
35ea cb1c      rr      h
35ec cb1d      rr      l
35ee cb19      rr      c
35f0 10de      djnz    $35d0            ; (-$22)
35f2 c9        ret     

35f3 b7        or      a
35f4 20d6      jr      nz,$35cc         ; (-$2a)
35f6 6c        ld      l,h
35f7 63        ld      h,e
35f8 5a        ld      e,d
35f9 57        ld      d,a
35fa c9        ret     

35fb cd872f    call    $2f87
35fe eb        ex      de,hl
35ff d5        push    de
3600 cd7d2f    call    $2f7d
3603 d1        pop     de
3604 d5        push    de
3605 fde1      pop     iy
3607 e5        push    hl
3608 dde1      pop     ix
360a af        xor     a
360b fd9604    sub     (iy+$04)
360e 285a      jr      z,$366a          ; (+$5a)
3610 cdaf36    call    $36af
3613 cae237    jp      z,$37e2
3616 304f      jr      nc,$3667         ; (+$4f)
3618 c5        push    bc
3619 4f        ld      c,a
361a 5e        ld      e,(hl)
361b 23        inc     hl
361c 56        ld      d,(hl)
361d 23        inc     hl
361e 7e        ld      a,(hl)
361f 23        inc     hl
3620 66        ld      h,(hl)
3621 6f        ld      l,a
3622 eb        ex      de,hl
3623 fd4603    ld      b,(iy+$03)
3626 cbf8      set     7,b
3628 cd9d36    call    $369d
362b 3806      jr      c,$3633          ; (+$06)
362d 79        ld      a,c
362e b7        or      a
362f 2008      jr      nz,$3639         ; (+$08)
3631 1833      jr      $3666            ; (+$33)
3633 0d        dec     c
3634 29        add     hl,hl
3635 cb13      rl      e
3637 cb12      rl      d
3639 dd7104    ld      (ix+$04),c
363c cd7236    call    $3672
363f dd7103    ld      (ix+$03),c
3642 cd7236    call    $3672
3645 dd7102    ld      (ix+$02),c
3648 cd7236    call    $3672
364b dd7101    ld      (ix+$01),c
364e cd7236    call    $3672
3651 3f        ccf     
3652 dc9d36    call    c,$369d
3655 3f        ccf     
3656 9f        sbc     a,a
3657 69        ld      l,c
3658 dd6601    ld      h,(ix+$01)
365b dd5e02    ld      e,(ix+$02)
365e dd5603    ld      d,(ix+$03)
3661 c1        pop     bc
3662 4f        ld      c,a
3663 c3b737    jp      $37b7
3666 c1        pop     bc
3667 c3ea37    jp      $37ea
366a dd4603    ld      b,(ix+$03)
366d cdea37    call    $37ea
3670 af        xor     a
3671 c9        ret     

3672 0e01      ld      c,$01
3674 3808      jr      c,$367e          ; (+$08)
3676 7a        ld      a,d
3677 b8        cp      b
3678 cca036    call    z,$36a0
367b 3f        ccf     
367c 3013      jr      nc,$3691         ; (+$13)
367e 7d        ld      a,l
367f fd9600    sub     (iy+$00)
3682 6f        ld      l,a
3683 7c        ld      a,h
3684 fd9e01    sbc     a,(iy+$01)
3687 67        ld      h,a
3688 7b        ld      a,e
3689 fd9e02    sbc     a,(iy+$02)
368c 5f        ld      e,a
368d 7a        ld      a,d
368e 98        sbc     a,b
368f 57        ld      d,a
3690 37        scf     
3691 cb11      rl      c
3693 9f        sbc     a,a
3694 29        add     hl,hl
3695 cb13      rl      e
3697 cb12      rl      d
3699 3c        inc     a
369a 20d8      jr      nz,$3674         ; (-$28)
369c c9        ret     

369d 7a        ld      a,d
369e b8        cp      b
369f c0        ret     nz

36a0 7b        ld      a,e
36a1 fdbe02    cp      (iy+$02)
36a4 c0        ret     nz

36a5 7c        ld      a,h
36a6 fdbe01    cp      (iy+$01)
36a9 c0        ret     nz

36aa 7d        ld      a,l
36ab fdbe00    cp      (iy+$00)
36ae c9        ret     

36af 4f        ld      c,a
36b0 dd7e03    ld      a,(ix+$03)
36b3 fdae03    xor     (iy+$03)
36b6 47        ld      b,a
36b7 dd7e04    ld      a,(ix+$04)
36ba b7        or      a
36bb c8        ret     z

36bc 81        add     a,c
36bd 4f        ld      c,a
36be 1f        rra     
36bf a9        xor     c
36c0 79        ld      a,c
36c1 f2cf36    jp      p,$36cf
36c4 ddcb03fe  set     7,(ix+$03)
36c8 d67f      sub     $7f
36ca 37        scf     
36cb c0        ret     nz

36cc fe01      cp      $01
36ce c9        ret     

36cf b7        or      a
36d0 f8        ret     m

36d1 af        xor     a
36d2 c9        ret     

36d3 e5        push    hl
36d4 dde1      pop     ix
36d6 dd7e04    ld      a,(ix+$04)
36d9 b7        or      a
36da c8        ret     z

36db d680      sub     $80
36dd 37        scf     
36de c9        ret     

36df e5        push    hl
36e0 dde1      pop     ix
36e2 d5        push    de
36e3 fde1      pop     iy
36e5 dd7e04    ld      a,(ix+$04)
36e8 fdbe04    cp      (iy+$04)
36eb 382c      jr      c,$3719          ; (+$2c)
36ed 2033      jr      nz,$3722         ; (+$33)
36ef b7        or      a
36f0 c8        ret     z

36f1 dd7e03    ld      a,(ix+$03)
36f4 fdae03    xor     (iy+$03)
36f7 fa2237    jp      m,$3722
36fa dd7e03    ld      a,(ix+$03)
36fd fd9603    sub     (iy+$03)
3700 2017      jr      nz,$3719         ; (+$17)
3702 dd7e02    ld      a,(ix+$02)
3705 fd9602    sub     (iy+$02)
3708 200f      jr      nz,$3719         ; (+$0f)
370a dd7e01    ld      a,(ix+$01)
370d fd9601    sub     (iy+$01)
3710 2007      jr      nz,$3719         ; (+$07)
3712 dd7e00    ld      a,(ix+$00)
3715 fd9600    sub     (iy+$00)
3718 c8        ret     z

3719 9f        sbc     a,a
371a fdae03    xor     (iy+$03)
371d 87        add     a,a
371e 9f        sbc     a,a
371f d8        ret     c

3720 3c        inc     a
3721 c9        ret     

3722 dd7e03    ld      a,(ix+$03)
3725 18f6      jr      $371d            ; (-$0a)
3727 e5        push    hl
3728 dde1      pop     ix
372a dd7e04    ld      a,(ix+$04)
372d b7        or      a
372e c8        ret     z

372f 18f1      jr      $3722            ; (-$0f)
3731 e5        push    hl
3732 dde1      pop     ix
3734 dd7e03    ld      a,(ix+$03)
3737 ee80      xor     $80
3739 dd7703    ld      (ix+$03),a
373c c9        ret     

373d fe21      cp      $21
373f 3802      jr      c,$3743          ; (+$02)
3741 3e21      ld      a,$21
3743 5e        ld      e,(hl)
3744 23        inc     hl
3745 56        ld      d,(hl)
3746 23        inc     hl
3747 4e        ld      c,(hl)
3748 23        inc     hl
3749 66        ld      h,(hl)
374a 69        ld      l,c
374b eb        ex      de,hl
374c cbfa      set     7,d
374e 010000    ld      bc,$0000
3751 180b      jr      $375e            ; (+$0b)
3753 4f        ld      c,a
3754 78        ld      a,b
3755 b5        or      l
3756 47        ld      b,a
3757 79        ld      a,c
3758 4d        ld      c,l
3759 6c        ld      l,h
375a 63        ld      h,e
375b 5a        ld      e,d
375c 1600      ld      d,$00
375e d608      sub     $08
3760 30f1      jr      nc,$3753         ; (-$0f)
3762 c608      add     a,$08
3764 c8        ret     z

3765 cb3a      srl     d
3767 cb1b      rr      e
3769 cb1c      rr      h
376b cb1d      rr      l
376d cb19      rr      c
376f 3d        dec     a
3770 20f3      jr      nz,$3765         ; (-$0d)
3772 c9        ret     

3773 2017      jr      nz,$378c         ; (+$17)
3775 57        ld      d,a
3776 7b        ld      a,e
3777 b4        or      h
3778 b5        or      l
3779 b1        or      c
377a c8        ret     z

377b 7a        ld      a,d
377c d608      sub     $08
377e 381a      jr      c,$379a          ; (+$1a)
3780 c8        ret     z

3781 53        ld      d,e
3782 5c        ld      e,h
3783 65        ld      h,l
3784 69        ld      l,c
3785 0e00      ld      c,$00
3787 14        inc     d
3788 15        dec     d
3789 28f1      jr      z,$377c          ; (-$0f)
378b f8        ret     m

378c 3d        dec     a
378d c8        ret     z

378e cb21      sla     c
3790 ed6a      adc     hl,hl
3792 cb13      rl      e
3794 cb12      rl      d
3796 f28c37    jp      p,$378c
3799 c9        ret     

379a af        xor     a
379b c9        ret     

379c e5        push    hl
379d dde1      pop     ix
379f dd7004    ld      (ix+$04),b
37a2 47        ld      b,a
37a3 5e        ld      e,(hl)
37a4 23        inc     hl
37a5 56        ld      d,(hl)
37a6 23        inc     hl
37a7 7e        ld      a,(hl)
37a8 23        inc     hl
37a9 66        ld      h,(hl)
37aa 6f        ld      l,a
37ab eb        ex      de,hl
37ac dd7e04    ld      a,(ix+$04)
37af 15        dec     d
37b0 14        inc     d
37b1 f47337    call    p,$3773
37b4 dd7704    ld      (ix+$04),a
37b7 cb21      sla     c
37b9 3012      jr      nc,$37cd         ; (+$12)
37bb 2c        inc     l
37bc 200f      jr      nz,$37cd         ; (+$0f)
37be 24        inc     h
37bf 200c      jr      nz,$37cd         ; (+$0c)
37c1 13        inc     de
37c2 7a        ld      a,d
37c3 b3        or      e
37c4 2007      jr      nz,$37cd         ; (+$07)
37c6 dd3404    inc     (ix+$04)
37c9 281f      jr      z,$37ea          ; (+$1f)
37cb 1680      ld      d,$80
37cd 78        ld      a,b
37ce f67f      or      $7f
37d0 a2        and     d
37d1 dd7703    ld      (ix+$03),a
37d4 dd7302    ld      (ix+$02),e
37d7 dd7401    ld      (ix+$01),h
37da dd7500    ld      (ix+$00),l
37dd dde5      push    ix
37df e1        pop     hl
37e0 37        scf     
37e1 c9        ret     

37e2 af        xor     a
37e3 dd7704    ld      (ix+$04),a
37e6 18f5      jr      $37dd            ; (-$0b)
37e8 0600      ld      b,$00
37ea dde5      push    ix
37ec e1        pop     hl
37ed 78        ld      a,b
37ee f67f      or      $7f
37f0 dd7703    ld      (ix+$03),a
37f3 f6ff      or      $ff
37f5 dd7704    ld      (ix+$04),a
37f8 77        ld      (hl),a
37f9 dd7701    ld      (ix+$01),a
37fc dd7702    ld      (ix+$02),a
37ff c9        ret     

;; font graphics
3800 
--------------------------
;; character 0
;;
defb %11111111
defb %11000011
defb %11000011
defb %11000011
defb %11000011
defb %11000011
defb %11000011
defb %11111111
--------------------------
;; character 1
;;
defb %11111111
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
--------------------------
;; character 2
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %11111111
--------------------------
;; character 3
;;
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %11111111
--------------------------
;; character 4
;;
defb %00001100
defb %00011000
defb %00110000
defb %01111110
defb %00001100
defb %00011000
defb %00110000
defb %00000000
--------------------------
;; character 5
;;
defb %11111111
defb %11000011
defb %11100111
defb %11011011
defb %11011011
defb %11100111
defb %11000011
defb %11111111
--------------------------
;; character 6
;;
defb %00000000
defb %00000001
defb %00000011
defb %00000110
defb %11001100
defb %01111000
defb %00110000
defb %00000000
--------------------------
;; character 7
;;
defb %00111100
defb %01100110
defb %11000011
defb %11000011
defb %11111111
defb %00100100
defb %11100111
defb %00000000
--------------------------
;; character 8
;;
defb %00000000
defb %00000000
defb %00110000
defb %01100000
defb %11111111
defb %01100000
defb %00110000
defb %00000000
--------------------------
;; character 9
;;
defb %00000000
defb %00000000
defb %00001100
defb %00000110
defb %11111111
defb %00000110
defb %00001100
defb %00000000
--------------------------
;; character 10
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %11011011
defb %01111110
defb %00111100
defb %00011000
--------------------------
;; character 11
;;
defb %00011000
defb %00111100
defb %01111110
defb %11011011
defb %00011000
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 12
;;
defb %00011000
defb %01011010
defb %00111100
defb %10011001
defb %11011011
defb %01111110
defb %00111100
defb %00011000
--------------------------
;; character 13
;;
defb %00000000
defb %00000011
defb %00110011
defb %01100011
defb %11111110
defb %01100000
defb %00110000
defb %00000000
--------------------------
;; character 14
;;
defb %00111100
defb %01100110
defb %11111111
defb %11011011
defb %11011011
defb %11111111
defb %01100110
defb %00111100
--------------------------
;; character 15
;;
defb %00111100
defb %01100110
defb %11000011
defb %11011011
defb %11011011
defb %11000011
defb %01100110
defb %00111100
--------------------------
;; character 16
;;
defb %11111111
defb %11000011
defb %11000011
defb %11111111
defb %11000011
defb %11000011
defb %11000011
defb %11111111
--------------------------
;; character 17
;;
defb %00111100
defb %01111110
defb %11011011
defb %11011011
defb %11011111
defb %11000011
defb %01100110
defb %00111100
--------------------------
;; character 18
;;
defb %00111100
defb %01100110
defb %11000011
defb %11011111
defb %11011011
defb %11011011
defb %01111110
defb %00111100
--------------------------
;; character 19
;;
defb %00111100
defb %01100110
defb %11000011
defb %11111011
defb %11011011
defb %11011011
defb %01111110
defb %00111100
--------------------------
;; character 20
;;
defb %00111100
defb %01111110
defb %11011011
defb %11011011
defb %11111011
defb %11000011
defb %01100110
defb %00111100
--------------------------
;; character 21
;;
defb %00000000
defb %00000001
defb %00110011
defb %00011110
defb %11001110
defb %01111011
defb %00110001
defb %00000000
--------------------------
;; character 22
;;
defb %01111110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %11100111
--------------------------
;; character 23
;;
defb %00000011
defb %00000011
defb %00000011
defb %11111111
defb %00000011
defb %00000011
defb %00000011
defb %00000000
--------------------------
;; character 24
;;
defb %11111111
defb %01100110
defb %00111100
defb %00011000
defb %00011000
defb %00111100
defb %01100110
defb %11111111
--------------------------
;; character 25
;;
defb %00011000
defb %00011000
defb %00111100
defb %00111100
defb %00111100
defb %00111100
defb %00011000
defb %00011000
--------------------------
;; character 26
;;
defb %00111100
defb %01100110
defb %01100110
defb %00110000
defb %00011000
defb %00000000
defb %00011000
defb %00000000
--------------------------
;; character 27
;;
defb %00111100
defb %01100110
defb %11000011
defb %11111111
defb %11000011
defb %11000011
defb %01100110
defb %00111100
--------------------------
;; character 28
;;
defb %11111111
defb %11011011
defb %11011011
defb %11011011
defb %11111011
defb %11000011
defb %11000011
defb %11111111
--------------------------
;; character 29
;;
defb %11111111
defb %11000011
defb %11000011
defb %11111011
defb %11011011
defb %11011011
defb %11011011
defb %11111111
--------------------------
;; character 30
;;
defb %11111111
defb %11000011
defb %11000011
defb %11011111
defb %11011011
defb %11011011
defb %11011011
defb %11111111
--------------------------
;; character 31
;;
defb %11111111
defb %11011011
defb %11011011
defb %11011011
defb %11011111
defb %11000011
defb %11000011
defb %11111111
--------------------------
;; character 32
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 33
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00000000
defb %00011000
defb %00000000
--------------------------
;; character 34
;;
defb %01101100
defb %01101100
defb %01101100
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 35
;;
defb %01101100
defb %01101100
defb %11111110
defb %01101100
defb %11111110
defb %01101100
defb %01101100
defb %00000000
--------------------------
;; character 36
;;
defb %00011000
defb %00111110
defb %01011000
defb %00111100
defb %00011010
defb %01111100
defb %00011000
defb %00000000
--------------------------
;; character 37
;;
defb %00000000
defb %11000110
defb %11001100
defb %00011000
defb %00110000
defb %01100110
defb %11000110
defb %00000000
--------------------------
;; character 38
;;
defb %00111000
defb %01101100
defb %00111000
defb %01110110
defb %11011100
defb %11001100
defb %01110110
defb %00000000
--------------------------
;; character 39
;;
defb %00011000
defb %00011000
defb %00110000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 40
;;
defb %00001100
defb %00011000
defb %00110000
defb %00110000
defb %00110000
defb %00011000
defb %00001100
defb %00000000
--------------------------
;; character 41
;;
defb %00110000
defb %00011000
defb %00001100
defb %00001100
defb %00001100
defb %00011000
defb %00110000
defb %00000000
--------------------------
;; character 42
;;
defb %00000000
defb %01100110
defb %00111100
defb %11111111
defb %00111100
defb %01100110
defb %00000000
defb %00000000
--------------------------
;; character 43
;;
defb %00000000
defb %00011000
defb %00011000
defb %01111110
defb %00011000
defb %00011000
defb %00000000
defb %00000000
--------------------------
;; character 44
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00011000
defb %00011000
defb %00110000
--------------------------
;; character 45
;;
defb %00000000
defb %00000000
defb %00000000
defb %01111110
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 46
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 47
;;
defb %00000110
defb %00001100
defb %00011000
defb %00110000
defb %01100000
defb %11000000
defb %10000000
defb %00000000
--------------------------
;; character 48
;;
defb %01111100
defb %11000110
defb %11001110
defb %11010110
defb %11100110
defb %11000110
defb %01111100
defb %00000000
--------------------------
;; character 49
;;
defb %00011000
defb %00111000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %01111110
defb %00000000
--------------------------
;; character 50
;;
defb %00111100
defb %01100110
defb %00000110
defb %00111100
defb %01100000
defb %01100110
defb %01111110
defb %00000000
--------------------------
;; character 51
;;
defb %00111100
defb %01100110
defb %00000110
defb %00011100
defb %00000110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 52
;;
defb %00011100
defb %00111100
defb %01101100
defb %11001100
defb %11111110
defb %00001100
defb %00011110
defb %00000000
--------------------------
;; character 53
;;
defb %01111110
defb %01100010
defb %01100000
defb %01111100
defb %00000110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 54
;;
defb %00111100
defb %01100110
defb %01100000
defb %01111100
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 55
;;
defb %01111110
defb %01100110
defb %00000110
defb %00001100
defb %00011000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 56
;;
defb %00111100
defb %01100110
defb %01100110
defb %00111100
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 57
;;
defb %00111100
defb %01100110
defb %01100110
defb %00111110
defb %00000110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 58
;;
defb %00000000
defb %00000000
defb %00011000
defb %00011000
defb %00000000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 59
;;
defb %00000000
defb %00000000
defb %00011000
defb %00011000
defb %00000000
defb %00011000
defb %00011000
defb %00110000
--------------------------
;; character 60
;;
defb %00001100
defb %00011000
defb %00110000
defb %01100000
defb %00110000
defb %00011000
defb %00001100
defb %00000000
--------------------------
;; character 61
;;
defb %00000000
defb %00000000
defb %01111110
defb %00000000
defb %00000000
defb %01111110
defb %00000000
defb %00000000
--------------------------
;; character 62
;;
defb %01100000
defb %00110000
defb %00011000
defb %00001100
defb %00011000
defb %00110000
defb %01100000
defb %00000000
--------------------------
;; character 63
;;
defb %00111100
defb %01100110
defb %01100110
defb %00001100
defb %00011000
defb %00000000
defb %00011000
defb %00000000
--------------------------
;; character 64
;;
defb %01111100
defb %11000110
defb %11011110
defb %11011110
defb %11011110
defb %11000000
defb %01111100
defb %00000000
--------------------------
;; character 65
;;
defb %00011000
defb %00111100
defb %01100110
defb %01100110
defb %01111110
defb %01100110
defb %01100110
defb %00000000
--------------------------
;; character 66
;;
defb %11111100
defb %01100110
defb %01100110
defb %01111100
defb %01100110
defb %01100110
defb %11111100
defb %00000000
--------------------------
;; character 67
;;
defb %00111100
defb %01100110
defb %11000000
defb %11000000
defb %11000000
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 68
;;
defb %11111000
defb %01101100
defb %01100110
defb %01100110
defb %01100110
defb %01101100
defb %11111000
defb %00000000
--------------------------
;; character 69
;;
defb %11111110
defb %01100010
defb %01101000
defb %01111000
defb %01101000
defb %01100010
defb %11111110
defb %00000000
--------------------------
;; character 70
;;
defb %11111110
defb %01100010
defb %01101000
defb %01111000
defb %01101000
defb %01100000
defb %11110000
defb %00000000
--------------------------
;; character 71
;;
defb %00111100
defb %01100110
defb %11000000
defb %11000000
defb %11001110
defb %01100110
defb %00111110
defb %00000000
--------------------------
;; character 72
;;
defb %01100110
defb %01100110
defb %01100110
defb %01111110
defb %01100110
defb %01100110
defb %01100110
defb %00000000
--------------------------
;; character 73
;;
defb %01111110
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %01111110
defb %00000000
--------------------------
;; character 74
;;
defb %00011110
defb %00001100
defb %00001100
defb %00001100
defb %11001100
defb %11001100
defb %01111000
defb %00000000
--------------------------
;; character 75
;;
defb %11100110
defb %01100110
defb %01101100
defb %01111000
defb %01101100
defb %01100110
defb %11100110
defb %00000000
--------------------------
;; character 76
;;
defb %11110000
defb %01100000
defb %01100000
defb %01100000
defb %01100010
defb %01100110
defb %11111110
defb %00000000
--------------------------
;; character 77
;;
defb %11000110
defb %11101110
defb %11111110
defb %11111110
defb %11010110
defb %11000110
defb %11000110
defb %00000000
--------------------------
;; character 78
;;
defb %11000110
defb %11100110
defb %11110110
defb %11011110
defb %11001110
defb %11000110
defb %11000110
defb %00000000
--------------------------
;; character 79
;;
defb %00111000
defb %01101100
defb %11000110
defb %11000110
defb %11000110
defb %01101100
defb %00111000
defb %00000000
--------------------------
;; character 80
;;
defb %11111100
defb %01100110
defb %01100110
defb %01111100
defb %01100000
defb %01100000
defb %11110000
defb %00000000
--------------------------
;; character 81
;;
defb %00111000
defb %01101100
defb %11000110
defb %11000110
defb %11011010
defb %11001100
defb %01110110
defb %00000000
--------------------------
;; character 82
;;
defb %11111100
defb %01100110
defb %01100110
defb %01111100
defb %01101100
defb %01100110
defb %11100110
defb %00000000
--------------------------
;; character 83
;;
defb %00111100
defb %01100110
defb %01100000
defb %00111100
defb %00000110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 84
;;
defb %01111110
defb %01011010
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00111100
defb %00000000
--------------------------
;; character 85
;;
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 86
;;
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %00111100
defb %00011000
defb %00000000
--------------------------
;; character 87
;;
defb %11000110
defb %11000110
defb %11000110
defb %11010110
defb %11111110
defb %11101110
defb %11000110
defb %00000000
--------------------------
;; character 88
;;
defb %11000110
defb %01101100
defb %00111000
defb %00111000
defb %01101100
defb %11000110
defb %11000110
defb %00000000
--------------------------
;; character 89
;;
defb %01100110
defb %01100110
defb %01100110
defb %00111100
defb %00011000
defb %00011000
defb %00111100
defb %00000000
--------------------------
;; character 90
;;
defb %11111110
defb %11000110
defb %10001100
defb %00011000
defb %00110010
defb %01100110
defb %11111110
defb %00000000
--------------------------
;; character 91
;;
defb %00111100
defb %00110000
defb %00110000
defb %00110000
defb %00110000
defb %00110000
defb %00111100
defb %00000000
--------------------------
;; character 92
;;
defb %11000000
defb %01100000
defb %00110000
defb %00011000
defb %00001100
defb %00000110
defb %00000010
defb %00000000
--------------------------
;; character 93
;;
defb %00111100
defb %00001100
defb %00001100
defb %00001100
defb %00001100
defb %00001100
defb %00111100
defb %00000000
--------------------------
;; character 94
;;
defb %00011000
defb %00111100
defb %01111110
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 95
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %11111111
--------------------------
;; character 96
;;
defb %00110000
defb %00011000
defb %00001100
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 97
;;
defb %00000000
defb %00000000
defb %01111000
defb %00001100
defb %01111100
defb %11001100
defb %01110110
defb %00000000
--------------------------
;; character 98
;;
defb %11100000
defb %01100000
defb %01111100
defb %01100110
defb %01100110
defb %01100110
defb %11011100
defb %00000000
--------------------------
;; character 99
;;
defb %00000000
defb %00000000
defb %00111100
defb %01100110
defb %01100000
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 100
;;
defb %00011100
defb %00001100
defb %01111100
defb %11001100
defb %11001100
defb %11001100
defb %01110110
defb %00000000
--------------------------
;; character 101
;;
defb %00000000
defb %00000000
defb %00111100
defb %01100110
defb %01111110
defb %01100000
defb %00111100
defb %00000000
--------------------------
;; character 102
;;
defb %00011100
defb %00110110
defb %00110000
defb %01111000
defb %00110000
defb %00110000
defb %01111000
defb %00000000
--------------------------
;; character 103
;;
defb %00000000
defb %00000000
defb %00111110
defb %01100110
defb %01100110
defb %00111110
defb %00000110
defb %01111100
--------------------------
;; character 104
;;
defb %11100000
defb %01100000
defb %01101100
defb %01110110
defb %01100110
defb %01100110
defb %11100110
defb %00000000
--------------------------
;; character 105
;;
defb %00011000
defb %00000000
defb %00111000
defb %00011000
defb %00011000
defb %00011000
defb %00111100
defb %00000000
--------------------------
;; character 106
;;
defb %00000110
defb %00000000
defb %00001110
defb %00000110
defb %00000110
defb %01100110
defb %01100110
defb %00111100
--------------------------
;; character 107
;;
defb %11100000
defb %01100000
defb %01100110
defb %01101100
defb %01111000
defb %01101100
defb %11100110
defb %00000000
--------------------------
;; character 108
;;
defb %00111000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00111100
defb %00000000
--------------------------
;; character 109
;;
defb %00000000
defb %00000000
defb %01101100
defb %11111110
defb %11010110
defb %11010110
defb %11000110
defb %00000000
--------------------------
;; character 110
;;
defb %00000000
defb %00000000
defb %11011100
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %00000000
--------------------------
;; character 111
;;
defb %00000000
defb %00000000
defb %00111100
defb %01100110
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 112
;;
defb %00000000
defb %00000000
defb %11011100
defb %01100110
defb %01100110
defb %01111100
defb %01100000
defb %11110000
--------------------------
;; character 113
;;
defb %00000000
defb %00000000
defb %01110110
defb %11001100
defb %11001100
defb %01111100
defb %00001100
defb %00011110
--------------------------
;; character 114
;;
defb %00000000
defb %00000000
defb %11011100
defb %01110110
defb %01100000
defb %01100000
defb %11110000
defb %00000000
--------------------------
;; character 115
;;
defb %00000000
defb %00000000
defb %00111100
defb %01100000
defb %00111100
defb %00000110
defb %01111100
defb %00000000
--------------------------
;; character 116
;;
defb %00110000
defb %00110000
defb %01111100
defb %00110000
defb %00110000
defb %00110110
defb %00011100
defb %00000000
--------------------------
;; character 117
;;
defb %00000000
defb %00000000
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %00111110
defb %00000000
--------------------------
;; character 118
;;
defb %00000000
defb %00000000
defb %01100110
defb %01100110
defb %01100110
defb %00111100
defb %00011000
defb %00000000
--------------------------
;; character 119
;;
defb %00000000
defb %00000000
defb %11000110
defb %11010110
defb %11010110
defb %11111110
defb %01101100
defb %00000000
--------------------------
;; character 120
;;
defb %00000000
defb %00000000
defb %11000110
defb %01101100
defb %00111000
defb %01101100
defb %11000110
defb %00000000
--------------------------
;; character 121
;;
defb %00000000
defb %00000000
defb %01100110
defb %01100110
defb %01100110
defb %00111110
defb %00000110
defb %01111100
--------------------------
;; character 122
;;
defb %00000000
defb %00000000
defb %01111110
defb %01001100
defb %00011000
defb %00110010
defb %01111110
defb %00000000
--------------------------
;; character 123
;;
defb %00001110
defb %00011000
defb %00011000
defb %01110000
defb %00011000
defb %00011000
defb %00001110
defb %00000000
--------------------------
;; character 124
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 125
;;
defb %01110000
defb %00011000
defb %00011000
defb %00001110
defb %00011000
defb %00011000
defb %01110000
defb %00000000
--------------------------
;; character 126
;;
defb %01110110
defb %11011100
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 127
;;
defb %11001100
defb %00110011
defb %11001100
defb %00110011
defb %11001100
defb %00110011
defb %11001100
defb %00110011
--------------------------
;; character 128
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 129
;;
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 130
;;
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 131
;;
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 132
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
--------------------------
;; character 133
;;
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
--------------------------
;; character 134
;;
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %11110000
defb %11110000
defb %11110000
defb %11110000
--------------------------
;; character 135
;;
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %11110000
defb %11110000
defb %11110000
defb %11110000
--------------------------
;; character 136
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00001111
defb %00001111
defb %00001111
defb %00001111
--------------------------
;; character 137
;;
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %00001111
defb %00001111
defb %00001111
defb %00001111
--------------------------
;; character 138
;;
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %00001111
--------------------------
;; character 139
;;
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %00001111
defb %00001111
defb %00001111
defb %00001111
--------------------------
;; character 140
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %11111111
defb %11111111
defb %11111111
defb %11111111
--------------------------
;; character 141
;;
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11111111
defb %11111111
defb %11111111
defb %11111111
--------------------------
;; character 142
;;
defb %00001111
defb %00001111
defb %00001111
defb %00001111
defb %11111111
defb %11111111
defb %11111111
defb %11111111
--------------------------
;; character 143
;;
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %11111111
defb %11111111
--------------------------
;; character 144
;;
defb %00000000
defb %00000000
defb %00000000
defb %00011000
defb %00011000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 145
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 146
;;
defb %00000000
defb %00000000
defb %00000000
defb %00011111
defb %00011111
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 147
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011111
defb %00001111
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 148
;;
defb %00000000
defb %00000000
defb %00000000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 149
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 150
;;
defb %00000000
defb %00000000
defb %00000000
defb %00001111
defb %00011111
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 151
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011111
defb %00011111
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 152
;;
defb %00000000
defb %00000000
defb %00000000
defb %11111000
defb %11111000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 153
;;
defb %00011000
defb %00011000
defb %00011000
defb %11111000
defb %11110000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 154
;;
defb %00000000
defb %00000000
defb %00000000
defb %11111111
defb %11111111
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 155
;;
defb %00011000
defb %00011000
defb %00011000
defb %11111111
defb %11111111
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 156
;;
defb %00000000
defb %00000000
defb %00000000
defb %11110000
defb %11111000
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 157
;;
defb %00011000
defb %00011000
defb %00011000
defb %11111000
defb %11111000
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 158
;;
defb %00000000
defb %00000000
defb %00000000
defb %11111111
defb %11111111
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 159
;;
defb %00011000
defb %00011000
defb %00011000
defb %11111111
defb %11111111
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 160
;;
defb %00010000
defb %00111000
defb %01101100
defb %11000110
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 161
;;
defb %00001100
defb %00011000
defb %00110000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 162
;;
defb %01100110
defb %01100110
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 163
;;
defb %00111100
defb %01100110
defb %01100000
defb %11111000
defb %01100000
defb %01100110
defb %11111110
defb %00000000
--------------------------
;; character 164
;;
defb %00111000
defb %01000100
defb %10111010
defb %10100010
defb %10111010
defb %01000100
defb %00111000
defb %00000000
--------------------------
;; character 165
;;
defb %01111110
defb %11110100
defb %11110100
defb %01110100
defb %00110100
defb %00110100
defb %00110100
defb %00000000
--------------------------
;; character 166
;;
defb %00011110
defb %00110000
defb %00111000
defb %01101100
defb %00111000
defb %00011000
defb %11110000
defb %00000000
--------------------------
;; character 167
;;
defb %00011000
defb %00011000
defb %00001100
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 168
;;
defb %01000000
defb %11000000
defb %01000100
defb %01001100
defb %01010100
defb %00011110
defb %00000100
defb %00000000
--------------------------
;; character 169
;;
defb %01000000
defb %11000000
defb %01001100
defb %01010010
defb %01000100
defb %00001000
defb %00011110
defb %00000000
--------------------------
;; character 170
;;
defb %11100000
defb %00010000
defb %01100010
defb %00010110
defb %11101010
defb %00001111
defb %00000010
defb %00000000
--------------------------
;; character 171
;;
defb %00000000
defb %00011000
defb %00011000
defb %01111110
defb %00011000
defb %00011000
defb %01111110
defb %00000000
--------------------------
;; character 172
;;
defb %00011000
defb %00011000
defb %00000000
defb %01111110
defb %00000000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 173
;;
defb %00000000
defb %00000000
defb %00000000
defb %01111110
defb %00000110
defb %00000110
defb %00000000
defb %00000000
--------------------------
;; character 174
;;
defb %00011000
defb %00000000
defb %00011000
defb %00110000
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 175
;;
defb %00011000
defb %00000000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %00000000
--------------------------
;; character 176
;;
defb %00000000
defb %00000000
defb %01110011
defb %11011110
defb %11001100
defb %11011110
defb %01110011
defb %00000000
--------------------------
;; character 177
;;
defb %01111100
defb %11000110
defb %11000110
defb %11111100
defb %11000110
defb %11000110
defb %11111000
defb %11000000
--------------------------
;; character 178
;;
defb %00000000
defb %01100110
defb %01100110
defb %00111100
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 179
;;
defb %00111100
defb %01100000
defb %01100000
defb %00111100
defb %01100110
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 180
;;
defb %00000000
defb %00000000
defb %00011110
defb %00110000
defb %01111100
defb %00110000
defb %00011110
defb %00000000
--------------------------
;; character 181
;;
defb %00111000
defb %01101100
defb %11000110
defb %11111110
defb %11000110
defb %01101100
defb %00111000
defb %00000000
--------------------------
;; character 182
;;
defb %00000000
defb %11000000
defb %01100000
defb %00110000
defb %00111000
defb %01101100
defb %11000110
defb %00000000
--------------------------
;; character 183
;;
defb %00000000
defb %00000000
defb %01100110
defb %01100110
defb %01100110
defb %01111100
defb %01100000
defb %01100000
--------------------------
;; character 184
;;
defb %00000000
defb %00000000
defb %00000000
defb %11111110
defb %01101100
defb %01101100
defb %01101100
defb %00000000
--------------------------
;; character 185
;;
defb %00000000
defb %00000000
defb %00000000
defb %01111110
defb %11011000
defb %11011000
defb %01110000
defb %00000000
--------------------------
;; character 186
;;
defb %00000011
defb %00000110
defb %00001100
defb %00111100
defb %01100110
defb %00111100
defb %01100000
defb %11000000
--------------------------
;; character 187
;;
defb %00000011
defb %00000110
defb %00001100
defb %01100110
defb %01100110
defb %00111100
defb %01100000
defb %11000000
--------------------------
;; character 188
;;
defb %00000000
defb %11100110
defb %00111100
defb %00011000
defb %00111000
defb %01101100
defb %11000111
defb %00000000
--------------------------
;; character 189
;;
defb %00000000
defb %00000000
defb %01100110
defb %11000011
defb %11011011
defb %11011011
defb %01111110
defb %00000000
--------------------------
;; character 190
;;
defb %11111110
defb %11000110
defb %01100000
defb %00110000
defb %01100000
defb %11000110
defb %11111110
defb %00000000
--------------------------
;; character 191
;;
defb %00000000
defb %01111100
defb %11000110
defb %11000110
defb %11000110
defb %01101100
defb %11101110
defb %00000000
--------------------------
;; character 192
;;
defb %00011000
defb %00110000
defb %01100000
defb %11000000
defb %10000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 193
;;
defb %00011000
defb %00001100
defb %00000110
defb %00000011
defb %00000001
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 194
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000001
defb %00000011
defb %00000110
defb %00001100
defb %00011000
--------------------------
;; character 195
;;
defb %00000000
defb %00000000
defb %00000000
defb %10000000
defb %11000000
defb %01100000
defb %00110000
defb %00011000
--------------------------
;; character 196
;;
defb %00011000
defb %00111100
defb %01100110
defb %11000011
defb %10000001
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 197
;;
defb %00011000
defb %00001100
defb %00000110
defb %00000011
defb %00000011
defb %00000110
defb %00001100
defb %00011000
--------------------------
;; character 198
;;
defb %00000000
defb %00000000
defb %00000000
defb %10000001
defb %11000011
defb %01100110
defb %00111100
defb %00011000
--------------------------
;; character 199
;;
defb %00011000
defb %00110000
defb %01100000
defb %11000000
defb %11000000
defb %01100000
defb %00110000
defb %00011000
--------------------------
;; character 200
;;
defb %00011000
defb %00110000
defb %01100000
defb %11000001
defb %10000011
defb %00000110
defb %00001100
defb %00011000
--------------------------
;; character 201
;;
defb %00011000
defb %00001100
defb %00000110
defb %10000011
defb %11000001
defb %01100000
defb %00110000
defb %00011000
--------------------------
;; character 202
;;
defb %00011000
defb %00111100
defb %01100110
defb %11000011
defb %11000011
defb %01100110
defb %00111100
defb %00011000
--------------------------
;; character 203
;;
defb %11000011
defb %11100111
defb %01111110
defb %00111100
defb %00111100
defb %01111110
defb %11100111
defb %11000011
--------------------------
;; character 204
;;
defb %00000011
defb %00000111
defb %00001110
defb %00011100
defb %00111000
defb %01110000
defb %11100000
defb %11000000
--------------------------
;; character 205
;;
defb %11000000
defb %11100000
defb %01110000
defb %00111000
defb %00011100
defb %00001110
defb %00000111
defb %00000011
--------------------------
;; character 206
;;
defb %11001100
defb %11001100
defb %00110011
defb %00110011
defb %11001100
defb %11001100
defb %00110011
defb %00110011
--------------------------
;; character 207
;;
defb %10101010
defb %01010101
defb %10101010
defb %01010101
defb %10101010
defb %01010101
defb %10101010
defb %01010101
--------------------------
;; character 208
;;
defb %11111111
defb %11111111
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 209
;;
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
defb %00000011
--------------------------
;; character 210
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %11111111
defb %11111111
--------------------------
;; character 211
;;
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
defb %11000000
--------------------------
;; character 212
;;
defb %11111111
defb %11111110
defb %11111100
defb %11111000
defb %11110000
defb %11100000
defb %11000000
defb %10000000
--------------------------
;; character 213
;;
defb %11111111
defb %01111111
defb %00111111
defb %00011111
defb %00001111
defb %00000111
defb %00000011
defb %00000001
--------------------------
;; character 214
;;
defb %00000001
defb %00000011
defb %00000111
defb %00001111
defb %00011111
defb %00111111
defb %01111111
defb %11111111
--------------------------
;; character 215
;;
defb %10000000
defb %11000000
defb %11100000
defb %11110000
defb %11111000
defb %11111100
defb %11111110
defb %11111111
--------------------------
;; character 216
;;
defb %10101010
defb %01010101
defb %10101010
defb %01010101
defb %00000000
defb %00000000
defb %00000000
defb %00000000
--------------------------
;; character 217
;;
defb %00001010
defb %00000101
defb %00001010
defb %00000101
defb %00001010
defb %00000101
defb %00001010
defb %00000101
--------------------------
;; character 218
;;
defb %00000000
defb %00000000
defb %00000000
defb %00000000
defb %10101010
defb %01010101
defb %10101010
defb %01010101
--------------------------
;; character 219
;;
defb %10100000
defb %01010000
defb %10100000
defb %01010000
defb %10100000
defb %01010000
defb %10100000
defb %01010000
--------------------------
;; character 220
;;
defb %10101010
defb %01010100
defb %10101000
defb %01010000
defb %10100000
defb %01000000
defb %10000000
defb %00000000
--------------------------
;; character 221
;;
defb %10101010
defb %01010101
defb %00101010
defb %00010101
defb %00001010
defb %00000101
defb %00000010
defb %00000001
--------------------------
;; character 222
;;
defb %00000001
defb %00000010
defb %00000101
defb %00001010
defb %00010101
defb %00101010
defb %01010101
defb %10101010
--------------------------
;; character 223
;;
defb %00000000
defb %10000000
defb %01000000
defb %10100000
defb %01010000
defb %10101000
defb %01010100
defb %10101010
--------------------------
;; character 224
;;
defb %01111110
defb %11111111
defb %10011001
defb %11111111
defb %10111101
defb %11000011
defb %11111111
defb %01111110
--------------------------
;; character 225
;;
defb %01111110
defb %11111111
defb %10011001
defb %11111111
defb %11000011
defb %10111101
defb %11111111
defb %01111110
--------------------------
;; character 226
;;
defb %00111000
defb %00111000
defb %11111110
defb %11111110
defb %11111110
defb %00010000
defb %00111000
defb %00000000
--------------------------
;; character 227
;;
defb %00010000
defb %00111000
defb %01111100
defb %11111110
defb %01111100
defb %00111000
defb %00010000
defb %00000000
--------------------------
;; character 228
;;
defb %01101100
defb %11111110
defb %11111110
defb %11111110
defb %01111100
defb %00111000
defb %00010000
defb %00000000
--------------------------
;; character 229
;;
defb %00010000
defb %00111000
defb %01111100
defb %11111110
defb %11111110
defb %00010000
defb %00111000
defb %00000000
--------------------------
;; character 230
;;
defb %00000000
defb %00111100
defb %01100110
defb %11000011
defb %11000011
defb %01100110
defb %00111100
defb %00000000
--------------------------
;; character 231
;;
defb %00000000
defb %00111100
defb %01111110
defb %11111111
defb %11111111
defb %01111110
defb %00111100
defb %00000000
--------------------------
;; character 232
;;
defb %00000000
defb %01111110
defb %01100110
defb %01100110
defb %01100110
defb %01100110
defb %01111110
defb %00000000
--------------------------
;; character 233
;;
defb %00000000
defb %01111110
defb %01111110
defb %01111110
defb %01111110
defb %01111110
defb %01111110
defb %00000000
--------------------------
;; character 234
;;
defb %00001111
defb %00000111
defb %00001101
defb %01111000
defb %11001100
defb %11001100
defb %11001100
defb %01111000
--------------------------
;; character 235
;;
defb %00111100
defb %01100110
defb %01100110
defb %01100110
defb %00111100
defb %00011000
defb %01111110
defb %00011000
--------------------------
;; character 236
;;
defb %00001100
defb %00001100
defb %00001100
defb %00001100
defb %00001100
defb %00111100
defb %01111100
defb %00111000
--------------------------
;; character 237
;;
defb %00011000
defb %00011100
defb %00011110
defb %00011011
defb %00011000
defb %01111000
defb %11111000
defb %01110000
--------------------------
;; character 238
;;
defb %10011001
defb %01011010
defb %00100100
defb %11000011
defb %11000011
defb %00100100
defb %01011010
defb %10011001
--------------------------
;; character 239
;;
defb %00010000
defb %00111000
defb %00111000
defb %00111000
defb %00111000
defb %00111000
defb %01111100
defb %11010110
--------------------------
;; character 240
;;
defb %00011000
defb %00111100
defb %01111110
defb %11111111
defb %00011000
defb %00011000
defb %00011000
defb %00011000
--------------------------
;; character 241
;;
defb %00011000
defb %00011000
defb %00011000
defb %00011000
defb %11111111
defb %01111110
defb %00111100
defb %00011000
--------------------------
;; character 242
;;
defb %00010000
defb %00110000
defb %01110000
defb %11111111
defb %11111111
defb %01110000
defb %00110000
defb %00010000
--------------------------
;; character 243
;;
defb %00001000
defb %00001100
defb %00001110
defb %11111111
defb %11111111
defb %00001110
defb %00001100
defb %00001000
--------------------------
;; character 244
;;
defb %00000000
defb %00000000
defb %00011000
defb %00111100
defb %01111110
defb %11111111
defb %11111111
defb %00000000
--------------------------
;; character 245
;;
defb %00000000
defb %00000000
defb %11111111
defb %11111111
defb %01111110
defb %00111100
defb %00011000
defb %00000000
--------------------------
;; character 246
;;
defb %10000000
defb %11100000
defb %11111000
defb %11111110
defb %11111000
defb %11100000
defb %10000000
defb %00000000
--------------------------
;; character 247
;;
defb %00000010
defb %00001110
defb %00111110
defb %11111110
defb %00111110
defb %00001110
defb %00000010
defb %00000000
--------------------------
;; character 248
;;
defb %00111000
defb %00111000
defb %10010010
defb %01111100
defb %00010000
defb %00101000
defb %00101000
defb %00101000
--------------------------
;; character 249
;;
defb %00111000
defb %00111000
defb %00010000
defb %11111110
defb %00010000
defb %00101000
defb %01000100
defb %10000010
--------------------------
;; character 250
;;
defb %00111000
defb %00111000
defb %00010010
defb %01111100
defb %10010000
defb %00101000
defb %00100100
defb %00100010
--------------------------
;; character 251
;;
defb %00111000
defb %00111000
defb %10010000
defb %01111100
defb %00010010
defb %00101000
defb %01001000
defb %10001000
--------------------------
;; character 252
;;
defb %00000000
defb %00111100
defb %00011000
defb %00111100
defb %00111100
defb %00111100
defb %00011000
defb %00000000
--------------------------
;; character 253
;;
defb %00111100
defb %11111111
defb %11111111
defb %00011000
defb %00001100
defb %00011000
defb %00110000
defb %00011000
--------------------------
;; character 254
;;
defb %00011000
defb %00111100
defb %01111110
defb %00011000
defb %00011000
defb %01111110
defb %00111100
defb %00011000
--------------------------
;; character 255
;;
defb %00000000
defb %00100100
defb %01100110
defb %11111111
defb %01100110
defb %00100100
defb %00000000
defb %00000000