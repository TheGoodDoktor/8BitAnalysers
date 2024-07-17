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

// Analysis related

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
	{"GetMemPtr", GetMemPtr},
	// Analysis
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
	FLuaDocLib& luaDocLib = AddLuaDocLib("Core API");

	luaDocLib.Funcs.emplace_back(FLuaDocFunc("print", "Log text to the debug log.", "", { "..."}, ""));

	// Memory/Machine state
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("ReadByte", "Read a byte from memory.", "", { "int address" }, "int value"));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("ReadWord", "Read a word from memory.", "", { "int address" }, "int value"));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("GetMemPtr", "Get pointer at memory address.", "", { "int address" }, "uint8_t* memory"));
	
	// Analysis
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("SetDataItemComment", "Set comment of data item at address.", "", { "int address", "string comment" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("SetCodeItemComment", "Set comment of code item at address.", "", { "int address", "string comment" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("AddCommentBlock", "Set comment block at address.", "", { "int address", "string comment" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("SetDataItemDisplayType", "Set the display type of the item at address.", "See LuaBase.lua for supported enums.", { "int address", "EDataItemDisplayType type" }, ""));

	// Formatting
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("FormatMemory", "Format memory at address, using a table to specify formatting options",
		"Table fields\n\n"
		"DataType : int (EDataType)*\n"
		"DisplayType : int (EDataItemDisplayType)*\n"
		"StartAddress : int\n"
		"NoItems : int\n"
		"ItemSize : int\n"
		"CharacterSet : int\n"
		"GraphicsSetRef : int\n"
		"PaletteNo : int\n"
		"StructId : int\n"
		"RegisterItem : bool\n"
		"ClearCodeInfo : bool\n"
		"ClearLabels : bool\n"
		"AddLabelAtStart : bool\n"
		"LabelName : string\n"
		"AddCommentAtStart : bool\n"
		"CommentText : string\n\n"
		"* See LuaBase.lua for enums\n\n"
		"See the Format tab in the Code Analysis view for an idea of how this works. It follows the same procedure.", { "{table}" }, ""));

	luaDocLib.Funcs.emplace_back(FLuaDocFunc("FormatMemoryAsBitmap", "Format a region of memory as a bitmap.", "", { "int address", "int width", "int height", "int bpp" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("FormatMemoryAsCharMap", "Format a region of memory as a character map.", "", { "int address", "int width", "int height" }, ""));
	
	// UI
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("DrawAddressLabel", "Draw a summary of a given address.", "", { "int address" }, ""));
	
	//Graphics
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("GetImageScale", "Get scaling value. TODO", "", { }, "float scale"));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("ClearGraphicsView", "Fill graphics view with a single colour.", "", { "GraphicsView view", "int colour" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("DrawGraphicsView", "Draw the contents of a graphics view.", "", { "GraphicsView view" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("SaveGraphicsViewPNG", "Save graphics view to png file.", "", { "GraphicsView view", "string path" }, ""));
	luaDocLib.Funcs.emplace_back(FLuaDocFunc("DrawOtherGraphicsViewScaled", "TODO", "", { "GraphicsView view", "GraphicsView otherView", "int xPos", "int yPos", "int width", "int height" }, ""));

	luaDocLib.Verify(corelib);
}

int luaopen_corelib(lua_State *pState)
{
	lua_getglobal(pState, "_G");
	luaL_setfuncs(pState, corelib, 0);  // for Lua versions 5.2 or greater
	lua_pop(pState, 1);
	return 1;
}

