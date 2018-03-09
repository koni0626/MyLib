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
	size_t RecvSize;       //��M�������f�[�^�T�C�Y
	size_t SendSize;       //���M�������f�[�^�T�C�Y
	size_t RemainSendSize;  //�����M�f�[�^�̃T�C�Y
	byte *pSendData;        //���M�f�[�^�o�b�t�@
};
typedef map<SOCKET, EventSock*> SOCKEVENTMAP;
/* WSAStartup�֐��͖{�N���X�𐶐�����O�ɌĂԂ��� */
/* WSACleanup�֐��͏I����ɌĂԂ��� */
#define SAFE_DELETE_ARRAY(p) \
{                            \
     if(p != NULL)           \
	 {                       \
		 delete[] p;         \
         p = NULL;           \
	 }                       \
}
/* �J�������F�����_�ł͎��͂Őؒf���鏈�������Ă��Ȃ��@*/
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