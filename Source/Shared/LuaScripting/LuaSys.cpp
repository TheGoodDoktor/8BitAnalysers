extern "C" 
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaCoreAPI.h"
#include "Misc/EmuBase.h"

namespace LuaSys
{

lua_State*	GlobalState = nullptr;

bool Init(FEmuBase* pEmulator)
{
    GlobalState = luaL_newstate();	// create the global state

    // Add system libraries
    luaopen_base(GlobalState);		// opens the basic library
    luaopen_table(GlobalState);		// opens the table library
    luaopen_io(GlobalState);		// opens the I/O library
    luaopen_string(GlobalState);	// opens the string lib.
    luaopen_math(GlobalState);		//opens the math lib.

    // TODO: Add our libraries

    return true;
}

void Shutdown(void)
{

}

bool LoadFile(const char* pFileName)
{
    luaL_dofile(GlobalState, pFileName);
}

}//namespace LuaSys

