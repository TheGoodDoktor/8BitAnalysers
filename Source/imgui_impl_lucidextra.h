#pragma once

#include "imgui.h"

ImTextureID ImGui_ImplDX11_CreateTextureRGBA(unsigned char* pixels, int width, int height);
void ImGui_ImplDX11_UpdateTextureRGBA(ImTextureID texture, unsigned char* pixels);

