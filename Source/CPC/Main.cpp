#include "Misc/MainLoop.h"

#include "CPCEmu.h"

int main(int argc, char** argv)
{
	FCpcLaunchConfig config;
	config.ParseCommandline(argc, argv);
	FEmuBase* pEmulator = new FCpcEmu;

	RunMainLoop(pEmulator,config);
	return 0;
}