#pragma once

#include "imgui.h"
#include <string>
#include <vector>

enum class SpeccyModel
{
	Spectrum48K,
	Spectrum128K
};

struct FSpeccyConfig
{
	SpeccyModel	Model;
};

struct FSpeccy
{
	void*		EmuState;		// opaque point to zx state
	unsigned char* FrameBuffer;	// pixel buffer to store emu output
	ImTextureID	Texture;		// texture 

};

FSpeccy* InitSpeccy(const FSpeccyConfig& config);
void TickSpeccy(FSpeccy &speccyInstance);
void ShutdownSpeccy(FSpeccy*&pSpeccy);

const std::vector<std::string>& GetGameList();
bool LoadZ80File(FSpeccy &speccyInstance, const char *fName);
