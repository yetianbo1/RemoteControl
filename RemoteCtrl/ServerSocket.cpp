#include "pch.h"
#include "ServerSocket.h"

CServerSocket* CServerSocket::m_instance = NULL;  //懒汉式单例模式
CServerSocket::CHelper CServerSocket::m_helper;
CServerSocket* pserver = CServerSocket::getInstance();

int CServerSocket::Run(SOCKET_CALLBACK callback, void* arg, short port) {
	bool ret = InitSocket(port);
	if (ret == false)return -1;
	std::list<CPacket> lstPackets;
	int count = 0;
	//m_callback = callback;
	while (true) {
		if (AcceptClient() == false) {
			if (count >= 3) {
				return -2;
			}
			count++;
		}
		int ret = DealCommand();  //从客户端接受到cmd
		if (ret > 0) {
			callback(arg, ret, lstPackets, m_packet);    // RunCommand
			while (lstPackets.size() > 0) {
				Send(lstPackets.front());
				lstPackets.pop_front();
			}
		}
		CloseClient();
	}
	return 0;
}

bool CServerSocket::InitSocket(short port) {
	if (m_sock == -1) return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = INADDR_ANY;
	serv_adr.sin_port = htons(port);
	//绑定
	if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
		return false;
	}
	if (listen(m_sock, 1) == -1) {
		return false;
	}
	return true;
}

bool CServerSocket::AcceptClient() {
	TRACE("enter AcceptClient\r\n");
	sockaddr_in client_adr;
	int cli_sz = sizeof(client_adr);
	m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
	TRACE("m_client = %d\r\n", m_client);
	if (m_client == -1)return false;
	return true;
}

int CServerSocket::DealCommand() {
	if (m_client == -1)return -1;
	//char buffer[1024] = "";
	char* buffer = new char[BUFFER_SIZE];
	if (buffer == NULL) {
		TRACE("内存不足！\r\n");
		return -2;
	}
	memset(buffer, 0, BUFFER_SIZE);
	size_t index = 0;
	while (true) {
		size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
		if (len <= 0) {
			delete[]buffer;
			return -1;
		}
		TRACE("recv %d\r\n", len);
		index += len;
		len = index;
		m_packet = CPacket((BYTE*)buffer, len);
		if (len > 0) {
			memmove(buffer, buffer + len, BUFFER_SIZE - len);
			index -= len;
			delete[]buffer;
			return m_packet.sCmd;
		}
	}
	delete[]buffer;
	return -1;
}

bool CServerSocket::Send(const char* pData, int nSize) {
	if (m_client == -1)return false;
	return send(m_client, pData, nSize, 0) > 0;
}

bool CServerSocket::Send(CPacket& pack) {
	if (m_client == -1) return false;
	//Dump((BYTE*)pack.Data(), pack.Size());
	return send(m_client, pack.Data(), pack.Size(), 0) > 0;
}

void CServerSocket::CloseClient() {
	if (m_client != INVALID_SOCKET) {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
}