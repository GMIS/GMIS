
#include "System.h"
#include "LinkerPipe.h"

namespace PHYSIC{
	

CSysThreadWorker::CSysThreadWorker(int64 ID,Model* Parent,int32 Type)
	:CThreadWorker(ID,Parent,Type)
{
	assert(Type == SYSTEM_NEVER_WORK_TYPE || Type == SYSTEM_IO_WORK_TYPE  );
}
CSysThreadWorker::~CSysThreadWorker(){

}
	
bool CSysThreadWorker::Do(Energy* E){
	if (m_WorkType == SYSTEM_NEVER_WORK_TYPE)
	{
		NerveWorkProc();
	}else if (m_WorkType == SYSTEM_IO_WORK_TYPE)
	{
		SystemIOWorkProc();	
	}
	return true;
}
	
void CSysThreadWorker::SystemIOWorkProc(){
	System* Parent = (System*)m_Parent;
	try{	
		    char buf[MODEL_IO_BUFFER_SIZE];
			while(IsAlive() && Parent->IsAlive()){

				int64 SourceID = 0;

				CLinker Linker;
				Parent->GetClientLinkerList()->PopLinker(Linker);

				if ( Linker.IsValid())
				{
					Linker().ThreadIOWorkProc(buf,MODEL_IO_BUFFER_SIZE);
					Parent->GetClientLinkerList()->ReturnLinker(Linker);
				}else{
					SLEEP_MILLI(1);
				}			
			}
		}
		/*
		catch(TimeoutException& e){
			AnsiString s = e.displayText();
			ePipeline Data;
			Data.PushString(s);
			m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
			//CLockedSystemData* LockedData = m_Parent->GetSystemData();
			//LockedData->DeleteIOWork(m_ID);
		}
		*/
		catch(...){
			ePipeline NotifyData;
			NotifyData.PushString(_T("An exception occurred, SystemIOWork Closed"));
			Parent->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);
		}

		ePipeline NotifyData;
		NotifyData.PushInt(m_ID);
		Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_IO_WORKER_CLOSED,&NotifyData);

}
	
void CSysThreadWorker::NerveWorkProc(){
	System* Parent = (System*)m_Parent;
	System::CLockedSystemData* LockedData = Parent->GetSystemData();

	try{
		while (IsAlive() && Parent->IsAlive())
		{

			CMsg Msg;
			Parent->PopNerveMsg(Msg);

			if (Msg.IsValid())
			{	
				LockedData->IncreNerveWorkerCount();		
				Parent->NerveMsgProc(Msg);
				LockedData->DecreNerveWorkerCount();
			}else{
				m_IdleCount++;
				if (m_IdleCount>LockedData->GetNerveMaxIdleCount()) //Around 600 milliseconds to exit if without information can be handled
				{						
					int IdleNum = LockedData->GetIdleWorkerNum();
					if (IdleNum > 1 )
					{
						m_IdleCount = 0;
						break;  //thread exit
					}else{  //Keep at least one free
						m_IdleCount = 0;
					}
				}
				SLEEP_MILLI(1);
			}

		}
	}catch(...){

		ePipeline NotifyData;
		NotifyData.PushString(_T("An exception occurred, CNerveWork Closed"));
		Parent->NotifySysState(NOTIFY_EXCEPTION_OCCURRED,NULL,&NotifyData);
	}

	int n = LockedData->GetNerveWorkerNum();
	ePipeline NotifyData;
	NotifyData.PushInt(--n);
	NotifyData.PushInt(m_ID);
	Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_CLOSED,&NotifyData);
}

CClientLinkerList::CClientLinkerList()
	:m_PoolMaxRestSize(20)
{

}
CClientLinkerList::~CClientLinkerList(){
	_CLOCK(&m_Mutex);
	deque<CLinkerPipe*>::iterator it = m_LinkerPool.begin();
	while (it != m_LinkerPool.end())
	{
		CLinkerPipe* LinkPtr = *it;
		assert(LinkPtr->GetUserNum()==0);
		delete LinkPtr;
		it++;
	}
}	

void CClientLinkerList::CreateLinker(CLinker& Linker,Model* Parent,int64 ID,ePipeline& Param){
	_CLOCK(&m_Mutex);
	deque<CLinkerPipe*>::iterator it = m_LinkerPool.begin();
	while (it != m_LinkerPool.end())
	{
		CUserLinkerPipe* UserPipe = (CUserLinkerPipe*)*it;
		if (UserPipe->GetID()==-1)
		{
			if (UserPipe->GetUserNum()!=0)
			{
				it++;
				continue;
			}else if (m_LinkerPool.size()>m_PoolMaxRestSize)
			{
				delete UserPipe;
				it++;
				continue;
			}
		}
		UserPipe->Reset();
		UserPipe->SetSourceID(ID);
		Linker.Reset(UserPipe);
		m_LinkerPool.erase(it);
	    return;
	}
	CUserLinkerPipe* UserPipe = new CUserLinkerPipe(&m_ClientSitMutex,(System*)Parent,ID);
	Linker.Reset(UserPipe);	
};

bool   CClientLinkerList::DeleteLinker(int64 SourceID){
	_CLOCK(&m_Mutex);

	CLinkerPipe* LinkPtr = NULL;
	list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
	while(it != m_LinkerList.end()){
		LinkPtr = *it;
		if(LinkPtr->GetSourceID() == SourceID){
			LinkPtr->Close();
			m_LinkerList.erase(it);
			m_LinkerPool.push_back(LinkPtr);
			return true;
		}
		it++;
	}
	
	it = m_ActivelyLinker.begin();
	while(it != m_ActivelyLinker.end()){
		LinkPtr = *it;
		if(LinkPtr->GetSourceID() == SourceID){
			LinkPtr->SetID(-1); //标记为删除
			LinkPtr->Close();
			return true;
		}
		it++;
	}
	return false;
}; 

void   CClientLinkerList::PopLinker(CLinker& Linker){
	_CLOCK(&m_Mutex);
	list<CLinkerPipe*>::iterator it = m_LinkerList.begin();
	if(it != m_LinkerList.end()){
		CLinkerPipe* LinkerPtr = *it;
		Linker.Reset(LinkerPtr);
		m_ActivelyLinker.push_front(LinkerPtr);
		m_LinkerList.erase(it);
	}
};
void   CClientLinkerList::ReturnLinker(CLinker& Linker){
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
				m_LinkerPool.push_back(LinkPtr);	
				return;
			}
			m_LinkerList.push_back(LinkPtr);
			return;
		}
		it++;
	}
}

System::CNetListenWorker::CNetListenWorker(System* Parent, int32 Port)
#if defined(USING_POCO)	
	:m_Parent(Parent),m_Socket(Port),m_Port(Port)
#elif defined(USING_WIN32)
	:m_Parent(Parent),m_Port(Port)
#endif
{

};
System::CNetListenWorker::~CNetListenWorker(){

};

bool System::CNetListenWorker::Activation(){

#if defined(USING_WIN32)
	sockaddr_in Local;

	Local.sin_family=AF_INET; //Address family
	Local.sin_addr.s_addr=INADDR_ANY; //Wild card IP address
	Local.sin_port=htons((u_short)m_Port); //port to use

	m_Socket =socket(AF_INET,SOCK_STREAM,0);


	if(m_Socket==INVALID_SOCKET){
		int ErrorCode = WSAGetLastError();
		error = Format1024(_T("Listen Fail: Internal socket error[code:%d]."),ErrorCode);

		return FALSE;
	}

	int Ret = bind(m_Socket,(sockaddr*)&Local,sizeof(Local));
	if(Ret !=0 ){
		error = _T("Listen Fail: Socket or port has been binded");
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	if(listen(m_Socket,10)!=0){	
		error = _T("Listen Fail: listen() fail");
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	unsigned long ul = 1; 
	Ret = ioctlsocket(m_Socket,FIONBIO,(unsigned long*)&ul);
	assert(Ret != SOCKET_ERROR);
#endif
	Object::Activate();
	return TRUE;
};
void System::CNetListenWorker::Dead(){
	m_Alive = false;
#if defined(USING_POCO)
	m_Socket.close();

#elif defined(USING_WIN32)
	if(IsValid()){
		long s = m_Socket; //Maybe the thread is looping and  checking m_Socket
		m_Socket =INVALID_SOCKET;
		closesocket(s);
	}
#endif	
	Object::Dead();
};


bool System::CNetListenWorker::Do(Energy* E){
	while (m_Alive&&m_Parent->IsAlive())
	{
#if defined(USING_POCO)
		try{

			Poco::Timespan timeout(50);
			bool ret = m_Socket.poll(timeout,Socket::SELECT_READ);
			if (!ret)
			{
				continue;
			};

			SocketAddress ClientAddress;
			StreamSocket ClientSocket = m_Socket.acceptConnection(ClientAddress);

			CLinker Linker;
			int64 SourceID = CreateTimeStamp();
			ePipeline Param;
			m_Parent->m_ClientLinkerList.CreateLinker(Linker,m_Parent,SourceID,Param);

			if(!Linker.IsValid()){
				AnsiString Add = ClientAddress.toString();
				tstring  AddW = UTF8toWS(Add);

				tstring s = Format1024(_T("can't create new CUserLinkerPipe "),AddW.c_str());

				ePipeline Data;
				Data.PushInt(m_Port);
				Data.PushString(s);		
				m_Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_LISTEN_FAIL,&Data);
				ClientSocket.close();
				m_Parent->GetSystemData()->DelAcceptor(m_Port,false);
				return true;
			}

			ClientSocket.setBlocking(false);

			CUserLinkerPipe* LinkerPtr = (CUserLinkerPipe*)Linker.Release();
			LinkerPtr->m_Socket = ClientSocket;

			m_Parent->m_ClientLinkerList.AddLinker(LinkerPtr);

			CMsg Msg(MSG_WHO_ARE_YOU,DEFAULT_DIALOG,0);
			LinkerPtr->PushMsgToSend(Msg);

			m_Parent->CreateClientLinkerWorkerStrategy(m_Port);

		}catch(Poco::Net::NetException& NetError)
		{
			tstring errorText  =  UTF8toWS(NetError.what());

			ePipeline Data;
			Data.PushInt(m_Port);
			Data.PushString(errorText);		
			m_Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_LISTEN_FAIL,&Data);
			m_Parent->GetSystemData()->DelAcceptor(m_Port,false);
			return true;
		}catch(Exception& exc){
			tstring errorText  =  UTF8toWS(exc.what());

			ePipeline Data;
			Data.PushInt(m_Port);
			Data.PushString(errorText);		
			m_Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_LISTEN_FAIL,&Data);
			m_Parent->GetSystemData()->DelAcceptor(m_Port,false);
			return true;
		}
#elif defined(USING_WIN32)
		SOCKET Client;
		sockaddr_in ClientAddress;
		int len=sizeof(ClientAddress);	

		Client=accept(m_Socket,(struct sockaddr*)&ClientAddress,&len);
		if(Client == INVALID_SOCKET){
			if(WSAEWOULDBLOCK == WSAGetLastError()){
				return true;				
			}
			else{
				int ErrorCode = WSAGetLastError();
				tstring s = Format1024(_T("failed code:%d"),ErrorCode);
				ePipeline Data;
				Data.PushInt(m_Port);
				Data.PushString(errorText);		
				m_Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_LISTEN_FAIL,&Data);
				
				return true;
			}
		}

		unsigned long ul = 1; 
		int Ret = ioctlsocket(Client,FIONBIO,(unsigned long*)&ul);
		assert(Ret != SOCKET_ERROR);

		//AnsiString ip = (const char*)inet_ntoa(ClientAddress.sin_addr);

		CUserSystem* System = (CUserSystem*)m_Parent;

		CUserLinkerPipe* Linker = System->CreateClientLinkerPipe();

		if(Linker== NULL){
			closesocket(Client);
			return true;
		}

		Linker->m_Socket = Client;

		System::CLockedLinkerList* ClientList= System->GetClientLinkerList();
		ClientList->AddLinker(Linker);

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


#endif
	}	

	tstring s= Format1024(_T("the port %d has been closed"),m_Port);
	
	ePipeline Data;
	Data.PushInt(m_Port);
	Data.PushString(s);		
	m_Parent->NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_LISTEN_FAIL,&Data);
	m_Parent->GetSystemData()->DelAcceptor(m_Port,false);
	return true;
}



	//CLockedSystemData
	//////////////////////////////////////////////////////////////////////////
	System::CLockedSystemData::CLockedSystemData()
	{
			m_MaxNerveWorkerNum = 100;
			m_NerveMsgMaxNumInPipe = 10;
			m_NerveMsgMaxInterval = 10*1000*1000; //1秒
			m_NerveIdleMaxCount   = 30;

			m_NerveWorkingNum = 0;

	};

	System::CLockedSystemData::~CLockedSystemData(){
		

	}

	void    System::CLockedSystemData::Clear(){
		_CLOCK(&m_Mutex);

		map<int64,CThreadWorker*>::iterator ita =  m_SystemIOWorkerList.begin();
		while (ita != m_SystemIOWorkerList.end())
		{
			CThreadWorker* Worker = ita->second;
			ita->second = NULL;
			assert(!Worker->IsAlive());
			delete Worker;
			ita++;
		}
		m_SystemIOWorkerList.clear();

		map<int64,CThreadWorker*>::iterator itb = m_NerveWorkerList.begin();
		while (itb != m_NerveWorkerList.end())
		{
			CThreadWorker* NerveWork = itb->second;
			itb->second = NULL;
			assert(!NerveWork->IsAlive());
			delete NerveWork;
			itb++;
		}
		m_NerveWorkerList.clear();

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


	void  System::CLockedSystemData::IncreNerveWorkerCount(){
		_CLOCK(&m_Mutex);
		++m_NerveWorkingNum ;
	}
	void  System::CLockedSystemData::DecreNerveWorkerCount(){
		_CLOCK(&m_Mutex);
		--m_NerveWorkingNum;
	}

	int64   System::CLockedSystemData::GetNerveMsgInterval(){
		_CLOCK(&m_Mutex);
		return m_NerveMsgMaxInterval;
	}
	void    System::CLockedSystemData::SetNerveMsgInterval(int32 n){
		_CLOCK(&m_Mutex);
		m_NerveMsgMaxInterval = n;
	}

	int32   System::CLockedSystemData::GetNerveMaxIdleCount(){
		_CLOCK(&m_Mutex);
		return m_NerveIdleMaxCount;	
	}

	void   System::CLockedSystemData::SetNerveMaxIdleCount(int32 n){
		_CLOCK(&m_Mutex);
		m_NerveIdleMaxCount = n;
	}

	int32   System::CLockedSystemData::GetBusyNerveWorkerNum(){
		_CLOCK(&m_Mutex);
		return m_NerveWorkingNum;		
	};

	int32  System::CLockedSystemData::GetNerveWorkerNum(){
		_CLOCK(&m_Mutex);
		int32 n = m_NerveWorkerList.size();
		return n;
	}
	int32   System::CLockedSystemData::GetIdleWorkerNum(){
		_CLOCK(&m_Mutex);
		return m_NerveWorkerList.size() - m_NerveWorkingNum;
	}

	int32 System::CLockedSystemData::GetIOWorkerNum(){
		_CLOCK(&m_Mutex);
		return m_SystemIOWorkerList.size();
	}

	CThreadWorker* System::CLockedSystemData::CreateThreadWorker(int64 ID,System* Parent,int32 Type){
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
			Worker = new CSysThreadWorker(ID,Parent,Type);
		}

		if(!Worker){
			return NULL;
		}

		if (Type == SYSTEM_NEVER_WORK_TYPE)
		{
			m_NerveWorkerList[ID]= Worker;
			return Worker;
		}else if (Type == SYSTEM_IO_WORK_TYPE)
		{
			m_SystemIOWorkerList[ID] = Worker;
			return Worker;
		}
		assert(0);
		return NULL;
	};
	void   System::CLockedSystemData::DeleteThreadWorker(System* Parent,int64 ID,int32 Type){
		

		assert(Type == SYSTEM_NEVER_WORK_TYPE || Type == SYSTEM_IO_WORK_TYPE);

		_CLOCK(&m_Mutex);
		CThreadWorker* Worker = NULL;
		if (Type == SYSTEM_NEVER_WORK_TYPE)
		{
			map<int64,CThreadWorker*>::iterator it = m_NerveWorkerList.find(ID);
			assert(it != m_NerveWorkerList.end());
			if (it != m_NerveWorkerList.end())
			{
				Worker = it->second;
				m_NerveWorkerList.erase(it);
				m_ThreadWorkerPool.push_back(Worker);

			}
		}else if (Type == SYSTEM_IO_WORK_TYPE)
		{
			map<int64,CThreadWorker*>::iterator it = m_SystemIOWorkerList.find(ID);
			assert(it != m_SystemIOWorkerList.end());
			if (it != m_SystemIOWorkerList.end())
			{
				Worker = it->second;
				m_SystemIOWorkerList.erase(it);
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


	void  System::CLockedSystemData::WaitAllWorkThreadClosed(System* Parent){
		assert(!Parent->IsAlive());
		if (Parent->IsAlive())
		{
			return;
		}
		/*
		//先打破Acceptor的阻塞
		m_Mutex.Acquire();
		map<int32,CNetListenWorker*>::iterator it1 =  m_AcceptorList.begin();
		while(it != m_AcceptorList.end()){
			it->second->CloseSocket();
			it++;
		}
		m_Mutex.Release();
		*/

		int n =1;
		while(n){
			_CLOCK(&m_Mutex);
			n = m_AcceptorList.size();
		}

		n =1;
		while(n){
			_CLOCK(&m_Mutex);
			n = m_SystemIOWorkerList.size();
		}
		
		n =1;
		while(n){
			_CLOCK(&m_Mutex);
			n = m_NerveWorkerList.size();
		}

		//被删除的worker只是理论上被放入pool中，这里确保线程完全退出
		deque<CThreadWorker*>::iterator it = m_ThreadWorkerPool.begin();
		while(it != m_ThreadWorkerPool.end())
		{
			CThreadWorker* Worker = *it;
			Worker->Dead();
			it++;
		}

		deque<CNetListenWorker*>::iterator ita = m_AcceptorPool.begin();
		while(ita != m_AcceptorPool.end())
		{
			CNetListenWorker* Worker = *ita;
			Worker->Dead();
			ita++;
		}

	}

	bool  System::CLockedSystemData::RequestCreateNewNerveWorker(uint32 MsgNum,int64 Interval,uint32& Reason) 
	{
		_CLOCK(&m_Mutex);

		if (m_NerveWorkerList.size() == m_MaxNerveWorkerNum)
		{
			Reason =  REASON_LIMIT;
			return FALSE;
		}else if (m_NerveWorkerList.size()==0)
		{
			Reason = REASON_ALWAYS;
			return TRUE;
		}	
		else if (m_NerveWorkingNum == m_NerveWorkerList.size()) //All be occupied or both = 0
		{			
			Reason = REASON_MSG_TOO_MUCH;
			return TRUE;
		}else if (MsgNum > m_NerveMsgMaxNumInPipe) ////when Current message number exceeded a specified number,start new thread
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
	System::CNetListenWorker* System::CLockedSystemData::CreateAcceptor(System* Parent,int32 Port){
		_CLOCK(&m_Mutex);
		deque<CNetListenWorker*>::iterator it = m_AcceptorPool.begin();
		if (it != m_AcceptorPool.end())
		{
			CNetListenWorker* Acceptor = *it;
			Acceptor->Dead(); 
			Acceptor->m_Parent = Parent;
			Acceptor->m_Port   = Port;
			m_AcceptorList[Port] = Acceptor;
			return Acceptor;
		} 		
		CNetListenWorker* Acceptor = new CNetListenWorker(Parent,Port);
		if (!Acceptor)
		{
			return NULL;
		}

		m_AcceptorList[Port] = Acceptor;
		return Acceptor;
	};

	bool System::CLockedSystemData::HasAcceptor(int32 Port){
		_CLOCK(&m_Mutex);
		map<int32,CNetListenWorker*>::iterator it = m_AcceptorList.find(Port);
		while (it != m_AcceptorList.end())
		{
			if (it->first == Port)
			{
				return false;
			}
			it++;
		}
		return false;
	}
	void System::CLockedSystemData::DelAcceptor(int32 Port,bool bWaitDead){
		_CLOCK(&m_Mutex);
		map<int32,CNetListenWorker*>::iterator it = m_AcceptorList.find(Port);
		if (it == m_AcceptorList.end())
		{
			return;
		} 
		CNetListenWorker* Acceptor = it->second;
		if(bWaitDead){
			Acceptor->Dead();
		}
		
		m_AcceptorPool.push_back(Acceptor);
		m_AcceptorList.erase(it);
		while (m_AcceptorPool.size()>10)
		{
			Acceptor = m_AcceptorPool.front();
			Acceptor->Dead();
			delete Acceptor;
			m_AcceptorPool.pop_front();
		}
	};

	void System::CLockedSystemData::DelAllAcceptor(){
		_CLOCK(&m_Mutex);
		map<int32,CNetListenWorker*>::iterator it = m_AcceptorList.begin();
		while (it != m_AcceptorList.end())
		{
			CNetListenWorker* Acceptor = it->second;
			Acceptor->Dead();
			if (m_AcceptorPool.size()<10)
			{
				m_AcceptorPool.push_back(Acceptor);
			}
			it++;
		} 
		m_AcceptorList.clear();
	};
	//


	//SYSTEM
	//////////////////////////////////////////////////////////////////////////
	System::System(CUserTimer* Timer,CUserSpacePool* Pool)
		:Model(Timer,Pool),
		 m_NerveMutex(),
		 m_Nerve(&m_NerveMutex,_T("Nerve"),2)
	{

#if defined(USING_POCO)
		AnsiString ThreadName = "SysMainThread";
		m_ObjectDefaultThread.setName(ThreadName);
#endif
	}

	System::~System(){

	};

	bool System::Activate(){
		if(m_Alive){
			return TRUE;
		}

		if(!Model::Activate()){
			m_Alive = FALSE;
			return FALSE;
		}
		
		m_Alive= TRUE;
		return TRUE;
	}

	void System::Dead()
	{
		m_Alive= FALSE;
		m_SystemData.WaitAllWorkThreadClosed(this);
		m_SystemData.DelAllAcceptor();
		m_SystemData.Clear();
		Model::Dead();
	};


	CClientLinkerList*    System::GetClientLinkerList(){
		return &m_ClientLinkerList;
	};
	System::CLockedSystemData*    System::GetSystemData(){
		return &m_SystemData;
	};
/*
	void System::GetLinker(int64 ID,CLinker& Linker){
		CClientLinkerList*  ClientList = GetClientLinkerList();
		ClientList->GetLinker(ID,Linker);
		if (!Linker.IsValid())
		{
			CSuperiorLinkerList*  SuperiorList = GetSuperiorLinkerList();
			SuperiorList->GetLinker(ID,Linker);
		}
	}
*/
	int32	System::GetNerveMsgNum(){
		return m_Nerve.DataNum();

	};

	void	System::GetNerveMsgList(ePipeline& Pipe){
		m_Nerve.CopyTo(Pipe);
	};


	void	System::PopNerveMsg(CMsg& Msg){
		m_Nerve.Pop(Msg);

	};

	void  System::PushNerveMsg(CMsg& Msg,bool bUrgenceMsg,bool bDirectly){

		int64 MsgID = Msg.GetMsgID();
		int64 EventID = Msg.GetEventID();

		int64 LastMsgPopTime = 0;
		if(bUrgenceMsg){
			LastMsgPopTime = m_Nerve.PushUrgence(Msg.Release());
		}else {
			LastMsgPopTime = m_Nerve.Push(Msg.Release());
		}

		if(bDirectly)return;

		int64 NewMsgPushTime =  CreateTimeStamp();

		assert(NewMsgPushTime!=0);
		assert(NewMsgPushTime!=LastMsgPopTime);

		BOOL ret = CreateNerveWorkerStrategy(NewMsgPushTime,LastMsgPopTime);

		if (m_LogFlag & LOG_MSG_NERVE_PUSH)
		{
			tstring MsgStr = MsgID2Str(MsgID);
			if (MsgStr != _T("MSG_EVENT_TICK"))
			{
				tstring s = Format1024(_T("Nerve push msg: %s EventID:%I64ld MsgNum:%d,CreateNewThread:%d"),MsgStr.c_str(),EventID,m_Nerve.DataNum(),ret);
				OutputLog(LOG_MSG_NERVE_PUSH,s.c_str());
			}
		}	
	}


	BOOL  System::CreateNerveWorkerStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){

		int32 n = GetNerveMsgNum();

		ePipeline Data;
		Data.PushInt(n);
		NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_MSG_NUM,&Data);

		int64 t = NewMsgPushTime-LastMsgPopTime;

		//If more than 10 messages in the central nerve or there is message waiting for being handled more than 2 seconds, create a new thread
		uint32 Reason ;
		bool ret = m_SystemData.RequestCreateNewNerveWorker(n,t,Reason);
		if (!ret)
		{
			if (Reason == REASON_LIMIT)
			{
				
				NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_LIMIT,NULL);
			}
			return FALSE;
		}

		
		CThreadWorker* NerveWork = m_SystemData.CreateThreadWorker(NewMsgPushTime,this,SYSTEM_NEVER_WORK_TYPE);
		if (!NerveWork){
			return FALSE;
		}
			
		if(NerveWork->Activate())
		{
			int n = m_SystemData.GetNerveWorkerNum();
			ePipeline Data;
			Data.PushInt(n);
			Data.PushInt(NerveWork->m_ID);
			NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_JOIN,&Data);
			return TRUE;
		}else{
			m_SystemData.DeleteThreadWorker(this,NerveWork->m_ID,SYSTEM_NEVER_WORK_TYPE);
			NotifySysState(NOTIFY_SYSTEM_SCENE,NTID_NERVE_THREAD_FAIL,NULL);
		}
		return FALSE;
	}
	/*
	void System::BroadcastMsg(set<int64>& SourceList,int64 BCS_ID,ePipeline& MsgData){
		MsgData.SetID(BCS_ID);

		set<int64>::iterator it = SourceList.begin();
		while(it != SourceList.end()){
			CMsg Msg(MSG_BROADCAST_MSG,DEFAULT_DIALOG,0);
			Msg.GetLetter().Push_Directly(MsgData.Clone());

			int64 SourceID = *it;
			CLinker Linker;
			m_ClientLinkerList.GetLinker(SourceID,Linker);
			if (Linker.IsValid())
			{
				Linker().PushMsgToSend(Msg);
			}
			it++;
		}

	};
	*/
	void System::NotifySysState(int64 NotifyType,int64 NotifyID,ePipeline* Data){
		switch (NotifyType)
		{
		case NOTIFY_SYSTEM_SCENE:
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

						CLockedSystemData*  LockedData = GetSystemData();
						LockedData->DeleteThreadWorker(this,nClosedThreadID,SYSTEM_NEVER_WORK_TYPE);

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
						CLockedSystemData* SystemData = GetSystemData();
						SystemData->DeleteThreadWorker(this,ID,SYSTEM_IO_WORK_TYPE);
					}
					break;
				case NOTIFY_EXCEPTION_OCCURRED:
					{
						tstring ErrorInfo = Data->PopString();
						tstring info = Format1024(_T("Exception occurred: %s "),ErrorInfo.c_str());
						OutputLog(LOG_EXCEPTION,info.c_str());
					}
					break;
				case NTID_LISTEN_FAIL:
					{
						int64 Port = Data->PopInt();
						tstring s=Data->PopString();
						tstring info = Format1024(_T("Listen fail: %s  port=%d"),s.c_str(),(int32)Port);
						OutputLog(LOG_ERROR,info.c_str());
					}
				default:
					{

					}
				}
			}
			break;
		case NOTIFY_MODEL_SCENE:
			Model::NotifySysState(NotifyType,NotifyID,Data);
			break;
		}
	}
	void System::CreateClientLinkerWorkerStrategy(int32 Port){
		int32 WorkerNum = m_SystemData.GetIOWorkerNum();

		if (WorkerNum==0 || (m_ClientLinkerList.GetLinkerNum()>10 && WorkerNum< GetCpuNum()))
		{	
			int64 ID = CreateTimeStamp();
			CThreadWorker* IOWork = m_SystemData.CreateThreadWorker(ID,this,SYSTEM_IO_WORK_TYPE);
			if(!IOWork || !IOWork->Activate()){
				if(IOWork)m_SystemData.DeleteThreadWorker(this,ID,SYSTEM_IO_WORK_TYPE);

				tstring s= Format1024(_T("Create new IO worker fail, listen port: %d "),Port);
				OutputLog(LOG_WARNING,s.c_str());
			}
		}
	};

	bool System::OpenPort(int32 Port,tstring& error,bool bIP6){
		int64 ID = CreateTimeStamp();

		if(m_SystemData.HasAcceptor(Port)){
			error = Format1024(_T("the port %d has been opened "),Port);
			return false;	
		};

		CNetListenWorker* Acceptor = m_SystemData.CreateAcceptor(this,Port);

		if(!Acceptor){
			error = _T("Create acceptor fail");
			return FALSE;
		}

		if(!Acceptor->Activation()){
			error = _T("Activate acceptor fail");
			m_SystemData.DelAcceptor(Port,false);
			return FALSE;	
		};

		return TRUE;
	};

	void System::ClosePort(int32 Port){
		m_SystemData.DelAcceptor(Port,true);
	}

}; //end namespace ABSTRACT
