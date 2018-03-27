#include "TcpServer.h"
#include "KNSLog.h"

int TCPServer::m_ClassNum = 0;
//これはテストです
/*
* 機能:イベントとソケットの関連付けクラス　コンストラクタ
* 引数:なし
* 備考:
*/
EventSock::EventSock()
{
	pSendData = NULL;
	hEvent = INVALID_HANDLE_VALUE;
	NetWorkEvent = 0;
	RecvSize = 0;
	SendSize = 0;
	RemainSendSize = 0;
}

/*
* 機能:イベントとソケットの関連付けクラス　デストラクタ
* 引数:なし
* 備考:
*/
EventSock::~EventSock()
{
	if (pSendData != NULL)
	{
		delete[] pSendData;
		pSendData = NULL;
	}
}

TCPServer::TCPServer()
{
	WSADATA wsaData = { 0 };
	if (m_ClassNum == 0)
	{
		WSAStartup(MAKEWORD(2, 0), &wsaData);
		m_ClassNum++;
	}
	m_Port = 44444;
	m_dwTimeOut = 5000;
}

TCPServer::TCPServer(unsigned short usPort)
{
	WSADATA wsaData = { 0 };
	if (m_ClassNum == 0)
	{
		WSAStartup(MAKEWORD(2, 0), &wsaData);
		m_ClassNum++;
	}
	m_Port = usPort;
	m_dwTimeOut = 5000;
}

EventSock * TCPServer::GetEventSock(SOCKET soc)
{
	EventSock *pEvSock = NULL;
	if (m_MapEvSock.find(soc) != m_MapEvSock.end())
	{
		pEvSock = m_MapEvSock[soc];
	}
	else
	{
		pEvSock = new EventSock;
	}
	return pEvSock;
}

void TCPServer::DeleteEventSock(SOCKET soc)
{
	delete m_MapEvSock[soc];
	m_MapEvSock.erase(soc);
}
/*
* 機能:アクセプト要求を作成する
* 引数:SOCKET s サーバーのソケット
* 備考:ServerLoopが起動時に呼び出すので，継承する人は呼び出す必要なし
*/
void TCPServer::AcceptRequest(SOCKET s)
{
	EventSock *pEvSock = NULL;

	pEvSock = GetEventSock(s);

	if (pEvSock->hEvent == INVALID_HANDLE_VALUE)
	{
		pEvSock->hEvent = WSACreateEvent();

	}
	pEvSock->NetWorkEvent = pEvSock->NetWorkEvent|FD_ACCEPT | FD_CLOSE;
	WSAEventSelect(s, pEvSock->hEvent, pEvSock->NetWorkEvent);

	m_MapEvSock[s] = pEvSock;
}

/*
* 機能:データ受信要求を作成する
* 引数:SOCKET c クライアントのソケット
*      size_t buflen 受信したいデータのサイズ
* 備考:AcceptResponseの，RecvResponse，SendResponseの中から呼び出す想定
*/
void TCPServer::RecvRequest(SOCKET c,size_t buflen)
{
	EventSock *pEvSock = NULL;
	pEvSock = GetEventSock(c);


	if (pEvSock->hEvent == INVALID_HANDLE_VALUE)
	{
		pEvSock->hEvent = WSACreateEvent();

	}
	pEvSock->NetWorkEvent = pEvSock->NetWorkEvent|FD_READ | FD_CLOSE;
	pEvSock->RecvSize = buflen;
	WSAEventSelect(c, pEvSock->hEvent, pEvSock->NetWorkEvent);
	m_MapEvSock[c] = pEvSock;
}

/*
* 機能:データ送信要求を作成する
* 引数:SOCKET c クライアントのソケット
*      byte *pSendData 送信するデータ。データは本APIでコピーするので
*                      コール元は送信データを開放してよい。
*      size_t buflen 受信したいデータのサイズ
* 備考:AcceptResponseの，RecvResponse，SendResponseの中から呼び出す想定
*/
void TCPServer::SendRequest(SOCKET c, byte *pSendData, size_t buflen)
{
	EventSock *pEvSock = NULL;
	int nRet = 0;
	DWORD dwLastErr = 0;

	try
	{
		pEvSock = GetEventSock(c);

		if (pEvSock->hEvent == INVALID_HANDLE_VALUE)
		{
			pEvSock->hEvent = WSACreateEvent();

		}
		if (pEvSock->pSendData == NULL)
		{
			pEvSock->pSendData = new byte[buflen];
			pEvSock->SendSize = buflen;
			pEvSock->RemainSendSize = buflen;
			memcpy(pEvSock->pSendData, pSendData, buflen);
		}
		else
		{
			byte *p = new byte[pEvSock->SendSize + buflen];
			//古いデータを先頭にコピー
			memcpy(p, pEvSock->pSendData, pEvSock->SendSize);
			//新しいデータを後ろに追加
			memcpy(&p[pEvSock->SendSize], pSendData, buflen);
			pEvSock->SendSize = pEvSock->SendSize + buflen;
			pEvSock->RemainSendSize = pEvSock->RemainSendSize + buflen;
			delete[] pEvSock->pSendData;
			pEvSock->pSendData = p;
		}


		pEvSock->NetWorkEvent = pEvSock->NetWorkEvent | FD_WRITE | FD_CLOSE;
		WSAEventSelect(c, pEvSock->hEvent, pEvSock->NetWorkEvent);
		m_MapEvSock[c] = pEvSock;
		SendData(c, pEvSock);
	}
	catch (...)
	{

	}
}

void TCPServer::AcceptData(SOCKET s)
{
	SOCKET c = INVALID_SOCKET;
	int addrLen = sizeof(SOCKADDR_IN);
	c = accept(s, (LPSOCKADDR)&m_sAddr, &addrLen);
	this->AcceptResponse(c);

}
/*
* 機能:送信要求で受けたデータを非同期処理で送信する
* 引数:SOCKET c クライアントのソケット
*      EventSock *pEvSock イベントソケット関連付けクラス
* 備考:送信データを送り切れたらSendResponseを呼ぶ
*/
int TCPServer::SendData(SOCKET s, EventSock *pEvSock)
{
	int sendLen = 0;
	int Remain = 0;
	DWORD dwLastErr = 0;
	char *p = NULL;
	int nRet = 0;
	if (pEvSock->RemainSendSize == 0)
	{
		nRet = 1;
		goto FIN;
	}
	p = (char *)&pEvSock->pSendData[pEvSock->SendSize - pEvSock->RemainSendSize];
	sendLen = send(s, p, (int)pEvSock->SendSize, 0);
	if (sendLen != SOCKET_ERROR)
	{
		pEvSock->RemainSendSize = pEvSock->SendSize - sendLen;
		if (pEvSock->RemainSendSize == 0)
		{
			nRet = 0;
			pEvSock->SendSize = 0;
			delete pEvSock->pSendData;
			pEvSock->pSendData = NULL;
			goto FIN;
		}
	}
	else
	{
		dwLastErr = WSAGetLastError();
		printf("%d\n", dwLastErr);
		if (dwLastErr == WSAEWOULDBLOCK)
		{
			nRet = 1;
		}
		else
		{
			nRet = -1;
		}
	}
FIN:
	return nRet;
}

/*
* 機能:受信要求で受けたデータを非同期処理で受信する
* 引数:SOCKET c クライアントのソケット
*      EventSock *pEvSock イベントソケット関連付けクラス
* 備考:なし
*/
void TCPServer::RecvData(SOCKET s)
{
	size_t recvLen = 0;
	byte *pbyte = NULL;
	
	try
	{
		pbyte = new byte[m_MapEvSock[s]->RecvSize + 1];

		memset(pbyte, 0, m_MapEvSock[s]->RecvSize + 1);
		recvLen = recv(s, (char *)pbyte, (int)m_MapEvSock[s]->RecvSize, 0);
		if (recvLen != SOCKET_ERROR)
		{
			this->RecvResponse(s, (byte*)pbyte, recvLen);
		}
	}
	catch (...)
	{
		//メモリ不足くらい
	}
	SAFE_DELETE_ARRAY(pbyte);
}

/*
* 機能:イベントをキーにソケットを検索する
* 引数:HANDLE hEvent ソケットに関連づいたイベント
* 備考:なし
*/
SOCKET TCPServer::SearchSocket(HANDLE hEvent)
{
	SOCKEVENTMAP::iterator m_itr;
	SOCKET s = INVALID_SOCKET;
	for (m_itr = m_MapEvSock.begin(); m_itr != m_MapEvSock.end(); m_itr++)
	{
		if (hEvent == m_itr->second->hEvent)
		{
			s = m_itr->first;
			break;
		}
	}
	return s;
}

void TCPServer::CloseEventSocket(SOCKET s)
{
	HANDLE hEvent = INVALID_HANDLE_VALUE;
	if (m_MapEvSock.find(s) == m_MapEvSock.end())
	{
		return;
	}
	hEvent = m_MapEvSock[s]->hEvent;
	shutdown(s, SD_BOTH);
	closesocket(s);
	WSAResetEvent(hEvent);
	WSACloseEvent(hEvent);
	DeleteEventSock(s);
}
/*
* 機能:メインループ。本関数を呼び出すと継承したクラスのResponse関数を呼び出す
* 引数:なし
* 備考:なし
*/
int NmnTCPServer::ServerLoop()
{
	int nRet = 0;
	DWORD dwResult = 0;
	SOCKET s = INVALID_SOCKET;
	SOCKET soc = INVALID_SOCKET;
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS] = { INVALID_HANDLE_VALUE };
	WSAEVENT hEvent = 0;
	WSANETWORKEVENTS NetworkEvents = { 0 };
	int recvsize = 0;
	int optval = 1;
	DWORD dwEventNum = 0;
	int nIndex = 0;
	DWORD dwLastErr = 0;

	m_sAddr.sin_port = htons(m_Port);
	m_sAddr.sin_family = AF_INET;
	m_sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* ソケット作成 */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		dwLastErr = WSAGetLastError();
		printf("%d\n", dwLastErr);
		nRet = -1;
		goto FIN;
	}

	/* ソケットオプション設定 */
	nRet = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (nRet == SOCKET_ERROR)
	{
		nRet = -1;
		goto FIN;
	}
	/* バインド */
	nRet = bind(s, (LPSOCKADDR)&m_sAddr, sizeof(m_sAddr));
	if (nRet == SOCKET_ERROR)
	{
		nRet = -1;
		goto FIN;
	}

	/* リッスン設定 */
	nRet = listen(s, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		nRet = -1;
		goto FIN;
	}
	/* イベントの作成 */
	AcceptRequest(s);
	/* ソケットとイベントの関連づけ */
	KNSLog::OutputLog("通信待ちです");
	while (TRUE)
	{
		/* イベントとソケットの関連づけ */
		SOCKEVENTMAP::iterator m_itr;
		dwEventNum = 0;
		for (m_itr = m_MapEvSock.begin(); m_itr != m_MapEvSock.end(); m_itr++)
		{
			EventArray[dwEventNum] = m_itr->second->hEvent;
			dwEventNum++;
			if (m_itr->second->RemainSendSize != 0)
			{
				SendData(m_itr->first, m_itr->second);
			}
			
			if (dwEventNum > WSA_MAXIMUM_WAIT_EVENTS)
			{
				break;
			}
		}
		

		dwResult = WSAWaitForMultipleEvents(dwEventNum, EventArray, FALSE, m_dwTimeOut, FALSE);
		if ((dwResult - WSA_WAIT_EVENT_0) >= 0 && (dwResult - WSA_WAIT_EVENT_0) < WSA_MAXIMUM_WAIT_EVENTS)
		{
			nIndex = dwResult - WSA_WAIT_EVENT_0;
			hEvent = EventArray[nIndex];
			soc = SearchSocket(hEvent);
			WSAEnumNetworkEvents(soc, hEvent, &NetworkEvents);
			/* 接続切れが来たら他の要求は全部カット */

			if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
				{
					/* アクセプト */
					AcceptData(soc);
				}
			}
			if (NetworkEvents.lNetworkEvents & FD_READ)
			{
				/* ここでSSLの復号などを行う */
				if (NetworkEvents.iErrorCode[FD_READ_BIT] == 0)
				{
					RecvData(soc);
				}
			}
			if (NetworkEvents.lNetworkEvents & FD_WRITE)
			{
				/* ここでSSLの復号などを行う */
				if (NetworkEvents.iErrorCode[FD_WRITE_BIT] == 0)
				{
					nRet = SendData(soc, m_MapEvSock[soc]);
					if (nRet == -1)
					{
						//書き込みエラー
						CloseEventSocket(soc);
					}

				}
			}
			if (NetworkEvents.lNetworkEvents & FD_CLOSE)
			{
				CloseEventSocket(soc);
				continue;
			}
		}
		else if (dwResult == WSA_WAIT_TIMEOUT)
		{
			KNSLog::OutputLog("タイムアウトしました");
		}
		else
		{
			/* エラー */
		}
	}

FIN:
	if (nRet != 0)
	{
		if (s != INVALID_SOCKET)
		{
			shutdown(s,SD_BOTH);
			closesocket(s);
		}
	}
	return nRet;
}

TCPServer::~TCPServer()
{
	m_ClassNum--;
	if (m_ClassNum == 0)
	{
		WSACleanup();
	}
}

/*
* 機能:アクセプト完了時に呼び出す。継承前提
* 引数:SOCKET c クライアントソケット
* 備考:なし
*/
void TCPServer::AcceptResponse(SOCKET c)
{
	/* 継承前提 */

	RecvRequest(c, 100);
}
/*
* 機能:データ受信時に呼び出す。継承前提
* 引数:SOCKET c クライアントソケット
*      byte *pData 受信バッファ
*      size_t bytelen 受信サイズ
* 備考:なし
*/
void TCPServer::RecvResponse(SOCKET c, byte *pData, size_t bytelen)
{
	/* 継承前提 以下はテストコード */
	printf("%s,%llu\n", pData, bytelen);
	string senddata = "test send";

	SendRequest(c, (byte*)senddata.c_str(), senddata.length());
}

/*
* 機能:デ接続切れ時に呼び出す。継承前提
* 引数:SOCKET c クライアントソケット
* 備考:ソケットは呼び出し側が閉じるので本関数でソケットを閉じなくてよい
*/
void TCPServer::CloseResponse(SOCKET c)
{
	/* 継承前提 */
}
