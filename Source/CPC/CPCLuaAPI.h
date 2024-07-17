#pragma once

typedef struct lua_State lua_State;

int RegisterCPCLuaAPI(lua_State *pState);
void AddCPCLibLuaDocs(void);
