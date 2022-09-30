
#include "SkoolkitImporter.h"
#include "CodeAnalyser/CodeAnalyser.h"

const std::string kWhiteSpace = " \n\r\t\f\v";
const std::string kSkoolKitDirectives = "bcgistuw";
const char kSkoolkitDirectiveNone = '-';

struct FSkoolkitInstruction
{
	bool HasValidDirective()
	{
		return kSkoolKitDirectives.find(Directive) != std::string::npos;
	}
	char Directive = kSkoolkitDirectiveNone;
	uint16_t Address = 0;
	std::string Comment;
};

std::string TrimLeadingChars(const std::string& str, const std::string& charsToTrim)
{
	size_t start = str.find_first_not_of(charsToTrim);
	return (start == std::string::npos) ? "" : str.substr(start);
}

std::string TrimLeadingWhitespace(const std::string& str)
{
	return TrimLeadingChars(str, kWhiteSpace);
}

bool StringStartsWith(const std::string& str, const std::string& substring)
{
	return (str.rfind(substring, 0) == 0);
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

void ParseAsmDirective(FCodeAnalysisState& state, const std::string& strLine, std::string& label)
{
	if (StringStartsWith(strLine, "@label="))
	{
		// @label directive
		// Create label at current instruction's address.
		// eg @label=START

		size_t eqLoc = strLine.find('=');
		if (eqLoc != std::string::npos)
		{
			label = strLine.substr(eqLoc + 1);
			if (label.back() == '\n' || label.back() == '\r')
				label.pop_back();
		}
	}
	else if (StringStartsWith(strLine, "@equ="))
	{
		// @equ directive
		// Create label at given address.
		// eg @equ=KSTATE=$5C00

		size_t eqLoc = strLine.find('=');
		if (eqLoc != std::string::npos)
		{
			std::string str = strLine.substr(eqLoc + 1);

			// split into label and address
			eqLoc = str.find('=');
			if (eqLoc != std::string::npos)
			{
				std::string labelStr = str.substr(0, eqLoc);
				std::string addressStr = str.substr(eqLoc + 1);
				if (addressStr[0] == '$')
				{
					addressStr = addressStr.substr(1, 4);
					uint16_t address = static_cast<uint16_t>(std::stoul(addressStr, nullptr, 16));

					// Mark, not sure this is right?
					// I don't know if the address is code or data, or if it already contains a label.
					// Hence, me doing AddLabelAtAddress() first because that will only create a label if one doesn't exist.
					// AddLabelAtAddress() also seems to work out the label type so it feels right? 
					
					AddLabelAtAddress(state, address);
					if (FLabelInfo* pLabelInfo = state.GetLabelForAddress(address))
					{
						SetLabelName(state, pLabelInfo, labelStr.c_str());
					}
				}

				// todo: decimal and 0x notation
			}
		}
	}
}

// temp. for OutputDebugString
//#include <Windows.h>

// todo: backup existing game data

bool ImportSkoolKitFile(FCodeAnalysisState& state, const char* pTextFileName)
{
	FILE* fp = nullptr;
	char pchLine[1024];
	fopen_s(&fp, pTextFileName, "rt");

	if (fp == nullptr)
		return false;

	char curDirective = kSkoolkitDirectiveNone;
	std::string comments;
	std::string label;
	while (fgets(pchLine, 1024, fp))
	{
		std::string strLine = pchLine;

		if (strLine[0] == '\n' || strLine[0] == '\r')
		{
			// skip blank lines
			continue;
		}

		if (strLine[0] == '@')
		{
			ParseAsmDirective(state, strLine, label);
			continue;
		}

		if (strLine[0] == ';')
		{
			comments += TrimLeadingChars(strLine, "; ");
			continue;
		}

		std::string trimmed = TrimLeadingWhitespace(strLine);
		if (trimmed[0] == ';')
		{
			// todo: deal with instruction comment continuation
			// eg the second line here:
			// c$028E LD L, $2F		; The initial key value for each line will be & 2F,
			//						; &2E, ..., & 28. (Eight lines.)
			continue;
		}

		// we've got an instruction.
		// get directive, address and comment 
		FItem* pItem = nullptr;
		FSkoolkitInstruction instruction;
		ParseInstruction(strLine, instruction);

		if (instruction.HasValidDirective() && curDirective != instruction.Directive)
		{
			curDirective = instruction.Directive;
		}
		
		switch (curDirective)
		{
		case 'c':
		{
			// Address is code

			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(instruction.Address);

			if (!pCodeInfo)
			{
				// Mark, I presume this is ok to set as code?
				SetItemCode(state, instruction.Address);
				pCodeInfo = state.GetCodeInfoForAddress(instruction.Address);

			}
			pItem = pCodeInfo;
		}
		break;
		case 'b':
		case 'w':
		{
			// Address is data
			
			// Mark, is this right? How do I know if I need to get ReadData or WriteData?
			// Also, not sure if we need to deal with the scenario that this address is set to code?
			// Oh, but didn't you say all memory locations will have data info, even if set to code?
			// If so, then I presume this is ok?
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(instruction.Address);
			/*FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(instruction.Address);
			if (pCodeInfo)
			{
				OutputDebugString("item is set to code\n");
			}*/
			if (pDataInfo)
			{
				pItem = pDataInfo;

				if (instruction.Directive == 'w')
				{
					// data is 16 bits wide
					
					// Mark, couldn't get this to work.
					// Is what I'm doing here right?
					// Edit: I think it's because the location is set to code.
					// For an example of where it is used see STRMDATA table at $15c6

					pDataInfo->DataType = DataType::Word;
					pDataInfo->ByteSize = 2;
					SetItemData(state, pDataInfo);
					state.bCodeAnalysisDataDirty = true;
				}
			}
		}
		break;
		case 't':
		{
			// Address is text

			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(instruction.Address);
			if (pDataInfo)
			{
				// Mark, I presume this is all I need to do to mark this location as text?
				// Sorry, I'm repeating myself.
				SetItemText(state, pDataInfo);

				pItem = pDataInfo;
			}
		}
		break;
		}

		if (!instruction.Comment.empty())
		{
			if (pItem)
			{
				pItem->Comment = instruction.Comment;
			}
		}

		if (!comments.empty())
		{
			// if a comment already exists this will fail to set the comment
			FCommentBlock* pBlock = AddCommentBlock(state, instruction.Address);
			if (pBlock)
			{
				pBlock->Comment = comments;
			}
			comments.clear();
		}

		if (!label.empty())
		{
			LabelType labelType = LabelType::Data;

			/*
			if (instruction.Directive == 'c')
				labelType = LabelType::Function;
			else if (curDirective == 'c')
				labelType = LabelType::Code;
			// This this will create a memory leak if a label already exists
			AddLabel(state, instruction.Address, label.c_str(), labelType);*/
			 
			// Mark, I wasn't sure if I should infer the label type from the current directive (as above) or to be passive
			// and let AddLabelAtAddress() figure it out?
			
			AddLabelAtAddress(state, instruction.Address);
			if (FLabelInfo* pLabelInfo = state.GetLabelForAddress(instruction.Address))
			{
				SetLabelName(state, pLabelInfo, label.c_str());
			}
			
			label.clear();
		}
	}

	return true;
}
