// TCPIPLearningServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
/**************************************************
�ļ���server.cpp
windows��socket������ʵ��  -- �������˻���TCP
��������ַ��'127.0.0.1'
�˿ں� 8888
���ߣ�peter
***************************************************/
#include<WinSock2.h>  
#include<stdio.h>  
#include <Ws2tcpip.h>
#pragma comment(lib,"WS2_32.lib")  

const int PORT = 8888;

/*
��������
socket()
bind()
listen()
accept()
recv
sendto
close()
*/
int main(int argc, char* argv[])
{
	WSADATA wsaData;
	//ָ���汾��  
	WORD sockVersion = MAKEWORD(2, 0);
	//����winsock��dll  
	::WSAStartup(sockVersion, &wsaData);
										
	//�����׽��ֻ���TCP
	SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		printf("error");
		::WSACleanup();//�����ͷ���Դ  
		return 0;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	//�˿ں�8888
	sin.sin_port = htons(PORT);
	//INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ,  
	//��ַȫ��0��Ҳ�������еĵ�ַ
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	//��socket  
	if (::bind(s, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("error");
		::WSACleanup();//�����ͷ���Դ  
		return 0;
	}

	//����socket  
	if (::listen(s, 2) == SOCKET_ERROR)
	{
		printf("error");
		::WSACleanup();//�ͷ���Դ  
		return 0;
	}

	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	SOCKET client;
	char szText[] = "peter\n";//����������  

	while (1)
	{
		/*
		accept��������ʹ�ã����ú�����������״̬���ȴ��û����ӣ�
		���û�пͻ��˽������ӣ������������ط������ῴ�����档
		����пͻ������ӣ���ô�����ִ��һ��Ȼ�����ѭ��������ȴ���
		*/
		client = ::accept(s, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (client == INVALID_SOCKET)
		{
			printf("error");
			continue;
		}

		char str[INET_ADDRSTRLEN];
		printf("���ܵ�һ�����ӣ�%s\r\n", inet_ntop(AF_INET, &remoteAddr.sin_addr, str,sizeof(str)));

		::send(client, szText, strlen(szText), 0); //��������  

		::closesocket(client);//�ر��׽���  
	}

	::closesocket(s);
	::WSACleanup();
	return 0;
}