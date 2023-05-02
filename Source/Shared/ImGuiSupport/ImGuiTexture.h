#pragma once

typedef void* ImTextureID;

ImTextureID ImGui_CreateTextureRGBA(void* pixels, int width, int height);
void ImGui_FreeTexture(ImTextureID);
void ImGui_UpdateTextureRGBA(ImTextureID texture, void* pixels);
void ImGui_UpdateTextureRGBA(ImTextureID texture, void* pixels, int srcWidth, int srcHeight);
