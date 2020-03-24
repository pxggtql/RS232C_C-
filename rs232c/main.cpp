#include <iostream>
#include <Windows.h>
using namespace std;
#include "rs232c.h"

rs232c p;
DWORD WINAPI  send(LPVOID lpParamter)
{
	cout << "input your data:" << endl;
	while (TRUE)
	{
			char str[100] = {};
			cin >> str;
			int data_len = 0;
			data_len = strlen(str);
			p.send(str, data_len);
	}
	return 0L;
}

DWORD WINAPI  receive(LPVOID lpParamter)
{

	while (true)
	{
		char rcv_buf[1024] = {};
		p.receive(rcv_buf, 1024);

		string str = rcv_buf;
		int len =str.length();
		if (len > 0)
		{
			cout << "receive message is:" << str << endl;
		}
		else
		{
			continue;
		}
	}
	return 0L;
}
int main()
{
	char str1[10];
	HANDLE hThread[2];
	cout << "请输入要监听的串口号\n";
	scanf_s("%s", str1,10);
	p.open(str1);
	hThread[0] = CreateThread(NULL, 0, send, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, receive, NULL, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	p.close();
	CloseHandle(hThread);
	return 0;
}