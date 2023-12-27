// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Command.h"
#include <conio.h>
#include "CEdoyunQueue.h"
#include <MSWSock.h>
#include "EdoyunServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#pragma comment( linker, "/subsystem:windows /entry:WinMainCRTStartup" )
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
//#pragma comment( linker, "/subsystem:console /entry:mainCRTStartup" )
//#pragma comment( linker, "/subsystem:console /entry:WinMainCRTStartup" )

CWinApp theApp;
using namespace std;

//业务和通用
void iocp();
int main()
{
	if (!CEdoyunTool::Init())return 1;
	//iocp();
	CCommand cmd;
	int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
	switch (ret) {
	case -1:
		MessageBox(NULL, _T("网络初始化异常，未能成功初始，请检查网络状态！"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
		break;
	case -2:
		MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
		break;
	}
	return 0;
}

class COverlapped {
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;
	char m_buffer[4096];
	COverlapped() {
		m_operator = 0;
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		memset(m_buffer, 0, sizeof(m_buffer));
	}
};

void iocp()
{
	EdoyunServer server;
	server.StartService();
	getchar();
}