// BrainRoomView.cpp: implementation of the CBrainObjectView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)
#include "MainFrame.h"
#include "ObjectView.h"
#include "format.h"
#include "BitmapEx.h"
#include "crc32.h"
#include "StatusBar.h"
#include "WebView.h"

ObjectItem::ObjectItem(int64 ID,tstring  Name,OBJECTTYPE Type,HBITMAP hBMP,uint32 Crc32,tstring Time)
		:CVSpace2(ID),m_Type(Type),m_Name(Name),m_hBMP(hBMP),m_Crc32(Crc32),m_Time(Time)
{	
	SetIamgePos(0,0,0,0);
	m_AreaRight  = 250;
	m_AreaBottom = 75;
};

ObjectItem::~ObjectItem(){
	if (m_hBMP)
	{
		::DeleteObject(m_hBMP); 
	}
};

void ObjectItem::SetIamgePos(int32 x,int32 y, int32 w,int32 h){
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
};

void ObjectItem::Draw(HDC hDC, ePipeline* Pipe){
	
	RECT rc = GetArea();

	COLORREF crText=RGB(0x17,0x35,0x7b);
	if(m_State & SPACE_SELECTED){
        DrawEdge(hDC,&rc,RGB(128,128,128));
	}else if(m_State & SPACE_FOCUSED){
		DrawEdge(hDC,&rc,RGB(128,128,128));
	}
    InflateRect(&rc,-8,-4);

	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	
	rc.bottom = rc.top+20;

	//Label
	tstring Label = m_Name;
	Label += _T("   ");
	Label += m_Time;

	::DrawText(hDC,Label.c_str(),Label.size(),&rc,DT_LEFT|DT_VCENTER);	
  
	//坐标
    tstring PosStr = _T("undefine");
	if(m_w>0 && m_h >0){
		PosStr = tformat(_T("Pos: (%d,%d)- (%d,%d)"),m_x,m_y,m_w,m_h);
	}
	rc.top = rc.bottom;
	rc.bottom = rc.top+20;

	::DrawText(hDC,PosStr.c_str(),PosStr.size(),&rc,DT_LEFT|DT_VCENTER);	
	
	rc.top = rc.bottom;
	rc.bottom = rc.top+20;
   
	//ocr or crc
	
	if (m_OcrText.size())
	{
		tstring ocrStr = tformat(_T("OCR: %s"),m_OcrText.c_str());
		::DrawText(hDC,ocrStr.c_str(),ocrStr.size(),&rc,DT_LEFT|DT_VCENTER);	
	}else {
		tstring crcStr = tformat(_T("CRC: %x"),m_Crc32);
		::DrawText(hDC,crcStr.c_str(),crcStr.size(),&rc,DT_LEFT|DT_VCENTER);		
	}

	rc.top = rc.bottom;
	rc.bottom = m_AreaBottom;

	//图像
	if(m_hBMP){			
		HDC MemDC=CreateCompatibleDC(hDC); 
		HBITMAP Oldbitmap = (HBITMAP)SelectObject(MemDC,m_hBMP); 

		BitBlt(hDC, 
			rc.left,rc.top,
			RectWidth(rc),RectHeight(rc),
			MemDC, 
			0,0,
			SRCCOPY);
	
		::SelectObject(MemDC,Oldbitmap);
		::DeleteDC(MemDC); 
	}


	::SetTextColor(hDC,Oldcr);	

};
	

ObjectViewToolbar::ObjectViewToolbar()
:m_Owner(NULL){

};

void ObjectViewToolbar::Layout(bool Redraw /*= true*/){
	if(m_Owner==NULL)return;

	SetArea(m_Owner);
	m_AreaTop = m_AreaBottom-28;
	m_AreaLeft +=1;
	m_AreaRight -=1;
	m_AreaBottom -=4;

	int32 len = AreaWidth()-4;
	len = len/m_ChildList.size();
		
	deque<CVSpace2*>::iterator It =m_ChildList.begin();
	RECT rc = GetArea();
	rc.left += 2;
	while(It<m_ChildList.end()){
		rc.right = rc.left +len;
		CVSpace2* Space = *It;			
		Space->SetArea(rc);
		Space->m_AreaLeft +=2;
		Space->m_AreaRight-=2;
		Space->m_AreaTop+=1;
		Space->m_AreaBottom-=1;
		rc.left = rc.right;
		It++;
	}	
};

void ObjectViewToolbar::Draw(HDC hDC, ePipeline* Pipe){
	FillRect(hDC,GetArea(),RGB(198,198,255));
	CVSpace2::Draw(hDC);
};


void ToolbarBnt::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){
	
	RECT rc = GetArea();
	HFONT GuiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	
	HFONT OldFont = (HFONT)::SelectObject(hDC, GuiFont );	
	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	
	
	COLORREF crBorder = RGB(192,192,192);
	COLORREF crFkg = RGB(0,0,0);
	COLORREF crBkg = RGB(192,192,192);
	
	tstring Name = m_Name;
	
	if (m_State & SPACE_USING)
	{
		Name = _T("Cancle");
	};
	
	//画文字边框			
	//	DrawEdge(hDC,m_Area,crBorder);
    DeflateRect(&rc,1,1,1,1);
	COLORREF Oldcr; 
	if(m_State & SPACE_DISABLE){
		DrawEdge(hDC,&rc,crBorder);
		Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,Name.c_str(),Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if(m_State & SPACE_SELECTED){
		DrawEdge(hDC,&rc,crBorder);
		//FillRect(hDC,rc,crFkg);
		Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,Name.c_str(),Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if(m_State&SPACE_FOCUSED){
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,BDR_RAISEDOUTER|BDR_RAISEDINNER,BF_BOTTOMRIGHT|BF_TOPLEFT);
		
		Oldcr = ::SetTextColor(hDC,crFkg);
		rc.left-=1;
		rc.top -=1;
		
		::DrawText(hDC,Name.c_str(),Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else {
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,BDR_RAISEDOUTER|BDR_RAISEDINNER,BF_BOTTOMRIGHT|BF_TOPLEFT);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,Name.c_str(),Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);	
	}
	
    ::SelectObject(hDC,OldFont ); 
}
//CObjectView
////////////////////////////////////////////////////////////////////

CObjectView::CObjectView(){
	
	m_Outport = NULL;
    m_crWinBorder = RGB(32,32,32);
 
	m_crViewBkg = RGB(245,245,255);

    ToolbarBnt* Capture = new ToolbarBnt(OBJECT_CAPTURE,_T("Capture"));
	ToolbarBnt* Ocr = new ToolbarBnt(OBJECT_OCR,_T("OCR"));
    ToolbarBnt* Delete = new ToolbarBnt(OBJECT_DEL,_T("Delete"));
	
	m_Toolbar.PushChild(Capture);
	m_Toolbar.PushChild(Ocr);
	m_Toolbar.PushChild(Delete);

	m_ToolbarItemSelected = NULL;
};
 

void  CObjectView::Layout(bool Redraw){
	CWSListView::Layout(false);

	m_Toolbar.Layout();
	if(Redraw)Invalidate();
}

void CObjectView::Draw(HDC hDC, ePipeline* Pipe){
	CWSListView::Draw(hDC,Pipe);
	if(m_Toolbar.m_Owner)m_Toolbar.Draw(hDC,Pipe);
}

void CObjectView::DrawItemHeader( HDC hDC, CVSpace2* Space){   		 

	RECT rcHeader = GetHeaderBox(Space);
    ObjectItem*  Item = (ObjectItem*)Space;
	TCHAR buf[100];
	_stprintf(buf,_T("%3d"),(uint32)Item->m_Alias);
	RECT rc;

	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0x17,0x35,0x7b));
	::SetRect(&rc,0,rcHeader.top,rcHeader.right-2,rcHeader.bottom);
	::DrawText(hDC,buf,_tcslen(buf),&rc,DT_CENTER|DT_VCENTER|DT_NOPREFIX);
    ::SetTextColor(hDC,Oldcr);

}

RECT CObjectView::GetHeaderBox(CVSpace2* Space){
	RECT rc = Space->GetArea();
	rc.left -= m_HeaderWidth+4;
	rc.right = rc.left + m_HeaderWidth;
	return rc;
}


LRESULT CObjectView::OnAddObject(int64 wParam, int64 lParam){
	ObjectItem* Item = (ObjectItem*)wParam;
	m_ChildList.push_back(Item);
				
	Layout(true);
	return 0;
}

LRESULT CObjectView::OnSetObjectOcrTxt(int64 wParam, int64 lParam){
	int64 ID = wParam;;
    tstring* txt = (tstring*)lParam;
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while (It != m_ChildList.end())
	{
		ObjectItem* Item = (ObjectItem*)(*It);
		if(Item->m_Alias == ID){
			Item->m_OcrText = *txt;
			break;
		}
		It++;
	}
	return 0;
}

LRESULT CObjectView::OnSetObject(int64 wParam, int64 lParam){
	int64 ID = wParam;
	ePipeline* Pipe = (ePipeline*)lParam;
	
	ObjectItem* Item = (ObjectItem*)m_ChildList[ID];
	assert(Item->m_Alias == ID);
	
	Item->m_Time = Pipe->GetLabel();

	tstring& Name = *(tstring*)Pipe->GetData(1);
	int32&   Type = *(int32*)Pipe->GetData(2);
	int32      x = *(int32*)Pipe->GetData(3);
	int32      y = *(int32*)Pipe->GetData(4);
	int32      w = *(int32*)Pipe->GetData(5);
	int32      h = *(int32*)Pipe->GetData(6);
	uint32& Crc32 = *(uint32*)Pipe->GetData(7);
	tstring ocrtxt = *(tstring*)Pipe->GetData(8);
    uint32 Handle = *(uint32*)Pipe->GetData(9);


	HBITMAP hBitmap = (HBITMAP)Handle;

	if (Item->m_hBMP)
	{
		::DeleteObject(Item->m_hBMP);
		Item->m_hBMP = NULL;
	}

	CBitmapEx Image;
	Image.Load(hBitmap);
	HBITMAP hBitmap1 = NULL;
	Image.Save(hBitmap1);

	Item->m_hBMP = hBitmap1;
	
	if (Name.size())
	{
		Item->m_Name = Name;
	}else{
		Name = Item->m_Name;
	}
	
	Item->m_Crc32 = Crc32;
	Item->m_OcrText = ocrtxt;
	
	Item->SetIamgePos(x,y,w,h);
	
	int32 AreaWidth = max(250,w+16);  //最小180
	int32 AreaHeight = m_SpaceSelected==Item?h+95:h+75;//需要为toolbar留出位置
	Item->SetAreaSize(AreaWidth,AreaHeight); 
	
	Layout(true);

	return 0;
}

	
LRESULT CObjectView::OnDeleteObject(int64 wParam, int64 lParam){
	int64 ID = wParam;
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	LRESULT ret = 0;
	while(It < m_ChildList.end()){
		ObjectItem* Item = (ObjectItem*)(*It);
		if(Item->m_Alias == ID){               
			//delete Item;
			//Item = NULL;
			//It = m_ChildList.erase(It);
			::DeleteObject(Item->m_hBMP);
			Item->m_hBMP = NULL;
            Item->m_AreaBottom -= Item->m_h;
			Item->m_w = 0;
			Item->m_h = 0;
			ret = 1;
			break;
		}
		It++;
	}
	Layout(true);
	return ret;
}

LRESULT CObjectView::OnClearAll(int64 wParam, int64 lParam){

	m_HotSpaceList.clear();
	DeleteAll();
	Layout(true);
	return 0;
}

LRESULT CObjectView::OnFindObject(int64 wParam, int64 lParam){
	ePipeline& ObjectData = *(ePipeline*)lParam;

	int32 ID = wParam;
	assert(ID>=0 && ID<m_ChildList.size());
    deque<CVSpace2*>::iterator It = m_ChildList.begin();
	//while (It != m_ChildList.end())
	{
	    ObjectItem* Item = (ObjectItem*)m_ChildList[ID];
		assert(Item->m_Alias == wParam);
		{
			ObjectData.PushInt(wParam);
			ObjectData.PushString(Item->m_Name);
			ObjectData.PushInt(Item->m_Type);
			ObjectData.PushInt(Item->m_x);
			ObjectData.PushInt(Item->m_y);
			ObjectData.PushInt(Item->m_w);
			ObjectData.PushInt(Item->m_h);
			ObjectData.PushInt(Item->m_Crc32);
			ObjectData.PushString(Item->m_OcrText);
			ObjectData.PushInt((uint32)Item->m_hBMP);			
		}
	//	It++;
	}
	return 0;
}
	

LRESULT CObjectView::ToolbarReaction(ToolbarBnt* Bnt){
   
	ObjectItem* Item = (ObjectItem*)m_SpaceSelected;

	switch(Bnt->m_Alias)
	{
		case OBJECT_DEL:
		{
			RemoveChild(Item);
			m_Toolbar.m_Owner = NULL;
			Layout();
			tstring Name = Item->m_Name;
			SendParentMessage(CM_DELETE_OBJECT,(int64)&Name,0,NULL);			
		}
		break;
		case OBJECT_CAPTURE:
		{
			if (Bnt->m_State & SPACE_USING)
			{
				Bnt->m_State &=~ SPACE_USING;
				SendParentMessage(CM_CAPTURE_OBJECT,Item->m_Alias,0,NULL);			
			} 
			else
			{
				Bnt->m_State |=SPACE_USING;
				SendParentMessage(CM_CAPTURE_OBJECT,Item->m_Alias,1,NULL);			
			}
		}
		break;
		case OBJECT_OCR:
		{
			SendParentMessage(CM_OCR_OBJECT,Item->m_Alias,0,NULL);			
		}
 	}
	return 0;
}

LRESULT CObjectView::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	
	LRESULT ret = CWSScrollView::OnLButtonUp(wParam,lParam);

	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	
	CVSpace2* NewSelect = Hit(point);
	
	if(NewSelect &&  NewSelect == m_SpaceSelected){		
		RECT rc = m_SpaceSelected->GetArea();
		rc.bottom-=20;    //排除toolbar再次点击在Item上则取消选择
		if(::PtInRect(&rc,point)){
			return 0;
		}
		//点击发生在toolbar上
		ToolbarBnt* ToolItem = (ToolbarBnt*)m_Toolbar.HitTest(point.x,point.y);
		if(ToolItem && ToolItem == m_ToolbarItemSelected){
           ToolbarReaction(ToolItem);
		}	 
		
	}
	
	if (m_ToolbarItemSelected)
	{
		::ReleaseCapture();

		m_ToolbarItemSelected->m_State &= ~SPACE_SELECTED;
		m_ToolbarItemSelected = NULL;
		Invalidate();
	}
	return 0;
}
LRESULT CObjectView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
 	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;

	if(m_ScrollSelected){
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_AreaBottom-=20;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			//m_Toolbar.m_Owner = NULL;
			m_SpaceSelected = NULL;
		}
		return 0;
	}

	CVSpace2* NewSelect = Hit(point);

	if(NewSelect ==NULL)return 0; 

	if(NewSelect == m_SpaceSelected){		

		//点击发生在item?
		RECT rc = m_SpaceSelected->GetArea();
		rc.bottom-=20; //排除toolbar再次点击在Item上则取消选择
		if(::PtInRect(&rc,point)){
			m_SpaceSelected->SetArea(rc);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			m_SpaceSelected = NULL;
			m_Toolbar.m_Owner = NULL;
			Layout();
			return 0;
		}
		//点击发生在toolbar上
		ToolbarBnt* ToolItem = (ToolbarBnt*)m_Toolbar.HitTest(point.x,point.y);
		if(ToolItem){
			::SetCapture(m_hWnd);

			m_ToolbarItemSelected = ToolItem;
			m_ToolbarItemSelected->m_State |= SPACE_SELECTED;
			Invalidate();
		}	        		
	}else{// NewSelect != m_SpaceSelected	
		
		//取消旧的选择
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_AreaBottom-=20;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			m_Toolbar.m_Owner = NULL;
			m_SpaceSelected = NULL;
		}
		//设置新选择
		m_SpaceSelected = NewSelect;
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		m_SpaceSelected->m_AreaBottom += 20; //增加高度放置toolbar
		m_Toolbar.m_Owner = m_SpaceSelected;
		SendParentMessage(CM_SET_CUR_OBJECT,m_SpaceSelected->m_Alias,0,NULL);		
		Layout();		
	}
	return 0;
}

LRESULT CObjectView::ChildReaction(SpaceRectionMsg* srm){
	switch(srm->Msg)
	{
	case OBJECT_ADD:
        OnAddObject(srm->wParam,srm->lParam);
		break;
	case OBJECT_DEL:
		OnDeleteObject(srm->wParam,srm->lParam);
		break;
	case OBJECT_FIND:
		OnFindObject(srm->wParam,srm->lParam);
		break;
	case OBJECT_CLR:
		OnClearAll(srm->wParam,srm->lParam);
	    break;
	case OBJECT_SET:
		OnSetObject(srm->wParam,srm->lParam);
	    break;
	case OBJECT_OCR_TXT:
		OnSetObjectOcrTxt(srm->wParam,srm->lParam);
	}
	return 0;
}


LRESULT CObjectView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}
	else if (message == WM_LBUTTONUP)
	{
		return OnLButtonUp(wParam,lParam);
	}
	else return CWSListView::Reaction(message,wParam,lParam);
}


