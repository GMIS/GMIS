/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSYSTEM_H__
#define _USERSYSTEM_H__

#include "UserLinkerPipe.h"
#include "System.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CUserSystem : public System 
{ 
public:
	class CUserModelIOWork: public Model::CModelIOWork{
	protected:
		HANDLE           m_hObjectDefaultThread;
		DWORD            m_hObjectDefaultThreadID;
	public:
		CUserModelIOWork(int64 ID, CUserSystem* Parent);
		~CUserModelIOWork();
		virtual bool Activation();
		virtual void Dead();
	};
	
	class CUserCentralNerveWork: public Model::CCentralNerveWork{
	protected:
		HANDLE           m_hObjectDefaultThread;
		DWORD            m_hObjectDefaultThreadID;
	public:
		CUserCentralNerveWork(int64 ID, CUserSystem* Parent);
		~CUserCentralNerveWork();
		virtual bool Activation();
		virtual void Dead();	
	};
	

	class CUserSystemIOWork: public System::CSystemIOWork{
	protected:
		HANDLE           m_hObjectDefaultThread;
		DWORD            m_hObjectDefaultThreadID;
	public:
		CUserSystemIOWork(int64 ID, CUserSystem* Parent);
		~CUserSystemIOWork();
		virtual bool Activation();
		virtual void Dead();
	};
	
	class CUserNerveWork: public System::CNerveWork{
	protected:
		HANDLE           m_hObjectDefaultThread;
		DWORD            m_hObjectDefaultThreadID;
	public:
		CUserNerveWork(int64 ID, CUserSystem* Parent);
		~CUserNerveWork();
		virtual bool Activation();
		virtual void Dead();
	};

	class CUserLockedSystemData : public System::CLockedSystemData{
	private:
		map<int32,CUserLinkerPipe*>    m_AcceptorList;
	public:
		CUserLockedSystemData(CABMutex* mutex);
		virtual ~CUserLockedSystemData();
		CUserLinkerPipe* FindAcceptor(int32 Port);
		void  AddAcceptor(int32 Port,CUserLinkerPipe* Acceptor);
		void  DelAcceptor(int32 Port);
	};
	
	friend CUserAcceptLinkerPipe;
protected:
	int32            m_nCPU;
	HANDLE           m_hObjectDefaultThread;
	DWORD            m_hObjectDefaultThreadID;

	virtual void NotifySysState(int64 NotifyID,ePipeline* Data = NULL);

    virtual void NerveProc(CMsg& Msg){
		//needs user to implement
	};
	
	virtual System::CNerveWork* CreateNerveWorker(int64 ID,System* Parent,uint32 Reason);

public:
	CUserSystem(CSystemInitData* InitData);
	~CUserSystem();
	virtual bool Activation();
	virtual void Dead();

	//Allows to connect several servers at same time, if address and port were the same then to ignore
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

	//open a port and listen, allow to  open serveral ports at same time, if port repeated then to ignore
	bool OpenPort(int32 Port,tstring& error,bool bIP6);
    void ClosePort(int32 Port);

	//used for Accept()
	CUserLinkerPipe* CreateClientLinkerPipe();

};

#endif // _USERSYSTEM_H__