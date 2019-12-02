#include "GameConfig.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "Util/FileUtil.h"

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname)
{
	rapidjson::Document configJson;
	auto &allocator = configJson.GetAllocator();

	configJson.SetObject();
	configJson.AddMember("Name", rapidjson::StringRef(config.Name.c_str()), allocator);
	configJson.AddMember("Z80File", rapidjson::StringRef(config.Z80file.c_str()), allocator);

	rapidjson::Value spriteConfigs;
	spriteConfigs.SetArray();

	for(const auto&sprConfigIt : config.SpriteConfigs)
	{
		rapidjson::Value sprConfigJson;
		sprConfigJson.SetObject();
		sprConfigJson.AddMember("Name", rapidjson::StringRef(sprConfigIt.first.c_str()), allocator);

		char hexStr[16];
		sprintf(hexStr,"0x%x", sprConfigIt.second.BaseAddress);
		rapidjson::Value hexVal;
		hexVal.SetString(hexStr, allocator);
		sprConfigJson.AddMember("BaseAddress", hexVal, allocator);
		
		sprConfigJson.AddMember("Count", sprConfigIt.second.Count, allocator);
		sprConfigJson.AddMember("Width", sprConfigIt.second.Width, allocator);
		sprConfigJson.AddMember("Height", sprConfigIt.second.Height, allocator);

		spriteConfigs.PushBack(sprConfigJson,allocator);
	}

	configJson.AddMember("SpriteConfigs", spriteConfigs, allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	configJson.Accept(writer);

	FILE *fp = fopen(fname, "wt");
	if (fp == nullptr)
		return false;
	fwrite(buffer.GetString(), 1, buffer.GetSize(), fp);
	fclose(fp);

	return true;
}

bool LoadGameConfigFromFile(FGameConfig &config, const char *fname)
{
	// load text file
	FILE *fp = fopen(fname, "rt");
	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	const size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *pText = (char *)malloc(fileSize);
	fread(pText, 1, fileSize, fp);
	fclose(fp);

	// parse json
	rapidjson::Document jsonConfig;
	jsonConfig.Parse(pText);

	config.Name = jsonConfig["Name"].GetString();
	config.Z80file = jsonConfig["Z80File"].GetString();

	if (jsonConfig.HasMember("SpriteConfigs"))
	{
		for (const auto& jsonSprConfig : jsonConfig["SpriteConfigs"].GetArray())
		{
			const char* pName = jsonSprConfig["Name"].GetString();
			FSpriteDefConfig &sprConfig = config.SpriteConfigs[pName];
			int baseAddress = 0;
			sscanf(jsonSprConfig["BaseAddress"].GetString(), "0x%x", &baseAddress);
			sprConfig.BaseAddress = (uint16_t)baseAddress;
			sprConfig.Count = jsonSprConfig["Count"].GetInt();
			sprConfig.Width = jsonSprConfig["Width"].GetInt();
			sprConfig.Height = jsonSprConfig["Height"].GetInt();
			
		}
	}

	free(pText);
	return true;
}

bool LoadGameConfigs(FSpeccyUI *pUI)
{
	FDirFileList listing;

	if (EnumerateDirectory("Configs", listing) == false)
		return false;

	for (const auto &file : listing)
	{
		const std::string &fn = "configs/" + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FGameConfig newConfig;
			LoadGameConfigFromFile(newConfig, fn.c_str());
		}
	}
	return true;
}