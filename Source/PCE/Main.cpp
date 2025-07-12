#include "PCEEmu.h"
#include "Misc/MainLoop.h"

#ifndef TEST
int main(int argc, char** argv)
{
	FPCELaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FPCEEmu;
	RunMainLoop(pEmulator, launchConfig);
}
#endif