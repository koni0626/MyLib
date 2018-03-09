#pragma once
#ifndef __KNSLOG_H__
#define __KNSLOG_H__
#include <string>
using namespace std;
class KNSLog
{
public:
	KNSLog();
	static void OutputLog(string msg);
	~KNSLog();
};

#endif