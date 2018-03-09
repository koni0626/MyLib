#include "TCPClient.h"

int TCPClient::m_ClassNum = 0;

TCPClient::TCPClient()
{
	WSADATA wsaData = { 0 };
	if (m_ClassNum == 0)
	{
		WSAStartup(MAKEWORD(2, 0), &wsaData);
		m_ClassNum++;
	}
}

SOCKET TCPClient::Connect(string strIPAddr,unsigned short usPort)
{
	#if 0
	unsigned long ulDestAddr = 0;
	SOCKADDR_IN destSockAddr = { 0 };
	SOCKET c = 0;
	int nRet = 0;
	HANDLE hEvent = INVALID_HANDLE_VALUE;
	DWORD dwResult = 0;
	DWORD dwRet = 0;
	int nIndex = 0;
	WSANETWORKEVENTS NetworkEvents = { 0 };
	char szRecvBuf[1024] = { 0 };

//	ulDestAddr = inet_addr(strIPAddr.c_str());
	memcpy(&destSockAddr.sin_addr, &ulDestAddr, sizeof(ulDestAddr));
	destSockAddr.sin_port = htons(usPort);
	destSockAddr.sin_family = AF_INET;

	c = socket(AF_INET, SOCK_STREAM, 0);

	hEvent = WSACreateEvent();
	dwRet = WSAEventSelect(1, hEvent, FD_CONNECT|FD_CLOSE|FD_READ);
	dwResult = WSAWaitForMultipleEvents(1, &hEvent, FALSE, 5000, FALSE);
	if (dwResult - WSA_WAIT_EVENT_0 == 0)
	{
		WSAEnumNetworkEvents(c, hEvent, &NetworkEvents);
		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			shutdown(c, SD_BOTH);
			closesocket(c);
		}
		if (NetworkEvents.lNetworkEvents & FD_CONNECT)
		{
			if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] == 0)
			{
			}
		}
		if (NetworkEvents.lNetworkEvents & FD_READ)
		{
			if (NetworkEvents.iErrorCode[FD_READ_BIT] == 0)
			{
			}
		}
		if (NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (NetworkEvents.iErrorCode[FD_WRITE_BIT] == 0)
			{
			}
		}
	}
	else if (dwResult == WSA_WAIT_TIMEOUT)
	{

	}
	else
	{
		/* ÉGÉâÅ[ */
	}
//	nRet = connect(destSocket, (LPSOCKADDR)&destSockAddr, sizeof(destSockAddr));
#endif
	return 0;
}


TCPClient::~TCPClient()
{
	m_ClassNum--;
	if (m_ClassNum == 0)
	{
		WSACleanup();
	}
}
