#pragma once

typedef struct lua_State lua_State;

int RegisterSpectrumLuaAPI(lua_State *pState);
void AddZXLibLuaDocs(void);
