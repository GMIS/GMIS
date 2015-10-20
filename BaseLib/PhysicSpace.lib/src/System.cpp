
#include "System.h"
#include "LinkerPipe.h"

namespace PHYSIC{
	
System::CNetListenWork::CNetListenWork(System* Parent, int32 Port)
#if defined(USING_POCO)	
	:m_Parent(Parent),m_Socket(Port),m_Port(Port)
#elif defined(USING_WIN32)
	:m_Parent(Parent),m_Port(Port)
#endif
{

};
System::CNetListenWork::~CNetListenWork(){

};

bool System::CNetListenWork::Activation(){

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
	Object::Activation();
	return TRUE;
};

void System::CNetListenWork::Dead(){
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

bool System::CNetListenWork::Do(Energy* E){
	while (m_Alive)
	{
#if defined(USING_POCO)

		try{

			SocketAddress ClientAddress;
			StreamSocket ClientSocket = m_Socket.acceptConnection(ClientAddress);

			System* System = m_Parent;

			CUserLinkerPipe* Linker = System->CreateClientLinkerPipe();

			if(Linker== NULL){
				AnsiString Add = ClientAddress.toString();
				tstring  AddW = UTF8toWS(Add);

				tstring s = Format1024(_T("Accept %s Fail: can't create new CUserLinkerPipe "),AddW.c_str());

				CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
				Msg.GetLetter().PushInt(m_Port);
				Msg.GetLetter().PushString(s);		
				m_Parent->PushCentralNerveMsg(Msg,false,false);
				ClientSocket.close();
				return true;
			}

			ClientSocket.setBlocking(false);
			Linker->m_Socket = ClientSocket;

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


		}catch(Poco::Net::NetException& NetError)
		{
			tstring errorText  =  UTF8toWS(NetError.what());

			tstring s = Format1024(_T("Listen Fail: %s"),errorText.c_str());

			CMsg Msg(MSG_LINKER_ERROR,DEFAULT_DIALOG,0);
			Msg.GetLetter().PushInt(m_Port);
			Msg.GetLetter().PushString(s);		
			m_Parent->PushCentralNerveMsg(Msg,false,true);
			return true;
		}catch(Poco::Exception& e)
		{
			AnsiString s = e.what();
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
				tstring s = Format1024(_T("Listen Fail: Accept() failed [code:%d]"),ErrorCode);
				CMsg Msg(m_SourceID,DEFAULT_DIALOG,MSG_LINKER_ERROR,DEFAULT_DIALOG);
				Msg.GetLetter().PushInt(m_Port);
				Msg.GetLetter().PushString(s);		
				m_Parent->PushCentralNerveMsg(Msg);
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
	return true;
}


	//SystemIOWork
	//////////////////////////////////////////////////////////////////////////
	System::CSystemIOWork::CSystemIOWork(int64 ID,System* Parent)
		:m_Parent(Parent)
	{
		assert(m_Parent);
		m_ID = ID;		
#if defined(USING_POCO)
		AnsiString ThreadName = "SystemIOThread";
		m_ObjectDefaultThread.setName(ThreadName);
#endif
	};
	System::CSystemIOWork::~CSystemIOWork(){

	}

	bool System::CSystemIOWork::Do(Energy* E){	
		try{
			while(m_Alive && m_Parent->IsAlive()){

				int64 SourceID = 0;

				Model::CLockedLinkerList* LinkerList = m_Parent->GetClientLinkerList();
				CLinker Linker;
				LinkerList->GetNextAvailableLinker(SourceID,Linker);

				char buf[MODEL_IO_BUFFER_SIZE];
				while (m_Alive && Linker.IsValid())
				{
					SourceID = Linker().GetSourceID();				
					Linker().ThreadIOWorkProc(buf,MODEL_IO_BUFFER_SIZE);
					LinkerList->GetNextAvailableLinker(SourceID,Linker);
				}
				SLEEP_MILLI(20);
			}


			//这里不能直接调用NotifySystemState(),可能会引起mutex锁死
			CMsg Msg(MSG_SYS_MSG,DEFAULT_DIALOG,0);
			ePipeline& Letter = Msg.GetLetter();
			Letter.PushInt(NOTIFY_SYS_STATE);
			Letter.PushInt(SNOTIFY_IO_WORK_THREAD_CLOSE);
			ePipeline NotifyData;
			NotifyData.PushInt(m_ID);
			Letter.PushPipe(NotifyData);
			m_Parent->PushCentralNerveMsg(Msg,false,true);

			
			m_Alive = FALSE;
			return TRUE;
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

			CMsg Msg2(MSG_SYS_MSG,DEFAULT_DIALOG,0);
			ePipeline& Letter2 = Msg2.GetLetter();
			Letter2.PushInt(NOTIFY_SYS_STATE);
			Letter2.PushInt(MNOTIFY_EXCEPTION_OCCURRED);
			ePipeline NotifyData2;
			NotifyData2.PushString(_T("An exception occurred, SystemIOWork Closed"));
			m_Parent->PushCentralNerveMsg(Msg2,false,true);
		}

		CMsg Msg1(MSG_SYS_MSG,DEFAULT_DIALOG,0);
		ePipeline& Letter1 = Msg1.GetLetter();
		Letter1.PushInt(NOTIFY_SYS_STATE);
		Letter1.PushInt(SNOTIFY_IO_WORK_THREAD_CLOSE);
		ePipeline NotifyData1;
		NotifyData1.PushInt(m_ID);
		Letter1.PushPipe(NotifyData1);
		
		m_Parent->PushCentralNerveMsg(Msg1,false,true);

		m_Alive = FALSE;
		return TRUE;
	}			

	//Nerve
	//////////////////////////////////////////////////////////////////////////
	System::CNerveWork::CNerveWork(int64 ID,System* Parent)
		:m_Parent(Parent),m_IdleCount(0)
	{
		m_ID = ID;
#if defined(USING_POCO)
		AnsiString ThreadName = "NerveWorkThread";
		m_ObjectDefaultThread.setName(ThreadName);
#endif
	};
	System::CNerveWork::~CNerveWork(){

	};


	bool System::CNerveWork::Do(Energy* E){
		int n = 0;
		try{
			while (m_Alive && m_Parent->IsAlive())
			{
				CMsg Msg;
				m_Parent->PopNerveMsg(Msg);
				CLockedSystemData* LockedData = m_Parent->GetSystemData();

				if (Msg.IsValid())
				{		

					LockedData->IncreNerveWorkCount();
					m_Parent->NerveProc(Msg);

					LockedData->DecreNerveWorkCount();
				}else{
					m_IdleCount++;
					if (m_IdleCount>LockedData->GetNerveMaxIdleCount()) //Around 600 milliseconds to exit if without information can be handled
					{
						n = LockedData->GetNerveWorkNum();
						int n1 = LockedData->GetBusyNerveWorkNum();

						if (n-n1 == 1 )//Keep at least one free
						{
							m_IdleCount = 0;
						}else{
							//这里不能直接调用NotifySystemState(),可能会引起mutex锁死
							CMsg Msg(MSG_SYS_MSG,DEFAULT_DIALOG,0);
							ePipeline& Letter = Msg.GetLetter();
							Letter.PushInt(NOTIFY_SYS_STATE);
							Letter.PushInt(SNOTIFY_NERVE_THREAD_CLOSE);
							ePipeline NotifyData;
							NotifyData.PushInt(--n);
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
		}catch(...){

			CMsg Msg2(MSG_SYS_MSG,DEFAULT_DIALOG,0);
			ePipeline& Letter2 = Msg2.GetLetter();
			Letter2.PushInt(NOTIFY_SYS_STATE);
			Letter2.PushInt(MNOTIFY_EXCEPTION_OCCURRED);
			ePipeline NotifyData2;
			NotifyData2.PushString(_T("An exception occurred, CNerveWork Closed"));
			m_Parent->PushCentralNerveMsg(Msg2,false,true);
		}
		CMsg Msg1(MSG_SYS_MSG,DEFAULT_DIALOG,0);
		ePipeline& Letter1 = Msg1.GetLetter();
		Letter1.PushInt(NOTIFY_SYS_STATE);
		Letter1.PushInt(SNOTIFY_NERVE_THREAD_CLOSE);
		ePipeline NotifyData1;
		NotifyData1.PushInt(--n);
		NotifyData1.PushInt(m_ID);
		Letter1.PushPipe(NotifyData1);
		m_Parent->PushCentralNerveMsg(Msg1,false,true);

		m_Alive = FALSE;
		return TRUE;
	};

	//CLockedSystemData
	//////////////////////////////////////////////////////////////////////////
	System::CLockedSystemData::CLockedSystemData()
	{
			m_MaxNerveWorkerNum = 100;
			m_NerveMsgMaxNumInPipe = 10;
			m_NerveMsgMaxInterval = 10*1000*1000; //1秒
			m_NerveIdleMaxCount   = 30;

			m_NerveWorkingNum = 0;
			m_bClosed = false;
	};

	System::CLockedSystemData::~CLockedSystemData(){
		
		CLock lk(&m_Mutex);

		map<int64,CSystemIOWork*>::iterator ita =  m_SystemIOWorkList.begin();
		while (ita != m_SystemIOWorkList.end())
		{
			CSystemIOWork* SystemIOWork = ita->second;
			assert(!SystemIOWork->IsAlive());
			delete SystemIOWork;
			ita++;
		}


		map<int64,CNerveWork*>::iterator itd = m_NerveWorkList.begin();
		while (itd != m_NerveWorkList.end())
		{
			CNerveWork* NerveWork = itd->second;
			assert(!NerveWork->IsAlive());
			delete NerveWork;
			itd++;
		}
	}



	void  System::CLockedSystemData::IncreNerveWorkCount(){
		CLock lk(&m_Mutex);
		++m_NerveWorkingNum ;
	}
	void  System::CLockedSystemData::DecreNerveWorkCount(){
		CLock lk(&m_Mutex);
		--m_NerveWorkingNum;
	}

	int64   System::CLockedSystemData::GetNerveMsgInterval(){
		CLock lk(&m_Mutex);
		return m_NerveMsgMaxInterval;
	}
	void    System::CLockedSystemData::SetNerveMsgInterval(int32 n){
		CLock lk(&m_Mutex);
		m_NerveMsgMaxInterval = n;
	}

	int32   System::CLockedSystemData::GetNerveMaxIdleCount(){
		CLock lk(&m_Mutex);
		return m_NerveIdleMaxCount;	
	}

	void   System::CLockedSystemData::SetNerveMaxIdleCount(int32 n){
		CLock lk(&m_Mutex);
		m_NerveIdleMaxCount = n;
	}

	int32   System::CLockedSystemData::GetBusyNerveWorkNum(){
		CLock lk(&m_Mutex);
		return m_NerveWorkingNum;		
	};

	int32  System::CLockedSystemData::GetNerveWorkNum(){
		CLock lk(&m_Mutex);
		int32 n = m_NerveWorkList.size();
		return n;
	}

	int32 System::CLockedSystemData::GetIOWorkNum(){
		CLock lk(&m_Mutex);
		return m_SystemIOWorkList.size();
	}

	int32    System::CLockedSystemData::AddIOWork(CSystemIOWork* Work){
		CLock lk(&m_Mutex);
		if (m_bClosed)
		{
			Work->Dead();
			delete Work;
			return 0;
		}
		assert(m_SystemIOWorkList.find(Work->m_ID) == m_SystemIOWorkList.end());
		m_SystemIOWorkList[Work->m_ID] = Work;
		return m_SystemIOWorkList.size();
	}

	int32    System::CLockedSystemData::DeleteIOWork(int64 ID){
		CLock lk(&m_Mutex);
		if (m_bClosed)
		{
			return 0;
		}
		map<int64,CSystemIOWork*>::iterator it = m_SystemIOWorkList.find(ID);
		if (it != m_SystemIOWorkList.end())
		{
			CSystemIOWork* Work = it->second;
			m_SystemIOWorkList.erase(it);
			Work->Dead();
			delete Work;
		}
		return m_SystemIOWorkList.size();
	}

	int32    System::CLockedSystemData::AddNerveWork(CNerveWork* Work){
		CLock lk(&m_Mutex);
		if (m_bClosed)
		{
			Work->Dead();
			delete Work;
			return 0;
		}
		assert(m_NerveWorkList.find(Work->m_ID) == m_NerveWorkList.end());
		m_NerveWorkList[Work->m_ID] = Work;
		return m_NerveWorkList.size();
	}

	int32    System::CLockedSystemData::DeleteNerveWork(int64 ID){
		CLock lk(&m_Mutex);
		if(m_bClosed){
			return 0;
		}
		map<int64,CNerveWork*>::iterator it = m_NerveWorkList.find(ID);
		if (it != m_NerveWorkList.end())
		{
			CNerveWork* Work = it->second;
			m_NerveWorkList.erase(it);
			Work->Dead();
			delete Work;
		}
		return m_NerveWorkList.size();
	}
	void  System::CLockedSystemData::CloseAllWorkThread(){
		m_Mutex.Acquire();
		m_bClosed = true;	
		m_Mutex.Release();

		DelAllAcceptor();

		map<int64,CSystemIOWork*>::iterator ita =  m_SystemIOWorkList.begin();
		while(ita != m_SystemIOWorkList.end())
		{
			CSystemIOWork* Work = ita->second;
			Work->Dead();
			ita++;
		}

		map<int64,CNerveWork*>::iterator itb =  m_NerveWorkList.begin();
		while(itb != m_NerveWorkList.end())
		{
			CNerveWork* Work = itb->second;
			Work->Dead();
			itb++;
		}

	}
	bool  System::CLockedSystemData::RequestCreateNewNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason) 
	{
		CLock lk(&m_Mutex);

		if (m_NerveWorkList.size() == m_MaxNerveWorkerNum)
		{
			Reason =  REASON_LIMIT;
			return FALSE;
		}
		else if (m_NerveWorkingNum == m_NerveWorkList.size()) //All be occupied
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
		}else {
			Reason = REASON_REFUSE;
		}
		return FALSE;
	}
	void System::CLockedSystemData::AddAcceptor(int32 Port,CNetListenWork* Acceptor){
		CLock lk(&m_Mutex);

		if(m_bClosed){
			Acceptor->Dead();
			delete Acceptor;
			return;
		}
		map<int32,CNetListenWork*>::iterator it = m_AcceptorList.find(Port);
		if (it != m_AcceptorList.end())
		{
			return;
		}

		m_AcceptorList[Port] = Acceptor;
	};

	void System::CLockedSystemData::DelAcceptor(int32 Port){
		CLock lk(&m_Mutex);

		if(m_bClosed){
			return;
		}
		map<int32,CNetListenWork*>::iterator it = m_AcceptorList.find(Port);
		if (it != m_AcceptorList.end())
		{
			return;
		} 
		CNetListenWork* Acceptor = it->second;
		Acceptor->Dead(); 
		m_AcceptorList.erase(it);
	};

	void System::CLockedSystemData::DelAllAcceptor(){
		CLock lk(&m_Mutex);
		map<int32,CNetListenWork*>::iterator it = m_AcceptorList.begin();
		if (it != m_AcceptorList.end())
		{
			CNetListenWork* Acceptor = it->second;
			Acceptor->Dead(); 
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

	bool System::Activation(){
		if(m_Alive){
			return TRUE;
		}

		m_Alive= TRUE;

		if(!Model::Activation()){
			m_Alive = FALSE;
			return FALSE;
		}


		//缺省生成一个IO线程
		int64 ID = CreateTimeStamp();
		CSystemIOWork* IOWork = new CSystemIOWork(ID,this);
		if(!IOWork || !IOWork->Activation()){
			return FALSE;
		}
		CLockedSystemData* SystemData = (CLockedSystemData*)GetSystemData();
		SystemData->AddIOWork(IOWork);

		return TRUE;
	}

	void System::Dead()
	{
		m_Alive= FALSE;
		m_SystemData.CloseAllWorkThread();
		
		Model::Dead();
	};

	CSpaceMutex*          System::GetSpaceMutex(){
		return &m_ClientSitMutex;
	}

	Model::CLockedLinkerList*    System::GetClientLinkerList(){
		return &m_ClientList;
	};
	System::CLockedSystemData*    System::GetSystemData(){
		return &m_SystemData;
	};

	void System::GetLinker(int64 ID,CLinker& Linker){
		CLockedLinkerList*  ClientList = GetClientLinkerList();
		ClientList->GetLinker(ID,Linker);
		if (!Linker.IsValid())
		{
			CLockedLinkerList*  SuperiorList = GetSuperiorLinkerList();
			SuperiorList->GetLinker(ID,Linker);
		}
	}

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

		BOOL ret = NerveWorkStrategy(NewMsgPushTime,LastMsgPopTime);

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


	BOOL  System::NerveWorkStrategy(int64 NewMsgPushTime,int64 LastMsgPopTime){

		int32 n = GetNerveMsgNum();

		ePipeline Data;
		Data.PushInt(n);
		NotifySysState(SNOTIFY_NERVE_MSG_NUM,&Data);

		if(LastMsgPopTime==0){	//First ID is 0		
			CNerveWork* NerveWork = CreateNerveWorker(0,this,REASEON_ALWAYS);
			if (!NerveWork){
				assert(0);
				OutputLog(LOG_TIP,_T("Create first Nerver thread fail,Please reboot it"));
				return FALSE;
			}
			
			if(NerveWork->Activation())
			{
				int n = m_SystemData.AddNerveWork(NerveWork);
				ePipeline Data;
				Data.PushInt(n);
				Data.PushInt(NerveWork->m_ID);
				NotifySysState(SNOTIFY_NERVE_THREAD_JOIN,&Data);
				return TRUE;
			}else{
				if (NerveWork)
				{
					delete NerveWork;
				}
				assert(0);
				OutputLog(LOG_TIP,_T("Create first Nerver thread fail,Please reboot it"));

				NotifySysState(SNOTIFY_NERVE_THREAD_FAIL,NULL);
			}
			return FALSE;
		}

		int64 t = NewMsgPushTime-LastMsgPopTime;

		//If more than 10 messages in the central nerve or there is message waiting for being handled more than 2 seconds, create a new thread
		uint32 Reason ;
		bool ret = m_SystemData.RequestCreateNewNerveWork(n,t,Reason);
		if (!ret)
		{
			if (Reason == REASON_LIMIT)
			{
				
				NotifySysState(SNOTIFY_NERVE_THREAD_LIMIT,NULL);
			}
			return FALSE;
		}

		
		CNerveWork* NerveWork = CreateNerveWorker(NewMsgPushTime,this,Reason);
		if (!NerveWork){
			return FALSE;
		}
			
		if(NerveWork->Activation())
		{
			int n =m_SystemData.AddNerveWork(NerveWork);
			ePipeline Data;
			Data.PushInt(n);
			Data.PushInt(NerveWork->m_ID);
			NotifySysState(SNOTIFY_NERVE_THREAD_JOIN,&Data);
			return TRUE;
		}else{
			if(NerveWork){
				delete NerveWork;
			}
			assert(0);
			OutputLog(LOG_TIP,_T("Create Nerver thread fail,Please reboot it"));
			NotifySysState(SNOTIFY_NERVE_THREAD_FAIL,NULL);
		}
		return FALSE;
	}

	void System::BroadcastMsg(set<int64>& SourceList,int64 BCS_ID,ePipeline& MsgData){
		MsgData.SetID(BCS_ID);

		set<int64>::iterator it = SourceList.begin();
		while(it != SourceList.end()){
			CMsg Msg(MSG_BROADCAST_MSG,DEFAULT_DIALOG,0);
			Msg.GetLetter().Push_Directly(MsgData.Clone());

			int64 SourceID = *it;
			CLinker Linker;
			GetLinker(SourceID,Linker);
			if (Linker.IsValid())
			{
				Linker().PushMsgToSend(Msg);
			}
			it++;
		}

	};

	System::CNerveWork* System::CreateNerveWorker(int64 ID,System* Parent,uint32 Reason){
		CNerveWork* NerveWork = new CNerveWork(ID,this);
		return NerveWork;	
	}

	CUserLinkerPipe* System::CreateClientLinkerPipe(){
		int64 ID = CreateTimeStamp();
		CUserLinkerPipe* Linker = new CUserLinkerPipe(&m_ClientSitMutex,this,ID);
		return Linker;
	}

	void  System::NotifySysState(int64 NotifyID,ePipeline* Data /*= NULL*/){
		switch(NotifyID){

		case MNOTIFY_IO_WORK_THREAD_CLOSE:
		{
			int64 ID = Data->PopInt();
			CLockedModelData* ModelData = GetModelData();
			ModelData->DeleteIOWork(ID);
		}
		break;
		case SNOTIFY_IO_WORK_THREAD_CLOSE:
		{
			int64 ID = Data->PopInt();
			CLockedSystemData* SysDate = GetSystemData();
			SysDate->DeleteIOWork(ID);
		}
		case MNOTIFY_EXCEPTION_OCCURRED:
		{
			tstring ErrorInfo = Data->PopString();
			OutputLog(LOG_TIP,ErrorInfo.c_str());
		}
		break;
		}
	}

	bool System::Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock){
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

		//如果没有处理线程则生成一个
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


	bool System::OpenPort(int32 Port,tstring& error,bool bIP6){
		int64 ID = CreateTimeStamp();

		CNetListenWork* Acceptor = new CNetListenWork(this,Port);

		if(!Acceptor){
			error = _T("Create acceptor fail");
			return FALSE;
		}

		if(!Acceptor->Activation()){
			error = _T("Activate acceptor fail");
			delete Acceptor;
			return FALSE;	
		};

		CLockedSystemData* SystemData = (CLockedSystemData*)GetSystemData();
		SystemData->AddAcceptor(Port,Acceptor);	

		return TRUE;
	};

	void System::ClosePort(int32 Port){
		CLockedSystemData* SystemData = (CLockedSystemData*)GetSystemData();
		SystemData->DelAcceptor(Port);
	}

}; //end namespace ABSTRACT
