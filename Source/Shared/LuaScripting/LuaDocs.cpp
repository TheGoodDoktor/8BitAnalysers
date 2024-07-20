#include "LuaDocs.h"

#include <lauxlib.h>
#include <imgui.h>

#include "Debug/DebugLog.h"
#include "json.hpp"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

std::vector<FLuaDocLib> gLuaDocLibs;

void FLuaDocLib::LoadFromJson(const nlohmann::json& jsonDoc)
{
	for (const auto& funcJson : jsonDoc["Functions"])
	{
		FLuaDocFunc func;
		func.Name = funcJson["Name"].get<std::string>();
		func.Summary = funcJson["Summary"].get<std::string>();
		func.Description = funcJson["Description"].get<std::string>();
		func.Returns = funcJson["Returns"].get<std::string>();
		func.Usage = funcJson["Usage"].get<std::string>();

		if (funcJson.contains("Args"))
		{
			for (const auto& argJson : funcJson["Args"])
			{
				func.Args.push_back(argJson);
			}
		}
		Funcs.emplace_back(func);
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
			LOGWARNING("Lua func '%s' for lib '%s' not documented", pReg[i].name, Name.c_str());
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
		for (const FLuaDocLib& lib : gLuaDocLibs)
		{
			if (ImGui::CollapsingHeader(lib.Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() / 2.0f));
				static ImGuiTableFlags flags = ImGuiTableFlags_Borders /*| ImGuiTableFlags_SizingFixedFit*/;

				for (const FLuaDocFunc& func : lib.Funcs)
				{
					if (ImGui::BeginTable("table1", 1, flags))
					{
						ImGui::TableSetupColumn(func.Name.c_str());
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
						ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() / 2.0f));
					}
				}
			}
		}
	}
	ImGui::End();

}
