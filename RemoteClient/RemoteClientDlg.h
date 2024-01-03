
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"
#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER+2) //发送包数据应答
#endif

// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
	// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
	void LoadFileInfo();
private:
	bool m_isClosed; //监视是否关闭
private:
	/// <summary>
	/// 控制端作出反应，处理响应业务
	/// </summary>
	/// <param name="nCmd">cmd</param>
	/// <param name="strData">数据包</param>
	/// <param name="lParam">某些数据结构</param>
	void DealCommand(WORD nCmd, const std::string& strData, LPARAM lParam);
	void InitUIData();
	void LoadFileCurrent();
	void Str2Tree(const std::string& drivers, CTreeCtrl& tree);
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);
	void UpdateDownloadFile(const std::string& strData, FILE* pFile);
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	/// <summary>
	/// 接受线程中的数据，作相应窗口显示处理
	/// </summary>
	/// <param name="wParam">服务端发来的数据</param>
	/// <param name="lParam">发送消息附带的数据</param>
	/// <returns></returns>
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);
	//自定义消息函数
	//点击“连接测试”
	afx_msg void OnBnClickedBtnTest();
	//查看“查看文件信息”
	afx_msg void OnBnClickedBtnFileinfo();
	//双击“文件树”
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	//单击“文件树”
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	//展开“文件列表”
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	//点击“下载文件”
	afx_msg void OnDownloadFile();
	//点击“删除文件”
	afx_msg void OnDeleteFile();
	//点击“打开文件”
	afx_msg void OnRunFile();
	//点击“远程监控”
	afx_msg void OnBnClickedBtnStartWatch();
	//更改IP
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	//更改端口
	afx_msg void OnEnChangeEditPort();
public:
	DWORD m_server_address;
	CString m_nPort;
	CTreeCtrl m_Tree;
	CListCtrl m_List;
};
