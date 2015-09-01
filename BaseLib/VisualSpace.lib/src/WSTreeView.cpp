// WSTreeView.cpp : implementation file
//

#include "WSTreeView.h"

namespace VISUALSPACE{

	CWSTreeView::CToolbar::CToolbar()
		:m_Owner(NULL),m_Height(25){
		m_crToolBgk = RGB(160,160,160);
	}
	CWSTreeView::CToolbar::~CToolbar()
	{
		
	}
	void CWSTreeView::CToolbar::Layout(bool Redraw /*= true*/){
		if(m_Owner==NULL)return;
		
		SetArea(m_Owner);
		m_AreaTop = m_AreaBottom - m_Height;
		m_AreaLeft +=1;
		m_AreaRight -=1;
		m_AreaBottom -=2;
		
		int32 len = AreaWidth()-4;
		len = len/m_ChildList.size();
		
		if (len>80) //避免按钮太大
		{
			len = 80;
		}
		deque<CVSpace2*>::iterator It =m_ChildList.begin();
		RECT rc = GetArea();
		rc.left += 2;
		while(It<m_ChildList.end()){
			rc.right = rc.left +len;
			CVSpace2* Space = *It;			
			Space->SetArea(rc);
			Space->m_AreaLeft +=2;
			Space->m_AreaRight-=2;
			Space->m_AreaTop+=2;
			Space->m_AreaBottom-=2;
			rc.left = rc.right;
			It++;
		}	
	};
	
	void CWSTreeView::CToolbar::Draw(HDC hDC, ePipeline* Pipe){
		FillRect(hDC,GetArea(),m_crToolBgk);
		CVSpace2::Draw(hDC);
	};

CWSTreeView::CTreeItem::CTreeItem(int64 ID,const TCHAR* text)
		:CVSpace2(ID),m_Text(text)
{	
};
void CWSTreeView::CTreeItem::Draw(HDC hDC,ePipeline* Pipe){
					
	COLORREF   crBorder    = RGB(128,128,128);
	COLORREF   crFkg       = RGB(0,0,0); 
	COLORREF   crBkg       = RGB(255,255,255); 
	COLORREF   crSelectFkg = RGB(255,255,255); 
	COLORREF   crSelectBkg = RGB(255,0,0);  
				
	//输出内容
	RECT rc = GetArea();	
	if(m_State&SPACE_FOCUSED){
		//pDC->DrawFocusRect(&m_Area);
		FillRect(hDC,rc,crFkg);
  	    DeflateRect(&rc,4,4,4,4);
		COLORREF Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
			DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	
		::SetTextColor(hDC,Oldcr);
	}
    else if(m_State & SPACE_SELECTED){
        FillRect(hDC,rc,crSelectBkg);
		DeflateRect(&rc,4,4,4,4);
		COLORREF Oldcr = ::SetTextColor(hDC,crSelectFkg);
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
			DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);		
		::SetTextColor(hDC,Oldcr);
	}
	else {
		FillRect(hDC,rc,crBkg);
  	    DeflateRect(&rc,4,4,4,4);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
			DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	
		::SetTextColor(hDC,Oldcr);
	}

	//画文字边框			
	DrawEdge(hDC,GetArea(),crBorder);
}


/////////////////////////////////////////////////////////////////////////////
// CWSTreeView

CWSTreeView::CWSTreeView()
{
	m_State |= SPACE_SHOWWINBORDER;
	m_HeaderWidth = 16;
	m_crText           = RGB(0,0,0);
	m_crConnectingLine = RGB(128,128,128);
	m_bShowLine = true;
	m_Padding = 4;
	m_SpaceSelected = NULL;
	m_LinePen = ::CreatePen(PS_SOLID, 1, m_crConnectingLine);
	m_Font  = NULL;
	m_ToobarItemSelected = NULL;
}

CWSTreeView::~CWSTreeView()
{
	if(m_LinePen)::DeleteObject(m_LinePen);
	if(m_Font)::DeleteObject(m_Font);
}

void CWSTreeView::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* FontName )
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

	_tcscpy( LogFont.lfFaceName, FontName );

	if( m_Font!= NULL )
		::DeleteObject(m_Font);
	
	m_Font = ::CreateFontIndirect( &LogFont );
}

RECT CWSTreeView::GetHeaderBox(CVSpace2* Space){
	RECT rc=Space->GetArea();
	rc.left -= m_HeaderWidth;
	rc.right = rc.left + m_HeaderWidth;

	int32 x =  rc.left+RectWidth(rc)/2;
	int32 y =  rc.top+RectHeight(rc)/2;
	RECT rcHeaderBox;
	::SetRect(&rcHeaderBox,x,y,x,y);
    rcHeaderBox.left   -= 4;
	rcHeaderBox.right  += 5;
	rcHeaderBox.top    -= 4;
	rcHeaderBox.bottom += 5;
	
	return rcHeaderBox;
}

void CWSTreeView::SetLineColor(COLORREF color,int PenType/*=PS_SOLID*/){
	m_crConnectingLine = color;
	if(m_LinePen)::DeleteObject(m_LinePen);	
	m_LinePen = ::CreatePen(PenType, 1, m_crConnectingLine);
};

void CWSTreeView::DrawItemHeader(HDC hDC, CVSpace2* Space){


	RECT rcTreeBox = GetHeaderBox(Space);

	POINT Center;
	Center.x =  rcTreeBox.left+RectWidth(rcTreeBox)/2;
	Center.y =  rcTreeBox.top+RectHeight(rcTreeBox)/2;
	
    HPEN OldPen = (HPEN)::SelectObject(hDC,m_LinePen);

	if( Space->m_ChildList.size() >0 ){  
		RECT rc;
		SetRect(&rc,Center.x - 2,Center.y,Center.x + 3,Center.y+1);
		FillRect(hDC,rc,m_crConnectingLine);
		
		if (!(Space->m_State& SPACE_OPEN))
		{
			SetRect(&rc,Center.x,Center.y-2,Center.x+1,Center.y+3);
			FillRect(hDC,rc,m_crConnectingLine);
		}
		DrawEdge(hDC,rcTreeBox,m_crConnectingLine);
	}else{
		if (Space->m_Parent == NULL)
		{
			FillRect(hDC,rcTreeBox,m_crConnectingLine);
		} 
		else
		{
			::MoveToEx(hDC,rcTreeBox.left, Center.y,NULL); 
			::LineTo(hDC,rcTreeBox.right, Center.y);
		}
	}
 

	//画连接线
	if(m_bShowLine){

		
		int32 w = m_HeaderWidth-RectWidth(rcTreeBox)/2;		
		int32 x = rcTreeBox.left-w;
		
		//与Parent的连接线
		CVSpace2* Parent = Space->m_Parent;
		if(Parent && Parent != this){
			//横线	
			::MoveToEx(hDC,x, Center.y,NULL); 
			::LineTo(hDC,x+w, Center.y);
			//竖线
			CVSpace2* LastChild = Parent->m_ChildList.back();
			RECT rcTreeBox = GetHeaderBox(LastChild);
			int32 w1 = m_HeaderWidth-RectWidth(rcTreeBox)/2;		
			int32 x = rcTreeBox.left-w1;
			int32 y1 = Parent->m_AreaTop + ( Parent->AreaHeight() / 2 )+RectWidth(rcTreeBox)/2;;		
			int32 y2 =  rcTreeBox.top+RectHeight(rcTreeBox)/2;
			//int32 y2 = LastChild->m_AreaTop + ( RectHeight(LastChild->GetArea()) / 2 );
			
			::MoveToEx(hDC,x, y1,NULL); 
			::LineTo(hDC,x, y2);	
		};
		
		//连接自己所有Child竖线
		if (Space->m_ChildList.size()>0 && Space->m_State & SPACE_OPEN)
		{
			CVSpace2* LastChild = Space->m_ChildList.back();
			rcTreeBox = GetHeaderBox(LastChild);
			w = m_HeaderWidth-RectWidth(rcTreeBox)/2;		
			x = rcTreeBox.left-w;
			int32 y1 = Space->m_AreaTop +  Space->AreaHeight() / 2 +RectWidth(rcTreeBox)/2;;		
			int32 y2 =  rcTreeBox.top+RectHeight(rcTreeBox)/2;
			//int32 y2 = LastChild->m_AreaTop + ( RectHeight(LastChild->m_Area) / 2 );
			
			::MoveToEx(hDC,x, y1,NULL); 
			::LineTo(hDC,x, y2);	
		}
	}
    ::SelectObject(hDC,OldPen);
}
    


void CWSTreeView::Draw(HDC hDC,ePipeline* Pipe){
	HFONT OldFont;
	if(m_Font) OldFont = (HFONT)::SelectObject(hDC,m_Font);

	deque<CVSpace2*>::iterator It = m_HotSpaceList.begin();
	while(It<m_HotSpaceList.end()){
		CVSpace2*  Space = *It;
		if(Space){
		    DrawItemHeader(hDC,Space);
			Space->Draw(hDC);
		}
		It++;
	}
	if(m_Font)::SelectObject(hDC,OldFont);
}

void CWSTreeView::Layout(bool Redraw  /*=true*/){

	RECT rcViewport;
	::GetClientRect(m_hWnd,&rcViewport);
	
	if(RectWidth(rcViewport)==0 || RectHeight(rcViewport)==0)return;
    
	m_HotSpaceList.clear();

	int32 x = m_Padding+m_HeaderWidth - GetHScrollPos();
	int32 y = m_Padding - GetVScrollPos();

	SIZE size;
	size.cx = 0;
	size.cy = 0;

    m_SizeX = size.cx;
	m_SizeY = size.cy;

	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while(It<m_ChildList.end()){
		CVSpace2*  Space = *It;
		size = LayoutRecursive( Space, //返回右下点坐标
			        x,
			        y,
			        rcViewport );
		
		y += size.cy+m_Padding;

        m_SizeX = max(size.cx,m_SizeX);
		m_SizeY += size.cy+m_Padding;

		It++;
	}
	
	m_SizeX += m_Padding + m_Padding + m_HeaderWidth;
	m_SizeY += m_Padding*3;  
	
	LayoutScrollBar();

	if(Redraw){
		Invalidate();
	}
}

SIZE CWSTreeView::LayoutRecursive(CVSpace2* Space,int x, int y, const RECT& rcViewport){
    
	int32 w = Space->AreaWidth();
	int32 h = Space->AreaHeight();

	// 具体的显示区域坐标总是动态生成的
    Space->SetArea(x,y,x+w,y+h);

	RECT	Rect = Space->GetArea();
	
	bool  Saved = false; 
	// Draw only if the node is visible
	if( Rect.right  > 0  && Rect.left < rcViewport.right &&
		Rect.bottom > 0  && Rect.top  < rcViewport.bottom )
	{
		m_HotSpaceList.push_back(Space);
		Saved = true;
	}
    
	if(m_HeaderWidth>0){
		RECT rcTreeBox;
		::SetRect(&rcTreeBox,Rect.left-m_HeaderWidth,Rect.top,Rect.left,Rect.bottom);
		if(rcTreeBox.right  > 0  && rcTreeBox.left < rcViewport.right &&
			rcTreeBox.bottom > 0  && rcTreeBox.top  < rcViewport.bottom ){
			if(!Saved)m_HotSpaceList.push_back(Space);
		}
	}

	SIZE   DocSize;
	DocSize.cx = w;
	DocSize.cy = h;   // Total document size

	SIZE  Size;
	if( Space->m_State & SPACE_OPEN ){
		x += m_HeaderWidth;
		y = Rect.bottom+m_Padding;
		deque<CVSpace2*>::iterator It = Space->m_ChildList.begin();
		CVSpace2* Child;
		while(It<Space->m_ChildList.end()){
			Child = *It;
			Size = LayoutRecursive(Child, x, y, rcViewport );			
			DocSize.cx = max(Size.cx+m_HeaderWidth,DocSize.cx);
			DocSize.cy += Size.cy+m_Padding;

			y += Size.cy+m_Padding;
			It++;
		}
	}
	return DocSize ;
}

void CWSTreeView::ToggleSpace( CVSpace2* Space){
	if(Space->m_State & SPACE_OPEN)Space->m_State &= ~SPACE_OPEN;
    else  Space->m_State |= SPACE_OPEN;
	Layout();
}

void  CWSTreeView::EnsureVisible(CVSpace2* Space,bool FirstVisible){
    bool Update = false;

	//首先确保其所有parent都是打开的，如此才能保证pNode可见
	CVSpace2* Child = Space;
	while(Child->m_Parent){
		if(!(Child->m_Parent->m_State & SPACE_OPEN)){
		    Child->m_Parent->m_State |= SPACE_OPEN;
			Update = true;
		}
		Child = Child->m_Parent;
	}

	
	Layout(false);
	

	RECT rc ;//= m_rcClient;
	::GetClientRect(m_hWnd,&rc);
    if(RectHeight(rc)>m_SizeY){
		Invalidate();
		//UpdateWindow(m_hWnd);
		return;
	} 

	int32 TopPos = Space->m_AreaTop + GetVScrollPos();
    int32 Pos = TopPos;
	if(FirstVisible){		
		if(TopPos + RectHeight(rc) > m_SizeY){ //不足于在顶部显示
			Pos = RectHeight(rc)-Space->AreaHeight() + TopPos;
		}
	}
	else{
		if(Space->AreaHeight() < RectHeight(rc)){ //如果条目比显示面积下则底部显示，否则只能顶部显示
			Pos = RectHeight(rc)-Space->AreaHeight() + TopPos;
		}
	}
	SetVScrollPos(Pos,true);

}

CVSpace2* CWSTreeView::Hit(POINT& point){

//	if(!::PtInRect(&GetArea(),point))return NULL; 错误原因：GetArea()坐标是相对于父空间
	CVSpace2* Space = NULL;
	if(Space = m_VScrollbar.HitTest(point.x,point.y))return Space;
	if(Space = m_HScrollbar.HitTest(point.x,point.y))return Space;

	deque<CVSpace2*>::iterator It = m_HotSpaceList.begin();
	while(It<m_HotSpaceList.end()){
		Space = *It;
		Space->m_AreaLeft -= m_HeaderWidth;  //包含节点按钮
		RECT rc = Space->GetArea();
		if(::PtInRect(&rc,point)){
			Space->m_AreaLeft += m_HeaderWidth;
			CVSpace2* HitSpace = Space->HitTest(point.x,point.y);
			if(HitSpace)return HitSpace;
			else return Space;
		}
		Space->m_AreaLeft += m_HeaderWidth;
        It++;
	};
	return NULL;	
}

SIZE CWSTreeView::CalcuTextSize(const TCHAR* Text){
	HDC		hDC	    = ::GetDC(m_hWnd);
	int		Saved	= ::SaveDC(hDC);
	HFONT	OldFont	= (HFONT)::SelectObject(hDC, m_Font);

//  Item->m_SizeX = 0;
//	Item->m_SizeY = 0;

	SIZE  size;
	size.cx = 0;
	size.cy = 0;

	int n=0;

	const TCHAR* Pos = Text;

	while(*Text != L'\0'){
		if(*Text == '\r'){
			if(*(++Text)==L'\n')n++;
			else --Text;
			DWORD  s = ::GetTabbedTextExtent(hDC,Pos,n,0,0);
			size.cx = max(size.cx,LOWORD(s));
			size.cy += HIWORD(s);
			n = 0;
			Pos = Text;
		}
		else if(*Text == L'\n'){
			DWORD  s = ::GetTabbedTextExtent(hDC,Pos,n,0,0);
			size.cx = max(size.cx,LOWORD(s));
			size.cy += HIWORD(s);
			n = 0;
			Pos = Text;
		}
		Text++;
		n++;			
	}

	if(n !=0){ //处理非'\n'结尾的情况
		DWORD  s = ::GetTabbedTextExtent(hDC,Pos,n,0,0);
		size.cx = max(size.cx,LOWORD(s));
		size.cy += HIWORD(s);
		n = 0;
	}

	::SelectObject(hDC, OldFont );
	::RestoreDC(hDC,Saved);
	::ReleaseDC(m_hWnd,hDC);
	return size;
}
	
LRESULT CWSTreeView::ToolbarReaction(ButtonItem* Bnt)
{
	return 0;
}

LRESULT CWSTreeView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam,lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_SIZE:
		return OnSize(wParam,lParam);

	default: 
		return CWSScrollView::Reaction(message,wParam,lParam);
	}

}

LRESULT CWSTreeView::OnMouseWheel(WPARAM wParam, LPARAM lParam) 
{
	int32 ScrollPos = ::GetScrollPos(m_hWnd,SB_VERT);
	short zDelta = (short) HIWORD(wParam);
	ScrollPos += ( zDelta > 0 )? - SCROLL_MOVE_UNIT:SCROLL_MOVE_UNIT;
	::SetScrollPos(m_hWnd,SB_VERT,ScrollPos,true);
    Layout(true);
	return ::DefWindowProc(m_hWnd,WM_MOUSEWHEEL,wParam,lParam);
}

LRESULT CWSTreeView::OnMouseMove(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnMouseMove(wParam,lParam);
	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	
	ButtonItem* ToolItem = (ButtonItem*)m_Toolbar.HitTest(point.x,point.y);
	if (ToolItem && (CVSpace2*)ToolItem != (CVSpace2*)&m_Toolbar)
	{
		ToolItem->m_State |= SPACE_FOCUSED;
		CVSpace2* OldFocus = m_SpaceFocused;
		m_SpaceFocused = ToolItem;
		FocuseChanged(OldFocus,point);
	}
	
	return ret;
}


LRESULT CWSTreeView::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonUp(wParam,lParam);

	::ReleaseCapture();

    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	if(m_ScrollSelected ){
		return ret;
	}
	
	
	if(!m_SpaceSelected)
	{
		return ret;
	}
	

	CVSpace2* NewSelect = Hit(point);
	
	if(NewSelect && NewSelect == m_SpaceSelected)
	{
		ButtonItem* ToolItem = (ButtonItem*)m_Toolbar.HitTest(point.x,point.y);
		if (ToolItem && (CVSpace2*)ToolItem != (CVSpace2*)&m_Toolbar)
		{
			if (ToolItem->m_State & SPACE_SELECTED)
			{
				ToolItem->m_State &=~SPACE_SELECTED;
				ToolbarReaction(ToolItem);
			}
		}
	}
	
	if(m_ToobarItemSelected){
		m_ToobarItemSelected->m_State = 0;
		m_ToobarItemSelected = NULL;
		Invalidate();
	}
	
	return ret;
}

LRESULT CWSTreeView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;


    if(m_ScrollSelected){
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_AreaBottom-= m_Toolbar.m_Height;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			m_Toolbar.m_Owner = NULL;
			m_SpaceSelected = NULL;
		}
		return ret;
	}

	::SetCapture(m_hWnd);

	CVSpace2* NewSelect = Hit(point);
    if(NewSelect==NULL)return 0;
	
	if(NewSelect == m_SpaceSelected){		
		//点击发生在treebox?
		RECT TreeBox = GetHeaderBox(m_SpaceSelected);  
		if(::PtInRect(&TreeBox,point)){
			//取消工具条，避免因为收缩导致工具条依然在原地
			/*if(m_Toolbar.m_Owner){ 
				m_Toolbar.m_Owner->m_AreaBottom-=m_Toolbar.m_Height;
				m_Toolbar.m_Owner->m_State &= ~SPACE_SELECTED;
				m_Toolbar.m_Owner = NULL;
			}
			*/
			ToggleSpace(m_SpaceSelected);
			return 0;
		}
		
		//点击发生在item?
		RECT rc = m_SpaceSelected->GetArea();
		rc.bottom-=m_Toolbar.m_Height; //排除toolbar再次点击在Item上则取消选择
		if(::PtInRect(&rc,point)){
			m_SpaceSelected->SetArea(rc);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			m_SpaceSelected = NULL;
			m_Toolbar.m_Owner = NULL;
			Layout();
			return 0;
		}
		//点击发生在toolbar上
		CVSpace2* Space = m_Toolbar.HitTest(point.x,point.y);
		if(Space && Space != &m_Toolbar){
			Space->m_State |= SPACE_SELECTED;
			m_ToobarItemSelected = Space;
			Invalidate();
		}	        		
	}else {// NewSelect != m_SpaceSelected	
		
		RECT TreeBox = GetHeaderBox(NewSelect);  
		if(::PtInRect(&TreeBox,point)){
			//取消旧的选择
			if(m_SpaceSelected){ 
				m_SpaceSelected->m_AreaBottom-=m_Toolbar.m_Height;
				m_SpaceSelected->m_State &= ~SPACE_SELECTED;
				m_Toolbar.m_Owner = NULL;
				m_SpaceSelected = NULL;
			}
			ToggleSpace( NewSelect);
			return 0 ; //点击newselect treebox不影响当前选择
		}
		
		if(NewSelect->m_State & SPACE_NOTOOLABR )return 0;
		
		//取消旧的选择
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_AreaBottom-=m_Toolbar.m_Height;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			m_Toolbar.m_Owner = NULL;
			m_SpaceSelected = NULL;
		}
		//设置新选择
		m_SpaceSelected = NewSelect;
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		
		m_SpaceSelected->m_AreaBottom += m_Toolbar.m_Height; //增加高度放置toolbar
		m_Toolbar.m_Owner = m_SpaceSelected;
		Layout();	
	}
	return 0;
}



LRESULT CWSTreeView::OnSize(WPARAM wParam, LPARAM lParam) 
{
	LayoutScrollBar();
    Layout();
	return 0;
}

}//namespace VISUALSPACE
