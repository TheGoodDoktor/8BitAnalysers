#include "imgui.h"
#include <cstdint>

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <vector>


// assume it's 8bpp
ImTextureID ImGui_CreateTextureRGBA(const void* pixels, int width, int height)
{
	GLuint newTexture = 0;
	GLint lastTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
	glGenTextures(1, &newTexture);
	glBindTexture(GL_TEXTURE_2D, newTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	ImTextureID textureId = (ImTextureID)(intptr_t)newTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, lastTexture);

	return textureId;
}

ImTextureID ImGui_CreateTexturePal8(const void* pixels,uint32_t *pPalette, int width, int height)
{
	GLuint newTexture = 0;
	GLint lastTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
	glGenTextures(1, &newTexture);
	glBindTexture(GL_TEXTURE_2D, newTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

	

	//glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_PALETTE4_RGB8_OES, width, height, 0, width * height, pixels);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	ImTextureID textureId = (ImTextureID)(intptr_t)newTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, lastTexture);

	return textureId;
}

void ImGui_FreeTexture(ImTextureID texture)
{
	glDeleteTextures(1, (GLuint*)(intptr_t*)&texture);
}

void ImGui_UpdateTextureRGBA(ImTextureID texture,const void* pixels)
{
	GLint lastTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);

	glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)texture);
	int width, height;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Restore state
	glBindTexture(GL_TEXTURE_2D, lastTexture);
}

static std::vector<uint32_t>	g_UploadBuffer;

void ImGui_UpdateTextureRGBA(ImTextureID texture,const void* pixels, int srcWidth, int srcHeight)
{
	GLint lastTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);

	glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)texture);
	int width, height;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcWidth, srcHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
