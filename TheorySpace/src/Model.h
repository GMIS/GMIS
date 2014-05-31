/*
* author: ZhangHongBing(hongbing75@gmail.com)  
*  
* Model��Object�������ӵ�д��ⲿ�����Ϣ��������ͨ����ָ�������ӣ���ȻҲ�����������֮�ࡣ
* ���Ӧ��Ŀ����һ��������Ϣ������������ĳ����ܡ��ӳ���Ա�Ƕȿ���Model��һ�����͵�Client��
*
* ��ˣ�Model���븺�������ӣ�Ȼ���ͨ�����ӵ���Ϣת���ɱ��ظ�ʽ��Ϣ��������Ϣ��������Object
* ��ͬ����һ�ж����û���͸���ģ����ھ���ѡ������������ӷ�ʽ�����û�����ʵ�֣����������ǰ��ϵͳ
* ������
*/

#ifndef _MODEL_H__
#define _MODEL_H__


#include "LinkerPipe.h"
#include "Object.h"
#include <map>
#include <list>

using namespace std;


namespace ABSTRACT{


#define  SPACE_PORT   911
#define  ROBOT_PORT   912

#define    BUFFER_SIZE  1024*64

//log flag

	
enum Log_Flag{
	LOG_MSG_IO_PUSH           =0x00000001, 
	LOG_MSG_I0_RECEIVED       =0x00000002,
	LOG_MSG_IO_SENDED         =0x00000004,
	LOG_MSG_IO_REMOTE_RECEIVED=0x00000008,
	LOG_MSG_CENTRL_NERVE_PUSH =0x00000010,
	LOG_MSG_NERVE_PUSH        =0x00000020,
	LOG_MSG_TASK_PUSH         =0x00000040,
	LOG_MSG_PROC_BEGIN        =0x00000100,
	LOG_MSG_PROC_PATH         =0x00000200,
	LOG_MSG_RUNTIME_TIP       =0x00000400,
	LOG_ERROR                 =0x00001000,
	LOG_WARNING               =0x00002000,
	LOG_TIP                   =0x00004000
};

//NotifyModelState()
#define MNOTIFY_CENTRAL_NERVE_MSG_NUM       10000
#define MNOTIFY_CENTRAL_NERVE_THREAD_JOIN   10001
#define MNOTIFY_CENTRAL_NERVE_THREAD_CLOSE  10002
#define MNOTIFY_CENTRAL_NERVE_THREAD_LIMIT  10003
#define MNOTIFY_CENTRAL_NERVE_THREAD_FAIL   10004
#define MNOTIFY_CONNECT_FAIL                10005
#define MNOTIFY_IO_WORK_THREAD_CLOSE        10006
#define MNOTIFY_EXCEPTION_OCCURRED          10007
#define MNOTIFY_ILLEGAL_MSG                 10008

#define REASON_REFUSE       0x0000
#define REASON_TIME_OUT     0x0001
#define REASON_MSG_TOO_MUCH 0x0002
#define REASON_LIMIT        0x0003
#define REASON_WORKER_BUSY  0X0004
///////////////////////////////////////////////////////////////////////////


class  Model : public Object  
{
public:	
	class CLockedLinkerList
	{
	private:
		CABMutex*                   m_pMutex;
		map<int64,CLinkerPipe*>     m_LinkerList;     	
	    list<CLinkerPipe*>          m_DelLinkerList; //�ȴ�����ɾ����б�
    
		CLockedLinkerList(){};
	public:
		CLockedLinkerList(CABMutex* mutex);
		~CLockedLinkerList();	


		//���ڶ��߳�ʹ�ã���ֱ��ɾ���ȷ���m_DelLinkerList�У�����һ����ɾ��ʱ�����ǰһ��ɾ������Ƿ����ɾ��
		bool          DeleteLinker(int64 ID);  
	
		void	      GetLinker(int64 ID,CLinker& Linker);
		void          GetNextLinker(int64 SourceID,CLinker& Linker); //�õ�SourceID����һ�������ڱ����б�
		void          AddLinker(CLinkerPipe* Linker); 
	
		void		  DeleteAllLinker(); //���ã�ֻ�ڹر����ϵͳʱʹ�ã�����ȷ�������������߳�ʹ��CLinkerPipe
	
		bool          SetLinkerID(int64 OldID,int64 NewID);

		int32         GetLinkerNum();
		int32         GetDelLinkerNum();
	};


	//�����̣߳������������IO��ʵ��ʹ����Ҫ�̳�ʵ���߳�����
	class CModelIOWork : public Object{
	protected:	
		Model*     m_Parent;
	public:
		CModelIOWork(int64 ID,Model* Parent);
		virtual ~CModelIOWork();
		virtual bool Do(Energy* E);

	};
		
	//�����̣߳�������������Ϣ�Ĵ��?ʵ��ʹ����Ҫ�̳�ʵ���߳�����
	class CCentralNerveWork : public Object{
	protected:			
		Model*	    m_Parent;
		int32       m_IdleCount;  //����Ϣ�ɴ���ʱ��ѭ������
	public:
		CCentralNerveWork(int64 ID,Model* Parent);
		virtual ~CCentralNerveWork();
		virtual bool Do(Energy* E);
	};

	class CLockedModelData{
	protected:
		CABMutex*					     m_pMutex;
		uint32                           m_MaxNerveWorkerNum;      //������������񾭴����߳���Ŀ��default=20
		uint32                           m_NerveMsgMaxNumInPipe;   //�����������������������Ϣ��������µĴ����߳�,default = 10
		int64                            m_NerveMsgMaxInterval;    //���������ȡ������Ϣʱ�䳬�����������µĴ����߳�,default=10*1000*1000(��λ�ǰ����룬Ҳ����1��)
		uint32                           m_NerveIdleMaxCount;	   //�����񾭴����߳������м����������˳���default=30
		uint32							 m_NerveWorkingNum;        //���ڹ����ģ�Ҳ���Ǳ������߼�ռ�õ�

		map<int64,CModelIOWork*>		 m_ModelIOWorkList;        //�����������������
		map<int64,CCentralNerveWork*>    m_CentralNerveWorkList;  
		list<Object*>                    m_DelThreadWorkList;      //�ȴ�����ɾ����б�(�����߳����Լ�ִ���Լ���delete),����ModelIOWork��CentralNerveWork

		CLockedModelData(){};
	public:
		CLockedModelData(CABMutex* mutex);
		~CLockedModelData();
       
		virtual bool Activation();
		virtual void Dead();

		void    IncreNerveWorkCount();
		void    DecreNerveWorkCount();

		int64   GetNerveMsgInterval();
		void    SetNerveMsgInterval(int32 n);

		int32	GetNerveMaxIdleCount();
		void	SetNerveMaxIdleCount(int32 n);
		
		int32   GetCentralNerveWorkNum();
        int32   GetBusyNerveWorkNum();

		int32   GetIOWorkNum();
		int32   AddIOWork(CModelIOWork* Work);
		int32   DeleteIOWork(int64 ID);
	 

		int32   AddCentralNerveWork(CCentralNerveWork* Work);
        int32   DeleteCentralNerveWork(int64 ID);
		
		bool    RequestCreateNewCentralNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason); //��������
	};

    //����Model��ʼ���б?���û�ʵ��ʱ�м̴��༴�����Ӳ����������Խ��Խ��
	class CModelInitData{
	public:
		tstring					  m_Name;
		CTimeStamp*               m_Timer;
		CAbstractSpacePool*       m_Pool;
		CLockedModelData*         m_ModelData;
		CABMutex*                 m_ModelListMutex; 
		CLockPipe*                m_CentralNerve;  
		int32                     m_nCPU;   //CPU���������ж��߳�ʹ����Ŀ���ǵ�ǰ����
	public:
		CModelInitData();
		virtual ~CModelInitData();
	};
private: 
	//�����񾭣�������Ϣ����˹ܵ���ű����?��ȫ˽�У���ֹ������ֱ�Ӵ�ȡ��Ϣ����Ϊ��ȡ��Ϣʱ�漰�������̵߳ĵ���
	CLockPipe*           m_CentralNerve;

	//�����ڶ��̻߳����£���Ҫ����˲�����ֱ�����ã����ǵ���GetModelData()�õ���������;
	CLockedLinkerList    m_SuperiorList;               
    CLockedModelData*    m_ModelData;

	
	//���״̬�����Ƿ���Ҫ����Worker(�̣߳�����PushCentralNerveMsg����;
	//NewMsgPushTime=ѹ������Ϣ��ĵ�ǰʱ�䣬LastMsgPopTime�������ȡ����Ϣ��ʱ��
	//�����������߳��򷵻�true������false
	virtual BOOL  CentralNerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime);
protected:

	uint32         m_LogFlag;  //default=0
	
	/*
	  ��ϵͳ�����̣߳�ReasonĿǰ������:
	    һ����ʱ�����أ��ܵ���ĵȺ����Ϣ��Ȼ�������࣬�����һ���Ѿ�����Ԥ��ʱ��û��ȡ��������һ����workerȥ����
		һ�����������أ��ܵ���ȴ����Ϣ����̫�࣬�����̲߳����ã�����һ���µ�
		ͨ��,����һ��ͷ��˿��Բ�����reason,����һ���򵥵��̲߳�����
    */
	virtual CCentralNerveWork* CreateCentralNerveWorker(int64 ID, Model* Parent,uint32 Reason)=0;

	
public:
    Model(CModelInitData* InitData);
	virtual ~Model();

	virtual MASS_TYPE  MassType(){ return MASS_MODEL;};
    virtual bool	   Activation();
	virtual void       Dead();

	void    SetLogFlag(uint32 flag);
	uint32  GetLogFlag();
	
	//Type = OUTPUT_TEXT...if(Type&m_LogOutFlag)then output
	virtual void  OutputLog(uint32 Flag,const wchar_t* text)=0;
	
	virtual tstring MsgID2Str(int64 MsgID)=0;

    CLockedLinkerList* GetSuperiorLinkerList();
	CLockedModelData*  GetModelData();

	//����Ϣѹ�������񾭺����CentralNerveWorkStrategy()
	void			   PushCentralNerveMsg(CMsg& Msg,bool bUrgenceMsg=FALSE);
	void			   PopCentralNerveMsg(CMsg& Msg);
	int32			   GetCentralNerveMsgNum();
	void			   GetCentralNerveMsgList(ePipeline& Pipe);


	//ϵͳ�ڲ��κ���ɣ���Ҫ���̣߳�ֱ��ͨ��˺���֪ͨϵͳһЩ���飬DATAΪ֪ͨ���������
	virtual void       NotifySysState(int64 NotifyID,ePipeline* Data = NULL)=0;

	//�û�Ӧ������������ʵ�־���Ĵ��?��#define MY_MAIN_FRAME ��������ע�⣺Linker�Ѿ���������״̬
	virtual void       NotifyLinkerState(CLinkerPipe* Linker,int64 NotifyID,ePipeline& Data)=0;

     /* ����ָ����ַ����ַ���ѹ��Pipe�У�����������Ͷ�ͬ
	
	  Client��Server�����ֳ���
	  
		Client            ---Connect---->              Server
		|                                              | ���ӳɹ�
		|             <--MSG_WHO_ARE_YOU-----          | ѯ�ʶԷ�
		|                                              |  
		|             ------MSG_I_AM -------->         | �õ�Client����֤��Ϣ
		|                                              |
		|             <----MSG_CONNECT_OK-------       | ���� 
		|				<----MSG_DICCONNECT--------	     | ���߾ܾ�    	
    */
	//virtual BOOL Connect(ePipeline* Pipe)=0;
	
  	
	/*�м���Object�Ĺ����߳�,�û����������������,�����Model�������񾭴���
	  �û���ҵ���߼�������ʵ�֣�����̲߳�������ɣ��������ѹ��m_Nerver,
	  ��NerverWork�̴߳��?

      ����Ӧ�ô��������ȵ���Ϣ�����������������֯һ���������

      ע�⣺����Ϣ���?��TRUE,û��Ӧ�÷���FALSE,�Ա�֪ͨϵͳ�ص�������߳�
    */
	virtual bool Do(Energy* E=NULL){
		assert(E);

		//����ж����������ӣ���ôȱʡӦ�ô���������Ϣ������ֻ�Ǹ����ʾ��û�д��?
		//if(GetCentralNerveMsgNum()){
			ePipeline* Pipe = (ePipeline*)E;
			CMsg Msg(Pipe);		
/*			
		
			int64 MsgID  = Msg.GetMsgID();
			switch(MsgID)
			{
			case MSG_CONNECT_OK:
				break;
			case MSG_CONNECT_FAIL:
				break;
			case MSG_WHO_ARE_YOU:
				break;
			case MSG_RECEIVE_OK:
				//m_CentralNerve->Push(Msg);
				break;
			case MSG_RECEIVE_ERROR:
				break;
			case MSG_SEND_PROGRESS:
				break;
			case MSG_NEW_DIALOG:
				break;
			default:
				//m_CentralNerve->Push(Msg);
				break;
			}
*/
		//}
		return TRUE;	
	}
};

}// namespace ABSTRACT

#endif // !defined _MODEL_H__
