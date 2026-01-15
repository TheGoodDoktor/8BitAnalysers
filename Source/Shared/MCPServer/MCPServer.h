#pragma once

#include <string>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "json.hpp"
#include "MCPTransport.h"

class FMCPToolsRegistry;

struct FResourceInfo
{
	std::string		Uri;
	std::string		Title;
	std::string		Description;
	std::string		MimeType;
	std::string		Category;
	std::string		FilePath;
};

struct FMCPCommand
{
	int64_t			RequestId;
	std::string		ToolName;
	nlohmann::json	Arguments;
};

struct FMCPResponse
{
	int64_t			RequestId;
	bool			bIsError;
	int 			ErrorCode;
	std::string		ErrorMessage;
	nlohmann::json	Result;
};


class FMCPCommandQueue
{
public:
	void Push(FMCPCommand* pCmd)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Queue.push(pCmd);
	}

	FMCPCommand* Pop()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		if (Queue.empty())
		{
			return nullptr;
		}
		FMCPCommand* pCmd = Queue.front();
		Queue.pop();
		return pCmd;
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		while (!Queue.empty())
		{
			FMCPCommand* pCmd = Queue.front();
			Queue.pop();
			delete pCmd;
		}
	}

private:
	std::queue<FMCPCommand*>	Queue;
	std::mutex				Mutex;
};

class FMCPResponseQueue
{
public:
	void Push(FMCPResponse* pResp)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Queue.push(pResp);
		ConditionVariable.notify_one();
	}

	FMCPResponse* WaitAndPop()
	{
		std::unique_lock<std::mutex> lock(Mutex);
		ConditionVariable.wait(lock, [this]() { return !Queue.empty() || !bRunning; });
		
		if (Queue.empty())
			return nullptr;

		FMCPResponse* pResp = Queue.front();
		Queue.pop();
		return pResp;
	}

	void Stop()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		bRunning = false;
		ConditionVariable.notify_all();
	}

	void Reset()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		while (!Queue.empty())
		{
			FMCPResponse* pResp = Queue.front();
			Queue.pop();
			delete pResp;
		}

		bRunning = true;
	}

private:
	std::queue<FMCPResponse*>	Queue;
	std::mutex				Mutex;
	std::condition_variable	ConditionVariable;
	bool					bRunning = true;
};


class FMCPServer
{
public:
	FMCPServer(FMCPTransport* pTransport,
		FMCPToolsRegistry* toolsRegistry,
		FMCPCommandQueue& commandQueue,
		FMCPResponseQueue& responseQueue);

	~FMCPServer();

	void Start();
	void Stop();

	bool IsRunning() const
	{
		return bRunning.load();
	}

	FMCPTransport* GetTransport() const
	{
		return pTransport;
	}

	nlohmann::json ExecuteCommand(const std::string& toolName, const nlohmann::json& arguments);

	void ReaderLoop();
private:
	void Run();
	void HandleLine(const std::string& line);
	void HandleInitialise(const nlohmann::json& request);
	void HandleToolsList(const nlohmann::json& request);
	void HandleToolsCall(const nlohmann::json& request);
	void HandleResourcesList(const nlohmann::json& request);
	void HandleResourcesRead(const nlohmann::json& request);

	void LoadResources();
	void LoadResourcesFromCategory(const std::string& category, const std::string& tocPath);
	std::string ReadFileContents(const std::string& filePath);

	void SendResponse(const nlohmann::json& response);
	void SendError(int64_t id, int code, const std::string& message, const nlohmann::json& data = nlohmann::json::object());
private:
	FMCPTransport*		pTransport = nullptr;
	FMCPToolsRegistry*	pToolsRegistry = nullptr;
	FMCPCommandQueue&		CommandQueue;
	FMCPResponseQueue&		ResponseQueue;

	std::thread			Thread;
	std::atomic<bool>	bRunning;
	bool				bInitialized;


	std::vector<FResourceInfo>				Resources;
	std::map<std::string, FResourceInfo>	ResourceMap;
};