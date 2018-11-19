// TCPLearningClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
/**************************************************
�ļ���client.cpp
windows��socket������ʵ��  -- �ͻ��˻���TCP
��������ַ��'127.0.0.1'
�˿ں� 8888
���ߣ�peter
***************************************************/
#include<WinSock2.h>  
#include<stdio.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "WS2_32.lib")  

const int PORT = 8888;
const char LOCALIP[10] = "127.0.0.1";

int main(int argc, char* argv[])
{
	/*
	�ͻ��˻���˳��
	socket()
	connect()
	sendto()
	readfrom()
	close()
	*/

	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);//ָ���汾��  
	::WSAStartup(sockVersion, &wsaData);//����winsock��dll  
	
	SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//����socket������tcp  
	if (s == INVALID_SOCKET)
	{
		printf("error");
		::WSACleanup(); //�ͷ���Դ  
		return 0;
	}

	sockaddr_in servAddr;
	//AF_INET��IPv4 ����Э����׽������ͣ�
	//AF_INET6 ���� IPv6 �ģ�
	//AF_UNIX ���� Unix ϵͳ����ͨ��
	servAddr.sin_family = AF_INET;
	//�˿ںţ�htons�����������ֽ�˳��ת��Ϊ�����ֽ�˳��(to network short)
	servAddr.sin_port = htons(PORT);
	//IP
	//servAddr.sin_addr.S_un.S_addr = inet_addr(LOCALIP);
	inet_pton(AF_INET, LOCALIP, &servAddr.sin_addr);
	//����
	if (::connect(s, (sockaddr*)&servAddr, sizeof(servAddr)) == -1)
	{
		printf("error");
		::WSACleanup(); //�ͷ���Դ  
		return 0;
	}

	char buff[156];//������  
	int nRecv = ::recv(s, buff, 156, 0);//��������  
	if (nRecv > 0)
	{
		buff[nRecv] = '\0';
		printf_s("�������ݣ�%s", buff);
	}
	::closesocket(s); //�ر��׽���  
	::WSACleanup(); //�ͷ���Դ

	printf_s("�Ѿ�closesocket(s)��WSACleanup()\n");
	system("pause");
	return 0;
}