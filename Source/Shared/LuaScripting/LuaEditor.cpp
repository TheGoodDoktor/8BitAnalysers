#include "LuaEditor.h"

#include "LuaSys.h"

#include "Util/FileUtil.h"
#include "Misc/EmuBase.h"
#include "Misc/GameConfig.h"

#include <vector>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::vector<FLuaTextEditor> TextEditors;

std::vector<std::string> TemplateFiles;
std::string SelectedTemplate = "None";
std::string NewFilenameTxt;
bool bSaveBeforeReload = true;

bool EnumerateTemplates()
{
	FDirFileList listing;

	TemplateFiles.clear();

	if (EnumerateDirectory(GetBundlePath("Lua/Templates"), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		if (file.FileType == FDirEntry::File)
		{
			const std::string filename = file.FileName;
			const std::string extension = filename.substr(filename.find_last_of(".") + 1);;

			if (extension == "lua")
			{
				TemplateFiles.push_back(filename);
			}
		}
	}
	return true;
}

bool InitTextEditors(void)
{
	EnumerateTemplates();
	TextEditors.clear();
	return true;
}

bool CreateNewLuaFileFromTemplate(const char* pFileName, const char* pTemplateFilename)
{
	if (FileExists(pFileName))	// does the file already exist? - don't overwrite it!
		return false;

	char* pTextData = LoadTextFile(pTemplateFilename);	// load template
	if (pTextData != nullptr)
	{
		LuaSys::ExecuteString(pTextData);
	}
	
	FLuaTextEditor& editor = AddTextEditor(pFileName, pTextData);
	if (pTextData != nullptr)
		delete pTextData;

	return SaveTextFile(editor.SourceFileName.c_str(), editor.LuaTextEditor.GetText().c_str());	// Save new file
}

FLuaTextEditor& AddTextEditor(const char* pFileName, const char* pTextData)
{
	FLuaTextEditor& editor = TextEditors.emplace_back();

	editor.SourceName = GetFileFromPath(pFileName);
	editor.SourceFileName = std::string(pFileName);

	// set up text editor
	auto lang = TextEditor::LanguageDefinition::Lua();
	editor.LuaTextEditor.SetLanguageDefinition(lang);
	if(pTextData != nullptr)
		editor.LuaTextEditor.SetText(pTextData);

	return editor;
}

void DrawTextEditor(void)
{
	FEmuBase* pEmulator = LuaSys::GetEmulator();

	if (ImGui::Begin("Lua Editor"))
	{
		if (ImGui::Button("Reload Scripts"))
		{
			// save all - you might not want to do this if you're using an external editor
			if(bSaveBeforeReload)
			{ 
				for (auto& editor : TextEditors)
				{
					// Note: I think the GetText() function adds a newline character to the file
					std::string saveText = editor.LuaTextEditor.GetText();
					if(saveText[saveText.size() - 1] == '\n')	// remove newline added to end
						saveText[saveText.size() - 1] = 0;
					SaveTextFile(editor.SourceFileName.c_str(), saveText.c_str());
				}
			}
			pEmulator->LoadLua();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Save before reload",&bSaveBeforeReload);
		ImGui::SameLine();
		if (ImGui::Button("Generate globals file"))
		{
			LuaSys::ExportGlobalLabels();
		}

		ImGui::SetNextItemWidth(300);
		ImGui::InputText("Filename",&NewFilenameTxt);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(250);
		if(ImGui::BeginCombo("Template",SelectedTemplate.c_str()))
		{
			if(ImGui::Selectable("None", SelectedTemplate == "None"))
				SelectedTemplate = "None";
			for (const auto& luaTemplate : TemplateFiles)
			{
				if(ImGui::Selectable(luaTemplate.c_str(), SelectedTemplate == luaTemplate))
					SelectedTemplate = luaTemplate;
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Create New Lua File"))
		{
			// Ensure that NewFilename has a .lua extension
			const std::string saveFilename = RemoveFileExtension(NewFilenameTxt.c_str()) + ".lua";
			// Create Lua file
			const std::string gameRoot = pEmulator->GetGlobalConfig()->WorkspaceRoot + pEmulator->GetProjectConfig()->Name + "/";
			std::string luaScriptFName = gameRoot + saveFilename;
			std::string templateFilename = "Lua/Templates/" + SelectedTemplate;
			if (CreateNewLuaFileFromTemplate(luaScriptFName.c_str(), GetBundlePath(templateFilename.c_str())))
			{
				// bit of a hack - need better access implementation
				const_cast<FProjectConfig *>(pEmulator->GetProjectConfig())->AddLuaSourceFile(saveFilename.c_str());
			}
		}

		// This is very hard coded, we probably want a list of templates etc. somewhere
		/*if (ImGui::Button("Create Viewer Script"))
		{
			// Create Viewer Lua file
			const std::string gameRoot = pEmulator->GetGlobalConfig()->WorkspaceRoot + pEmulator->GetGameConfig()->Name + "/";
			std::string luaScriptFName = gameRoot + "ViewerScript.lua";
			CreateNewLuaFileFromTemplate(luaScriptFName.c_str(), GetBundlePath("Lua/ZXViewerTemplate.lua"));
		}*/

		if (ImGui::BeginTabBar("Editor Tabs"))
		{
			for (auto& editor : TextEditors)
			{
				const bool bTabOpen = ImGui::BeginTabItem(editor.SourceName.c_str());

				if (ImGui::IsItemHovered())  // full filename on tooltip
				{
					ImGui::BeginTooltip();
					ImGui::Text("%s", editor.SourceFileName.c_str());
					ImGui::EndTooltip();
				}

				if (bTabOpen)
				{
				/*
					if (ImGui::Button("Update"))
					{
						LuaSys::OutputDebugString("Updating script: %s", editor.SourceName.c_str());
						LuaSys::ExecuteString(editor.LuaTextEditor.GetText().c_str());
					}
					ImGui::SameLine();
					if (ImGui::Button("Save"))
					{
						SaveTextFile(editor.SourceFileName.c_str(), editor.LuaTextEditor.GetText().c_str());
					}*/
					editor.LuaTextEditor.Render(editor.SourceName.c_str());
					ImGui::EndTabItem();
				}

			}
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}
