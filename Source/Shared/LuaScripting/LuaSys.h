#pragma once

class FEmuBase;
class FLuaConsole;

typedef struct lua_State lua_State;

namespace LuaSys
{
    class FLuaScopeCheck
    {
    public:
        FLuaScopeCheck(lua_State* pState);
        ~FLuaScopeCheck();
    private:
        lua_State* LuaState = nullptr;
        int InitialStackItems = 0;
    };

	bool Init(FEmuBase* pEmulator);
	void Shutdown(void);

    lua_State*  GetGlobalState();

    bool LoadFile(const char* pFileName, bool bAddEditor);
    void ExecuteString(const char *pString);
    void OutputDebugString(const char* fmt, ...);

    //FLuaConsole* GetLuaConsole();
    FEmuBase* GetEmulator();
    void DrawUI();

    void DumpStack(lua_State *L);
}
