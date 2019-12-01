#include "GameConfig.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

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

		uint16_t	BaseAddress;
		int			Count;
		int			Width;
		int			Height;

		spriteConfigs.PushBack(sprConfigJson,allocator);
	}

	configJson.AddMember("SpriteConfigs", spriteConfigs, allocator);
	return true;
}

bool LoadGameConfigFromFile(FGameConfig &config, const char *fname)
{

	return true;
}