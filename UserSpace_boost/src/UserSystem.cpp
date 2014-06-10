// UserSystem.cpp: implementation of the CUserSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "UserLinkerPipe.h"
#include "UserSystem.h"
#include "Msg.h"

//CUserModelIOWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserModelIOWork::CUserModelIOWork(int64 ID, CUserSystem* Parent)
:Model::CModelIOWork(ID,Parent),m_Thread(NULL){
	
}
CUserSystem::CUserModelIOWork::~CUserModelIOWork(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserSystem::CUserModelIOWork::Activation()
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

void CUserSystem::CUserModelIOWork::Dead(){
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}

bool CUserSystem::CUserModelIOWork::IsAlive(){
	if (m_Alive || (m_Thread && m_Thread->joinable()))
	{
		return TRUE;
	}
	return FALSE;
}

bool CUserSystem::CUserModelIOWork::Do(Energy* E){
	try{
		CUserSystem*  UserSystem = (CUserSystem*)m_Parent;
		UserSystem->m_IOService.run();
		if (m_Parent->IsAlive())
		{
			ePipeline Data;
			Data.PushInt(m_ID);
			UserSystem->NotifySysState(MNOTIFY_IO_WORK_THREAD_CLOSE,&Data);
		}
	}catch(...){
		assert(0);
		ePipeline Data;
		Data.PushString(_T("An exception occurred, ModelIOWork Close"));
		m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
	}
	
	CLockedModelData* LockedData = m_Parent->GetModelData();
	LockedData->DeleteIOWork(m_ID); //注意：由于是自删除，此行应作为最后一行
	m_Alive = FALSE;
	return TRUE;
};

//CUserCentralNerveWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserCentralNerveWork::CUserCentralNerveWork(int64 ID, CUserSystem* Parent)
:Model::CCentralNerveWork(ID,Parent),m_Thread(NULL)
{
	assert(Parent);	
}
CUserSystem::CUserCentralNerveWork::~CUserCentralNerveWork(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserSystem::CUserCentralNerveWork::Activation(){
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

void CUserSystem::CUserCentralNerveWork::Dead(){
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}

bool CUserSystem::CUserCentralNerveWork::IsAlive(){
	if (m_Alive || (m_Thread && m_Thread->joinable()))
	{
		return TRUE;
	}
	return FALSE;
}

//CUserNerveWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserNerveWork::CUserNerveWork(int64 ID, CUserSystem* Parent)
:CNerveWork(ID,Parent),m_Thread(NULL)
{
	
}
CUserSystem::CUserNerveWork::~CUserNerveWork(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserSystem::CUserNerveWork::Activation(){
	if(m_Alive)return TRUE;

	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	};
	m_Alive  = TRUE;
	m_Thread = new boost::thread(ObjectDefaultThreadFunc,this);
	m_Alive = m_Thread?TRUE:FALSE;
	return m_Alive;
};

void CUserSystem::CUserNerveWork::Dead(){
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}
bool CUserSystem::CUserNerveWork::IsAlive(){
	if (m_Alive || (m_Thread && m_Thread->joinable()))
	{
		return TRUE;
	}
	return FALSE;
}
//CUserSystemIOWork
//////////////////////////////////////////////////////////////////////////
CUserSystem::CUserSystemIOWork::CUserSystemIOWork(int64 ID, CUserSystem* Parent)
:System::CSystemIOWork(ID,Parent),m_Thread(NULL)
{
	
};
CUserSystem::CUserSystemIOWork::~CUserSystemIOWork(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserSystem::CUserSystemIOWork::Activation(){
	if(m_Alive)return TRUE;

	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	};
	m_Alive  = TRUE;
	m_Thread = new boost::thread(ObjectDefaultThreadFunc,this);
	m_Alive = m_Thread?TRUE:FALSE;
	return m_Alive;
};

void CUserSystem::CUserSystemIOWork::Dead(){
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}

bool CUserSystem::CUserSystemIOWork::IsAlive(){
	if (m_Alive || (m_Thread && m_Thread->joinable()))
	{
		return TRUE;
	}
	return FALSE;
}

bool CUserSystem::CUserSystemIOWork::Do(Energy* E){
	ePipeline Data;
	try{
		CUserSystem*  UserSystem = (CUserSystem*)m_Parent;
		UserSystem->m_IOService.run();
		
		if (m_Parent->IsAlive()) 
		{
			ePipeline Data;
			Data.PushInt(m_ID);
			UserSystem->NotifySysState(SNOTIFY_IO_WORK_THREAD_CLOSE,&Data);
		}
	}catch(...){
		assert(0);
		Data.PushString(_T("An exception occurred, SystemIOWork Close"));
		m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
	}
	CLockedSystemData* LockedData = m_Parent->GetSystemData();
	LockedData->DeleteIOWork(m_ID);
	m_Alive = FALSE;
	return TRUE;
};

//AccptPair
//////////////////////////////////////////////////////////////////////////
CUserSystem::AcceptPair::AcceptPair(boost::asio::io_service&  IOService)
	:m_pSocket(new boost::asio::ip::tcp::socket(IOService)),
	 m_Acceptor(IOService)
{	
};

//Connectpair
//////////////////////////////////////////////////////////////////////////
CUserSystem::ConnectPair::ConnectPair(int64 ID,boost::asio::io_service&  IOService,int32 TimeOut)
:m_ID(ID),m_TimeCount(TimeOut),
 m_pSocket(new boost::asio::ip::tcp::socket(IOService)),
 m_Timer(IOService,boost::posix_time::seconds(1))
{

}

//CUserLockedSystemData
//////////////////////////////////////////////////////////////////////////


CUserSystem::CUserLockedSystemData::CUserLockedSystemData(CABMutex* mutex)
:CLockedSystemData(mutex)
{
};

CUserSystem::CUserLockedSystemData::~CUserLockedSystemData(){

};

void CUserSystem::CUserLockedSystemData::AddAcceptor(CUserSystem* System, int32 Port,bool bIP6){
	CLock lk(m_pMutex);

	map<int32,boost::shared_ptr<AcceptPair>>::iterator it = m_AcceptorList.find(Port);
	if (it != m_AcceptorList.end())
	{
		return;
	}
	
	boost::asio::ip::tcp tcpversion= bIP6?boost::asio::ip::tcp::v6():boost::asio::ip::tcp::v4();
	
	boost::asio::ip::tcp::endpoint endpoint(tcpversion, Port);
	
	boost::shared_ptr<AcceptPair> ap(new AcceptPair(System->m_IOService));
	ap->m_Acceptor.open(endpoint.protocol());
	ap->m_Acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	ap->m_Acceptor.bind(endpoint);
	ap->m_Acceptor.listen();
	m_AcceptorList[Port] = ap;
	System->StartAccept(ap);
	

};

void CUserSystem::CUserLockedSystemData::DelAcceptor(int32 Port){
	CLock lk(m_pMutex);
	map<int32,boost::shared_ptr<AcceptPair>>::iterator it = m_AcceptorList.find(Port);
	if (it != m_AcceptorList.end())
	{
		return;
	} 
	boost::shared_ptr<AcceptPair>& ap = it->second;
	ap->m_Acceptor.close();
	ap->m_pSocket->close();
    m_AcceptorList.erase(it);
};

void CUserSystem::CUserLockedSystemData::Dead(){
	map<int32,boost::shared_ptr<AcceptPair>>::iterator it = m_AcceptorList.begin();
	while(it != m_AcceptorList.end()){
		boost::shared_ptr<AcceptPair>& ap  = it->second;
		ap->m_Acceptor.close();
		ap->m_pSocket->close();
		it++;
	}
	m_AcceptorList.clear();

	CLockedSystemData::Dead();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserSystem::CUserSystem(CSystemInitData* InitData)
:System(InitData),m_nCPU(InitData->m_nCPU),m_IOService(),m_Thread(NULL)
{

};
CUserSystem::~CUserSystem(){
	assert(!m_Alive);
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	}
}
bool CUserSystem::Activation(){
	
	if(m_Alive)return TRUE;
	
	if (!System::Activation())
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
};

void CUserSystem::Dead(){
	m_Alive = FALSE;
	
	m_IOService.stop();
	
	System::Dead();	

	CLockedLinkerList* SuperiorList = GetSuperiorLinkerList(); //必须在stop()后手工删除，否则m_IOService被析构后再自动析构删除socket导致错误
	SuperiorList->DeleteAllLinker();

	CLockedLinkerList* ClientList = GetClientLinkerList();
	ClientList->DeleteAllLinker();

	if(m_Thread){
		m_Thread->join();
	}
};

void CUserSystem::StartAccept(boost::shared_ptr<AcceptPair> ap){
	ap->m_pSocket.reset(new boost::asio::ip::tcp::socket(m_IOService));
	ap->m_Acceptor.async_accept(*(ap->m_pSocket), boost::bind(&CUserSystem::AcceptHandler,this,boost::asio::placeholders::error,ap));	
}

CUserLinkerPipe* CUserSystem::CreateClientLinkerPipe(){
	int64 ID = CreateTimeStamp();
	CUserLinkerPipe* Linker = new CUserLinkerPipe(m_ClientSitMutex,this,ID);
	return Linker;
}

System::CNerveWork* CUserSystem::CreateNerveWorker(int64 ID,System* Parent,uint32 Reason){
	CUserNerveWork* NerveWork = new CUserNerveWork(ID,this);
	return NerveWork;	
}

bool CUserSystem::OpenPort(int32 Port,tstring& error,bool bIP6){
	if (m_IOService.stopped())
	{
		m_IOService.reset();
	}
	
	CUserLockedSystemData* SystemData = (CUserLockedSystemData*)GetSystemData();
	SystemData->AddAcceptor(this,Port,bIP6);


	//如果没有处理线程则生成一个
	if (SystemData->GetIOWorkNum()<m_nCPU*2)
    {
		int64 ID = CreateTimeStamp();
		CUserSystemIOWork* IOWork = new CUserSystemIOWork(ID,this);
		if(!IOWork ||!IOWork->Activation()){
			error = _T("Create acceptor IOWork fail");
			return FALSE;
		}
		SystemData->AddIOWork(IOWork);
    }

  	
	return TRUE;
}

void CUserSystem::ClosePort(int32 Port){
    CUserLockedSystemData* SystemData = (CUserLockedSystemData*)GetSystemData();
    SystemData->DelAcceptor(Port);
}

void  CUserSystem::NotifySysState(int64 NotifyID,ePipeline* Data /*= NULL*/){

	if(MNOTIFY_IO_WORK_THREAD_CLOSE == NotifyID){
/*		int64 ID = Data->PopInt64(); 已经有线程自己完成
		CLockedModelData* ModelData = GetModelData();
		ModelData->DeleteIOWork(ID);
*/	}else if (SNOTIFY_IO_WORK_THREAD_CLOSE == NotifyID){
/*		int64 ID = Data->PopInt64();
		CLockedSystemData* SystemData = GetSystemData();
		SystemData->DeleteIOWork(ID);
*/	}
}
bool CUserSystem::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
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
		
		cp->m_pSocket->async_connect(ep,boost::bind(&CUserSystem::ConnectHandler,this,boost::asio::placeholders::error,cp));
		
		cp->m_Timer.async_wait(boost::bind(&CUserSystem::ConnectTimeoutHandler,this,boost::asio::placeholders::error,cp));
		
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

void CUserSystem::AcceptHandler(const boost::system::error_code& error, boost::shared_ptr<AcceptPair> ap){
	if (error)
	{
		ePipeline Data;
		AnsiString s = error.message();		
		Data.PushString(s);
		NotifySysState(SNOTIFY_LISTEN_FAIL,&Data);
		return;
	}
	
	CUserLinkerPipe* Linker = CreateClientLinkerPipe();
	
	if(Linker== NULL){
		ap->m_pSocket->close();
		return;
	}
	CLockedLinkerList* ClientList= GetClientLinkerList();
	ClientList->AddLinker(Linker);

	Linker->AttachSocket(ap->m_pSocket);	
	
	/*先不忙产生显式对话，而是要求对方发送认证信息，认证后才生成对应的对话（这样可以把物理连接的处理限制在UserSystem）
		由于已经加入了连接表，此时连接处于陌生人状态，循环读取时如果超过一定时间没有改变状态则删除
		tstring s = tformat(_T("The new linker(%s) coming in"),Linker->GetLabel().c_str());
		OutSysInfo(s);
		  
		int64 ID = Linker->GetSourceID();
		CSysDialog* ChildTask = new CSysDialog(this,ID,DIALOG_OTHER_MAIN);
			
		m_BrainData.AddDialog(ChildTask);
	*/
	
	CMsg Msg(MSG_WHO_ARE_YOU,DEFAULT_DIALOG,0);
	Linker->PushMsgToSend(Msg);
	StartAccept(ap);
}

void CUserSystem::ConnectHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp){
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

void CUserSystem::ConnectTimeoutHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp){
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
		cp->m_Timer.async_wait(boost::bind(&CUserSystem::ConnectTimeoutHandler,this,boost::asio::placeholders::error,cp));
		
	}else{
		cp->m_pSocket->close();
		cp->m_Timer.cancel();
		ePipeline Data;
		NotifySysState(MNOTIFY_CONNECT_FAIL,&Data);
		cp->m_TimeCount = 0;
	}
}
