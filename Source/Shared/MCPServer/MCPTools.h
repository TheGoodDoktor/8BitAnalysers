#pragma once

#include "json.hpp"

class FProgramInterface
{
public:

};

class FMCPTool
{
public:
	virtual ~FMCPTool() = default;

	virtual nlohmann::json Execute(FProgramInterface* pProgramIF, const nlohmann::json& arguments) = 0;

	std::string		Description;
	nlohmann::json	InputSchema;
};

class FMCPToolsRegistry
{
public:

	FMCPToolsRegistry(FProgramInterface* pProgramIF)
		: pProgramInterface(pProgramIF)
	{
	}

	void RegisterTool(const std::string& nName, FMCPTool* pTool)
	{
		Tools[nName] = pTool;
	}

	FMCPTool* GetTool(const std::string& name) const
	{
		auto it = Tools.find(name);
		if (it != Tools.end())
		{
			return it->second;
		}
		return nullptr;
	}

	void ForEachTool(const std::function<void(const std::string&, FMCPTool*)>& callback) const
	{
		for (const auto& pair : Tools)
		{
			callback(pair.first, pair.second);
		}
	}

	void GenerateToolsList(nlohmann::json& outToolsList) const
	{
		for (const auto& pair : Tools)
		{
			nlohmann::json toolJson;
			toolJson["name"] = pair.first;
			toolJson["description"] = pair.second->Description;
			toolJson["inputSchema"] = pair.second->InputSchema;

			outToolsList.push_back(toolJson);
		}
	}

	bool ExecuteTool(const std::string& name, const nlohmann::json& arguments, nlohmann::json& outResult) const
	{
		FMCPTool* Tool = GetTool(name);
		if (Tool)
		{
			outResult = Tool->Execute(pProgramInterface, arguments);
			return true;
		}
		return false;
	}

private:
	FProgramInterface* pProgramInterface = nullptr;
	std::unordered_map<std::string, FMCPTool*>	Tools;
};