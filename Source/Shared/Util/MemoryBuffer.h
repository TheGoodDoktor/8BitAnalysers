#pragma once 

#include <string>

class FMemoryBuffer
{
public:
	~FMemoryBuffer();
	void	Init(size_t initialSize = 1024);
	void	Init(const void* pData, size_t dataSize);
	bool	Finished() const { return ReadPosition == CurrentSize; }
	void	ResetPosition() { ReadPosition = 0; }
	void	WriteBytes(const void* pData, size_t noBytes);
	bool	ReadBytes(void* Dest, size_t noBytes);

	template <class T>
	void	Write(T item) { WriteBytes(&item, sizeof(T)); }
	void	WriteString(const std::string& str)
	{
		Write<uint16_t>((uint16_t)str.size());
		WriteBytes(str.c_str(), str.size());
	}

	template <class T>
	bool	Read(T& item) { return ReadBytes(&item, sizeof(T)); }
	template <class T>
	T	Read() { T item;  ReadBytes(&item, sizeof(T)); return item; }

	std::string	ReadString(int noChars)
	{
		std::string str;
		str.resize(noChars);
		ReadBytes(&str[0], noChars);
		return str;
	}

	std::string	ReadString(void)
	{
		std::string str;
		uint16_t stringSize = Read<uint16_t>();
		str.resize(stringSize);
		ReadBytes(&str[0], stringSize);
		return str;
	}

	bool LoadFromFile(const char* pFileName);
	bool SaveToFile(const char* pFileName) const;
private:
	bool	bReadOnly = false;
	size_t	AllocationSize = 0;
	size_t	CurrentSize = 0;
	size_t	ReadPosition = 0;
	void* BasePtr = nullptr;
};