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
void RegisterLuaFunctionToolTip(const char* functionName, const char* tooltipText);
FLuaTextEditor& AddTextEditor(const char* fileName, const char* pTextData);
void DrawTextEditor(void);