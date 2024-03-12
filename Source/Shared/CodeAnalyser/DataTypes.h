#pragma once

#include "CodeAnalyserTypes.h"
#include "Misc/EmuBase.h"
#include <json_fwd.hpp>

struct FDataType
{
	FDataType() = default;
	FDataType(const char* pName):TypeId(AllocTypeId()),Name(pName){}
	
	virtual ~FDataType(){};
	
	int32_t         TypeId = -1;
	std::string     Name;
	
	virtual void    DrawDetailsUI() = 0;
	virtual void    ReadJson(const nlohmann::json& jsonIn) = 0;
	virtual void    WriteJson(nlohmann::json& jsonOut) const = 0;
 
	static int32_t  NextTypeId;
	static int32_t  AllocTypeId() { return NextTypeId++;}

};

// enum type
struct FEnumEntry
{
	FEnumEntry(const char* name, int val):Name(name), Value(val){}
	FEnumEntry(const std::string& name, int val):Name(name), Value(val){}

	std::string Name;
	int         Value = -1;
};

struct FEnum : public FDataType
{
	FEnum() {}
	FEnum(const char* pName):FDataType(pName){}
	
	void    DrawDetailsUI() override;
	
	bool    AddEntry(const char* name, int val);
	
	void    ReadJson(const nlohmann::json& jsonIn) override;
	void    WriteJson(nlohmann::json& jsonOut) const override;
	
	std::vector<FEnumEntry> Entries;
};

struct FFlagBit
{
	FFlagBit(const char* pPrefix, int index)
	{
		char flagName[16];
		snprintf(flagName,16,"%s_%d",pPrefix,index);
		Name = flagName;
	}
	FFlagBit(const std::string& name):Name(name){}

	std::string Name;
};

struct FFlagSet : public FDataType
{
	FFlagSet(){}
	FFlagSet(const char* Name);
	
	void    DrawDetailsUI() override;
	void    ReadJson(const nlohmann::json& jsonIn) override;
	void    WriteJson(nlohmann::json& jsonOut) const override;

	std::vector<FFlagBit>    Bits;
};

// struct type
struct FStructMember
{
	std::string             Name;
	int						ByteOffset = 0;
	EDataType               DataType = EDataType::Byte;
	EDataItemDisplayType    DisplayType = EDataItemDisplayType::Unknown;
};

struct FStruct : public FDataType
{
	FStruct(){}
	FStruct(const char *pName):FDataType(pName){}
	void	CalcByteOffsets();
	FStructMember*	GetStructMemberFromByteOffset(int byteOffset);
	void    DrawDetailsUI() override;
	void    ReadJson(const nlohmann::json& jsonIn) override;
	void    WriteJson(nlohmann::json& jsonOut) const override;


	std::vector<FStructMember> Members;
};

struct FDataTypeList
{
	void        Reset() { Items.clear();}
	bool        AddType(FDataType* pType);
	FDataType*  GetTypeFromId(int32_t typeId);
	FDataType*  GetTypeFromName(const char* pName);
	bool        RemoveType(int32_t typeId);
	
	virtual bool    Add(const char* pName) = 0;
	
	std::vector<FDataType*> Items;
};

struct FEnumTypeList : FDataTypeList
{
	bool    Add(const char* pName) override;
};

struct FFlagTypeList : FDataTypeList
{
	bool    Add(const char* pName) override;
};

struct FStructTypeList : FDataTypeList
{
	bool    Add(const char* pName) override;
};

class FDataTypes
{
public:
	// General
	void        Reset();
	
	// Enums
	//FEnum*  CreateNewEnum(const char* name);
	FEnum*  GetEnumFromTypeId(int32_t typeId) { return (FEnum*)EnumTypes.GetTypeFromId(typeId);}
	FEnum*  GetEnumFromName(const char * pName) { return (FEnum*)EnumTypes.GetTypeFromName(pName);}
	
	// Flags
	FFlagSet*   CreateNewFlagSet(const char* name);
	FFlagSet*   GetFlagsFromTypeId(int32_t typeId) { return (FFlagSet*)FlagTypes.GetTypeFromId(typeId);}
	FFlagSet*   GetFlagsFromName(const char * pName) { return (FFlagSet*)FlagTypes.GetTypeFromName(pName);}

	// Structs
	FStruct* GetStructFromTypeId(int32_t typeId) { return (FStruct*)StructTypes.GetTypeFromId(typeId); }

	
	const FEnumTypeList&    GetEnums() const { return EnumTypes;}
	FEnumTypeList&    GetEnums() { return EnumTypes;}
	
	const FFlagTypeList&    GetFlagSets() const { return FlagTypes;}
	FFlagTypeList&    GetFlagSets() { return FlagTypes;}
	
	const FStructTypeList&    GetStructs() const { return StructTypes;}
	FStructTypeList&    GetStructs() { return StructTypes;}

	bool				DrawStructComboBox(const char* pLabel,int& selection);
	void				DrawStructMember(int structId, int byteOffset);

	bool                WriteToJson(nlohmann::json& jsonDataTypes) const;
	bool                ReadFromJson(const nlohmann::json& jsonDataTypes);
	
private:
	FEnumTypeList       EnumTypes;
	FFlagTypeList       FlagTypes;
	FStructTypeList     StructTypes;
};


// Graphics Viewer
class FDataTypesViewer : public FViewerBase
{
public:
	FDataTypesViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Data Types";}
	bool            Init(void) override;
	void            Shutdown(void) override;
	
	void            DrawUI() override;
private:
	void            DrawTypeList(FDataTypeList& typeList);
	void            DrawFlags();
	void            DrawStructs();

	FDataTypes*     DataTypes = nullptr;
	
	int32_t             SelectedType = -1;
	std::string         TypeName;   // for editing
	//static const int    kNameTextSize = 16;
	std::string         NameText;
};
