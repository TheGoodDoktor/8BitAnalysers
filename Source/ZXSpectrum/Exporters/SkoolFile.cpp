#include "SkoolFile.h"

#include "Shared/Util/Misc.h"

#include <cassert>
#include <sstream>

FSkoolEntry::~FSkoolEntry()
{
	for (FSkoolInstruction* pInst : Instructions)
	{
		delete pInst;
	}
}
FSkoolInstruction* FSkoolEntry::AddInstruction(uint16_t address, const std::string& comment, const std::string& operation, char prefixChar, const std::string& commentLines /* = ""*/)
{
	FSkoolInstruction* pInst = new FSkoolInstruction(address, comment, operation, prefixChar, commentLines);
	Instructions.push_back(pInst);
	return pInst;
}


FSkoolFile::~FSkoolFile()
{
	for (FSkoolEntry* pEntry : Entries)
	{
		delete pEntry;
	}
}

void FSkoolFile::Parse()
{
	// todo
}

int FSkoolFile::WriteLinesToFile(FILE* fp, const std::string& str)
{
	std::stringstream stringStream(str);
	std::string line;
	int linesWritten = 0;
	while (std::getline(stringStream, line, '\n'))
	{
		if (!line.empty() && line[0] == '@')
			fprintf(fp, "%s\n", line.c_str());
		else
			fprintf(fp, "; %s\n", line.c_str());
		linesWritten++;
	}
	return linesWritten;
}

bool FSkoolFile::Export(const char* pFilename, Base base)
{
	FILE* fp = nullptr;
	fopen_s(&fp, pFilename, "wt");

	if (fp == nullptr)
		return false;

	// go through all the entries and write to disk
	for (FSkoolEntry* pEntry : Entries)
	{
		assert(!pEntry->Instructions.empty());

		for (FSkoolInstruction* pInst : pEntry->Instructions)
		{
			if (!pInst->CommentLines.empty())
			{
				WriteLinesToFile(fp, pInst->CommentLines);
			}

			if (const char* pLabel = GetLabel(pInst->Address))
			{
				fprintf(fp, "@label=%s\n", pLabel);
			}

			if (!pInst->Comment.empty() || !pInst->Operation.empty())
			{
				std::vector<std::string> commentLines;
				Tokenize(pInst->Comment, '\n', commentLines);

				// code lines always have a semicolon, even if the comment is empty.
				// other types only have a semicolon if we have a comment or we're in a brace comment segment.
				bool bDisplaySemicolon = true;
				if (pEntry->Type != SkoolDirective::Code && pInst->Comment.empty())
					bDisplaySemicolon = false;

				for (int i=0; i<commentLines.size(); i++)
				{
					if (i == 0)
					{
						fprintf(fp, base == Base::Decimal ? "%c%05d " : "%c$%04X ", pInst->CharPrefix, pInst->Address);
						fprintf(fp, "%-14s", pInst->Operation.c_str());
						if (pInst->Operation.length() > 14)
							fprintf(fp, " ");
						if (bDisplaySemicolon)
						{	
							if (commentLines[i].empty()) 
								fprintf(fp, ";");
							else
								fprintf(fp, "; ");
						}
						fprintf(fp, "%s\n", commentLines[i].c_str());
					}
					else
					{
						fprintf(fp, "%-20s ; %s\n", "", commentLines[i].c_str());
					}
				}
			}
		}

		if (pEntry != Entries.back())
			fprintf(fp, "\n");
	}
	fclose(fp);

	return true;
}

void FSkoolFile::Dump()
{
}

FSkoolEntry* FSkoolFile::GetEntry(uint16_t address) const
{
	// iterate from the end because the one we are looking for is most likely to be at the end
	for (TEntrylist::const_reverse_iterator it = Entries.rbegin(); it != Entries.rend(); ++it)
	{
		FSkoolEntry* pEntry = *it;
		if (pEntry->Address == address)
			return pEntry;
	}
	return nullptr;
}

FSkoolEntry* FSkoolFile::AddEntry(SkoolDirective type, uint16_t address)
{
	FSkoolEntry* pEntry = new FSkoolEntry(type, address);
	Entries.push_back(pEntry);
	return pEntry;
}

void FSkoolFile::AddLabel(uint16_t address, const std::string& label)
{
	Labels[address] = label;
}

const char* FSkoolFile::GetLabel(uint16_t address) const
{
	TLabelMap::const_iterator it = Labels.find(address);
	if (it == Labels.end())
		return nullptr;
	return it->second.c_str();
}

const char kDirectiveChars[] = " *bcgistuw";
static_assert(sizeof(kDirectiveChars)-1 == SkoolDirective::Count);

char GetCharFromDirective(SkoolDirective directive)
{
	return kDirectiveChars[(uint16_t)directive];
}

SkoolDirective GetDirectiveFromChar(unsigned char directiveChar)
{
	switch (directiveChar)
	{
		case ' ': return SkoolDirective::None;
		case '*': return SkoolDirective::BranchDestination;
		case 'b': 
		case 'B': 
			return SkoolDirective::Data;
		case 'c': 
		case 'C': 
			return SkoolDirective::Code;
		case 'g': 
		case 'G': 
			return SkoolDirective::GameStatusBuffer;
		case 'i': 
		case 'I': 
			return SkoolDirective::Ignored;
		case 's': 
		case 'S': 
			return SkoolDirective::RepeatedData;
		case 't': 
		case 'T': 
			return SkoolDirective::Text;
		case 'u': 
		case 'U': 
			return SkoolDirective::Unused;
		case 'w': 
		case 'W': 
			return SkoolDirective::WordData;
	}
	return SkoolDirective::None;
}
