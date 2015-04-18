
#include "System.h"
#include "LinkerPipe.h"

namespace ABSTRACT{

	//SystemIOWork
	//////////////////////////////////////////////////////////////////////////
	System::CSystemIOWork::CSystemIOWork(int64 ID,System* Parent)
		:m_Parent(Parent)
	{
		assert(m_Parent);
		m_ID = ID;		
	};
	System::CSystemIOWork::~CSystemIOWork(){

	}

	bool System::CSystemIOWork::Do(Energy* E){	
		try{
			while(m_Alive){

				int64 SourceID = 0;

				Model::CLockedLinkerList* LinkerList = m_Parent->GetClientLinkerList();
				CLinker Linker;
				LinkerList->GetNextLinker(SourceID,Linker);

				char buf[BUFFER_SIZE];
				while (m_Alive && Linker.IsValid())
				{
					SourceID = Linker().GetSourceID();

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
				assert(0); //This situation is very unlikely
				ePipeline Data;
				Data.PushInt(m_ID);
				m_Parent->NotifySysState(SNOTIFY_IO_WORK_THREAD_CLOSE,&Data);
				CLockedSystemData* LockedData = m_Parent->GetSystemData();
				LockedData->DeleteIOWork(m_ID);
			}
			m_Alive = FALSE;
			return TRUE;
		}catch(...){
			assert(0);
			ePipeline Data;
			Data.PushString(_T("An exception occurred, SystemIOWork Close"));
			m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);
			CLockedSystemData* LockedData = m_Parent->GetSystemData();
			LockedData->DeleteIOWork(m_ID);
		}
		m_Alive = FALSE;
		return TRUE;
	}			

	//Nerve
	//////////////////////////////////////////////////////////////////////////
	System::CNerveWork::CNerveWork(int64 ID,System* Parent)
		:m_Parent(Parent),m_IdleCount(0)
	{
		m_ID = ID;
	};
	System::CNerveWork::~CNerveWork(){

	};


	bool System::CNerveWork::Do(Energy* E){

#ifdef NDEBUG
		try{
#endif
			while (m_Alive)
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
						int n = LockedData->GetNerveWorkNum();
						int n1 = LockedData->GetBusyNerveWorkNum();

						if (n-n1 == 1 )//Keep at least one free
						{
							m_IdleCount = 0;
						}else{
							ePipeline Data;
							Data.PushInt(--n);
							Data.PushInt(m_ID);
							m_Parent->NotifySysState(SNOTIFY_NERVE_THREAD_CLOSE,&Data);
							LockedData->DeleteNerveWork(m_ID);
							m_Alive = FALSE;
							return TRUE;
						}
					}
					SLEEP_MILLI(20);
				}
			}
#ifdef NDEBUG
		}catch(...){
			assert(0);

			ePipeline Data;
			Data.PushString(_T("An exception occurred, CNerveWork Close"));
			m_Parent->NotifySysState(MNOTIFY_EXCEPTION_OCCURRED,&Data);			
			CLockedSystemData* LockedData = m_Parent->GetSystemData();
			LockedData->DeleteNerveWork(m_ID);	
		}
#endif
		m_Alive = FALSE;
		return TRUE;
	};

	//CLockedSystemData
	//////////////////////////////////////////////////////////////////////////
	System::CLockedSystemData::CLockedSystemData(CABMutex* mutex)
		:m_pMutex(mutex){
			assert(m_pMutex);
			m_MaxNerveWorkerNum = 100;
			m_NerveMsgMaxNumInPipe = 10;
			m_NerveMsgMaxInterval = 10*1000*1000; //1��
			m_NerveIdleMaxCount   = 30;

			m_NerveWorkingNum = 0;
	};

	System::CLockedSystemData::~CLockedSystemData(){
		Dead();

		CLock lk(m_pMutex);

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

		while(m_DelThreadWorkList.size()){
			Object* Thread  = m_DelThreadWorkList.front();
			m_DelThreadWorkList.pop_front();
			delete Thread;
		} 
	}

	bool System::CLockedSystemData::Activation(){
		CLock lk(m_pMutex);

		map<int64,CSystemIOWork*>::iterator ita = m_SystemIOWorkList.begin();
		while (ita != m_SystemIOWorkList.end())
		{
			CSystemIOWork* IOWork = ita->second;
			assert(!IOWork->IsAlive());
			if(!IOWork->Activation())return FALSE;
			ita++;
		}
		return TRUE;
	}
	void System::CLockedSystemData::Dead(){
		//CLock lk(m_pMutex); IOWork->Dead()由于要调用CLockedSystemData，引发嵌套死锁


		map<int64,CSystemIOWork*>::iterator ita = m_SystemIOWorkList.begin();
		while (ita != m_SystemIOWorkList.end())
		{
			CSystemIOWork* IOWork = ita->second;
			//assert(ModelIOWork->IsAlive());
			IOWork->Dead();
			ita++;
		}

		map<int64,CNerveWork*>::iterator itc = m_NerveWorkList.begin();
		while (itc != m_NerveWorkList.end())
		{
			CNerveWork* NerveWork = itc->second;
			//assert(NerveWork->IsAlive());
			NerveWork->Dead();
			itc++;
		}		

	}

	void  System::CLockedSystemData::IncreNerveWorkCount(){
		CLock lk(m_pMutex);
		++m_NerveWorkingNum ;
	}
	void  System::CLockedSystemData::DecreNerveWorkCount(){
		CLock lk(m_pMutex);
		--m_NerveWorkingNum;
	}

	int64   System::CLockedSystemData::GetNerveMsgInterval(){
		CLock lk(m_pMutex);
		return m_NerveMsgMaxInterval;
	}
	void    System::CLockedSystemData::SetNerveMsgInterval(int32 n){
		CLock lk(m_pMutex);
		m_NerveMsgMaxInterval = n;
	}

	int32   System::CLockedSystemData::GetNerveMaxIdleCount(){
		CLock lk(m_pMutex);
		return m_NerveIdleMaxCount;	
	}

	void   System::CLockedSystemData::SetNerveMaxIdleCount(int32 n){
		CLock lk(m_pMutex);
		m_NerveIdleMaxCount = n;
	}

	int32   System::CLockedSystemData::GetBusyNerveWorkNum(){
		CLock lk(m_pMutex);
		return m_NerveWorkingNum;		
	};

	int32  System::CLockedSystemData::GetNerveWorkNum(){
		CLock lk(m_pMutex);
		int32 n = m_NerveWorkList.size();
		return n;
	}

	int32 System::CLockedSystemData::GetIOWorkNum(){
		CLock lk(m_pMutex);
		return m_SystemIOWorkList.size();
	}

	int32    System::CLockedSystemData::AddIOWork(CSystemIOWork* Work){
		CLock lk(m_pMutex);
		assert(m_SystemIOWorkList.find(Work->m_ID) == m_SystemIOWorkList.end());
		m_SystemIOWorkList[Work->m_ID] = Work;
		return m_SystemIOWorkList.size();
	}

	int32    System::CLockedSystemData::DeleteIOWork(int64 ID){
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

		map<int64,CSystemIOWork*>::iterator it = m_SystemIOWorkList.find(ID);
		if (it != m_SystemIOWorkList.end())
		{
			CSystemIOWork* Work = it->second;
			m_SystemIOWorkList.erase(it);
			m_DelThreadWorkList.push_back(Work);
		}
		return m_SystemIOWorkList.size();
	}

	int32    System::CLockedSystemData::AddNerveWork(CNerveWork* Work){
		CLock lk(m_pMutex);
		assert(m_NerveWorkList.find(Work->m_ID) == m_NerveWorkList.end());
		m_NerveWorkList[Work->m_ID] = Work;
		return m_NerveWorkList.size();
	}

	int32    System::CLockedSystemData::DeleteNerveWork(int64 ID){
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

		map<int64,CNerveWork*>::iterator it = m_NerveWorkList.find(ID);
		if (it != m_NerveWorkList.end())
		{
			CNerveWork* Work = it->second;
			m_NerveWorkList.erase(it);
			m_DelThreadWorkList.push_back(Work);
		}
		return m_NerveWorkList.size();
	}

	bool  System::CLockedSystemData::RequestCreateNewNerveWork(uint32 MsgNum,int64 Interval,uint32& Reason) 
	{
		CLock lk(m_pMutex);

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

	//
	//////////////////////////////////////////////////////////////////////////
	System::CSystemInitData::CSystemInitData():
	m_SystemData(NULL),
		m_SystemListMutex(NULL),
		m_ClientSitMutex(NULL),
		m_Nerve(NULL)
	{

	};

	System::CSystemInitData::~CSystemInitData(){

	};


	//SYSTEM
	//////////////////////////////////////////////////////////////////////////
	System::System(CSystemInitData* InitData)
		:Model(InitData),
		m_Nerve(InitData->m_Nerve),
		m_ClientList(InitData->m_SystemListMutex),
		m_SystemData(InitData->m_SystemData),
		m_ClientSitMutex(InitData->m_ClientSitMutex)
	{
		assert(m_SystemData);	
	}

	System::~System(){

	};

	bool System::Activation(){
		if(m_Alive){
			return TRUE;
		}

		if(!Model::Activation()){
			m_Alive = FALSE;
			return FALSE;
		}


		if(!m_SystemData->Activation()){
			m_Alive = FALSE;
			return FALSE;	
		};

		m_Alive = TRUE;
		return TRUE;
	}

	void System::Dead()
	{
		m_Alive= FALSE;
		Model::Dead();
		m_SystemData->Dead();
	};

	Model::CLockedLinkerList*    System::GetClientLinkerList(){
		return &m_ClientList;
	};
	System::CLockedSystemData*    System::GetSystemData(){
		return m_SystemData;
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
		assert(m_Nerve);
		return m_Nerve->DataNum();

	};

	void	System::GetNerveMsgList(ePipeline& Pipe){
		assert(m_Nerve);
		m_Nerve->CopyTo(Pipe);
	};


	void	System::PopNerveMsg(CMsg& Msg){
		assert(m_Nerve);
		m_Nerve->Pop(Msg);

	};


	void  System::PushNerveMsg(CMsg& Msg,bool bUrgenceMsg){

		int64 MsgID = Msg.GetMsgID();
		int64 EventID = Msg.GetEventID();

		int64 LastMsgPopTime = 0;
		if(bUrgenceMsg){
			LastMsgPopTime = m_Nerve->PushUrgence(Msg.Release());
		}else {
			LastMsgPopTime = m_Nerve->Push(Msg.Release());
		}

		int64 NewMsgPushTime =  CreateTimeStamp();

		assert(NewMsgPushTime!=0);
		assert(NewMsgPushTime!=LastMsgPopTime);

		BOOL ret = NerveWorkStrategy(NewMsgPushTime,LastMsgPopTime);

		if (m_LogFlag & LOG_MSG_NERVE_PUSH)
		{
			tstring MsgStr = MsgID2Str(MsgID);
			if (MsgStr != _T("MSG_EVENT_TICK"))
			{
				tstring s = Format1024(_T("Nerve push msg: %s EventID:%I64ld MsgNum:%d,CreateNewThread:%d"),MsgStr.c_str(),EventID,m_Nerve->DataNum(),ret);
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
				int n = m_SystemData->AddNerveWork(NerveWork);
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
		bool ret = m_SystemData->RequestCreateNewNerveWork(n,t,Reason);
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
			int n =m_SystemData->AddNerveWork(NerveWork);
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
}; //end namespace ABSTRACT
