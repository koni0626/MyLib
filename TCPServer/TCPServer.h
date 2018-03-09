#pragma once
#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__
#include <WinSock2.h>
#include <windows.h>
#include <map>
using namespace std;

class EventSock
{
public:
	HANDLE hEvent;
	long NetWorkEvent;
	EventSock();
	~EventSock();
	size_t RecvSize;       //受信したいデータサイズ
	size_t SendSize;       //送信したいデータサイズ
	size_t RemainSendSize;  //未送信データのサイズ
	byte *pSendData;        //送信データバッファ
};
typedef map<SOCKET, EventSock*> SOCKEVENTMAP;
/* WSAStartup関数は本クラスを生成する前に呼ぶこと */
/* WSACleanup関数は終了後に呼ぶこと */
#define SAFE_DELETE_ARRAY(p) \
{                            \
     if(p != NULL)           \
	 {                       \
		 delete[] p;         \
         p = NULL;           \
	 }                       \
}
/* 開発メモ：現時点では自力で切断する処理が作れていない　*/
class TCPServer
{
private:
	static int m_ClassNum;
	unsigned short m_Port;
	SOCKEVENTMAP m_MapEvSock;
	SOCKADDR_IN m_sAddr;
	DWORD m_dwTimeOut;

private:
	EventSock * GetEventSock(SOCKET s);
	void DeleteEventSock(SOCKET s);
	void AcceptRequest(SOCKET s);
	SOCKET SearchSocket(HANDLE hEvent);
	void AcceptData(SOCKET s);
	void RecvData(SOCKET s);
	int SendData(SOCKET s, EventSock *pEvSock);
	void CloseEventSocket(SOCKET s);

public:
	TCPServer();
	TCPServer(unsigned short usPort);
	void RecvRequest(SOCKET c, size_t buflen);
	void SendRequest(SOCKET c, byte *pSendData, size_t buflen);
	
	virtual void AcceptResponse(SOCKET c);
	virtual void RecvResponse(SOCKET c, byte *pData, size_t bytelen);
	//virtual void SendResponse(SOCKET c);
	virtual void CloseResponse(SOCKET c);
	int ServerLoop();
	~TCPServer();
};

#endif