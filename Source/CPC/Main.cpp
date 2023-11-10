#include "Misc/MainLoop.h"

#include "CPCEmu.h"

int main(int argc, char** argv)
{
	FCPCLaunchConfig config;
	config.ParseCommandline(argc, argv);
	FEmuBase* pEmulator = new FCPCEmu;

	RunMainLoop(pEmulator,config);
	return 0;
}