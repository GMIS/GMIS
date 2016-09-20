#include "..\Brain.h"
#include "ConnectNetElt.h"


bool CConnectSocket::Do(ePipeline* Pipe,ePipeline* LocalAddress){

	assert(Pipe->Size()==3);
	
    tstring  s;
	Pipe->PopString(s);
    
	AnsiString ServerName = WStoUTF8(s);

	int32 Port = Pipe->PopInt();
	int32 Time = Pipe->PopInt();
	
	m_StopTime = AbstractSpace::CreateTimeStamp();
	m_StopTime = AbstractSpace::GetTimer()->AddTime(m_StopTime,Time);

		
	if(ServerName[0] == '\0'){
	    Pipe->GetLabel()= Format1024(_T("Connect Fail: Invalid ServerName."));
		return false;
	}
	

	m_Socket=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
	
	if(m_Socket==INVALID_SOCKET){
		Pipe->GetLabel()= Format1024(_T("Connect Fail: can't create socket."));
		return TRUE;
	}
				
	struct hostent *pHost;
	if(inet_addr(ServerName.c_str())==INADDR_NONE)
	{
		pHost=gethostbyname(ServerName.c_str());
	}
	else
	{
		unsigned int addr=inet_addr(ServerName.c_str());
		pHost=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}
	if(pHost==NULL)
	{
		Pipe->GetLabel()= Format1024(_T("Connect Fail: Invalid ServerName."));
		return TRUE;
	}
	
	struct sockaddr_in Server;
	Server.sin_addr.s_addr=*((unsigned long*)pHost->h_addr);
	Server.sin_family=AF_INET;
	Server.sin_port=htons(Port);
	
	//��m_SocketSelf��Ϊ������,�Ա��ܼ�����ӳ�ʱ
	unsigned long ul = 1; //����Ϊ������
	int Ret = ioctlsocket(m_Socket,FIONBIO,(unsigned long*)&ul);
    
	if(Ret == SOCKET_ERROR){
        int ErrorCode = WSAGetLastError();
		Pipe->GetLabel()= Format1024(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
		return TRUE;
	}
	
	int ret = connect(m_Socket,(struct sockaddr*)&Server,sizeof(Server));
	if(ret==SOCKET_ERROR)
	{	//����
		int ErrorCode = WSAGetLastError();
		if (ErrorCode != WSAEWOULDBLOCK){
			Pipe->GetLabel()= Format1024(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);
			return TRUE;
		}
		
	}
	return CheckConnected(Pipe,LocalAddress);
};


bool  CConnectSocket::CheckConnected(ePipeline* Pipe, ePipeline* LocalAddress)
{
/*
	fd_set  WriteFDs; 
	int32   SecondCount = 0;    //��������ʱ���������һ��ı���  
	
	switch(m_ConState)
	{
	case PHYSICAL_CON:
		{
			FD_ZERO(&WriteFDs);		
			FD_SET(m_Socket, &WriteFDs);		
			
			timeval TimeOut;  //����1���ӵļ����
			TimeOut.tv_sec  = 0; 
			TimeOut.tv_usec = 0;
			
			int ret = select(0,NULL, &WriteFDs, NULL, &TimeOut);
			if(ret>0)
			{

				if(FD_ISSET(m_Socket,&WriteFDs)){
				

					CUserLinkerPipe* Linker = GetBrain()->CreateClientLinkerPipe();
					
					assert(0);//�����ƺ�Ӧ���ƶ���userspace
					//Linker->AttachSocket(m_Socket);

					GetBrain()->GetSuperiorLinkerList()->AddLinker(Linker);
					
					//Dialog->m_Brain->CreateLinkerDialog(Linker);  �Ի�����֤��������

					tstring s = tformat(_T("Connect ok...wait feedback"));
                    GetBrain()->OutSysInfo(s);
									//m_AddressBar.SetConnectState(TRUE);
					m_LinkerID = Linker->GetSourceID();
					m_ConState = LOGIC_CON;
				}
			}
			else if (ret == SOCKET_ERROR) { //�˳�

				int ErrorCode = WSAGetLastError();
				Pipe->PrintLabel(_T("Connect Fail: Internal socket error[code:%d]."),ErrorCode);

				return TRUE;
			}
			else if(ret == 0){ //timeout
				

				int64 TimeStamp = GetBrain()->CreateTimeStamp();

				tstring t1 = AbstractSpace::GetTimer()->GetFullTime(TimeStamp);
				tstring t2 = AbstractSpace::GetTimer()->GetFullTime(m_StopTime);
				
				if(TimeStamp <m_StopTime){				
										
					CMsg Msg(MSG_SET_TIMER,0,0);

					ePipeline& Letter = Msg.GetLetter(false);
					TimeStamp += (int64)50*1000*10; //����ת���ɰ�����
					Letter.PushInt64(TimeStamp);
					
					Msg.GetSender() =  *LocalAddress;
					
					GetBrain()->PushCentralNerveMsg(Msg);

					return FALSE;
					
				}else{
					
					//���������Ԥ��ʱ�仹û�������򱨴��˳�
					tstring t1 = AbstractSpace::GetTimer()->GetFullTime(TimeStamp);
					tstring t2 = AbstractSpace::GetTimer()->GetFullTime(m_StopTime);
                
					Pipe->PrintLabel(_T("Connect Fail: Time out(%s > %s)"),t1.c_str(),t2.c_str());
				}
				return TRUE;
			}
	}
	case LOGIC_CON:
		{ //����Ƿ�������ֶԻ�
			

			CLinker Linker = GetBrain()->GetSuperiorLinkerList()->GetLinker(m_LinkerID);
			if(!Linker.IsValid()){
				
				Pipe->PrintLabel(_T("Connect Fail: remote break"));
				return false;
			};

			if (Linker().GetRecoType() == LINKER_FRIEND)
			{
				return false ;
			}else{

				//Ԥ��ʱ������飬����ʱ���򷵻�				
				int64 TimeStamp = GetBrain()->CreateTimeStamp();

				if(TimeStamp <m_StopTime){		
						
					CMsg Msg(MSG_SET_TIMER,0,0); 
					ePipeline& Letter = Msg.GetLetter(false);
					int64 TimeStamp = AbstractSpace::CreateTimeStamp();
					TimeStamp += (int64)50*1000*10; //����ת���ɰ�����
					Letter.PushInt64(TimeStamp);
					
					Msg.GetSender() =  *LocalAddress;
					
					GetBrain()->PushCentralNerveMsg(Msg);

					return false;
				}else{
					

					//���������Ԥ��ʱ�仹û�������򱨴��˳�

					tstring t1 = AbstractSpace::GetTimer()->GetFullTime(TimeStamp);
					tstring t2 = AbstractSpace::GetTimer()->GetFullTime(m_StopTime);

					Pipe->PrintLabel(_T("Connect Fail: Time out(%s > %s)"),t1.c_str(),t2.c_str());
					
				}
				return false;
			}
			
	}
	    break;
	default:
	    break;
	}
	return true;
*/
return TRUE;
}
void CConnectSocket::SysMsgProc(CLogicDialog* Dialog,CMsg& SysMsg,ePipeline* ExePipe,ePipeline* LocalAddress){
/*
	assert(LocalAddress);

	int64 MsgID = SysMsg.GetMsgID();
	
	switch(MsgID){
	case MSG_TIME_OUT:
		Dialog->TaskRun();
        CheckConnected(Dialog,LocalAddress);
		break;
	default:
		Element::SysMsgProc(Dialog,SysMsg,LocalAddress);
		break;
	};
	return;
*/	
}

