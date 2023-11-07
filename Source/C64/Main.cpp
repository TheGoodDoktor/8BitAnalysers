#include "C64Emulator.h"
#include "Misc/MainLoop.h"


int main(int argc, char** argv)
{
	FC64LaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FC64Emulator;
	RunMainLoop(pEmulator, launchConfig);
}