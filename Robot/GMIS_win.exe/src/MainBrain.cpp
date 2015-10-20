// MainBrain.cpp: implementation of the CMainBrain class.
//
//////////////////////////////////////////////////////////////////////
#pragma  warning (disable:4786)

#include "GMIS.h"
#include "MainBrain.h"
#include "GUIMsgDefine.h"
#include "MainFrame.h"
#include "LogicDialog.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainBrain::CMainBrain(CUserTimer* Timer,CUserSpacePool* Pool,tstring Name)
:CBrain(Timer,Pool)
{
	m_Name = Name;
//	m_LogFlag =0;
//	m_bCheckInstinct = FALSE;
}

CMainBrain::~CMainBrain()
{

}


void CMainBrain::SendMsgToGUI(int64 GuiID,CMsg& GuiMsg){
	if (GuiID == LOCAL_GUI)
	{
		CMainFrame* MainFrame = GetGUI();
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

#endif 

