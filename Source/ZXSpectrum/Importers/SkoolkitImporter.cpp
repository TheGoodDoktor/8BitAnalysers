
#include "SkoolkitImporter.h"
#include "../Exporters/SkoolFileInfo.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Debug/DebugLog.h"
#include "Util/Misc.h"

#include <algorithm> // for std::count

const std::string kWhiteSpace = " \n\r\t\f\v";
const char kSkoolkitDirectiveNone = '-';

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

void RemoveCarriageReturn(std::string& str)
{
	if (str.empty())
		return;
	if (str.back() == '\n' || str.back() == '\r')
		str.pop_back();
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
		std::string numStr = strLine.substr(2, 4);
		instruction.Address = static_cast<uint16_t>(strtol(numStr.c_str(), nullptr, 16));
		if (instruction.Address == 0)
		{
			if (numStr == "0000")
			{
				// Deal with the special case of the address being 0.
				// We do this because strtol will return 0 if it fails.
				// We want to differentiate between a valid address of 0 and a failure.
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		// decimal address
		std::string numStr = strLine.substr(1, 5);
		instruction.Address = static_cast<uint16_t>(strtol(numStr.c_str(), nullptr, 10));
		if (instruction.Address == 0)
		{
			if (numStr == "00000")
			{
				// do nothing
			}
			else
			{
				return false;
			}
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
			instruction.Comment = strLine.substr(commentStart);

			RemoveCarriageReturn(instruction.Comment); // do I need to do this?
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

	RemoveCarriageReturn(instruction.Operation);

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

					AddLabelAtAddress(state, state.AddressRefFromPhysicalAddress(address));
					if (FLabelInfo* pLabelInfo = state.GetLabelForAddress(address))
					{
						SetLabelName(state, pLabelInfo, labelStr.c_str());
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

	std::string comments;
	std::string label;
	FCodeAnalysisItem LastItem;

	uint16_t minAddr=0xffff;
	uint16_t maxAddr=0;

	// only used if pSkoolInfo is set
	FSkoolFileLocation skoolLocation;
	const FSkoolFileLocation kSkoolLocationDefault;
	bool bInRsubSection = false;

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
				comments += strLine;

			if (StringStartsWith(strLine, "@rsub+begin"))
			{
				bInRsubSection = true;
			}
			
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
			// instruction comment continuation
			if (LastItem.IsValid())
			{
				if (LastItem.Item->Comment.back() != '\n')
					LastItem.Item->Comment += "\n";
				LastItem.Item->Comment += trimmed.substr(2);
				RemoveCarriageReturn(LastItem.Item->Comment);
			}
			continue;
		}

		if (trimmed.empty())
		{
			// skip blank lines
			continue;
		}
		
		// we've got an instruction.
		// get directive, address and comment 
		FItem* pItem = nullptr;
		FSkoolkitInstruction instruction;
		if (!ParseInstruction(strLine, instruction))
		{
			RemoveCarriageReturn(strLine);
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

			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(instruction.Address);
			if (!pCodeInfo)
			{
				WriteCodeInfoForAddress(state, instruction.Address);
				pCodeInfo = state.GetCodeInfoForAddress(instruction.Address);

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
			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(instruction.Address);
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
				pItem->Comment = instruction.Comment;
			}
		}

		if (!comments.empty())
		{
			FCommentBlock* pBlock = state.GetCommentBlockForAddress(instruction.Address);
			
			if (pBlock == nullptr)
				pBlock = AddCommentBlock(state, instruction.Address);
			else
			{
				std::string commentExcerpt = pBlock->Comment.substr(0, 1024);
				RemoveCarriageReturn(commentExcerpt);
				LOGWARNING("SkoolkitImporter: Replacing existing comment block: '%s'", commentExcerpt.c_str());
			}

			pBlock->Comment = comments;
			comments.clear();
		}

		if (!label.empty())
		{
			ELabelType labelType = ELabelType::Data;
			
			AddLabelAtAddress(state, state.AddressRefFromPhysicalAddress(instruction.Address));
			if (FLabelInfo* pLabelInfo = state.GetLabelForAddress(instruction.Address))
			{
				SetLabelName(state, pLabelInfo, label.c_str());
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

	if (pSkoolInfo)
	{
		pSkoolInfo->StartAddr = minAddr;
		pSkoolInfo->EndAddr = maxAddr;
	}

	state.SetAddressRangeDirty();	
	fclose(fp);
	return true;
}
