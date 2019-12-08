#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>

#include "SpeccyUI.h"

using json = nlohmann::json;

std::string MakeHexString(uint16_t val)
{
	char hexStr[16];
	sprintf(hexStr, "0x%x", val);
	return std::string(hexStr);
}

uint8_t ParseHexString8bit(const std::string &string)
{
	unsigned int val;
	sscanf(string.c_str(), "0x%x", &val);
	return static_cast<uint8_t>(val);
}

uint16_t ParseHexString16bit(const std::string &string)
{
	unsigned int val;
	sscanf(string.c_str(), "0x%x", &val);
	return static_cast<uint16_t>(val);
}

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname)
{
	json jsonConfigFile;
	jsonConfigFile["Name"] = config.Name;
	jsonConfigFile["Z80File"] = config.Z80File;

	for (const auto&sprConfigIt : config.SpriteConfigs)
	{
		json spriteConfig;
		const FSpriteDefConfig& spriteDefConfig = sprConfigIt.second;
		
		spriteConfig["Name"] = sprConfigIt.first;
		spriteConfig["BaseAddress"] = MakeHexString(spriteDefConfig.BaseAddress);
		spriteConfig["Count"] = spriteDefConfig.Count;
		spriteConfig["Width"] = spriteDefConfig.Width;
		spriteConfig["Height"] = spriteDefConfig.Height;

		jsonConfigFile["SpriteConfigs"].push_back(spriteConfig);
	}

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}


bool LoadGameConfigFromFile(FGameConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	config.Name = jsonConfigFile["Name"].get<std::string>();
	config.Z80File = jsonConfigFile["Z80File"].get<std::string>();

	for(const auto & jsonSprConfig : jsonConfigFile["SpriteConfigs"])
	{
		const std::string &name = jsonSprConfig["Name"].get<std::string>();
		
		FSpriteDefConfig &sprConfig = config.SpriteConfigs[name];
		sprConfig.BaseAddress = ParseHexString16bit(jsonSprConfig["BaseAddress"].get<std::string>());
		sprConfig.Count = jsonSprConfig["Count"].get<int>();
		sprConfig.Width = jsonSprConfig["Width"].get<int>();
		sprConfig.Height = jsonSprConfig["Height"].get<int>();
	}

	return true;
}

void SaveLabels(const FSpeccyUI *pUI, json &parentObject, uint16_t startAddress, uint16_t endAddress)
{
	json labelListJson;

	for (int i = startAddress; i <= endAddress; i++)
	{
		FLabelInfo *pLabel = pUI->Labels[i];
		if (pLabel != nullptr)
		{
			json labelJson;
			labelJson["Type"] = magic_enum::enum_name(pLabel->LabelType);
			labelJson["Address"] = pLabel->Address;
			labelJson["Size"] = pLabel->ByteSize;
			labelJson["Name"] = pLabel->Name;
			labelJson["Comment"] = pLabel->Comment;
			
			// References?

			labelListJson.push_back(labelJson);
		}

	}

	parentObject["Labels"] = labelListJson;
}

void SaveCodeInfo(const FSpeccyUI *pUI, json &parentObject,uint16_t startAddress,uint16_t endAddress)
{
	json codeInfoListJson;
	for (int i = startAddress; i <= endAddress; i++)
	{
		FCodeInfo *pCodeInfo = pUI->CodeInfo[i];
		if (pCodeInfo != nullptr)
		{
			json codeInfoJson;
			codeInfoJson["Address"] = pCodeInfo->Address;
			codeInfoJson["Size"] = pCodeInfo->ByteSize;
			codeInfoJson["JumpAddress"] = pCodeInfo->JumpAddress;
			codeInfoJson["Text"] = pCodeInfo->Text;
			codeInfoJson["Comment"] = pCodeInfo->Comment;
			codeInfoListJson.push_back(codeInfoJson);
		}
	}
	parentObject["CodeInfo"] = codeInfoListJson;
}

void WriteStringToFile(const std::string &str, FILE *fp)
{
	const int stringLength = str.size;
	fwrite(&stringLength, sizeof(int), 1, fp);
	fwrite(str.c_str(), 1, stringLength, fp);
}

void ReadStringFromFile(std::string &str, FILE *fp)
{
	int stringLength = 0;
	fread(&stringLength, sizeof(int), 1, fp);
	str.resize(stringLength);
	fread(&str[0], 1, stringLength, fp);
}

void SaveCodeInfoBin(const FSpeccyUI *pUI, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (pUI->CodeInfo[i] != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);
	
	for (int i = startAddress; i <= endAddress; i++)
	{
		FCodeInfo *pCodeInfo = pUI->CodeInfo[i];
		if (pCodeInfo != nullptr)
		{
			fwrite(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
			fwrite(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
			fwrite(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
			WriteStringToFile(pCodeInfo->Text, fp);
			WriteStringToFile(pCodeInfo->Comment, fp);
		}
	}
}

void LoadCodeInfoBin(FSpeccyUI *pUI, FILE *fp)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCodeInfo *pCodeInfo = new FCodeInfo;
		
		fread(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
		fread(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
		fread(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
		ReadStringFromFile(pCodeInfo->Text, fp);
		ReadStringFromFile(pCodeInfo->Comment, fp);
		pUI->CodeInfo[pCodeInfo->Address] = pCodeInfo;
	}
}

void SaveDataInfo(const FSpeccyUI *pUI, json &parentObject, uint16_t startAddress, uint16_t endAddress)
{
	json dataInfoListJson;
	for (int i = startAddress; i <= endAddress; i++)
	{
		FDataInfo *pDataInfo = pUI->DataInfo[i];
		if (pDataInfo != nullptr)
		{
			json dataInfoJson;
			dataInfoJson["Type"] = magic_enum::enum_name(pDataInfo->DataType);
			dataInfoJson["Address"] = pDataInfo->Address;
			dataInfoJson["Size"] = pDataInfo->ByteSize;
			dataInfoJson["Comment"] = pDataInfo->Comment;
			
			dataInfoListJson.push_back(dataInfoJson);
		}
	}
	parentObject["DataInfo"] = dataInfoListJson;
}

bool SaveGameData(FSpeccyUI *pUI, const char *fname)
{
	json gameDataJson;

	SaveLabels(pUI, gameDataJson,0x4000,0xFFFF);
	SaveCodeInfo(pUI, gameDataJson,0x4000,0xFFFF);
	SaveDataInfo(pUI, gameDataJson, 0x4000, 0xFFFF);

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << gameDataJson << std::endl;
		return true;
	}

	return false;// not implemented
}

bool LoadGameData(FSpeccyUI *pUI, const char *fname)
{
	return false;
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