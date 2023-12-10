#include "LuaSys.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaCoreAPI.h"
#include "LuaConsole.h"

#include "Misc/EmuBase.h"
#include "Misc/GlobalConfig.h"
#include "Util/FileUtil.h"

#include "Debug/DebugLog.h"

#include "ImGuiColorTextEdit/TextEditor.h"

namespace LuaSys
{

lua_State*	GlobalState = nullptr;

FLuaConsole LuaConsole;

FEmuBase*   EmuBase = nullptr;

// text editor stuff - move?
struct FLuaTextEditor
{
    std::string SourceFileName;
    std::string SourceName;
    TextEditor  LuaTextEditor;
};
FLuaTextEditor& AddTextEditor(const char* fileName, const char* pTextData);
void DrawTextEditor(void);

void lua_warning_function(void *ud, const char *msg, int tocont)
{
    LuaConsole.AddLog("%s",msg);
}


bool Init(FEmuBase* pEmulator)
{
    lua_State* pState = luaL_newstate();	// create the global state

    // Add system libraries
    luaL_openlibs(pState);		// opens the basic library

    // TODO: Add our libraries
    luaopen_corelib(pState);
    
    lua_setwarnf(pState,lua_warning_function,nullptr);

    GlobalState = pState;
    EmuBase = pEmulator;
    
    // Create the global 'Viewers' table
    lua_getglobal(pState, "_G");
    lua_pushstring(pState, "Viewers");
    lua_newtable(pState);
    lua_rawset(pState, -3);
    
    return true;
}

void Shutdown(void)
{

}

lua_State*  GetGlobalState()
{
    return GlobalState;
}

bool LoadFile(const char* pFileName)
{
    char* pTextData = LoadTextFile(pFileName);
    if(pTextData != nullptr)
    {
        ExecuteString(pTextData);
        AddTextEditor(pFileName, pTextData);
        delete pTextData;
        return true;
    }
    return false;
    
    const int ret = luaL_dofile(GlobalState, pFileName);
    
    if (ret == LUA_OK)
    {
        return true;
    }
    
    LOGWARNING("Lua script %s not found",pFileName);
    return false;
}

void ExecuteString(const char *pString)
{
    //luaL_dostring(GlobalState, pString);
    lua_State* pState = GlobalState;
    
    const int ret = luaL_dostring(pState, pString);

    if (ret != LUA_OK)
    {
        LuaConsole.AddLog("[error] %s", lua_tostring(pState, -1));
        lua_pop(pState, 1); // pop error message
    }
}

void OutputDebugString(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    LuaConsole.AddLog("%s",buf);
    printf("%s\n",buf);
}

FLuaConsole* GetLuaConsole()
{
    return &LuaConsole;
}

FEmuBase* GetEmulator()
{
    return EmuBase;
}

void DrawViewerTab(lua_State* pState)
{
    //DumpStack(pState);
    if(!lua_istable(pState, -1))
        return;
    
    lua_pushstring(pState,"name");
    lua_gettable(pState, -2);
    if(lua_isstring(pState, -1))
    {
        if(ImGui::BeginTabItem(lua_tostring(pState, -1)))
        {
            lua_pushvalue(pState,-2);
            lua_pushstring(pState, "onDrawUI");
            lua_gettable(pState, -2);
            if(lua_isfunction(pState, -1))
            {
                lua_pcall(pState, 0, 0, 0); // 0 arguments, 0 return values
            }
            lua_pop(pState,2);  // onDraw function
        }
        ImGui::EndTabItem();
    }
    lua_pop(pState,1);  // name string
    
    
}

void DrawUI()
{
    if( EmuBase->GetGlobalConfig()->bEnableLua == false)
        return;
    
    static bool bOpen = true;
    LuaConsole.Draw("Lua Console", &bOpen);
    
    lua_State* pState = GlobalState;
    const int top = lua_gettop(pState);

    lua_getglobal(pState, "Viewers");
    //DumpStack(pState);
    if(lua_istable(pState,-1) && lua_rawlen(pState, -1) > 0)
    {
        if(ImGui::Begin("Game Viewers"))
        {
            if(ImGui::BeginTabBar("Viewer Tabs"))
            {
                const int itemCount = (int)lua_rawlen(pState, -1);
                for(int i=0;i<itemCount;i++)
                {
                    lua_pushnumber(pState, i+1);// indexes start at 1
                    lua_gettable(pState,-2);
                    DrawViewerTab(pState);
                    lua_pop(pState,1);  // pop item
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
    lua_pop(pState,1);
    
    const int top2 = lua_gettop(pState);

    DrawTextEditor();
}

void DumpStack(lua_State *L)
{
    const int top = lua_gettop(L);
    
    for (int i = 1; i <= top; i++)
    {
        const int t = lua_type(L, i);
        switch (t)
        {
            case LUA_TSTRING:  /* strings */
                OutputDebugString("`%s'", lua_tostring(L, i));
                break;

            case LUA_TBOOLEAN:  /* booleans */
                OutputDebugString(lua_toboolean(L, i) ? "true" : "false");
                break;

            case LUA_TNUMBER:  /* numbers */
                OutputDebugString("%g", lua_tonumber(L, i));
                break;

            default:  /* other values */
                OutputDebugString("%s", lua_typename(L, t));
                break;

        }
        OutputDebugString("  ");  /* put a separator */
    }
    OutputDebugString("\n");  /* end the listing */
}

// Text Editor - move?


std::vector<FLuaTextEditor> TextEditors;


FLuaTextEditor& AddTextEditor(const char* pFileName, const char* pTextData)
{
    FLuaTextEditor& editor = TextEditors.emplace_back();
    
    editor.SourceName = GetFileFromPath(pFileName);
    editor.SourceFileName = std::string(pFileName);
    
    // set up text editor
    auto lang = TextEditor::LanguageDefinition::Lua();
    editor.LuaTextEditor.SetLanguageDefinition(lang);
    editor.LuaTextEditor.SetText(pTextData);
    
    return editor;
}

void DrawTextEditor(void)
{
    if(ImGui::Begin("Lua Editor"))
    {
        if(ImGui::BeginTabBar("Editor Tabs"))
        {
            for(auto& editor : TextEditors)
            {
                if(ImGui::BeginTabItem(editor.SourceName.c_str()))
                {
                    if(ImGui::Button("Update"))
                    {
                        ExecuteString(editor.LuaTextEditor.GetText().c_str());
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Save"))
                    {
                        // TODO: Save Action
                    }
                    editor.LuaTextEditor.Render(editor.SourceName.c_str());
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}


}//namespace LuaSys

