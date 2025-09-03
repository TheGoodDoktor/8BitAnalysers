#include "LuaSys.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaCoreAPI.h"
#include "LuaConsole.h"
#include "LuaEditor.h"
#include "LuaDocs.h"

#include "Misc/EmuBase.h"
#include "Misc/GlobalConfig.h"
#include "Util/FileUtil.h"

#include "Debug/DebugLog.h"

#include <Misc/GlobalConfig.h>
#include <Misc/GameConfig.h>

#include <CodeAnalyser/UI/GraphicsViewer.h>

// For the imgui bindings
extern lua_State* lState;
extern void LoadImguiBindings();

namespace LuaSys
{

static bool g_EnableExecutionHandlers = true;
static std::unordered_map<uint16_t, std::string>	g_ExecutionHandlers;

FLuaScopeCheck::FLuaScopeCheck(lua_State* pState):LuaState(pState)
{
	InitialStackItems = lua_gettop(LuaState);
}

FLuaScopeCheck::~FLuaScopeCheck()
{
	if(InitialStackItems != lua_gettop(LuaState))
	{
		LOGWARNING("Lua stack imbalance now:%d, should be:%d",lua_gettop(LuaState),InitialStackItems);
		LOGWARNING("Stack:");
		DumpStack(LuaState);
	}
}

// Globals
lua_State*	GlobalState = nullptr;
FLuaConsole LuaConsole;
FEmuBase*   EmuBase = nullptr;

void lua_warning_function(void *ud, const char *msg, int tocont)
{
	LuaConsole.AddLog("%s",msg);
}


bool Init(FEmuBase* pEmulator)
{
	if(GlobalState != nullptr)  // shutdown old instance
		Shutdown();
	
	InitTextEditors();

	for (int i = 0; i < GetNumLuaDocLibs(); i++)
	{
		for (const FLuaDocFunc* pFunc : GetLuaDocLibFromIndex(i)->Funcs)
		{
			if (pFunc)
			{
				std::string toolTip = pFunc->Definition + "\n\n" + pFunc->Summary;
				RegisterLuaFunctionToolTip(pFunc->Name.c_str(), toolTip.c_str());
			}
		}
	}

	lua_State* pState = luaL_newstate();	// create the global state

	// Add system libraries
	luaL_openlibs(pState);		// opens the basic library

	// Add our libraries
	luaopen_corelib(pState);
	
	lua_setwarnf(pState,lua_warning_function,nullptr);

	GlobalState = pState;
	EmuBase = pEmulator;
	
	for(const auto& luaFile : EmuBase->GetGlobalConfig()->LuaBaseFiles)
		LoadFile(GetBundlePath(luaFile.c_str()), EmuBase->GetGlobalConfig()->bEditLuaBaseFiles);

	lState = GlobalState;
	LoadImguiBindings();

	//ExportGlobalLabels();	// might have this on a button if frequently updating proves to be problematic

	LoadGlobals(pEmulator);
	return true;
}

bool LoadGlobals(FEmuBase* pEmulator)
{
	// Load globals
	const std::string gameRoot = EmuBase->GetGlobalConfig()->WorkspaceRoot + EmuBase->GetProjectConfig()->Name + "/";
	std::string globalsFName = gameRoot + "Globals.lua";
	return LoadFile(globalsFName.c_str(), true);
}

void Shutdown(void)
{
	if (GlobalState)
		lua_close(GlobalState);
	GlobalState = nullptr;

	FCodeAnalysisState& state = EmuBase->GetCodeAnalysis();

	for (auto& handler : g_ExecutionHandlers)
	{
		FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(handler.first);
		if(pCodeInfo != nullptr)
			pCodeInfo->bHasLuaHandler = false;
	}
	g_ExecutionHandlers.clear();
}

lua_State*  GetGlobalState()
{
	return GlobalState;
}

bool LoadFile(const char* pFileName, bool bAddEditor)
{
	if (GlobalState == nullptr)
		return false;

	char* pTextData = LoadTextFile(pFileName);
	if(pTextData != nullptr)
	{
		lua_State* pState = GlobalState;

		const int ret = luaL_dostring(pState, pTextData);

		if (ret != LUA_OK)
		{
			OutputDebugString("%s:[error] %s", pFileName, lua_tostring(pState, -1));
			lua_pop(pState, 1); // pop error message
		}

		if(bAddEditor)
			AddTextEditor(pFileName, pTextData);
		delete pTextData;
		return true;
	}
	return false;
	
	const int ret = luaL_dofile(GlobalState, pFileName);
	
	if (ret == LUA_OK)
	{
		return true;
	}
	
	LOGWARNING("Lua script %s not found",pFileName);
	return false;
}

void ExecuteString(const char *pString)
{
	if (GlobalState == nullptr)
		return;

	//luaL_dostring(GlobalState, pString);
	lua_State* pState = GlobalState;
	
	const int ret = luaL_dostring(pState, pString);

	if (ret != LUA_OK)
	{
		OutputDebugString("[error] %s", lua_tostring(pState, -1));
		lua_pop(pState, 1); // pop error message
	}
}

void OutputDebugString(const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	LuaConsole.AddLog("%s",buf);
	LOGDEBUG("%s",buf);
}

void CallFunction(const char* pFunctionName)
{
	if (GlobalState == nullptr)
		return;

	lua_State* pState = GlobalState;

	lua_getglobal(pState, pFunctionName);
	if (lua_isfunction(pState, -1))
	{
		if (lua_pcall(pState, 0, 0, 0) != LUA_OK)
		{
		}
	}
}



void RegisterExecutionHandler(uint16_t address, const char* functionName)
{
	FCodeAnalysisState& state = EmuBase->GetCodeAnalysis();

	FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(address);
	pCodeInfo->bHasLuaHandler = true;
	g_ExecutionHandlers[address] = functionName;
}

void RemoveExecutionHandler(uint16_t address)
{
	FCodeAnalysisState& state = EmuBase->GetCodeAnalysis();

	auto findIt = g_ExecutionHandlers.find(address);
	if (findIt != g_ExecutionHandlers.end())
	{
		FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(address);
		pCodeInfo->bHasLuaHandler = false;
		g_ExecutionHandlers.erase(findIt);
	}
}

bool OnInstructionExecuted(uint16_t pc)
{
	if(g_EnableExecutionHandlers == false)
		return false;

	if (GlobalState == nullptr)
		return false;

	// look up execution handler
	auto handlerIt = g_ExecutionHandlers.find(pc);
	if(handlerIt == g_ExecutionHandlers.end())
	{
		// no handler found - remove flag
		FCodeAnalysisState& state = EmuBase->GetCodeAnalysis();
		FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(pc);
		pCodeInfo->bHasLuaHandler = false;
		return false;
	}

	// Call execution handler
	const char* pFunctionName = handlerIt->second.c_str();
	lua_State* pState = GlobalState;

	lua_getglobal(pState, pFunctionName);
	if (lua_isfunction(pState, -1))
	{
		if (lua_pcall(pState, 0, 1, 0) == LUA_OK)
		{
			if (!lua_isboolean(pState, -1))
				return false;
			else
				return lua_toboolean(pState,-1);
		}
	}

	return false;
}

// Offscreen Buffers
static const char* kOSBHandlersTable = "OffScreenBufferHandlers";
static const char* kDrawFunctionName = "Draw";
static const char* kGetAddressOffsetFunctionName = "GetAddressOffsetFromPos";

// Draw ImGui Combo Box for all entries in kOSBHandlersTable
void DrawOSBHandlersComboBox(std::string& HandlerName)
{
	const char* kDefaultHandler = "Default";
	lua_State* pState = GlobalState;

	FLuaScopeCheck StackCheck(pState);

	lua_getglobal(pState, kOSBHandlersTable);
	if (lua_istable(pState, -1) == false)
		return;

	if(ImGui::BeginCombo("OSB Handlers", HandlerName.c_str()))
	{
		if (ImGui::Selectable(kDefaultHandler, HandlerName == kDefaultHandler))	// default Handler
			HandlerName = kDefaultHandler;
		// iterate through each key in table

		// Push the first key
		lua_pushnil(pState);

		// Iterate through the table
		while (lua_next(pState, -2) != 0)
		{
			// Key is at index -2 and value is at index -1
			// Do something with the key and value
			if (lua_isstring(pState, -2))
			{
				const char* key = lua_tostring(pState, -2);
				if (ImGui::Selectable(key, HandlerName == key))
					HandlerName = key;
				//printf("Key: %s\n", key);
			}

			// Remove the value, keep the key for the next iteration
			lua_pop(pState, 1);
		}

		ImGui::EndCombo();
	}

	lua_pop(pState, 1);	// pop table
}

// Get OSB hanndler from OSB handlers table with name pHandlerTableName
bool GetOSBHandlerTable(const char* pHandlerTableName)
{
	lua_State* pState = GlobalState;

	lua_getglobal(pState, kOSBHandlersTable);

	// check if we have the handler table
	if (lua_istable(pState, -1) == false)
	{
		lua_pop(pState, 1);	// balance stack
		return false;
	}

	// Get the handler entry in the table with the name in buffer.LuaHandlerName
	lua_pushstring(pState, pHandlerTableName);
	lua_gettable(pState, -2);
	if (lua_istable(pState, -1) == false)
	{
		lua_pop(pState, 1);	// balance stack
		return false;
	}

	return true;
}

bool DrawOffScreenBuffer(const FOffScreenBuffer& buffer, FGraphicsView* pView)
{
	lua_State* pState = GlobalState;

	if (buffer.LuaHandlerName.empty())
		return false;

	if (GetOSBHandlerTable(buffer.LuaHandlerName.c_str()) == false)
		return false;
	
	// Get 'Draw' function from table
	lua_pushstring(pState, kDrawFunctionName);
	lua_gettable(pState, -2);
	if (lua_isfunction(pState, -1) == false)
	{
		lua_pop(pState, 1);	// balance stack
		return false;
	}

	// call 'Draw' function
	lua_pushlightuserdata(pState, pView);
	lua_pushinteger(pState, buffer.Address.GetAddress());
	lua_pushnumber(pState, buffer.XSizePixels);
	lua_pushnumber(pState, buffer.YSizePixels);
	if (lua_pcall(pState, 4, 0, 0) == LUA_OK)
	{
		return true;
	}
	return false;
}

uint16_t GetAddressOffsetFromPositionInBuffer(const FOffScreenBuffer& buffer, int x, int y)
{
	lua_State* pState = GlobalState;

	if(buffer.LuaHandlerName.empty())
		return 0xffff;

	if (GetOSBHandlerTable(buffer.LuaHandlerName.c_str()) == false)
		return 0xffff;

	// Get 'GetAddressOffset' function from table
	lua_pushstring(pState, kGetAddressOffsetFunctionName);
	lua_gettable(pState, -2);
	if (lua_isfunction(pState, -1) == false)
	{
		lua_pop(pState, 1);	// balance stack
		return 0xffff;
	}

	// Call function
	lua_pushnumber(pState, x);
	lua_pushnumber(pState, y);
	lua_pushnumber(pState, buffer.XSizePixels);
	lua_pushnumber(pState, buffer.YSizePixels);

	if (lua_pcall(pState, 4, 1, 0) == LUA_OK)
	{
		if (!lua_isinteger(pState, -1))
			return 0xffff;
		else
			return lua_isinteger(pState, -1);
	}

	return 0xffff;
}


bool GetPositionInBufferFromAddress(const FOffScreenBuffer& buffer, FAddressRef address, int& x, int& y)
{
	if (buffer.LuaHandlerName.empty())
		return false;

	// TODO:
	return false;
}

bool OnEmulatorScreenDrawn(float x, float y, float scale)
{
	if (GlobalState == nullptr)
		return false;

	lua_State* pState = GlobalState;

	lua_getglobal(pState, "OnScreenDraw");
	if (lua_isfunction(pState, -1))
	{
		lua_pushnumber(pState, x);
		lua_pushnumber(pState, y);
		lua_pushnumber(pState, scale);
		if (lua_pcall(pState, 3, 0, 0) != LUA_OK)
		{
			OutputDebugString("[error] %s", lua_tostring(pState, -1));
			lua_pop(pState, 1); // pop error message
			return false;
		}
		return true;
	}
	else
	{
		lua_pop(pState, 1);
	}

	return false;
}


FLuaConsole* GetLuaConsole()
{
	return &LuaConsole;
}

FEmuBase* GetEmulator()
{
	return EmuBase;
}

void DrawViewerTab(lua_State* pState)
{
	FLuaScopeCheck StackCheck(pState);
	//DumpStack(pState);
	if(!lua_istable(pState, -1))
		return;
	
	lua_pushstring(pState,"name");
	lua_gettable(pState, -2);
	if(lua_isstring(pState, -1))
	{
		if(ImGui::BeginTabItem(lua_tostring(pState, -1)))
		{
			lua_pushvalue(pState,-2);
			lua_pushstring(pState, "onDrawUI");
			lua_gettable(pState, -2);
			if(lua_isfunction(pState, -1))
			{
				lua_pushvalue(pState, -2);  // get table as argument for function
				//DumpStack(pState);
				if(lua_pcall(pState, 1, 0, 0) != LUA_OK)
				{
					OutputDebugString("Error calling 'onDraw' function for viewer: %s", lua_tostring(pState, -1));
					DumpStack(pState);
					lua_pop(pState,1); // pop error
				}
			}
			lua_pop(pState,1);  // onDraw function & table copy
		
			ImGui::EndTabItem();
		}
	}
	lua_pop(pState,1);  // name string
	
	//DumpStack(pState);
}

void DrawUI()
{
	if(GlobalState == nullptr)
		return;
	
	static bool bOpen = true;
	LuaConsole.Draw("Lua Console", &bOpen);
	
	lua_State* pState = GlobalState;

	// scope block for stack check
	{
		FLuaScopeCheck StackCheck(pState);
   
		lua_getglobal(pState, "Viewers");
		
		if(lua_istable(pState,-1) && lua_rawlen(pState, -1) > 0)
		{
			if(ImGui::Begin("Game Viewers"))
			{
				if(ImGui::BeginTabBar("Viewer Tabs"))
				{
					const int itemCount = (int)lua_rawlen(pState, -1);
					for(int i=0;i<itemCount;i++)
					{
						lua_pushnumber(pState, i+1);// indexes start at 1
						lua_gettable(pState,-2);
						DrawViewerTab(pState);
						lua_pop(pState,1);  // pop item
					}
				
					ImGui::EndTabBar();
			   }
			}
			ImGui::End();
		}
		lua_pop(pState,1);
	}

	DrawTextEditor();

	DrawLuaDocs();
}

void DumpStack(lua_State *L)
{
	const int top = lua_gettop(L);
	const int kValueStrSize = 32;
	char valueStr[kValueStrSize];
	
	for (int i = 1; i <= top; i++)
	{
		const int t = lua_type(L, i);
	   
		switch (t)
		{
			case LUA_TSTRING:  /* strings */
				snprintf(valueStr,kValueStrSize,"`%s'", lua_tostring(L, i));
				//OutputDebugString("`%s'", lua_tostring(L, i));
				break;

			case LUA_TBOOLEAN:  /* booleans */
				//OutputDebugString(lua_toboolean(L, i) ? "true" : "false");
				snprintf(valueStr,kValueStrSize,"%s",lua_toboolean(L, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:  /* numbers */
				snprintf(valueStr,kValueStrSize,"%g", lua_tonumber(L, i));
				break;

			default:  /* other values */
				snprintf(valueStr,kValueStrSize,"");
				//OutputDebugString("%s", lua_typename(L, t));
				break;

		}
		OutputDebugString("%d(%d) : %s %s",i, -1 - (top - i),lua_typename(L, t),valueStr);
		//OutputDebugString("  ");  /* put a separator */
	}
	//OutputDebugString("\n");  /* end the listing */
}

bool ExportGlobalLabels()
{
	if(EmuBase == nullptr)
		return false;

	const FCodeAnalysisState& state = EmuBase->GetCodeAnalysis();

	std::string outputStr;

	outputStr += "-- Auto generated global labels file for " + EmuBase->GetProjectConfig()->Name + "\n";
	outputStr += "globals = {\n";

	for (const auto& global : state.GlobalDataItems)
	{
		// Skip Machine ROM labels
		const FCodeAnalysisBank* pBank = state.GetBank(global.AddressRef.GetBankId());
		if (pBank)
		{
			if (pBank->bMachineROM)
				continue;
		}

		const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(global.Item);
		const int kLabelStringLength = 128;
		char labelString[kLabelStringLength];
		snprintf(labelString,kLabelStringLength,"\t%s = 0x%X, \n", pLabelInfo->GetName(),global.AddressRef.GetAddress());

		outputStr+=std::string(labelString);
	}
	outputStr += "}";

	const std::string gameRoot = EmuBase->GetGlobalConfig()->WorkspaceRoot + EmuBase->GetProjectConfig()->Name + "/";
	std::string luaScriptFName = gameRoot + "Globals.lua";

	if( SaveTextFile(luaScriptFName.c_str(),outputStr.c_str()) == true)
	{
		LoadGlobals(EmuBase);	// reload globals file
		return true;
	}

	LOGERROR("Failed to save globals file %s", luaScriptFName.c_str());
	return false;
}

}//namespace LuaSys

