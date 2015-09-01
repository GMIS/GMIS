// ThreadDebug.cpp: implementation of the CThreadDebug class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "GMIS.h"
#include "DebugView.h"
#include "BrainMemory.h"
#include "SystemSetting.h"
#include "MainFrame.h"
#include "Conversation.h"
#include <CommDlg.h>


BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName ) 
{ 
	HDC hDC; //设备描述表 
	int iBits; //当前显示分辨率下每个像素所占字节数 
	WORD wBitCount; //位图中每个像素所占字节数 
	DWORD dwPaletteSize=0, //定义调色板大小， 位图中像素字节大小 ，位图文件大小 ， 写入文件字节数 
		dwBmBitsSize, 
		dwDIBSize, dwWritten; 
	BITMAP Bitmap; //位图属性结构 
	BITMAPFILEHEADER bmfHdr; //位图文件头结构 
	BITMAPINFOHEADER bi; //位图信息头结构 
	LPBITMAPINFOHEADER lpbi; //指向位图信息头结构 

	HANDLE fh, hDib, hPal,hOldPal=NULL; //定义文件，分配内存句柄，调色板句柄 

	//计算位图文件每个像素所占字节数 
	HDC hWndDC = CreateDC(_T("DISPLAY"),NULL,NULL,NULL); 
	hDC = ::CreateCompatibleDC( hWndDC ) ; 
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES); 
	DeleteDC(hDC); 

	if (iBits <= 1) 
		wBitCount = 1; 
	else if (iBits <= 4) 
		wBitCount = 4; 
	else if (iBits <= 8) 
		wBitCount = 8; 
	else if (iBits <= 24) 
		wBitCount = 24; 
	else 
		wBitCount = 24 ; 

	//计算调色板大小 
	if (wBitCount <= 8) 
		dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD); 

	//设置位图信息头结构 
	GetObjectW(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 
	bi.biSize = sizeof(BITMAPINFOHEADER); 
	bi.biWidth = Bitmap.bmWidth; 
	bi.biHeight = Bitmap.bmHeight; 
	bi.biPlanes = 1; 
	bi.biBitCount = wBitCount; 
	bi.biCompression = BI_RGB; 
	bi.biSizeImage = 0; 
	bi.biXPelsPerMeter = 0; 
	bi.biYPelsPerMeter = 0; 
	bi.biClrUsed = 0; 
	bi.biClrImportant = 0; 

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount+31)/32) * 4 * Bitmap.bmHeight ; 

	//为位图内容分配内存 
	hDib = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER)); 
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
	*lpbi = bi; 

	// 处理调色板 
	hPal = GetStockObject(DEFAULT_PALETTE); 
	if (hPal) 
	{ 
		hDC = ::GetDC(NULL); 
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE); 
		RealizePalette(hDC); 
	} 

	// 获取该调色板下新的像素值 
	GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, 
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
		+dwPaletteSize, 
		(LPBITMAPINFO ) 
		lpbi, DIB_RGB_COLORS); 

	//恢复调色板 
	if (hOldPal) 
	{ 
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE); 
		RealizePalette(hDC); 
		::ReleaseDC(NULL, hDC); 
	} 

	//创建位图文件 
	fh = CreateFileW(lpFileName, GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

	if (fh == INVALID_HANDLE_VALUE) 
		return FALSE; 

	// 设置位图文件头 
	bmfHdr.bfType = 0x4D42; // "BM" 
	dwDIBSize = sizeof(BITMAPFILEHEADER) 
		+ sizeof(BITMAPINFOHEADER) 
		+ dwPaletteSize + dwBmBitsSize; 
	bmfHdr.bfSize = dwDIBSize; 
	bmfHdr.bfReserved1 = 0; 
	bmfHdr.bfReserved2 = 0; 
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) 
		+ (DWORD)sizeof(BITMAPINFOHEADER) 
		+ dwPaletteSize; 

	// 写入位图文件头 
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 

	// 写入位图文件其余内容 
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, 
		&dwWritten, NULL); 

	//清除 
	GlobalUnlock(hDib); 
	GlobalFree(hDib); 
	CloseHandle(fh); 

	return TRUE; 

}


void CDebugView::MassItem::Init(ePipeline& Pipe){
	m_Alias = Pipe.GetID();
	m_Name = Pipe.GetLabel();
	int64 Type = Pipe.PopInt();

	m_Type   = Type;

	if (m_Type == MASS_ELEMENT)
	{
		while(Pipe.Size()){
			eElectron E;
			Pipe.Pop(&E);
			ePipeline* ChildPipe = (ePipeline*)E.Value();
			MassItem* ChildItem = new MassItem();
			PushChild(ChildItem);
			ChildItem->Init(*ChildPipe);
		}
	}
}


void CDebugView::MassItem::Draw(HDC hDC, ePipeline* Pipe){
	
	RECT rc = GetArea();
	COLORREF crFg = SS.crTaskMassText, crBk = SS.crTaskMassBk;
	
	
	if(m_State & SPACE_SELECTED){
		//	crFg = m_crBk; crBk = m_crText;
		rc.bottom -= 25;
	}
	

	//画背景	
    if(m_State & SPACE_PAUSE ){	
        FillRect(hDC,rc,SS.crTaskMassPause);
		crFg = SS.crTaskMassBk;
	}else if (m_State & SPACE_BREAK)
	{
		FillRect(hDC,rc,SS.crTaskMassBreak);
		crFg = SS.crTaskMassBk;
	}
	else{
		FillRect(hDC,rc,crBk);
	}
	
	//	rc.left += RectHeight(rc)+2;
	
	//Draw MassName
	COLORREF crOld = SetTextColor(hDC,crFg);
	tstring s = Format1024(_T("%I64ld  %s"),m_Alias,m_Name.c_str());	
	::DrawText(hDC,s.c_str(),s.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	::SetTextColor(hDC,crOld);	
	
	if(m_State & SPACE_FOCUSED || m_State & SPACE_SELECTED)
	{
		DrawEdge(hDC,GetArea(),SS.crTaskMassBorder);
	}
}

CDebugView::CDebugView(){
	m_ToobarItemSelected = NULL;
	m_HeaderWidth = 16;
	m_Padding = 4;
	m_crViewBkg = RGB(255,255,255);

	m_Toolbar.m_crToolBgk = SS.crTaskMassBk;

	m_crConnectingLine = RGB(128,128,128);
	m_bShowLine = true;
	m_SpaceSelected = NULL;
	m_LinePen = ::CreatePen(PS_SOLID, 1, m_crConnectingLine);

	ButtonItem* Step = new ButtonItem(BNT_STEP,_T("Step"),NULL,false,NULL);
	m_Toolbar.PushChild(Step);
	ButtonItem* Run = new ButtonItem(BNT_RUN,_T("Run"),NULL,false,NULL);
	m_Toolbar.PushChild(Run);
	ButtonItem* Goto = new ButtonItem(BNT_BREAK,_T("SetBreak"),NULL,false,NULL);
	m_Toolbar.PushChild(Goto);

};
	
CDebugView::~CDebugView(){
	if(m_LinePen)::DeleteObject(m_LinePen);
};
	


void  CDebugView::Item2Path(CVSpace2* Item,ePipeline& Path){
	assert(Item);
	
	Path.Push_Front(new eINT(Item->m_Alias));

	CVSpace2* Parent = Item->m_Parent;
	while (Parent != this)
	{
		Path.Push_Front(new eINT(Parent->m_Alias));
		Parent = Parent->m_Parent;
	}
}

CVSpace2* CDebugView::Path2Item(ePipeline& Path){

	CVSpace2* Parent = this;
	while(Path.Size())
	{
		bool bFind = false;
		int64 ID = Path.PopInt();
		deque<CVSpace2*>::iterator it = Parent->m_ChildList.begin();
		while(it != Parent->m_ChildList.end())
		{
			CVSpace2* Child = *it;
			if (Child->m_Alias == ID)
			{
				bFind = true;
				if (Path.Size()==0)
				{
					return Child;
				}else{
					Parent = Child;
				}
				break;
			}
			it++;
		}
		if (!bFind)
		{
			return NULL;
		}
		
	}
	return NULL;
}

void CDebugView::OpenAllTree(CVSpace2* Parent /*=NULL*/)
{
	if (Parent == NULL)
	{
		Parent = this;
	}

	deque<CVSpace2*>::iterator it = Parent->m_ChildList.begin();
	while (it != Parent->m_ChildList.end())
	{
		CVSpace2* Child = *it;
		Child->m_State |= SPACE_OPEN;
		OpenAllTree(Child);
		it++;
	}
}
void CDebugView::InfoProc(ePipeline& Info){
	SendChildMessage(m_hWnd,INFO_PROC,(int64)&Info,0);
};

void CDebugView::SetStepBnt(bool bStep){
	
	CVSpace2* Item = m_Toolbar.m_ChildList[0];
	if(bStep)Item->m_State &= ~SPACE_DISABLE;
	else Item->m_State |= SPACE_DISABLE;
}
void CDebugView::SetRunBnt(bool bRun){
	CVSpace2* Item = m_Toolbar.m_ChildList[1];
	if(bRun)Item->m_State &= ~SPACE_DISABLE;
	else Item->m_State |= SPACE_DISABLE;
}


void CDebugView::SetBreakBnt(bool bBreak,int32 BreakFlag){
	
	CVSpace2* Item = m_Toolbar.m_ChildList[2];
	if(bBreak)Item->m_State &= ~SPACE_DISABLE;
	else Item->m_State |= SPACE_DISABLE;
	
	ButtonItem* bnt = (ButtonItem*)Item;
	if (BreakFlag == 0)
	{
		return;
	}
	else if (BreakFlag == 1)
	{
		bnt->m_Name = _T("SetBreak");
		bnt->m_State |= SPACE_BREAK;
	}else if(BreakFlag == 2){
		bnt->m_Name = _T("UnBreak");
		bnt->m_State &= ~SPACE_BREAK;
	}	
}

void CDebugView::MarkPauseItem(int64 PauseItemID){

	CVSpace2* Item = FindSpace(PauseItemID);
	if (Item)
	{

		if (m_Toolbar.m_Owner)
		{
			m_Toolbar.m_Owner->m_AreaBottom-=m_Toolbar.m_Height;
			m_Toolbar.m_Owner->m_State &= ~SPACE_SELECTED;
		}

		m_Toolbar.m_Owner = Item;
        m_Toolbar.m_Owner->m_AreaBottom+=m_Toolbar.m_Height;

		Item->m_State |= SPACE_PAUSE|SPACE_SELECTED;

		m_SpaceSelected = Item;

		SetStepBnt(true);
		SetRunBnt(true);
		SetBreakBnt(false,0);

		Layout();
		EnsureVisible(Item,true);
	}else
	{
		Invalidate();
	}
	
}


void CDebugView::Draw(HDC hDC, ePipeline* Pipe){
	CWSTreeView::Draw(hDC,Pipe);

	if(m_Toolbar.m_Owner){
		m_Toolbar.Draw(hDC);
	}

};

void CDebugView::Layout(bool Redraw /* =true */){
    
    CWSTreeView::Layout(false);
   
	if (m_Toolbar.m_Owner)
	{
		CVSpace2* Item = m_Toolbar.m_Owner;
		if(!(Item->m_State & SPACE_PAUSE))
		{
			SetStepBnt(false);
			SetRunBnt(false);
			if (Item->m_State & SPACE_BREAK)
			{
				SetBreakBnt(true,2);
			} 
			else
			{
				SetBreakBnt(true,1);
			}
		}else{

			SetStepBnt(true);
			SetRunBnt(true);
			SetBreakBnt(false,0);
		}
	}
	m_Toolbar.Layout();

	if(Redraw){
		Invalidate();
	}
};



LRESULT CDebugView::ChildReaction(SpaceRectionMsg* SRM){

	switch(SRM->Msg)
	{
	case INFO_PROC:
		return OnInfoProc(SRM->wParam,SRM->lParam);
	default:
	    break;
	}
    return 0;
}

LRESULT CDebugView::OnInfoProc(WPARAM wParam, LPARAM lParam)
{
	
	ePipeline* Info = (ePipeline*)wParam;
	
	int64 Cmd  = Info->PopInt();
	switch(Cmd){
	case DEBUG_RESET:
		{
			ePipeline* DebugTree = (ePipeline*)Info->GetData(0);
			int64  TaskTimeStamp = DebugTree->GetID();

			ePipeline* PauseList = (ePipeline*)Info->GetData(1);
			
			Reset(*DebugTree);
			
			if (TaskTimeStamp!=0)
			{
				if(TaskTimeStamp != m_TaskTimeStamp){
					m_TaskTimeStamp = TaskTimeStamp; //先改，否则会出现第一次发出GUI_GET_DEBUG_ITEM信息后，还没来得及处理完，第二次又重复发
					ePipeline Msg(GUI_GET_DEBUG_ITEM);
					Msg.PushInt(m_TaskTimeStamp);
					GetGUI()->SendMsgToBrainFocuse(Msg);
				}else{
					while (PauseList->Size())
					{
						int64 PauseItemID = PauseList->PopInt();
						MarkPauseItem(PauseItemID);
					}
				}
			}
			
		}
		break;
	case DEBUG_MARK_PAUSE:
		{
			int64 TaskTimeStamp = Info->PopInt();
			int64 PauseItemID   = Info->PopInt();
			
			assert(TaskTimeStamp!=0);
			
			if(TaskTimeStamp != m_TaskTimeStamp){
				m_TaskTimeStamp = TaskTimeStamp; //先改，否则会出现第一次发出GUI_GET_DEBUG_ITEM信息后，还没来得及处理完，第二次又重复发
				ePipeline Msg(GUI_GET_DEBUG_ITEM);
				Msg.PushInt(m_TaskTimeStamp);
				GetGUI()->SendMsgToBrainFocuse(Msg);
			}else{
				MarkPauseItem(PauseItemID);
			}
		}
		break;
	case DEBUG_SET_BREAK:
		{
			int64 bBreak = Info->PopInt();
			ePipeline* Path = (ePipeline*)Info->GetData(0);

			CVSpace2* Item = Path2Item(*Path);
            if (Item)
            {
				if(bBreak){
					Item->m_State |= SPACE_BREAK;
					SetBreakBnt(TRUE,2);
				}else{
					Item->m_State &= ~SPACE_BREAK;
					SetBreakBnt(TRUE,1);
				}
				Invalidate();
            }

		}
		break;
	case DEBUG_INSERT_LOGIC:
		{
			ePipeline& InsertAddress = *(ePipeline*)Info->GetData(0);
			ePipeline& DebugItemList = *(ePipeline*)Info->GetData(1);
			
			InsertDebugItem(InsertAddress,DebugItemList);
		}
		break;
	case DEBUG_REMOVE_LOGIC:
		{
			ePipeline& RemoveAddress = *(ePipeline*)Info->GetData(0);
			int64 ChildID = *(int64*)Info->GetData(1);
			RemoveDebugItem(RemoveAddress,ChildID);
		}
		break;
	default:
		break;
	}

	return 0;
}
void CDebugView::Reset(ePipeline& ItemList){

	DeleteAll();

	m_Toolbar.m_Owner = NULL;
    m_SpaceFocused    = NULL;

	Layout();	
	
	m_TaskTimeStamp = ItemList.GetID();
	if(ItemList.Size()==0){
		return;
	};

	ePipeline* TaskPipe = (ePipeline*)ItemList.GetData(0);
	MassItem* Task = new MassItem;
	PushChild(Task);
	Task->Init(*TaskPipe);
	Task->m_Alias = 1; //任务ID是一个64位整数，显示不方便，这里改为1
    Layout();
	return ;
};


void CDebugView::InsertDebugItem(ePipeline& InsertAddress,ePipeline& DebugItemList)
{

	if (m_ChildList.size()!=1)
	{
		return;
	}

	MassItem* Parent = (MassItem*)m_ChildList[0];
	Parent = (MassItem*)FindeChild(InsertAddress,Parent);
	if (Parent==NULL)
	{
		return;
	}

	MassItem* NewLogic = new MassItem;
	NewLogic->Init(DebugItemList);

	Parent->PushChild(NewLogic);
	Layout();
	return;
};

void CDebugView::RemoveDebugItem(ePipeline& RemoveAddress,int64 ChildID)
{
	if (m_ChildList.size()!=1)
	{
		return;
	}
	MassItem* Parent = (MassItem*)m_ChildList[0];
	Parent = (MassItem*)FindeChild(RemoveAddress,Parent);
	if (Parent==NULL)
	{
		return;
	}

	deque<CVSpace2*>::iterator it = Parent->m_ChildList.begin();
	while(it != Parent->m_ChildList.end())
	{
		CVSpace2* Child = *it;
		if (Child->m_Alias == ChildID)
		{
			delete Child;
			Parent->m_ChildList.erase(it);
			break;
		}
		it++;
	}
	m_Toolbar.m_Owner = NULL;
	m_SpaceFocused = NULL;
	Layout();
	return;
}

LRESULT CDebugView::ToolbarReaction(ButtonItem* Bnt){
	//向brain发相应的命令
	switch(Bnt->m_Alias){
		
		case BNT_STEP:
			{

				if(Bnt->m_State & SPACE_DISABLE)return 0;
			
				SetStepBnt(false);
				SetRunBnt(false);
				
				
				ePipeline Msg(GUI_TASK_CONTROL);
				Msg.PushInt(CMD_DEBUG_STEP);
				Msg.PushInt(m_SpaceSelected->m_Alias);
			
				m_Toolbar.m_Owner = NULL;
				m_SpaceSelected->m_State &= ~SPACE_PAUSE;
				m_SpaceSelected->m_State &= ~SPACE_SELECTED; //不能直接=0,因为可能还有SPACE_BREAK标志有效
				m_SpaceSelected->m_AreaBottom-=m_Toolbar.m_Height;
				m_SpaceSelected = NULL;
				Layout();

				GetGUI()->SendMsgToBrainFocuse(Msg);
			}
			break;
		case BNT_RUN:
			{
				if(Bnt->m_State & SPACE_DISABLE)return 0;
				
				SetStepBnt(false);
				SetRunBnt(false);
				
				
				ePipeline Msg(GUI_TASK_CONTROL);
				Msg.PushInt(CMD_RUN);
				Msg.PushInt(m_SpaceSelected->m_Alias);
				
				m_Toolbar.m_Owner = NULL;
				m_SpaceSelected->m_State = 0;
				m_SpaceSelected->m_AreaBottom-=m_Toolbar.m_Height;
				m_SpaceSelected = NULL;
				Layout();
				
				GetGUI()->SendMsgToBrainFocuse(Msg);
			}
			break;
		case BNT_BREAK: 
			{
				if(Bnt->m_State & SPACE_DISABLE)return 0;
				
				assert(m_SpaceSelected);

				ePipeline Path;
				Item2Path(m_SpaceSelected,Path);

				ePipeline Msg(GUI_TASK_CONTROL);
				
				Msg.PushInt(CMD_DEBUG_BREAK);

				
				if (m_SpaceSelected->m_State&SPACE_BREAK)
				{
					Msg.PushInt(FALSE);		
				}else{				
					Msg.PushInt(TRUE);
				}
				Msg.PushPipe(Path);
					
				SetBreakBnt(false,0);
				GetGUI()->SendMsgToBrainFocuse(Msg);
				Invalidate();

			}
			break;
		default:
			break;
	} 
	return 0;
}


LRESULT CDebugView::OnCaptrueWin()
{
	OPENFILENAME ofn ;

	ofn.lStructSize 				= sizeof (OPENFILENAME) ;
	ofn.hwndOwner					= NULL ;
	ofn.hInstance					= NULL ;
	ofn.lpstrFilter 				= _T("bmp");
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.nMaxFile				= MAX_PATH ;
	ofn.nMaxFileTitle			= MAX_PATH ;
	ofn.lpstrInitialDir 		= NULL ;
	ofn.lpstrTitle				= NULL ;
	ofn.nFileOffset 			= 0 ;
	ofn.nFileExtension			= 0 ;
	ofn.lpstrDefExt 			= 0;
	ofn.lCustData				= 0L ;
	ofn.lpfnHook				= NULL ;
	ofn.lpTemplateName			= NULL ;

	TCHAR   szFileName[MAX_PATH], szFileTitle[MAX_PATH];
	*szFileTitle = 0;
	*szFileName  = 0;

	//strcpy(szFileName,".bmp");
	ofn.hwndOwner               = GetGUI()->GetHwnd();
	ofn.lpstrFile         		= szFileName ;
	ofn.lpstrFileTitle    		= szFileTitle ;
	ofn.Flags             		= OFN_OVERWRITEPROMPT ;

	if(!::GetSaveFileName (&ofn)){
		int ret = ::GetLastError();
		return 0;
	}
	tstring FileName = szFileName;
	FileName+=_T(".bmp");

	
	RECT rc;
	::GetClientRect(GetHwnd(),&rc);
	HDC hDC = ::GetDC(GetHwnd());

	HBITMAP hBmpCapture = CreateCompatibleBitmap(hDC,rc.right-rc.left,rc.bottom-rc.top); 

	HDC DCMem = ::CreateCompatibleDC(hDC);

	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, hBmpCapture);

	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rc,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT);

	Draw(DCMem);

	if(m_UseSystemScrollbar == false){
		DrawScrollbar(DCMem);
	};
	::SetBkMode(DCMem, OldMode );


	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rc,m_crWinBorder);
	}

	::SelectObject(DCMem, OldBitmap);
	DeleteObject(DCMem);
	::ReleaseDC(GetHwnd(),hDC);

	SaveBitmapToFile(hBmpCapture,FileName.c_str());
	return 0;

}

LRESULT CDebugView::OnCaptrueTree(){
	OPENFILENAME ofn ;

	ofn.lStructSize 				= sizeof (OPENFILENAME) ;
	ofn.hwndOwner					= NULL ;
	ofn.hInstance					= NULL ;
	ofn.lpstrFilter 				= _T("bmp");
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.nMaxFile				= MAX_PATH ;
	ofn.nMaxFileTitle			= MAX_PATH ;
	ofn.lpstrInitialDir 		= NULL ;
	ofn.lpstrTitle				= NULL ;
	ofn.nFileOffset 			= 0 ;
	ofn.nFileExtension			= 0 ;
	ofn.lpstrDefExt 			= 0;
	ofn.lCustData				= 0L ;
	ofn.lpfnHook				= NULL ;
	ofn.lpTemplateName			= NULL ;

	TCHAR   szFileName[MAX_PATH], szFileTitle[MAX_PATH];
	*szFileTitle = 0;
	*szFileName  = 0;

	//strcpy(szFileName,".bmp");
	ofn.hwndOwner               = GetGUI()->GetHwnd();
	ofn.lpstrFile         		= szFileName ;
	ofn.lpstrFileTitle    		= szFileTitle ;
	ofn.Flags             		= OFN_OVERWRITEPROMPT ;

	if(!::GetSaveFileName (&ofn)){
		int ret = ::GetLastError();
		return 0;
	}
	tstring FileName = szFileName;
	FileName+=_T(".bmp");


	RECT rcOld;
	::GetClientRect(GetHwnd(),&rcOld);
	
	::SetWindowPos(GetHwnd(),HWND_TOP,0,0,m_SizeX,m_SizeY,SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW);

	RECT rc;
	::GetClientRect(GetHwnd(),&rc);


	HDC hDC = ::GetDC(GetHwnd());
	HBITMAP hBmpCapture = CreateCompatibleBitmap(hDC,m_SizeX,m_SizeY); 

	HDC DCMem = ::CreateCompatibleDC(hDC);

	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, hBmpCapture);


	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rc,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT);

	Draw(DCMem);

	if(m_UseSystemScrollbar == false){
		DrawScrollbar(DCMem);
	};
	::SetBkMode(DCMem, OldMode );


	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rc,m_crWinBorder);
	}


	::SelectObject(DCMem, OldBitmap);
	DeleteObject(DCMem);
	::ReleaseDC(GetHwnd(),hDC);

	
	::SetWindowPos(GetHwnd(),HWND_TOP,0,0,rcOld.right-rcOld.left,rcOld.bottom-rcOld.top,SWP_NOZORDER|SWP_NOMOVE);

	SaveBitmapToFile(hBmpCapture,FileName.c_str());
	DeleteObject(hBmpCapture);
	return 0;
}

LRESULT CDebugView::OnCaptrueTreeOpened(){


	OPENFILENAME ofn ;

	ofn.lStructSize 				= sizeof (OPENFILENAME) ;
	ofn.hwndOwner					= NULL ;
	ofn.hInstance					= NULL ;
	ofn.lpstrFilter 				= _T("bmp");
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.nMaxFile				= MAX_PATH ;
	ofn.nMaxFileTitle			= MAX_PATH ;
	ofn.lpstrInitialDir 		= NULL ;
	ofn.lpstrTitle				= NULL ;
	ofn.nFileOffset 			= 0 ;
	ofn.nFileExtension			= 0 ;
	ofn.lpstrDefExt 			= 0;
	ofn.lCustData				= 0L ;
	ofn.lpfnHook				= NULL ;
	ofn.lpTemplateName			= NULL ;

	TCHAR   szFileName[MAX_PATH], szFileTitle[MAX_PATH];
	*szFileTitle = 0;
	*szFileName  = 0;

	//strcpy(szFileName,".bmp");
	ofn.hwndOwner               = GetGUI()->GetHwnd();
	ofn.lpstrFile         		= szFileName ;
	ofn.lpstrFileTitle    		= szFileTitle ;
	ofn.Flags             		= OFN_OVERWRITEPROMPT ;

	if(!::GetSaveFileName (&ofn)){
		int ret = ::GetLastError();
		return 0;
	}
	tstring FileName = szFileName;
	FileName+=_T(".bmp");

	OpenAllTree();
	Layout(FALSE);

	RECT rcOld;
	::GetClientRect(GetHwnd(),&rcOld);

	::SetWindowPos(GetHwnd(),HWND_TOP,0,0,m_SizeX,m_SizeY,SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW);

	RECT rc;
	::GetClientRect(GetHwnd(),&rc);


	HDC hDC = ::GetDC(GetHwnd());
	HBITMAP hBmpCapture = CreateCompatibleBitmap(hDC,m_SizeX,m_SizeY); 

	HDC DCMem = ::CreateCompatibleDC(hDC);

	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, hBmpCapture);


	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rc,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT);

	Draw(DCMem);

	if(m_UseSystemScrollbar == false){
		DrawScrollbar(DCMem);
	};
	::SetBkMode(DCMem, OldMode );


	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rc,m_crWinBorder);
	}


	::SelectObject(DCMem, OldBitmap);
	DeleteObject(DCMem);
	::ReleaseDC(GetHwnd(),hDC);


	::SetWindowPos(GetHwnd(),HWND_TOP,0,0,rcOld.right-rcOld.left,rcOld.bottom-rcOld.top,SWP_NOZORDER|SWP_NOMOVE);

	SaveBitmapToFile(hBmpCapture,FileName.c_str());
	DeleteObject(hBmpCapture);
	return 0;
}

LRESULT CDebugView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){

	case WM_RBUTTONDOWN:
		{
			return OnRButtonDown(wParam,lParam);
		}
	case WM_COMMAND:
		{
			return OnCommand(wParam,lParam);
		}
	default:
		return CWSTreeView::Reaction(message,wParam,lParam);
	}
}


LRESULT CDebugView::OnRButtonDown(WPARAM wParam, LPARAM lParam){

	//弹出菜单
	
	POINT pos;
	::GetCursorPos(&pos);
	
	HMENU hMenu = CreateMenu();
	if (!hMenu)
	{
		return 0;
	}
	HMENU hMenuPopup = CreatePopupMenu();
	if (!hMenuPopup)
	{
		::DestroyMenu(hMenu);
		return 0;
	}


	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuPopup, _T("Operator")) ;

	AppendMenu(hMenuPopup, 	MF_STRING, IDM_CAPTURE_WIN, _T("Capture &Win"));
	AppendMenu(hMenuPopup, 	MF_STRING, IDM_CAPTURE_TREE, _T("Capture &Tree"));
	AppendMenu(hMenuPopup, 	MF_STRING, IDM_CAPTURE_TREE_OPENED, _T("Capture Tree &Opened"));


	HMENU subMenu;			
	subMenu = GetSubMenu(hMenu, 0); 
	
	::SetForegroundWindow(GetHwnd()); 
	::TrackPopupMenu(subMenu,0, pos.x, pos.y,0,GetHwnd(),NULL);
	::DestroyMenu(hMenu); 
	::PostMessage(GetHwnd(),WM_NULL, 0, 0);
	
	return 0;
}

LRESULT CDebugView::OnCommand(WPARAM wParam, LPARAM lParam){
	switch(wParam){
	case IDM_CAPTURE_WIN:
		return OnCaptrueWin();
	case IDM_CAPTURE_TREE:
		return OnCaptrueTree();
	case IDM_CAPTURE_TREE_OPENED:
		return OnCaptrueTreeOpened();
	}

	return 0;
}
