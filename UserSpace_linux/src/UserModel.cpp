// UserModel.cpp: implementation of the CUserModel class.
//
//////////////////////////////////////////////////////////////////////

#include "UserLinkerPipe.h"
#include "UserModel.h"


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
	if(m_Alive)return true;
	m_Alive  = true;
	
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}
	
	return true;
	
}

void CUserModel::CUserModelIOWork::Dead(){
	m_Alive = false;
	pthread_join(m_hObjectDefaultThread,NULL);
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
	if(m_Alive)return true;
	m_Alive  = true;
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}

	return false;
};

void CUserModel::CUserCentralNerveWork::Dead(){
	m_Alive = false;
	pthread_join(m_hObjectDefaultThread,NULL);
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
	

	if (!Model::Activation())
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
}

void CUserModel::Dead(){
	m_Alive = FALSE;
	Model::Dead();
	pthread_join(m_hObjectDefaultThread,NULL);

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
	//���û�д����߳������һ��
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
