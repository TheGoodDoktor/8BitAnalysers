#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>

#include "SpeccyUI.h"

using json = nlohmann::json;

void WriteStringToFile(const std::string &str, FILE *fp)
{
	const int stringLength = (int)str.size();
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

// Labels

void SaveLabelsBin(const FCodeAnalysisState &state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.Labels[i] != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		FLabelInfo *pLabel = state.Labels[i];
		if (pLabel != nullptr)
		{
			WriteStringToFile(std::string(magic_enum::enum_name(pLabel->LabelType)), fp);
			fwrite(&pLabel->Address, sizeof(pLabel->Address), 1, fp);
			fwrite(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
			WriteStringToFile(pLabel->Name, fp);
			WriteStringToFile(pLabel->Comment, fp);

			// References?
			int noRefences = (int)pLabel->References.size();
			fwrite(&noRefences, sizeof(int), 1, fp);
			for (const auto &ref : pLabel->References)
			{
				uint16_t refAddr = ref.first;
				fwrite(&refAddr, sizeof(refAddr), 1, fp);
			}
		}
	}
}

void LoadLabelsBin(FCodeAnalysisState &state, FILE *fp,int versionNo)
{
	int recordCount = 0;

	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FLabelInfo *pLabel = new FLabelInfo;
		
		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		pLabel->LabelType = magic_enum::enum_cast<LabelType>(enumVal).value();
		fread(&pLabel->Address, sizeof(pLabel->Address), 1, fp);
		fread(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
		ReadStringFromFile(pLabel->Name, fp);
		ReadStringFromFile(pLabel->Comment, fp);
		
		// References?
		if(versionNo > 1)
		{
			int noReferences;// = (int)pLabel->References.size();
			fread(&noReferences, sizeof(int), 1, fp);
			for (int i=0;i< noReferences;i++)
			{
				uint16_t refAddr;
				fread(&refAddr, sizeof(refAddr), 1, fp);
				pLabel->References[refAddr] = 1;
			}
		}

		state.Labels[pLabel->Address] = pLabel;
	}
}

// Code Info

void SaveCodeInfoBin(const FCodeAnalysisState &state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.CodeInfo[i] != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);
	
	for (int i = startAddress; i <= endAddress; i++)
	{
		FCodeInfo *pCodeInfo = state.CodeInfo[i];
		if (pCodeInfo != nullptr)
		{
			fwrite(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
			fwrite(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
			fwrite(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
			fwrite(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
			WriteStringToFile(pCodeInfo->Text, fp);
			WriteStringToFile(pCodeInfo->Comment, fp);
		}
	}
}

void LoadCodeInfoBin(FCodeAnalysisState &state, FILE *fp, int versionNo)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCodeInfo *pCodeInfo = new FCodeInfo;
		
		fread(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
		fread(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
		fread(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
		fread(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
		ReadStringFromFile(pCodeInfo->Text, fp);
		ReadStringFromFile(pCodeInfo->Comment, fp);
		state.CodeInfo[pCodeInfo->Address] = pCodeInfo;
	}
}

// Data Info

void SaveDataInfoBin(const FCodeAnalysisState& state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.DataInfo[i] != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		FDataInfo *pDataInfo = state.DataInfo[i];
		if (pDataInfo != nullptr)
		{
			WriteStringToFile(std::string(magic_enum::enum_name(pDataInfo->DataType)), fp);
			fwrite(&pDataInfo->Address, sizeof(pDataInfo->Address), 1, fp);
			fwrite(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
			WriteStringToFile(pDataInfo->Comment, fp);

			// References?
			int noRefences = (int)pDataInfo->References.size();
			fwrite(&noRefences, sizeof(int), 1, fp);
			for (const auto &ref : pDataInfo->References)
			{
				uint16_t refAddr = ref.first;
				fwrite(&refAddr, sizeof(refAddr), 1, fp);
			}
		}
	}
	
	
}

void LoadDataInfoBin(FCodeAnalysisState& state, FILE *fp, int versionNo)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FDataInfo *pDataInfo = new FDataInfo;

		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		pDataInfo->DataType = magic_enum::enum_cast<DataType>(enumVal).value();
		fread(&pDataInfo->Address, sizeof(pDataInfo->Address), 1, fp);
		fread(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
		ReadStringFromFile(pDataInfo->Comment, fp);

		// References?
		if (versionNo > 1)
		{
			int noReferences;// = (int)pLabel->References.size();
			fread(&noReferences, sizeof(int), 1, fp);
			for (int i = 0; i < noReferences; i++)
			{
				uint16_t refAddr;
				fread(&refAddr, sizeof(refAddr), 1, fp);
				pDataInfo->References[refAddr] = 1;
			}
		}

		state.DataInfo[pDataInfo->Address] = pDataInfo;
	}
}

static const int g_kBinaryFileVersionNo = 2;
static const int g_kBinaryFileMagic = 0xdeadface;

// Binary save
bool SaveGameDataBin(const FCodeAnalysisState& state, const char *fname, uint16_t addrStart, uint16_t addrEnd)
{
	FILE *fp = fopen(fname, "wb");
	if (fp == NULL)
		return false;

	fwrite(&g_kBinaryFileMagic, sizeof(int), 1, fp);
	fwrite(&g_kBinaryFileVersionNo, sizeof(int), 1, fp);
	SaveLabelsBin(state, fp, addrStart, addrEnd);
	SaveCodeInfoBin(state, fp, addrStart, addrEnd);
	SaveDataInfoBin(state, fp, addrStart, addrEnd);

	fclose(fp);
	return true;
}

// Binary load
bool LoadGameDataBin(FCodeAnalysisState& state, const char *fname, uint16_t addrStart, uint16_t addrEnd)
{
	FILE *fp = fopen(fname, "rb");
	if (fp == NULL)
		return false;
	int magic, versionNo;
	fread(&magic, sizeof(int), 1, fp);
	if (magic != g_kBinaryFileMagic)
	{
		fclose(fp);
		return false;
	}

	fread(&versionNo, sizeof(int), 1, fp);

	// loop across address range
	// clear what we're replacing
	for (int i = addrStart; i <= addrEnd; i++)	
	{
		delete state.Labels[i];
		state.Labels[i] = nullptr;

		delete state.CodeInfo[i];
		state.CodeInfo[i] = nullptr;

		delete state.DataInfo[i];
		state.DataInfo[i] = nullptr;
	}

	LoadLabelsBin(state, fp, versionNo);
	LoadCodeInfoBin(state, fp, versionNo);
	LoadDataInfoBin(state, fp, versionNo);

	fclose(fp);
	return true;
}

bool SaveGameData(const FCodeAnalysisState& state, const char *fname)
{
	return SaveGameDataBin(state, fname,0x4000, 0xffff);
}

bool SaveROMData(const FCodeAnalysisState& state, const char *fname)
{
	return SaveGameDataBin(state, fname, 0x0000, 0x3fff);
}

bool LoadGameData(FCodeAnalysisState& state, const char *fname)
{
	return LoadGameDataBin(state,fname,0x4000,0xffff);
}

bool LoadROMData(FCodeAnalysisState& state, const char *fname)
{
	return LoadGameDataBin(state, fname,0x0000,0x3fff);
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