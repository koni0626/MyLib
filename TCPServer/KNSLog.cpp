#include "KNSLog.h"
#include <windows.h>
#include <stdio.h>


KNSLog::KNSLog()
{
}

void KNSLog::OutputLog(string msg)
{
	SYSTEMTIME st;
	char szTime[25] = { 0 };
	string strLog;
	//YYYY/MM/DD hh:mm:ss.SSS
	GetSystemTime(&st);
	snprintf(szTime,sizeof(szTime)-1,"%04d/%02d/%02d %02d:%02d:%02d.%03d",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond,st.wMilliseconds);

	strLog = szTime;
	strLog = strLog + " ";
	strLog = strLog + msg;

	printf("%s\n", strLog.c_str());

}
KNSLog::~KNSLog()
{
}
