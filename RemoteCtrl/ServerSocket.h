#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"

//callback����
typedef void (*SOCKET_CALLBACK)(void* ,int, std::list<CPacket>&, CPacket&);

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {//��̬����û��thisָ�룬�����޷�ֱ�ӷ��ʳ�Ա����
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	
	/// <summary>
	/// ��ѭ���ṹ:socket��bind��listen��accept��read��write��close
	/// </summary>
	/// <param name="callback">�ص���RunCommand</param>
	/// <param name="arg">����cmd�����ַ�����ص�����ʹ��</param>
	/// <param name="port">�˿ںţ�Ĭ��9527</param>
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
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,�����������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
		if (m_instance != NULL) {//�����Ա��
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

