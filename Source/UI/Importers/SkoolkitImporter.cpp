
#include "SkoolkitImporter.h"
#include "CodeAnalyser/CodeAnalyser.h"

const std::string kWhiteSpace = " \n\r\t\f\v";

struct FSkoolkitInstruction
{
	char Directive = '-';
	uint16_t Address = 0;
	std::string Comment;
};

std::string TrimLeadingChars(const std::string& s, const std::string charsToTrim)
{
	size_t start = s.find_first_not_of(charsToTrim);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string TrimLeadingWhitespace(const std::string& s)
{
	return TrimLeadingChars(s, kWhiteSpace);
}

void ParseInstruction(std::string strLine, FSkoolkitInstruction& instruction)
{
	if (strLine.length() < 6)
		return;

	instruction.Directive = strLine[0];

	if (strLine[1] == '$')
	{
		// hexadecimal address
		instruction.Address = static_cast<uint16_t>(strtol(strLine.substr(2, 4).c_str(), NULL, 16));
	}
	else
	{
		// decimal address
		instruction.Address = static_cast<uint16_t>(strtol(strLine.substr(1, 5).c_str(), NULL, 10));
	}

	size_t commentStart = strLine.find_first_of(';');
	if (commentStart != std::string::npos)
	{
		// skip leading space and ';'
		commentStart += 2;

		if (commentStart < strLine.length())
		{
			instruction.Comment = strLine.substr(commentStart);

			// remove carriage return
			if (instruction.Comment.back() == '\n' || instruction.Comment.back() == '\r')
				instruction.Comment.pop_back();
		}
	}
}

//#include <Windows.h>

// todo: backup existing game data

bool ImportSkoolKitFile(FCodeAnalysisState& state, const char* pTextFileName)
{
	FILE* fp = nullptr;
	char line[1024];
	fopen_s(&fp, pTextFileName, "rt");

	if (fp == nullptr)
		return false;

	int adddress = -1;
	int l = 0;
	char directive = '-';
	std::string comments;
	while (fgets(line, 1024, fp))
	{
		std::string strLine = line;

		if (strLine[0] == '\n' || strLine[0] == '\r')
		{
			// skip blank lines
			continue;
		}

		if (strLine[0] == '@')
		{
			// todo parse asm directive
			continue;
		}

		if (strLine[0] == ';')
		{
			// todo deal with full line comment
			comments += TrimLeadingChars(strLine, "; ");
			continue;
		}

		std::string trimmed = TrimLeadingWhitespace(strLine);
		if (trimmed[0] == ';')
		{
			// todo deal with instruction comment continuation
			continue;
		}

		// we've got an instruction
		// get directive, address and comment 
		FSkoolkitInstruction instruction;
		ParseInstruction(strLine, instruction);

		if (!instruction.Comment.empty())
		{
			// todo: this might be comment to data
			if (FCodeInfo* pInfo = state.GetCodeInfoForAddress(instruction.Address))
			{
				pInfo->Comment = instruction.Comment;
			}
		}

		FCommentBlock* pBlock = AddCommentBlock(state, instruction.Address);
		pBlock->Comment = comments;
		comments.clear();
		//OutputDebugString(comments.c_str());
	}

	return true;
}
