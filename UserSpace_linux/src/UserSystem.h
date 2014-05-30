/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSYSTEM_H__
#define _USERSYSTEM_H__

#include "../../TheorySpace/src/System.h"
#include "UserLinkerPipe.h"
#include <pthread.h>

class CUserSystem : public System 
{ 
public:
	class CUserModelIOWork: public Model::CModelIOWork{
	protected:
		pthread_t        m_hObjectDefaultThread;
	public:
		CUserModelIOWork(int64 ID, CUserSystem* Parent);
		~CUserModelIOWork();
		virtual bool Activation();
		virtual void Dead();
	};
	
	class CUserCentralNerveWork: public Model::CCentralNerveWork{
	protected:
		pthread_t           m_hObjectDefaultThread;
	public:
		CUserCentralNerveWork(int64 ID, CUserSystem* Parent);
		~CUserCentralNerveWork();
		virtual bool Activation();
		virtual void Dead();	
	};
	

	class CUserSystemIOWork: public System::CSystemIOWork{
	protected:
		pthread_t           m_hObjectDefaultThread;
	public:
		CUserSystemIOWork(int64 ID, CUserSystem* Parent);
		~CUserSystemIOWork();
		virtual bool Activation();
		virtual void Dead();
	};
	
	class CUserNerveWork: public System::CNerveWork{
	protected:
		pthread_t           m_hObjectDefaultThread;
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
	
	friend class CUserAcceptLinkerPipe;
protected:
	int32            m_nCPU;
	pthread_t        m_hObjectDefaultThread;

	virtual void NotifySysState(int64 NotifyID,ePipeline* Data = NULL);

    virtual void NerveProc(CMsg& Msg){
		//һ���񾭶���Ϣ�Ĵ��?���û���Ҫ�Լ�ʵ��
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

#endif // _USERSYSTEM_H__
