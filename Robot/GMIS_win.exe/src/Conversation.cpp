// Conversation.cpp: implementation of the CConversation class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786) 
#pragma warning (disable: 4244)
#include "GMIS.h"
#include "MainFrame.h"
#include "Conversation.h"
#include "LinkerView.h"
#include "SystemSetting.h"
#include "LogicThread.h"
#include "LogicDialog.h"

ConvProgressStruct::ConvProgressStruct(){
	m_OutputProgressBar.m_crProgress = RGB(0x7c,0xfc,0);
	m_InputProgressBar.m_crProgress =  RGB(0xFF,0x6C,0x6C);
	
};

ConvProgressStruct::~ConvProgressStruct(){
	
};

void ConvProgressStruct::SetOutputPer(int32 Per){
	m_OutputProgressBar.m_Per = Per;
};

void ConvProgressStruct::SetInputPer(int32 Per){
	m_InputProgressBar.m_Per = Per;
};

void ConvProgressStruct::Draw(HDC hDC, ePipeline* Pipe){
	int32 w = AreaWidth();
	if(w==0)return;
	
	int p = (w)/3;
	m_InputProgressBar.SetArea(this);
	m_InputProgressBar.m_AreaRight = m_InputProgressBar.m_AreaLeft+p;
    m_InputProgressBar.m_AreaLeft = m_InputProgressBar.m_AreaRight-6 ;
	
	m_OutputProgressBar.SetArea(this);
	m_OutputProgressBar.m_AreaLeft += 2*p;
    m_OutputProgressBar.m_AreaRight = m_OutputProgressBar.m_AreaLeft+6;
	
	m_InputProgressBar.Draw(hDC);
	m_OutputProgressBar.Draw(hDC);
};


SendBnt::SendBnt(int ID)
:ButtonItem(ID,_T("Send"),NULL,false){
}

SendBnt::~SendBnt(){};

void SendBnt::Draw(HDC hDC, ePipeline* Pipe){
    
	RECT rc = GetArea();

	DeflateRect(&rc,1,1,1,1);
      
	COLORREF crText = RGB(100,100,100);
	if (m_State & SPACE_SELECTED)
	{
		FillRectGlass(hDC,rc,RGB(0x36,0x6c,0xa4),FALSE);  
		//Draw3DEdge(hDC,rc);
		::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
		crText = RGB(0,0,0);
	}else if(m_State & SPACE_FOCUSED){
		FillRectGlass(hDC,rc,RGB(0x36,0x6c,0xa4),FALSE);  
		crText = RGB(0,0,0);
		//DrawEdge(hDC,rc,crText);
	}else{
		FillRectGlass(hDC,rc,RGB(0xb0,0xc4,0xde),FALSE);
		//DrawEdge(hDC,rc,crText);
	}

	HFONT oldfont = (HFONT)::SelectObject(hDC,SS.m_Font12B);
	COLORREF crOld = ::SetTextColor(hDC,crText);
	::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    ::SetTextColor(hDC,crOld);
	::SelectObject(hDC,oldfont);

}
EnterBnt::EnterBnt(int ID)
:ButtonItem(ID,_T("Enter"),NULL,false){
}

EnterBnt::~EnterBnt(){};
#define DRAW_LINE_H(x,y,l)   \
	{\
	::MoveToEx(hDC,x,y,NULL); \
	::LineTo(hDC,x+l,y);   \
}
#define DRAW_LINE_V(x,y,l)   \
	{\
	::MoveToEx(hDC,x,y,NULL); \
	::LineTo(hDC,x,y+l);   \
}
	
void EnterBnt::Draw(HDC hDC, ePipeline* Pipe){
    
	RECT rc = GetArea();

	//DeflateRect(&rc,1,1,1,1);
	
	COLORREF crDraw = RGB(100,100,100);;
	if (m_State & SPACE_SELECTED)
	{
		crDraw = RGB(0,0x80,0);
		FillRectGlass(hDC,rc,RGB(0xb0,0xc4,0xde),FALSE);
		//Draw3DEdge(hDC,rc);
		::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
	}else if(m_State & SPACE_FOCUSED){
		crDraw = RGB(0,0,0);
		FillRectGlass(hDC,rc,RGB(0x36,0x6c,0xa4),FALSE);  
	}else{
        FillRectGlass(hDC,rc,RGB(0xb0,0xc4,0xde),FALSE);
	}

	HPEN   hPen   = ::CreatePen(PS_SOLID,1,crDraw);
	HPEN   hOldPen   = (HPEN)::SelectObject(hDC,hPen);	
   
	POINT pt;		
	pt.x = rc.left+ 10;
	pt.y = rc.top+RectHeight(rc)/2+1;
	
	/*	           |   
	   绘制 <------
    */

	//左箭头
	DRAW_LINE_V(pt.x,pt.y,1);
	DRAW_LINE_V(pt.x+1,pt.y-1,3);
	DRAW_LINE_V(pt.x+2,pt.y-2,5);
	DRAW_LINE_V(pt.x+3,pt.y-3,7);
	DRAW_LINE_V(pt.x+4,pt.y-4,9);

	//箭杆
	DRAW_LINE_H(pt.x+5,pt.y-1,15);
	DRAW_LINE_H(pt.x+5,pt.y,15);
	DRAW_LINE_H(pt.x+5,pt.y+1,15);

	//箭尾
	DRAW_LINE_V(pt.x+20,pt.y-6,8);
	DRAW_LINE_V(pt.x+21,pt.y-6,8);
	DRAW_LINE_V(pt.x+22,pt.y-6,8);

	::SelectObject(hDC,hOldPen);	
	::DeleteObject(hPen);
}

ConvRightToolbar::ConvRightToolbar(HANDLE Image){
	m_Image = Image;
	
    ConvProgressStruct* Progress = new ConvProgressStruct();
	SendBnt* BntSend = new SendBnt(BNT_SEND);
	EnterBnt* BntEnter = new EnterBnt(BNT_ENTER);
	
	PushChild(Progress);
	PushChild(BntSend);
	PushChild(BntEnter);
}

ConvRightToolbar::~ConvRightToolbar(){};

void ConvRightToolbar::Layout(bool Redraw){
	
	RECT rc = GetArea();
	
	ConvProgressStruct* ProgressBar = (ConvProgressStruct*)m_ChildList[0];
	SendBnt* BntSend = (SendBnt*)m_ChildList[1];
    EnterBnt* BntEnter = (EnterBnt*)m_ChildList[2];
	
	rc.top = rc.bottom-15;
    BntEnter->SetArea(rc);
	
	rc.bottom =rc.top+1;
    rc.top = rc.bottom-50;
    BntSend->SetArea(rc);
	//	::InflateRect(&BntSend->m_Area,-2,-2);
    
    rc.bottom = rc.top -5;
	rc.top = m_AreaTop+5;
	rc.left  = m_AreaLeft +2;
	rc.right = m_AreaRight-2;
	ProgressBar->SetArea(rc);
}

void ConvRightToolbar::Draw(HDC hDC, ePipeline* Pipe){
    FillTile(hDC,m_Image,GetArea());
	CVSpace2::Draw(hDC);
	RECT rc = GetArea();
	rc.right = rc.left+1;
	FillRect(hDC,rc,RGB(192,192,192));
	rc.right = m_AreaRight;
	rc.left  = rc.right-1;
	FillRect(hDC,rc,RGB(192,192,192));
}


IOViewBntA::IOViewBntA(int ID,tstring Name,int32 Width /*= 30*/)
:ButtonItem(ID,Name.c_str(),NULL,false){
	m_SizeX = Width;
}

IOViewBntA::~IOViewBntA(){};

void IOViewBntA::Draw(HDC hDC, ePipeline* Pipe){
    
	RECT rc = GetArea();
	
	//	DeflateRect(rc,1,1,1,1);
	
	HFONT oldfont = (HFONT)::SelectObject(hDC,SS.m_SystemFont);
	
	if (m_State & SPACE_SELECTED)
	{
		FillRectGlass(hDC,rc,RGB(0x36,0x6c,0xa4),FALSE);    
		COLORREF crOld = ::SetTextColor(hDC,RGB(255,255,255));
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        ::SetTextColor(hDC,crOld);
	}else if(m_State & SPACE_WARNING){
		FillRectGlass(hDC,rc,RGB(0x83,0xb5,0),FALSE);
		COLORREF crOld = ::SetTextColor(hDC,RGB(255,255,255));
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        ::SetTextColor(hDC,crOld);        
	}else{
		COLORREF crText = RGB(100,100,100);
		if(m_State & SPACE_FOCUSED){
			FillRectGlass(hDC,rc,RGB(0xb0,0xc4,0xde),FALSE);
			crText = RGB(0,0,0);
		}
		COLORREF crOld = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        ::SetTextColor(hDC,crOld);
	}
	::SelectObject(hDC,oldfont);
	
}

IOViewLockBnt::IOViewLockBnt(int ID,int32 Width /* = 10 */)
:IOViewBntA(ID,_T(""),Width)
{
};
IOViewLockBnt::~IOViewLockBnt(){};

void IOViewLockBnt::Draw(HDC hDC, ePipeline* Pipe){
    
	if (m_State & SPACE_SELECTED)
	{
		::DrawIconEx(hDC,m_AreaLeft,m_AreaTop+1,(HICON )SS.m_imgViewLockClose,16,16,FALSE,NULL,DI_NORMAL);
		::DrawEdge(hDC,&GetArea(),BDR_SUNKENINNER,BF_TOPLEFT|BF_BOTTOMRIGHT);
	}else if(m_State & SPACE_FOCUSED){
		
		::DrawIconEx(hDC,m_AreaLeft-1,m_AreaTop,(HICON )SS.m_imgViewLockOpen,16,16,FALSE,NULL,DI_NORMAL);
	}
	else{
		::DrawIconEx(hDC,m_AreaLeft,m_AreaTop+1,(HICON )SS.m_imgViewLockOpen,16,16,FALSE,NULL,DI_NORMAL);
		
	}
}

IOViewToolbarA::IOViewToolbarA(HANDLE Image)
:STDToolbar(Image),
m_BntDialog(BNT_DIALOG,_T("Dialog"),50),
m_BntOutput(BNT_OUTPUT,_T("Output"),50),
m_BntMemory(BNT_MEMORY,_T("Memory"),60),
m_BntDebug(BNT_DEBUG,_T("Debug"),50),
m_BntLogic(BNT_LOGIC,_T("Logic"),50),
m_BntObject(BNT_OBJECT,_T("Object"),50),
m_BntFind(BNT_FIND,_T("Find"),50)
{
	m_Alias = ID_SPLITTER;
/*    
	SeparatorLine* Drag = new SeparatorLine(ID_SPLITTER,_T("Separator"),5);
	Drag->m_SizeX = 4;
    Drag->m_crDark  = RGB(128,128,128);
	Drag->m_crLight = RGB(192,192,192);
	PushChild(Drag);
*/	
	//	IOViewLockBnt* LockView   = new IOViewLockBnt(BNT_VIEWLOCK,10);
	
	m_SizeX = 300;
	
	//AddBnt(LockView);	
	AddBnt(&m_BntDialog);
	AddBnt(&m_BntOutput);
	AddBnt(&m_BntMemory);
	AddBnt(&m_BntLogic);
	AddBnt(&m_BntObject);
	AddBnt(&m_BntFind);
}

IOViewToolbarA::~IOViewToolbarA(){
    m_ChildList.clear();
};

void IOViewToolbarA::EnableBnt(int64 ID,BOOL bEnable){
	CVSpace2* bnt = FindSpace(ID);
	if(bnt){
		if(bEnable){
			bnt->m_State &= ~SPACE_SELECTED;
		}else{
			bnt->m_State = SPACE_SELECTED;
		}

	}
}


IOViewToolbarB::IOViewToolbarB(HANDLE Image)
:STDToolbar(Image)
{
	m_Alias = ID_SPLITTER;

	ButtonItem* Run = new ButtonItem(BNT_RUN,_T("Run"),NULL,false,SS.m_imgRun);
	Run->m_SizeX = 50;
	PushChild(Run);
	ButtonItem* Pause = new ButtonItem(BNT_PAUSE,_T("Pause"),NULL,false,SS.m_imgPause);
	Pause->m_SizeX = 50;
	PushChild(Pause);
	ButtonItem* Stop = new ButtonItem(BNT_STOP,_T("Stop"),NULL,false,SS.m_imgStop);
	Stop->m_SizeX = 50;
	PushChild(Stop);
	
	m_SizeX = 160;

}

IOViewToolbarB::~IOViewToolbarB(){
};

void IOViewToolbarB::SetState(BOOL bRun,BOOL bPause,BOOL bStop){
	ButtonItem* Run =   (ButtonItem*)m_ChildList[0];
	ButtonItem* Pause = (ButtonItem*)m_ChildList[1];
	ButtonItem* Stop =  (ButtonItem*)m_ChildList[2];

	if (bRun)
	{
		Run->m_State = 0;
	}else{
		Run->m_State = SPACE_DISABLE;
	}

	if (bPause)
	{
		Pause->m_State = 0;
	}else{
		Pause->m_State = SPACE_DISABLE;
	}

	if (bStop)
	{
		Stop->m_State = 0;
	}else{
		Stop->m_State = SPACE_DISABLE;
	}
}


IOViewToolbarC::IOViewToolbarC(HANDLE Image)
:STDToolbar(Image)
{
	m_Alias = ID_SPLITTER;
    /*
	SeparatorLine* Drag = new SeparatorLine(ID_SPLITTER,_T("Separator"),5);
	Drag->m_SizeX = 5;
    Drag->m_crDark  = RGB(128,128,128);
	Drag->m_crLight = RGB(192,192,192);
	PushChild(Drag);
	*/
	IOViewBntA* BntRun     = new IOViewBntA(BNT_RUN,_T("Run"),50);
    IOViewBntA* BntPause   = new IOViewBntA(BNT_PAUSE,_T("Pause"),50);
	IOViewBntA* BntStop   = new IOViewBntA(BNT_PAUSE,_T("Stop"),50);

	m_SizeX = 144;
	AddBnt(BntRun);
	AddBnt(BntPause);
	AddBnt(BntStop);
}

IOViewToolbarC::~IOViewToolbarC(){};

	
IOViewSplitterItem::IOViewSplitterItem(int64 ID, HANDLE Image): 
	  SplitterItem(ID,Image){

	  IOViewToolbarA* ToolbarA  = new IOViewToolbarA(NULL);
	  PushChild(ToolbarA);

	  IOViewToolbarB* ToolbarB  = new IOViewToolbarB(NULL);
	  PushChild(ToolbarB);
};

IOViewSplitterItem::~IOViewSplitterItem(){

};


CVSpace2*  IOViewSplitterItem::HitTest(int32 x,int32 y){
	CVSpace2* Space = CVSpace2::HitTest(x,y);
	if(Space && Space->m_Alias == ID_SPLITTER)return this;
	return Space;
};

void IOViewSplitterItem::Layout(bool Redraw /*= true*/){
   RECT rc = GetArea();
   ::InflateRect(&rc,-1,-1);

   int w =0;
   int i;
   for ( i=0; i<m_ChildList.size();i++)
   {
       CVSpace2* v = m_ChildList[i];
	   w += v->m_SizeX;
   }

   int x = RectWidth(rc);
   for (i=0; i<m_ChildList.size();i++)
   {
       CVSpace2* v = m_ChildList[i];
	   int l =  x* v->m_SizeX/w;
	   rc.right = rc.left +l;
	   v->SetArea(rc);
	   v->Layout(false);
	   rc.left = rc.right+2;
   }
}

void IOViewSplitterItem::Draw(HDC hDC, ePipeline* Pipe){
	if (m_imagSkin)
	{
		FillTile(hDC,m_imagSkin,GetArea());
	}
	Layout(false);
	CVSpace2::Draw(hDC,Pipe);
/*
	RECT rc = GetArea();
	rc.top = rc.bottom-1;
	FillRect(hDC,&rc,RGB(192,192,192));
*/
}


SplitterItem* IOViewSplitterStruct::CreateSplitterItem(){
		return new IOViewSplitterItem(ID_SPLITTER,m_ImageSkin);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConversation::CConversation()
:m_Splitter(this,24,20,false),
 m_Created(FALSE),
 m_CurTaskState(TASK_DELELTE),
 m_SpaceSelected(NULL),
 m_PopMenu(NULL)
{

	m_Splitter.m_ImageSkin = SS.m_imgConvToolSkin;

    PushChild(&m_Splitter);	
	m_State &= ~SPACE_SHOWWINBORDER;	
}

CConversation::~CConversation()
{
	m_ChildList[0] = NULL; //avoid delete m_Splitter
	::DestroyMenu(m_PopMenu);
}

STDToolbar* CConversation::GetToolbar(int index){
	IOViewSplitterItem* sl = (IOViewSplitterItem*)m_Splitter.m_ChildList[0];
	assert(index>=0 && index<sl->m_ChildList.size());
	STDToolbar* toolbar = (STDToolbar*)sl->m_ChildList[index];
	return toolbar;
}

void CConversation::SetTaskToolbarState(BOOL bRun,BOOL bPause,BOOL bStop){
	IOViewSplitterItem* sl = (IOViewSplitterItem*)m_Splitter.m_ChildList[0];

	IOViewToolbarB* Toolbar = (IOViewToolbarB*)sl->m_ChildList[1];
    Toolbar->SetState(bRun,bPause,bStop);
	Invalidate();
}


void CConversation::ShowView(int32 ViewID /*=DIALOG_VIEW*/,BOOL bShow ){

	if (bShow)
	{
		STDToolbar* Toolbar = GetToolbar(0);
		ButtonItem* Item = Toolbar->GetBnt(ViewID);
		if (Item == NULL)
		{
			return;
		}
		if(Item->m_State & SPACE_SELECTED){ //已经打开不需要再打开
			return;
		}
		SetSpaceWarning(Item,FALSE);
	    Item->m_State |= SPACE_SELECTED;
		Invalidate();
		m_OutputWin.ShowView(ViewID,TRUE);
	} 
	else
	{
		m_OutputWin.ShowView(ViewID,FALSE);
		
		STDToolbar* Toolbar = GetToolbar(0);
		ButtonItem* Item = Toolbar->GetBnt(ViewID);
		if (Item == NULL)
		{
			return;
		}

		if(Item && Item->m_State & SPACE_SELECTED){ //已打开才需要关闭
			Item->m_State &= ~SPACE_SELECTED;
			Invalidate();
		}
	}
}


void CConversation::AddDialogItem(ePipeline& Item){
	m_OutputWin.m_DialogView.AddDialogItem(Item);
	
	if (!IsViewOpened(DIALOG_VIEW))
	{
		ViewWarning(DIALOG_VIEW);
	}
}

void CConversation::AddMoreDialogItem(int64 SourceID,int64 DialogID,ePipeline& Pipe){
	if (GetGUI()->IsFocusDialog(SourceID,DialogID))
    {
		m_OutputWin.m_DialogView.AddMoreDialogItem(Pipe);
	}
};

void CConversation::ClearDialogInfo(){
	m_OutputWin.m_DialogView.ClearAllItem();
}


void CConversation::AddRuntimeInfo(ePipeline& Item){

	tstring timeStr = Item.GetLabel();
	
	int64 MassID = Item.GetID();
	tstring text = Item.PopString();
	
	tstring s = Format1024(_T("%s  %I64ld \n%s"),timeStr.c_str(),MassID,text.c_str());
	m_OutputWin.m_OutputView.AddText(s.c_str());

	::SetFocus(m_InputWin.m_Edit.GetHandle());
	if (!IsViewOpened(OUTPUT_VIEW))
	{
		ViewWarning(OUTPUT_VIEW);
	}
}

void CConversation::ClearRuntimeInfo(){
	m_OutputWin.m_OutputView.ClearText();
}

void CConversation::AddForecastInfo(tstring& s){
	
}

void CConversation::ClearForecastInfo(){

};

void CConversation::SetInputTip(tstring tip){
	m_InputWin.m_Edit.SetInputTip(tip);
}


bool CConversation::IsViewOpened(int64 ViewID){
	STDToolbar* tool = GetToolbar(0);
	CVSpace2* bnt = tool->FindSpace(ViewID);
	if(bnt==NULL)return false;
	if (bnt->m_State & SPACE_SELECTED)
	{
		return true;
	}
	return false;
}

void CConversation::ViewWarning(int64 ViewID){
	STDToolbar* tool = GetToolbar(0);
	CVSpace2* bnt = tool->FindSpace(ViewID);
	if(bnt==NULL)return ;
	if(IsWarning(bnt)){
		return ;
	}
	SetSpaceWarning(bnt,TRUE);
	SetTimer(GetHwnd(),ViewID,2500,NULL); //只闪烁3次
};

void CConversation::DebugViewProc(ePipeline& Info){

	m_OutputWin.m_DebugView.InfoProc(Info);

	if (Info.Size())
	{
		if (!IsViewOpened(DEBUG_VIEW))
		{
			ViewWarning(DEBUG_VIEW);
		}
	}
};

void CConversation::SetThinkView(ePipeline& ItemList){

	
	/*
	m_OutputWin.m_SceneView.Reset(ItemList);
	
	if (ItemList.Size())
	{
		STDToolbar* tool = GetToolbar(0);
	
		CVSpace2* bnt = tool->FindSpace(BNT_THINK);	
	  
		if (!(bnt->m_State&SPACE_SELECTED))
		{
			bnt->m_State |= SPACE_SELECTED;			
			m_OutputWin.ShowView(BNT_THINK,TRUE);
		} 
	}

	Invalidate();
	*/
};

void CConversation::MemoryViewProc(ePipeline& Info){
	if (Info.Size()==0)return;

    m_OutputWin.m_MemoryView.InfoProc(Info);

	if (!IsViewOpened(MEMORY_VIEW))
	{
		ViewWarning(MEMORY_VIEW);
	}
}

void CConversation::SetLogicView(ePipeline& Item){
	if (Item.Size())
	{
		m_OutputWin.m_LogicView.LogicElementProc(&Item);
	
		if (!IsViewOpened(LOGIC_VIEW))
		{
			ViewWarning(LOGIC_VIEW);
		}
	}
}

void CConversation::SetObjectView(ePipeline& Item){
	if (Item.Size())
	{
		m_OutputWin.m_ObjectView.ObjectProc(&Item);
		
		if (!IsViewOpened(OBJECT_VIEW))
		{
			ViewWarning(OBJECT_VIEW);
		}
	}
}



void CConversation::SetCurDialog(int64 SourceID,int64 DialogID,ePipeline& Pipe){

	if (Pipe.Size()==0)
	{
		return;
	}

    //更新对话历史
	ePipeline* DialogData = (ePipeline*)Pipe.GetData(0);
	m_OutputWin.m_DialogView.SetDialogList(*DialogData);

	//更新运行时输出
	ePipeline* RuntimeData = (ePipeline*)Pipe.GetData(1);
    m_OutputWin.m_OutputView.ClearText();

	//第一个数据表明还有多少没有显示
	ePipeline* Item0 = (ePipeline*)RuntimeData->GetData(0);
	int32 n = Item0->PopInt(); 

	for (int i=1; i<RuntimeData->Size(); i++)
	{
		ePipeline* Item = (ePipeline*)RuntimeData->GetData(i);
		
		tstring s = Item->PopString();
		m_OutputWin.m_OutputView.AddText(s.c_str());
	}
	
    //更新临时逻辑列表
    ePipeline* LogicData = (ePipeline*)Pipe.GetData(2);
    
	//更新临时capa列表
    ePipeline* CapaData = (ePipeline*)Pipe.GetData(3);

	//更新临时indu列表
    ePipeline* InduData = (ePipeline*)Pipe.GetData(4);
   
	//更新临时物体列表
	ePipeline* ObjectData = (ePipeline*)Pipe.GetData(5);

	//更新临时数据
	ePipeline* TableData = (ePipeline*)Pipe.GetData(6);


    ePipeline List;
	List.PushPipe(*LogicData);
	List.PushPipe(*CapaData);
	List.PushPipe(*InduData);

	m_OutputWin.m_LogicView.ResetLocalElement(List);
	m_OutputWin.m_ObjectView.ResetLocalObject(*ObjectData);
	m_OutputWin.m_MemoryView.ResetData(*TableData);

    //输入窗口状态
    int64 bEditValid = *(int64*)Pipe.GetData(7);
	tstring EditText = *(tstring*)Pipe.GetData(8);	
	
	m_InputWin.ContinueEdit(EditText.c_str());
    m_InputWin.EnableInput(bEditValid);

	//状态条文字
	tstring StatusText = *(tstring*)Pipe.GetData(9);
    GetGUI()->m_Status.SetTip(StatusText);

	//工作模式
    int64 WorkMode = *(int64*)Pipe.GetData(10);
	switch(WorkMode){
	case WORK_TASK:
		SetInputTip(_T("Cmd>"));
		break;
	case WORK_THINK:
		SetInputTip(_T("Think>"));
		break;
	case WORK_DEBUG:
		SetInputTip(_T("Debug>"));
		break;
	case WORK_CHAT:
		SetInputTip(_T("Chat>"));
		break;
	case WORK_LEARN:
		SetInputTip(_T("Learn>"));
		break;
	case WORK_TEST:
		SetInputTip(_T("Test"));
		break;

	}

	//任务状态
    int64 TaskState = *(int64*)Pipe.GetData(11); 

	if (TaskState == TASK_PAUSE)
	{		
		int64 bIncludeDebugInfo = *(int64*)Pipe.GetData(12);
		if (bIncludeDebugInfo)
		{
			ePipeline* DebugItemTree = (ePipeline*)Pipe.GetData(13);
			
			int64 DebugTimeStamp = *(int64*)Pipe.GetData(14);
			ePipeline* PauseList = (ePipeline*)Pipe.GetData(15);
		
			ePipeline Cmd;
			Cmd.PushInt(DEBUG_RESET);
			DebugItemTree->SetID(DebugTimeStamp);
			Cmd.PushPipe(*DebugItemTree);
			Cmd.PushPipe(*PauseList);
			DebugViewProc(Cmd);

		}
	};

	SetCurTaskState((TASK_STATE)TaskState);

	//处理搜索数据
	ePipeline* SearchResult = (ePipeline*)Pipe.GetLastData();
	if (SearchResult->Size()==0)
	{
		m_OutputWin.m_FindView.ClearAll();
		return;
	};

	ePipeline Cmd;
	Cmd.PushInt(FIND_UPDATE);
	Cmd.PushPipe(*SearchResult);
	FindViewProc(Cmd);
	
	Cmd.Clear();
    if (SearchResult->GetID())
    {
		Cmd.PushInt(FIND_STOP);
		Cmd.PushInt(FALSE);
    }else{
		Cmd.PushInt(FIND_STOP);
		Cmd.PushInt(TRUE);
	}
	FindViewProc(Cmd);
	
	Pipe.Clear();
}

void  CConversation::SetCurTaskState(TASK_STATE State)
{

	if (State == m_CurTaskState )
	{
		return;
	}

	m_CurTaskState = State;

	//根据新状态改变工具条布局,关掉不在布局中的窗口
	IOViewToolbarA* Toolbar1 = (IOViewToolbarA*)GetToolbar(0);
	
    if(State == TASK_PAUSE){
		/*ShowView(LOGIC_VIEW,FALSE);
		ShowView(OBJECT_VIEW,FALSE);
		ShowView(FIND_VIEW,FALSE);

		Toolbar1->m_ChildList.clear();
		Toolbar1->m_VisibleBntList.clear();

		Toolbar1->AddBnt(&Toolbar1->m_BntDialog);
		Toolbar1->AddBnt(&Toolbar1->m_BntOutput);
		Toolbar1->AddBnt(&Toolbar1->m_BntMemory);*/

		Toolbar1->AddBnt(&Toolbar1->m_BntDebug);

	}
	else{
		ShowView(DEBUG_VIEW,FALSE);

		Toolbar1->m_ChildList.clear();
		Toolbar1->m_VisibleBntList.clear();
		
		Toolbar1->AddBnt(&Toolbar1->m_BntDialog);
		Toolbar1->AddBnt(&Toolbar1->m_BntOutput);
		Toolbar1->AddBnt(&Toolbar1->m_BntMemory);
		Toolbar1->AddBnt(&Toolbar1->m_BntLogic);
		Toolbar1->AddBnt(&Toolbar1->m_BntObject);
		Toolbar1->AddBnt(&Toolbar1->m_BntFind);
	}
	
	//缺省打开必要的窗口
	switch(State){
	case TASK_STOP:
		m_InputWin.ContinueEdit(NULL);
		ShowView(DIALOG_VIEW,TRUE);
		ShowView(OUTPUT_VIEW,TRUE);
		SetTaskToolbarState(FALSE,FALSE,FALSE);
		GetGUI()->m_Status.LightLamp(IN_LAMP,FALSE);
		break;
	case TASK_COMPILE:
		ClearRuntimeInfo();
		break;
	case TASK_COMPILE_ERROR:
		break;
	case TASK_RUN:
	case TASK_WAIT:
		ShowView(DIALOG_VIEW,TRUE);
		ShowView(OUTPUT_VIEW,TRUE);
		SetTaskToolbarState(FALSE,TRUE,TRUE);
		break;
	case TASK_PAUSE:
		ShowView(DIALOG_VIEW,FALSE);
		ShowView(OUTPUT_VIEW,TRUE);
		ShowView(DEBUG_VIEW,TRUE);

		SetTaskToolbarState(TRUE,FALSE,TRUE);
		break;
	case TASK_DELELTE:
		SetTaskToolbarState(FALSE,FALSE,FALSE);
	}
	
};

void CConversation::Layout(bool Redraw /*= true*/){
	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);
	
    //if(::EqualRect(&m_Splitter.m_Area,&rcClient))return;
    if(m_Created){	
		m_Splitter.Layout(rcClient);
	}
};


LRESULT CConversation::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	case WM_TIMER:
		return OnTimer(wParam,lParam);
	case WM_SIZE:
		return OnSize(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_COMMAND:
		return OnCommand(wParam,lParam);
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	default:
		return ::DefWindowProc(m_hWnd,message,wParam,lParam);
	}
};

LRESULT CConversation::OnPaint(WPARAM wParam, LPARAM lParam) 
{
	RECT rcViewport; 
	GetClientRect(m_hWnd,&rcViewport);

	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(rcViewport.right==rcViewport.left || rcViewport.top==rcViewport.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}
	
	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcViewport),RectHeight(rcViewport));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcViewport,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT );
    Draw(DCMem);
	
	::SetBkMode(DCMem, OldMode );

	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rcViewport,m_crWinBorder);// Border
	}	
	::BitBlt(hdc, 0, 0, RectWidth(rcViewport), RectHeight(rcViewport), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);

	return 0;
}
LRESULT CConversation::OnSize(WPARAM wParam, LPARAM lParam) 
{	
	if(m_Created){
		Layout();
	}
	return 0;
}

LRESULT CConversation::OnCreate( WPARAM wParam, LPARAM lParam) 
{		

	m_Created = true;
	m_Splitter.AddView(&m_OutputWin,false,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN);
	m_Splitter.AddView(&m_InputWin,false,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN);
	//OpenView();
	//Layout();
	//SetTaskToolbarState(FALSE,FALSE,FALSE);
	SetCurTaskState(TASK_STOP);
	SetInputTip(_T("Cmd>"));
    return 0;
}


LRESULT CConversation::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS p = MAKEPOINTS(lParam);

	POINT point;
	point.x = p.x;
	point.y = p.y;	
	m_SpaceSelected = Hit(point);
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){	
           m_Splitter.OnLButtonDown((SplitterItem*)m_SpaceSelected);
		}else{
			STDToolbar* Tool1 = GetToolbar(0);
			STDToolbar* Tool2 = GetToolbar(1);
			if (m_SpaceSelected->m_Parent == Tool1)
			{
				if (m_SpaceSelected->m_Alias == BNT_OTHER)
				{
					OtherBnt* Bnt = (OtherBnt*)m_SpaceSelected;
					STDToolbar* Toolbar = (STDToolbar*)(Bnt->m_Parent);
					assert(Toolbar);
					vector<CVSpace2*> InvisibleList;
					Toolbar->GetInvisibleBnt(InvisibleList);
					
					POINT pos;
					::GetCursorPos(&pos);
					
					::DestroyMenu(m_PopMenu);

					m_PopMenu = ::CreatePopupMenu();		
					::SetForegroundWindow(GetHwnd());
					
					int n=0;
					for (int i=0; i<InvisibleList.size(); i++)
					{
						CVSpace2* Space = InvisibleList[i];
						if(Bnt->m_Alias == ID_SPLITTER)continue;
						ButtonItem* Bnt = (ButtonItem*)Space;
				
						::InsertMenu(m_PopMenu,n++, MF_BYPOSITION , Bnt->m_Alias,Bnt->m_Name.c_str());
						if (Bnt->m_State & SPACE_SELECTED)
						{
							::CheckMenuItem(m_PopMenu,Bnt->m_Alias ,MF_CHECKED);					
						}
					}
					
					::TrackPopupMenu(m_PopMenu,TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
						pos.x, pos.y,0, GetHwnd(),NULL);	
					::PostMessage(GetHwnd(),WM_NULL, 0, 0);			
				}

				else if (m_SpaceSelected->m_State&SPACE_SELECTED)
				{
					m_SpaceSelected->m_State &= ~SPACE_SELECTED;
					m_OutputWin.ShowView(m_SpaceSelected->m_Alias,FALSE);
					
					if (!m_OutputWin.HasView())
					{
						ShowView(DIALOG_VIEW,TRUE);
					}
				}else{

					ShowView(m_SpaceSelected->m_Alias,TRUE);
					m_SpaceSelected->m_State |= SPACE_SELECTED;
				}
			} 
			else if(m_SpaceSelected->m_Parent == Tool2) //任务控制
			{
				if (m_SpaceSelected->m_Alias == BNT_OTHER)
				{
					OtherBnt* Bnt = (OtherBnt*)m_SpaceSelected;
					STDToolbar* Toolbar = (STDToolbar*)(Bnt->m_Parent);
					assert(Toolbar);
					vector<CVSpace2*> InvisibleList;
					Toolbar->GetInvisibleBnt(InvisibleList);
					
					POINT pos;
					::GetCursorPos(&pos);
					
					::DestroyMenu(m_PopMenu);
					
					m_PopMenu = ::CreatePopupMenu();		
					::SetForegroundWindow(GetHwnd());
					
					int n=0;
					for (int i=0; i<InvisibleList.size(); i++)
					{
						CVSpace2* Space = InvisibleList[i];
						if(Bnt->m_Alias == ID_SPLITTER)continue;
						ButtonItem* Bnt = (ButtonItem*)Space;
						
						::InsertMenu(m_PopMenu,n++, MF_BYPOSITION , Bnt->m_Alias,Bnt->m_Name.c_str());
						if (Bnt->m_State & SPACE_DISABLE)
						{
							EnableMenuItem(m_PopMenu,Bnt->m_Alias,MF_DISABLED);	
						}
					}
					
					::TrackPopupMenu(m_PopMenu,TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
						pos.x, pos.y,0, GetHwnd(),NULL);	
					::PostMessage(GetHwnd(),WM_NULL, 0, 0);			
				}
				else if (m_SpaceSelected->m_State & SPACE_SELECTED)
				{
					return 0;
				}else if(!(m_SpaceSelected->m_State & SPACE_DISABLE)){

				//	m_SpaceSelected->m_State |= SPACE_SELECTED;
					
					ePipeline Msg(TO_BRAIN_MSG::TASK_CONTROL::ID);
					Msg.PushInt(m_SpaceSelected->m_Alias);
					Msg.PushInt(0);
					GetGUI()->SendMsgToBrainFocuse(Msg);
				}
			}

			Invalidate();
		}
	}
	return 0;
}

LRESULT CConversation::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{
	
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER) {
			m_Splitter.OnLButtonUp((SplitterItem*)m_SpaceSelected);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
            m_SpaceSelected = NULL; 	
		}
	}
	return 0;
}

LRESULT CConversation::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{

	POINTS point = MAKEPOINTS(lParam);
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){
            m_Splitter.OnMouseMove(point.x,point.y,(SplitterItem*)m_SpaceSelected);
		}
	}		
	return 0;
}

LRESULT CConversation::OnTimer(WPARAM wParam, LPARAM lParam){
	long nIDEvent = wParam;
	if(nIDEvent < 20){ 
	    int64 ViewID = nIDEvent;
		STDToolbar* Toolbar = GetToolbar(0);
		ButtonItem* Item = Toolbar->GetBnt(ViewID);
		if (Item == NULL)
		{
			return 0;
		}
		SetSpaceWarning(Item,FALSE);  //取消警告
		::KillTimer(m_hWnd,nIDEvent);
	}
    return ::DefWindowProc(m_hWnd,WM_TIMER,wParam,lParam);
}


LRESULT CConversation::ParentReaction(SpaceRectionMsg* SRM){

	SendParentMessage(SRM->Msg,SRM->wParam,0,NULL);
	
	return 0;
}


void   CConversation::FindViewProc(ePipeline& Info){
    int64 Cmd = Info.PopInt();
	switch(Cmd)
	{
	case FIND_START:
		{
			ShowView(FIND_VIEW,TRUE);
			//tstring Title;
			//Pipe.PopString(Title);
			//tstring Name = tformat(_T("<-> Find \"%s\""),Title.c_str());
			//m_IOView.m_OutputWin.m_FindView.SetTitle(Name.c_str());
			m_OutputWin.m_FindView.ClearAll();	
		}
		break;
	case FIND_UPDATE:
		{
			
			ePipeline& SearchResult = *(ePipeline*)Info.GetData(0);
			if (!IsViewOpened(FIND_VIEW))
			{
					ViewWarning(FIND_VIEW);
			}

			if(SearchResult.Size()==0){
				m_OutputWin.m_FindView.ClearAll();	
				tstring Tip = _T("Find 0 result");
				m_OutputWin.m_FindView.SetFindTip(Tip);
				return;
			}else{
				tstring Tip = _T("");
				m_OutputWin.m_FindView.SetFindTip(Tip);
			}

			for (int i=0; i<SearchResult.Size(); i++)
			{
				ePipeline& Item  = *(ePipeline*)SearchResult.GetData(i);
				int64 Type = Item.PopInt();
				switch(Type){
				case LOGIC_RESULT:
					{
						int64 n = Item.PopInt();
						int64 SpaceID = Item.PopInt();
						tstring Text;
						Item.PopString(Text);
						tstring Memo;
						Item.PopString(Memo);
						m_OutputWin.m_FindView.AddCommandOrLogic(n,SpaceID,Text.c_str(),Memo.c_str());
					}
					break;
				case OBJECT_RESULT:
					{
						int64 n = Item.PopInt();
						int64 SpaceID = Item.PopInt();
						tstring Text;
						Item.PopString(Text);
						tstring Memo;
						Item.PopString(Memo);
						m_OutputWin.m_FindView.AddObjectOrPeople(n, SpaceID,Text.c_str(),Memo.c_str());
					}
					break;
				case TEXT_RESULT:
					{
						int64 n = Item.PopInt();
						int64 SpaceID = Item.PopInt();
						tstring Text;
						Item.PopString(Text);
						m_OutputWin.m_FindView.AddText(n,SpaceID,Text.c_str());
					}
					break;
				default:
					assert(0);
					break;
				}
			}
		}
		break;

	case FIND_STOP:
		{
			int64 IsAllEnd = Info.PopInt();
			m_OutputWin.m_FindView.FindEnd(IsAllEnd);
		}
		break;
	}
}
    
LRESULT CConversation::OnCommand(WPARAM wParam, LPARAM lParam){
    int ID = LOWORD(wParam);
	int NotifCode = HIWORD(wParam);

	if (ID>=BNT_DIALOG && ID<=BNT_FIND)
	{
		STDToolbar* Tool1 = GetToolbar(0);
		CVSpace2* Bnt = Tool1->FindSpace(ID);
		if (Bnt->m_State & SPACE_SELECTED)
		{
			ShowView(ID,FALSE);
		} 
		else
		{
			ShowView(ID,TRUE);
		}
	}else if (ID>=BNT_RUN && ID<=BNT_STOP)
	{
        STDToolbar* Tool2 = GetToolbar(1);
		CVSpace2* Bnt = Tool2->FindSpace(ID);
		if(!(Bnt->m_State & SPACE_DISABLE)){
			
			m_SpaceSelected->m_State |= SPACE_SELECTED;
			
			ePipeline Msg(TO_BRAIN_MSG::TASK_CONTROL::ID);
			Msg.PushInt(m_SpaceSelected->m_Alias);
			Msg.PushInt(0);
			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
	}
	return 0;
}
