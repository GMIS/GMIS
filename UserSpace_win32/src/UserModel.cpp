// UserModel.cpp: implementation of the CUserModel class.
//
//////////////////////////////////////////////////////////////////////
#pragma  warning(disable:4786)

#include "UserLinkerPipe.h"
#include "UserModel.h"
#include <process.h>

//CUserModelIOWork
//////////////////////////////////////////////////////////////////////////

CUserModel::CUserModelIOWork::CUserModelIOWork(int64 ID, CUserModel* Parent)
:CModelIOWork(ID,Parent){
	
}
CUserModel::CUserModelIOWork::~CUserModelIOWork(){
	if (m_Alive)
	{
		Dead();
	}
}
bool CUserModel::CUserModelIOWork::Activation()
{
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	uint32 ThreadID = 0;
	m_hObjectDefaultThread = (HANDLE)_beginthreadex(NULL, // Security
		0,							                     // Stack size - use default
		ObjectDefaultThreadFunc,     		
		(void*)this,	      
		0,					                			 // Init flag
		&ThreadID);					                     // Thread address
	
	if( m_hObjectDefaultThread == NULL ){
		int ret = ::GetLastError();
		assert(0);
		return FALSE;	
	};        

	
	m_hObjectDefaultThreadID = ThreadID;
	
	return TRUE;
	
}

void CUserModel::CUserModelIOWork::Dead(){
	m_Alive = FALSE;
	WaitForSingleObject (m_hObjectDefaultThread, INFINITE);
}


//CUserCentralNerveWork
//////////////////////////////////////////////////////////////////////////
CUserModel::CUserCentralNerveWork::CUserCentralNerveWork(int64 ID, CUserModel* Parent)
:CCentralNerveWork(ID,Parent)
{
	assert(Parent);	
}

CUserModel::CUserCentralNerveWork::~CUserCentralNerveWork(){
	if (m_Alive)
	{
		Dead();
	}
}

bool CUserModel::CUserCentralNerveWork::Activation(){
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	uint32 ThreadID = 0;
	m_hObjectDefaultThread = (HANDLE)_beginthreadex(NULL, // Security
		0,							                     // Stack size - use default
		ObjectDefaultThreadFunc,     		
		(void*)this,	      
		0,					                			 // Init flag
		&ThreadID);					                     // Thread address
	
	if( m_hObjectDefaultThread == NULL ){
		int ret = ::GetLastError();
		assert(0);
		return FALSE;	
	};        

	
	m_hObjectDefaultThreadID = ThreadID;
	return TRUE;
};

void CUserModel::CUserCentralNerveWork::Dead(){
	m_Alive = FALSE;
	WaitForSingleObject (m_hObjectDefaultThread, INFINITE);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUserModel::CUserModel(CModelInitData* InitData)
:Model(InitData),m_nCPU(InitData->m_nCPU)
{
};
CUserModel::~CUserModel(){
	if (m_Alive)
	{
		Dead();
	}
}
bool CUserModel::Activation(){
	
	if(m_Alive)return TRUE;
	
	WSADATA wsaData;		
	if (WSAStartup(MAKEWORD(2,2),&wsaData)!=0)return FALSE;		
	
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ){ 
		WSACleanup();
		return FALSE; 
	}		
	

	if (!Model::Activation())
	{
		return FALSE;
	}
	

	m_Alive  = TRUE;
	uint32 ThreadID = 0;
	m_hObjectDefaultThread = (HANDLE)_beginthreadex(NULL, // Security
		0,							                     // Stack size - use default
		ObjectDefaultThreadFunc,     		
		(void*)this,	      
		0,					                			 // Init flag
		&ThreadID);					                     // Thread address
	
	assert( m_hObjectDefaultThread != NULL );        
	
	m_hObjectDefaultThreadID = ThreadID;
	return TRUE;
}

void CUserModel::Dead(){
	m_Alive = FALSE;
	Model::Dead();
	WaitForSingleObject (m_hObjectDefaultThread, INFINITE);
	WSACleanup();
}

Model::CCentralNerveWork* CUserModel::CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason){
	CUserCentralNerveWork* NerveWork = new CUserCentralNerveWork(ID,this);
	
	return NerveWork;
} 

bool CUserModel::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
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
