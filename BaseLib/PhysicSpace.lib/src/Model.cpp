#pragma warning(disable:4244)

#include "Model.h"
#include "LinkerPipe.h"
#include "UserLinkerPipe.h"

namespace PHYSIC{
	CSuperiorLinkerList::CSuperiorLinkerList(){

	};

	CSuperiorLinkerList::~CSuperiorLinkerList(){
		_CLOCK(&m_Mutex);
		list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
		while(it != m_LinkerList.end()){
			CLinkerPipe* Linker = *it;
			delete Linker;
			it++;
		}

		it = m_ActivelyLinker.begin();
		if(it != m_ActivelyLinker.end()){
			CLinkerPipe* Linker = *it;
			Linker->SetID(-1); //标记为删除
			return;
		}
	};

	int32  CSuperiorLinkerList::GetLinkerNum(){
		_CLOCK(&m_Mutex);
		return m_LinkerList.size() + m_ActivelyLinker.size();
	}
	void CSuperiorLinkerList::CreateLinker(CLinker& Linker,Model* Parent,int64 SourceID,ePipeline& Param){
		tstring Addr = Param.PopString();
		AnsiString Address = WStoUTF8(Addr);
		int64 Port = Param.PopInt();
		int64 TimeOut = Param.PopInt();

		CUserConnectLinkerPipe* ConLinkerPipe = new CUserConnectLinkerPipe(Parent,SourceID,Address,Port,TimeOut);
		Linker.Reset(ConLinkerPipe);
	};

	void   CSuperiorLinkerList::AddLinker(CLinkerPipe* Linker){
		assert(Linker);

		_CLOCK(&m_Mutex);
		int64 SourceID = Linker->GetSourceID();

#ifdef _DEBUG
		list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
		while(it != m_LinkerList.end()){
			CLinkerPipe* Linker = *it;
			assert(Linker->GetSourceID() != SourceID);
			it++;
		}
#endif
		m_LinkerList.push_back(Linker);
	};

	bool   CSuperiorLinkerList::DeleteLinker(int64 SourceID){
		_CLOCK(&m_Mutex);
		CLinkerPipe* LinkPtr = NULL;
		list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
		while(it != m_LinkerList.end()){
			LinkPtr = *it;
			if(LinkPtr->GetSourceID() == SourceID){
				if (LinkPtr->GetUserNum()==0)
				{
					LinkPtr->Close();
					m_LinkerList.erase(it);
					delete LinkPtr;
				}else{
					LinkPtr->SetID(-1); //标记为删除。
				}
				return true;
			}
			it++;
		}
		
		it = m_ActivelyLinker.begin();
		while(it != m_ActivelyLinker.end()){
			LinkPtr = *it;
			if(LinkPtr->GetSourceID() == SourceID){	
				LinkPtr->SetID(-1); //标记为删除。
				LinkPtr->Close();
				return true;
			};
			it++;
		}
		return false;
	};  

	void   CSuperiorLinkerList::GetLinker(int64 SourceID,CLinker& Linker){
		_CLOCK(&m_Mutex);
		CLinkerPipe* LinkPtr = NULL;
		list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
		while(it != m_LinkerList.end()){
			LinkPtr = *it;
			if(LinkPtr->GetSourceID() == SourceID){
				Linker.Reset(LinkPtr);
				return;
			};
			it++;
		}
		
		it = m_ActivelyLinker.begin();
		while(it != m_ActivelyLinker.end()){
			LinkPtr = *it;
			if(LinkPtr->GetSourceID() == SourceID){
				Linker.Reset(LinkPtr);
				return;
			};
			it++;
		}

		Linker.Reset(NULL);
	};

	bool   CSuperiorLinkerList::HasLinker(const AnsiString& Address,const int32 Port){
		_CLOCK(&m_Mutex);
		list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
		while(it != m_LinkerList.end()){
			CLinkerPipe* LinkerPtr = *it;
			CUserConnectLinkerPipe* ConnectLinker = (CUserConnectLinkerPipe*)LinkerPtr;
			if (ConnectLinker->m_Address == Address && ConnectLinker->m_Port == Port)
			{
				return true;
			}
			it++;
		}
	    it = m_ActivelyLinker.begin();
		while(it != m_ActivelyLinker.end()){
			CLinkerPipe* LinkerPtr = *it;
			CUserConnectLinkerPipe* ConnectLinker = (CUserConnectLinkerPipe*)LinkerPtr;
			if ( ConnectLinker->m_Address == Address && ConnectLinker->m_Port == Port)
			{
				return true;
			}
			it++;
		}
		return false;
	}
	void   CSuperiorLinkerList::PopLinker(CLinker& Linker){
		_CLOCK(&m_Mutex);
		list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
		while(it != m_LinkerList.end()){
			CLinkerPipe* LinkerPtr = *it;
			if (LinkerPtr->GetID()==-1)
			{
				if (LinkerPtr->GetUserNum()==0)
				{
					delete LinkerPtr;
					it = m_LinkerList.erase(it);
				}else{
					it++;
				}
				continue;
			}
			
			Linker.Reset(LinkerPtr);
			m_ActivelyLinker.push_front(LinkerPtr);
			m_LinkerList.erase(it);
			break;			
		}
	};
	
	void   CSuperiorLinkerList::ReturnLinker(CLinker& Linker){
		_CLOCK(&m_Mutex);
		int64 SourceID = Linker().GetSourceID();
		list<CLinkerPipe*>::iterator it = m_ActivelyLinker.begin();
		while(it != m_ActivelyLinker.end()){
			CLinkerPipe* LinkPtr = *it;
			if(LinkPtr->GetSourceID() == SourceID)
			{
				m_ActivelyLinker.erase(it);
				if (LinkPtr->GetID() == -1)//已经被标记删除
				{
					Linker.Release();
					if(LinkPtr->GetUserNum()==0){ //如果其它线程还在使用此连接，那不能直接删除			
						delete LinkPtr;	
						return;
					}
				}
				m_LinkerList.push_back(LinkPtr);
				return;
			};
			it++;
		}
	}


//////////////////////////////////////////////////////////////////////////
CThreadWorker::CThreadWorker(int64 ID,Model* Parent,int32 Type)
	:m_Parent(Parent),m_WorkType(Type)
{
	m_ID = ID;
	m_WorkType = Type;
	m_IdleCount = 0;
};

CThreadWorker::~CThreadWorker(){

}

void CThreadWorker::ModelIOWorkProc(){
	try
	{
		char buf[MODEL_IO_BUFFER_SIZE];
		CLinker Linker;
		while(m_Alive && m_Parent->IsAlive()){

			m_Parent->GetSuperiorLinkerList()->PopLinker(Linker);
			
			if (Linker.IsValid())
			{
 				Linker().ThreadIOWorkProc(buf,MODEL_IO_BUFFER_SIZE);
				m_Parent->GetSuperiorLinkerList()->ReturnLinker(Linker);
			}else{
				SLEEP_MILLI(20);
			}
		}
	}
	catch (...)
	{
		ePipeline NotifyData;
		NotifyData.PushString(_T("An exception occurred, ModelIOWork Close"));
		m_Parent->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);
	}

	ePipeline NotifyData;
	NotifyData.PushInt(m_ID);	
	m_Parent->NotifySysState(NOTIFY_MODEL_SCENE,NTID_IO_WORKER_CLOSED,&NotifyData);

}

void CThreadWorker::CentralNerveWorkProc(){
	int32 WorkNum = 0;
	Model::CLockedModelData* LockedData = m_Parent->GetModelData();

	try{
		while (IsAlive() && m_Parent->IsAlive())
		{
			CMsg Msg;
			m_Parent->PopCentralNerveMsg(Msg);

			if (Msg.IsValid())
			{			
				m_Parent->CentralNerveMsgProc(Msg);

				m_IdleCount = 0;
			}else{
				m_IdleCount++;	
				int Max = LockedData->GetNerveMaxIdleCount();

				if ( m_IdleCount > Max ) //Default exit if 1000 milliseconds  without information can be handled 
				{
					m_IdleCount = 0;
					break;
				}			
				SLEEP_MILLI(20);
			}
		}
	}
	catch(...){
		ePipeline NotifyData;
		NotifyData.PushString(_T("An exception occurred, CCentralNerveWork Close"));
		m_Parent->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);
	}

	ePipeline NotifyData;
	NotifyData.PushInt(--WorkNum);
	NotifyData.PushInt(m_ID);
	m_Parent->NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_CLOSED,&NotifyData);

}
bool CThreadWorker::Do(Energy* E){
	if (m_WorkType == MODEL_IO_WORK_TYPE)
	{
		ModelIOWorkProc();
		return true;
	}else if (m_WorkType == MODEL_CENTRAL_NEVER_WORK_TYPE)
	{
		CentralNerveWorkProc();
		return true;
	}else if (m_WorkType == MODEL_TEST_WORK_TYPE)
	{
		UnitTest();
	}
	assert(0);
	return true;
}

void CThreadWorker::UnitTest(){
	try
	{
		m_Parent->UnitTest();
	}
	catch (...)
	{
		ePipeline NotifyData;
		NotifyData.PushString(_T("An exception occurred, ModelIOWork Close"));
		m_Parent->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);
	}

	ePipeline NotifyData;
	NotifyData.PushInt(m_ID);	
	m_Parent->NotifySysState(NOTIFY_MODEL_SCENE,NTID_TEST_WORKER_CLOSED,&NotifyData);

}

//CLockedModelData
//////////////////////////////////////////////////////////////////////////
Model::CLockedModelData::CLockedModelData(){
	m_MaxNerveWorkerNum    = 20;
	m_NerveMsgMaxNumInPipe = 10;
	m_NerveMsgMaxInterval  = 10*1000*1000; //1秒
	m_NerveIdleMaxCount    = 50;

};

Model::CLockedModelData::~CLockedModelData(){


}
void    Model::CLockedModelData::Clear(){
	_CLOCK(&m_Mutex);

	map<int64,CThreadWorker*>::iterator ita =  m_ModelIOWorkerList.begin();
	while (ita != m_ModelIOWorkerList.end())
	{
		CThreadWorker* Worker = ita->second;
		ita->second = NULL;
		assert(!Worker->IsAlive());
		delete Worker;
		ita++;
	}
	m_ModelIOWorkerList.clear();

	map<int64,CThreadWorker*>::iterator itd = m_CentralNerveWorkerList.begin();
	while (itd != m_CentralNerveWorkerList.end())
	{
		CThreadWorker* Worker = itd->second;
		itd->second = NULL;
		assert(!Worker->IsAlive());
		delete Worker;
		itd++;
	}
	m_CentralNerveWorkerList.clear();

	deque<CThreadWorker*>::iterator itc = m_ThreadWorkerPool.begin();
	while(itc != m_ThreadWorkerPool.end())
	{
		CThreadWorker* Worker = *itc;
		*itc = NULL;
		assert(!Worker->IsAlive());
		delete Worker;
		itc++;
	}
	m_ThreadWorkerPool.clear();
}

int64   Model::CLockedModelData::GetNerveMsgInterval(){
	_CLOCK(&m_Mutex);
	return m_NerveMsgMaxInterval;
}
void    Model::CLockedModelData::SetNerveMsgInterval(int32 n){
	_CLOCK(&m_Mutex);
	m_NerveMsgMaxInterval = n;
}
	
int32   Model::CLockedModelData::GetNerveMaxIdleCount(){
	_CLOCK(&m_Mutex);
	return m_NerveIdleMaxCount;	
}
	
void   Model::CLockedModelData::SetNerveMaxIdleCount(int32 n){
	_CLOCK(&m_Mutex);
	m_NerveIdleMaxCount = n;
}

int32  Model::CLockedModelData::GetCentralNerveWorkerNum(){
	_CLOCK(&m_Mutex);
	int32 n = m_CentralNerveWorkerList.size();
	return n;
}

int32 Model::CLockedModelData::GetIOWorkerNum(){
	_CLOCK(&m_Mutex);
	return m_ModelIOWorkerList.size();
}

CThreadWorker* Model::CLockedModelData::CreateThreadWorker(int64 ID,Model* Parent,int32 Type){
	
	if(!Parent->IsAlive())return NULL;

	_CLOCK(&m_Mutex);
	
	CThreadWorker* Worker = NULL;
	deque<CThreadWorker*>::iterator it = m_ThreadWorkerPool.begin();
	if (it != m_ThreadWorkerPool.end())
	{   
		Worker = *it;
		Worker->Dead();
		Worker->m_ID = ID;
		Worker->m_IdleCount =0;
		Worker->m_Parent = Parent;
		m_ThreadWorkerPool.pop_front();
	}else{
		Worker = new CThreadWorker(ID,Parent,Type);
	}
	
	if(!Worker){
		return NULL;
	}

	if (Type == MODEL_CENTRAL_NEVER_WORK_TYPE)
	{
		m_CentralNerveWorkerList[ID]= Worker;
		return Worker;
	}else if (Type == MODEL_IO_WORK_TYPE)
	{
		m_ModelIOWorkerList[ID] = Worker;
		return Worker;
	}
	assert(0);
	return NULL;
}
void   Model::CLockedModelData::DeleteThreadWorker(Model* Parent,int64 ID,int32 Type){

	_CLOCK(&m_Mutex);
	
	assert(Type == MODEL_CENTRAL_NEVER_WORK_TYPE  || Type == MODEL_IO_WORK_TYPE);

	CThreadWorker* Worker = NULL;
	if (Type == MODEL_CENTRAL_NEVER_WORK_TYPE)
	{
		map<int64,CThreadWorker*>::iterator it = m_CentralNerveWorkerList.find(ID);
		assert(it != m_CentralNerveWorkerList.end());
		if (it != m_CentralNerveWorkerList.end())
		{
			Worker = it->second;
			m_CentralNerveWorkerList.erase(it);
			m_ThreadWorkerPool.push_back(Worker);

		}

	}else if (Type == MODEL_IO_WORK_TYPE)
	{
		map<int64,CThreadWorker*>::iterator it = m_ModelIOWorkerList.find(ID);
		assert(it != m_ModelIOWorkerList.end());
		if (it != m_ModelIOWorkerList.end())
		{
			Worker = it->second;
			m_ModelIOWorkerList.erase(it);
			m_ThreadWorkerPool.push_back(Worker);

		}
	}
	//限制ThreadWorkerPool大小为20	
	if(m_ThreadWorkerPool.size()>20)
	{
		deque<CThreadWorker*>::iterator it = m_ThreadWorkerPool.begin();
		CThreadWorker* Worker = *it;
		Worker->Dead();
		it = m_ThreadWorkerPool.erase(it);
		delete Worker;
	}
}

void  Model::CLockedModelData::WaitAllWorkerThreadClosed(Model* Parent){
	assert(!Parent->IsAlive());
	if (Parent->IsAlive())
	{
		return;
	}
	int n = 1;
	while (n)
	{
		SLEEP_MILLI(100);
		_CLOCK(&m_Mutex);	
		n = m_ModelIOWorkerList.size();	
	}

    n = 1;
	while(n){
		SLEEP_MILLI(100);
		_CLOCK(&m_Mutex);	
		n = m_CentralNerveWorkerList.size();
	}
	
	//被删除的worker只是理论上被放入pool中，这里确保线程完全退出
	deque<CThreadWorker*>::iterator it = m_ThreadWorkerPool.begin();
	while(it != m_ThreadWorkerPool.end())
	{
		CThreadWorker* Worker = *it;
		Worker->Dead();
		it++;
	}

}

 bool Model::CLockedModelData::RequestCreateNewCentralNerveWorker(uint32 MsgNum,int64 Interval,uint32& Reason){
	
	_CLOCK(&m_Mutex);		
	if (m_CentralNerveWorkerList.size() == m_MaxNerveWorkerNum)
	{
		Reason =  REASON_LIMIT;
		return FALSE;
	}
/*   Model::Do() is a default central nerve worker, so...
	else if (m_CentralNerveWorkerList.size()==0)
	{
		Reason == REASON_ALWAYS;
		return TRUE;
	}
*/
	else if (MsgNum > m_NerveMsgMaxNumInPipe)//when Current message number exceeded a specified number,start new thread
	{
		Reason = REASON_MSG_TOO_MUCH;
		return TRUE;
	}else if(Interval> m_NerveMsgMaxInterval){ //when exceeded specified interval, there is no  message  be  popped to handled, create  new thread
		Reason = REASON_TIME_OUT;
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
	
#if defined(USING_POCO) 
	initializeNetwork();

#elif defined(USING_WIN32)
	WSADATA wsaData;		
	if (WSAStartup(MAKEWORD(2,2),&wsaData)!=0)return FALSE;		

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ){ 
			WSACleanup();
			return false; 
	}
#endif

	if(!Object::Activation()){
		m_Alive = FALSE;
		return false;
	}

	/*
	int64 ID = CreateTimeStamp();
	CThreadWorker* TestWorker = m_ModelData.CreateThreadWorker(ID,this,MODEL_TEST_WORK_TYPE);
	if (!TestWorker)
	{
		return false;
	}
	if (!TestWorker->Activation())
	{
		return false;
	}
	*/
	return true;
}

void Model::Dead(){
	m_Alive = FALSE;
	m_ModelData.WaitAllWorkerThreadClosed(this);
	m_ModelData.Clear();
	Object::Dead();
}


void  Model::SetLogFlag(uint32 flag){
	m_LogFlag = flag;
}

uint32  Model::GetLogFlag(){
	return m_LogFlag;
}	
int32   Model::GetCpuNum(){
	return m_nCPU;
}
CSuperiorLinkerList*  Model::GetSuperiorLinkerList(){
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

void  Model::NotifySysState(int64 NotifyType,int64 NotifyID,ePipeline* Data){
	switch (NotifyType)
	{
	case NOTIFY_MODEL_SCENE:
		{

			switch(NotifyID){
			case NTID_NERVE_MSG_NUM:
				{
				}
				break;	

			case NTID_NERVE_THREAD_JOIN:
				{
					int64 n = Data->PopInt();
				}
				break;
			case NTID_NERVE_THREAD_CLOSED:
				{
					int64 nThreadNum = Data->PopInt();
					int64 nClosedThreadID = Data->PopInt();
					CLockedModelData*  LockedData = GetModelData();
					LockedData->DeleteThreadWorker(this,nClosedThreadID,MODEL_CENTRAL_NEVER_WORK_TYPE);

				}
				break;
			case NTID_NERVE_THREAD_LIMIT:
				{


				}
				break;
			case NTID_NERVE_THREAD_FAIL:
				{

				}
				break;
			case NTID_IO_WORKER_CLOSED:
				{
					int64 ID = Data->PopInt();
					CLockedModelData* ModelData = GetModelData();
					ModelData->DeleteThreadWorker(this,ID,MODEL_IO_WORK_TYPE);
				}
				break;
			case NTID_CONNECT_FAIL:
				{

				}
				break;
			case NTID_CONNECT_OK:
				{

				}
				break;
			case NTID_TEST_WORKER_CLOSED:
				{
					int64 ID = Data->PopInt();
					CLockedModelData* ModelData = GetModelData();
					ModelData->DeleteThreadWorker(this,ID,MODEL_IO_WORK_TYPE);
					
				}
				break;
			}
		}
		break;
	}
}

BOOL  Model::CreateCentralNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){
	int32 n = GetCentralNerveMsgNum();
	ePipeline Data;
	Data.PushInt(n);
	NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_MSG_NUM,&Data);

	int64 t = NewMsgPushTime-LastMsgPopTime; //convert into second

	//If more than 10 messages in the central nerve or there is message waiting for being handled more than 2 seconds, create a new thread
	uint32 Reason ;
	bool ret = m_ModelData.RequestCreateNewCentralNerveWorker(n,t,Reason);
	if (!ret)
	{
		if (Reason == REASON_LIMIT)
		{
			NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_LIMIT,NULL);
		}
		return FALSE;
	}

	CThreadWorker* CentralNerveWork =m_ModelData.CreateThreadWorker(NewMsgPushTime,this,MODEL_CENTRAL_NEVER_WORK_TYPE);
	if (!CentralNerveWork)
	{
		return FALSE;
	}
	if (CentralNerveWork->Activation())
	{
		int n =m_ModelData.GetCentralNerveWorkerNum();
		ePipeline Data;
		Data.PushInt(n);
		Data.PushInt(CentralNerveWork->m_ID);
		NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_JOIN,&Data);

		return TRUE;
	}else{
		m_ModelData.DeleteThreadWorker(this,CentralNerveWork->m_ID,MODEL_CENTRAL_NEVER_WORK_TYPE);
		NotifySysState(NOTIFY_MODEL_SCENE,NTID_NERVE_THREAD_FAIL,NULL);
		return FALSE;
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
	assert(NewMsgPushTime!=LastMsgPopTime);

	BOOL ret = CreateCentralNerveWorkerStrategy(NewMsgPushTime,LastMsgPopTime);
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


bool Model::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
	assert(TimeOut>0&& TimeOut<60);

	if (m_SuperiorList.HasLinker(Address,Port))
	{
		error = _T("the connection has existed");
		return false;
	}

	ePipeline Param;
	Param.PushString(Address);
	Param.PushInt(Port);
	Param.PushInt(TimeOut);

	CLinker Linker;
	m_SuperiorList.CreateLinker(Linker,this,ID,Param);
	
	//CUserConnectLinkerPipe* ConLinkerPipe = new CUserConnectLinkerPipe(this,ID,Address,Port,TimeOut);

	if(!Linker.IsValid()){
		error = _T("Create connect pipe fail");
		return false; 
	};

	CUserConnectLinkerPipe* ConLinkerPipe = (CUserConnectLinkerPipe*)Linker.Release();
	if(!ConLinkerPipe->Init(error)){
		delete ConLinkerPipe;
		return false;	
	};

	if (bBlock)
	{
		if(!ConLinkerPipe->BlockConnect(error)){
			delete ConLinkerPipe;
			return false;
		};
		
	}else{
		//Note: auto connect by thread function
		m_SuperiorList.AddLinker(ConLinkerPipe);
	}

	//if there is not thread to handle,create new one
	CLockedModelData* ModelData = GetModelData();
	if (ModelData->GetIOWorkerNum()==0)
	{
		int64 WorkerID = CreateTimeStamp();

		CThreadWorker* IOWork = m_ModelData.CreateThreadWorker(WorkerID,this,MODEL_IO_WORK_TYPE);
		if (!IOWork||!IOWork->Activation())
		{
			if(IOWork)m_ModelData.DeleteThreadWorker(this,IOWork->m_ID,MODEL_IO_WORK_TYPE);
			error = _T("Activate IOWorker fail");

			m_SuperiorList.DeleteLinker(ID);
			return false;
		}
	}
	return TRUE;
}

bool Model::Do(Energy* E){

	try{
		while (IsAlive())
		{
			CMsg Msg;
			PopCentralNerveMsg(Msg);

			if (Msg.IsValid())
			{			
				CentralNerveMsgProc(Msg);
			}
			else{
				SLEEP_MILLI(10);
			}
			
		}
	}
	catch(...){
		ePipeline NotifyData;
		NotifyData.PushString(_T("Where default CentralNerveWork have an exception occurred"));
		NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);

	}
	return true;
}
}; //end namespace ABSTRACT
