#include "LuaUtils.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaSys.h"

#include "Misc/EmuBase.h"
#include "Misc/GlobalConfig.h"
#include "Misc/GameConfig.h"
#include "Util/GraphicsView.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

// Get Address ref from stack
FAddressRef GetAddressRefFromLua(lua_State* pState, int stackPos)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	if (lua_isinteger(pState, stackPos))	// numerical physical address
	{
		return state.AddressRefFromPhysicalAddress((uint16_t)lua_tointeger(pState, stackPos));
	}

	return FAddressRef();	// return invalid
}

// table getting functions
// assumes the table is at the top of the stack
bool GetLuaTableField(lua_State* pState, const char* fieldName, FAddressRef& value)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	lua_getfield(pState, -1, fieldName);
	FAddressRef addrRef = GetAddressRefFromLua(pState, -1);
	if(addrRef.IsValid())
		value = addrRef;
	lua_pop(pState, 1);
	return addrRef.IsValid();
}

bool GetLuaTableField(lua_State* pState, const char* fieldName, int& value)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	lua_getfield(pState, -1, fieldName);
	const bool bValidField = lua_isinteger(pState, -1);
	if (bValidField)
		value = (int)lua_tointeger(pState, -1);
	lua_pop(pState, 1);
	return bValidField;
}

bool GetLuaTableField(lua_State* pState, const char* fieldName, bool& value)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	lua_getfield(pState, -1, fieldName);
	const bool bValidField = lua_isboolean(pState, -1);
	if (bValidField)
		value = lua_toboolean(pState, -1);
	lua_pop(pState, 1);
	return bValidField;
}

bool GetLuaTableField(lua_State* pState, const char* fieldName, std::string& value)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	lua_getfield(pState, -1, fieldName);
	const bool bValidField = lua_isstring(pState, -1);
	if (bValidField)
		value = lua_tostring(pState, -1);
	lua_pop(pState, 1);
	return bValidField;
}