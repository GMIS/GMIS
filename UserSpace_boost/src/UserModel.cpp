// UserModel.cpp: implementation of the CUserModel class.
//
//////////////////////////////////////////////////////////////////////
#include "UserModel.h"
#include "UserLinkerPipe.h"

//CUserModelIOWork
//////////////////////////////////////////////////////////////////////////
CUserModel::CUserModelIOWork::CUserModelIOWork(int64 ID, CUserModel* Parent)
:CModelIOWork(ID,Parent),m_Thread(NULL){
	
}
CUserModel::CUserModelIOWork::~CUserModelIOWork(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserModel::CUserModelIOWork::Activation()
{
	if(m_Alive)return TRUE;
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	};
	m_Alive = TRUE; //先设置为TRUE,避免线程函数自动退出
	m_Thread = new boost::thread(ObjectDefaultThreadFunc,this);
	m_Alive = m_Thread?TRUE:FALSE;
	return m_Alive;

}

void CUserModel::CUserModelIOWork::Dead(){
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}

bool CUserModel::CUserModelIOWork::IsAlive(){
	if (m_Alive || (m_Thread && m_Thread->joinable()))
	{
		return TRUE;
	}
	return FALSE;
}


bool CUserModel::CUserModelIOWork::Do(Energy* E){
    try{
		CUserModel*  UserModel = (CUserModel*)m_Parent;
		UserModel->m_IOService.run();
				
        if (m_Parent->IsAlive())
        {		
			ePipeline Data;
			Data.PushInt(m_ID);
			UserModel->NotifySysState(MNOTIFY_IO_WORK_THREAD_CLOSE,&Data);			
			CLockedModelData* LockedData = m_Parent->GetModelData();
			LockedData->DeleteIOWork(m_ID);
        }
		
	}catch(...)
	{
		assert(0);
		ePipeline Data;
		Data.PushString(_T("An exception occurred, ModelIOWork Close"));
		m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
		CLockedModelData* LockedData = m_Parent->GetModelData();
		LockedData->DeleteIOWork(m_ID);
	}
	m_Alive = FALSE;
	return TRUE;
};

//CUserCentralNerveWork
//////////////////////////////////////////////////////////////////////////
CUserModel::CUserCentralNerveWork::CUserCentralNerveWork(int64 ID, CUserModel* Parent)
:CCentralNerveWork(ID,Parent),m_Thread(NULL)
{
	assert(Parent);	
}
CUserModel::CUserCentralNerveWork::~CUserCentralNerveWork(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserModel::CUserCentralNerveWork::Activation(){
	if(m_Alive)return TRUE;	
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	};
	m_Alive = TRUE;
	m_Thread = new boost::thread(ObjectDefaultThreadFunc,this);
	m_Alive = m_Thread?TRUE:FALSE;
	return m_Alive;
};

void CUserModel::CUserCentralNerveWork::Dead(){
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}

bool CUserModel::CUserCentralNerveWork::IsAlive(){
	if (m_Alive || (m_Thread && m_Thread->joinable()))
	{
		return TRUE;
	}
	return FALSE;
}

//Connectpair
//////////////////////////////////////////////////////////////////////////
CUserModel::ConnectPair::ConnectPair(int64 ID,boost::asio::io_service&  IOService,int32 TimeOut)
:m_ID(ID),m_TimeCount(TimeOut),
m_pSocket(new boost::asio::ip::tcp::socket(IOService)),
m_Timer(IOService,boost::posix_time::seconds(1)) //1秒为间隔
{
	
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserModel::CUserModel(CModelInitData* InitData)
:Model(InitData),m_nCPU(InitData->m_nCPU),m_Thread(NULL)
{

};

CUserModel::~CUserModel(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
};

bool CUserModel::Activation(){
	
	if(m_Alive)return TRUE;
	
	if (!Model::Activation())
	{
		return FALSE;
	}
	
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	};
	m_Alive = TRUE;
	m_Thread = new boost::thread(ObjectDefaultThreadFunc,this);
	m_Alive = m_Thread?TRUE:FALSE;
	return m_Alive;
}

void CUserModel::Dead(){
	m_Alive = FALSE;

	m_IOService.stop();

	Model::Dead();
	
	CLockedLinkerList* List = GetSuperiorLinkerList(); //必须在stop()后手工删除，否则m_IOService被析构后再自动析构删除socket导致错误
	List->DeleteAllLinker();

	if(m_Thread){
		m_Thread->join();
	}
}

void  CUserModel::NotifySysState(int64 NotifyID,ePipeline* Data /*= NULL*/){
	/*
	if(MNOTIFY_IO_WORK_THREAD_CLOSE == NotifyID){
		int64 ID = Data->PopInt64(); 
		CLockedModelData* ModelData = GetModelData();  线程自己做这一步，避免用户遗忘处理
		ModelData->DeleteIOWork(ID);
	}
	*/
}

Model::CCentralNerveWork* CUserModel::CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason){
	CUserCentralNerveWork* NerveWork = new CUserCentralNerveWork(ID,this);	
	return NerveWork;
} 

bool CUserModel::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
	assert(TimeOut>0&&TimeOut<60);

	if (m_IOService.stopped())
	{
		m_IOService.reset();
	}

	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(Address),Port);
	
	boost::shared_ptr<ConnectPair> cp(new ConnectPair(ID,m_IOService,TimeOut));
	
	if (bBlock) //等到连接成功后再返回
	{
		boost::system::error_code ec;
		cp->m_pSocket->connect(ep,ec);
		
		if(ec){
			error =  _T("Connect fail");
			return FALSE;
		}

		CUserLinkerPipe* Linker = new CUserLinkerPipe(this,0,_T("ConnectServer"));
		Linker->SetSourceID(cp->m_ID);
		GetSuperiorLinkerList()->AddLinker(Linker);
		Linker->AttachSocket(cp->m_pSocket);
   
	}else{

		cp->m_pSocket->async_connect(ep,boost::bind(&CUserModel::ConnectHandler,this,boost::asio::placeholders::error,cp));

		cp->m_Timer.async_wait(boost::bind(&CUserModel::ConnectTimeoutHandler,this,boost::asio::placeholders::error,cp));

	}

	//如果没有处理线程则生成一个
	CLockedModelData* ModelData = GetModelData();
	if (ModelData->GetIOWorkNum()<m_nCPU*2)
	{
		int64 ID = CreateTimeStamp();
		CUserModelIOWork* IOWork = new CUserModelIOWork(ID,this);
		if (!IOWork || !IOWork->Activation())
		{
			error = _T("Create ModelIOWork fail");
			return FALSE;
		}
		ModelData->AddIOWork(IOWork);
	}

	return TRUE;
}

void CUserModel::ConnectHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp){
	if (error)
	{
		ePipeline Data;
		Data.PushString(error.message());
		NotifySysState(MNOTIFY_CONNECT_FAIL,&Data);
		return;
	}
	cp->m_Timer.cancel();
	
//	int64 ID = CreateTimeStamp();
	CUserLinkerPipe* Linker = new CUserLinkerPipe(this,0,_T("ConnectServer"));
	Linker->SetSourceID(cp->m_ID);
    GetSuperiorLinkerList()->AddLinker(Linker);
	Linker->AttachSocket(cp->m_pSocket);
}

void CUserModel::ConnectTimeoutHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp){
	if (error)
	{
		cp->m_pSocket->close(); //应该会引发错误，自动转向错误处理	
		ePipeline Data;
		NotifySysState(MNOTIFY_CONNECT_FAIL,&Data);
		cp->m_TimeCount = 0;
		return;
	}
	
	CLockedLinkerList* LinkerList = GetSuperiorLinkerList();
	CLinker Linker;
	LinkerList->GetLinker(cp->m_ID,Linker);
	if (Linker.IsValid()) //已经连接
	{
		cp->m_TimeCount = 0;
		return;	
	}	
	
	if (cp->m_TimeCount)
	{
		--cp->m_TimeCount;
		cp->m_Timer.expires_at(cp->m_Timer.expires_at() + boost::posix_time::seconds(1));
		cp->m_Timer.async_wait(boost::bind(&CUserModel::ConnectTimeoutHandler,this,boost::asio::placeholders::error,cp));

	}else{
		cp->m_pSocket->close();
		cp->m_Timer.cancel();
		ePipeline Data;
		NotifySysState(MNOTIFY_CONNECT_FAIL,&Data);
		cp->m_TimeCount = 0;
	}

}
