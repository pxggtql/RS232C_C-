#include "rs232c.h"

#include <stdio.h>
#include <string.h>

#include <WinSock2.h>
#include <windows.h>
rs232c::rs232c()
{ }
rs232c::~rs232c()
{ }
bool rs232c::open(const char* portname)
{
	HANDLE hCom = NULL;

	hCom= CreateFileA(portname, //串口名
		GENERIC_READ | GENERIC_WRITE, //支持读写
		0, //独占方式，串口不支持共享
		NULL,//安全属性指针，默认值为NULL
		OPEN_EXISTING, //打开现有的串口文件
		0, //0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
		NULL);//用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL
	if (hCom == (HANDLE)-1)
	{
		return false;
	}
	if (!SetupComm(hCom, 1024, 1024))
	{
		return false;
	}
	DCB p;
	memset(&p, 0, sizeof(p));
	p.DCBlength = sizeof(p);
	p.BaudRate = 9600; // 波特率
	p.ByteSize = 8; // 数据位
	p.Parity = ODDPARITY; //奇校验
	p.StopBits = ONESTOPBIT; //1位停止位
	if (!SetCommState(hCom, &p))
	{
		// 设置参数失败
		return false;
		printf("open port error!\n");
	}
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 1000; //读间隔超时
	TimeOuts.ReadTotalTimeoutMultiplier = 500; //读时间系数
	TimeOuts.ReadTotalTimeoutConstant = 5000; //读时间常量
	TimeOuts.WriteTotalTimeoutMultiplier = 500; // 写时间系数
	TimeOuts.WriteTotalTimeoutConstant = 2000; //写时间常量
	SetCommTimeouts(hCom, &TimeOuts);

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);//清空串口缓冲区

	memcpy(phandle, &hCom, sizeof(hCom));// 保存句柄

	return true;
}

void rs232c::close()
{
	HANDLE hCom = *(HANDLE*)phandle;
	CloseHandle(hCom);
}

int rs232c::send(const void* buf, int len)
{
	HANDLE hCom = *(HANDLE*)phandle;
	DWORD dwBytesWrite = len; //成功写入的数据字节数
	BOOL bWriteStat = WriteFile(hCom, //串口句柄
		buf, //数据首地址
		dwBytesWrite, //要发送的数据字节数
		&dwBytesWrite, //DWORD*，用来接收返回成功发送的数据字节数
		NULL); //NULL为同步发送，OVERLAPPED*为异步发送
	if (!bWriteStat)
	{
		return 0;
	}
	return dwBytesWrite;
}

int rs232c::receive(void* buf, int maxlen)
{
	HANDLE hCom = *(HANDLE*)phandle;
	DWORD wCount = maxlen; //成功读取的数据字节数
	BOOL bReadStat = ReadFile(hCom, //串口句柄
		buf, //数据首地址
		wCount, //要读取的数据最大字节数
		&wCount, //DWORD*,用来接收返回成功读取的数据字节数
		NULL); //NULL为同步发送，OVERLAPPED*为异步发送
	if (!bReadStat)
	{
		return 0;
	}
	return wCount;
	return 0;
}
