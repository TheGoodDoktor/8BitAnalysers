#pragma once

typedef void* ImTextureID;

ImTextureID ImGui_CreateTextureRGBA(const void* pixels, int width, int height);
void ImGui_FreeTexture(ImTextureID);
void ImGui_UpdateTextureRGBA(ImTextureID texture, const void* pixels);
void ImGui_UpdateTextureRGBA(ImTextureID texture, const void* pixels, int srcWidth, int srcHeight);
