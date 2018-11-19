/*
==========================================================================
* �����CIOCPModel�Ǳ�����ĺ����࣬����˵��WinSock�������˱��ģ���е���ɶ˿�(IOCP)��ʹ�÷���

* ���е�IOContext���Ƿ�װ������ÿһ���ص������Ĳ���

* ����˵���˷������˽�����ɶ˿ڡ������������̡߳�Ͷ��Recv����Ͷ��Accept����ķ��������еĿͻ��������Socket����Ҫ�󶨵�IOCP�ϣ����дӿͻ��˷��������ݣ�������ûص�������

*�÷�������һ�����࣬���ػص�����

Author: TTGuoying

Date: 2018/02/07 16:22

==========================================================================
*/
#pragma once
#ifndef _IOCPBASE_H_
#define _IOCPBASE_H_

#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <vector>
#include <list>
#include <string>


#include <atlstr.h>
#include <atltime.h>
#include <locale.h>
//ʹ��ϵͳ��ȫ�ַ�������֧��
#include <strsafe.h>
//ʹ��ATL���ַ���ת��֧��
#include <atlconv.h>

using std::vector;
using std::list;
using std::wstring;

const int BUFF_SIZE = (1024 * 4);					// I/O ����Ļ�������С
const int WORKER_THREADS_PER_PROCESSOR = (2);		// ÿ���������ϵ��߳���
const int MAX_POST_ACCEPT = (10);					// ͬʱͶ�ݵ�Accept����
const int INIT_IOCONTEXT_NUM = (100);				// IOContextPool�еĳ�ʼ����
const int EXIT_CODE = (-1);							// ���ݸ�Worker�̵߳��˳��ź�
const wchar_t DEFAULT_IP[] = (L"127.0.0.1");			// Ĭ��IP��ַ
const int DEFAULT_PORT = (8888);					// Ĭ�϶˿�

// �ͷ�ָ��ĺ�
#define RELEASE(x)			{if(x != NULL) {delete x; x = NULL;}}
// �ͷž���ĺ�
#define RELEASE_HANDLE(x)	{if(x != NULL && x != INVALID_HANDLE_VALUE) { CloseHandle(x); x = INVALID_HANDLE_VALUE; }}
// �ͷ�Socket�ĺ�
#define RELEASE_SOCKET(x)	{if(x != INVALID_SOCKET) { closesocket(x); x = INVALID_SOCKET; }}
// �ͷž���ĺ�
void ReleaseHandle(HANDLE x);
// �ͷ�Socket
void ReleaseSocket(SOCKET x);

#ifndef TRACE
#include <atltrace.h>

#define TRACE							AtlTrace
#define TRACE0(f)						TRACE(f)
#define TRACE1(f, p1)					TRACE(f, p1)
#define TRACE2(f, p1, p2)				TRACE(f, p1, p2)
#define TRACE3(f, p1, p2, p3)			TRACE(f, p1, p2, p3)
#define TRACE4(f, p1, p2, p3, p4)		TRACE(f, p1, p2, p3, p4)
#define TRACE5(f, p1, p2, p3, p4, p5)	TRACE(f, p1, p2, p3, p4, p5)
#endif

enum IO_OPERATION_TYPE
{
	NULL_POSTED,		// ���ڳ�ʼ����������
	ACCEPT_POSTED,		// Ͷ��Accept����
	SEND_POSTED,		// Ͷ��Send����
	RECV_POSTED,		// Ͷ��Recv����
};

class IOContext
{
public:
	WSAOVERLAPPED		overLapped;		// ÿ��socket��ÿһ��IO��������Ҫһ���ص��ṹ
	SOCKET				ioSocket;		// ��IO������Ӧ��socket
	WSABUF				wsaBuf;			// ���ݻ���
	IO_OPERATION_TYPE	ioType;			// IO��������
	UINT				connectID;		// ����ID

	IOContext()
	{
		ZeroMemory(&overLapped, sizeof(overLapped));
		ioSocket = INVALID_SOCKET;
		wsaBuf.buf = (char *)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFF_SIZE);
		wsaBuf.len = BUFF_SIZE;
		ioType = NULL_POSTED;
		connectID = 0;
	}

	~IOContext()
	{
		RELEASE_SOCKET(ioSocket);

		if (wsaBuf.buf != NULL)
			::HeapFree(::GetProcessHeap(), 0, wsaBuf.buf);
	}

	void Reset()
	{
		if (wsaBuf.buf != NULL)
			ZeroMemory(wsaBuf.buf, BUFF_SIZE);
		else
			wsaBuf.buf = (char *)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFF_SIZE);
		ZeroMemory(&overLapped, sizeof(overLapped));
		ioType = NULL_POSTED;
		connectID = 0;
	}
};

// ���е�IOContext������(IOContext��)
class IOContextPool
{
private:
	list<IOContext *> contextList;
	CRITICAL_SECTION csLock;

public:
	IOContextPool()
	{
		InitializeCriticalSection(&csLock);
		contextList.clear();

		EnterCriticalSection(&csLock);
		for (size_t i = 0; i < INIT_IOCONTEXT_NUM; i++)
		{
			IOContext *context = new IOContext;
			contextList.push_back(context);
		}
		LeaveCriticalSection(&csLock);

	}

	~IOContextPool()
	{
		EnterCriticalSection(&csLock);
		for (list<IOContext *>::iterator it = contextList.begin(); it != contextList.end(); it++)
		{
			delete (*it);
		}
		contextList.clear();
		LeaveCriticalSection(&csLock);

		DeleteCriticalSection(&csLock);
	}

	// ����һ��IOContxt
	IOContext *AllocateIoContext()
	{
		IOContext *context = NULL;

		EnterCriticalSection(&csLock);
		if (contextList.size() > 0) //list��Ϊ�գ���list��ȡһ��
		{
			context = contextList.back();
			contextList.pop_back();
		}
		else	//listΪ�գ��½�һ��
		{
			context = new IOContext;
		}
		LeaveCriticalSection(&csLock);

		return context;
	}

	// ����һ��IOContxt
	void ReleaseIOContext(IOContext *pContext)
	{
		pContext->Reset();
		EnterCriticalSection(&csLock);
		contextList.push_front(pContext);
		LeaveCriticalSection(&csLock);
	}
};

class SocketContext
{
public:
	SOCKET connSocket;								// ���ӵ�socket
	SOCKADDR_IN clientAddr;							// ���ӵ�Զ�̵�ַ

private:
	vector<IOContext*> arrIoContext;				// ͬһ��socket�ϵĶ��IO����
	static IOContextPool ioContextPool;				// ���е�IOContext��
	/*
	�ٽ���
	��һ���߳�ִ����EnterCritialSection֮��cs�������Ϣ�㱻
	�޸ģ���ָ����һ���߳�ռ������������ʱ����û���κ���Դ��
	��������������ʲô��Դ�������̶߳����ǿ��Է��ʵģ���Ȼ��
	ִ�еĽ�������Ǵ���ģ���ֻ������������߳���δִ��
	LeaveCriticalSection֮ǰ�������߳�����EnterCritialSection
	���Ļ����ͻᴦ�ڵȴ�״̬���൱���̱߳������ˡ� �������
	�£������˱���������Դ�����á�
	*/
	CRITICAL_SECTION csLock;

public:
	SocketContext()
	{
		//�˺�����ʼ��һ���ٽ�������
		InitializeCriticalSection(&csLock);
		arrIoContext.clear();
		connSocket = INVALID_SOCKET;
		//RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
		ZeroMemory(&clientAddr, sizeof(clientAddr));
	}

	~SocketContext()
	{
		RELEASE_SOCKET(connSocket);

		// �������е�IOContext
		for (vector<IOContext*>::iterator it = arrIoContext.begin(); it != arrIoContext.end(); it++)
		{
			ioContextPool.ReleaseIOContext(*it);
		}

		EnterCriticalSection(&csLock);
		arrIoContext.clear();
		LeaveCriticalSection(&csLock);

		DeleteCriticalSection(&csLock);
	}

	// ��ȡһ���µ�IoContext
	IOContext *GetNewIOContext()
	{
		IOContext *context = ioContextPool.AllocateIoContext();
		if (context != NULL)
		{
			EnterCriticalSection(&csLock);
			arrIoContext.push_back(context);
			LeaveCriticalSection(&csLock);
		}
		return context;
	}

	// ���������Ƴ�һ��ָ����IoContext
	void RemoveContext(IOContext* pContext)
	{
		for (vector<IOContext*>::iterator it = arrIoContext.begin(); it != arrIoContext.end(); it++)
		{
			if (pContext == *it)
			{
				ioContextPool.ReleaseIOContext(*it);

				EnterCriticalSection(&csLock);
				arrIoContext.erase(it);
				LeaveCriticalSection(&csLock);

				break;
			}
		}
	}

	// 
};

// IOCP����
class IOCPBase
{
public:
	IOCPBase();
	~IOCPBase();

	// ��ʼ����
	BOOL Start(int port = 8888, int maxConn = 2000, int maxIOContextInPool = 256,
		int maxSocketContextInPool = 200);
	// ֹͣ����
	void Stop();
	// ��ָ���ͻ��˷�������
	BOOL SendData(SocketContext *socketContext, char *data, int size);

	// ��ȡ������IP
	wstring GetLocalIP();

	// ��ȡ��ǰ������
	ULONG GetConnectCnt() { return connectCnt; }

	// ��ȡ��ǰ������
	UINT GetPort() { return port; }

	// �¼�֪ͨ����(���������ش��庯��)
	// ������
	virtual void OnConnectionEstablished(SocketContext *sockContext) = 0;
	// ���ӹر�
	virtual void OnConnectionClosed(SocketContext *sockContext) = 0;
	// �����Ϸ�������
	virtual void OnConnectionError(SocketContext *sockContext, int error) = 0;
	// ���������
	virtual void OnRecvCompleted(SocketContext *sockContext, IOContext *ioContext) = 0;
	// д�������
	virtual void OnSendCompleted(SocketContext *sockContext, IOContext *ioContext) = 0;

protected:
	HANDLE					stopEvent;				// ֪ͨ�߳��˳���ʱ��
	HANDLE					completionPort;			// ��ɶ˿�
	HANDLE					*workerThreads;			// �������̵߳ľ��ָ��
	int						workerThreadNum;		// �������̵߳�����
	wstring					IP;						// ����IP
	int						port;					// �����˿�
	SocketContext			*listenSockContext;		// ����socket��Context
	LONG					connectCnt;				// ��ǰ����������
	LONG					acceptPostCnt;			// ��ǰͶ�ݵĵ�Accept����

	LPFN_ACCEPTEX			fnAcceptEx;				//AcceptEx����ָ��
													//GetAcceptExSockAddrs;����ָ��
	LPFN_GETACCEPTEXSOCKADDRS	fnGetAcceptExSockAddrs;

private:
	static DWORD WINAPI WorkerThreadProc(LPVOID lpParam); // �����̺߳���

														  // ��ʼ��IOCP
	BOOL InitializeIOCP();
	// ��ʼ��Socket
	BOOL InitializeListenSocket();
	// �ͷ���Դ
	void DeInitialize();
	// socket�Ƿ���
	BOOL IsSocketAlive(SOCKET sock);
	// ��ȡ����CPU������
	int GetNumOfProcessors();
	// �����(Socket)�󶨵���ɶ˿���
	BOOL AssociateWithIOCP(SocketContext *sockContext);
	// Ͷ��IO����
	BOOL PostAccept(SocketContext *sockContext, IOContext *ioContext);
	BOOL PostRecv(SocketContext *sockContext, IOContext *ioContext);
	BOOL PostSend(SocketContext *sockContext, IOContext *ioContext);

	// IO������
	BOOL DoAccpet(SocketContext *sockContext, IOContext *ioContext);
	BOOL DoRecv(SocketContext *sockContext, IOContext *ioContext);
	BOOL DoSend(SocketContext *sockContext, IOContext *ioContext);
	BOOL DoClose(SocketContext *sockContext);
};

#endif // !_IOCPBASE_H_

