#include "LuaCoreAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

static int core_readbyte(lua_State *L)
{
    // TODO: get address
    // TODO: read from address
    lua_Integer byteVal = 0;
    
    lua_pushinteger(L, byteVal);
    return 1;
}

static const luaL_Reg corelib[] =
{
    {"readbyte", core_readbyte},
    {NULL, NULL}    // terminator
};

int luaopen_string (lua_State *L) 
{
  luaL_newlib(L, corelib);
  //createmetatable(L);
  return 1;
}
