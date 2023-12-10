#include "LuaCoreAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaSys.h"
//#include "LuaConsole.h"

#include "Misc/EmuBase.h"



static int print(lua_State* pState)
{
    int nargs = lua_gettop(pState);

    for (int i=1; i <= nargs; i++) 
    {
        if (lua_isstring(pState, i))
        {
            /* Pop the next arg using lua_tostring(L, i) and do your print */
            const char* pString = lua_tostring(pState, i);
            LuaSys::OutputDebugString("%s", pString);
        }
        else 
        {
        /* Do something with non-strings if you like */
        }
    }

    return 0;
}

static int readbyte(lua_State* pState)
{
    FEmuBase* pEmu = LuaSys::GetEmulator();
    
    if(pEmu != nullptr && lua_isinteger(pState, -1))
    {
        const lua_Integer address = lua_tointeger(pState, -1);
        lua_pushinteger(pState, pEmu->ReadByte(address));
        return 1;
    }
    
    return 0;
}

static int readword(lua_State* pState)
{
    FEmuBase* pEmu = LuaSys::GetEmulator();
    
    if(pEmu != nullptr && lua_isinteger(pState, -1))
    {
        const lua_Integer address = lua_tointeger(pState, -1);
        lua_pushinteger(pState, pEmu->ReadWord(address));
        return 1;
    }
    
    return 0;
}

static int getMemPtr(lua_State* pState)
{
    FEmuBase* pEmu = LuaSys::GetEmulator();
    
    if(pEmu != nullptr && lua_isinteger(pState, -1))
    {
        const lua_Integer address = lua_tointeger(pState, -1);
        lua_pushlightuserdata(pState, (void*)pEmu->GetMemPtr(address));
        return 1;
    }
    
    return 0;
}
static int addViewer(lua_State* pState)
{
    //LuaSys::FLuaScopeCheck check(pState);
    
    if(lua_istable(pState, -1))
    {
        lua_getglobal(pState, "Viewers");
        if(lua_istable(pState, -1))
        {
            lua_pushvalue(pState,-2);
            lua_rawseti(pState,-2,lua_rawlen(pState,-2) + 1);
        }
    }
    return 0;
}

static const luaL_Reg corelib[] =
{
    {"print", print},
    {"readbyte", readbyte},
    {"readword", readword},
    //{"addViewer", addViewer},
    {NULL, NULL}    // terminator
};


int luaopen_corelib(lua_State *pState)
{
    lua_getglobal(pState, "_G");
    luaL_setfuncs(pState, corelib, 0);  // for Lua versions 5.2 or greater
    lua_pop(pState, 1);
    return 1;
}

