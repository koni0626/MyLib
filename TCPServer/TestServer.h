#pragma once
#include "TCPServer.h"
class TestServer :
	public TCPServer
{
public:
	TestServer(unsigned short usPort) ;
	TestServer();
	~TestServer();
	virtual void AcceptResponse(SOCKET c);
	virtual void RecvResponse(SOCKET c, byte *pData, size_t bytelen);
	//virtual void SendResponse(SOCKET c);
	virtual void CloseResponse(SOCKET c);
};

