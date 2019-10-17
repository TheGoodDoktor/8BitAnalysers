#include "FileUtil.h"

#include "Debug/Debug.h"

#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/error/en.h"

static eastl::string g_DataDirFromRoot("Tools/MapDataTool/Data/");
static eastl::string g_DataDirectory;

const eastl::string &GetDataDirectory()
{
	return g_DataDirectory;
}


bool DetermineDataDirectory(const char *pRouteIdentifier)
{
	g_DataDirectory = "";
	bool bFound = false;

	while (bFound == false)
	{
		FDirFileList dirListing;

		if (EnumerateDirectory(g_DataDirectory.c_str(), dirListing) == false)
			return false;

		for (FDirFileList::iterator entry = dirListing.begin(); entry < dirListing.end(); ++entry)
		{
			if (entry->FileType == FDirEntry::EType::File)
			{
				if (strcmp(entry->FileName.c_str(), ".root.txt") == 0)
				{
					g_DataDirectory += g_DataDirFromRoot;
					return true;
				}
			}
		}

		g_DataDirectory += "../";
	}

	return false;
}

bool FileExists(const char *pFilename)
{
	FILE *fp = fopen(pFilename, "rt");
	if (fp == nullptr)
		return false;
	fclose(fp);

	return true;
}

char *LoadTextFile(const char *pFilename)
{
	FILE *fp = fopen(pFilename, "rt");
	if (fp == nullptr)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *pTextData = new char[fileSize];
	if (pTextData == nullptr)
		return nullptr;
	fread(pTextData, fileSize, 1, fp);
	fclose(fp);

	return pTextData;
}

bool ParseJSONStringIntoDocument(const char *pJsonString, rapidjson::Document &document)
{
	document.Parse<rapidjson::kParseStopWhenDoneFlag>(pJsonString);
	if (document.HasParseError())
	{
		switch (document.GetParseError())
		{
		case rapidjson::kParseErrorDocumentEmpty:                   //!< The document is empty.
			break;
		case rapidjson::kParseErrorDocumentRootNotSingular:         //!< The document root must not follow by other values.
			break;
		case rapidjson::kParseErrorValueInvalid:                    //!< Invalid value.
			break;
		case rapidjson::kParseErrorObjectMissName:                  //!< Missing a name for object member.
			break;
		case rapidjson::kParseErrorObjectMissColon:                 //!< Missing a colon after a name of object member.
			break;
		case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket:   //!< Missing a comma or '}' after an object member.
			break;
		case rapidjson::kParseErrorArrayMissCommaOrSquareBracket:   //!< Missing a comma or ']' after an array element.
			break;
		case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex:   //!< Incorrect hex digit after \\u escape in string.
			break;
		case rapidjson::kParseErrorStringUnicodeSurrogateInvalid:   //!< The surrogate pair in string is invalid.
			break;
		case rapidjson::kParseErrorStringEscapeInvalid:             //!< Invalid escape character in string.
			break;
		case rapidjson::kParseErrorStringMissQuotationMark:         //!< Missing a closing quotation mark in string.
			break;
		case rapidjson::kParseErrorStringInvalidEncoding:           //!< Invalid encoding in string.
			break;
		case rapidjson::kParseErrorNumberTooBig:                    //!< Number too big to be stored in double.
			break;
		case rapidjson::kParseErrorNumberMissFraction:              //!< Miss fraction part in number.
			break;
		case rapidjson::kParseErrorNumberMissExponent:              //!< Miss exponent in number.
			break;
		case rapidjson::kParseErrorTermination:                     //!< Parsing was terminated.
			break;
		case rapidjson::kParseErrorUnspecificSyntaxError:            //!< Unspecific syntax error.
			break;
		}
	}
	return true;
}

bool LoadJSONFileIntoDocument(const char *pPath, rapidjson::Document &document)
{
	char *pJsonFile = LoadTextFile(pPath);
	if (pJsonFile == nullptr)
		return false;
	const bool bSuccess = ParseJSONStringIntoDocument(pJsonFile, document);

	delete[] pJsonFile;

	return bSuccess;
}

bool WriteJsonDocumentToFile(rapidjson::Document &doc, const char *pFileName)
{
	FILE *fp = fopen(pFileName, "wt");
	if (fp == NULL)
	{
		LOGERROR("Can't create JSON file %s", pFileName);
		return false;
	}

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
	doc.Accept(writer);
	fclose(fp);
	return true;
}

bool DownloadURLToJSONDocument(const char*site, const char *url, bool bHttps, rapidjson::Document &document)
{
	eastl::string output;
	if (DownloadURLToString(site, url, bHttps, output) == false)
		return false;
	
	return ParseJSONStringIntoDocument(output.c_str(), document);
}