#include <iostream>
using namespace std;
#include "rs232c.h"
void send()
{
	rs232c p;
	if (p.open("COM1"))
	{
		for (int i = 0; i < 10; i++)
		{
			p.send("helloworld", 10);
		}
		cout << "send demo finished...";
	}
}
void receive()
{
	rs232c w;
	if (w.open("COM1"))
	{
		char buf[1024];
		while (true)
		{
			memset(buf, 0, 1024);
			w.receive(buf, 1024);
			cout << buf;
		}
	}
}
int main()
{
	send();
	receive();
	return 0;
}