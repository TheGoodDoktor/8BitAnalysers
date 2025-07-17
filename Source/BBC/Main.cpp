#include "BBCEmulator.h"
#include "Misc/MainLoop.h"


int main(int argc, char** argv)
{
	FBBCLaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FBBCEmulator;
	RunMainLoop(pEmulator, launchConfig);
}