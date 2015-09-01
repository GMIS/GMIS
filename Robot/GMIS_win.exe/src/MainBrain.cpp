// MainBrain.cpp: implementation of the CMainBrain class.
//
//////////////////////////////////////////////////////////////////////
#pragma  warning (disable:4786)

#include "GMIS.h"
#include "MainBrain.h"
#include "GUIMsgDefine.h"
#include "MainFrame.h"
#include "TaskDialog.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainBrain::CMainBrain(CSystemInitData* InitData)
:CBrain(InitData)
{
//	m_LogFlag =0;
//	m_bCheckInstinct = FALSE;
}

CMainBrain::~CMainBrain()
{

}


void CMainBrain::SendMsgToGUI(int64 GuiID,CMsg& GuiMsg){
	if (GuiID == LOCAL_GUI)
	{
		CMainFrame* MainFrame =GetGUI();
		if(MainFrame){
			MainFrame->PushGUIMsg(GuiMsg);
		}
	}else{
		CLinker Linker;
		GetLinker(GuiID,Linker);
		if (Linker.IsValid())
		{
			Linker().PushMsgToSend(GuiMsg);
		}
	}
}




#ifndef USING_GUI  //直接使用主线程

bool CMainBrain::Do(Energy* E){
	while (m_Alive)
	{
		CBrain::Do();
	}
	return true;
}

bool CMainBrain::Activation(){
	if (m_Alive)
	{
		return true;
	}
	
	tstring error;

   
	//生成系统缺省任务
	int64 ID = SYSTEM_SOURCE;

	CSystemTask* Task = new CSystemTask(this,ID,DIALOG_SYSTEM_MAIN);
	m_TaskList[ID]=Task;


	if(!CBrain::Activation()){
		return false;
	}
	return true;

}

#else

bool CMainBrain::Activation(){
	if (m_Alive)
	{
		return true;
	}
	
    if(!CBrain::Activation())return false;
	

	return true;
	
}
#endif 

