#include "GetSpaceCatalogElt.h"
#include "..\GUIMsgDefine.h"
#include "..\LogicDialog.h"

CGetSpaceCatalog::CGetSpaceCatalog(int64 ID)
	:CElement(ID,_T("UseObject")),m_InstanceID(0)
{
};

CGetSpaceCatalog::~CGetSpaceCatalog(){

}

MsgProcState CGetSpaceCatalog::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_SPACE_CATALOG)
	{	
		ePipeline& Letter = Msg.GetLetter();

		CBrainEvent EventInfo;
		bool ret = Dialog->m_Brain->GetBrainData()->GetEvent(GetEventID(),EventInfo,true);
		if (!ret) 
		{
			ExePipe.GetLabel() = Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str());
			ExePipe.SetID(RETURN_ERROR);
			return RETURN_DIRECTLY;
		}

		CMsg GuiMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);

		ePipeline Cmd(TO_SPACE_VIEW::ID);
		Cmd.PushInt(TO_SPACE_VIEW::CATALOG);
		Cmd<<Letter;

		GuiMsg.GetLetter().PushPipe(Cmd);

		ePipeline& ClientAddress = EventInfo.m_ClientAddress;
		int64 SourceID= ClientAddress.GetID();
		if (SourceID==0) //向所有用户广播此信息
		{
			Dialog->m_Brain->GetBrainData()->SendMsgToGUI(Dialog->m_Brain,GuiMsg,-1);
		}else{ //向指定对话发送此信息
			Dialog->m_Brain->GetBrainData()->SendMsgToGUI(Dialog->m_Brain,SourceID,GuiMsg);
		}

		ePipeline& OldExePipe = EventInfo.m_ClientExePipe;

		Dialog->CloseChildDialog(GetEventID(),OldExePipe,ExePipe);


		return RETURN_DIRECTLY; // 不在继续执行TaskProc
	}else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		return ret;
	}

	return RETURN_DIRECTLY;
}


bool  CGetSpaceCatalog::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	CLinker Linker;
	Dialog->m_Brain->GetLinker(SPACE_SOURCE,Linker);

	if(Linker.IsValid()){
		return false;
	}else{
		CMsg rMsg(Dialog->m_SourceID,DEFAULT_DIALOG,MSG_ROBOT_GOTO_SPACE,DEFAULT_DIALOG,0);
		rMsg.GetSender() = LocalAddress;
		ePipeline& rLetter = rMsg.GetLetter();

		ePipeline Addr(LOCAL_SPACEID);
		rLetter.PushPipe(Addr);  //意味着要求获得根空间目录
		Linker().PushMsgToSend(rMsg);
	}

	return true;
};
