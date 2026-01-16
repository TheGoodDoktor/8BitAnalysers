#pragma once
#include "MCPTransport.h"
#include "MCPServer.h"


enum class EMCPTransportType
{
	Stdio,
	HTTP,
};



class FMCPManager
{
public:
	FMCPManager(FMCPToolsRegistry* pTools, FMCPResourceRegistry* pResources) : pToolsRegistry(pTools), pResourcesRegistry(pResources) {}

	~FMCPManager()
	{
	}

	void SetTransportType(EMCPTransportType type, int port = 7777)
	{
		TransportType = type;
		Port = port;
	}

	void Start()
	{
		if(pMCPServer && pMCPServer->IsRunning())
			return;

		CommandQueue.Clear();
		ResponseQueue.Reset();

		// Create transport
		FMCPTransport* pTransport = nullptr;
		if (TransportType == EMCPTransportType::Stdio)
		{
			pTransport = new FStdioTransport();
		}
		else if (TransportType == EMCPTransportType::HTTP)
		{
			pTransport = new FHttpTransport(Port);
		}

		pMCPServer = new FMCPServer(pTransport, pToolsRegistry, pResourcesRegistry, CommandQueue, ResponseQueue);
		pMCPServer->Start();
	}

	void Stop()
	{
		if (pMCPServer)
		{
			pMCPServer->Stop();

			if(pMCPServer->GetTransport() != nullptr)
				pMCPServer->GetTransport()->Close();

			delete pMCPServer;
			pMCPServer = nullptr;
		}
	}

	bool IsRunning() const
	{
		return pMCPServer && pMCPServer->IsRunning();
	}

	int GetTransportType() const
	{
		return (int)TransportType;
	}

	void ProcessCommands()
	{
		FMCPCommand* pCmd = nullptr;
		while ((pCmd = CommandQueue.Pop()) != nullptr)
		{
			FMCPResponse* pResponse = new FMCPResponse();
			pResponse->RequestId = pCmd->RequestId;
			pResponse->bIsError = false;

			pResponse->Result = pMCPServer->ExecuteCommand(pCmd->ToolName, pCmd->Arguments);

			if(pResponse->Result.contains("error"))
			{
				pResponse->bIsError = true;
				pResponse->ErrorCode = -32603;
				pResponse->ErrorMessage = pResponse->Result["error"];
			}

			ResponseQueue.Push(pResponse);
			delete pCmd;
			pCmd = nullptr;
		}
	}

private:
	FMCPServer*			pMCPServer = nullptr;
	FMCPToolsRegistry*	pToolsRegistry = nullptr;
	FMCPResourceRegistry* pResourcesRegistry = nullptr;
	FMCPCommandQueue		CommandQueue;
	FMCPResponseQueue		ResponseQueue;
	EMCPTransportType	TransportType = EMCPTransportType::Stdio;

	int			Port = 7777;

};