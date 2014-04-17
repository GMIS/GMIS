// UserSystem.h: interface for the CUserSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERSYSTEM_H__BD1D0519_113C_4099_8C34_602FEE3F3FFC__INCLUDED_)
#define AFX_USERSYSTEM_H__BD1D0519_113C_4099_8C34_602FEE3F3FFC__INCLUDED_

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
		//һ���񾭶���Ϣ�Ĵ��������û���Ҫ�Լ�ʵ��
	};
	
	virtual System::CNerveWork* CreateNerveWorker(int64 ID,System* Parent,uint32 Reason);

public:
	CUserSystem(CSystemInitData* InitData);
	~CUserSystem();
	virtual bool Activation();
	virtual void Dead();

	//����ͬʱ���Ӷ������������ַ�Ͷ˿���ͬ�����
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

	//��һ���˿ڲ�listen,����ͬʱ�򿪶��ض˿ڣ��˿��ظ������
	bool OpenPort(int32 Port,tstring& error,bool bIP6);
    void ClosePort(int32 Port);

	//����Accept()
	CUserLinkerPipe* CreateClientLinkerPipe();

};

#endif // !defined(AFX_USERSYSTEM_H__BD1D0519_113C_4099_8C34_602FEE3F3FFC__INCLUDED_)
