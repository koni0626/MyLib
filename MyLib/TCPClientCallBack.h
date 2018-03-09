#pragma once
#ifndef __TCPCLIENTCALLBACK_H__
#define __TCPCLIENTCALLBACK_H__
#include <string>
using namespace std;
class TCPClientCallBack
{
public:
	TCPClientCallBack();
	virtual void Recv(string strRecv);
	virtual void TimeOut();

	~TCPClientCallBack();
};

#endif