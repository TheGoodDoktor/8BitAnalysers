#include "LuaDocs.h"

#include <lauxlib.h>
#include <imgui.h>

#include "Debug/DebugLog.h"

std::vector<FLuaDocLib> gLuaDocLibs;

const FLuaDocFunc* FLuaDocLib::GetFunctionByName(const char* pName) const
{
	for (const FLuaDocFunc& func : Funcs)
	{
		if (func.Name == pName)
			return &func;
	}
	return nullptr;
}

#ifndef NDEBUG
void FLuaDocLib::Verify(const luaL_Reg* pReg) const
{
	// Go through all lua libs and see if we have documentation for all the functions.
	for (int i = 0; pReg[i].name != nullptr; i++)
	{
		if (GetFunctionByName(pReg[i].name) == nullptr)
			LOGWARNING("Lua func '%s' for lib '%s' not documented", pReg[i].name, Name.c_str());
	}
}
#endif

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
						ImGui::EndTable();
						ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() / 2.0f));
					}
				}
			}
		}
	}
	ImGui::End();

}
