/*
* ����ʵ������߲�εĳ��󣬾������������������
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SYSTEM_H__
#define _SYSTEM_H__


#include "Model.h"
#include <vector>
#include <map>
#include "ABMutex.h"
#include "SpaceMutex.h"


#define SNOTIFY_NERVE_MSG_NUM               20000
#define SNOTIFY_NERVE_THREAD_JOIN			20001
#define SNOTIFY_NERVE_THREAD_CLOSE          20002
#define SNOTIFY_NERVE_THREAD_LIMIT          20003
#define SNOTIFY_NERVE_THREAD_FAIL           20004
#define SNOTIFY_LISTEN_FAIL                 20005
#define SNOTIFY_IO_WORK_THREAD_CLOSE        20006

namespace ABSTRACT{
class  System : public Model  
{
public:
	class CSystemIOWork : public Object{
	public:
		System*      m_Parent;
	public:
		CSystemIOWork(int64 ID,System* Parent);
		virtual ~CSystemIOWork();
		virtual bool Do(Energy* E);
	};

	class CNerveWork : public Object{
	protected:			
		System*	     m_Parent;
		int32        m_IdleCount;
	public:
		CNerveWork(int64 ID,System* Parent);
		virtual ~CNerveWork();
		
		virtual bool Do(Energy* E);
	};

	class CLockedSystemData{
	protected:
 		CABMutex*				   m_pMutex;
		uint32                     m_MaxNerveWorkerNum;      //������������񾭴����߳���Ŀ��default=100
		uint32                     m_NerveMsgMaxNumInPipe;   //�����������������������Ϣ��������µĴ����߳�,default = 10
		int64                      m_NerveMsgMaxInterval;    //���������ȡ������Ϣʱ�䳬�����������µĴ����߳�,default=10*1000*1000(��λ�ǰ����룬1��)
		int32                      m_NerveIdleMaxCount;	     //�����񾭴����߳������м����������˳���default=30

		uint32                     m_NerveWorkingNum;        //���ڹ����ģ�Ҳ���Ǳ������߼�ռ�õ�
		map<int64,CSystemIOWork*>  m_SystemIOWorkList;  
		map<int64,CNerveWork*>     m_NerveWorkList;

		list<Object*>              m_DelThreadWorkList;      //�ȴ�����ɾ����б�(�����߳����Լ�ִ���Լ���delete),����CSystemIOWork��CNerveWork

		CLockedSystemData(){};
	public:
		CLockedSystemData(CABMutex* mutex);
		virtual ~CLockedSystemData();

		virtual bool Activation();
		virtual void Dead();

		void    IncreNerveWorkCount();
        void    DecreNerveWorkCount();

		int64   GetNerveMsgInterval();
		void    SetNerveMsgInterval(int32 n);
		
		int32	GetNerveMaxIdleCount();
		void	SetNerveMaxIdleCount(int32 n);
		
		int32   GetNerveWorkNum();
		int32   GetBusyNerveWorkNum();
        
		int32   GetIOWorkNum();
		int32   AddIOWork(CSystemIOWork* Work);
		int32   DeleteIOWork(int64 ID);
		
		int32   AddNerveWork(CNerveWork* Work);
        int32   DeleteNerveWork(int64 ID);
			
		bool    RequestCreateNewNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason); //��������

	};

    //����System��ʼ���б?���û�ʵ��ʱ�м̴��༴�����Ӳ����������Խ��Խ��
	class CSystemInitData:public Model::CModelInitData{
	public:
		CLockedSystemData*  m_SystemData;
		CABMutex*		    m_SystemListMutex;
		CSpaceMutex*		m_ClientSitMutex;
		CLockPipe*          m_Nerve;
		
	public:
		CSystemInitData();
		virtual ~CSystemInitData();
	};

    friend class CSystemIOWork;
	friend class CNerveWork;
private:
	//һ���񾭣����������ֱ�Ӵ�����Ϣ�Ͱ���Ϣ����һ���񾭣�Ȼ���ж��̸߳�����Ϣ�Ĵ���,
	CLockPipe*                      m_Nerve;
	CLockedLinkerList               m_ClientList;
	CLockedSystemData*              m_SystemData;

protected:	

	CSpaceMutex*					m_ClientSitMutex;  //Child CLinkerPipe ������

    virtual void		NerveProc(CMsg& Msg)=0;//һ���񾭶���Ϣ�Ĵ��?���û���Ҫ�Լ�ʵ��
	virtual BOOL		NerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
	virtual CNerveWork* CreateNerveWorker(int64 ID,System* Parent,uint32 Reason)=0;


	virtual void BroadcastMsg(set<int64>& SourceList,int64 BCS_ID,ePipeline& MsgData);
	

public:
//	System(CLockedSystemData* SystemData,CABMutex* ModelMutex, CABMutex* ModelListMutex,CABMutex* SysMutex, CABMutex* SystemListMutex, CSpaceMutex* ChildSitMutex, CLockPipe* CentralNerve, CLockPipe* Nerve,const TCHAR* Name);
	System(CSystemInitData* InitData);
	virtual MASS_TYPE  MassType(){ return MASS_SYSTEM;};
	virtual ~System();
	
    virtual bool Activation();
	virtual void Dead();

	CLockedLinkerList*    GetClientLinkerList();
	CLockedSystemData*    GetSystemData();

	void GetLinker(int64 ID,CLinker& Linker );

    void	     PopNerveMsg(CMsg& Msg);
	int32		 GetNerveMsgNum();
	void		 GetNerveMsgList(ePipeline& Pipe);
	void		 PushNerveMsg(CMsg& Msg,bool bUrgenceMsg=FALSE);
};

} //namespace ABSTRACT

#endif // !defined(AFX_SYSTEM_H__C57907E3_4F12_11DA_8739_F810D83B8278__INCLUDED_)
