#include "TcpServer.h"
#include "KNSLog.h"

int TCPServer::m_ClassNum = 0;
//����̓e�X�g�ł�
/*
* �@�\:�C�x���g�ƃ\�P�b�g�̊֘A�t���N���X�@�R���X�g���N�^
* ����:�Ȃ�
* ���l:
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
* �@�\:�C�x���g�ƃ\�P�b�g�̊֘A�t���N���X�@�f�X�g���N�^
* ����:�Ȃ�
* ���l:
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
* �@�\:�A�N�Z�v�g�v�����쐬����
* ����:SOCKET s �T�[�o�[�̃\�P�b�g
* ���l:ServerLoop���N�����ɌĂяo���̂ŁC�p������l�͌Ăяo���K�v�Ȃ�
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
* �@�\:�f�[�^��M�v�����쐬����
* ����:SOCKET c �N���C�A���g�̃\�P�b�g
*      size_t buflen ��M�������f�[�^�̃T�C�Y
* ���l:AcceptResponse�́CRecvResponse�CSendResponse�̒�����Ăяo���z��
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
* �@�\:�f�[�^���M�v�����쐬����
* ����:SOCKET c �N���C�A���g�̃\�P�b�g
*      byte *pSendData ���M����f�[�^�B�f�[�^�͖{API�ŃR�s�[����̂�
*                      �R�[�����͑��M�f�[�^���J�����Ă悢�B
*      size_t buflen ��M�������f�[�^�̃T�C�Y
* ���l:AcceptResponse�́CRecvResponse�CSendResponse�̒�����Ăяo���z��
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
			//�Â��f�[�^��擪�ɃR�s�[
			memcpy(p, pEvSock->pSendData, pEvSock->SendSize);
			//�V�����f�[�^�����ɒǉ�
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
* �@�\:���M�v���Ŏ󂯂��f�[�^��񓯊������ő��M����
* ����:SOCKET c �N���C�A���g�̃\�P�b�g
*      EventSock *pEvSock �C�x���g�\�P�b�g�֘A�t���N���X
* ���l:���M�f�[�^�𑗂�؂ꂽ��SendResponse���Ă�
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
* �@�\:��M�v���Ŏ󂯂��f�[�^��񓯊������Ŏ�M����
* ����:SOCKET c �N���C�A���g�̃\�P�b�g
*      EventSock *pEvSock �C�x���g�\�P�b�g�֘A�t���N���X
* ���l:�Ȃ�
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
		//�������s�����炢
	}
	SAFE_DELETE_ARRAY(pbyte);
}

/*
* �@�\:�C�x���g���L�[�Ƀ\�P�b�g����������
* ����:HANDLE hEvent �\�P�b�g�Ɋ֘A�Â����C�x���g
* ���l:�Ȃ�
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
* �@�\:���C�����[�v�B�{�֐����Ăяo���ƌp�������N���X��Response�֐����Ăяo��
* ����:�Ȃ�
* ���l:�Ȃ�
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

	/* �\�P�b�g�쐬 */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		dwLastErr = WSAGetLastError();
		printf("%d\n", dwLastErr);
		nRet = -1;
		goto FIN;
	}

	/* �\�P�b�g�I�v�V�����ݒ� */
	nRet = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (nRet == SOCKET_ERROR)
	{
		nRet = -1;
		goto FIN;
	}
	/* �o�C���h */
	nRet = bind(s, (LPSOCKADDR)&m_sAddr, sizeof(m_sAddr));
	if (nRet == SOCKET_ERROR)
	{
		nRet = -1;
		goto FIN;
	}

	/* ���b�X���ݒ� */
	nRet = listen(s, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		nRet = -1;
		goto FIN;
	}
	/* �C�x���g�̍쐬 */
	AcceptRequest(s);
	/* �\�P�b�g�ƃC�x���g�̊֘A�Â� */
	KNSLog::OutputLog("�ʐM�҂��ł�");
	while (TRUE)
	{
		/* �C�x���g�ƃ\�P�b�g�̊֘A�Â� */
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
			/* �ڑ��؂ꂪ�����瑼�̗v���͑S���J�b�g */

			if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
				{
					/* �A�N�Z�v�g */
					AcceptData(soc);
				}
			}
			if (NetworkEvents.lNetworkEvents & FD_READ)
			{
				/* ������SSL�̕����Ȃǂ��s�� */
				if (NetworkEvents.iErrorCode[FD_READ_BIT] == 0)
				{
					RecvData(soc);
				}
			}
			if (NetworkEvents.lNetworkEvents & FD_WRITE)
			{
				/* ������SSL�̕����Ȃǂ��s�� */
				if (NetworkEvents.iErrorCode[FD_WRITE_BIT] == 0)
				{
					nRet = SendData(soc, m_MapEvSock[soc]);
					if (nRet == -1)
					{
						//�������݃G���[
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
			KNSLog::OutputLog("�^�C���A�E�g���܂���");
		}
		else
		{
			/* �G���[ */
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
* �@�\:�A�N�Z�v�g�������ɌĂяo���B�p���O��
* ����:SOCKET c �N���C�A���g�\�P�b�g
* ���l:�Ȃ�
*/
void TCPServer::AcceptResponse(SOCKET c)
{
	/* �p���O�� */

	RecvRequest(c, 100);
}
/*
* �@�\:�f�[�^��M���ɌĂяo���B�p���O��
* ����:SOCKET c �N���C�A���g�\�P�b�g
*      byte *pData ��M�o�b�t�@
*      size_t bytelen ��M�T�C�Y
* ���l:�Ȃ�
*/
void TCPServer::RecvResponse(SOCKET c, byte *pData, size_t bytelen)
{
	/* �p���O�� �ȉ��̓e�X�g�R�[�h */
	printf("%s,%llu\n", pData, bytelen);
	string senddata = "test send";

	SendRequest(c, (byte*)senddata.c_str(), senddata.length());
}

/*
* �@�\:�f�ڑ��؂ꎞ�ɌĂяo���B�p���O��
* ����:SOCKET c �N���C�A���g�\�P�b�g
* ���l:�\�P�b�g�͌Ăяo����������̂Ŗ{�֐��Ń\�P�b�g����Ȃ��Ă悢
*/
void TCPServer::CloseResponse(SOCKET c)
{
	/* �p���O�� */
}
