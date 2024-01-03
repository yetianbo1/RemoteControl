#pragma once
#include "pch.h"
#include "framework.h"
#include "EdoyunTool.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#define WM_SEND_PACK (WM_USER+1) //���Ͱ�����
#define WM_SEND_PACK_ACK (WM_USER+2) //���Ͱ�����Ӧ��
#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	~CPacket() {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	/// <summary>
	/// ���ͻ��˵����ݴ����׼�����������
	/// </summary>
	/// <param name="nCmd">����</param>
	/// <param name="pData">����</param>
	/// <param name="nSize">���ݴ�С</param>
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}

	/// <summary>
	/// �����ӷ����(�����ƶ�)�����İ�
	/// </summary>
	/// <param name="pData">�����ƶ˷���������</param>
	/// <param name="nSize">tcp���������յ�pData�Ĵ�С(���1300�ֽ�)��������������ָ���λ��(�����������Ϊ��ĩβ������)������0��ʾ����ʧ��</param>
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//�����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ�
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//��δ��ȫ���յ����ͷ��أ�����ʧ��
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			TRACE("%s\r\n", strData.c_str() + 12);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;//head2 length4 data...
			return;
		}
		nSize = 0;
	}
	
	//�����ݵĴ�С
	int Size() {
		return nLength + 6;
	}

	/// <summary>
	/// ����ã���sendʹ�á�ת����const char*
	/// </summary>
	/// <returns></returns> const char* �İ�����
	const char* Data(std::string& strOut) const {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)(pData) = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
public:
	WORD sHead;//�̶�λ 0xFEFF
	DWORD nLength;//�����ȣ��ӿ������ʼ������У�������
	WORD sCmd;//��������
	std::string strData;//������
	WORD sSum;//��У��
};
#pragma pack(pop)

enum {
	CSM_AUTOCLOSE = 1,//CSM = Client Socket Mode �Զ��ر�ģʽ
};

class CClientSocket
{
public:
	static CClientSocket* getInstance();

	/// <summary>
	/// socket��connect
	/// </summary>
	/// <returns></returns>
	bool InitSocket();

#define BUFFER_SIZE 4096000
	int DealCommand();

	/// <summary>
	/// ԭʼ�����η�װ��ͨ����Ϣ���͸������߳�
	/// </summary>
	/// <param name="hWnd">Ӧ�𴰿�</param>
	/// <param name="pack">����õ�����</param>
	/// <param name="isAutoClosed">�Ƿ��Զ��رմ���</param>
	/// <param name="wParam">ĳЩ���ݽṹ��Ĭ��Ϊ0</param>
	/// <returns>�����Ƿ�ɹ�</returns>
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed = true, WPARAM wParam = 0);
	bool GetFilePath(std::string& strPath);
	bool GetMouseEvent(MOUSEEV& mouse);
	CPacket& GetPacket() { return m_packet;}
	void CloseSocket();
	void UpdateAddress(int nIP, int nPort);
private:
	CClientSocket();
	~CClientSocket();
	CClientSocket& operator=(const CClientSocket& ss) {}
	CClientSocket(const CClientSocket& ss);
	static unsigned __stdcall threadEntry(void* arg);
	void threadFunc2();  //��Ϣ�����߳�
	//��ʼ��sock�汾
	BOOL InitSockEnv();
	static void releaseInstance();
	bool Send(const char* pData, int nSize);
	bool Send(const CPacket& pack);
	/// <summary>
	/// �߳��յ���Ϣ��send֮��ȴ�recv
	/// </summary>
	/// <param name="nMsg">��Ϣ</param>
	/// <param name="wParam">���η�װ���ݰ�</param>
	/// <param name="lParam">����</param>
	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	static CClientSocket* m_instance;
	HANDLE m_eventInvoke;//�����¼�
	UINT m_nThreadID;
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapFunc;
	HANDLE m_hThread;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	int m_nIP;//��ַ
	int m_nPort;//�˿�
	std::vector<char> m_buffer;
	SOCKET m_sock;
	CPacket m_packet;

private:
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};