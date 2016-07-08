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

	m_LogFlag = LOG_MSG_RUNTIME_TIP|LOG_TIP;

	tstring Dir = GetCurrentDir();
	tstring LogDir =  Dir+_T("\\Log.DB");
	m_LogDB.Open(LogDir);

}

CMainBrain::~CMainBrain()
{

}

void  CMainBrain::SendMsgToLocalGUI(CMsg& GuiMsg){
	CMainFrame* MainFrame = GetGUI();
	if(MainFrame){
		MainFrame->PushGUIMsg(GuiMsg);
	}
}

