

#include "Model.h"
#include "LinkerPipe.h"

namespace ABSTRACT{

//CLockedLinkerList
//////////////////////////////////////////////////////////////////////////
Model::CLockedLinkerList::CLockedLinkerList(CABMutex* mutex):m_pMutex(mutex){
	assert(mutex);
}

Model::CLockedLinkerList::~CLockedLinkerList(){
	DeleteAllLinker();
}
	
bool Model::CLockedLinkerList::DeleteLinker(int64 ID) 
{
	CLock lk(m_pMutex);
	
	while(m_DelLinkerList.size()){
		CLinkerPipe* Linker  = m_DelLinkerList.front();
		m_DelLinkerList.pop_front();
		if(Linker->GetUserNum()){
            m_DelLinkerList.push_back(Linker);
			break;
		}else{
			delete Linker;
		} 
	};

	map<int64,CLinkerPipe*>::iterator it = m_LinkerList.find(ID);
	if(it != m_LinkerList.end()){
		CLinkerPipe* Linker = it->second;
		Linker->Close();
		m_LinkerList.erase(it);
		Linker->SetRecoType(LINKER_DEL);
		m_DelLinkerList.push_back(Linker);
		return true;
	}
	return false;
}

void Model::CLockedLinkerList::GetLinker(int64 ID,CLinker& Linker)
{
	CLock lk(m_pMutex);
	map<int64,CLinkerPipe*>::iterator it = m_LinkerList.find(ID);
	if(it != m_LinkerList.end()){
		CLinkerPipe* LinkerPtr = it->second;
		Linker.Reset(LinkerPtr);
	}else{
		Linker.Reset(NULL);
	}
}
int32  Model::CLockedLinkerList::GetLinkerNum(){
	CLock lk(m_pMutex);
	return m_LinkerList.size();
};

int32  Model::CLockedLinkerList::GetDelLinkerNum(){
	CLock lk(m_pMutex);
	return m_DelLinkerList.size();
}

void  Model::CLockedLinkerList::AddLinker(CLinkerPipe* Linker){
	assert(Linker);
	
	CLock lk(m_pMutex);
	int64 SourceID = Linker->GetSourceID();   		
	assert(m_LinkerList.find(SourceID) == m_LinkerList.end());
	m_LinkerList[SourceID] = Linker;
}

bool Model::CLockedLinkerList::SetLinkerID(int64 OldID,int64 NewID){
	CLock lk(m_pMutex);
	map<int64,CLinkerPipe*>::iterator it = m_LinkerList.find(OldID);
	if(it != m_LinkerList.end()){
		CLinkerPipe* LinkerPtr = it->second;
	    m_LinkerList[NewID] = LinkerPtr;
		LinkerPtr->SetSourceID(NewID);
		m_LinkerList.erase(it);
		return true;
	}	
	return false;
};

void Model::CLockedLinkerList::DeleteAllLinker(){
	CLock lk(m_pMutex);
	map<int64,CLinkerPipe*>::iterator it = m_LinkerList.begin();
	while(it != m_LinkerList.end()){
		CLinkerPipe* LinkerPtr = it->second;	
		LinkerPtr->Close();
		delete LinkerPtr;
		it++;
	}	
	
	m_LinkerList.clear();	
	
	while(m_DelLinkerList.size()){
		CLinkerPipe* Linker  = m_DelLinkerList.front();
		m_DelLinkerList.pop_front();
		delete Linker;
	} 
}
	
void Model::CLockedLinkerList::GetNextLinker(int64 SourceID,CLinker& Linker){
	CLock lk(m_pMutex);
	if (m_LinkerList.size()==0)
	{
		Linker.Reset(NULL);
		return;
	};
	
	map<int64,CLinkerPipe*>::iterator it;
	if (SourceID == 0)
	{
		it = m_LinkerList.begin();
		
	}else{
		
		it = m_LinkerList.find(SourceID);
		if(it == m_LinkerList.end()){
			Linker.Reset(NULL);
			return;
		}
		
		it++;
		if (it == m_LinkerList.end())
		{
			Linker.Reset(NULL);
			return;
		}
		
	}
	
	//�����Ѿ����ɾ���Linker����ʵ��ɾ����
	CLinkerPipe* LinkPipe = it->second;
	
	int32 LinkerType = LinkPipe->GetRecoType();
	while (LinkerType < LINKER_STRANGER)
	{
		if (LinkerType == LINKER_DEL && LinkPipe->GetUserNum()==0)
		{
			m_LinkerList.erase(it++);
			LinkPipe->Close();
			delete LinkPipe;
			LinkPipe = NULL;
		}else{		
			it++;
		}
		if (it == m_LinkerList.end())
		{
			Linker.Reset(NULL);
			return;
		}else{
			LinkPipe = it->second;
			LinkerType = LinkPipe->GetRecoType();
		}
	}
	Linker.Reset(LinkPipe);
};

//CIOWork
//////////////////////////////////////////////////////////////////////////
Model::CModelIOWork::CModelIOWork(int64 ID,Model* Parent)
:m_Parent(Parent)
{
	m_ID = ID;
};

Model::CModelIOWork::~CModelIOWork(){
		
};
	
bool Model::CModelIOWork::Do(Energy* E){
#ifndef _DEBUG
	try
	{
#endif	
	CLinker Linker;
	while(m_Alive){
			int64 SourceID = 0;
			
			CLockedLinkerList* LinkerList = m_Parent->GetSuperiorLinkerList();
			
			LinkerList->GetNextLinker(SourceID,Linker);

			char buf[BUFFER_SIZE];
			while (m_Alive && Linker.IsValid())
			{
				SourceID = Linker().GetSourceID();
				//��ServerIO
				if (!Linker().IOBusy())
				{
					Linker().ThreadIOWorkProc(buf,BUFFER_SIZE);
				}
				LinkerList->GetNextLinker(SourceID,Linker);
			}
			
			SLEEP_MILLI(20);
		}
        if (m_Parent->IsAlive()) 
        {
			ePipeline Data;
			Data.PushInt(m_ID);
			m_Parent->NotifySysState(MNOTIFY_IO_WORK_THREAD_CLOSE,&Data);
			CLockedModelData* LockedData = m_Parent->GetModelData();
			LockedData->DeleteIOWork(m_ID);
        }
#ifndef _DEBUG	
	}
	catch (...)
	{
		ePipeline Data;
		Data.PushString(_T("An exception occurred, ModelIOWork Close"));
		m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
		CLockedModelData* LockedData = m_Parent->GetModelData();
		LockedData->DeleteIOWork(m_ID);
	}
#endif	
	m_Alive = FALSE;
	return TRUE;			
}
	
//CCentralNerveWork
//////////////////////////////////////////////////////////////////////////

Model::CCentralNerveWork::CCentralNerveWork(int64 ID,Model* Parent)
	:m_Parent(Parent),m_IdleCount(0)
{
	m_ID = ID;
};
	
Model::CCentralNerveWork::~CCentralNerveWork(){
		
};
	
bool Model::CCentralNerveWork::Do(Energy* E){
//	try{
		while (m_Alive)
		{
			CMsg Msg;
			m_Parent->PopCentralNerveMsg(Msg);
			CLockedModelData* LockedData = m_Parent->GetModelData();

			if (Msg.IsValid())
			{		

				LockedData->IncreNerveWorkCount();
				m_Parent->Do(Msg.Release());

				LockedData->DecreNerveWorkCount();
				m_IdleCount = 0;
			}else{
				m_Parent->Do(NULL);
				m_IdleCount++;
	
				int n = LockedData->GetNerveMaxIdleCount();
				
				if ( m_IdleCount > n ) //ȱʡ��Լ1000��������Ϣ�ɴ������˳�
				{
					int WorkNum = LockedData->GetCentralNerveWorkNum();
					if(WorkNum>1){ //����޶ȱ���һ��
						if(m_Parent->IsAlive()){
							ePipeline Data;
							Data.PushInt(--WorkNum);
							Data.PushInt(m_ID);
							m_Parent->NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_CLOSE,&Data);
							LockedData->DeleteCentralNerveWork(m_ID);	
						}
						m_Alive = FALSE;
						return TRUE;
					}
				}
				
				SLEEP_MILLI(20);
			}
		}
//	}
/*    catch(...){
		assert(0);

		ePipeline Data;
		Data.PushString(_T("An exception occurred, CCentralNerveWork Close"));
		m_Parent->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,&Data);

		CLockedModelData* LockedData = m_Parent->GetModelData();
		LockedData->DeleteCentralNerveWork(m_ID);	
	}
*/
	m_Alive = FALSE;
	return TRUE;
}


	
//CLockedModelData
//////////////////////////////////////////////////////////////////////////
Model::CLockedModelData::CLockedModelData(CABMutex* mutex)
	:m_pMutex(mutex){
	assert(m_pMutex);
	m_MaxNerveWorkerNum    = 20;
	m_NerveMsgMaxNumInPipe = 10;
	m_NerveMsgMaxInterval  = 10*1000*1000; //1��
	m_NerveIdleMaxCount    = 50;
	m_NerveWorkingNum      = 0;
};

Model::CLockedModelData::~CLockedModelData(){
	Dead();

	//CLock lk(m_pMutex);
	
	map<int64,CModelIOWork*>::iterator ita =  m_ModelIOWorkList.begin();
	while (ita != m_ModelIOWorkList.end())
	{
		CModelIOWork* ModelIOWork = ita->second;
		assert(!ModelIOWork->IsAlive());
		delete ModelIOWork;
		ita++;
	}
	
	
	map<int64,CCentralNerveWork*>::iterator itd = m_CentralNerveWorkList.begin();
	while (itd != m_CentralNerveWorkList.end())
	{
		CCentralNerveWork* CentralNerveWork = itd->second;
		assert(!CentralNerveWork->IsAlive());
		delete CentralNerveWork;
		itd++;
	}

	while(m_DelThreadWorkList.size()){
		Object* Thread  = m_DelThreadWorkList.front();
		m_DelThreadWorkList.pop_front();
		delete Thread;
	} 
}

bool Model::CLockedModelData::Activation(){
	CLock lk(m_pMutex);

	map<int64,CModelIOWork*>::iterator ita = m_ModelIOWorkList.begin();
	while (ita != m_ModelIOWorkList.end())
	{
		CModelIOWork* ModelIOWork = ita->second;
		assert(!ModelIOWork->IsAlive());
		if(!ModelIOWork->Activation())return FALSE;
		ita++;
    }
	return TRUE;
}

void Model::CLockedModelData::Dead(){
//	CLock lk(m_pMutex);work->Dead()�����CLockedModelData����Ƕ������

	map<int64,CModelIOWork*>::iterator ita = m_ModelIOWorkList.begin();
	while (ita != m_ModelIOWorkList.end())
	{
		CModelIOWork* ModelIOWork = ita->second;
		//assert(ModelIOWork->IsAlive());
		ModelIOWork->Dead();
		ita++;
	}
	
	map<int64,CCentralNerveWork*>::iterator itc = m_CentralNerveWorkList.begin();
	while (itc != m_CentralNerveWorkList.end())
	{
		CCentralNerveWork* CentralNerveWork = itc->second;
		//assert(NerveWork->IsAlive());
		CentralNerveWork->Dead();
		itc++;
    }		

}


void  Model::CLockedModelData::IncreNerveWorkCount(){
	CLock lk(m_pMutex);
	++m_NerveWorkingNum ;
}
void  Model::CLockedModelData::DecreNerveWorkCount(){
	CLock lk(m_pMutex);
	--m_NerveWorkingNum;
}

int64   Model::CLockedModelData::GetNerveMsgInterval(){
	CLock lk(m_pMutex);
	return m_NerveMsgMaxInterval;
}
void    Model::CLockedModelData::SetNerveMsgInterval(int32 n){
	CLock lk(m_pMutex);
	m_NerveMsgMaxInterval = n;
}
	
int32   Model::CLockedModelData::GetNerveMaxIdleCount(){
	CLock lk(m_pMutex);
	return m_NerveIdleMaxCount;	
}
	
void   Model::CLockedModelData::SetNerveMaxIdleCount(int32 n){
	CLock lk(m_pMutex);
	m_NerveIdleMaxCount = n;
}

int32  Model::CLockedModelData::GetCentralNerveWorkNum(){
	CLock lk(m_pMutex);
	int32 n = m_CentralNerveWorkList.size();
	return n;
}
int32   Model::CLockedModelData::GetBusyNerveWorkNum(){
	CLock lk(m_pMutex);
	return m_NerveWorkingNum;		
};
int32 Model::CLockedModelData::GetIOWorkNum(){
	CLock lk(m_pMutex);
	return m_ModelIOWorkList.size();
}
int32    Model::CLockedModelData::AddIOWork(CModelIOWork* Work){
	CLock lk(m_pMutex);
	assert(m_ModelIOWorkList.find(Work->m_ID) == m_ModelIOWorkList.end());
	m_ModelIOWorkList[Work->m_ID] = Work;
	return m_ModelIOWorkList.size();
}

int32    Model::CLockedModelData::DeleteIOWork(int64 ID){
	CLock lk(m_pMutex);

	while(m_DelThreadWorkList.size()){
		Object* Thread  = m_DelThreadWorkList.front();
		m_DelThreadWorkList.pop_front();
		if(!Thread->IsAlive()){
            m_DelThreadWorkList.push_back(Thread);
			break;
		}else{
			delete Thread;
		} 
	};

	map<int64,CModelIOWork*>::iterator it = m_ModelIOWorkList.find(ID);
	if (it != m_ModelIOWorkList.end())
	{
		CModelIOWork* Work = it->second;
		m_ModelIOWorkList.erase(it);
		m_DelThreadWorkList.push_back(Work);
	}
	return m_ModelIOWorkList.size();
}

int32    Model::CLockedModelData::AddCentralNerveWork(CCentralNerveWork* Work){
	CLock lk(m_pMutex);
	assert(m_CentralNerveWorkList.find(Work->m_ID) == m_CentralNerveWorkList.end());
	m_CentralNerveWorkList[Work->m_ID] = Work;
	return m_CentralNerveWorkList.size();
	
}

int32    Model::CLockedModelData::DeleteCentralNerveWork(int64 ID){
	CLock lk(m_pMutex);

	while(m_DelThreadWorkList.size()){
		Object* Thread  = m_DelThreadWorkList.front();
		m_DelThreadWorkList.pop_front();
		if(Thread->IsAlive()){
            m_DelThreadWorkList.push_back(Thread);
			break;
		}else{
			delete Thread;
		} 
	};

	map<int64,CCentralNerveWork*>::iterator it = m_CentralNerveWorkList.find(ID);
	if (it != m_CentralNerveWorkList.end())
	{
		CCentralNerveWork* Work = it->second;
		m_CentralNerveWorkList.erase(it);
		m_DelThreadWorkList.push_back(Work);

	}
	return m_CentralNerveWorkList.size();
}

 bool Model::CLockedModelData::RequestCreateNewCentralNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason){
	
	CLock lk(m_pMutex);		
	if (m_CentralNerveWorkList.size() == m_MaxNerveWorkerNum)
	{
		Reason =  REASON_LIMIT;
		return FALSE;
	}	
	else if (MsgNum > m_NerveMsgMaxNumInPipe)//��ǰ��Ϣ��Ŀ����ָ����Ŀ���������߳�
	{
		Reason = REASON_MSG_TOO_MUCH;
		return TRUE;
	}else if(Interval> m_NerveMsgMaxInterval){ //����ָ�����ʱ��û��ȡ������Ϣ���������������߳�
		Reason = REASON_TIME_OUT;
		return TRUE;
	}else if (m_NerveWorkingNum == m_CentralNerveWorkList.size())
	{
		Reason = REASON_WORKER_BUSY;
		return TRUE;
	}
	else {
		Reason = REASON_REFUSE;
	}
	return FALSE;
}

 Model::CModelInitData::CModelInitData()
	 :m_Timer(NULL),
	  m_Pool(NULL),
      m_ModelData(NULL),
	  m_ModelListMutex(NULL), 
	  m_CentralNerve(NULL),  
	  m_nCPU(1)
 {
 };

 Model::CModelInitData::~CModelInitData(){

 }

//Model
//////////////////////////////////////////////////////////////////////////
 
Model::Model(CModelInitData* InitData)
:Object(InitData->m_Timer,InitData->m_Pool),
m_CentralNerve(InitData->m_CentralNerve),
m_SuperiorList(InitData->m_ModelListMutex),
m_ModelData(InitData->m_ModelData),
m_LogFlag(0)
{
	m_Name = InitData->m_Name;
   assert(m_ModelData);
}

Model::~Model()
{

};    

bool Model::Activation(){
	if(m_Alive){
		return TRUE;
	}
	if (!m_ModelData->Activation())
	{
		return FALSE;
	}
				
	m_Alive = TRUE;
	return TRUE;
}

void Model::Dead(){
	m_Alive = FALSE;
	m_ModelData->Dead();
	Object::Dead();
}

void  Model::SetLogFlag(uint32 flag){
	m_LogFlag = flag;
}

uint32  Model::GetLogFlag(){
	return m_LogFlag;
}	

Model::CLockedLinkerList*  Model::GetSuperiorLinkerList(){
	return &m_SuperiorList;
};

Model::CLockedModelData* Model::GetModelData(){
	return m_ModelData;
}

int32 Model::GetCentralNerveMsgNum(){
	assert(m_CentralNerve);
	return m_CentralNerve->DataNum();
};

void  Model::GetCentralNerveMsgList(ePipeline& Pipe){
	assert(m_CentralNerve);
	m_CentralNerve->CopyTo(Pipe);
}



BOOL  Model::CentralNerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){
	int32 n = GetCentralNerveMsgNum();
	ePipeline Data;
	Data.PushInt(n);
	NotifySysState(MNOTIFY_CENTRAL_NERVE_MSG_NUM,&Data);
	
	int64 t = NewMsgPushTime-LastMsgPopTime; //ת������
	
	if(LastMsgPopTime==0){	//��һ��		
		CCentralNerveWork* CentralNerveWork = CreateCentralNerveWorker(0,this,REASON_MSG_TOO_MUCH); //��һ��ID����Ϊ0
		if (CentralNerveWork && CentralNerveWork->Activation())
		{
			int n = m_ModelData->AddCentralNerveWork(CentralNerveWork);
			ePipeline Data;
			Data.PushInt(n);
			Data.PushInt(CentralNerveWork->m_ID);
			NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_JOIN,&Data);
			return TRUE;
		}else{
			if(CentralNerveWork){
				delete CentralNerveWork;
			}
			NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_FAIL,NULL);
			return FALSE;
		}
	}
	else if(m_Alive){//������������г���10��������Ϣ���߳���2����û��ȡ������Ϣ���������������߳�
		uint32 Reason ;
		bool ret = m_ModelData->RequestCreateNewCentralNerveWork(n,t,Reason);
		if (!ret)
		{
			if (Reason == REASON_LIMIT)
			{
				//֪ͨ�Ѿ��ﵽ�������
				NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_LIMIT,NULL);
			}
			return FALSE;
		}
		
		CCentralNerveWork* CentralNerveWork = CreateCentralNerveWorker(NewMsgPushTime,this,Reason);
		if (CentralNerveWork && CentralNerveWork->Activation())
		{
			int n =m_ModelData->AddCentralNerveWork(CentralNerveWork);
			ePipeline Data;
			Data.PushInt(n);
			Data.PushInt(CentralNerveWork->m_ID);
			NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_JOIN,&Data);

			return TRUE;
		}else{
			if(CentralNerveWork){
				delete CentralNerveWork;
			}
			NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_FAIL,NULL);
			return FALSE;
		}
		
	}
	return FALSE;
}

void  Model::PushCentralNerveMsg(CMsg& Msg,bool bUrgenceMsg){
	int64 MsgID = Msg.GetMsgID();
	int64 EventID = Msg.GetEventID();

	assert(m_CentralNerve);
	int64 LastMsgPopTime = 0;
	if(bUrgenceMsg){
		LastMsgPopTime = m_CentralNerve->PushUrgence(Msg.Release());
	}else {
		LastMsgPopTime = m_CentralNerve->Push(Msg.Release());
	}
	
	int64 NewMsgPushTime = CreateTimeStamp();
	assert(NewMsgPushTime!=0);
	assert(NewMsgPushTime!=LastMsgPopTime);

	BOOL ret = CentralNerveWorkStrategy(NewMsgPushTime,LastMsgPopTime);
	if (m_LogFlag & LOG_MSG_CENTRL_NERVE_PUSH)
	{
		tstring MsgStr = MsgID2Str(MsgID);
		if (MsgStr != _T("MSG_EVENT_TICK"))
		{
			tstring s = Format1024(_T("Central nerve push msg: %s EventID:%I64ld MsgNum:%d,CreateNewThread:%d"),MsgStr.c_str(),EventID,m_CentralNerve->DataNum(),ret);
			OutputLog(LOG_MSG_CENTRL_NERVE_PUSH,s.c_str());
		}
	}
}

void Model::PopCentralNerveMsg(CMsg& Msg){
	assert(m_CentralNerve);
	m_CentralNerve->Pop(Msg);

};


}; //end namespace ABSTRACT
