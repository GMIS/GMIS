/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _CONVERSATION_H__
#define _CONVERSATION_H__

#include "WinSpace2.h"
#include "Splitter.h"
#include "LogicView.h"
#include "InputWin.h"
#include "Outputwin.h"
#include "GuiGadget.h"
#include "LogicDialog.h"



using namespace VISUALSPACE;

#define BNT_ENTER    101

#define BNT_RUN   TO_BRAIN_MSG::TASK_CONTROL::CMD_RUN
#define BNT_PAUSE TO_BRAIN_MSG::TASK_CONTROL::CMD_PAUSE
#define BNT_STOP  TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP

#define BNT_STEP  TO_BRAIN_MSG::TASK_CONTROL::CMD_DEBUG_STEP
#define BNT_BREAK TO_BRAIN_MSG::TASK_CONTROL::CMD_SET_BREAKPOINT

//用于对话进程
class ConvProgressStruct : public CVSpace2
{
public:
    
    VertiacalProgressItem1   m_OutputProgressBar;  
    VertiacalProgressItem1   m_InputProgressBar; 
	
public:
    ConvProgressStruct();
	virtual ~ConvProgressStruct();
	
	void SetOutputPer(int32 Per);
	void SetInputPer(int32 Per);
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};

class SendBnt: public ButtonItem{
public:
	SendBnt(int ID);
	~SendBnt();
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};

class EnterBnt: public ButtonItem{
public:
	EnterBnt(int ID);
	~EnterBnt();
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};

class ConvRightToolbar : public CVSpace2{
public:
	HANDLE          m_Image;
public:
	ConvRightToolbar(HANDLE Image=NULL);
	~ConvRightToolbar();
	
	virtual void Layout(bool Redraw=false);
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};

//窗口按钮
class IOViewBntA: public ButtonItem{
	
public:
	IOViewBntA(int ID,tstring Name,int32 Width = 30);
	~IOViewBntA();
	
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};
class IOViewLockBnt: public IOViewBntA{
public:
	IOViewLockBnt(int ID,int32 Width = 10);
	~IOViewLockBnt();
	
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};


/*用户信息显示窗口
*/
class IOViewToolbarA : public STDToolbar{
public:
	HANDLE         m_Image;
	IOViewBntA     m_BntDialog;
    IOViewBntA     m_BntOutput;
    IOViewBntA     m_BntMemory;
	IOViewBntA     m_BntDebug;
    IOViewBntA     m_BntLogic;
    IOViewBntA     m_BntObject;
    IOViewBntA     m_BntFind;
public:
	IOViewToolbarA(HANDLE Image);
	~IOViewToolbarA();	
	
	void EnableBnt(int64 ID,BOOL bEnable);
};

/*运行时控制
*/
class IOViewToolbarB : public STDToolbar{
public:
	HANDLE          m_Image;
public:
	IOViewToolbarB(HANDLE Image);
	~IOViewToolbarB();	

	void SetState(BOOL bRun,BOOL bPause,BOOL bStop);
	
};
/*用于当前分析结果按钮
*/
class IOViewToolbarC : public STDToolbar{
public:
	HANDLE          m_Image;
public:
	IOViewToolbarC(HANDLE Image);
	~IOViewToolbarC();	
};


//同时扮演工具条容器的角色
class  IOViewSplitterItem:public SplitterItem
{
public:
	IOViewSplitterItem(int64 ID, HANDLE Image);
	virtual ~IOViewSplitterItem();
	virtual void Layout(bool Redraw = true);
    virtual CVSpace2*  HitTest(int32 x,int32 y);		
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};


class IOViewSplitterStruct : public SplitterStruct{
	
public:
	IOViewSplitterStruct(CWinSpace2* Win,int32 W, int32 MinW,bool Vertical=false,HANDLE Image=NULL)
		:SplitterStruct(Win,W,MinW,Vertical,Image){};
	
	virtual SplitterItem* CreateSplitterItem();
};


class CConversation : public CWinSpace2  
{
protected:
    BOOL                   m_Created;
  	CVSpace2*              m_SpaceSelected;
    IOViewSplitterStruct   m_Splitter;
	
	HMENU                  m_PopMenu;
public:
	COutputWin		       m_OutputWin;
	CInputWin		       m_InputWin;	
    TASK_STATE             m_CurTaskState;

public:
	CConversation();
	virtual ~CConversation();

	STDToolbar* GetToolbar(int index);
	void SetCurTaskState(TASK_STATE Type);


	void AddDialogItem(ePipeline& Pipe);
	void AddMoreDialogItem(int64 SourceID,int64 DialogID,ePipeline& Pipe);

	void ClearDialogInfo();
    void AddRuntimeInfo(ePipeline& Item);
	void ClearRuntimeInfo();

    void AddForecastInfo(tstring& s);
	void ClearForecastInfo();

	void SetInputTip(tstring tip);
	
	void ViewWarning(int64 ViewID);
    bool IsViewOpened(int64 ViewID);


	//如果ItemList.size()=0，那么只是简单的设置对应的BNT有效
	void DebugViewProc(ePipeline& Info);
	void SetThinkView(ePipeline& ItemList);
    void SetLogicView(ePipeline& ItemList);
	void SetObjectView(ePipeline& ItemList);

    void FindViewProc(ePipeline& Info);
	void MemoryViewProc(ePipeline& Info);

	void SetCurDialog(int64 SourceID,int64 DialogID,ePipeline& Pipe);
   
    void SetTaskToolbarState(BOOL bRun,BOOL bPause,BOOL bStop);
 
public:
	virtual void    Layout(bool Redraw = true);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);
protected:
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize( WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	void ShowView(int32 ViewID,BOOL bShow);

};

#endif // !defined(AFX_CONVERSATION_H__0DA728ED_9D95_4C56_B7B3_32FA3F6CCFED__INCLUDED_)
