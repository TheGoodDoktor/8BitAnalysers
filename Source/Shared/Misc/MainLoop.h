#pragma once

class FEmuBase;
struct FEmulatorLaunchConfig;

int RunMainLoop(FEmuBase* pEmulator, const FEmulatorLaunchConfig& launchConfig);