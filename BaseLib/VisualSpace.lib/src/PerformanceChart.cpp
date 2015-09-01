// PerformanceChart.cpp: implementation of the CPerformanceChart class.
//
//////////////////////////////////////////////////////////////////////

#include "PerformanceChart.h"

CPerformanceChart::Line::Line(int64 ID,CVSpace2* Parent)
:CVSpace2(ID)
{
	m_Parent = Parent;
	m_LinePen = NULL;
}
CPerformanceChart::Line::~Line()
{
	if (m_LinePen)
	{
		::DeleteObject(m_LinePen);	
	}
}

void CPerformanceChart::Line::SetPen(COLORREF cr){
	if(m_LinePen)::DeleteObject(m_LinePen);	
	m_LinePen = ::CreatePen(PS_SOLID, 1, cr);
}

void CPerformanceChart::Line::Draw(HDC hDC,ePipeline* Pipe){

	if (m_LineValue.Size()==0)
	{
		return;
	}

	HPEN oldPen = (HPEN)SelectObject(hDC,m_LinePen);

	Chart* Ch = (Chart*)m_Parent;
	RECT rc = Ch->GetArea();

	int32 h = Ch->m_MaxValue - Ch->m_MinValue;
      
	int32 v = *(int32*)m_LineValue.GetData(0);
	int   x = rc.left;
	int32 y = rc.bottom - (v- Ch->m_MinValue)*(rc.bottom-rc.top)/h;

	::MoveToEx(hDC,x, y,NULL); 

	if (m_LineValue.Size()==1)
	{
		::LineTo(hDC,x, y);
		
	}else{
		for (int i=1; i<m_LineValue.Size(); i++)
		{
			v = *(int32*)m_LineValue.GetData(i);
			x = i*Ch->m_xgap +rc.left;
			y = rc.bottom - (v- Ch->m_MinValue)*(rc.bottom-rc.top)/h;
			::LineTo(hDC,x, y);
		}
	}
	::SelectObject(hDC,oldPen);
}

CPerformanceChart::Chart::Chart()
:m_crBk(RGB(0,0,0)),
 m_crText(RGB(255,255,255)),
 m_MinValue(0),
 m_MaxValue(100),
 m_Pading(20),
 m_LinePen(0),
 m_xgap(4)
{
	SetPen(RGB(0,100,0));
	m_Title = _T("效率");
}

CPerformanceChart::Chart::~Chart(){
	if (m_LinePen)
	{
		::DeleteObject(m_LinePen);	
	}
};

CPerformanceChart::Line* CPerformanceChart::Chart::GetLine(int64 ID){
	for (uint32 i=0; i<m_ChildList.size(); i++)
	{
		CVSpace2* L = m_ChildList[i];
		if (L->m_Alias == ID)
		{
			return (Line*)L;
		}
	}
	return NULL;
}

void CPerformanceChart::Chart::SetPen(COLORREF crPen){
	if (m_LinePen)
	{
		::DeleteObject(m_LinePen);	
	}
	m_LinePen = ::CreatePen(PS_SOLID, 1, crPen);
}

void CPerformanceChart::Chart::AddLine(int64 ID,COLORREF crPen){
	Line* Lin = GetLine(ID);
	if (Lin)
	{
		Lin->SetPen(crPen);
		return;
	}
	else{
		Lin = new Line(ID,this);
		PushChild(Lin);
        Lin->SetPen(crPen);
	}	
}

void CPerformanceChart::Chart::AppendLineData(int64 ID,ePipeline& ValueList){
	Line* l = GetLine(ID);
    if (l == NULL)
    {
		return;
    }

	RECT rc = GetArea();
	int32 w = rc.right-rc.left;
	
	if (w==0)
	{
		return;
	}

	for (int i=0; i<ValueList.Size(); i++)
	{
		int32 v = *(int32*)ValueList.GetData(i);
		if (v>m_MaxValue)
		{
			int m = v%10; 
			m_MaxValue = m==0?v:v+(10-m); //确保10的倍数
		}else if (v<m_MinValue)
		{
			int m = v%10;
			m_MinValue = m==0?v:v-m;     //确保10的倍数
		}
	}

    l->m_LineValue<<ValueList;
	int32 n = l->m_LineValue.Size() - w/m_xgap;

	if (n>0) //如果数量超过当前横轴能容纳的点，则删除最前面的
	{
		l->m_LineValue.EraseEnergy(0,n);
	}
}

void CPerformanceChart::Chart::AppendLineData(int64 ID,int32 Value){
	Line* l = GetLine(ID);
    if (l == NULL)
    {
		return;
    }
	
	RECT rc = GetArea();
	int32 w = rc.right-rc.left;
	if (w==0)
	{
		return;
	}
	
	if (Value>m_MaxValue)
	{
		int m = Value%10; 
		m_MaxValue = m==0?Value:Value+(10-m); //确保10的倍数
	}else if (Value<m_MinValue)
	{
		int m = Value%10;
		m_MinValue = m==0?Value:Value-m;     //确保10的倍数
	}
	
	
    l->m_LineValue.PushInt(Value);

	int32 n = l->m_LineValue.Size() - w/m_xgap;
	
	if (n>0) //如果数量超过当前横轴能容纳的点，则删除最前面的
	{
		l->m_LineValue.EraseEnergy(0,n);
	}
}

void CPerformanceChart::Chart::Draw(HDC hDC,ePipeline* Pipe)
{
	RECT rc = GetArea();
	FillRect(hDC,rc,m_crBk);
   
	//画横线
	//rc.left+=30; //留空输出刻度文字

	HPEN oldPen = (HPEN)SelectObject(hDC,m_LinePen);

    int y = rc.bottom;
	int32 nHLine = 0;
    while(1){
		y=y-m_Pading;
		if (y<rc.top)
		{
			break;
		}
		::MoveToEx(hDC,rc.left, y,NULL); 
		::LineTo(hDC,rc.right, y);
        nHLine ++;
	}
   
	//画竖线；
    int x = rc.left;
	int32 nVLine = 0;
	while(1){
		x = x+m_Pading;
		if(x>rc.right){
			break;
		}
		::MoveToEx(hDC,x,rc.top,NULL);
		::LineTo(hDC,x,rc.bottom);
		nVLine ++;
	}

	::SelectObject(hDC,oldPen);

	//曲线
	CVSpace2::Draw(hDC,NULL);

	//画竖轴刻度

	int32 y0=m_MinValue;
	int32 y1=m_MaxValue;
	
	int32 d = (y1-y0)/nHLine;

	rc.left = 2;
	rc.right = rc.left+30;

	COLORREF Oldcr = ::SetTextColor(hDC,m_crText);
	for (int i=0; i<nHLine; i+=2)
	{
	    rc.bottom = m_AreaBottom-i*m_Pading-1;
		rc.top   = rc.bottom -16;
	    tstring Text = Format1024(_T("%d"),y0);
		::DrawText(hDC,Text.c_str(),Text.size(),&rc,DT_LEFT);	
		y0 += d+d;
	}

	//画标题
    rc = GetArea();
    ::DrawText(hDC,m_Title.c_str(),m_Title.size(),&rc,DT_CENTER);	

	::SetTextColor(hDC,Oldcr);		

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPerformanceChart::CPerformanceChart()
{
	Chart* ch = new Chart;

	PushChild(ch);
}

CPerformanceChart::~CPerformanceChart()
{

}

CPerformanceChart::Chart* CPerformanceChart::GetChart(){
	assert(m_ChildList.size());
	Chart* ch = (Chart*)m_ChildList[0];
    return ch;	
}

void CPerformanceChart::SetTitle(tstring s){
	Chart* ch = GetChart();
    if (ch)
    {
		ch->m_Title  = s;
	}
}

void CPerformanceChart::InitRang(int low,int32 high){
	Chart* ch = GetChart();
    if (ch)
    {
		ch->m_MinValue = low;
		ch->m_MaxValue = high;
	}
}
void CPerformanceChart::SetXGap(int32 gap){
	Chart* ch = GetChart();
    if (ch)
    {
		ch->m_xgap = gap;
	}
}
void CPerformanceChart::AddLine(int64 ID, COLORREF cr){

	Chart* ch = GetChart();
    if (ch)
    {
		ch->AddLine(ID,cr);
		Invalidate();
    }
}
void CPerformanceChart::AppendLineData(int64 ID,int32 Value){
	if (GetHwnd()==NULL)
	{
		return;
	}
	ePipeline Pipe;
	Pipe.PushInt(Value);
	SendChildMessage(GetHwnd(),ADD_LINEDATA,ID,(int64)&Pipe);	


}

void CPerformanceChart::AppendLineData(int64 ID,ePipeline& ValueList){
	if (GetHwnd()==NULL)
	{
		return;
	}
	SendChildMessage(GetHwnd(),ADD_LINEDATA,ID,(int64)&ValueList);	

}

LRESULT CPerformanceChart::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case ADD_LINEDATA:
		{
			int64 ID = SRM->wParam;
			ePipeline* Pipe = (ePipeline*)SRM->lParam;

			Chart* ch = GetChart();
			if (ch)
			{
				ch->AppendLineData(ID,*Pipe);
				Invalidate();
			}
			break;
		}
	}
	return 0;
}
LRESULT CPerformanceChart::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if (message == WM_PAINT)
	{
		return OnPaint(wParam,lParam);
	}else if (message == WM_SIZE)
	{
		return OnSize(wParam,lParam);
	}else if (message == WM_ERASEBKGND)
	{
		return TRUE;
	}
	else{
		return CWinSpace2::Reaction(message,wParam,lParam);
	}
}

LRESULT CPerformanceChart::OnSize(WPARAM wParam, LPARAM lParam){
	RECT rcClient; 
	GetClientRect(m_hWnd,&rcClient);

    Chart* ch = GetChart();
	if (ch)
	{
		ch->SetArea(rcClient.left,rcClient.top,rcClient.right,rcClient.bottom);
	}
	return 0;
}

LRESULT CPerformanceChart::OnPaint(WPARAM wParam, LPARAM lParam){
	RECT rcClient; 
	
	GetClientRect(m_hWnd,&rcClient);
	
	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);
	
	if(rcClient.right==rcClient.left || rcClient.top==rcClient.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}
	
	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcClient),RectHeight(rcClient));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );
	
	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcClient,m_crViewBkg);
	}
	
	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT );
	
	Draw(DCMem);
	::SetBkMode(DCMem, OldMode );
	
	if(m_State & SPACE_SHOWCLIENTBORDER){
		DrawEdge(DCMem,rcClient,m_crWinBorder);// Border
	}		
	
	::BitBlt(hdc, 0, 0, RectWidth(rcClient), RectHeight(rcClient), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;	
}

