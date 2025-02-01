
#include "SkoolkitImporter.h"
#include "../Exporters/SkoolFileInfo.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Debug/DebugLog.h"
#include "Util/Misc.h"

#include <algorithm> // for std::count

const std::string kWhiteSpace = " \n\r\t\f\v";
const char kSkoolkitDirectiveNone = '-';

// Set this to true to strip carriage returns from inline comments.
// This won't affect block comments.
const bool bStripCarriageReturnsFromComments = false;
const bool bStripBracesFromComments = false;

struct FSkoolkitInstruction
{
	char BlockDirective = kSkoolkitDirectiveNone;
	char SubBlockDirective = kSkoolkitDirectiveNone;
	bool bBranchDestination = false; // is this address a branch destination (i.e. a line starting with an asterisk '*')
	uint16_t Address = 0;
	std::string Comment;
	std::string Operation; // the disassembly text
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

void RemoveTrailingChar(std::string& str, char ch)
{
	if (!str.empty() && str.back() == ch)
		str.pop_back();
}
void RemoveTrailingCarriageReturn(std::string& str)
{
	RemoveTrailingChar(str, '\n');
	RemoveTrailingChar(str, '\r');
}

void AddTrailingCarriageReturn(std::string& str)
{
	if (!str.empty() && str.back() != '\n')
	{
		str += "\n";
	}
}

bool StringStartsWith(const std::string& str, const std::string& substring)
{
	return (str.rfind(substring, 0) == 0);
}

char GetDirectiveFromAsm(const std::string& str)
{
	if (StringStartsWith(str, "DEF") || StringStartsWith(str, "def"))
	{
		if (str[3] == 'B' || str[3] == 'b')
			return 'b';
		if (str[3] == 'W' || str[3] == 'w')
			return 'w';
		if (str[3] == 'M' || str[3] == 'm')
			return 't';
		if (str[3] == 'S' || str[3] == 's')
			return 'b'; // todo
	}
	// if it's not a DEF* statement then we presume it's code
	return 'c';
}

bool IsNumeric(char c)
{
	return c >= '0' && c <= '9';
}

// macros to strip or deal with
// #UDGTABLE
// #TABLE
// #SPRITE
// #AUDIO
// #DEF
// #FACT
// #FOR
// #GBUF

// https://skoolkit.ca/docs/skoolkit/skool-macros.html
std::string ProcessMacro(const char** pMacroText, const char* pTxtStart)
{
	const char* pInTxtPtr = *pMacroText;

	/*
	static bool bInTable = false;
	if (bInTable == false)
	{
		if (!strncmp(pInTxtPtr, "UDGTABLE", 8) || !strncmp(pInTxtPtr, "TABLE", 5))
		{
			bInTable = true;
			return "";
		}
	}
	else
	{
		// todo check we're not going off start of string
		if (!strncmp(pInTxtPtr - 6, "TABLE", 5))
		{
			bInTable = false;
		}
		return "";
	}*/

	if (!strncmp(pInTxtPtr, "REG", 3))
	{
		pInTxtPtr += 3;

		static const int numEntries = 35;
		static const char* map[numEntries][2] =
		{
			{"ixh", "#REG:IXH#"},
			{"ixl", "#REG:IXL#"},
			{"iyl", "#REG:IYL#"},
			{"iyh", "#REG:IYH#"},
			{"ix", "#REG:IX#"},
			{"iy", "#REG:IY#"},

			{"af'", ""},
			{"bc'", ""},
			{"de'", ""},
			{"hl'", ""},

			{"a'", ""},
			{"f'", ""},
			{"b'", ""},
			{"c'", ""},
			{"d'", ""},
			{"e'", ""},
			{"h'", ""},
			{"l'", ""},

			{"af", "#REG:AF#"},
			{"bc", "#REG:BC#"},
			{"de", "#REG:DE#"},
			{"hl", "#REG:HL#"},
			{"pc", "#REG:PC#"},
			{"sp", "#REG:SP#"},
			{"ir", "#REG:IR#"},

			{"a", "#REG:A#"},
			{"f", "#REG:F#"},
			{"b", "#REG:B#"},
			{"c", "#REG:C#"},
			{"d", "#REG:D#"},
			{"e", "#REG:E#"},
			{"h", "#REG:H#"},
			{"l", "#REG:L#"},
			{"i", "#REG:I#"},
			{"r", "#REG:R#"},
		};

		for (int i = 0; i < numEntries; i++)
		{
			const char* pSrc = map[i][0];
			const size_t srcLen = strlen(pSrc);
			if (!strncmp(pInTxtPtr, pSrc, srcLen))
			{
				const char* pDst = map[i][1];
				if (pDst[0] != 0)
					*pMacroText = pInTxtPtr + srcLen;
				return map[i][1];
			}
		}

		return "";
	}

	if (*pInTxtPtr == 'R')
	{
		static const int kBufSize = 32;
		static char buf[kBufSize] = { 0 };

		pInTxtPtr++;

		const bool bHex = *pInTxtPtr == '$';
		if (bHex)
			pInTxtPtr++;

		char* pNumEnd;
		const int address = static_cast<uint16_t>(strtol(pInTxtPtr, &pNumEnd, bHex ? 16 : 10));
		if (pInTxtPtr != pNumEnd)
		{
			snprintf(buf, kBufSize, "#ADDR:0x%04X#", address);
			*pMacroText = pNumEnd;
			return buf;
		}

		return "";
	}

	if (*pInTxtPtr == 'b')
	{
		static const int kBufSize = 32;
		static char buf[kBufSize] = { 0 };

		pInTxtPtr++;

		char* pNumEnd;
		const int num = static_cast<uint16_t>(strtol(pInTxtPtr, &pNumEnd, 10));
		if (pInTxtPtr != pNumEnd)
		{
			snprintf(buf, kBufSize, "%d", num);
			*pMacroText = pNumEnd;
			return buf;
		}
		return "";
	}

#ifndef NDEBUG
	//LOGINFO("[MACRO] %s", pInTxtPtr);
#endif
	return "";
}

std::string ProcessComment(const char* pText)
{
	const char* pTxtPtr = pText;
	std::string outString;

	if (bStripBracesFromComments)
	{
		// skip leading brace
		if (*pTxtPtr == '{')
			pTxtPtr++;
	}
	
	while (*pTxtPtr != 0)
	{
		const char ch = *pTxtPtr++;

		if (ch == '#')
		{
			// See if we've hit a supported skoolkit macro.
			// If so, convert it to SA markup.
			std::string markupTxt = ProcessMacro(&pTxtPtr, pText);
			if (markupTxt.empty())
			{
				// add escape character so SA doesn't try to treat this macro as SA markup.
				outString += "\\#";
			}
			else
			{
				outString += markupTxt;
			}
		}
		else
		{
			outString += ch;	// add to string
		}
	}

	RemoveTrailingCarriageReturn(outString);
	
	if (bStripBracesFromComments)
	{
		RemoveTrailingChar(outString, '}');
	}

#ifndef NDEBUG
	//LOGINFO("REPLACED '%s' with '%s'", pText, outString.c_str());
#endif

	return outString;
}

std::string ProcessBlockComment(const char* pText)
{
	return ProcessComment(pText);
}

std::string ProcessInlineComment(const char* pText)
{
	return ProcessComment(pText);
}

bool ParseInstruction(std::string strLine, FSkoolkitInstruction& instruction)
{
	if (strLine.length() < 6)
		return false;

	if (strLine[0] == '*')
		instruction.bBranchDestination = true;
	else if (strLine[0] != ' ')
		instruction.BlockDirective = strLine[0];
	
	if (strLine[1] == '$')
	{
		// hexadecimal address
		char* pNumEnd;
		std::string numStr = strLine.substr(2, 4);
		instruction.Address = static_cast<uint16_t>(strtol(numStr.c_str(), &pNumEnd, 16));
		if (pNumEnd == numStr.c_str())
		{
			return false;
		}
	}
	else
	{
		// decimal address
		char* pNumEnd;
		std::string numStr = strLine.substr(1, 5);
		instruction.Address = static_cast<uint16_t>(strtol(numStr.c_str(), &pNumEnd, 10));
		if (pNumEnd == numStr.c_str())
		{
			return false;
		}
	}

	const size_t opStart = 7;
	size_t opLen = std::string::npos;

	// get the comment string
	// todo deal with semicolons in strings
	const size_t semicolonPos = strLine.find_first_of(';');
	if (semicolonPos != std::string::npos)
	{
		// calculate where the operation text begins
		opLen = semicolonPos - opStart;

		size_t strLen = strLine.length();

		// skip ';' and leading space of comment
		size_t commentStart = semicolonPos + 2;

		if (commentStart == strLen)
		{
			// Special case. We have an empty comment.
			// Empty comments occur in the skool file on data lines when we're between lines that contain 
			// a comment with an open and close brace. i.e. { and }
			// To preserve these we set the comment to be a carriage return. This forces an empty comment 
			// to be written out when exporting.
			instruction.Comment = "\n";
		}
		else if (commentStart < strLen)
		{
			const char* pCommentTxt = strLine.c_str() + commentStart;
			instruction.Comment = ProcessInlineComment(pCommentTxt);
		}
	}
	
	// skip trailing spaces of disassembly text
	if (semicolonPos != std::string::npos)
	{
		size_t opEnd = strLine.find_last_not_of(' ', semicolonPos-1);
		if (opEnd != std::string::npos)
			opLen = opEnd+1 - opStart;
	}

	// get the disassembly text inbetween the address and the comment
	instruction.Operation = strLine.substr(opStart, opLen);
	RemoveTrailingCarriageReturn(instruction.Operation);
	instruction.SubBlockDirective = GetDirectiveFromAsm(instruction.Operation);

	return true;
}

bool ParseAsmDirective(FCodeAnalysisState& state, const std::string& strLine, std::string& label)
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
		return true;
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
					const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(address);
					AddLabelAtAddress(state, addrRef);
					if (FLabelInfo* pLabelInfo = state.GetLabelForAddress(addrRef))
					{
						pLabelInfo->ChangeName(labelStr.c_str(), addrRef);
					}
				}
				// todo: decimal and 0x notation

				return false;
			}
		}
	}

	return false;
}

// Split a string containing comma delimited items into individual strings.
// Items can be text in quotes or numeric values.
void SplitCommaDelimitedItems(const std::string& str, std::vector<std::string>& items)
{
	items.clear();
	
	if (str.empty())
		return;

	bool bInString = false;
	size_t start = 0;
	char c;
	bool bEscapeChar = false;
	for (size_t i=0; i<str.size(); i++)
	{
		c = str[i];
		
		if (c == '"')
		{
			if (!bEscapeChar)
			{
				// ignore this quote if it's part of an escape sequence
				bInString = !bInString;
			}
		}

		bEscapeChar = c == '\\';

		if (!bInString)
		{
			if (c == ',')
			{
				std::string subStr = str.substr(start, i-start);
				items.push_back(subStr);
				start = i+1; 
			}
		}
	}
	// add the remainder of the string 
	items.push_back(str.substr(start));
}

// Given a string will count the number of bytes that string represents.
// The string can contain either text in quotes or numeric values
// eg "RND" = 3 bytes
//    255 = 1 byte
//    "\"" = 1 byte
uint16_t CountDataBytes(std::string str)
{
	uint16_t size = 0;
	size_t first = str.find('"');
	size_t last = str.find_last_of('"');
	if (first != std::string::npos && last != std::string::npos)
	{
		for (size_t i=first+1; i<last; i++)
		{
			char c = str[i];
			if (c != '\\') // don't count escape characters
				size++;
		}
	}
	else
	{
		// if we didn't find a string we presume it's a byte value
		// todo word values
		size += 1; 
	}
	return size;
}

bool ImportSkoolKitFile(FCodeAnalysisState& state, const char* pTextFileName, FSkoolFileInfo* pSkoolInfo /*=nullptr*/)
{
	FILE* fp = fopen(pTextFileName, "rt");
	char pchLine[65536];
	// note: "rt" = text mode, which means Windows line endings \r\n will be converted into \n
	
	if (fp == nullptr)
		return false;

	char blockDirective = kSkoolkitDirectiveNone;
	char subBlockDirective = kSkoolkitDirectiveNone;

	std::string commentBlock;
	std::string label;
	FCodeAnalysisItem LastItem;

	uint16_t minAddr=0xffff;
	uint16_t maxAddr=0;

	// only used if pSkoolInfo is set
	FSkoolFileLocation skoolLocation;
	const FSkoolFileLocation kSkoolLocationDefault;
	bool bInRsubSection = false;

#ifndef NDEBUG
	FCodeAnalysisItem longestCommentItem;
	int longestCommentLineNum = -1;
#endif

	unsigned int lineNum = 0;
	while (fgets(pchLine, 65536, fp))
	{
		std::string strLine = pchLine;
		lineNum++;

		if (bInRsubSection)
		{
			if (StringStartsWith(strLine, "@rsub+end"))
				bInRsubSection = false;
			else
				LOGINFO("Skipping @rsub text '%s' on line %d", pchLine, lineNum);
			continue;
		}

		if (strLine[0] == '@')
		{
			if (!ParseAsmDirective(state, strLine, label))
				commentBlock += strLine;

			if (StringStartsWith(strLine, "@rsub+begin"))
			{
				bInRsubSection = true;
			}
			
			continue;
		}

		// Is this a comment block?
		if (strLine[0] == ';')
		{
			commentBlock += TrimLeadingChars(strLine, "; ");
			continue;
		}

		std::string trimmed = TrimLeadingWhitespace(strLine);
		if (trimmed[0] == ';')
		{
			// instruction comment continuation
			if (LastItem.IsValid() && !LastItem.Item->Comment.empty())
			{
				if (bStripCarriageReturnsFromComments)
				{
					LastItem.Item->Comment += " ";
				}
				else
				{
					AddTrailingCarriageReturn(LastItem.Item->Comment);
				}
				
				LastItem.Item->Comment += ProcessInlineComment(trimmed.c_str() + 2);
				RemoveTrailingCarriageReturn(LastItem.Item->Comment);

#ifndef NDEBUG
				if (!longestCommentItem.Item || LastItem.Item->Comment.size() > longestCommentItem.Item->Comment.size())
				{
					longestCommentItem = LastItem;
					longestCommentLineNum = lineNum;
				}
#endif
			}
			continue;
		}

		if (trimmed.empty())
		{
			// skip blank lines
			continue;
		}
		
		// We've got an instruction.
		// Get directive, address and comment 
		FItem* pItem = nullptr;
		FSkoolkitInstruction instruction;
		if (!ParseInstruction(strLine, instruction))
		{
			RemoveTrailingCarriageReturn(strLine);
			LOGWARNING("Parse error on line %d. Could not parse instruction: '%s'", lineNum, strLine.c_str());
			fclose(fp);
			return false;
		}

		if (LastItem.IsValid() && instruction.Address < LastItem.AddressRef.Address)
		{
			// if this address is lower than the last one we saw then something has gone wrong, so abort
			LOGWARNING("Parse error on line %d. Address $%x (%d) is lower than previous read address: $%x (%d)", lineNum, instruction.Address, instruction.Address, LastItem.AddressRef.Address, LastItem.AddressRef.Address);
			fclose(fp);
			return false;
		}

		if (pSkoolInfo)
		{
			skoolLocation = FSkoolFileLocation();
			skoolLocation.bBranchDestination = instruction.bBranchDestination;
			skoolLocation.BlockDirective = GetDirectiveFromChar(instruction.BlockDirective);
		}

		if (instruction.BlockDirective != kSkoolkitDirectiveNone && blockDirective != instruction.BlockDirective)
		{
			// we've encountered a new block
			blockDirective = instruction.BlockDirective;
			subBlockDirective = instruction.SubBlockDirective; 
			if (pSkoolInfo)
				skoolLocation.BlockDirective = GetDirectiveFromChar(blockDirective);// is this needed? we're doing it above
		}

		if (instruction.SubBlockDirective != subBlockDirective)
		{
			// we've encountered a new sub-block
			subBlockDirective = instruction.SubBlockDirective;

			if (pSkoolInfo)
				skoolLocation.SubBlockDirective = GetDirectiveFromChar(subBlockDirective);
		}

		switch (instruction.SubBlockDirective)
		{
		case 'c':
		{
			// Address is code

			FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(instruction.Address);
			if (!pCodeInfo)
			{
				WriteCodeInfoForAddress(state, instruction.Address);
				pCodeInfo = state.GetCodeInfoForPhysicalAddress(instruction.Address);

			}
			pItem = pCodeInfo;

			
			if (blockDirective == 'u')
				pCodeInfo->bUnused = true;
		}
		break;
		case 'b':
		case 'w':
		{
			// Address is data
			
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(instruction.Address);
			FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(instruction.Address);
			if (pCodeInfo)
			{
				LOGWARNING("Item at $%02X was set to code: %s",instruction.Address, pCodeInfo->Text.c_str());
				LOGWARNING("Code item removed and replace as data");
				// remove the code item
				state.SetCodeInfoForAddress(instruction.Address, nullptr);	// memory will get cleared up 
			}
			if (pDataInfo)
			{
				pItem = pDataInfo;

				// count how many entries we have
				const uint16_t numItems = static_cast<uint16_t>(std::count(instruction.Operation.begin(), instruction.Operation.end(), ',') + 1);
				std::string defStatement = instruction.Operation.substr(0, 4);
				
				if (defStatement == "DEFB" || defStatement == "defb")
				{
					if (numItems == 1)
						pDataInfo->DataType = EDataType::Byte;
					else
						pDataInfo->DataType = EDataType::ByteArray;
					pDataInfo->ByteSize = numItems;
				}
				else if (defStatement == "DEFW" || defStatement == "defw")
				{
					if (numItems == 1)
						pDataInfo->DataType = EDataType::Word;
					else
						pDataInfo->DataType = EDataType::WordArray;
					pDataInfo->ByteSize = numItems * 2;
				}

				if (blockDirective == 'g')
					pDataInfo->bGameState = true;
				else if (blockDirective == 'u')
					pDataInfo->bUnused2 = true;	// What is this?
			}
		}
		break;
		case 't':
		{
			// Address is text
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(instruction.Address);
			if (pDataInfo)
			{
				// If this is set to true it will parse the DEFM statement and calculate
				// how many bytes the text needs to be. This DEFM statement could contain
				// non-ascii byte values mixed in with the text.
				// This means when the DEFM statement is exported it will match exactly 
				// the DEFM statement that was imported.
				// This will bypass SetItemText() so may not display correctly in the tool.
				const bool bSkoolKitCompatibleText = false;

				if (bSkoolKitCompatibleText)
				{
					std::string defStatement = instruction.Operation.substr(0, 4);

					std::vector<std::string> elements;
					SplitCommaDelimitedItems(instruction.Operation.substr(5), elements);

					// This loop counts the number of bytes declared in the DEFM instruction.
					// It can deal with byte values in addition to text strings.
					// eg DEFM "One",2,"Three"
					uint16_t byteSize = 0;
					for (std::string& str : elements)
					{
						byteSize += CountDataBytes(str);
					}

					if (byteSize > 0)
					{
						pDataInfo->DataType = EDataType::Text;

						// SetItemText doesnt set the number of bytes correctly (compared to how skoolkit does it),
						// so we set the byte size manually based on how many bytes we counted in the statement.
						pDataInfo->ByteSize = byteSize;

						// todo set bBit7Terminator flag on the FDataItem
						// todo check we're not overlapping items.
					}
				}
				else
				{
					// force to byte type otherwise SetItemText() does nothing
					pDataInfo->DataType = EDataType::Byte;

					SetItemText(state, FCodeAnalysisItem(pDataInfo,state.GetBankFromAddress(instruction.Address), instruction.Address));
				}
				pItem = pDataInfo;
			}
		}
		break;
		}

		if (!instruction.Comment.empty())
		{
			if (pItem)
			{
				pItem->Comment = instruction.Comment.c_str();
			}
		}

		if (!commentBlock.empty())
		{
			FCommentBlock* pBlock = state.GetCommentBlockForAddress(state.AddressRefFromPhysicalAddress(instruction.Address));
			
			if (pBlock == nullptr)
				pBlock = AddCommentBlock(state, state.AddressRefFromPhysicalAddress(instruction.Address));
			else
			{
				std::string commentExcerpt = pBlock->Comment.substr(0, 1024);
				RemoveTrailingCarriageReturn(commentExcerpt);
				LOGWARNING("SkoolkitImporter: Replacing existing comment block: '%s'", commentExcerpt.c_str());
			}

			pBlock->Comment = ProcessBlockComment(commentBlock.c_str());
			commentBlock.clear();
		}

		if (!label.empty())
		{
			ELabelType labelType = ELabelType::Data;
			FAddressRef addrRef = state.AddressRefFromPhysicalAddress(instruction.Address);
			AddLabelAtAddress(state, addrRef);
			if (FLabelInfo* pLabelInfo = state.GetLabelForAddress(addrRef))
			{
				pLabelInfo->ChangeName(label.c_str(), addrRef);
			}
			
			label.clear();
		}

		if (pSkoolInfo)
		{
			if (instruction.BlockDirective != 's') // todo: repeated data
			{
				if (!(skoolLocation == kSkoolLocationDefault))
					pSkoolInfo->Locations[instruction.Address] = skoolLocation;
			}
		}

		minAddr = std::min(instruction.Address, minAddr);
		maxAddr = std::max(instruction.Address, maxAddr);

		LastItem = FCodeAnalysisItem(pItem, state.GetBankFromAddress(instruction.Address), instruction.Address);
	}

#ifndef NDEBUG
	if (longestCommentItem.Item)
	{
		LOGINFO("Longest comment: line %d. %d chars. address %d 0x%x", 
			longestCommentLineNum,
			longestCommentItem.Item->Comment.size(), 
			longestCommentItem.AddressRef.Address,
			longestCommentItem.AddressRef.Address);
		LOGINFO("Comment is '%s'", longestCommentItem.Item->Comment.c_str());
	}
#endif

	if (pSkoolInfo)
	{
		pSkoolInfo->StartAddr = minAddr;
		pSkoolInfo->EndAddr = maxAddr;
	}

	state.SetAddressRangeDirty();	
	fclose(fp);
	return true;
}
