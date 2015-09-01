// ThreadFind.cpp: implementation of the CFindView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "GMIS.h"
#include "FindView.h"
#include "WSScrollView.h"

FindItem::FindItem(int64 ID,const TCHAR* text)
	:CVSpace2(ID),m_Text(text)
{	
};

void FindItem::Draw(HDC hDC, ePipeline* Pipe){
	//输出文字
    RECT rc = GetArea();
	rc.bottom = rc.top+16;
	FillRect(hDC,&rc,RGB(215,215,215));

	rc = GetArea();
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			                          DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	
	::SetTextColor(hDC,Oldcr);
}



void ArrowBnt::Draw(HDC hDC, ePipeline* Pipe){

	COLORREF   crText = RGB(0,0,0);
	if(m_State & SPACE_DISABLE)return; //crText = RGB(128,128,128);

	RECT rc = GetArea();
	if(m_State & SPACE_FOCUSED){
		FillRect(hDC,&rc,RGB(0xee,0xee,0xee));
	}
	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_CENTER|DT_SINGLELINE|DT_VCENTER);	
	::SetTextColor(hDC,Oldcr);	

};
PageBnt::PageBnt(int64 ID,const TCHAR* text)
	:CVSpace2(ID),m_Text(text)
{	
};

void PageBnt::Draw(HDC hDC, ePipeline* Pipe){
	//输出文字
    if(m_State & SPACE_DISABLE)return;

    RECT rc = GetArea();
	if(m_State & SPACE_SELECTED){
	    FillRect(hDC,&rc,RGB(0xee,0xee,0xee));
		DrawEdge(hDC,&rc,RGB(0x7d,0xaa,0xd0));
	}
	else if(m_State & SPACE_FOCUSED){
		FillRect(hDC,&rc,RGB(0xee,0xee,0xee));
	}
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
	::SetTextColor(hDC,Oldcr);
}

CFindToolbar::CFindToolbar()
{

};
void CFindToolbar::Draw(HDC hDC, ePipeline* Pipe){
	AlphaBlendGlass(hDC,m_AreaLeft,m_AreaTop,AreaWidth(),AreaHeight(),RGB(128,128,128),128);
	for(int i=0; i<m_ChildList.size(); i++){m_ChildList[i]->Draw(hDC);}
}
void CFindToolbar::Reset(){
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
    while(It != m_ChildList.end()){
		CVSpace2* Item = *It;
		Item->m_State |= SPACE_DISABLE;
		It++;
	}
} 



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFindView::CFindView()
{
	m_UseSystemScrollbar = true;
	m_Padding = 4;
	m_crViewBkg = RGB(255,255,255);
	m_crText    = RGB(0,0,0);
//	m_crViewBorder = m_crText; 
	m_Font  = NULL;

	m_SpaceSelected  = NULL;
	m_ItemNumPerPage = 10;
	m_PageCount = 0;
    m_CurPage = 0;
    m_bFindEnd = FALSE;
}

CFindView::~CFindView()
{
	if(m_Font)::DeleteObject(m_Font);
}


void CFindView::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName )
{
	LOGFONT	LogFont;
	LogFont.lfHeight			= -MulDiv( nHeight, GetDeviceCaps( GetDC(m_hWnd), LOGPIXELSY ), 72 );
	LogFont.lfWidth				= 0;
	LogFont.lfEscapement		= 0;
	LogFont.lfOrientation		= 0;
	LogFont.lfWeight			= ( bBold )? FW_BOLD:FW_DONTCARE;
	LogFont.lfItalic			= (BYTE)( ( bItalic )? TRUE:FALSE );
	LogFont.lfUnderline			= FALSE;
	LogFont.lfStrikeOut			= FALSE;
	LogFont.lfCharSet			= DEFAULT_CHARSET;
	LogFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality			= DEFAULT_QUALITY;
	LogFont.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;

	_tccpy( LogFont.lfFaceName, csFaceName );

	if( m_Font!= NULL )
		::DeleteObject(m_Font);

	m_Font = ::CreateFontIndirect( &LogFont );
}

void CFindView::SetFindTip(tstring Tip){
	m_FindTip = Tip;	
};


LRESULT CFindView::OnAddResult(WPARAM wParam, LPARAM lParam){
	const TCHAR* Text = (const TCHAR*)wParam;
	FindItem* Item = new FindItem(lParam,Text);
	    
	int PageNum = (lParam-1)/m_ItemNumPerPage+1;
	if(m_CurPage != PageNum){ //显示新一页结果
		DeleteAll();
		m_CurPage = PageNum;
		m_PageCount = max(m_PageCount,PageNum); 
		UpdateToolbar();
	}

	SIZE s = CalcuTextSize(Text);
	Item->SetArea(0,0,s.cx+8,s.cy+8);
	m_ChildList.push_back(Item);
	Layout();

	return 0;
}

LRESULT CFindView::OnClearResult(WPARAM wParam, LPARAM lParam){  
	DeleteAll();
	m_Toolbar.DeleteAll();
	m_PageCount = 0;
	m_CurPage = 0;

	Layout();
	UpdateToolbar();
	return 0;
}	
LRESULT CFindView::OnFindEnd(WPARAM wParam, LPARAM lParam){ 
	m_bFindEnd = wParam;
	ArrowBnt* Back = (ArrowBnt*)m_Toolbar.m_ChildList.back();		
	if(m_bFindEnd && m_CurPage == m_PageCount)Back->m_State = SPACE_DISABLE;
	else Back->m_State = 0;
	Invalidate();
	return 0;
}; //当前搜索已经全部完成



LRESULT CFindView::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case FIND_ADDRESULT:
		return OnAddResult(SRM->wParam,SRM->lParam);
	case FIND_CLRRESULT:
		return OnClearResult(SRM->wParam,SRM->lParam);
	case FIND_END:
		return OnFindEnd(SRM->wParam,SRM->lParam);
	}
	return 0;
}


void CFindView::Draw(HDC hDC, ePipeline* Pipe){
    CWSTreeView::Draw(hDC);
	m_Toolbar.Draw(hDC);

	if (m_FindTip.size())
	{
		RECT rc;
		rc.left=0;
		rc.right=100;
		rc.top=0;
		rc.bottom=20;
		COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
		::DrawText(hDC,m_FindTip.c_str(),m_FindTip.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_WORDBREAK|DT_NOPREFIX);	
		::SetTextColor(hDC,Oldcr);
	}
};

void  CFindView::Layout(bool Redraw /* =true*/){
	CWSTreeView::Layout();
	UpdateToolbar();

	
}

void  CFindView::UpdateToolbar(){
	PageBnt* Bnt = NULL;
	TCHAR buf[30];	
	
	//WM_SIZE和m_CurPage改变都可能导致BntNum改变，如果有改变则重置BntNum
	RECT rcOld = m_Toolbar.GetArea();
	RECT rc;
	GetClientRect(GetHwnd(),&rc);
	if(RectWidth(rc)<100){
		rc.right = rc.left+100;
		m_SizeX = max(m_SizeX,120);
	}
	rc.top = rc.bottom-20;
	m_Toolbar.SetArea(rc);

	rc.left+=10;
	rc.right-=10;
	_itot(m_CurPage,buf,10);
	SIZE size = CalcuTextSize(buf);
	if(size.cx<16)size.cx = 16;
	int32 w = RectWidth(rc);
	w -= 40; //前后两个箭头的宽度
	int32 PageBntNum = w/size.cx;
	
	//只有BntNum发生改变时才重新设置toolbar的按纽数目
	if(m_Toolbar.m_ChildList.size() != (PageBntNum+2)){
		m_Toolbar.DeleteAll();				
		ArrowBnt* Front = new ArrowBnt(-1,_T("<<"));
        
		rc.right = rc.left+20;
		Front->SetArea(rc);
		Front->m_State |= SPACE_DISABLE;
		m_Toolbar.m_ChildList.push_back(Front);
		
		rc.left = rc.right;
		for (int32 i=0; i<PageBntNum; i++)
		{
			rc.right = rc.left + size.cx;
			PageBnt* Page = new PageBnt(0,_T(""));
			Page->SetArea(rc);
			Page->m_State |= SPACE_DISABLE;
			m_Toolbar.m_ChildList.push_back(Page);
			rc.left = rc.right;
		}
		ArrowBnt*  Back  = new ArrowBnt(-2,_T(">>"));
		rc.right = rc.left + 20;
		Back->SetArea(rc);
		Back->m_State |= SPACE_DISABLE;
		m_Toolbar.m_ChildList.push_back(Back);
	}else if(!::EqualRect(&rc,&rcOld)){ //WM_SIZE改变
		for (int32 i=0; i<m_Toolbar.m_ChildList.size(); i++)
		{
			//只需要处理高度
			CVSpace2* Space = m_Toolbar.m_ChildList[i];
            Space->m_AreaTop  = rc.bottom-20;
			Space->m_AreaBottom = rc.bottom;
		}
	}

	if(m_PageCount == 0){
		m_Toolbar.Reset();
		return ;
	}
	//现在给设置每一个按纽的值和状态
	if(m_PageCount < PageBntNum){
		for(int i=1; i<m_PageCount+1; i++){
			Bnt = (PageBnt*)m_Toolbar.m_ChildList[i];
			Bnt->m_Alias = i;
			_itot(i,buf,10);
			Bnt->m_Text = buf;
			Bnt->m_State = 0; //去掉SPACE_DIABLE
			if(i == m_CurPage){
				Bnt->m_State = SPACE_SELECTED;
			}
		}
	}else if(m_PageCount == m_CurPage){			
		int32 n = m_CurPage;
		for(int i=PageBntNum; i>0; i--){
			Bnt = (PageBnt*)m_Toolbar.m_ChildList[i];
			Bnt->m_Alias = n;
			_itot(n,buf,10);
			Bnt->m_Text = buf;
			Bnt->m_State = 0;
			n--;
		}
		//最后那个为当前选择
		Bnt = (PageBnt*)m_Toolbar.m_ChildList[m_Toolbar.m_ChildList.size()-2];
		Bnt->m_State = SPACE_SELECTED;

	}else { //m_PageCount > m_CurPage 
		int32 n = m_PageCount - m_CurPage;
		int32 t;
		if(n < PageBntNum/2){
			t = m_PageCount;

		}else{ //令m_CurPage位于中间
			t = m_CurPage + PageBntNum/2;
		}
		for(int i=m_Toolbar.m_ChildList.size()-2; i>0; i--){
			Bnt = (PageBnt*)m_Toolbar.m_ChildList[i];
			Bnt->m_Alias = t;
			_itot(t,buf,10);
			Bnt->m_Text = buf;
			Bnt->m_State = 0;
			if(t == m_CurPage){
				Bnt->m_State = SPACE_SELECTED;
			}
			t--;
		}		
	}

 	ArrowBnt* Front = (ArrowBnt*)m_Toolbar.m_ChildList.front();
	ArrowBnt* Back = (ArrowBnt*)m_Toolbar.m_ChildList.back();
		
	if(m_CurPage > 1)Front->m_State = 0;
	else Front->m_State = SPACE_DISABLE;

	if(m_bFindEnd && m_CurPage == m_PageCount)Back->m_State = SPACE_DISABLE;
	else Back->m_State = 0;

	Invalidate();	
}
	

CVSpace2* CFindView::Hit(int32 x, int32 y){
	POINT point;
	point.x = x;
	point.y = y;

	if(::PtInRect(&m_Toolbar.GetArea(),point)){
		deque<CVSpace2*>::iterator It = m_Toolbar.m_ChildList.begin();
		while(It<m_Toolbar.m_ChildList.end()){
			CVSpace2* Space = *It;
			RECT rc = Space->GetArea();
			if(::PtInRect(&rc,point)){
				return Space;
			}
			It++;
		};
		return NULL;
	}

	deque<CVSpace2*>::iterator It = m_HotSpaceList.begin();
	while(It<m_HotSpaceList.end()){
		CVSpace2* Space = *It;
		RECT rc = Space->GetArea();
		if(::PtInRect(&rc,point)){
			return Space;
		}
        It++;
	};
	return NULL;	
};


LRESULT CFindView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	default:
		return CWSTreeView::Reaction(message,wParam,lParam);
	}
};


LRESULT CFindView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
 	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;

	CVSpace2* NewSelect = Hit(point.x,point.y);
	if(!NewSelect)return 0;
	if(NewSelect->m_State & SPACE_DISABLE)return 0;

	if(::PtInRect(&m_Toolbar.GetArea(),point)){
		int32 n=0;
		if(NewSelect->m_Alias == -1 ){
              n = (m_CurPage-2)*m_ItemNumPerPage;
		}else if(NewSelect->m_Alias == -2)
		{
              n = m_CurPage*m_ItemNumPerPage;
		}else{
			  if(NewSelect->m_Alias == m_CurPage)return 0;
              n = (NewSelect->m_Alias-1) * m_ItemNumPerPage;
		}

		DeleteAll();
		Layout();
			
		ePipeline Msg(GUI_GET_FIND_REUSLT);
		Msg.PushInt(n);

		GetGUI()->SendMsgToBrainFocuse(Msg);

	}
	return 0;
}




void CFindView::AddText(int32 Index, int64 RoomID,const TCHAR* Text){
	return;
	tstring s = Format1024(_T("#%3d    TEXT              %s\n   %s"),
		Index,AbstractSpace::GetTimer()->GetYMD(RoomID).c_str(),Text);

	if(GetHwnd())
		SendChildMessage(GetHwnd(),FIND_ADDRESULT,(WPARAM)s.c_str(),(LPARAM)Index);
	else 
		OnAddResult((WPARAM)s.c_str(),(LPARAM)Index);
}

void CFindView::AddObjectOrPeople(int32 Index, int64 RoomID,const TCHAR* Text, const TCHAR* Memo){
	tstring s = Format1024(_T("#%3d    OBJECT / PEOPLE   %s\n%s\nMemo: %s"),
		Index,AbstractSpace::GetTimer()->GetYMD(RoomID).c_str(),
		Text,Memo);
	if(GetHwnd())
		SendChildMessage(GetHwnd(),FIND_ADDRESULT,(WPARAM)s.c_str(),(LPARAM)Index);
	else 
		OnAddResult((WPARAM)s.c_str(),(LPARAM)Index);
}

void CFindView::AddCommandOrLogic(int32 Index, int64 RoomID,const TCHAR* Text, const TCHAR* Memo){
	tstring s = Format1024(_T("#%3d    CMD / LOGIC       %s\n%s\nMemo: %s"),
		Index,AbstractSpace::GetTimer()->GetYMD(RoomID).c_str(),
		Text,Memo);
	if(GetHwnd())
		SendChildMessage(GetHwnd(),FIND_ADDRESULT,(WPARAM)s.c_str(),(LPARAM)Index);
	else 
		OnAddResult((WPARAM)s.c_str(),(LPARAM)Index);
}
	
	
void CFindView::FindEnd(BOOL IsAllEnd){
	if(GetHwnd())
		SendChildMessage(GetHwnd(),FIND_END,(WPARAM)IsAllEnd,0);
	else 
		OnFindEnd((WPARAM)IsAllEnd,0);
}
	

void CFindView::ClearAll(){
	if(GetHwnd())
		SendChildMessage(GetHwnd(),FIND_CLRRESULT,0,0);
	else 
		OnClearResult(0,0);
};

