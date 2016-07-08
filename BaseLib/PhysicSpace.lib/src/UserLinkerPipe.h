/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _USERLINKERPIPE_H__
#define _USERLINKERPIPE_H__


#include "SpaceMutex.h"
#include "LinkerPipe.h"


#if defined(USING_POCO)

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
using namespace Poco::Net;

#elif defined(USING_WIN32)

#include <WINSOCK2.H>
#pragma comment (lib,"ws2_32.lib")

#else 
#error "Lack of specified platform #define(eg.USING_POCO or USING_WIN32 or other UserSpace implementation relied on your OS,default using USING_POCO because of it is cross-platform"

#endif


namespace PHYSIC
{

class Model;
class System;

class CUserLinkerPipe : public CLinkerPipe  
{
	//SUPPORT_ABSTRACT_SAPCE_POOL(CUserLinkerPipe);

private:
	bool          m_bDeleteMutex;

public:

#if defined(USING_POCO)
	StreamSocket  m_Socket;

#elif defined(USING_WIN32)
	SOCKET        m_Socket;
	AnsiString    m_IP;

#endif

public:
	CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name);         //used for connecting server
	CUserLinkerPipe(CSpaceMutex* Mutex,System* Parent,int64 SourceID);  //used for accepted client connection

	virtual ~CUserLinkerPipe();

	virtual bool   IsValid();
	virtual void   Close();
	tstring GetIP();

private:
	bool  PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag=0);
	bool  PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag=0);
		
};

//With active connection ability
class CUserConnectLinkerPipe:public CUserLinkerPipe
{
	//SUPPORT_ABSTRACT_SAPCE_POOL(CUserConnectLinkerPipe);

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
	bool Connect(tstring& error);

};

}//end namespace PHYSIC

#endif // _USERLINKERPIPE_H__