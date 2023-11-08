#include "SpectrumEmu.h"
#include "Misc/MainLoop.h"

#ifndef TEST
int main(int argc, char** argv)
{
	FSpectrumLaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FSpectrumEmu;
	RunMainLoop(pEmulator, launchConfig);
}
#endif