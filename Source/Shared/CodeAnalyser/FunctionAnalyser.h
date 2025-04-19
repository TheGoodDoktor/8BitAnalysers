#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "CodeAnalyserTypes.h"

class FCodeAnalysisState;
class FDisplayTypeBase;

// we use int type so it can work with ImGui::Combo

enum class EFunctionParamSourceZ80 : int
{
	RegA,
	RegB,
	RegC,
	RegD,
	RegE,
	RegH,
	RegL,
	RegBC,
	RegDE,
	RegHL,
	RegIX,
	RegIY,
};

enum class EFuctionParamSourceM6502 : int
{
	RegA,
	RegX,
	RegY,
};

enum class EFunctionParamType : int
{
	Unknown = 0,
	Number,
	HexNumber,
	Address,
	XPos,
	YPos,
	XCharPos,
	YCharPos,
	XYPos,		// x/y pair
	XYCharPos,	// x/y pair
};

// structure to hold function parameters
struct FFunctionParam
{
	FFunctionParam();

	void StoreValue(uint16_t value)
	{
		History[HistoryIndex % kMaxHistory] = value;
		HistoryIndex++;
	}

	uint16_t GetLastValue(int offset) const
	{
		if (HistoryIndex == 0)
			return 0;
		return History[(HistoryIndex - offset - 1) % kMaxHistory];
	}

	std::string 	Name;
	// Union to cover z80 and m6502 + int value for loading/saving
	union {
		EFunctionParamSourceZ80		Z80Source;
		EFuctionParamSourceM6502	M6502Source;
		int							SourceIntValue = 0;
	};

	const FDisplayTypeBase*	pDisplayType = nullptr;	// display type for this parameter

	uint16_t 				HistoryIndex = 0;	// index of last value in history
	static const int		kMaxHistory = 16;	// max history size
	uint16_t				History[kMaxHistory];	// list of values for this parameter
};

// This contains information on a function in the code
// It stores the address range of the function and any exit points
struct FFunctionInfo
{
	FAddressRef		StartAddress;
	FAddressRef		EndAddress;
	std::vector<FFunctionParam>	Params;
	std::vector<FFunctionParam>	ReturnValues;
	std::vector<FCPUFunctionCall>	CallPoints;	// points in the function where a call is made
	std::vector<FAddressRef>	ExitPoints;	// points in the function where a return is made
	std::string		Name;
	std::string		Description;
	bool bManualEdit = false;
	bool bROMFunction = false;

	bool IsVisited() const { return StartAddress != EndAddress; }

	bool AddCallPoint(FCPUFunctionCall callPoint)
	{
		// check if call point already exists
		if (std::find(CallPoints.begin(), CallPoints.end(), callPoint) != CallPoints.end())
			return false;

		CallPoints.push_back(callPoint);
		return true;
	}

	void RegisterExecutionPoint(FAddressRef address)
	{
		if (bManualEdit)
			return;

		if (address.Address > EndAddress.Address && address.BankId == EndAddress.BankId)
			EndAddress = address;
	}

	bool AddExitPoint(FAddressRef exitPoint)
	{
		if (std::find(ExitPoints.begin(), ExitPoints.end(), exitPoint) == ExitPoints.end())
		{
			//RegisterExecutionPoint(exitPoint);
			ExitPoints.push_back(exitPoint);
			return true;
		}
		return false;
	}

	void OnCalled(FCodeAnalysisState& state);
	void OnReturned(FCodeAnalysisState& state);
};

// This class holds a collection of functions
// It also contains methods to find functions based on an address
class FFunctionInfoCollection
{
public:

	void Clear()
	{
		Functions.clear();
	}

	bool DoesFunctionExist(FAddressRef address) const
	{
		return Functions.find(address) != Functions.end();
	}

	bool AddFunction(const FFunctionInfo& function)
	{
		if (DoesFunctionExist(function.StartAddress))
			return false;

		Functions[function.StartAddress] = function;
		return true;
	}
	/*
	bool CreateNewFunctionAtAddress(FAddressRef address, const char* pName)
	{
		if (DoesFunctionExist(address))
			return false;
		FFunctionInfo newFunction;
		newFunction.StartAddress = address;
		newFunction.EndAddress = address;
		newFunction.Name = pName;
		Functions[address] = newFunction;
		return true;
	}*/

	FFunctionInfo* GetFunctionBeforeAddress(FAddressRef address)
	{
		auto it = Functions.upper_bound(address);
		if (it != Functions.begin())
		{
			--it;
			if (address >= it->second.StartAddress)// && address <= it->second.EndAddress)
			{
				return &it->second;
			}
		}
		return nullptr;
	}

	FFunctionInfo* GetFunctionAtAddress(FAddressRef startAddress)
	{
		auto it = Functions.find(startAddress);
		if (it != Functions.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	const FFunctionInfo* GetFunctionAtAddress(FAddressRef startAddress) const
	{
		auto it = Functions.find(startAddress);
		if (it != Functions.end())
		{
			return &it->second;
		}
		return nullptr;
	}


	FFunctionInfo* FindFunction(FAddressRef address)
	{
		auto it = Functions.upper_bound(address);
		if (it != Functions.begin())
		{
			--it;
			if (address >= it->second.StartAddress && address <= it->second.EndAddress)
			{
				return &it->second;
			}
		}
		return nullptr;
	}

	const std::map<FAddressRef, FFunctionInfo>& GetFunctions() const { return Functions; }

private:
	std::map<FAddressRef, FFunctionInfo> Functions;
};