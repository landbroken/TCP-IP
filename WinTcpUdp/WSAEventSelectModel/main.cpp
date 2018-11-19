// AsyncTcpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ClientBase.h"
#include <iostream>

using namespace std;

const int PORT = 8888;
const char LOCALIP[10] = "127.0.0.1";

class Client : public ClientBase
{
public:
	// ���ӹر�
	virtual void OnConnectionClosed()
	{
		printf("   Close\n");
	}
	// �����Ϸ�������
	virtual void OnConnectionError()
	{
		printf("   Error\n");
	}
	// ���������
	virtual void OnRecvCompleted(BYTE* buffer, int len)
	{
		printf("recv[%d]:%s\n", len, (char*)buffer);
	}
	// д�������
	virtual void OnSendCompleted()
	{
		printf("*Send success\n");
	}

};

int main()
{
	Client client;
	if (!client.Start(LOCALIP, PORT))
	{
		cout << "   start error" << endl;
	}

	int i = 0;
	while (true)
	{
		char buff[128];
		//scanf_s("%s", &buff, 128);

		sprintf_s(buff, 128, "��%d��Msg", i++);
		//cout << buff << "��" << i++ << "��Msg" << endl;

		Sleep(1000);
		client.Send((BYTE*)buff, strlen(buff) + 1);
	}
}