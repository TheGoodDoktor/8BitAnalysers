#include "ArcadeZ80.h"
#include "Misc/MainLoop.h"


int main(int argc, char** argv)
{
	FArcadeZ80LaunchConfig launchConfig;
	launchConfig.ParseCommandline(argc, argv);

	FEmuBase* pEmulator = new FArcadeZ80;
	RunMainLoop(pEmulator, launchConfig);

	delete pEmulator;
}