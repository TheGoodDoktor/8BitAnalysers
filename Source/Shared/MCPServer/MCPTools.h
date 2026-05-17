#pragma once

#include "json.hpp"

class FEmuBase;

class FMCPTool
{
public:
	virtual ~FMCPTool() = default;

	uint32_t GetNumericalArgument(const char* argName, const nlohmann::json& arguments, uint32_t defaultValue = 0)
	{
		if (!arguments.contains(argName))
			return defaultValue;

		const auto& arg = arguments[argName];

		if (arg.is_number())
			return arg.get<uint32_t>();

		if (arg.is_string())
		{
			try
			{
				std::string strValue = arg.get<std::string>();
				// base 0: auto-detect hex (0x prefix) or decimal
				return static_cast<uint32_t>(std::stoul(strValue, nullptr, 0));
			}
			catch (const std::exception&)
			{
				return defaultValue;
			}
		}

		return defaultValue;
	}

	virtual nlohmann::json Execute(FEmuBase* pEmulator, const nlohmann::json& arguments) = 0;

	std::string		Description;
	nlohmann::json	InputSchema;
};

class FMCPToolsRegistry
{
public:

	FMCPToolsRegistry(FEmuBase* pEmu)
		: pEmulator(pEmu)
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
			try
			{
				outResult = Tool->Execute(pEmulator, arguments);
			}
			catch (const std::exception& e)
			{
				outResult = { {"error", std::string("Tool execution error: ") + e.what()} };
			}
			catch (...)
			{
				outResult = { {"error", "Tool execution error: unknown exception"} };
			}
			return true;
		}
		return false;
	}

private:
	FEmuBase* pEmulator = nullptr;
	std::unordered_map<std::string, FMCPTool*>	Tools;
};

void RegisterBaseTools(FMCPToolsRegistry& Registry);