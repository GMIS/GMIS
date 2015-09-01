// ListView.cpp: implementation of the CListView class.
//
//////////////////////////////////////////////////////////////////////


#include "ListView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CListView::CListView()
{
	m_bUnitWidth = FALSE;
}

CListView::~CListView()
{

}
void CListView::DrawItemHeader(HDC hDC, RECT& rcClient,CVSpace2* Space) //显示tree open状态及连接线 
{
    
};
/*
把窗口转换成基于文档的坐标，去检验文档哪些部分在可见范围，然后绘制在可见范围以内的
绘制时，把可见范围以内的条目转换成窗口坐标
问题是：
1 条目还有子空间，条目坐标转了，那么子条目必须显式跟着转换，可以规定子条目的坐标是基于
		父空间的，这样子条目的绘制时，必须转换成基于父空间的坐标，这样可以自然转换。

2 如果窗口的滚动条没有任何移动，那么绘制时不需要任何坐标计算，是否能简单快速绘制？
  如果所有子条目的坐标系统统一基于窗口系统则可能，绘制时不需要任何改变。但问题是，在设计时，我们只可能
  基于父空间来设计。
 
解决方案有：
1 所有子条目坐标基于父空间来设计，但条目加入后都转换成基于窗口空间的坐标，这样一来，只要拖动了滚动棒，
就必须更新所有条目的坐标，如果条目很多可能导致滚动慢。这种设计的好处是可以记忆位于窗口的条目，
重复绘制点击测试会比较快

2 所有子条目都是基于父空间来设计，窗口只是一个移动的摄像机，绘制时，窗口转换成基于顶级父空间（文档）的坐标，
与顶级父空间下的子空间比较，确定哪些可见，可见的空间绘制时则必须转换成窗口坐标绘制，然后再把窗口转换成基于
可见空间的坐标，在于其下子空间比较，确定哪些可见，如此递归绘制完所有条目。
优点是，原理清晰，算法形式简单，2D和3D能统一，但要求所有操作都必须动态转换坐标系统，任何情况下的绘制耗费都一样。


void CListView::Draw(HDC hDC,ePipeline* Pipe){

	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);

	int32 y = GetVScrollPos();
	int32 x = GetHScrollPos();

	//转换坐标基于文档
	rcClient.left   +=x;
	rcClient.right  +=x;
	rcClient.top    +=y;
	rcClient.bottom +=y;

	deque<CVSpace2*>::iterator It = m_ChildList.begin();

	while(It<m_ChildList.end()){
		CVSpace2*  Space = *It;
		assert(Space);

//		Space->m_AreaLeft   -=x;
//		Space->m_AreaRight  -=x;
//		Space->m_AreaTop    -=y;
//      Space->m_AreaBottom -=y;

		
		if (Space->m_AreaBottom<rcClient.top || Space->m_AreaRight<rcClient.left)
		{
			It++;
			
			continue;
		}
		if (Space->m_AreaTop>rcClient.bottom || Space->m_AreaLeft > rcClient.right)
		{
			return;
		}
		

//		Space->Layout(FALSE);
		DrawItem(hDC,rcClient,Space);

//		Space->m_AreaLeft   +=x;
//		Space->m_AreaRight  +=x;
//		Space->m_AreaTop    +=y;
//		Space->m_AreaBottom +=y;

//		Space->Layout(FALSE);
		//DrawItemHeader(hDC,rcClient,Space);
		It++;
	};
}
*/


void CListView::DrawChild(CVSpace2* Parent,HDC hDC,ePipeline* Pipe){

	RECT rcClient;

	rcClient.left   = *(int32*)Pipe->GetData(0);
	rcClient.top    = *(int32*)Pipe->GetData(1);
	rcClient.right  = *(int32*)Pipe->GetData(2);
	rcClient.bottom = *(int32*)Pipe->GetData(3);
	
	deque<CVSpace2*>::iterator It = Parent->m_ChildList.begin();

	ePipeline ChildPipe;
	ChildPipe.PushInt(0);
	ChildPipe.PushInt(0);
	ChildPipe.PushInt(0);
	ChildPipe.PushInt(0);

	while(It<Parent->m_ChildList.end()){
		CVSpace2*  Space = *It;
		assert(Space);
		
		if (Space->m_AreaBottom<rcClient.top || Space->m_AreaRight<rcClient.left)
		{
			It++;
			
			continue;
		}
		if (Space->m_AreaTop>rcClient.bottom || Space->m_AreaLeft > rcClient.right)
		{
			return;
		}
       
		
		Space->DrawSelf(hDC,Pipe);
        
		if(Space->m_ChildList.size()){
            *(int32*)ChildPipe.GetData(0) = rcClient.left - Space->m_AreaLeft;
			*(int32*)ChildPipe.GetData(1) = rcClient.top  - Space->m_AreaTop;
			*(int32*)ChildPipe.GetData(2) = rcClient.right  - Space->m_AreaLeft;
			*(int32*)ChildPipe.GetData(3) = rcClient.bottom - Space->m_AreaTop;
			DrawChild(Space,hDC,&ChildPipe);
		}
		It++;
	};
}


CVSpace2* CListView::HitTestChild(CVSpace2* Parent,POINT& point){
	deque<CVSpace2*>::iterator It = Parent->m_ChildList.begin();
	while(It<Parent->m_ChildList.end()){
		CVSpace2*  Space = *It;
		assert(Space);
		RECT rc = Space->GetArea();	

		if(::PtInRect(&rc,point)){	
			
			if(Space->m_ChildList.size()){

				//转换成基于Space坐标
				POINT p = point;
				p.x -= Space->m_AreaLeft;
				p.y -= Space->m_AreaTop;
				CVSpace2* ChildSpace =	HitTestChild(Space,p);
				if(ChildSpace)return ChildSpace;
			}else{
				return Space;
			}
		}
		It++;
	};
	return NULL;
};

CVSpace2* CListView::Hit(POINT& point){
	CVSpace2* Space = NULL;
	if(Space = m_VScrollbar.HitTest(point.x,point.y))return Space;
	if(Space = m_HScrollbar.HitTest(point.x,point.y))return Space;
	
	int32 y = GetVScrollPos();
	int32 x = GetHScrollPos();

	//转换坐标基于文档
    point.x   +=x;
	point.y   +=y;

    return HitTestChild(this,point);
}

void CListView::SizeReaction(CVSpace2* Parent,deque<CVSpace2*>::iterator& ChildIt){

    CVSpace2* Child=NULL;
	CVSpace2* PreChild = NULL;
	
	if (ChildIt != Parent->m_ChildList.begin())
	{
	   PreChild = *(--ChildIt);
	   ++ChildIt;
	};

	int32 x=0, y=0;
	int32 w=0, h=0;

	int MaxWidth = Parent->AreaWidth();
	
	if (ChildIt == Parent->m_ChildList.end())//删除或首次会出现这种情况
	{
		if (PreChild)
		{
			Child = PreChild;
		}else{//此Parent删除所有Child
			Child = Parent;
		}
	}else{	
		//首先查找当前所有Child，以其中最大宽度为Parent的Size
		if (m_bUnitWidth)
		{
			MaxWidth = Child->m_AreaRight;
		} 
		else
		{			
			deque<CVSpace2*>::iterator It = Parent->m_ChildList.begin();	
			while (It<ChildIt)
			{
				Child = *It;			
				MaxWidth = max(MaxWidth,Child->m_AreaRight);
				It++;
			}
		}

		//重新设置变化点及之后所有child的坐标,基于父空间
		while (ChildIt<Parent->m_ChildList.end())
		{
			Child = *ChildIt;		
			if (PreChild)
			{
				x = PreChild->m_AreaLeft;					
				y = PreChild->m_AreaBottom;
				
			}else{
				x = m_HeaderWidth+m_Padding;
				y = Parent->m_AreaTop; 
			}
			
			w = Child->m_SizeX;
			h = Child->m_SizeY;
			
			Child->m_AreaLeft   = x;
			Child->m_AreaTop    = y+m_Padding;
			Child->m_AreaRight  = Child->m_AreaLeft+w;
			Child->m_AreaBottom = Child->m_AreaTop+h; 
			

			MaxWidth = max(MaxWidth,Child->m_AreaRight);
			
			PreChild = Child;
			ChildIt++;
		}
	}
	
	//Parent->m_SizeX = MaxWidth+50;
	Parent->m_SizeY = Child->m_AreaBottom+50;
	
	Layout();	
}

LRESULT CListView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	default:
		return CTreeView::Reaction(message,wParam,lParam);
	}
};

LRESULT CListView::OnPaint(WPARAM wParam, LPARAM lParam) 
{
	RECT rcWin; 
	GetClientRect(m_hWnd,&rcWin);

	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(rcWin.right==rcWin.left || rcWin.top==rcWin.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}
 	
	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcWin),RectHeight(rcWin));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcWin,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT);
    
	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);

	int32 y = GetVScrollPos();
	int32 x = GetHScrollPos();

	//转换坐标基于文档
	rcClient.left   +=x;
	rcClient.right  +=x;
	rcClient.top    +=y;
	rcClient.bottom +=y;

	ePipeline Pipe;
	Pipe.PushInt(rcClient.left);
	Pipe.PushInt(rcClient.top);
	Pipe.PushInt(rcClient.right);
	Pipe.PushInt(rcClient.bottom);

	DrawChild(this,DCMem,&Pipe);
    
	if(m_UseSystemScrollbar == false){
		DrawScrollbar(DCMem);
	};
	::SetBkMode(DCMem, OldMode );


	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rcWin,m_crWinBorder);
	}

	::BitBlt(hdc, 0, 0, RectWidth(rcWin), RectHeight(rcWin), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;
}