#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"

//callback声明
typedef void (*SOCKET_CALLBACK)(void* ,int, std::list<CPacket>&, CPacket&);

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {//静态函数没有this指针，所以无法直接访问成员变量
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	
	/// <summary>
	/// 主循环结构:socket、bind、listen、accept、read、write、close
	/// </summary>
	/// <param name="callback">回调：RunCommand</param>
	/// <param name="arg">传入cmd对象地址，供回调函数使用</param>
	/// <param name="port">端口号，默认9527</param>
	/// <returns></returns>
	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527);
protected:
	bool InitSocket(short port);
	bool AcceptClient();
#define BUFFER_SIZE 4096
	int DealCommand();
	bool Send(const char* pData, int nSize);
	bool Send(CPacket& pack);
	void CloseClient();

private:
	CServerSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}
	class CHelper {
	public:
		CHelper() {CServerSocket::getInstance();}
		~CHelper() {CServerSocket::releaseInstance();}
	};

	CServerSocket& operator=(const CServerSocket& ss) {}
	CServerSocket(const CServerSocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL) {//防御性编程
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
private:
	//SOCKET_CALLBACK m_callback;
	//void* m_arg;
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
	static CHelper m_helper;
	static CServerSocket* m_instance;
};

