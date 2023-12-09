#pragma once

class FEmuBase;
class FLuaConsole;

typedef struct lua_State lua_State;

namespace LuaSys
{
	bool Init(FEmuBase* pEmulator);
	void Shutdown(void);

    lua_State*  GetGlobalState();

    bool LoadFile(const char* pFileName);
    void ExecuteString(const char *pString);
    void OutputDebugString(const char* fmt, ...);

    //FLuaConsole* GetLuaConsole();
    FEmuBase* GetEmulator();
    void DrawUI();

    void DumpStack(lua_State *L);
}
