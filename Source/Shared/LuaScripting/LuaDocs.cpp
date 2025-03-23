#include "LuaDocs.h"

#include <lauxlib.h>
#include <imgui.h>

#include "Debug/DebugLog.h"
#include <ImGuiSupport/ImGuiScaling.h>

#include "json.hpp"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

std::vector<FLuaDocLib> gLuaDocLibs;
int gSelectedFunctionId = 0;
int gCurFunctionId = 0;

void FLuaDocFunc::MakeDefinition()
{
	// Build a string for the function's definition, to save making it every time we want it.
	std::string allArgs;
	for (int i = 0; i < Args.size(); i++)
	{
		allArgs += Args[i];
		if (i < Args.size() - 1)
			allArgs += ", ";
	}

	const int kBufSize = 256;
	char definitionString[kBufSize];
	snprintf(definitionString, kBufSize, "%s%s(%s)", Returns.empty() ? "" : std::string(Returns + " ").c_str(), Name.c_str(), allArgs.c_str());
		
	Definition = definitionString;
}

void FLuaDocLib::LoadFromJson(const nlohmann::json& jsonDoc)
{
	for (const auto& funcJson : jsonDoc["Functions"])
	{
		std::string name = funcJson["Name"].get<std::string>();
		std::string summary = funcJson["Summary"].get<std::string>();
		std::string description = funcJson["Description"].get<std::string>();
		std::string returns = funcJson["Returns"].get<std::string>();
		std::string usage = funcJson["Usage"].get<std::string>();

		std::string group;
		if (funcJson.contains("Group"))
		{
			group = funcJson["Group"].get<std::string>();
		}
		
		std::vector<std::string> args;

		if (funcJson.contains("Args"))
		{
			for (const auto& argJson : funcJson["Args"])
			{
				args.push_back(argJson);
			}
		}

		FLuaDocFunc* pFunc = new FLuaDocFunc(gCurFunctionId, name.c_str(), summary.c_str(), description.c_str(), args, returns.c_str(), usage.c_str());
		Funcs.push_back(pFunc);
		FuncGroups[group].Funcs.push_back(pFunc);
		gCurFunctionId++;
	}

	std::sort(Funcs.begin(), Funcs.end(), [](FLuaDocFunc* pFuncA, FLuaDocFunc* pFuncB)
		{
			return pFuncA->Name < pFuncB->Name;
		});

	// Sort functions in each group
	std::map<std::string, FLuaDocFuncGroup>::iterator it;
	for (it = FuncGroups.begin(); it != FuncGroups.end(); ++it)
	{
		std::sort(it->second.Funcs.begin(), it->second.Funcs.end(), [](FLuaDocFunc* pFuncA, FLuaDocFunc* pFuncB)
			{
				return pFuncA->Name < pFuncB->Name;
			});
	}
}

const FLuaDocFunc* FLuaDocLib::GetFunctionByName(const char* pName) const
{
	for (const FLuaDocFunc* pFunc : Funcs)
	{
		if (pFunc->Name == pName)
			return pFunc;
	}
	return nullptr;
}

void FLuaDocLib::Verify(const luaL_Reg* pReg) const
{
#ifndef NDEBUG
	// Go through all lua libs and see if we have documentation for all the functions.
	for (int i = 0; pReg[i].name != nullptr; i++)
	{
		if (GetFunctionByName(pReg[i].name) == nullptr)
			LOGWARNING("Lua function '%s' for lib '%s' not documented", pReg[i].name, Name.c_str());
	}
#endif
}

bool LoadLuaDocLibFromJson(FLuaDocLib& luaDocLib, const char* fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonFile;

	inFileStream >> jsonFile;
	inFileStream.close();

	luaDocLib.LoadFromJson(jsonFile);

	return true;
}

// This is for development purposes only.
bool SaveLuaDocLibToJson(const FLuaDocLib& luaDocLib, const char* fname)
{
//#ifndef NDEBUG
#if 0
	json jsonFile;

	for (const FLuaDocFunc& func : luaDocLib.Funcs)
	{
		json funcJson;
		funcJson["Name"] = func.Name;
		funcJson["Summary"] = func.Summary;
		funcJson["Description"] = func.Description;
		funcJson["Returns"] = func.Returns;
		funcJson["Usage"] = func.Usage;

		for (const auto& arg : func.Args)
		{
			funcJson["Args"].push_back(arg);
		}
		jsonFile["Functions"].push_back(funcJson);
	}

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonFile << std::endl;
		return true;
	}
#endif
	return false;
}

void ClearLuaDocs(void)
{
	gLuaDocLibs.clear();
}

FLuaDocLib& AddLuaDocLib(const char* pName)
{
	FLuaDocLib lib(pName);
	return gLuaDocLibs.emplace_back(lib);
}

int GetNumLuaDocLibs(void) 
{ 
	return (int)gLuaDocLibs.size(); 
}

FLuaDocLib* GetLuaDocLibFromIndex(int index) 
{
	if (index < gLuaDocLibs.size())
		return &gLuaDocLibs[index];

	return nullptr; 
}

void DrawLuaDocs(void)
{
	if (ImGui::Begin("Lua API Docs"))
	{
		static bool bFlatList = false;
		const float glyphWidth = ImGui_GetFontCharWidth();
		ImGui::BeginChild("##LuaDocsFunctionList", ImVec2(glyphWidth * 30.f, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		ImGui::Checkbox("Flat List", &bFlatList);
		
		int curIndex = 0;
		for (const FLuaDocLib& lib : gLuaDocLibs)
		{
			if (ImGui::CollapsingHeader(lib.Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (bFlatList)
				{
					for (const FLuaDocFunc* pFunc : lib.Funcs)
					{
						if (ImGui::Selectable(pFunc->Name.c_str(), gSelectedFunctionId == pFunc->Id))
							gSelectedFunctionId = pFunc->Id;
					}
				}
				else
				{
					std::map<std::string, FLuaDocFuncGroup>::const_iterator it;
					for (it = lib.FuncGroups.begin(); it != lib.FuncGroups.end(); ++it)
					{
						ImGui::SeparatorText(it->first.c_str());
						{
							for (const FLuaDocFunc* pFunc : it->second.Funcs)
							{
								if (ImGui::Selectable(pFunc->Name.c_str(), gSelectedFunctionId == pFunc->Id))
									gSelectedFunctionId = pFunc->Id;
							}
						}
					}
				}
			}
		}
			
		ImGui::EndChild();
		ImGui::SameLine();

		if (ImGui::BeginChild("##LuaDocsFunctionDetails"))
		{
			for (const FLuaDocLib& lib : gLuaDocLibs)
			{
				static ImGuiTableFlags flags = ImGuiTableFlags_Borders;
				
				for (const FLuaDocFunc* pFunc : lib.Funcs)
				{
					if (pFunc->Id == gSelectedFunctionId)
					{
						if (ImGui::BeginTable("luadoctable", 1, flags))
						{
							ImGui::TableSetupColumn(pFunc->Definition.c_str());
							ImGui::TableHeadersRow();

							// Summary & Description
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::TextWrapped(pFunc->Summary.c_str());
							if (!pFunc->Description.empty())
							{
								ImGui::Text("");
								ImGui::TextWrapped(pFunc->Description.c_str());
							}
							ImGui::Spacing();

							// Arguments
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Arguments");
							if (pFunc->Args.empty())
							{
								ImGui::BulletText("None");
							}
							else
							{
								for (const std::string& arg : pFunc->Args)
								{
									ImGui::BulletText(arg.c_str());
								}
							}
							ImGui::Spacing();

							// Returns
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Returns");

							if (pFunc->Returns.empty())
							{
								ImGui::BulletText("Nothing");
							}
							else
							{
								ImGui::BulletText(pFunc->Returns.c_str());
							}
							ImGui::Spacing();

							// Usage
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Usage");
							ImGui::Spacing();
							ImGui::Indent();
							ImGui::TextWrapped(pFunc->Usage.c_str());

							if (ImGui::BeginPopupContextItem("doc usage menu"))
							{
								if (ImGui::Selectable("Copy Text"))
								{
									ImGui::SetClipboardText(pFunc->Usage.c_str());
								}
								ImGui::EndPopup();
							}

							ImGui::Unindent();
							ImGui::Spacing();

							ImGui::EndTable();
						}
					}
				}
				
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void GoToLuaFunctionDoc(const char* pName)
{
	ImGui::SetWindowFocus("Lua API Docs");
	
	for (const FLuaDocLib& lib : gLuaDocLibs)
	{
		for (const FLuaDocFunc* pFunc : lib.Funcs)
		{
			if (pFunc->Name == pName)
			{
				gSelectedFunctionId = pFunc->Id;
				return;
			}
		}
	}
}