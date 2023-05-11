	DEVICE ZXSPECTRUM48

	ORG $8000
Data:
	DEFB $1

Start:
	DI
	LD A, (Data)
	INC A
	LD (Data), A

Loop:
	JR Loop

	SAVESNA "TestMinimal.sna", Start
