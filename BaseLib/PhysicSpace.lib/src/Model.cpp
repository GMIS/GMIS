

#include "Model.h"
#include "LinkerPipe.h"
#include "UserLinkerPipe.h"

namespace PHYSIC{

//CLockedLinkerList
//////////////////////////////////////////////////////////////////////////
Model::CLockedLinkerList::CLockedLinkerList(){

}

Model::CLockedLinkerList::~CLockedLinkerList(){
	DeleteAllLinker();
}
	
bool Model::CLockedLinkerList::DeleteLinker(int64 ID) 
{
	CLock lk(&m_Mutex);
	
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
	CLock lk(&m_Mutex);
	map<int64,CLinkerPipe*>::iterator it = m_LinkerList.find(ID);
	if(it != m_LinkerList.end()){
		CLinkerPipe* LinkerPtr = it->second;
		Linker.Reset(LinkerPtr);
	}else{
		Linker.Reset(NULL);
	}
}
int32  Model::CLockedLinkerList::GetLinkerNum(){
	CLock lk(&m_Mutex);
	return m_LinkerList.size();
};

int32  Model::CLockedLinkerList::GetDelLinkerNum(){
	CLock lk(&m_Mutex);
	return m_DelLinkerList.size();
}

void  Model::CLockedLinkerList::AddLinker(CLinkerPipe* Linker){
	assert(Linker);
	
	CLock lk(&m_Mutex);
	int64 SourceID = Linker->GetSourceID();   		
	assert(m_LinkerList.find(SourceID) == m_LinkerList.end());
	m_LinkerList[SourceID] = Linker;
}

bool Model::CLockedLinkerList::SetLinkerID(int64 OldID,int64 NewID){
	CLock lk(&m_Mutex);
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
	CLock lk(&m_Mutex);
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
	CLock lk(&m_Mutex);
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

	CLinkerPipe* LinkPipe = it->second;
	Linker.Reset(LinkPipe);
};
void Model::CLockedLinkerList::GetNextAvailableLinker(int64 SourceID,CLinker& Linker){
	CLock lk(&m_Mutex);
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
		it++; //the next linker we wanted
	}

	while(it != m_LinkerList.end()){
		CLinkerPipe* LinkPipe = it->second;

		if(LinkPipe->IOBusy()){
			it++;
			continue;
		}
		
		int32 LinkerType = LinkPipe->GetRecoType();
		if(LinkerType < LINKER_STRANGER)
		{
			if (LinkerType == LINKER_DEL && LinkPipe->GetUserNum()==0)
			{
				it = m_LinkerList.erase(it);
				LinkPipe->Close();
				delete LinkPipe;
				LinkPipe = NULL;
				continue;
			}	
			it++;
		}else{
			Linker.Reset(LinkPipe);
			return;
		}
		
	}
	
	Linker.Reset(NULL);
	return;
};
//CIOWork
//////////////////////////////////////////////////////////////////////////
Model::CModelIOWork::CModelIOWork(int64 ID,Model* Parent)
:m_Parent(Parent)
{
	m_ID = ID;

#if defined(USING_POCO)
	AnsiString ThreadName = "ModelIOThread";
	m_ObjectDefaultThread.setName(ThreadName);
#endif
};

Model::CModelIOWork::~CModelIOWork(){
		
};
	
bool Model::CModelIOWork::Do(Energy* E){
	try
	{
	CLinker Linker;
	while(m_Alive && m_Parent->IsAlive()){
			int64 SourceID = 0;
			
			CLockedLinkerList* LinkerList = m_Parent->GetSuperiorLinkerList();
			
			LinkerList->GetNextAvailableLinker(SourceID,Linker);

			char buf[MODEL_IO_BUFFER_SIZE];
			while (m_Alive && Linker.IsValid())
			{
				SourceID = Linker().GetSourceID();
				//与ServerIO
				Linker().ThreadIOWorkProc(buf,MODEL_IO_BUFFER_SIZE);
				LinkerList->GetNextAvailableLinker(SourceID,Linker);
			}
			
			SLEEP_MILLI(20);
		}
       
		CMsg Msg(MSG_SYS_MSG,DEFAULT_DIALOG,0);
		ePipeline& Letter = Msg.GetLetter();
		Letter.PushInt(NOTIFY_SYS_STATE);
		Letter.PushInt(MNOTIFY_IO_WORK_THREAD_CLOSE);
		ePipeline NotifyData;
		NotifyData.PushInt(m_ID);
		Letter.PushPipe(NotifyData);
		m_Parent->PushCentralNerveMsg(Msg,false,true);

	}
	catch (...)
	{
		CMsg Msg2(MSG_SYS_MSG,DEFAULT_DIALOG,0);
		ePipeline& Letter2 = Msg2.GetLetter();
		Letter2.PushInt(NOTIFY_SYS_STATE);
		Letter2.PushInt(MNOTIFY_EXCEPTION_OCCURRED);
		ePipeline NotifyData2;
		NotifyData2.PushString(_T("An exception occurred, ModelIOWork Close"));
		Letter2.PushPipe(NotifyData2);
		m_Parent->PushCentralNerveMsg(Msg2,false,true);
	}
	CMsg Msg1(MSG_SYS_MSG,DEFAULT_DIALOG,0);
	ePipeline& Letter1 = Msg1.GetLetter();
	Letter1.PushInt(NOTIFY_SYS_STATE);
	Letter1.PushInt(MNOTIFY_IO_WORK_THREAD_CLOSE);
	ePipeline NotifyData1;
	NotifyData1.PushInt(m_ID);
	Letter1.PushPipe(NotifyData1);
	m_Parent->PushCentralNerveMsg(Msg1,false,true);

	m_Alive = FALSE;
	return TRUE;			
}
	
//CCentralNerveWork
//////////////////////////////////////////////////////////////////////////

Model::CCentralNerveWork::CCentralNerveWork(int64 ID,Model* Parent)
	:m_Parent(Parent),m_IdleCount(0)
{
	m_ID = ID;

#if defined(USING_POCO)
	AnsiString ThreadName = "CentralNerveThread";
	m_ObjectDefaultThread.setName(ThreadName);
#endif
	assert(Parent);	
};
	
Model::CCentralNerveWork::~CCentralNerveWork(){
		
};
	
bool Model::CCentralNerveWork::Do(Energy* E){
	int32 WorkNum = 0;
	try{
		while (m_Alive && m_Parent->IsAlive())
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
				
				if ( m_IdleCount > n ) //Default exit if 1000 milliseconds  without information can be handled 
				{
					WorkNum = LockedData->GetCentralNerveWorkNum();
					if(WorkNum>1){ //reserve one at least
						
						CMsg Msg(MSG_SYS_MSG,DEFAULT_DIALOG,0);
						ePipeline& Letter = Msg.GetLetter();
						Letter.PushInt(NOTIFY_SYS_STATE);
						Letter.PushInt(MNOTIFY_CENTRAL_NERVE_THREAD_CLOSE);
						ePipeline NotifyData;
						NotifyData.PushInt(--WorkNum);
						NotifyData.PushInt(m_ID);
						Letter.PushPipe(NotifyData);
						m_Parent->PushCentralNerveMsg(Msg,false,true);
						
						m_Alive = FALSE;
						return TRUE;
					}
				}
				
				SLEEP_MILLI(20);
			}
		}
	}
    catch(...){

		CMsg Msg2(MSG_SYS_MSG,DEFAULT_DIALOG,0);
		ePipeline& Letter2 = Msg2.GetLetter();
		Letter2.PushInt(NOTIFY_SYS_STATE);
		Letter2.PushInt(MNOTIFY_EXCEPTION_OCCURRED);
		ePipeline NotifyData2;
		NotifyData2.PushString(_T("An exception occurred, CCentralNerveWork Closed"));
		m_Parent->PushCentralNerveMsg(Msg2,false,true);
	}
	
	CMsg Msg1(MSG_SYS_MSG,DEFAULT_DIALOG,0);
	ePipeline& Letter1 = Msg1.GetLetter();
	Letter1.PushInt(NOTIFY_SYS_STATE);
	Letter1.PushInt(MNOTIFY_CENTRAL_NERVE_THREAD_CLOSE);
	ePipeline NotifyData1;
	NotifyData1.PushInt(--WorkNum);
	NotifyData1.PushInt(m_ID);
	Letter1.PushPipe(NotifyData1);
	m_Parent->PushCentralNerveMsg(Msg1,false,true);

	m_Alive = FALSE;
	return TRUE;
}


	
//CLockedModelData
//////////////////////////////////////////////////////////////////////////
Model::CLockedModelData::CLockedModelData(){
	m_MaxNerveWorkerNum    = 20;
	m_NerveMsgMaxNumInPipe = 10;
	m_NerveMsgMaxInterval  = 10*1000*1000; //1秒
	m_NerveIdleMaxCount    = 50;
	m_NerveWorkingNum      = 0;
	m_bClosed             = false;
};

Model::CLockedModelData::~CLockedModelData(){

	CLock lk(&m_Mutex);
	
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

}


void  Model::CLockedModelData::IncreNerveWorkCount(){
	CLock lk(&m_Mutex);
	++m_NerveWorkingNum ;
}
void  Model::CLockedModelData::DecreNerveWorkCount(){
	CLock lk(&m_Mutex);
	--m_NerveWorkingNum;
}

int64   Model::CLockedModelData::GetNerveMsgInterval(){
	CLock lk(&m_Mutex);
	return m_NerveMsgMaxInterval;
}
void    Model::CLockedModelData::SetNerveMsgInterval(int32 n){
	CLock lk(&m_Mutex);
	m_NerveMsgMaxInterval = n;
}
	
int32   Model::CLockedModelData::GetNerveMaxIdleCount(){
	CLock lk(&m_Mutex);
	return m_NerveIdleMaxCount;	
}
	
void   Model::CLockedModelData::SetNerveMaxIdleCount(int32 n){
	CLock lk(&m_Mutex);
	m_NerveIdleMaxCount = n;
}

int32  Model::CLockedModelData::GetCentralNerveWorkNum(){
	CLock lk(&m_Mutex);
	int32 n = m_CentralNerveWorkList.size();
	return n;
}
int32   Model::CLockedModelData::GetBusyNerveWorkNum(){
	CLock lk(&m_Mutex);
	return m_NerveWorkingNum;		
};
int32 Model::CLockedModelData::GetIOWorkNum(){
	CLock lk(&m_Mutex);
	return m_ModelIOWorkList.size();
}
int32    Model::CLockedModelData::AddIOWork(CModelIOWork* Work){
	CLock lk(&m_Mutex);
	if(m_bClosed){
		Work->Dead();
		delete Work;
		return 0;
	}
	assert(m_ModelIOWorkList.find(Work->m_ID) == m_ModelIOWorkList.end());
	m_ModelIOWorkList[Work->m_ID] = Work;
	return m_ModelIOWorkList.size();
}

int32    Model::CLockedModelData::DeleteIOWork(int64 ID){
	CLock lk(&m_Mutex);
    if(m_bClosed)return 0;

	map<int64,CModelIOWork*>::iterator it = m_ModelIOWorkList.find(ID);
	if (it != m_ModelIOWorkList.end())
	{
		CModelIOWork* Work = it->second;
		Work->Dead();
		m_ModelIOWorkList.erase(it);
		delete Work;
	}
	return m_ModelIOWorkList.size();
}

int32    Model::CLockedModelData::AddCentralNerveWork(CCentralNerveWork* Work){
	CLock lk(&m_Mutex);
	if (m_bClosed)
	{
		Work->Dead();
		delete Work;
		return 0;
	}
	assert(m_CentralNerveWorkList.find(Work->m_ID) == m_CentralNerveWorkList.end());
	m_CentralNerveWorkList[Work->m_ID] = Work;
	return m_CentralNerveWorkList.size();
	
}

int32    Model::CLockedModelData::DeleteCentralNerveWork(int64 ID){
	CLock lk(&m_Mutex);

	if(m_bClosed){
		return 0;
	}

	map<int64,CCentralNerveWork*>::iterator it = m_CentralNerveWorkList.find(ID);
	if (it != m_CentralNerveWorkList.end())
	{
		CCentralNerveWork* Work = it->second;
		m_CentralNerveWorkList.erase(it);
		Work->Dead();
		delete Work;
	}
	return m_CentralNerveWorkList.size();
}

void  Model::CLockedModelData::CloseAllWorkThread(){
	m_Mutex.Acquire();
	m_bClosed = true;	
	m_Mutex.Release();

	map<int64,CModelIOWork*>::iterator ita =  m_ModelIOWorkList.begin();
	while(ita != m_ModelIOWorkList.end())
	{
		CModelIOWork* ModelIOWork = ita->second;
		ModelIOWork->Dead();
		ita++;
	}

	map<int64,CCentralNerveWork*>::iterator itb =  m_CentralNerveWorkList.begin();
	while(itb != m_CentralNerveWorkList.end())
	{
		CCentralNerveWork* Work = itb->second;
		Work->Dead();
		itb++;
	}

}

 bool Model::CLockedModelData::RequestCreateNewCentralNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason){
	
	CLock lk(&m_Mutex);		
	if (m_CentralNerveWorkList.size() == m_MaxNerveWorkerNum)
	{
		Reason =  REASON_LIMIT;
		return FALSE;
	}	
	else if (MsgNum > m_NerveMsgMaxNumInPipe)//when Current message number exceeded a specified number,start new thread
	{
		Reason = REASON_MSG_TOO_MUCH;
		return TRUE;
	}else if(Interval> m_NerveMsgMaxInterval){ //when exceeded specified interval, there is no  message  be  popped to handled, create  new thread
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


//Model
//////////////////////////////////////////////////////////////////////////
 
Model::Model(CUserTimer* Timer,CUserSpacePool* Pool)
:Object(Timer,Pool),
m_CentralNerveMutex(),
m_CentralNerve(&m_CentralNerveMutex,_T("CentralNerve"),1),
m_LogFlag(0),
m_nCPU(2)
{
	m_Name = _T("Model");
#if defined(USING_POCO)	
   AnsiString ThreadName = "ModeMainThread";
   m_ObjectDefaultThread.setName(ThreadName);
#endif
}

Model::~Model()
{

};    

bool Model::Activation(){
	if(m_Alive){
		return true;
	}
	
	m_Alive = TRUE;

#if defined(USING_POCO) 
	initializeNetwork();

#elif defined(USING_WIN32)
	WSADATA wsaData;		
	if (WSAStartup(MAKEWORD(2,2),&wsaData)!=0)return FALSE;		

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ){ 
			WSACleanup();
			return FALSE; 
	}
#endif

	if(!Object::Activation()){
		m_Alive = FALSE;
		return false;
	}
	return TRUE;
}

void Model::Dead(){
	m_Alive = FALSE;
	m_ModelData.CloseAllWorkThread();
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
	return &m_ModelData;
}

int32 Model::GetCentralNerveMsgNum(){
	return m_CentralNerve.DataNum();
};

void  Model::GetCentralNerveMsgList(ePipeline& Pipe){
	m_CentralNerve.CopyTo(Pipe);
}



BOOL  Model::CentralNerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){
	int32 n = GetCentralNerveMsgNum();
	ePipeline Data;
	Data.PushInt(n);
	NotifySysState(MNOTIFY_CENTRAL_NERVE_MSG_NUM,&Data);
	
	int64 t = NewMsgPushTime-LastMsgPopTime; //convert into second
	
	if(LastMsgPopTime==0){	//First ID is 0	
		CCentralNerveWork* CentralNerveWork = CreateCentralNerveWorker(0,this,REASEON_ALWAYS); 
		if (!CentralNerveWork)
		{
			assert(0);
			OutputLog(LOG_TIP,_T("Create first centtral Nerver thread fail,Please reboot it"));
			return FALSE;
		}
		if (CentralNerveWork->Activation())
		{
			int n = m_ModelData.AddCentralNerveWork(CentralNerveWork);
			ePipeline Data;
			Data.PushInt(n);
			Data.PushInt(CentralNerveWork->m_ID);
			NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_JOIN,&Data);
			return TRUE;
		}else{
			if(CentralNerveWork){
				delete CentralNerveWork;
			}
			assert(0);
			OutputLog(LOG_TIP,_T("Create first centtral Nerver thread fail,Please reboot it"));

			NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_FAIL,NULL);
			return FALSE;
		}
	}
	else{//If more than 10 messages in the central nerve or there is message waiting for being handled more than 2 seconds, create a new thread
		uint32 Reason ;
		bool ret = m_ModelData.RequestCreateNewCentralNerveWork(n,t,Reason);
		if (!ret)
		{
			if (Reason == REASON_LIMIT)
			{
				NotifySysState(MNOTIFY_CENTRAL_NERVE_THREAD_LIMIT,NULL);
			}
			return FALSE;
		}
		
		CCentralNerveWork* CentralNerveWork = CreateCentralNerveWorker(NewMsgPushTime,this,Reason);
		if (!CentralNerveWork)
		{
			return FALSE;
		}
		if (CentralNerveWork->Activation())
		{
			int n =m_ModelData.AddCentralNerveWork(CentralNerveWork);
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

void  Model::PushCentralNerveMsg(CMsg& Msg,bool bUrgenceMsg,bool bDireclty){
	int64 MsgID = Msg.GetMsgID();
	int64 EventID = Msg.GetEventID();

	int64 LastMsgPopTime = 0;
	if(bUrgenceMsg){
		LastMsgPopTime = m_CentralNerve.PushUrgence(Msg.Release());
	}else {
		LastMsgPopTime = m_CentralNerve.Push(Msg.Release());
	}
	
	if (bDireclty)
	{
		return;
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
			tstring s = Format1024(_T("Central nerve push msg: %s EventID:%I64ld MsgNum:%d,CreateNewThread:%d"),MsgStr.c_str(),EventID,m_CentralNerve.DataNum(),ret);
			OutputLog(LOG_MSG_CENTRL_NERVE_PUSH,s.c_str());
		}
	}
}

void Model::PopCentralNerveMsg(CMsg& Msg){
	m_CentralNerve.Pop(Msg);

};

Model::CCentralNerveWork* Model::CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason){
	CCentralNerveWork* NerveWork = new CCentralNerveWork(ID,this);

	return NerveWork;
} 



bool Model::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
	assert(TimeOut>0&& TimeOut<60);

	CUserConnectLinkerPipe* ConLinkerPipe = new CUserConnectLinkerPipe(this,ID,Address,Port,TimeOut);

	if(!ConLinkerPipe){
		error = _T("Create connect pipe fail");
		return FALSE; 
	}
	if(!ConLinkerPipe->Init(error)){
		delete ConLinkerPipe;
		return FALSE;	
	};

	if (bBlock)
	{
		if(!ConLinkerPipe->BlockConnect(error)){
			delete ConLinkerPipe;
			return FALSE;
		};
	}else{
		CLockedLinkerList* ServerList = GetSuperiorLinkerList();
		ServerList->AddLinker(ConLinkerPipe);
	}
	//if there is not thread to handle,create new one
	CLockedModelData* ModelData = GetModelData();
	if (ModelData->GetIOWorkNum()<m_nCPU*2)
	{
		int64 ID = CreateTimeStamp();
		CModelIOWork* IOWork = new CModelIOWork(ID,this);
		if (!IOWork || !IOWork->Activation())
		{
			error = _T("Create connect IOWork fail");
			return FALSE;
		}
		ModelData->AddIOWork(IOWork);
	}
	return TRUE;
}



}; //end namespace ABSTRACT
