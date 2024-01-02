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
	/// <param name="nCmd">�ӿͻ��˽��ܵ�cmd</param>
	/// <param name="lstPacket">�������ͻ��˵�list</param>
	/// <param name="inPacket">�ӿͻ����õ���packet</param>
	/// <returns></returns>
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket);

	/// <summary>
	/// �þ�̬����,����ʵ���޹�,Run()���ò���Ҫ��������
	/// </summary>
	/// <param name="arg">main()�д�����command�����ַ</param>
	/// <param name="status">�ӿͻ��˽��ܵ�cmd</param>
	/// <param name="lstPacket">�������ͻ��˵�list</param>
	/// <param name="inPacket">�ӿͻ����õ���packet</param>
	static void RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket) {
		CCommand* thiz = (CCommand*)arg;  //main����������cmd����
		if (status > 0) {
			int ret = thiz->ExcuteCommand(status, lstPacket, inPacket);
			if (ret != 0) {
				TRACE("ִ������ʧ�ܣ�%d ret=%d\r\n", status, ret);
			}
		}
		else {
			MessageBox(NULL, _T("�޷����������û����Զ�����"), _T("�����û�ʧ�ܣ�"), MB_OK | MB_ICONERROR);
		}
	}

protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPacket);//��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFunction; //������ŵ����ܵ�ӳ���
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

