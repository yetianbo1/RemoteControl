#pragma once
#include <Windows.h>
#include <string>
#include <atlimage.h>

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//������ƶ���˫��
	WORD nButton;//������Ҽ����м�
	POINT ptXY;//����
}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//�Ƿ���Ч
	BOOL IsDirectory;//�Ƿ�ΪĿ¼ 0 �� 1 ��
	BOOL HasNext;//�Ƿ��к��� 0 û�� 1 ��
	char szFileName[256];//�ļ���
}FILEINFO, * PFILEINFO;

//��ԭʼ�����ݶ��η�װ(ԭʼ��+�Ƿ��Զ��ر�socketģʽ���Զ��رմ��ڣ�)
typedef struct PacketData {
	std::string strData;
	UINT nMode;
	WPARAM wParam;
	/// <summary>
	/// ��ԭʼ�����ݶ��η�װ
	/// </summary>
	/// <param name="pData">ԭʼ������</param>
	/// <param name="nLen">ԭʼ����С</param>
	/// <param name="mode">�Ƿ��Զ��ر�</param>
	/// <param name="nParam">ĳЩ����</param>
	PacketData(const char* pData, size_t nLen, UINT mode, WPARAM nParam = 0) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;

class CEdoyunTool
{
public:
	static void Dump(BYTE* pData, size_t nSize){
		std::string strOut;
		for (size_t i = 0; i < nSize; i++)
		{
			char buf[8] = "";
			if (i > 0 && (i % 16 == 0))strOut += "\n";
			snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
			strOut += buf;
		}
		strOut += "\n";
		OutputDebugStringA(strOut.c_str());
	}

	static std::string GetErrInfo(int wsaErrCode){
		std::string ret;
		LPVOID lpMsgBuf = NULL;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			wsaErrCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);
		ret = (char*)lpMsgBuf;
		LocalFree(lpMsgBuf);
		return ret;
	}

	static int Bytes2Image(CImage& image, const std::string& strBuffer){
		BYTE* pData = (BYTE*)strBuffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL) {
			TRACE("�ڴ治���ˣ�\r\n");
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (hRet == S_OK) {
			ULONG length = 0;
			pStream->Write(pData, strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != NULL)
				image.Destroy();
			image.Load(pStream);
		}
		return hRet;
	}
};

