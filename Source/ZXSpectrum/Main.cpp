#include "SpectrumEmu.h"
#include "Misc/MainLoop.h"

#ifndef TEST
int main(int argc, char** argv)
{
	FAddressRef addrTestA(10, 1);
	FAddressRef addrTestB(12, 2);
	FAddressRef addrTestC(14, 3);
	FAddressRef addrTestD(16, 4);
	FSpectrumLaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FSpectrumEmu;
	RunMainLoop(pEmulator, launchConfig);
}
#endif