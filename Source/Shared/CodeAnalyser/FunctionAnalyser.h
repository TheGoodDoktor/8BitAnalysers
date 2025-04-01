#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "CodeAnalyserTypes.h"

class FCodeAnalysisState;

// we use int type so it can work with ImGui::Combo

enum class EFunctionParamTypeZ80 : int
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

enum class EFuctionParamTypeM6502 : int
{
	RegA,
	RegX,
	RegY,
};

// structure to hold function parameters
struct FFunctionParam
{
	std::string 	Name;
	// Union to cover z80 and m6502 + int value for loading/saving
	union {
		EFunctionParamTypeZ80	Z80Type;
		EFuctionParamTypeM6502	M6502Type;
		int						TypeIntValue = 0;
	};

	uint16_t	LastValue = 0;
};

// This contains information on a function in the code
// It stores the address range of the function and any exit points
struct FFunctionInfo
{
	FAddressRef		StartAddress;
	FAddressRef		EndAddress;
	std::vector<FFunctionParam>	Params;
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