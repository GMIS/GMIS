// TreeView.cpp: implementation of the CTreeView class.
//
//////////////////////////////////////////////////////////////////////
#pragma  warning(disable:4786)


#include "TreeView.h"
#include  <algorithm>
#include "WSListView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreeView::CTreeView()
{
	m_State |= SPACE_SHOWWINBORDER;
	m_HeaderWidth = 16;
	m_crConnectingLine = RGB(128,128,128);
	m_bShowLine = true;
	m_Padding = 4;
	m_SpaceSelected = NULL;
	m_LinePen = ::CreatePen(PS_SOLID, 1, m_crConnectingLine);
	m_UseSystemScrollbar = true;
	
}

CTreeView::~CTreeView()
{
	if(m_LinePen)::DeleteObject(m_LinePen);

}

RECT CTreeView::GetHeaderBox(CVSpace2* Space){
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

void CTreeView::SizeReaction(CVSpace2* Parent,deque<CVSpace2*>::iterator& ChildIt){

    CVSpace2* Child=NULL;
	CVSpace2* PreChild = NULL;
	
	if (ChildIt != Parent->m_ChildList.begin())
	{
	   PreChild = *(--ChildIt);
	   ++ChildIt;
	};

	int32 x=0, y=0;
	int32 w=0, h=0;

	for(;;) 
	{
		int MaxWidth = Parent->AreaWidth();
		
		if (ChildIt == Parent->m_ChildList.end())//删除会出现这种情况
		{
			if (PreChild)
			{
				Child = PreChild;
			}else{//此Parent的所有Child都已删除
				Child = Parent;
			}
		}else{	
			//首先查找当前所有Child，以其中最大宽度为Parent的Size
            deque<CVSpace2*>::iterator It = Parent->m_ChildList.begin();	
			while (It<ChildIt)
			{
                Child = *It;
				if (Child->m_State & SPACE_OPEN)
				{	
					MaxWidth = max(MaxWidth,Child->m_AreaLeft+Child->m_SizeX);
				}else{
					MaxWidth = max(MaxWidth,Child->m_AreaRight);
				}
				It++;
			}
			//重新设置变化点及之后所有child的坐标,基于父空间
			while (ChildIt<Parent->m_ChildList.end())
			{
				Child = *ChildIt;		
				if (PreChild)
				{
					x = PreChild->m_AreaLeft;
					if (PreChild->m_State & SPACE_OPEN)
					{	
						y = PreChild->m_AreaTop+PreChild->m_SizeY;
					}else{
						y = PreChild->m_AreaBottom;
					}

				}else{
					x = m_HeaderWidth+m_Padding;
					if (Parent == this)
					{
						y = Parent->m_AreaTop;
					} 
					else
					{
						y = Parent->m_AreaBottom-Parent->m_AreaTop; //注意：Parent的坐标是基于其Parent的，不能直接被Child使用
					}			
				}
				
				w = Child->m_AreaRight-Child->m_AreaLeft;
				h = Child->m_AreaBottom-Child->m_AreaTop;
				
				
				Child->m_AreaLeft   = x;
				Child->m_AreaTop    = y+m_Padding;
				Child->m_AreaRight  = Child->m_AreaLeft+w;
				Child->m_AreaBottom = Child->m_AreaTop+h; 
				
				if (Child->m_State & SPACE_OPEN)
				{	
					MaxWidth = max(MaxWidth,Child->m_AreaLeft+Child->m_SizeX);
				}else{
					MaxWidth = max(MaxWidth,Child->m_AreaRight);
				}

				PreChild = Child;
				ChildIt++;
			}
		}
		
		if (Child->m_State & SPACE_OPEN)
		{
			Parent->m_SizeX = MaxWidth;
			Parent->m_SizeY = Child->m_AreaTop + Child->m_SizeY;

		}else{
			Parent->m_SizeX = MaxWidth;
			Parent->m_SizeY = Child->m_AreaBottom;
		}

		if(Parent == this){
			if (Parent != Child)
			{
				Parent->m_SizeX += 50;
				Parent->m_SizeY += 50;
			}
			Layout();
			break;
        }
		//如果Parent没有打开状态，那么现实大小未变，则不再传递给其父空间
		if(!(Parent->m_State & SPACE_OPEN)){
			break;
		}
		
		//否则把size增大引发的坐标位置的变动向根空间传递
		Child = Parent;
		Parent = Parent->m_Parent;

        Parent->m_SizeX = max(Parent->m_SizeX,MaxWidth);

		assert(Parent);
		ChildIt = find(Parent->m_ChildList.begin(),Parent->m_ChildList.end(),Child);

		if (ChildIt != Parent->m_ChildList.begin())
		{
			PreChild = *(--ChildIt);
			++ChildIt;
		}else{
			PreChild = NULL;
		}
	}
}

/*
把指定条目插入父空间，设置子空间位置坐标相对于父空间，如果是中间插入，父空间调整插入后其他child的位置坐标。
由于父空间插入后自身可能变大，它自己的父空间则重复这个过程调整其后子空间坐标，直到根空间。
NOTE: 插入之前，Child必须设置自己的size
*/
void CTreeView::Insert(CVSpace2* Parent,CVSpace2* Child,int32 Pos /*=-1*/){
    assert(Parent);
	Child = Parent->PushChild(Child);
    assert(Child);

	//Parent->m_State |= SPACE_OPEN;
	
	deque<CVSpace2*>::iterator ChildIt = Parent->m_ChildList.end();
	if (Pos !=-1)
    {
		ChildIt = Parent->m_ChildList.begin()+Pos;
    }else{
		--ChildIt;
	}

	SizeReaction(Parent,ChildIt);


};

CVSpace2* CTreeView::Delete(CVSpace2* Child){
    CVSpace2* temp = Child;
    CVSpace2* Parent = Child->m_Parent;
	assert(Parent);

	deque<CVSpace2*>::iterator ChildIt = find(Parent->m_ChildList.begin(),Parent->m_ChildList.end(),Child);
	if (ChildIt== Parent->m_ChildList.end())
    {
		return NULL;
    }else{
		ChildIt = Parent->m_ChildList.erase(ChildIt);
		if (Parent->m_ChildList.size()==0)
		{
			Parent->m_State &= ~SPACE_OPEN;
		}
	}

	SizeReaction(Parent,ChildIt);

	return temp;
}

void CTreeView::ToggleSpace( CVSpace2* Child){

	if (Child->m_ChildList.size()==0)
	{
		return;
	};

    CVSpace2* Parent = Child->m_Parent;
	assert(Parent);

	deque<CVSpace2*>::iterator ChildIt = find(Parent->m_ChildList.begin(),Parent->m_ChildList.end(),Child);
	if (ChildIt== Parent->m_ChildList.end())
    {
		return ;
    }

	int32 h=0;

	if (Child->m_State & SPACE_OPEN)
	{
		Child->m_State &= ~SPACE_OPEN;
	}else{
        Child->m_State |= SPACE_OPEN;
	};
	
	SizeReaction(Parent,ChildIt);

};

CVSpace2* CTreeView::BinaryHit(POINT& p, deque<CVSpace2*>& List,int32 begin, int32 n)
{
    if(n <= 0){
		return NULL;
	} 
    int32 m = begin+n/2;
    CVSpace2* Space = List[m];
	Space->m_AreaLeft -= m_HeaderWidth;  //包含节点按钮
	RECT rc = Space->GetArea();
	
	if (::PtInRect(&rc,p))
	{
			
		Space->m_AreaLeft += m_HeaderWidth;
		return Space;
	}
	Space->m_AreaLeft += m_HeaderWidth;

	if (Space->m_State & SPACE_OPEN)
	{
		rc.right = rc.left + Space->m_SizeX;
		rc.bottom = rc.top + Space->m_SizeY;
		if (::PtInRect(&rc,p))
		{
			p.x-=Space->m_AreaLeft; //表测试点坐标转换成基于Space的
			p.y-=Space->m_AreaTop;
			CVSpace2* ResultSpace =  BinaryHit(p,Space->m_ChildList,0,Space->m_ChildList.size());
		//	p.x+=Space->m_AreaLeft;
		//	p.y+=Space->m_AreaTop;
			return ResultSpace;
		}
	}
	
    if(p.y < Space->m_AreaTop) {
		return BinaryHit(p, List,begin, m-begin);
	}
    else{
		return BinaryHit(p, List,m+1, n-m-1+begin);
	} 
}

CVSpace2* CTreeView::Hit(POINT& point){

//	if(!::PtInRect(&GetArea(),point))return NULL; 错误原因：GetArea()坐标是相对于父空间
	CVSpace2* Space = NULL;
	if(Space = m_VScrollbar.HitTest(point.x,point.y))return Space;
	if(Space = m_HScrollbar.HitTest(point.x,point.y))return Space;

	int32 y = GetVScrollPos();
	int32 x = GetHScrollPos();
    point.x +=x;
	point.y +=y;
    Space = BinaryHit(point, m_ChildList,0, m_ChildList.size());
	return Space;	
}

void CTreeView::DrawItemHeader(HDC hDC,RECT& rcClient, CVSpace2* Space)//显示tree open状态及连接线 
{

	RECT rcTreeBox = GetHeaderBox(Space);

	int32 HalfTreeBoxWidth = RectWidth(rcTreeBox)/2;
	POINT Center;
	Center.x =  rcTreeBox.left+HalfTreeBoxWidth;
	Center.y =  rcTreeBox.top+HalfTreeBoxWidth;
	
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
	if(m_bShowLine && Space->m_State&SPACE_OPEN && Space->m_ChildList.size()>0 ){

		//连接自己所有Child的竖线
		CVSpace2* LastChild = Space->m_ChildList.back();

		LastChild->m_AreaLeft   += Space->m_AreaLeft;
		LastChild->m_AreaTop    += Space->m_AreaTop;
		LastChild->m_AreaRight  += Space->m_AreaLeft;
		LastChild->m_AreaBottom += Space->m_AreaTop;

		RECT rcChildTreeBox = GetHeaderBox(LastChild);
		int32 x = Center.x;
		int32 y1 = rcTreeBox.bottom;		
		int32 y2 =  rcChildTreeBox.top+HalfTreeBoxWidth;
		
		::MoveToEx(hDC,x, y1,NULL); 
		::LineTo(hDC,x, y2);	
		
		LastChild->m_AreaLeft   -= Space->m_AreaLeft;
		LastChild->m_AreaTop    -= Space->m_AreaTop;
		LastChild->m_AreaRight  -= Space->m_AreaLeft;
		LastChild->m_AreaBottom -= Space->m_AreaTop;
		
		//横线
		deque<CVSpace2*>::iterator It = Space->m_ChildList.begin();
		while (It != Space->m_ChildList.end())
		{
			CVSpace2* ChildSpace = *It;
		
			ChildSpace->m_AreaLeft   += Space->m_AreaLeft;
			ChildSpace->m_AreaTop    += Space->m_AreaTop;
			ChildSpace->m_AreaRight  += Space->m_AreaLeft;
			ChildSpace->m_AreaBottom += Space->m_AreaTop;

            RECT rcChildTreeBox = GetHeaderBox(ChildSpace);
            int32 y = rcChildTreeBox.top+HalfTreeBoxWidth;
			int32 x1 = Center.x;
			int32 x2 = rcChildTreeBox.left;

			::MoveToEx(hDC,x1, y,NULL); 
			::LineTo(hDC,x2, y);

			ChildSpace->m_AreaLeft   -= Space->m_AreaLeft;
			ChildSpace->m_AreaTop    -= Space->m_AreaTop;
			ChildSpace->m_AreaRight  -= Space->m_AreaLeft;
			ChildSpace->m_AreaBottom -= Space->m_AreaTop;
			It++;
		}

	}

    ::SelectObject(hDC,OldPen);	
};

void CTreeView::DrawItem(HDC hDC, RECT& rcClient,CVSpace2* Space)
{
    //绘制自身，这里Space未必位于rcClient，可能只是其Child位于rcClient
	if (Space->m_AreaTop>rcClient.bottom ||
		Space->m_AreaBottom<rcClient.top ||
		Space->m_AreaRight<rcClient.left || 
		Space->m_AreaLeft>rcClient.right)
	{
		//draw nothing
	}else{
		Space->Draw(hDC);

	}

    DrawItemHeader(hDC,rcClient,Space);

	//绘制其Child
	if (Space->m_State & SPACE_OPEN && Space->m_ChildList.size())
	{

		deque<CVSpace2*>::iterator It = Space->m_ChildList.begin();
		while (It != Space->m_ChildList.end())
		{
			CVSpace2* ChildSpace = *It;

			//把Child的坐标转换成基于父空间的，而父空间已经被转换成基于窗口屏幕的
			ChildSpace->m_AreaLeft   += Space->m_AreaLeft;
			ChildSpace->m_AreaTop    += Space->m_AreaTop;
			ChildSpace->m_AreaRight  += Space->m_AreaLeft;
			ChildSpace->m_AreaBottom += Space->m_AreaTop;

			DrawItem(hDC,rcClient,ChildSpace);

			ChildSpace->m_AreaLeft   -= Space->m_AreaLeft;
			ChildSpace->m_AreaTop    -= Space->m_AreaTop;
			ChildSpace->m_AreaRight  -= Space->m_AreaLeft;
			ChildSpace->m_AreaBottom -= Space->m_AreaTop;
			
			It++;
		}
		

	}

}
void CTreeView::Draw(HDC hDC,ePipeline* Pipe){

	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);

	int32 y = GetVScrollPos();
	int32 x = GetHScrollPos();

	deque<CVSpace2*>::iterator It = m_ChildList.begin();

	while(It<m_ChildList.end()){
		CVSpace2*  Space = *It;
		assert(Space);

		//转换坐标基于窗口屏幕
		Space->m_AreaLeft   -=x;
		Space->m_AreaRight  -=x;
		Space->m_AreaTop    -=y;
        Space->m_AreaBottom -=y;

		//如果一个空间的实际大小在当前Client之外则不用绘制
		if (Space->m_State & SPACE_OPEN)
		{
			if (Space->m_AreaTop+Space->m_SizeY<rcClient.top || Space->m_AreaLeft+Space->m_SizeX<rcClient.left)
			{
				It++;
				
				Space->m_AreaLeft   +=x;
				Space->m_AreaRight  +=x;
				Space->m_AreaTop    +=y;
				Space->m_AreaBottom +=y;
				
				continue;
			}
			
			if (Space->m_AreaTop>rcClient.bottom || Space->m_AreaLeft > rcClient.right)
			{
				Space->m_AreaLeft   +=x;
				Space->m_AreaRight  +=x;
				Space->m_AreaTop    +=y;
				Space->m_AreaBottom +=y;
				return;
			}

		}else{			
			if (Space->m_AreaBottom<rcClient.top || Space->m_AreaRight<rcClient.left)
			{
				It++;
				
				Space->m_AreaLeft   +=x;
				Space->m_AreaRight  +=x;
				Space->m_AreaTop    +=y;
				Space->m_AreaBottom +=y;
				
				continue;
			}
			if (Space->m_AreaTop>rcClient.bottom || Space->m_AreaLeft > rcClient.right)
			{
				Space->m_AreaLeft   +=x;
				Space->m_AreaRight  +=x;
				Space->m_AreaTop    +=y;
				Space->m_AreaBottom +=y;
				return;
			}
		}
		DrawItem(hDC,rcClient,Space);

		Space->m_AreaLeft   +=x;
		Space->m_AreaRight  +=x;
		Space->m_AreaTop    +=y;
		Space->m_AreaBottom +=y;

		//DrawItemHeader(hDC,rcClient,Space);
		It++;
	};
}

void  CTreeView::Layout(bool Redraw /*=true*/){

	RECT rcViewport;
	::GetClientRect(m_hWnd,&rcViewport);
	
	if(RectWidth(rcViewport)==0 || RectHeight(rcViewport)==0)return;
	
	LayoutScrollBar();

	if(Redraw){
		Invalidate();
	}
};


void  CTreeView::EnsureVisible(CVSpace2* Space,bool FirstVisible){

	//首先确保其所有parent都是打开的，如此才能保证pNode可见
	CVSpace2* Child = Space;
	CVSpace2* Parent = Child->m_Parent;
	while(Parent && Parent != this){
		if(!(Parent->m_State & SPACE_OPEN)){
		    ToggleSpace(Parent);
		}
		Parent = Parent->m_Parent;
	}

	RECT rc ;//= m_rcClient;
	::GetClientRect(m_hWnd,&rc);
    if(RectHeight(rc)>m_SizeY){
		Invalidate();
		//UpdateWindow(m_hWnd);
		return;
	} 

    //得到此空间基于文档的坐标位置
 	int32 x=Child->m_AreaLeft;
	int32 y=Child->m_AreaTop;
	Parent = Child->m_Parent;
	
	while(Parent && Parent != this){
		x += Parent->m_AreaLeft;
		y += Parent->m_AreaTop;
		Parent = Parent->m_Parent;
	}   
    
	//转换成基于显示窗口的坐标位置
    int32 xPos = x-GetHScrollPos();
	int32 yPos = y-GetVScrollPos();

	if(FirstVisible){	 //确保条目在左上角	
	    if(xPos + RectWidth(rc) > m_SizeX){ 
			xPos = (m_SizeX -RectWidth(rc)-xPos);
		}
		
		if(yPos + RectHeight(rc) > m_SizeY){ 
			yPos = (m_SizeY -RectHeight(rc)-yPos);
		}
	}
	else if(xPos>0 && xPos<RectWidth(rc)&& yPos>0 && yPos<RectHeight(rc) ){
        //已在可见区域保持不变
		return;
	}
	else{//确保在中间显示
		int32 w = RectWidth(rc)/2;
		int32 h = RectHeight(rc)/2;
	
		xPos = xPos-w;
		yPos = yPos-h;
	    
		if(xPos + Space->AreaWidth() > m_SizeX){ 
			xPos = (m_SizeX -Space->AreaWidth()-xPos);
		}
		
		if(yPos + Space->AreaHeight() > m_SizeY){ 
			yPos = (m_SizeY -Space->AreaHeight()-yPos);
		}	
	}

	//再转回基于文档的坐标位置
    xPos = xPos+GetHScrollPos();
	yPos = yPos+GetVScrollPos();

	//设置新的坐标
	SetHScrollPos(xPos,false);
	SetVScrollPos(yPos,true);
}



SIZE CTreeView::CalcuTextSize(const TCHAR* Text,HFONT Font){
	HDC		hDC	    = ::GetDC(m_hWnd);
	int		Saved	= ::SaveDC(hDC);
	HFONT	OldFont	= (HFONT)::SelectObject(hDC, Font);

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


LRESULT CTreeView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONDBLCLK:
		return OnLButtonDBCLK(wParam,lParam);
	case WM_SIZE:
		return OnSize(wParam,lParam);
	default:
		return CWSScrollView::Reaction(message,wParam,lParam);
	}
}

LRESULT CTreeView::OnLButtonDBCLK(WPARAM wParam, LPARAM lParam) 
{
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam); 	
    POINTS* p = (POINTS*)(&lParam);

	if(m_ScrollSelected){
		return 0;//点击了Scrollbar不能再点击其它
	}
		
	POINT point;
	point.x = p->x;
	point.y = p->y;
	CVSpace2* NewSelect = Hit(point);
	if(NewSelect){

		RECT TreeBox = GetHeaderBox(NewSelect);  
		
		if(::PtInRect(&TreeBox,point)){
			return 0;
		}
		else{	
/*			//测试在当前节点下插入

			 ListItem* Itema = new ListItem(0,_T("test treeview insert"));
			Itema->SetAreaSize(200,20);
            Insert(NewSelect,Itema);
*/
/*          //测试删除
		    CVSpace2* s = Delete(NewSelect);
			if (m_SpaceSelected == s)
			{
				m_SpaceSelected = NULL;
			}
			delete s;
*/            //Invalidate();

			//测试可见条目
		   // EnsureVisible(m_EnsureVisble,false);
		}
	}	
	return 0;
};

LRESULT CTreeView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
 	
    POINTS* p = (POINTS*)(&lParam);


	if(m_ScrollSelected){
		return 0;//点击了Scrollbar不能再点击其它
	}
		
	POINT point;
	point.x = p->x;
	point.y = p->y;
	CVSpace2* NewSelect = Hit(point);
	if(NewSelect){

		RECT TreeBox = GetHeaderBox(NewSelect);  
		
		if(::PtInRect(&TreeBox,point)){
			//放弃焦点，以与击中Space自身区别
			//m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			ToggleSpace( NewSelect);
			//m_SpaceSelected = NULL;
		}
		else{
			if(m_SpaceSelected && m_SpaceSelected->m_State & SPACE_SELECTED){
				m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			}
		    NewSelect->m_State |= SPACE_SELECTED;
			m_SpaceSelected = NewSelect;
		    Invalidate();
		}
	}	
	return 0;
};

LRESULT CTreeView::OnSize(WPARAM wParam, LPARAM lParam) 
{
	//LayoutScrollBar();
    Layout();
	return 0;
}
	