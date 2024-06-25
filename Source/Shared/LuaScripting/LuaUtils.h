#pragma once

#include <string>

struct lua_State;
struct FAddressRef;

FAddressRef GetAddressRefFromLua(lua_State* pState, int stackPos = -1);
bool GetLuaTableField(lua_State* pState, const char* fieldName, FAddressRef& value);
bool GetLuaTableField(lua_State* pState, const char* fieldName, int& value);
bool GetLuaTableField(lua_State* pState, const char* fieldName, bool& value);
bool GetLuaTableField(lua_State* pState, const char* fieldName, std::string& value);
