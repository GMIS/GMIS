#pragma warning (disable:4786)

#include "SpaceMsgList.h"
#include "SpacePortal.h"

void   CSpacePortal::OnRequestStartObject(int64 SourceID,ePipeline& RequestInfo){

  int64 EventID = RequestInfo.PopInt();
  ePipeline* ObjectInfo = (ePipeline*)RequestInfo.GetData(0);
  ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(1);

  CObjectData Object(*ObjectInfo);
  
  ePipeline AddrData = Object.m_Address;
  assert(Object.m_ID != 0);

  //Checks whether the specified object addresses is valid
  SpaceAddress Address = FindChildSpace(AddrData,Object.m_Fingerprint);
  if (!Address.IsValid())
  {
	  CLinker Requester;
	  GetLinker(SourceID,Requester);
	  if (Requester.IsValid())
	  {
		CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
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
		  CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
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
				  CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
				  ePipeline& Letter = FeedbackMsg.GetLetter();
				  
				  
				  Letter.PushPipe(*ExePipe);  
				  Requester().PushMsgToSend(FeedbackMsg);
			  }
			  return;
		  }
  }

  //start a executer     
  int64 ExeSourceID = Object.m_ID;  //ExecuterID is equal to the external object instance ID
  

  bool ret = StartExecuter(ExeSourceID,ExecuterFile);
  if (!ret)
  {
	  ExePipe->GetLabel() = Format1024(_T("Error: Executer start fail."));
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
  RegisterExecuterUser(SourceID,ExeSourceID);

  CLinker ExecuterLinker;
  GetLinker(ExeSourceID,ExecuterLinker);	  
 
  //Register a start  external object event
  PushExecuterEvent(ExeSourceID,SourceID,EventID);

  WriteLogDB(_T("Start Object Event:%I64ld"),EventID);

  //send execute msg
  CMsg NewMsg(MSG_OBJECT_START,NULL,EventID);
  ePipeline& NewLetter = NewMsg.GetLetter();
  NewLetter.PushString(FilePath);
  NewLetter.PushPipe(*ExePipe);
  
  ExecuterLinker().PushMsgToSend(NewMsg);  
};

void   CSpacePortal::OnRequestUseObject(int64 SourceID,ePipeline& RequestInfo){

  int64 EventID = RequestInfo.PopInt();
  ePipeline* ObjectInfo = (ePipeline*)RequestInfo.GetData(0);
  ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(1);

  CObjectData Object(*ObjectInfo);
  
  int64  ExecuterID = Object.m_ID;

  CLinker ExecuterLinker;
  GetLinker(ExecuterID,ExecuterLinker);
  if (!ExecuterLinker.IsValid())
  {		  
	  ExePipe->GetLabel() = Format1024(_T("Error: Executer not started."));
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

  PushExecuterEvent(ExecuterID,SourceID,EventID);

  WriteLogDB(_T("Use Object Event:%I64ld"),EventID);


  CMsg NewMsg(MSG_OBJECT_RUN,NULL,EventID);
  ePipeline& NewLetter = NewMsg.GetLetter();
  NewLetter.PushPipe(*ExePipe);
  
  ExecuterLinker().PushMsgToSend(NewMsg);
  
};
void    CSpacePortal::OnRequestGetObjectDoc(int64 SourceID, ePipeline& RequestInfo)
{
	int64 EventID = RequestInfo.PopInt();
	ePipeline* ObjectInfo = (ePipeline*)RequestInfo.GetData(0);
	ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(1);

	CObjectData Object(*ObjectInfo);


	int64  ExecuterID = Object.m_ID;

	CLinker ExecuterLinker;
	GetLinker(ExecuterID,ExecuterLinker);
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
	PushExecuterEvent(ExecuterID,SourceID,EventID);

    WriteLogDB(_T("Get Object Doc Event:%I64ld"),EventID);


	CMsg NewMsg(MSG_OBJECT_GET_DOC,NULL,EventID);
	ePipeline& NewLetter = NewMsg.GetLetter();
	NewLetter.PushPipe(*ExePipe);

	ExecuterLinker().PushMsgToSend(NewMsg);
};

void    CSpacePortal::OnRequestCloseObject(int64 SourceID,ePipeline& RequestInfo){

	int64 EventID = RequestInfo.PopInt();
	ePipeline* ObjectInfo = (ePipeline*)RequestInfo.GetData(0);
	ePipeline* ExePipe = (ePipeline*)RequestInfo.GetData(1);
	
	CObjectData Object(*ObjectInfo);
		

	//If it does not start the external organs, this is considered a normal close
	int64 ExecuterID = Object.m_ID;
	CLinker ExecuterLinker;
	GetLinker(ExecuterID,ExecuterLinker);
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
	

	PushExecuterEvent(ExecuterID,SourceID,EventID);

	WriteLogDB(_T("Close Object Event:%I64ld"),EventID);


	CMsg NewMsg(MSG_OBJECT_CLOSE,NULL,EventID);
	ePipeline& NewLetter = NewMsg.GetLetter();

	NewLetter.PushPipe(*ExePipe);
	
    ExecuterLinker().PushMsgToSend(NewMsg);
}
