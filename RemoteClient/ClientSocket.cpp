#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

CClientSocket::CClientSocket(const CClientSocket& ss) {
	m_hThread = INVALID_HANDLE_VALUE;
	m_bAutoClose = ss.m_bAutoClose;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it != ss.m_mapFunc.end(); it++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}

CClientSocket::CClientSocket() :
	m_nIP(INADDR_ANY), 
	m_nPort(0), 
	m_sock(INVALID_SOCKET), 
	m_bAutoClose(true),
	m_hThread(INVALID_HANDLE_VALUE)
{
	if (InitSockEnv() == FALSE) {
		MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,�����������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	//ͬ������
	m_eventInvoke = CreateEvent(NULL, TRUE, FALSE, NULL);  //���������̵߳ȴ����߳�������״̬�����߳���������ʱ��Ὣ�¼�����Ϊsignaled ״̬
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT) { //����ڹ涨ʱ�����¼�û�б�Ϊ signaled ״̬
		TRACE("������Ϣ�����߳�����ʧ���ˣ�\r\n");
	}
	CloseHandle(m_eventInvoke);

	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);

	//��Ϣ-��Ӧ�����Ľṹ������
	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEND_PACK,&CClientSocket::SendPack},
		{0,NULL}
	};
	for (int i = 0; funcs[i].message != 0; i++) {
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("����ʧ�ܣ���Ϣֵ��%d ����ֵ:%08X ���:%d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}
}

CClientSocket::~CClientSocket() {
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
	WSACleanup();
}

CClientSocket* CClientSocket::getInstance() {
	if (m_instance == NULL) {//��̬����û��thisָ�룬�����޷�ֱ�ӷ��ʳ�Ա����
		m_instance = new CClientSocket();
		TRACE("CClientSocket size is %d\r\n", sizeof(*m_instance));
	}
	return m_instance;
}

bool CClientSocket::InitSocket()
{
	if (m_sock != INVALID_SOCKET) CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1)return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	TRACE("addr %08X nIP %08X\r\n", inet_addr("127.0.0.1"), m_nIP);
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("ָ����IP��ַ�������ڣ�");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1) {
		TRACE("����ʧ�ܣ�%d %s\r\n", WSAGetLastError(), CEdoyunTool::GetErrInfo(WSAGetLastError()).c_str());
		AfxMessageBox("����ʧ��!");
		return false;
	}
	TRACE("socket init done!\r\n");
	return true;
}

int CClientSocket::DealCommand() {
	if (m_sock == -1) return -1;
	char* buffer = m_buffer.data();
	static size_t index = 0;
	while (true) {
		size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
		if (((int)len <= 0) && ((int)index <= 0)) {
			return -1;
		}
		TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
		index += len;
		len = index;
		TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
		m_packet = CPacket((BYTE*)buffer, len);
		TRACE("command %d\r\n", m_packet.sCmd);
		if (len > 0) {
			memmove(buffer, buffer + len, index - len);
			index -= len;
			return m_packet.sCmd;
		}
	}
	return -1;
}

bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed, WPARAM wParam){
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;   // �Ƿ��Զ��ر�socket�������Ҫ�Զ��رմ��ڣ�����Ҫ�Զ��ر�socket
	std::string strOut;
	pack.Data(strOut);
	//���η�װ������
	PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
	bool ret = PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)pData, (LPARAM)hWnd);
	if (ret == false) delete pData;
	return ret;
}

bool CClientSocket::GetFilePath(std::string& strPath) {
	if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
		strPath = m_packet.strData;
		return true;
	}
	return false;
}

bool CClientSocket::GetMouseEvent(MOUSEEV& mouse) {
	if (m_packet.sCmd == 5) {
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
		return true;
	}
	return false;
}

void CClientSocket::CloseSocket() {
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}

void CClientSocket::UpdateAddress(int nIP, int nPort) {
	if ((m_nIP != nIP) || (m_nPort != nPort)) {
		m_nIP = nIP;
		m_nPort = nPort;
	}
}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}

void CClientSocket::threadFunc2(){
	SetEvent(m_eventInvoke);  // ����Ϊsignal
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {   //���ϵȴ�����WM_SEND_PACK��Ϣ
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		TRACE("Get Message :%08X \r\n", msg.message);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

BOOL CClientSocket::InitSockEnv() {
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
		return FALSE;
	}
	return TRUE;
}

void CClientSocket::releaseInstance() {
	TRACE("CClientSocket has been called!\r\n");
	if (m_instance != NULL) {
		CClientSocket* tmp = m_instance;
		m_instance = NULL;
		delete tmp;
		TRACE("CClientSocket has released!\r\n");
	}
}

bool CClientSocket::Send(const char* pData, int nSize) {
	if (m_sock == -1)return false;
	return send(m_sock, pData, nSize, 0) > 0;
}

bool CClientSocket::Send(const CPacket& pack)
{
	TRACE("m_sock = %d\r\n", m_sock);
	if (m_sock == -1)return false;
	std::string strOut;
	pack.Data(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam){
	PACKET_DATA data = *(PACKET_DATA*)wParam;  //ȡ�����η�װ������ݰ�
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	size_t nTemp = data.strData.size();  //ԭʼ��
	CPacket current((BYTE*)data.strData.c_str(), nTemp); //���
	if (InitSocket() == true) {
		int ret = send(m_sock, (char*)data.strData.c_str(), (int)data.strData.size(), 0); //���͵������
		if (ret > 0) {
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sock != INVALID_SOCKET) {
				int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);  // �ӷ���˽���
				if (length > 0 || (index > 0)) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						TRACE("ack pack %d to hWnd %08X %d %d\r\n", pack.sCmd, hWnd, index, nLen);
						TRACE("%04X\r\n", *(WORD*)(pBuffer + nLen));
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);   //����Ӧ����Ϣ
						if (data.nMode & CSM_AUTOCLOSE) {
							CloseSocket();
							return;
						}
						//�����Զ��رմ��ڣ�������������
						index -= nLen;
						memmove(pBuffer, pBuffer + nLen, index);
					}
				}
				else {//�Է��ر����׽��֣����������豸�쳣
					TRACE("recv failed length %d index %d cmd %d\r\n", length, index, current.sCmd);
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(current.sCmd, NULL, 0), 1);
				}
			}
		}
		else {
			CloseSocket();
			//������ֹ����
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else {
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
	}
}