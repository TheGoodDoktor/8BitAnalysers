#pragma once

#include <string>
#include <vector>
#include <json_fwd.hpp>

struct luaL_Reg;

/*struct FLuaDocArg
{
	std::string name;
	std::string description;
	std::string defaultValue; // if the argument has a default value then this argument is optional
};*/

struct FLuaDocFunc
{
	FLuaDocFunc(const char* pName, const char* pSummary, const char* pDescription, std::vector<std::string> args, const char* pReturns, const char * pUsage)
		: Name(pName)
		, Summary(pSummary)
		, Description(pDescription) 
		, Args(args)
		, Returns(pReturns)
		, Usage(pUsage)
	{
		MakeDefinition(); 
	}
	void MakeDefinition();

	std::string Name;
	std::string Summary;
	std::string Description;
	std::string Returns;
	std::string Usage;
	std::vector<std::string> Args;

	std::string Definition;
};

struct FLuaDocLib
{
	FLuaDocLib(const char* pName)
		: Name(pName) {}
	const FLuaDocFunc* GetFunctionByName(const char* pName) const;
	void Verify(const luaL_Reg* pReg) const;
	void LoadFromJson(const nlohmann::json& jsonDoc);

	std::string Name;
	std::vector<FLuaDocFunc> Funcs;
};

void ClearLuaDocs(void);
FLuaDocLib& AddLuaDocLib(const char* pName);

bool LoadLuaDocLibFromJson(FLuaDocLib& luaDocLib, const char* fname);
bool SaveLuaDocLibToJson(const FLuaDocLib& luaDocLib, const char* fname);

int GetNumLuaDocLibs(void);
FLuaDocLib* GetLuaDocLibFromIndex(int index);

void DrawLuaDocs(void);
void GoToFunction(const char* pName);