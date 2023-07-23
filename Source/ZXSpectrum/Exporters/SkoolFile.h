#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <string>

// Note: Entries in a skoolfile cannot start with ' ' or '*'.
enum SkoolDirective : unsigned char
{
	None = 0,				// ' '
	BranchDestination,		// '*'
	Data,					// 'b' or 'B'
	Code,					// 'c' or 'C'
	GameStatusBuffer,		// 'g' or 'G'
	Ignored,				// 'i' or 'I'
	RepeatedData,			// 's' or 'S'
	Text,					// 't' or 'T'
	Unused,					// 'u' or 'U'
	WordData,				// 'w' or 'W'

	Count,
};

class FSkoolEntry
{
public:
	FSkoolEntry(SkoolDirective type, uint16_t address)
		: Type(type)
		, Address(address) { }
	virtual ~FSkoolEntry();
	struct FSkoolInstruction* AddInstruction(uint16_t address, const std::string& comment, const std::string& operation, char prefixChar, const std::string& leadingComment = "");
	uint16_t Address;
	SkoolDirective Type = SkoolDirective::None;
	std::vector<FSkoolInstruction*> Instructions;
};

struct FSkoolInstruction
{
	FSkoolInstruction() { }
	FSkoolInstruction(uint16_t address, const std::string& comment, const std::string& operation, char prefixChar, const std::string& commentLines)
		: Address(address)
		, Comment(comment)
		, Operation(operation)
		, CharPrefix(prefixChar)
		, CommentLines(commentLines) { }

	std::string CommentLines;	// full line comment(s) before the instruction line

	char CharPrefix = ' ';
	uint16_t Address = 0;
	std::string Comment;		// the comment after the disassembly text
	std::string Operation;		// the disassembly text
};

class FSkoolFile
{
public:
	enum class Base
	{
		Decimal,
		Hexadecimal,
	};

	~FSkoolFile();
	void Parse();
	bool Export(const char* pFilename, Base base);

	FSkoolEntry* GetEntry(uint16_t address) const;
	FSkoolEntry* AddEntry(SkoolDirective type, uint16_t address);
	void AddLabel(uint16_t address, const std::string& label);
	const char* GetLabel(uint16_t address) const;

private:
	int WriteLinesToFile(FILE* fp, const std::string& str);
	void Dump();
	
	typedef std::map<uint16_t, std::string> TLabelMap;
	TLabelMap Labels;
	typedef std::vector<FSkoolEntry*> TEntrylist;
	TEntrylist Entries; // list of Entries, aka Blocks
};

SkoolDirective GetDirectiveFromChar(unsigned char directiveChar);
char GetCharFromDirective(SkoolDirective directive);