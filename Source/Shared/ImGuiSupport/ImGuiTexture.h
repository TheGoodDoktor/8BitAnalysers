#pragma once

typedef void* ImTextureID;

ImTextureID ImGui_CreateTextureRGBA(unsigned char* pixels, int width, int height);
void ImGui_FreeTexture(ImTextureID);
void ImGui_UpdateTextureRGBA(ImTextureID texture, unsigned char* pixels);
void ImGui_UpdateTextureRGBA(ImTextureID texture, unsigned char* pixels, int srcWidth, int srcHeight);
