#pragma once
#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__
#include <winsock2.h>
#include <windows.h>
#include <string>
using namespace std;

class TCPClient
{
private:
	static int m_ClassNum;
public:
	TCPClient();
	SOCKET Connect(string strIPAddr,unsigned short usPort);
	~TCPClient();
};
#endif


