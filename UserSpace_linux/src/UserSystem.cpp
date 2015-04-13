// UserSystem.cpp: implementation of the CUserSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "UserSystem.h"

//#include "MsgList.h"

//CUserModelIOWork
//////////////////////////////////////////////////////////////////////////

CUserSystem::CUserModelIOWork::CUserModelIOWork(int64 ID, CUserSystem* Parent)
:Model::CModelIOWork(ID,Parent){
	
}

CUserSystem::CUserModelIOWork::~CUserModelIOWork(){
	if (m_Alive)
	{
		Dead();
	}
};

bool CUserSystem::CUserModelIOWork::Activation()
{
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}
	return TRUE;
}

void CUserSystem::CUserModelIOWork::Dead(){
	m_Alive = FALSE;
	pthread_join(m_hObjectDefaultThread,NULL);
}


//CUserCentralNerveWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserCentralNerveWork::CUserCentralNerveWork(int64 ID, CUserSystem* Parent)
:Model::CCentralNerveWork(ID,Parent)
{
	assert(Parent);	
};

CUserSystem::CUserCentralNerveWork::~CUserCentralNerveWork(){
	if (m_Alive)
	{
		Dead();
	}
};

bool CUserSystem::CUserCentralNerveWork::Activation(){
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}

	return TRUE;
};

void CUserSystem::CUserCentralNerveWork::Dead(){
	m_Alive = FALSE;
	pthread_join(m_hObjectDefaultThread,NULL);
}

//CUserSystemIOWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserSystemIOWork::CUserSystemIOWork(int64 ID, CUserSystem* Parent)
:System::CSystemIOWork(ID,Parent)
{
	
};

CUserSystem::CUserSystemIOWork::~CUserSystemIOWork(){
	if (m_Alive)
	{
		Dead();
	}
};

bool CUserSystem::CUserSystemIOWork::Activation(){
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}

	return TRUE;
};

void CUserSystem::CUserSystemIOWork::Dead(){
	m_Alive = FALSE;
	pthread_join(m_hObjectDefaultThread,NULL);
}


//CUserNerveWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserNerveWork::CUserNerveWork(int64 ID, CUserSystem* Parent)
:CNerveWork(ID,Parent)
{
	
}

CUserSystem::CUserNerveWork::~CUserNerveWork(){
	if (m_Alive)
	{
		Dead();
	}
};

bool CUserSystem::CUserNerveWork::Activation(){
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}

	return TRUE;
};

void CUserSystem::CUserNerveWork::Dead(){
	m_Alive = FALSE;
	pthread_join(m_hObjectDefaultThread,NULL);
}


//CUserLockedSystemData
//////////////////////////////////////////////////////////////////////////

CUserSystem::CUserLockedSystemData::CUserLockedSystemData(CABMutex* mutex)
:CLockedSystemData(mutex)
{
};

CUserSystem::CUserLockedSystemData::~CUserLockedSystemData(){
	
};

CUserLinkerPipe* CUserSystem::CUserLockedSystemData::FindAcceptor(int32 Port){
	CLock lk(m_pMutex);
	
	map<int32,CUserLinkerPipe*>::iterator it = m_AcceptorList.find(Port);
	if (it != m_AcceptorList.end())
	{
		return it->second;
	}
	return NULL;
}

void CUserSystem::CUserLockedSystemData::AddAcceptor(int32 Port,CUserLinkerPipe* Acceptor){
	CLock lk(m_pMutex);
	
	map<int32,CUserLinkerPipe*>::iterator it = m_AcceptorList.find(Port);
	if (it != m_AcceptorList.end())
	{
		return;
	}
	
	m_AcceptorList[Port] = Acceptor;
};

void CUserSystem::CUserLockedSystemData::DelAcceptor(int32 Port){
	CLock lk(m_pMutex);
	map<int32,CUserLinkerPipe*>::iterator it = m_AcceptorList.find(Port);
	if (it != m_AcceptorList.end())
	{
		return;
	} 
	CUserLinkerPipe* Acceptor = it->second;
	Acceptor->Close(); //A Linker error is thrown that tells the system to delete the actual CUserLinkerPipe
    m_AcceptorList.erase(it);
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUserSystem::CUserSystem(CSystemInitData* InitData)
:System(InitData),m_nCPU(InitData->m_nCPU)
{
	
};
CUserSystem::~CUserSystem(){
	if (m_Alive)
	{
		Dead();
	}
}
bool CUserSystem::Activation(){

	if(m_Alive)return TRUE;

	if (!System::Activation())
	{
		return FALSE;
	}
	
	m_Alive  = TRUE;
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}

	return TRUE;

};

void CUserSystem::Dead(){
	m_Alive = FALSE;
	System::Dead();	
	pthread_join(m_hObjectDefaultThread,NULL);

};

System::CNerveWork* CUserSystem::CreateNerveWorker(int64 ID,System* Parent,uint32 Reason){
	CUserNerveWork* NerveWork = new CUserNerveWork(ID,this);
	return NerveWork;	
}

CUserLinkerPipe* CUserSystem::CreateClientLinkerPipe(){
	int64 ID = CreateTimeStamp();
	CUserLinkerPipe* Linker = new CUserLinkerPipe(m_ClientSitMutex,this,ID);
	return Linker;
}

void  CUserSystem::NotifySysState(int64 NotifyID,ePipeline* Data /*= NULL*/){
	if(MNOTIFY_IO_WORK_THREAD_CLOSE == NotifyID){
		int64 ID = Data->PopInt();
		CLockedModelData* ModelData = GetModelData();
		ModelData->DeleteIOWork(ID);
	}
}

bool CUserSystem::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
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
	}

	//if no thread to handle this connection, create new one
	CLockedModelData* ModelData = GetModelData();
	if (ModelData->GetIOWorkNum()<m_nCPU*2)
	{
		int64 ID = CreateTimeStamp();
		CUserModelIOWork* IOWork = new CUserModelIOWork(ID,this);
		if (!IOWork || !IOWork->Activation())
		{
			error = _T("Create connect IOWork fail");
			return FALSE;
		}
		ModelData->AddIOWork(IOWork);
	}
	return TRUE;
}


bool CUserSystem::OpenPort(int32 Port,tstring& error,bool bIP6){
   	int64 ID = CreateTimeStamp();

	CUserAcceptLinkerPipe* Acceptor = new CUserAcceptLinkerPipe(this,ID,Port);
    
	if(!Acceptor){
		error = _T("Create acceptor fail");
		return FALSE;
	}
		
	if(!Acceptor->Init(error)){
		delete Acceptor;
		return FALSE;	
	};

	CUserLockedSystemData* SystemData = (CUserLockedSystemData*)GetSystemData();
	SystemData->AddAcceptor(Port,Acceptor);	
	
	CLockedLinkerList* ClientList = GetClientLinkerList();
	ClientList->AddLinker(Acceptor);

	//if no thread to handle this task, create new one
    if (SystemData->GetIOWorkNum()<m_nCPU*2)
    {
		int64 ID = CreateTimeStamp();
		CUserSystemIOWork* IOWork = new CUserSystemIOWork(ID,this);
		if(!IOWork || !IOWork->Activation()){
			error = _T("Create Acceptor IOWork fail");
			return FALSE;
		}
		SystemData->AddIOWork(IOWork);
    }
	return TRUE;
};

void CUserSystem::ClosePort(int32 Port){
    CUserLockedSystemData* SystemData = (CUserLockedSystemData*)GetSystemData();
    SystemData->DelAcceptor(Port);
}
