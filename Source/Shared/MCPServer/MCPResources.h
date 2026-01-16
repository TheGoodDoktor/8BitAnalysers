#pragma once

#include "json.hpp"

class FEmuBase;

class FMCPResource
{
public:
	virtual ~FMCPResource() = default;

	virtual std::string Read(FEmuBase* pEmulator) = 0;

	std::string		URI;
	std::string		Title;
	std::string		Description;
	std::string		MimeType;
	std::string		Category;
};


class FMCPResourceRegistry
{
public:

	FMCPResourceRegistry(FEmuBase* pEmu)
		: pEmulator(pEmu)
	{
	}

	void RegisterResource(FMCPResource* pResource)
	{
		Resources[pResource->URI] = pResource;
	}

	void GenerateResourcesList(nlohmann::json& outResourcesList) const
	{
		for (const auto& pair : Resources)
		{
			nlohmann::json resourceJson;
			FMCPResource& resource = *pair.second;

			resourceJson["uri"] = resource.URI;
			resourceJson["name"] = resource.Title;
			resourceJson["description"] = resource.Description;
			resourceJson["mimeType"] = resource.MimeType;
			outResourcesList.push_back(resourceJson);
		}
	}

	FMCPResource* GetResource(const std::string& name) const
	{
		auto it = Resources.find(name);
		if (it != Resources.end())
		{
			return it->second;
		}
		return nullptr;
	}

	std::string ReadResource(FMCPResource* pResource)
	{
		return pResource->Read(pEmulator);
	}

private:
	FEmuBase* pEmulator = nullptr;
	std::map<std::string, FMCPResource*> Resources;
};
