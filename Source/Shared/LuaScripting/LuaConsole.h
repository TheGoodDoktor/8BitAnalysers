#pragma once

#include <imgui.h>
#include <ctype.h>

class FLuaConsole
{
public:
    FLuaConsole();
    ~FLuaConsole();

    void    ClearLog();
    void    AddLog(const char* fmt, ...);
    void    Draw(const char* title, bool* p_open);
    void    ExecCommand(const char* command_line);
    int     TextEditCallback(ImGuiInputTextCallbackData* data);
    
private:
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

};
