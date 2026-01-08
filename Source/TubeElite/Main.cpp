#include "TubeElite.h"
#include "Misc/MainLoop.h"


int main(int argc, char** argv)
{
	FTubeEliteLaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FTubeElite;
	RunMainLoop(pEmulator, launchConfig);
}