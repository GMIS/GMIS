// SocketLinkerPipe.h: interface for the CUserLinkerPipe class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERLINKERPIPE_H__CBADBA55_64F7_4884_B2D2_3BA004C4CE71__INCLUDED_)
#define AFX_USERLINKERPIPE_H__CBADBA55_64F7_4884_B2D2_3BA004C4CE71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "SpaceMutex.h"
#include "LinkerPipe.h"
#include <WINSOCK2.H>

#pragma comment (lib,"ws2_32.lib")

class CUserModel;
class CUserSystem;

class CUserLinkerPipe : public CLinkerPipe  
{
	SUPPORT_ABSTRACT_SAPCE_POOL(CUserLinkerPipe);

private:
	bool          m_bDeleteMutex;
protected:
	SOCKET        m_Socket;
	AnsiString    m_IP;
public:
	CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name);
	CUserLinkerPipe(CSpaceMutex* Mutex,CUserSystem* Parent,int64 SourceID); 

	virtual ~CUserLinkerPipe();

	void AttachSocket(SOCKET sc);
    SOCKET  GetSocket();

	virtual bool   IsValid();
	virtual void   Close();

	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize);

private:
	//����ʵ�ʴ�����ֽ���
	uint32 ThreadInputProc(char* Buffer,uint32 BufSize);
	//��m_OutputPort��ȡ����Ϣ��������ַ�����ʽ׼�����
    uint32 ThreadOutputProc(char* Buffer,uint32 BufSize);

	bool  PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag=0);
	bool  PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag=0);
		
};

//�����������ӹ���
class CUserConnectLinkerPipe:public CUserLinkerPipe
{
	SUPPORT_ABSTRACT_SAPCE_POOL(CUserConnectLinkerPipe);

public:
	int32		m_TimeOut;
	bool		m_bConnected;
	AnsiString  m_Address;
	int32       m_Port;
public:
	CUserConnectLinkerPipe(Model* Parent,int64 SourceID,AnsiString Address,int32 Port,int32 TimeOut=10);
	virtual ~CUserConnectLinkerPipe();
    bool Init(tstring& error);

	bool BlockConnect(tstring& error);
	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize);
private:
	void Connect();

};

//����Accept����
class CUserAcceptLinkerPipe:public CUserLinkerPipe{
	SUPPORT_ABSTRACT_SAPCE_POOL(CUserAcceptLinkerPipe);

protected:
	int32  m_Port;
public:
	CUserAcceptLinkerPipe(CUserSystem* Parent,int64 SourceID, int32 Port);
	virtual ~CUserAcceptLinkerPipe();
	bool Init(tstring& error);
	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize);
	
private:
	void Accept();
};

#endif // !defined(AFX_USERLINKERPIPE_H__CBADBA55_64F7_4884_B2D2_3BA004C4CE71__INCLUDED_)
