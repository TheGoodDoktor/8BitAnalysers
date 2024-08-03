#include "LuaCoreAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaSys.h"
#include "LuaUtils.h"
#include "LuaDocs.h"

#include "Misc/EmuBase.h"
#include "Misc/GlobalConfig.h"
#include "Misc/GameConfig.h"
#include "Util/GraphicsView.h"
#include <ImGuiSupport/ImGuiScaling.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"


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

static int ReadByte(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	
	if(pEmu != nullptr && lua_isinteger(pState, -1))
	{
		const lua_Integer address = lua_tointeger(pState, -1);
		lua_pushinteger(pState, pEmu->ReadByte((uint16_t)address));
		return 1;
	}
	
	return 0;
}

static int ReadWord(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	
	if(pEmu != nullptr && lua_isinteger(pState, -1))
	{
		const lua_Integer address = lua_tointeger(pState, -1);
		lua_pushinteger(pState, pEmu->ReadWord((uint16_t)address));
		return 1;
	}
	
	return 0;
}

static int WriteByte(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isinteger(pState, 1) && lua_isinteger(pState, 2))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		if(state.bAllowEditing)
		{
			const lua_Integer address = lua_tointeger(pState, 1);
			const lua_Integer value = lua_tointeger(pState, 2);
			pEmu->WriteByte((uint16_t)address,(uint8_t)value);
		}
	}

	return 0;
}

static int WriteWord(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isinteger(pState, 1) && lua_isinteger(pState, 2))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		if (state.bAllowEditing)
		{
			const lua_Integer address = lua_tointeger(pState, 1);
			const lua_Integer value = lua_tointeger(pState, 2);
			pEmu->WriteByte((uint16_t)address, (uint8_t)(value & 255));
			pEmu->WriteByte((uint16_t)address + 1, (uint8_t)((value>>8) & 255));
		}
	}

	return 0;
}

static int GetMemPtr(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	
	if(pEmu != nullptr && lua_isinteger(pState, -1))
	{
		const lua_Integer address = lua_tointeger(pState, -1);
		lua_pushlightuserdata(pState, (void*)pEmu->GetMemPtr((uint16_t)address));
		return 1;
	}
	
	return 0;
}

static int GetRegValue(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isstring(pState, -1))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
		size_t length = 0;
		const char* pRegName = luaL_tolstring(pState, -1, &length);

		uint8_t byteVal = 0;
		if (state.Debugger.GetRegisterByteValue(pRegName, byteVal))
		{
			lua_pushinteger(pState,byteVal);
			return 1;
		}
		uint16_t wordVal = 0;
		if(state.Debugger.GetRegisterWordValue(pRegName, wordVal))
		{
			lua_pushinteger(pState, wordVal);
			return 1;
		}
	}

	return 0;
}

static int RegisterExecutionHandler(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isinteger(pState, 1) && lua_isstring(pState, 2))
	{
		size_t length = 0;
		const lua_Integer address = lua_tointeger(pState, 1);
		const char* pFunctionName = luaL_tolstring(pState, 2, &length);
		LuaSys::RegisterExecutionHandler((uint16_t)address,pFunctionName);
	}

	return 0;	// TODO: return success?
}

static int RemoveExecutionHandler(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isinteger(pState, 1))
	{
		const lua_Integer address = lua_tointeger(pState, 1);
		LuaSys::RemoveExecutionHandler((uint16_t)address);
	}

	return 0;
}

// Analysis related
static int SetEditMode(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isboolean(pState, 1))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
		const bool bEnterEditMode = lua_toboolean(pState,1);
		if (bEnterEditMode != state.bAllowEditing)
		{
			state.bAllowEditing = bEnterEditMode;
			if(bEnterEditMode)
				pEmu->OnEnterEditMode();
			else
				pEmu->OnExitEditMode();
		}
	}
	return 0;
}

static int SetDataItemComment(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isstring(pState, 2))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		const FAddressRef addrRef = GetAddressRefFromLua(pState, 1);
		if (addrRef.IsValid())
		{
			size_t length = 0;
			const char* pText = luaL_tolstring(pState, 2, &length);

			FDataInfo* pDataInfo = state.GetDataInfoForAddress(addrRef);
			pDataInfo->Comment = pText;
		}
	}

	return 0;
}

static int SetCodeItemComment(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isstring(pState, 2))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		const FAddressRef addrRef = GetAddressRefFromLua(pState, 1);
		if(addrRef.IsValid())
		{
			size_t length = 0;
			const char* pText = luaL_tolstring(pState, 2, &length);

			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addrRef);
			if (pCodeInfo)
				pCodeInfo->Comment = pText;
		}
	}

	return 0;
}

static int AddCommentBlock(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isstring(pState, 2))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		const FAddressRef addrRef = GetAddressRefFromLua(pState, 1);
		if (addrRef.IsValid())
		{
			size_t length = 0;
			const char* pText = luaL_tolstring(pState, 2, &length);

			FCommentBlock* pCommentBlock = AddCommentBlock(state, addrRef);
			pCommentBlock->Comment = pText;
		}
	}

	return 0;
}


static int SetDataItemDisplayType(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isinteger(pState, 1) && lua_isinteger(pState, 2))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		// TODO: we'll need bank specified at some point
		const lua_Integer address = lua_tointeger(pState, 1);
		const EDataItemDisplayType displayType = (EDataItemDisplayType)lua_tointeger(pState, 2);
		FAddressRef addrRef = state.AddressRefFromPhysicalAddress((uint16_t)address);

		FDataInfo* pDataInfo = state.GetDataInfoForAddress(addrRef);
		pDataInfo->DisplayType = displayType;

		if(displayType == EDataItemDisplayType::Pointer)
		{
			//if(pDataInfo->ByteSize == 1)
			{
				pDataInfo->ByteSize = 2;
				pDataInfo->DataType = EDataType::Word;
				state.SetCodeAnalysisDirty(addrRef);
			}
		}
	}
	return 0;
}

// Formatting

// Generic format command
static int FormatMemory(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	if (pEmu == nullptr || lua_istable(pState, 1) == false)
		return 0;

	FDataFormattingOptions formatOptions;
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	// Pull values out of table
	lua_settop(pState,1);	// make sure table is at top of stack
	GetLuaTableField(pState, "DataType", (int&)formatOptions.DataType);
	GetLuaTableField(pState, "DisplayType", (int&)formatOptions.DisplayType);
	GetLuaTableField(pState, "StartAddress", formatOptions.StartAddress);
	GetLuaTableField(pState, "ItemSize", formatOptions.ItemSize);
	GetLuaTableField(pState, "NoItems", formatOptions.NoItems);
	GetLuaTableField(pState, "CharacterSet", formatOptions.CharacterSet);
	GetLuaTableField(pState, "GraphicsSetRef", formatOptions.GraphicsSetRef);
	GetLuaTableField(pState, "PaletteNo", formatOptions.PaletteNo);
	GetLuaTableField(pState, "StructId", formatOptions.StructId);
	// TODO: uint8_t					EmptyCharNo = 0;
	GetLuaTableField(pState, "RegisterItem", formatOptions.RegisterItem);
	GetLuaTableField(pState, "ClearCodeInfo", formatOptions.ClearCodeInfo);
	GetLuaTableField(pState, "ClearLabels", formatOptions.ClearLabels);
	GetLuaTableField(pState, "AddLabelAtStart", formatOptions.AddLabelAtStart);
	GetLuaTableField(pState, "LabelName", formatOptions.LabelName);
	GetLuaTableField(pState, "AddCommentAtStart", formatOptions.AddCommentAtStart);
	GetLuaTableField(pState, "CommentText", formatOptions.CommentText);

	// format
	FormatData(state, formatOptions);
	state.SetCodeAnalysisDirty(formatOptions.StartAddress);
	return 0;
}


static int FormatMemoryAsBitmap(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	if(pEmu == nullptr || lua_isinteger(pState, 1) == false)
		return 0;
	
	FDataFormattingOptions formatOptions;
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	// get parameters
	const lua_Integer address = lua_tointeger(pState, 1);	// address
	FAddressRef addrRef = state.AddressRefFromPhysicalAddress((uint16_t)address);
	const lua_Integer width = lua_isinteger(pState, 2) ? lua_tointeger(pState, 2) : 8;	// width - default to 8
	const lua_Integer height = lua_isinteger(pState, 3) ? lua_tointeger(pState, 3) : 8;	// height - default to 8
	const lua_Integer bpp = lua_isinteger(pState,4) ? lua_tointeger(pState, 4) : 1;	// bpp - default to 1

	formatOptions.SetupForBitmap(addrRef,(int)width, (int)height, (int)bpp);

	FormatData(state,formatOptions);
	state.SetCodeAnalysisDirty(formatOptions.StartAddress);
	return 0;
}

static int FormatMemoryAsCharMap(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();
	if (pEmu == nullptr || lua_isinteger(pState, 1) == false)
		return 0;

	FDataFormattingOptions formatOptions;
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	// get parameters
	const FAddressRef addrRef = GetAddressRefFromLua(pState,1);
	const lua_Integer width = lua_isinteger(pState, 2) ? lua_tointeger(pState, 2) : 1;	// width - default to 1
	const lua_Integer height = lua_isinteger(pState, 3) ? lua_tointeger(pState, 3) : 1;	// height - default to 1

	formatOptions.SetupForCharmap(addrRef, (int)width, (int)height);
	formatOptions.CharacterSet = GetAddressRefFromLua(pState, 4);

	FormatData(state, formatOptions);
	state.SetCodeAnalysisDirty(formatOptions.StartAddress);
	return 0;
}


// Gui related

static int DrawAddressLabel(lua_State* pState)
{
	FEmuBase* pEmu = LuaSys::GetEmulator();

	if (pEmu != nullptr && lua_isinteger(pState, -1))
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
		const lua_Integer address = lua_tointeger(pState, -1);
		FAddressRef addrRef = state.AddressRefFromPhysicalAddress((uint16_t)address);
		DrawAddressLabel(state,state.GetFocussedViewState(), addrRef);
	}

	return 0;
}

// Graphics related

static int GetImageScale(lua_State* pState)
{
	lua_pushnumber(pState, ImGui_GetScaling());
	return 1;
}

static int ClearGraphicsView(lua_State *pState)
{
	FGraphicsView* pGraphicsView = (FGraphicsView*)lua_touserdata(pState, 1 );
	if(pGraphicsView == nullptr)
		return 0;

	uint32_t clearCol = 0;
	if(lua_isinteger(pState, 2))
		clearCol = (uint32_t)lua_tointeger(pState, 2);
	pGraphicsView->Clear(clearCol);
	return 0;
}

static int DrawGraphicsView(lua_State *pState)
{
	FGraphicsView* pGraphicsView = (FGraphicsView*)lua_touserdata(pState, 1 );
	if (pGraphicsView == nullptr)
		return 0;

	pGraphicsView->UpdateTexture();
	pGraphicsView->Draw();
	return 0;
}

static int SaveGraphicsViewPNG(lua_State *pState)
{
	FGraphicsView* pGraphicsView = (FGraphicsView*)lua_touserdata(pState, 1 );
	if (pGraphicsView == nullptr)
		return 0;
	
	FEmuBase* pEmulator = LuaSys::GetEmulator();
	const std::string gameRoot = pEmulator->GetGlobalConfig()->WorkspaceRoot + pEmulator->GetProjectConfig()->Name + "/";
	const std::string fname = gameRoot + luaL_optstring(pState, 2, "temp.png");
	
	pGraphicsView->SavePNG(fname.c_str());
	return 0;
}

static int DrawOtherGraphicsViewScaled(lua_State *pState)
{
	FGraphicsView* pGraphicsView = (FGraphicsView*)lua_touserdata(pState, 1 );
	if (pGraphicsView == nullptr)
		return 0;

	FGraphicsView* pOtherGraphicsView = (FGraphicsView*)lua_touserdata(pState, 2 );
	if (pOtherGraphicsView == nullptr)
		return 0;

	const int xp = (int)luaL_optinteger(pState,3, 0);
	const int yp = (int)luaL_optinteger(pState,4, 0);
	const int xsize = (int)luaL_optinteger(pState,5, 64);
	const int ysize = (int)luaL_optinteger(pState,6, 64);

	pGraphicsView->DrawOtherGraphicsViewScaled(pOtherGraphicsView, xp, yp, xsize, ysize);
	return 0;
}

static const luaL_Reg corelib[] =
{
	{"print", print},
	// Memory/Machine state
	{"ReadByte", ReadByte},
	{"ReadWord", ReadWord},
	{"WriteByte", WriteByte},
	{"WriteWord", WriteWord},
	{"GetMemPtr", GetMemPtr},
	{"GetRegValue", GetRegValue},
	{"RegisterExecutionHandler", RegisterExecutionHandler},
	{"RemoveExecutionHandler", RemoveExecutionHandler},
	// Analysis
	{"SetEditMode", SetEditMode},
	{"SetDataItemComment", SetDataItemComment},
	{"SetCodeItemComment", SetCodeItemComment},
	{"AddCommentBlock", AddCommentBlock},
	{"SetDataItemDisplayType", SetDataItemDisplayType},
	// Formatting
	{"FormatMemory", FormatMemory},
	{"FormatMemoryAsBitmap", FormatMemoryAsBitmap},
	{"FormatMemoryAsCharMap", FormatMemoryAsCharMap},
	// UI
	{"DrawAddressLabel", DrawAddressLabel},
	//Graphics
	{"GetImageScale", GetImageScale},
	{"ClearGraphicsView", ClearGraphicsView},
	{"DrawGraphicsView", DrawGraphicsView},
	{"SaveGraphicsViewPNG", SaveGraphicsViewPNG},
	{"DrawOtherGraphicsViewScaled", DrawOtherGraphicsViewScaled},

	{NULL, NULL}    // terminator
};

void AddCoreLibLuaDoc(void)
{
	FLuaDocLib& coreLuaDocLib = AddLuaDocLib("Core API");
	LoadLuaDocLibFromJson(coreLuaDocLib, "Lua\\Docs\\LuaCoreAPIDocs.json");
	coreLuaDocLib.Verify(corelib);
}

int luaopen_corelib(lua_State *pState)
{
	lua_getglobal(pState, "_G");
	luaL_setfuncs(pState, corelib, 0);  // for Lua versions 5.2 or greater
	lua_pop(pState, 1);
	return 1;
}

