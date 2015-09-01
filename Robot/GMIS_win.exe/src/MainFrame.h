/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _MAINFRAME_H__
#define _MAINFRAME_H__

#include "MainBrain.h"
#include "DialogView.h"
#include "StatusBar.h"
#include "WSFrameView.h"
#include "Conversation.h"
#include "LinkerView.h"
#include "AddressBar.h"
#include "PerformanceView.h"
#include "OptionView.h"
#include "Splitter.h"
#include "GUIMsgDefine.h"
#include "WorldShow.h"
#include "MapView.h"
#include "ObjectView.h"
#include "Ipclass.h"
#include "Object.h"
#include "LogView.h"


#define GUI_MSG_PROC_TIMER 9000

class CMainFrame : public CWSFrameView   
{
	BOOL				m_Created;

	class GUIMsgObject:public Object
	{
		CMainFrame*  m_Parent;
	public:
		GUIMsgObject(CMainFrame* Parent){ m_Parent = Parent;}
		~GUIMsgObject(){};
		bool Do(Energy* E/* =NULL */)
		{
			while (m_Alive)
			{
				m_Parent->GUIMsgProc();
				SLEEP_MILLI(20);
			}
			return true;
		}
	};

public:
 
	BOOL                m_bOpenPerformanceView;

	CAddressBar         m_AddressBar;
	CConversation		m_ConvView;
	CLinkerView			m_LinkerView;
	CPerformanceView    m_PerformanceView;
	COptionView         m_OptionView;
	CLogView            m_LogView;

	CStatusBar			m_Status;

	BOOL                m_bShowWorld;
	CWorldShow          m_WorldShow;

	CTitleMapView       m_MapView;

    CObjectView         m_ObjectView;

	CVSpace2*           m_SpaceSelected;
	SplitterStruct      m_Splitter;

	CUserMutex			m_Mutex;
	CLockPipe           m_MsgList;
	GUIMsgObject        m_MsgProcObject;
	
	int32               m_OldSplitterPos;
//Construction/Destruction
//////////////////////////////////////////////////////////
public:
	CMainFrame();
	virtual ~CMainFrame();

	void  InitSplitterPos(int32 Pos){
		m_OldSplitterPos = Pos;
		m_Splitter.SetFirstSplitterPos(Pos);
	};

	void GetSpacePath(ePipeline& Path){
		m_MapView.m_MapView.GetSpacePath(Path);
	};

	void  GetFocusDialog(int64& SourceID,int64& DialogID);
    bool  IsFocusDialog(int64 SourceID,int64 DialogID);
	void  SetCurDialogState(TASK_STATE State);

	void  SendMsgToBrain(int64 SourceID,int64 DialogID, ePipeline& Msg,int64 EventID=0); //GUI界面消息
	void  SendMsgToBrainFocuse( ePipeline& Msg,int64 EventID=0);

	void  PushGUIMsg(CMsg& Msg){
		m_MsgList.Push(Msg.Release());
	}
     	
	void GUIMsgProc();   

protected:
	
	void OnSpaceOutput(ePipeline& Letter);
		void OnSpaceOutput_Catalog(ePipeline& Data);
        void OnSpaceOutput_Added(ePipeline& Data);
		void OnSpaceOutput_Deleted(ePipeline& Data);
		void OnSpaceOutput_SendObject(ePipeline& Data);
		void OnSpaceOutput_AccessDenied(ePipeline& Data);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT			OnCreate(WPARAM wParam, LPARAM lParam);
	virtual void    Layout(bool Redraw=true);
	LRESULT			ParentReaction(SpaceRectionMsg* SRM);

	LRESULT         OnTimer(WPARAM wParam, LPARAM lParam);
	LRESULT			OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT			OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT			OnMouseMove(WPARAM wParam, LPARAM lParam);

};



#endif // !defined(_MAINFRAME_H__)
