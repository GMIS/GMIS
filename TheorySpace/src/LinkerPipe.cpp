// CLinkerPipe.cpp: implementation of the CCLinkerPipe class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "LinkerPipe.h"
#include "Model.h"
#include "TimeStamp.h"

namespace ABSTRACT{
	
	//CLinker
	//////////////////////////////////////////////////////////////////////////
	CLinker::CLinker(CLinkerPipe* LinkerPipe)
		:m_Ptr(LinkerPipe)
	{
		if(m_Ptr){
			m_Ptr->IncreUserNum();
		}
	};
	
	CLinker::CLinker(CLinker& Linker): m_Ptr(Linker.m_Ptr){
		Linker.m_Ptr = NULL;
	};
	
	CLinker& CLinker::operator=(CLinker& Linker)
	{
		if(this != &Linker){
			if (m_Ptr)
			{
				m_Ptr->DecreUserNum();
			}
			m_Ptr = Linker.m_Ptr;
			Linker.m_Ptr = NULL;
		}
		return *this;
	}
	
	CLinker::~CLinker(){
		if(m_Ptr){
			m_Ptr->DecreUserNum();
		}
	}
	bool CLinker::IsValid(){
		return m_Ptr != NULL;
		
	};
	
	const ePipeline& CLinker::GetCompileData()const //用于观察LinkerPipe当前正在组装中的数据
	{
		return m_Ptr->m_CurRevMsg;
	}
	
	CLinkerPipe& CLinker::operator()(){
		assert(m_Ptr);
		return *m_Ptr;
	}

//CInnerIOLock
//////////////////////////////////////////////////////////////////////////
	

	CLinkerPipe::_CInnerIOLock::_CInnerIOLock( CABMutex* mutex,CLinkerPipe* User)
		: m_Mutex(mutex),m_User(User){
		assert(mutex);
		assert(User);
		m_Mutex->AcquireThis(m_User);				
		m_User->m_bThreadUse = TRUE;

	}
	CLinkerPipe::_CInnerIOLock::~_CInnerIOLock (){

		m_User->m_bThreadUse = FALSE;
		m_Mutex->ReleaseThis(m_User);
	}   	

	CLinkerPipe::CLinkerPipe(CABMutex* m,Model* Parent,bool bClientLink)
		:CLockPipe(m),
		m_UseCounter(0),
		m_bThreadUse(FALSE),
		m_SourceID(-1),
		m_RecoType(LINKER_STRANGER),
		m_Parent(Parent),
 		m_SendState(WAIT_MSG),
		m_CurSendMsg(NULL),
		m_PendingMsgID(0),
		m_PendMsgSenderID(0),
		m_SendPos(0),
		m_bRevError(FALSE),
		m_ErrorSaveLen(1024),
		m_StateOutputLevel(NORMAL_LEVEL),
		m_bClientLink(bClientLink),
		m_Owner(NULL)
	{
		
	};
	



	CLinkerPipe::CLinkerPipe(CABMutex* m,Model* Parent,bool bClientLink,int64 SourceID,tstring Name)
		:CLockPipe(m,Name.c_str()),
		m_UseCounter(0),
		m_bThreadUse(FALSE),
		m_SourceID(SourceID),
		m_RecoType(LINKER_STRANGER),
		m_Parent(Parent),
		m_SendState(WAIT_MSG),
		m_CurSendMsg(NULL),
		m_PendingMsgID(0),
		m_PendMsgSenderID(0),
		m_SendPos(0),
		m_bRevError(FALSE),
		m_ErrorSaveLen(1024),
		m_StateOutputLevel(NORMAL_LEVEL),
		m_bClientLink(bClientLink),
		m_Owner(NULL)
	{

	};
	
	CLinkerPipe::~CLinkerPipe(){
		deque<RevContextInfo*>::iterator it = m_ContextStack.begin();
		while (it != m_ContextStack.end())
		{
			RevContextInfo* Info = *it;
			delete Info;
			it++;
		}
		
		if (m_Owner)
		{
			delete m_Owner;
		}
	};
	
	bool IsEqualAddress(ePipeline& Address1,ePipeline& Address2)
	{
		int32 n = Address1.Size();
		if (n != Address2.Size())
		{
			return FALSE;
		}
		
		for (int i=0; i<n; i++)
		{
			int64& ID1 = *(int64*)Address1.GetData(i);
			int64& ID2 = *(int64*)Address2.GetData(i);
			if (ID1 != ID2)
			{
				return FALSE;
			}
		}
		return TRUE;
}
//以下函数内部使用无需加锁
//////////////////////////////////////////////////////////////////////////
	bool  CLinkerPipe::ReceiverID2LocalAddress(ePipeline& Receiver,ePipeline& LocalAddress){
		int64 ReceiverID = Receiver.PopInt();

		if (ReceiverID==SYSTEM_SOURCE)
		{
			LocalAddress.PushInt(ReceiverID);
			LocalAddress<<Receiver; //可能还有其它地址
			return TRUE;
		}
		
		assert(Receiver.Size()==0); //其它情况应该没有多余地址

		map<int64,ePipeline>::iterator It = m_LocalAddressList.find(ReceiverID);
		if(It != m_LocalAddressList.end()){
			ePipeline& Pipe =  It->second;
			LocalAddress = Pipe;
			return TRUE;
		}
		return FALSE;
	};
	
	int64  CLinkerPipe::LocalAddress2SenderID(ePipeline& LocalAddress)
	{
		assert(LocalAddress.Size()!=0);
		
		//CLock lk(m_Mutex);
		map<int64,ePipeline>::iterator It = m_LocalAddressList.begin();
		while (It != m_LocalAddressList.end())
		{
			ePipeline& Address = It->second;
			if(IsEqualAddress(Address,LocalAddress)){
				return It->first;
			}
			It++;
		}
	
		int64 SenderID = CreateTimeStamp();
		tstring s = GetTimer()->GetFullTime(SenderID);

		ePipeline& Address = m_LocalAddressList[SenderID];
		Address<<LocalAddress;
		return SenderID;
	};
	
	/* 流式的处理特定CLinkerPipe接收到的信息,此函数对于理解本类非常重要,
	s为接收到的字节，或多或少
	*/
	void CLinkerPipe::CompileMsg(const char* s, int32 size)
	{
		
		if(m_RecoType == LINKER_BAN)return;   	
				
		CLinkerPipe::RevContextInfo* Info  = NULL;
		
		if( m_ContextStack.size() > 0 ){ 
			Info = m_ContextStack.front(); //取出之前数据的处理信息
		}
		else{
			Info = new CLinkerPipe::RevContextInfo(&m_CurRevMsg);
			
			m_ContextStack.push_front(Info);
		}
		
		assert(Info != NULL);
		

		bool bCompleteOneData = FALSE;
		
		int i=0;
		while(i<size)
		{				
			char ch = s[i++];
			
			Info->InfoLen++;
			
			//在错误状态下将只关心是否有空管道出现
			//在检测到空管道之前，根据要求，即使是出错的数据也将可能被保存起来,以便分析错误原因
			if(m_bRevError)
			{
				if (Info->Buffer.size() < m_ErrorSaveLen)
				{
					Info->Buffer += ch;
				}
				
				if (ch == '@')
				{
					Info->DataLen ++;
					
					if (Info->DataLen ==ERROR_RESUME_LENGTH)
					{
						EndErrorState(Info);
					}
				}else{
					Info->DataLen =0;
				}
				
				continue;
			}
			
			//正常状态
			switch(Info->State){                                              
			case TYPE_PART:                                                    
				{         
					Info->HeaderStr += ch;
					
					if( IsDataType(ch) && Info->DataType == -1){                   
						Info->DataType = CHAR_TO_TYPE(ch);                                
						if(Info->DataType == TYPE_PIPELINE){                      
							Info->Data = new ePipeline();
							Info->ParentPipe->Push_Directly(Info->Data);

						}                                                          
						//不是PIPELINE但是第一个数据则报错，因为信息的第一个数据类型肯定是TYPE_PIPELINE                       
						else if(Info->ParentPipe == this ){               							
							BeginErrorState(Info,ERROR_TYPE_PART);
						}
						else{ //其他类型则预先生成一个空数据
						
							Info->Data = CreateEmptyData(Info->DataType);
							Info->ParentPipe->Push_Directly(Info->Data);

						}
					}  //确定类型后，改变数据处理状态
					else if(ch == '@' && Info->DataType != -1) 
					{						 
						Info->State = (Info->DataType == TYPE_PIPELINE)?ID_PART : LENGTH_PART;
						Info->Buffer="";
						Info->DataLen = NUMBER_LENGTH_LIMIT;
					}
					else{
						BeginErrorState(Info,ERROR_TYPE_PART);
					}
				}
				break;
			case LENGTH_PART:
				{
					Info->HeaderStr += ch;
					
					// ch ->[0-9] 并且 单个数据长度不能超过10位整数				
					if(isdigit(ch) && Info->Buffer.size() < Info->DataLen)
					{
						Info->Buffer  += ch;
					}
					else if(ch == '@' && Info->Buffer.size() >0)
					{
						Info->DataLen = atol(Info->Buffer.c_str());
						Info->Buffer = "";

						//初步检查长度的合理性
						if(Info->DataType == TYPE_PIPELINE)
						{ 
							if (Info->ParentPipe == this)
							{   

							}

							if(Info->DataLen == 0){
								bCompleteOneData = TRUE;
							}else{

								ePipeline  NotifData;
								NotifData.PushInt(Info->DataLen); //总长度
								NotifData.PushInt(0);             //相对于Parent的完成进度，0则表示本Pipe刚开始
								NotifData.Push_Directly(Info->Data->Clone());
								m_Parent->NotifyLinkerState(this,LINKER_RECEIVE_STEP,NotifData);
								

								//对于管道，得到长度以后即压入堆栈，因为它的数据其实是其它数据的集合								
								Info->HeaderStr="";
								
								ePipeline* Parent = (ePipeline*)Info->Data;
								assert(Parent);
								RevContextInfo* NewInfo = new RevContextInfo(Parent);     
								
								m_ContextStack.push_front(NewInfo);
								Info = NewInfo;
							}

							break;    
						}else	if(Info->DataType == TYPE_NULL)
						{
							if(Info->DataLen != 0){
								BeginErrorState(Info,ERROR_LENGTH_PART);
							}else{
								bCompleteOneData = TRUE;
							}
						}
						else if(Info->DataLen ==0 )
						{ 
							if(Info->DataType == TYPE_STRING){ //允许为0
								bCompleteOneData = TRUE;
							}
							else{//	其他数据不能为0 error
								BeginErrorState(Info,ERROR_LENGTH_PART);
							}
						}else if ((Info->DataType==TYPE_INT || Info->DataType==TYPE_FLOAT) && Info->DataLen>20)
						{
							BeginErrorState(Info,ERROR_LENGTH_PART);
						}
						else{	
							Info->State = DATA_PART;   //一切OK，准备开始处理数据本身
						}
					}
					else{
						BeginErrorState(Info,ERROR_LENGTH_PART);
					}
				}
				break;
				
			case DATA_PART:
				{
					Info->Buffer += ch;
					
					if( Info->Buffer.size() == Info->DataLen ){
						
						Info->HeaderStr+=Info->Buffer;							
						int32 n = Info->Data->FromString(Info->HeaderStr,0);
						assert(n!=0);
						
						bCompleteOneData = TRUE;

					}
				}
				break;
			case ID_PART:
				{		
					//	Info->HeaderStr += ch;
					
					// ch ->[0-9] 并且数据ID不能超过20位整数				
					if(isdigit(ch) && Info->Buffer.size() < Info->DataLen)
					{
						Info->Buffer += ch;
					}
					else if(ch == '@' && Info->Buffer.size() >0)
					{
						int64 ID = atoint64(Info->Buffer.c_str());
						
						assert(Info->DataType == TYPE_PIPELINE);
						ePipeline* Data = (ePipeline*)Info->Data;
						Data->SetID(ID); 
						
						Info->State = NAMELEN_PART;
						Info->Buffer = "";
						Info->DataLen = DATA_LENGTH_LIMIT;
					}
					else{
						BeginErrorState(Info,ERROR_ID_PART);						
					}
				}
				break;
			case NAMELEN_PART:
				{
					//	Info->HeaderStr += ch;
					
					// ch ->[0-9] 并且 不能超过10位整数				
					if(isdigit(ch) && Info->Buffer.size() < Info->DataLen)
					{
						Info->Buffer += ch;
					}
					else if(ch == '@' && Info->Buffer.size() >0)
					{
						uint32 len = atol(Info->Buffer.c_str());
						
						Info->DataLen = len; 
						Info->Buffer = "";
						assert(Info->Buffer.size()==0);
						Info->State = NAME_PART;
					}
					else{
						BeginErrorState(Info,ERROR_NAMELEN_PART);		
					}
				}
				break;
			case NAME_PART:
				{
					//	Info->HeaderStr += ch;
					
					// ch ->[0-9] 并且 不能超过约定位数				
					if(Info->Buffer.size() < Info->DataLen)
					{
						Info->Buffer += ch;
					}
					else if(ch == '@'){
						eSTRING s;
						s.FromString(Info->Buffer,0);
						tstring Name = s;

						assert(Info->DataType == TYPE_PIPELINE);
						ePipeline* Data = (ePipeline*)Info->Data;
						Data->SetLabel(Name.c_str()); 
						
						Info->Buffer = "";
						Info->State = LENGTH_PART;
						Info->DataLen = DATA_LENGTH_LIMIT;
					}
					else{
						BeginErrorState(Info,ERROR_NAME_PART);
					}
				}
				break;
			default:
				assert(0);
				break;
				
			}
			
			if(bCompleteOneData){
				bCompleteOneData = FALSE;
				
				//处理得到的实际数据
				while(m_ContextStack.size()){	
					
					//Info->ParentPipe->Push_Directly(Info->Data);
					
					int32 Len = Info->InfoLen;
		            Energy* Data = Info->Data;
					
					if (m_ContextStack.size()>1)
					{
						RevContextInfo* PreInfo = m_ContextStack[1];
						
						PreInfo->InfoLen += Len;
						
						PreInfo->DataLen -= Len;
						
						if (PreInfo->DataLen  == 0) //已经完成一个嵌套Pipe,重复上述步骤
						{
						
							ePipeline  NotifData;
							NotifData.PushInt(Len);   //总长度								
							NotifData.PushInt(PreInfo->InfoLen); //Parent Pipe已经完成的
							NotifData.Push_Directly(Data->Clone());								
							m_Parent->NotifyLinkerState(this,LINKER_RECEIVE_STEP,NotifData);
															
							delete Info;
							m_ContextStack.pop_front();
							Info = m_ContextStack.front();
							
							assert(PreInfo == Info);	

						}else if (PreInfo->DataLen > 0) //继续接收下一个数据
						{
							Info->Reset();  
							break;
						}else{ //错误
							BeginErrorState(Info,ERROR_OTHER_PARNT);
							Info->Reset();
						}
					}else{ //已经完成一个完整信息的组装
						
						assert(m_ContextStack.size()==1);
						assert(m_CurRevMsg.Size()==1);

						eElectron E;
						m_CurRevMsg.Pop(&E);
						
					
						RevOneMsg(E);

						assert(Info->ParentPipe == &m_CurRevMsg);
						Info->Reset();
						break;
					}
				}//while
			}
      }//while
};

	void CLinkerPipe::RevOneMsg(eElectron& E){
		ePipeline* Msg = (ePipeline*)E.Get();
		
		Msg = Decrypt(Msg);

        //  Msg->AutoTypeAB();
		
		int64 ID = Msg->GetID();
        if (ID < 100) //内部控制信息
        {
			assert(ID == LINKER_FEEDBACK);
            if (ID != LINKER_FEEDBACK)
            {
				ePipeline Info;
				Info.Push_Directly(E.Release());
				m_Parent->NotifyLinkerState(this,LINKER_ILLEGAL_MSG,Info);
				return;
            }

			if(m_PendingMsgID==NULL){
				return;
			}
			
			if(m_SendState == SEND_MSG){ 
				m_SendState = WAIT_MSG;
			}

			int32 RevResult   = Msg->PopInt();
 
			if (RevResult == RECEIVE_ERROR)
			{						
				//先发送一个重置接收信息，以便对方能从错误接收状态下恢复
				ePipeline*  Pipe = new ePipeline((uint64)LINKER_RESET);

                ePipeline Info;
				Info.PushInt(LINKER_FEEDBACK);
				Info.PushInt(RevResult);
				Info.PushInt(m_PendingMsgID);
				m_Parent->NotifyLinkerState(this,LINKER_MSG_RECEIVED,Info);
				
				m_PendingMsgID = 0;//取消未决信息，让本连接能继续发送
				m_PendMsgSenderID = 0;

				FeedbackDirectly(Pipe);

			}else{
				int64 TimeStamp   = Msg->PopInt();
				int64 ReceiverID  = Msg->PopInt();

				assert(m_PendMsgSenderID == ReceiverID);
				//assert(m_PendingMsg->GetID() == TimeStamp); 可能在m_CurSendMsg.Reset()时被delete
			
                ePipeline Info;
				Info.PushInt(LINKER_FEEDBACK);
				Info.PushInt(RevResult);
				Info.PushInt(m_PendingMsgID);
				m_Parent->NotifyLinkerState(this,LINKER_MSG_RECEIVED,Info);

				m_PendingMsgID = 0;//取消未决信息，让本连接能继续发送
				m_PendMsgSenderID = 0;
			}
       } 
       else
       {

			Msg->AutoTypeAB();
			if(Msg->GetTypeAB() != 0x44400000){ //格式不正确，丢弃
				ePipeline Info;
				Info.Push_Directly(E.Release());
				m_Parent->NotifyLinkerState(this,LINKER_ILLEGAL_MSG,Info);
				return;
			}
			
			CMsg m((ePipeline*)E.Release()); //包装成标准封装格式的信息
			int64 SenderID = m.GetSenderID();
		
			int64 MsgID = m.GetMsgID();

			//首先用对方的时间戳和发信者回复一个OK
			int64 TimeStamp = Msg->GetID();	
			ePipeline* rMsg = new ePipeline(LINKER_FEEDBACK);
			rMsg->PushInt(RECEIVE_OK);
			rMsg->PushInt(TimeStamp);
			rMsg->PushInt(SenderID);
			
			FeedbackDirectly(rMsg);

			ePipeline& Receiver = m.GetReceiver(); 
			
			//assert(Receiver.Size()==0); 缺省的MSG_DIALOG_NOTIFY接受者地址size=2 

			//把接收者ID转换成本地地址
			ePipeline LocalAddress;
			bool ret = ReceiverID2LocalAddress(Receiver,LocalAddress);
			if (!ret)
			{
				ePipeline Info;
				Info.Push_Directly(m.Release());
				m_Parent->NotifyLinkerState(this,LINKER_INVALID_ADDRESS,Info);
				return;
			}
			
			
			m.SetSourceID(m_SourceID);
			m.GetReceiver() << LocalAddress;	
			m_Parent->PushCentralNerveMsg(m);			
        }

	};
	
	void CLinkerPipe::BeginErrorState(RevContextInfo* Info,int32 ErrorType)
	{
		m_bRevError = true;
		Info->DataLen = 0;
	
		//通知对方接收错误，等对方发一个重置信息回来
		ePipeline* rMsg = new ePipeline(LINKER_FEEDBACK);
		rMsg->PushInt(RECEIVE_ERROR);
        
        FeedbackDirectly(rMsg);
        
		ePipeline Data;
		Data.PushInt(ErrorType);
		Data.Push_Directly(m_CurRevMsg.Clone());
		m_Parent->NotifyLinkerState(this,LINKER_COMPILE_ERROR,Data);
	};
	
	void CLinkerPipe::EndErrorState(RevContextInfo* Info)
	{
	
		m_bRevError = false;     
		
		ePipeline* Pipe =Info->ParentPipe;
		Pipe->PushString(Info->Buffer);
		
		eElectron E;
		m_CurRevMsg.Pop(&E);
		
        ePipeline* Msg = (ePipeline*)E.Release();
		
		ePipeline Data;
        m_Parent->NotifyLinkerState(this,LINKER_RECEIVE_RESUME,Data);
		
		deque<RevContextInfo*>::iterator it = m_ContextStack.begin();
        while (it != m_ContextStack.end())
        {
			RevContextInfo* r = *it;
			delete r;
			it++;
        }
		m_ContextStack.clear();
		
		assert(m_CurRevMsg.Size()==0);
	}

    void   CLinkerPipe::IncreUserNum(){
		CLock lk(m_Mutex);
		m_UseCounter++;
	}
	void   CLinkerPipe::DecreUserNum(){
		CLock lk(m_Mutex);
		m_UseCounter--;
	}
	void CLinkerPipe::ClearSendMsgList(){
		Clear();
		m_CurSendMsg.Reset();
		m_SendPos = 0;
		m_SendBuffer = "";
	};

	
	Energy* CLinkerPipe::CreateEmptyData(int32 Type){
		Energy* Data = NULL;
		switch((ENERGY_TYPE)Type){
		case TYPE_NULL:  
			Data = new eNULL();
			break;
		case TYPE_INT:
			Data = new eINT();
			break;
    	case TYPE_FLOAT:
			Data = new eFLOAT();
			break;
		case TYPE_STRING:
			Data = new eSTRING();
			break;
		case TYPE_BLOB:
			Data = new eBLOB();
			break;
		default:
			assert(0);
			
		}//switch
		return Data;
	};

//以下函数必须加锁保证线程安全，但又要避免递归死锁
//////////////////////////////////////////////////////////////////////////

	void   CLinkerPipe::SetSourceID(int64 SourceID){ 
		CLock lk(m_Mutex,this);
		m_SourceID = SourceID;
	}
	
	int64  CLinkerPipe::GetSourceID(){
		CLock lk(m_Mutex,this);
        int64 ID = m_SourceID;
		return ID;
	};

	void  CLinkerPipe::SetStateOutputLevel(STATE_OUTPUT_LEVEL  Level){
		CLock lk(m_Mutex,this);
		m_StateOutputLevel = Level;
	}

    STATE_OUTPUT_LEVEL  CLinkerPipe::GetOutputLevel(){
		return m_StateOutputLevel;
	}

	bool  CLinkerPipe::IsValid(){
		CLock lk(m_Mutex,this);
		return m_RecoType>LINKER_INVALID;
	};
	
	void CLinkerPipe::Close(){
		CLock lk(m_Mutex,this);

		Clear();
		m_RecoType = LINKER_INVALID;
		m_SendPos = 0;
		m_SendBuffer = "";
	};
	
	void CLinkerPipe::Reset(){
		CLock lk(m_Mutex,this);

		m_LocalAddressList.clear();
		m_RecoType    = LINKER_INVALID;
		m_SendState   = WAIT_MSG;
		m_SendPos     = 0;
		m_CurRevMsg.Clear();
		m_bRevError = FALSE;
      
		deque<RevContextInfo*>::iterator it = m_ContextStack.begin();
		while (it != m_ContextStack.end())
		{
			RevContextInfo* Info = *it;
			delete Info;
			it++;
		}

	}

	void   CLinkerPipe::CloseDialog(int64 LocalID){
		CLock lk(m_Mutex,this);
		
		map<int64,ePipeline>::iterator It = m_LocalAddressList.begin();
		while (It != m_LocalAddressList.end())
		{
			ePipeline& Address = It->second;
			
			if (Address.GetID() == LocalID)
			{
				It = m_LocalAddressList.erase(It);
			}else{
				It++;
			}
		}
	};
		
	int32  CLinkerPipe::GetDialogCount(){
		CLock lk(m_Mutex,this);
		return m_LocalAddressList.size();
	};

	int32 CLinkerPipe::GetRecoType(){
		CLock lk(m_Mutex,this);
		int32 r =  m_RecoType;
		return r;
	};
	
	void  CLinkerPipe::SetRecoType(int32  Type)
	{
		CLock lk(m_Mutex,this);
		m_RecoType = Type;
	}
	  

	int32  CLinkerPipe::GetUserNum(){
		CLock lk(m_Mutex,this);
		int n = m_UseCounter;
		return n;
	}

	SendState CLinkerPipe::GetSendState(){
		CLock lk(m_Mutex,this);
		SendState s = m_SendState;
		return s;
	};


	void CLinkerPipe::FeedbackDirectly(ePipeline* Msg){
		Msg = Encrypt(Msg);
		m_UrgenceMsg.Push_Directly(Msg);
	}

	int64 CLinkerPipe::PushMsgToSend(CMsg& Msg,bool bUrgence){
		
		assert(Msg.IsValid());
		
		CLock lk(m_Mutex,this);
		
		ePipeline& Sender = Msg.GetSender();
		ePipeline& Receiver = Msg.GetReceiver();
        ePipeline& Letter   = Msg.GetLetter();
		
		int64 MsgID = Letter.GetID();
        assert(MsgID != 0);
        if (MsgID==0)
        {
			return 0;
        }

		int64 EventID = Msg.GetEventID();

		//把本地发送者地址用一个INT64代替
		int64 SenderID = LocalAddress2SenderID(Sender);	
		Sender.Clear();
		Sender.PushInt(SenderID);
		
		//用时间戳代替源ID，对方返回此时间戳表示正常接受
		int64 TimeStamp = CreateTimeStamp();
        Msg.SetSourceID(TimeStamp);

		ePipeline* MsgPtr = (ePipeline*)Msg.Release();
		Encrypt(MsgPtr);

		ePipeline Info;
		Info.PushInt(MsgID);
		Info.PushInt(EventID);
		Info.PushInt(TimeStamp);
		Info.PushInt(m_PendingMsgID);
		Info.PushInt(Size());
		Info.PushInt(m_UrgenceMsg.Size());
		m_Parent->NotifyLinkerState(this,LINKER_PUSH_MSG,Info);

		if (bUrgence)
		{
			m_UrgenceMsg.Push_Directly(MsgPtr);
		}else{
			Push_Directly(MsgPtr);
		}
		return MsgID;
	}; 

	
	void CLinkerPipe::BreakSend()
	{
		CLock lk(m_Mutex,this);
		
		if(m_SendState == SEND_MSG)
		{
			int32 n = m_SendBuffer.size() - m_SendPos; //还剩多少数据没有发送			
			memset((void*)(m_SendBuffer.c_str()+m_SendPos),'\0',n);
			m_SendState   = SEND_BREAK;	
		}
	}
		
} //namespace ABSTRACT
