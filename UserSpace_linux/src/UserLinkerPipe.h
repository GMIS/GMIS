/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERLINKERPIPE_H__
#define _USERLINKERPIPE_H__


#include "../../TheorySpace/src/SpaceMutex.h"
#include "../../TheorySpace/src/LinkerPipe.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class CUserModel;
class CUserSystem;

class CUserLinkerPipe : public CLinkerPipe  
{
	SUPPORT_ABSTRACT_SAPCE_POOL(CUserLinkerPipe);

private:
	bool          m_bDeleteMutex;
protected:
	int32         m_Socket;
	AnsiString    m_IP;
public:
	CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name);
	CUserLinkerPipe(CSpaceMutex* Mutex,CUserSystem* Parent,int64 SourceID); 

	virtual ~CUserLinkerPipe();

	void AttachSocket(int32 sc);
    int32  GetSocket();

	virtual bool   IsValid();
	virtual void   Close();

	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize);

private:
	//Returns the number of bytes actually processed
	uint32 ThreadInputProc(char* Buffer,uint32 BufSize);
    uint32 ThreadOutputProc(char* Buffer,uint32 BufSize);

	bool  PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag=0);
	bool  PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag=0);
		
};

//With active connection ability
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

//with active acception  ability
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

#endif // _USERLINKERPIPE_H__
