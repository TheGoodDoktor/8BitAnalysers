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
int gSelectedFunctionIndex = 0;

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
		std::vector<std::string> args;

		if (funcJson.contains("Args"))
		{
			for (const auto& argJson : funcJson["Args"])
			{
				args.push_back(argJson);
			}
		}

		Funcs.emplace_back(FLuaDocFunc(name.c_str(), summary.c_str(), description.c_str(), args, returns.c_str(), usage.c_str()));
	}
}

const FLuaDocFunc* FLuaDocLib::GetFunctionByName(const char* pName) const
{
	for (const FLuaDocFunc& func : Funcs)
	{
		if (func.Name == pName)
			return &func;
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
#ifndef NDEBUG
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
		const float glyphWidth = ImGui_GetFontCharWidth();
		ImGui::BeginChild("##LuaDocsFunctionList", ImVec2(glyphWidth * 30.f, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		
		int curIndex = 0;
		for (const FLuaDocLib& lib : gLuaDocLibs)
		{
			if (ImGui::CollapsingHeader(lib.Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (const FLuaDocFunc& func : lib.Funcs)
				{
					if (ImGui::Selectable(func.Name.c_str(), gSelectedFunctionIndex == curIndex))
						gSelectedFunctionIndex = curIndex;
					curIndex++;
				}
			}
		}
			
		ImGui::EndChild();
		ImGui::SameLine();

		if (ImGui::BeginChild("##LuaDocsFunctionDetails"))
		{
			curIndex = 0;

			for (const FLuaDocLib& lib : gLuaDocLibs)
			{
				static ImGuiTableFlags flags = ImGuiTableFlags_Borders;

				for (const FLuaDocFunc& func : lib.Funcs)
				{
					if (curIndex == gSelectedFunctionIndex)
					{
						if (ImGui::BeginTable("luadoctable", 1, flags))
						{
							ImGui::TableSetupColumn(func.Definition.c_str());
							ImGui::TableHeadersRow();

							// Summary & Description
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::TextWrapped(func.Summary.c_str());
							if (!func.Description.empty())
							{
								ImGui::Text("");
								ImGui::TextWrapped(func.Description.c_str());
							}
							ImGui::Spacing();

							// Arguments
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Arguments");
							if (func.Args.empty())
							{
								ImGui::BulletText("None");
							}
							else
							{
								for (const std::string& arg : func.Args)
								{
									ImGui::BulletText(arg.c_str());
								}
							}
							ImGui::Spacing();

							// Returns
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Returns");

							if (func.Returns.empty())
							{
								ImGui::BulletText("Nothing");
							}
							else
							{
								ImGui::BulletText(func.Returns.c_str());
							}
							ImGui::Spacing();

							// Usage
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Usage");
							ImGui::Spacing();
							ImGui::Indent();
							ImGui::TextWrapped(func.Usage.c_str());

							if (ImGui::BeginPopupContextItem("doc usage menu"))
							{
								if (ImGui::Selectable("Copy Text"))
								{
									ImGui::SetClipboardText(func.Usage.c_str());
								}
								ImGui::EndPopup();
							}

							ImGui::Unindent();
							ImGui::Spacing();

							ImGui::EndTable();
						}
					}
					curIndex++;
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

	int curIndex = 0;
	for (const FLuaDocLib& lib : gLuaDocLibs)
	{
		for (const FLuaDocFunc& func : lib.Funcs)
		{
			if (func.Name == pName)
			{
				gSelectedFunctionIndex = curIndex;
				return;
			}
			curIndex++;
		}
	}
}