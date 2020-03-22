#pragma once
class rs232c
{
public:
	
	rs232c();
	~rs232c();
	bool open(const char* portname);
	void close();
	int send(const void* buf, int len);
	int receive(void* buf, int maxlen);
private:
	int phandle[16];
};