#include "TestServer.h"
#include "KNSLog.h"


TestServer::TestServer(unsigned short usPort) : TCPServer(usPort)
{
	//TCPServer::TCPServer(44444);
}

TestServer::TestServer()
{
}
TestServer::~TestServer()
{
}

void TestServer::AcceptResponse(SOCKET c)
{
	KNSLog::OutputLog("アクセプトを受け付けました");
	this->RecvRequest(c, 1000);
}

void TestServer::RecvResponse(SOCKET c, byte *pData, size_t bytelen)
{
	char *pszBuf = new char[bytelen + 1];
	memset(pszBuf, 0, bytelen + 1);
	strncpy(pszBuf, (char *)pData, bytelen);
	string msg = "データを受信しました";
	msg = msg + " ";
	msg = msg + pszBuf;
	KNSLog::OutputLog(msg);
	string rtn = "rtn ";
	rtn = rtn +pszBuf + "\r\n";
	this->SendRequest(c, (byte *)rtn.c_str(), rtn.size());
	delete[] pszBuf;
}

void TestServer::CloseResponse(SOCKET c)
{
	KNSLog::OutputLog("接続が切れました");

}