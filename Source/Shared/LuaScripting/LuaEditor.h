#pragma once

#include "ImGuiColorTextEdit/TextEditor.h"

// text editor stuff - move?
struct FLuaTextEditor
{
	std::string SourceFileName;
	std::string SourceName;
	TextEditor  LuaTextEditor;
};

bool InitTextEditors(void);
FLuaTextEditor& AddTextEditor(const char* fileName, const char* pTextData);
void DrawTextEditor(void);