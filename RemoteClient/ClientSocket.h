#pragma once
#include "pch.h"
#include "framework.h"
#include "EdoyunTool.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#define WM_SEND_PACK (WM_USER+1) //发送包数据
#define WM_SEND_PACK_ACK (WM_USER+2) //发送包数据应答
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
	/// 将客户端的数据打包，准备发给服务端
	/// </summary>
	/// <param name="nCmd">命令</param>
	/// <param name="pData">数据</param>
	/// <param name="nSize">数据大小</param>
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
	/// 解析从服务端(被控制端)发来的包
	/// </summary>
	/// <param name="pData">被控制端发来的数据</param>
	/// <param name="nSize">tcp缓冲区接收到pData的大小(最多1300字节)，输出代表解析后指向的位置(如果正常，则为包末尾的数据)，等于0表示解析失败</param>
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全，或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包未完全接收到，就返回，解析失败
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
	
	//包数据的大小
	int Size() {
		return nLength + 6;
	}

	/// <summary>
	/// 打包好，供send使用。转换成const char*
	/// </summary>
	/// <returns></returns> const char* 的包数据
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
	WORD sHead;//固定位 0xFEFF
	DWORD nLength;//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
};
#pragma pack(pop)

enum {
	CSM_AUTOCLOSE = 1,//CSM = Client Socket Mode 自动关闭模式
};

class CClientSocket
{
public:
	static CClientSocket* getInstance();

	/// <summary>
	/// socket、connect
	/// </summary>
	/// <returns></returns>
	bool InitSocket();

#define BUFFER_SIZE 4096000
	int DealCommand();

	/// <summary>
	/// 原始包二次封装，通过消息发送给处理线程
	/// </summary>
	/// <param name="hWnd">应答窗口</param>
	/// <param name="pack">打包好的数据</param>
	/// <param name="isAutoClosed">是否自动关闭窗口</param>
	/// <param name="wParam">某些数据结构，默认为0</param>
	/// <returns>发送是否成功</returns>
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
	void threadFunc2();  //消息处理线程
	//初始化sock版本
	BOOL InitSockEnv();
	static void releaseInstance();
	bool Send(const char* pData, int nSize);
	bool Send(const CPacket& pack);
	/// <summary>
	/// 线程收到消息后，send之后等待recv
	/// </summary>
	/// <param name="nMsg">消息</param>
	/// <param name="wParam">二次封装数据包</param>
	/// <param name="lParam">窗口</param>
	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	static CClientSocket* m_instance;
	HANDLE m_eventInvoke;//启动事件
	UINT m_nThreadID;
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapFunc;
	HANDLE m_hThread;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	int m_nIP;//地址
	int m_nPort;//端口
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