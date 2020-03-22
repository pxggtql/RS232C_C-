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

	//�첽��ʽ
	hCom = CreateFileA(portname, //������
		GENERIC_READ | GENERIC_WRITE, //֧�ֶ�д
		0, //��ռ��ʽ�����ڲ�֧�ֹ���
		NULL,//��ȫ����ָ�룬Ĭ��ֵΪNULL
		OPEN_EXISTING, //�����еĴ����ļ�
		FILE_FLAG_OVERLAPPED, //0��ͬ����ʽ��FILE_FLAG_OVERLAPPED���첽��ʽ
		NULL);//���ڸ����ļ������Ĭ��ֵΪNULL���Դ��ڶ��Ըò���������ΪNULL
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
	p.BaudRate = 9600; // ������
	p.ByteSize = 8; // ����λ
	p.Parity = ODDPARITY; //��У��
	p.StopBits = ONESTOPBIT; //1λֹͣλ
	if (!SetCommState(hCom, &p))
	{
		// ���ò���ʧ��
		return false;
		printf("open port error!\n");
	}
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 1000; //�������ʱ
	TimeOuts.ReadTotalTimeoutMultiplier = 500; //��ʱ��ϵ��
	TimeOuts.ReadTotalTimeoutConstant = 5000; //��ʱ�䳣��
	TimeOuts.WriteTotalTimeoutMultiplier = 500; // дʱ��ϵ��
	TimeOuts.WriteTotalTimeoutConstant = 2000; //дʱ�䳣��
	SetCommTimeouts(hCom, &TimeOuts);

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);//��մ��ڻ�����

	memcpy(phandle, &hCom, sizeof(hCom));// ������

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
	DWORD dwBytesWrite = len; //�ɹ�д��������ֽ���
	DWORD dwErrorFlags; //�����־
	COMSTAT comStat; //ͨѶ״̬
	OVERLAPPED m_osWrite; //�첽��������ṹ��

	//����һ������OVERLAPPED���¼��������������õ�����ϵͳҪ����ô��
	memset(&m_osWrite, 0, sizeof(m_osWrite));
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, L"WriteEvent");

	ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ���󣬻���豸��ǰ״̬
	BOOL bWriteStat = WriteFile(hCom, //���ھ��
		buf, //�����׵�ַ
		dwBytesWrite, //Ҫ���͵������ֽ���
		&dwBytesWrite, //DWORD*���������շ��سɹ����͵������ֽ���
		&m_osWrite); //NULLΪͬ�����ͣ�OVERLAPPED*Ϊ�첽����
	if (!bWriteStat)
	{
		if (GetLastError() == ERROR_IO_PENDING) //�����������д��
		{
			WaitForSingleObject(m_osWrite.hEvent, 1000); //�ȴ�д���¼�1����
		}
		else
		{
			ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ����
			CloseHandle(m_osWrite.hEvent); //�رղ��ͷ�hEvent�ڴ�
			return 0;
		}
	}
	return dwBytesWrite;

}

int rs232c::receive(void* buf, int maxlen)
{
	HANDLE hCom = *(HANDLE*)phandle;
	//�첽��ʽ
	DWORD wCount = maxlen; //�ɹ���ȡ�������ֽ���
	DWORD dwErrorFlags; //�����־
	COMSTAT comStat; //ͨѶ״̬
	OVERLAPPED m_osRead; //�첽��������ṹ��

	//����һ������OVERLAPPED���¼��������������õ�����ϵͳҪ����ô��
	memset(&m_osRead, 0, sizeof(m_osRead));
	m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, L"ReadEvent");

	ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ���󣬻���豸��ǰ״̬
	if (!comStat.cbInQue)return 0; //������뻺�����ֽ���Ϊ0���򷵻�false

	BOOL bReadStat = ReadFile(hCom, //���ھ��
		buf, //�����׵�ַ
		wCount, //Ҫ��ȡ����������ֽ���
		&wCount, //DWORD*,�������շ��سɹ���ȡ�������ֽ���
		&m_osRead); //NULLΪͬ�����ͣ�OVERLAPPED*Ϊ�첽����
	if (!bReadStat)
	{
		if (GetLastError() == ERROR_IO_PENDING) //����������ڶ�ȡ��
		{
			//GetOverlappedResult���������һ��������ΪTRUE
			//������һֱ�ȴ���ֱ����������ɻ����ڴ��������
			GetOverlappedResult(hCom, &m_osRead, &wCount, TRUE);
		}
		else
		{
			ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ����
			CloseHandle(m_osRead.hEvent); //�رղ��ͷ�hEvent���ڴ�
			return 0;
		}
	}
	return wCount;
}
