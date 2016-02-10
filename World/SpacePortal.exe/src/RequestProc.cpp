#pragma warning (disable:4786)

#include "SpaceMsgList.h"
#include "SpacePortal.h"
//#include "poco\Process.h"

void   CSpacePortal::OnRequestStartObject(int64 SourceID,int64 EventID,ePipeline& RequestInfo){

	ePipeline* ObjectInfo = (ePipeline*)RequestInfo.GetData(0);
	ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(1);

	CObjectData Object(*ObjectInfo);

	ePipeline AddrData = Object.m_Address;

	//Checks whether the specified object addresses is valid
	SpaceAddress Address = FindChildSpace(AddrData,Object.m_Fingerprint);
	if (!Address.IsValid())
	{
		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();
			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Error: object address invalid"));
			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}

	//Need to open the corresponding DLL object in advance, in order to determine the type of executer

	//CSpace Space(Address.ParentID,Address.ChildID);	  

	AddrData = Object.m_Address;

	tstring FilePath = SpacePath2FileAddress(AddrData);
	tstring ObjectName = GetFileName(Object.m_Name); //不含扩展名

	FilePath += _T("\\")+ObjectName;
	FilePath += _T("\\")+Object.m_Name;

	Dll_Object TempDll(Object.m_ID,FilePath);
	if (!TempDll.IsValid())
	{
		int32 error = ::GetLastError();

		ExePipe->SetID(RETURN_ERROR);  
		ExePipe->GetLabel() = Format1024(_T("Load object fail: <%s>"),FilePath.c_str());
		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();

			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return ;
	}

	DLL_TYPE DllType = TempDll.GetDllType();
	tstring ExecuterFile;
	switch (DllType)
	{
	case DLL_VC6:
		{
			ExecuterFile = _T("EXE_VC6.EXE");
		}
		break;
	case DLL_VC6D:
		{
			ExecuterFile = _T("EXE_VC6D.EXE");
		}
		break;
	case DLL_VC10:
		{
			ExecuterFile = _T("EXE_VC10.EXE");
		}
		break;
	case DLL_VC10D:
		{
			ExecuterFile = _T("EXE_VC10D.EXE");
		}
		break;
	default:
		{
			ExePipe->GetLabel() = Format1024(_T("Error: dll type not support:%d"),(int)DllType);
			ExePipe->SetID(RETURN_ERROR);

			CLinker Requester;
			GetLinker(SourceID,Requester);
			if (Requester.IsValid())
			{
				CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
				ePipeline& Letter = FeedbackMsg.GetLetter();
				

				Letter.PushPipe(*ExePipe);  
				Requester().PushMsgToSend(FeedbackMsg);
			}
			return;
		}
	}

	CExecuter* Executer = FindExecuter(DllType);
	if (Executer) 
	{
		int64 SpaceEventID = CreateTimeStamp();										
		CSpaceEvent SpaceEvent(FilePath,CSpaceEvent::EVENT_DONE,EventID,SourceID,*ExePipe);

		Executer->PushEvent(SpaceEventID,SpaceEvent);

		CLinker ExecuterLinker;
		GetLinker(Executer->m_ID,ExecuterLinker);	  

		tstring s = Format1024(_T("Start Object Event:%I64ld"),EventID);
		OutputLog(LOG_TIP,s.c_str());

		//send execute msg
		CMsg NewMsg(MSG_OBJECT_START,DEFAULT_DIALOG,SpaceEventID);
		ePipeline& NewLetter = NewMsg.GetLetter();
		NewLetter.PushString(FilePath);
		NewLetter.PushPipe(*ExePipe);

		ExecuterLinker().PushMsgToSend(NewMsg);  
	}else{
		Executer = AddExecuter(DllType,-1); //-1表示无效连接ID
		assert(Executer);
		int64 SpaceEventID = CreateTimeStamp();
		CSpaceEvent SpaceEvent(FilePath,CSpaceEvent::EVENT_PENDING,EventID,SourceID,*ExePipe);

		Executer->PushEvent(SpaceEventID,SpaceEvent);

		TCHAR buf[512];
		GetCurrentDirectoryW(512,buf);
		tstring ExecuterPath = buf;
		ExecuterPath += _T("\\");
		ExecuterPath += ExecuterFile;

		OutputLog(LOG_TIP,ExecuterPath.c_str());

		STARTUPINFO si;
		PROCESS_INFORMATION  pi;	
		::ZeroMemory(&si, sizeof(si));	
		si.cb = sizeof(si);

		tstring CryptText = _T("GMIS_")+ExecuterFile;
		tstring CmdLine = Format1024(_T(" %s %d"),CryptText.c_str(),DllType);

		TCHAR CmdLineBuf[512];
		_tcscpy(CmdLineBuf,CmdLine.c_str());

		BOOL started = ::CreateProcessW(ExecuterPath.c_str(),        // command is part of input string
			CmdLineBuf,  // (writeable) command string
			NULL,        // process security
			NULL,        // thread security
			FALSE,       // inherit handles flag
			0,           // flags
			NULL,        // inherit environment
			NULL,        // inherit directory
			&si,    // STARTUPINFO
			&pi);   // PROCESS_INFORMATION

		if(!started){
			CSpaceEvent Event;
			Executer->PopEvent(SpaceEventID,Event);

			ExePipe->GetLabel() = Format1024(_T("Start Executer(%s) fail"),ExecuterFile.c_str());
			ExePipe->SetID(RETURN_ERROR);

			CLinker Requester;
			GetLinker(SourceID,Requester);
			if (Requester.IsValid())
			{
				CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
				ePipeline& Letter = FeedbackMsg.GetLetter();

				Letter.PushPipe(*ExePipe);  
				Requester().PushMsgToSend(FeedbackMsg);
			}
		}

		tstring s = Format1024(_T("Waite Executer(%s) starting..."),ExecuterFile.c_str());
		OutputLog(LOG_TIP,s.c_str());
	}
};


void   CSpacePortal::OnRequestUseObject(int64 SourceID,int64 EventID,ePipeline& RequestInfo){

	int64 ExecuterType = RequestInfo.PopInt();
	int64 ObjectInstanceID = RequestInfo.PopInt();

	ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(0);

	CExecuter* Executer = FindExecuter((DLL_TYPE)ExecuterType);

	if (!Executer)
	{
		ExePipe->GetLabel() = Format1024(_T("Error: Executer not started."));
		ExePipe->SetID(RETURN_ERROR);

		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();

			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}
	
	CLinker ExecuterLinker;
	GetLinker(Executer->m_ID,ExecuterLinker);
	if (!ExecuterLinker.IsValid())
	{		  
		ExePipe->GetLabel() = Format1024(_T("Error: Executer not started."));
		ExePipe->SetID(RETURN_ERROR);

		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();

			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}
	int64 SpaceEventID = CreateTimeStamp();
	tstring ObjectPath;
	CSpaceEvent SpaceEvent(ObjectPath,CSpaceEvent::EVENT_DONE,EventID,SourceID,*ExePipe);
	Executer->PushEvent(SpaceEventID,SpaceEvent);

	CMsg NewMsg(MSG_OBJECT_RUN,NULL,SpaceEventID);
	ePipeline& NewLetter = NewMsg.GetLetter();
	NewLetter.PushInt(ObjectInstanceID);
	NewLetter.PushPipe(*ExePipe);

	ExecuterLinker().PushMsgToSend(NewMsg);

};

void    CSpacePortal::OnRequestGetObjectDoc(int64 SourceID,int64 EventID, ePipeline& RequestInfo)
{
	int64 ExecuterType = RequestInfo.PopInt();
	int64 ObjectInstanceID = RequestInfo.PopInt();
	ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(0);

	CExecuter* Executer = FindExecuter((DLL_TYPE)ExecuterType);

	if (!Executer)
	{
		ExePipe->GetLabel() = Format1024(_T("Error: Executer not find."));
		ExePipe->SetID(RETURN_ERROR);

		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();

			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}

	CLinker ExecuterLinker;
	GetLinker(Executer->m_ID,ExecuterLinker);

	if (!ExecuterLinker.IsValid())
	{		  
		ExePipe->GetLabel()=Format1024(_T("Error: Executer not started."));
		ExePipe->SetID(RETURN_ERROR);

		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();

			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}

	int64 SpaceEventID = CreateTimeStamp();
	tstring ObjectPath;
	CSpaceEvent SpaceEvent(ObjectPath,CSpaceEvent::EVENT_DONE,EventID,SourceID,*ExePipe);
	Executer->PushEvent(SpaceEventID,SpaceEvent);

	CMsg NewMsg(MSG_OBJECT_GET_DOC,DEFAULT_DIALOG,SpaceEventID);
	ePipeline& NewLetter = NewMsg.GetLetter();
	NewLetter.PushInt(ObjectInstanceID);
	NewLetter.PushPipe(*ExePipe);

	ExecuterLinker().PushMsgToSend(NewMsg);
};

void    CSpacePortal::OnRequestCloseObject(int64 SourceID,int64 EventID,ePipeline& RequestInfo){

	int64 ExecuterType = RequestInfo.PopInt();
	int64 ObjectInstanceID = RequestInfo.PopInt();
	ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(0);	
		
	CExecuter* Executer = FindExecuter((DLL_TYPE)ExecuterType);
	if (!Executer)
	{
		ExePipe->GetLabel() = Format1024(_T("Error: Executer not find."));
		ExePipe->SetID(RETURN_ERROR);

		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();

			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}

	CLinker ExecuterLinker;
	GetLinker(Executer->m_ID,ExecuterLinker);
	if (!ExecuterLinker.IsValid())
	{				
		CLinker Requester;
		GetLinker(SourceID,Requester);
		if (Requester.IsValid())
		{
			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
			ePipeline& Letter = FeedbackMsg.GetLetter();
			
			Letter.PushPipe(*ExePipe);  
			Requester().PushMsgToSend(FeedbackMsg);
		}
		return;
	}
	
	int64 SpaceEventID = CreateTimeStamp();
	tstring ObjectPath;
	CSpaceEvent SpaceEvent(ObjectPath,CSpaceEvent::EVENT_DONE,EventID,SourceID,*ExePipe);
	Executer->PushEvent(SpaceEventID,SpaceEvent);

	CMsg NewMsg(MSG_OBJECT_CLOSE,DEFAULT_DIALOG,SpaceEventID);
	ePipeline& NewLetter = NewMsg.GetLetter();
	NewLetter.PushInt(ObjectInstanceID);
	NewLetter.PushPipe(*ExePipe);
	
    ExecuterLinker().PushMsgToSend(NewMsg);
}
