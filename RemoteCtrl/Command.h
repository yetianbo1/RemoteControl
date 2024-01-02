#pragma once
#include "Resource.h"
#include <map>
#include <atlimage.h>
#include "Packet.h"
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <list>
#include "EdoyunTool.h"
#include "LockInfoDialog.h"
#pragma warning(disable:4966) // fopen sprintf strcpy strstr 
class CCommand
{
public:
	CCommand();
	~CCommand() {}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="nCmd">从客户端接受到cmd</param>
	/// <param name="lstPacket">反馈给客户端的list</param>
	/// <param name="inPacket">从客户端拿到的packet</param>
	/// <returns></returns>
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket);

	/// <summary>
	/// 用静态函数,与类实例无关,Run()调用不需要创建对象
	/// </summary>
	/// <param name="arg">main()中创建的command对象地址</param>
	/// <param name="status">从客户端接受到cmd</param>
	/// <param name="lstPacket">反馈给客户端的list</param>
	/// <param name="inPacket">从客户端拿到的packet</param>
	static void RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket) {
		CCommand* thiz = (CCommand*)arg;  //main函数创建的cmd对象
		if (status > 0) {
			int ret = thiz->ExcuteCommand(status, lstPacket, inPacket);
			if (ret != 0) {
				TRACE("执行命令失败：%d ret=%d\r\n", status, ret);
			}
		}
		else {
			MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
		}
	}

protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPacket);//成员函数指针
	std::map<int, CMDFUNC> m_mapFunction; //从命令号到功能的映射表
	CLockInfoDialog dlg;
	unsigned threadid;
protected:
	static unsigned __stdcall threadLockDlg(void* arg);
	void threadLockDlgMain();
	int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int DownloadFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int MouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int UnlockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket);
	int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
};

