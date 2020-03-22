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

	//异步方式
	hCom = CreateFileA(portname, //串口名
		GENERIC_READ | GENERIC_WRITE, //支持读写
		0, //独占方式，串口不支持共享
		NULL,//安全属性指针，默认值为NULL
		OPEN_EXISTING, //打开现有的串口文件
		FILE_FLAG_OVERLAPPED, //0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
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
	DWORD dwErrorFlags; //错误标志
	COMSTAT comStat; //通讯状态
	OVERLAPPED m_osWrite; //异步输入输出结构体

	//创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
	memset(&m_osWrite, 0, sizeof(m_osWrite));
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, L"WriteEvent");

	ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
	BOOL bWriteStat = WriteFile(hCom, //串口句柄
		buf, //数据首地址
		dwBytesWrite, //要发送的数据字节数
		&dwBytesWrite, //DWORD*，用来接收返回成功发送的数据字节数
		&m_osWrite); //NULL为同步发送，OVERLAPPED*为异步发送
	if (!bWriteStat)
	{
		if (GetLastError() == ERROR_IO_PENDING) //如果串口正在写入
		{
			WaitForSingleObject(m_osWrite.hEvent, 1000); //等待写入事件1秒钟
		}
		else
		{
			ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误
			CloseHandle(m_osWrite.hEvent); //关闭并释放hEvent内存
			return 0;
		}
	}
	return dwBytesWrite;

}

int rs232c::receive(void* buf, int maxlen)
{
	HANDLE hCom = *(HANDLE*)phandle;
	//异步方式
	DWORD wCount = maxlen; //成功读取的数据字节数
	DWORD dwErrorFlags; //错误标志
	COMSTAT comStat; //通讯状态
	OVERLAPPED m_osRead; //异步输入输出结构体

	//创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
	memset(&m_osRead, 0, sizeof(m_osRead));
	m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, L"ReadEvent");

	ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
	if (!comStat.cbInQue)return 0; //如果输入缓冲区字节数为0，则返回false

	BOOL bReadStat = ReadFile(hCom, //串口句柄
		buf, //数据首地址
		wCount, //要读取的数据最大字节数
		&wCount, //DWORD*,用来接收返回成功读取的数据字节数
		&m_osRead); //NULL为同步发送，OVERLAPPED*为异步发送
	if (!bReadStat)
	{
		if (GetLastError() == ERROR_IO_PENDING) //如果串口正在读取中
		{
			//GetOverlappedResult函数的最后一个参数设为TRUE
			//函数会一直等待，直到读操作完成或由于错误而返回
			GetOverlappedResult(hCom, &m_osRead, &wCount, TRUE);
		}
		else
		{
			ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误
			CloseHandle(m_osRead.hEvent); //关闭并释放hEvent的内存
			return 0;
		}
	}
	return wCount;
}
