#pragma once
#ifndef _CLIENTBASE_H_
#define _CLIENTBASE_H_

#include <WinSock2.h>
#include <Windows.h>

// �ͷ�ָ��ĺ�
#define RELEASE(x)            {if(x != NULL) {delete x; x = NULL;}}
// �ͷž���ĺ�
void ReleaseHandle(HANDLE x);
// �ͷ�Socket
void ReleaseSocket(SOCKET x);

class ClientBase
{
public:
	ClientBase();
	~ClientBase();

	// ����ͨ��
	BOOL Start(const char *IPAddress, USHORT port);
	// �ر�ͨ��
	BOOL Stop();
	// ��������
	BOOL Send(const BYTE* buffer, int len);
	// �Ƿ�������
	BOOL HasStarted();

	// �¼�֪ͨ����(���������ش��庯��)
	// ���ӹر�
	virtual void OnConnectionClosed() = 0;
	// �����Ϸ�������
	virtual void OnConnectionError() = 0;
	// ���������
	virtual void OnRecvCompleted(BYTE* buffer, int len) = 0;
	// д�������
	virtual void OnSendCompleted() = 0;

private:
	// �����̺߳���
	static DWORD WINAPI RecvThreadProc(LPVOID lpParam);
	// socket�Ƿ���
	BOOL IsSocketAlive(SOCKET sock);
	SOCKET clientSock;
	WSAEVENT socketEvent;
	HANDLE stopEvent;
	HANDLE thread;
};

#endif // !_CLIENTBASE_H_
