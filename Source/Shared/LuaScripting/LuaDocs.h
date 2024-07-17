#pragma once

#include <string>
#include <vector>

struct luaL_Reg;

struct FLuaDocFunc
{
	FLuaDocFunc(const char* pName, const char* pSummary, const char* pDescription, std::vector<std::string> args, const char* returns)
		: Name(pName)
		, Summary(pSummary)
		, Description(pDescription) 
		, Args(args)
		, Returns(returns) 
			{}
	std::string Name;
	std::string Summary;
	std::string Description;
	std::string Returns;
	std::vector<std::string> Args;
};

struct FLuaDocLib
{
	FLuaDocLib(const char* pName)
		: Name(pName) {}
	const FLuaDocFunc* GetFunctionByName(const char* pName) const;
	void Verify(const luaL_Reg* pReg) const;
	std::string Name;
	std::vector<FLuaDocFunc> Funcs;
};

void ClearLuaDocs(void);
FLuaDocLib& AddLuaDocLib(const char* pName);

int GetNumLuaDocLibs(void);
FLuaDocLib* GetLuaDocLibFromIndex(int index);

void DrawLuaDocs(void);