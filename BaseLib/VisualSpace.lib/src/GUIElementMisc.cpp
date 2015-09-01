// GUIElementMisc.cpp: implementation of some common GUI elment class.
//
//////////////////////////////////////////////////////////////////////

#include "GUIElementMisc.h"

namespace VISUALSPACE{
	
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
	
	
	ArrowItem::ArrowItem(){
		m_ArrowDirection = ARROW_DOWN;      
		m_crFg   = RGB(0,220,0);
		m_crBk   = RGB(245,245,243);
		m_crEdge = RGB(100,100,100);
	}
	
	ArrowItem::ArrowItem(int64 ID,ARROW_DIRECTION ArrowDir,
		COLORREF crFg,
		COLORREF crBk,
		COLORREF crEdge
		):
	CVSpace2(ID),
		m_crBk(crBk),
		m_crFg(crFg),
		m_crEdge(crEdge)
	{
		m_ArrowDirection = ArrowDir;
	}
	ArrowItem::~ArrowItem(){
	
	};

	void ArrowItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		
		int32 offset = 5;
		
		RECT rc = GetArea();	
		POINT pt;
		
		HPEN   hPen   = ::CreatePen(PS_SOLID,1,m_crFg);
		HPEN   hOldPen   = (HPEN)::SelectObject(hDC,hPen);	
		
		if(m_ArrowDirection == ARROW_RIGHT)
		{
			FillRectGlass(hDC,rc,m_crBk,FALSE);	
			pt.x = rc.right-offset;
			pt.y = rc.top+ RectHeight(rc)/2;
			DRAW_LINE_V(pt.x,pt.y,1);
			DRAW_LINE_V(pt.x-1,pt.y-1,3);
			DRAW_LINE_V(pt.x-2,pt.y-2,5);
			DRAW_LINE_V(pt.x-3,pt.y-3,7);
		}
		else if(m_ArrowDirection == ARROW_LEFT)
		{
			FillRectGlass(hDC,rc,m_crBk,FALSE);
			
			pt.x = rc.left+ offset;
			pt.y = rc.top+RectHeight(rc)/2;
			
			DRAW_LINE_V(pt.x,pt.y,1);
			DRAW_LINE_V(pt.x+1,pt.y-1,3);
			DRAW_LINE_V(pt.x+2,pt.y-2,5);
			DRAW_LINE_V(pt.x+3,pt.y-3,7);
		}
		else if(m_ArrowDirection == ARROW_DOWN)
		{
			FillRectGlass(hDC,rc,m_crBk,TRUE);
			pt.x = rc.left+ RectWidth(rc)/2;
			pt.y = rc.bottom-offset;
			
			DRAW_LINE_H(pt.x,pt.y,1);
			DRAW_LINE_H(pt.x-1,pt.y-1,3);
			DRAW_LINE_H(pt.x-2,pt.y-2,5);
			DRAW_LINE_H(pt.x-3,pt.y-3,7);
		}
		else if(m_ArrowDirection == ARROW_UP)
		{
			FillRectGlass(hDC,rc,m_crBk,TRUE);
			pt.x = rc.left+ RectWidth(rc)/2;
			pt.y = rc.top+offset;
			
			DRAW_LINE_H(pt.x,pt.y,1);
			DRAW_LINE_H(pt.x-1,pt.y+1,3);
			DRAW_LINE_H(pt.x-2,pt.y+2,5);
			DRAW_LINE_H(pt.x-3,pt.y+3,7);
		}
		DrawEdge(hDC,rc,m_crEdge);
		::SelectObject(hDC,hOldPen);	
		::DeleteObject(hPen);
	}
	
	ThumbItem::ThumbItem(){
		m_bVertical = false;      
		m_crFg   = RGB(0,200,0);
		m_crBk   = RGB(245,245,243);
		m_crEdge = RGB(100,100,100);
	}
	
	ThumbItem::ThumbItem(int64 ID,bool bVertical,
		COLORREF crFg,
		COLORREF crBk,
		COLORREF crEdge
		):
	CVSpace2(ID),
		m_crBk(crBk),
		m_crFg(crFg),
		m_crEdge(crEdge),
		m_bVertical(bVertical)
	{
		m_SizeX = 10;
		m_SizeY = 10;
	}
	ThumbItem::~ThumbItem(){
	
	};
	void ThumbItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		RECT rc = GetArea();
		int32 d=0;
		
		HPEN   hPen   = ::CreatePen(PS_SOLID,1,m_crFg);
		HPEN   hOldPen   = (HPEN)::SelectObject(hDC,hPen);	
		
		
		
		if(m_bVertical){
			FillRectGlass(hDC,rc,m_crBk,TRUE,127);
			rc.top+=1;
			rc.bottom-=1;	
			int offset = 4;
			DrawEdge(hDC,rc,m_crEdge);
			d = RectHeight(rc);
			if(d>15){ //画三条横线
				d = rc.top + d/2;
				::MoveToEx(hDC,rc.left+offset,d,NULL);
				::LineTo(hDC,rc.right-offset,d);
				
				::MoveToEx(hDC,rc.left+offset,d-3,NULL);
				::LineTo(hDC,rc.right-offset,d-3);
				
				::MoveToEx(hDC,rc.left+offset,d+3,NULL);
				::LineTo(hDC,rc.right-offset,d+3);
			}else if(d>8){ //画两条横线
				d = rc.top + d/2;
				::MoveToEx(hDC,rc.left+offset,d-2,NULL);
				::LineTo(hDC,rc.right-offset,d-2);
				
				::MoveToEx(hDC,rc.left+offset,d+2,NULL);
				::LineTo(hDC,rc.right-offset,d+2);
			}else{ //画一条横线
				d = rc.top + d/2;
				::MoveToEx(hDC,rc.left+offset,d,NULL);
				::LineTo(hDC,rc.right-offset,d);
			}
		}
		else{
			FillRectGlass(hDC,rc,m_crBk,FALSE,127);
			rc.left+=1;
			rc.right-=1;
			DrawEdge(hDC,rc,m_crEdge);
			
			int offset = 4;
			d = RectWidth(rc);
			if(d>10){ //画三条竖线
				d = rc.left + d/2;
				::MoveToEx(hDC,d,rc.top+offset,NULL);
				::LineTo(hDC,d,rc.bottom-offset);
				
				::MoveToEx(hDC,d-3,rc.top+offset,NULL);
				::LineTo(hDC,d-3,rc.bottom-offset);
				
				::MoveToEx(hDC,d+3,rc.top+offset,NULL);
				::LineTo(hDC,d+3,rc.bottom-offset);
			}else if(d>5){ //画两条竖线
				d = rc.top + d/2;
				::MoveToEx(hDC,d-2,rc.top+offset,NULL);
				::LineTo(hDC,d-2,rc.bottom-offset);
				
				::MoveToEx(hDC,d+2,rc.top+offset,NULL);
				::LineTo(hDC,d+2,rc.bottom-offset);
			}else{ //画一条竖线
				d = rc.top + d/2;
				::MoveToEx(hDC,d,rc.top+offset,NULL);
				::LineTo(hDC,d,rc.bottom-offset);
			}
		}
		::SelectObject(hDC,hOldPen);	
		::DeleteObject(hPen);	
	}   
	
	BorderItem::BorderItem(){
		
	}
	BorderItem::~BorderItem(){
	
	};

	BorderItem::BorderItem(int64 ID,COLORREF crAct,COLORREF crInac,COLORREF crFocused)
		:CVSpace2(ID),
		m_crActivate(crAct),
		m_crInactivate(crInac),
		m_crFocused(crFocused)
	{
	}
	
	void BorderItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		RECT rc = GetArea();
		if(!(m_State & SPACE_ACTIVATE)){
			FillRectGlass(hDC,rc, m_crInactivate);
			//FillRectGlass(hDC,rc,m_crActivate);
		}	
		else if(m_State & SPACE_FOCUSED){
			FillRectGlass(hDC,rc, m_crFocused);
		}	
		else{
			//FillRect(hDC,rc, m_crActivate);
			FillRectGlass(hDC,rc,m_crActivate);
		}
		
		for(uint32 i=0; i<m_ChildList.size(); i++){
			m_ChildList[i]->Draw(hDC);
		}
	}
	
	CVSpace2*  BorderItem::HitTest(int32 x, int32 y){
		POINT point;
		point.x = x;
		point.y = y;
		RECT rc = GetArea();
		if(!::PtInRect(&rc,point))return NULL;
		CVSpace2* Ret = NULL;
		for(uint32 i=0;i<m_ChildList.size(); i++){
			Ret = m_ChildList[i]->HitTest(x,y);
			if(Ret)return Ret;
		}
		return this;
	}
	
	ButtonItem::ButtonItem()
		:m_AttachSpace(NULL),
		m_Image(NULL),
		m_bVerDrawText(false){
		
	}		
	ButtonItem::~ButtonItem(){
	
	};

	ButtonItem::ButtonItem(int64 CmdID,const TCHAR* Name,CWinSpace2* Win,bool VerticalText,HANDLE Image /*=NULL*/)
		:CVSpace2(CmdID),
		m_Name(Name),
		m_AttachSpace(Win),
		m_bVerDrawText(VerticalText),
		m_Image(Image)
	{
	};
	void ButtonItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		COLORREF crBorder    = RGB(128,128,128);
		COLORREF crText      = RGB(0,0,0);
		//输出内容
		RECT rc = GetArea();
		
		int32 tx,ty,ix,iy;
		
		HFONT GuiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		LOGFONT LogFont;
		::GetObjectW(GuiFont, sizeof(LOGFONT), &LogFont);
		DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;
		
		int32 w =0;
		uint32 flag = DST_ICON;
		bool bDrawText = true;
		if(m_bVerDrawText){
			w = RectWidth(rc);
			if(m_Image==NULL)w=0;
			LogFont.lfEscapement  = 900;
			LogFont.lfOrientation = 900;		
			ix = rc.left;
			iy = rc.bottom-w;
			tx = rc.left;
			ty = iy;
			bDrawText = !(m_Image && ty-rc.top<20); //如果有图像，文字区小于20则不绘制text
		}else{
			w = RectHeight(rc);
			if(m_Image==NULL)w=0;
			ix = rc.left;
			iy = rc.top;
			rc.left+=w;
			bDrawText = !(m_Image && rc.right-rc.left<20);
		}
		
		HFONT NewFont = ::CreateFontIndirect( &LogFont );
		HFONT OldFont = (HFONT)::SelectObject(hDC, NewFont );	
		
		rc = GetArea();
		if(m_State & SPACE_DISABLE){
			COLORREF crSelectBkg = RGB(198,198,198);  	
			FillRectGlass(hDC,rc,crSelectBkg);
			::DrawEdge(hDC,&rc,BDR_SUNKENINNER|BDR_SUNKENOUTER,BF_BOTTOMRIGHT);
			flag |= DSS_DISABLED;
			crText = RGB(128,128,128);
		}
		else if(m_State & SPACE_WARNING){
			COLORREF crFkg    = RGB(0,0,0); 
			FillRectGlass(hDC,rc,crFkg);
		}
		else if(m_State & SPACE_SELECTED){
			COLORREF crSelectBkg = RGB(198,198,198); 
			FillRectGlass(hDC,rc,crSelectBkg);
		}
		else if(m_State&SPACE_FOCUSED){
			COLORREF crFkg       = RGB(155,29,155); 
			FillRectGlass(hDC,rc,crFkg);
			crText = RGB(255,255,255);
		}
		else{
			COLORREF crBkg       = RGB(255,129,255); 
			FillRectGlass(hDC,rc,crBkg);
		}
		if(m_Image){
			::DrawState(hDC,NULL,NULL,(LPARAM)m_Image,NULL,ix,iy,w,w,flag);
			if(m_bVerDrawText){
				rc.top  += w+2;
			}
			else {
				rc.left += w+2;
			}
		}
		if(bDrawText){ 
			COLORREF Oldcr = ::SetTextColor(hDC,crText);
			if(m_bVerDrawText){
				::ExtTextOut(hDC,tx,ty,ETO_CLIPPED,NULL, m_Name.c_str(),m_Name.size(), NULL);
			}else{
				::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
			}
			::SetTextColor(hDC,Oldcr);
		}	
		::SelectObject(hDC,OldFont ); 
		::DeleteObject(NewFont);
		//画文字边框			
		DrawEdge(hDC,rc, crBorder);
	}
    
	IconItem::IconItem()
		:m_hIcon(NULL)
	{
		
	}
	IconItem::IconItem(int64 ID,HICON Icon,int32 Size)
		:CVSpace2(ID)
	{
		m_SizeX = Size;
		m_SizeY = Size;
		m_hIcon = Icon;
	}
	IconItem::~IconItem(){
	
	};

	void IconItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		if(m_hIcon){
			int h = (AreaHeight()-m_SizeY)/2;
			if(h<0)h=0;
			::DrawIconEx(hDC, m_AreaLeft+h,m_AreaTop+h,m_hIcon, m_SizeX, m_SizeY, 0, 0, DI_NORMAL);
		}
	};
	
	TextItem::TextItem(){
		m_crText = RGB(255,255,255);
		m_TextFont = NULL;
	}	
	TextItem::TextItem(int64 ID,tstring Text)
		:CVSpace2(ID),
		m_Text(Text)	
	{
		m_crText=RGB(255,255,255);
		m_State |= TEXT_LEFT;
		m_TextFont = NULL;
	};
	TextItem::~TextItem(){
	
	};

	void TextItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		if(m_Text.size()){
			//COLORREF crOldBk = ::SetBkColor(hDC,RGB(255,0,0)); 
			COLORREF crOld = ::SetTextColor(hDC,m_crText);
		    HFONT OldFont = (HFONT)::SelectObject(hDC,m_TextFont);	

			RECT rc = GetArea();
			rc.left+=2;
			rc.right-=2;
			if(m_State & TEXT_RIGHT){
				::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_RIGHT|DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
			}else if (m_State & TEXT_CENTER)
			{
				::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_CENTER|DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
			}else{
				::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
			}
			::SetTextColor(hDC,crOld);
			//::SetBkColor(hDC,crOldBk);
			::SelectObject(hDC,OldFont);
		}
	}
	MaxboxItem::MaxboxItem():
	m_bMax(false),
		m_crBorder(RGB(255,255,255)),
		m_crNormal(RGB(192,192,192)),
		m_crFocused(RGB(0,255,0))
	{
		
	};
	MaxboxItem::MaxboxItem(int64 ID,
		COLORREF crBorder,
		COLORREF crNormal,
		COLORREF crFocused
		      )
			  :CVSpace2(ID),
			  m_bMax(false),
			  m_crBorder(crBorder),
			  m_crNormal(crNormal),
			  m_crFocused(crFocused)
	{
	};
	MaxboxItem::~MaxboxItem(){
	
	};
	void MaxboxItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		
		RECT rc = GetArea();	
		rc.left  += 3;
		rc.right -=3;
		rc.top   +=3;
		rc.bottom -=3;
		
		COLORREF crDraw = m_crNormal;
		if(m_State&SPACE_FOCUSED){
			crDraw = m_crFocused;
		}
		
		DrawEdge(hDC,rc,crDraw);
		DrawEdge(hDC,GetArea(),m_crBorder);
		
	}
	MinboxItem::MinboxItem():
	m_crBorder(RGB(255,255,255)),
		m_crNormal(RGB(192,192,192)),
		m_crFocused(RGB(0,255,0))
	{};
	MinboxItem::MinboxItem(int64 ID,
		COLORREF crBorder,
		COLORREF crNormal,
		COLORREF crFocused)
		:CVSpace2(ID),
		m_crBorder(crBorder),
		m_crNormal(crNormal),
		m_crFocused(crFocused)
	{
	};
	MinboxItem::~MinboxItem(){
	
	};
	void MinboxItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		RECT rc = GetArea();	
		rc.left  += 3;
		rc.right -=3;
		rc.top   +=3;
		rc.bottom -=3;
		
		COLORREF crDraw = m_crNormal;
		if(m_State&SPACE_FOCUSED){
			crDraw = m_crFocused;
		}
		
		rc.top=rc.bottom-2;
		DrawEdge(hDC,rc,crDraw);
		DrawEdge(hDC,GetArea(),m_crBorder);
	}
	
	CloseboxItem::CloseboxItem():
	    m_crBorder(RGB(255,255,255)),
		m_crNormal(RGB(192,192,192)),
		m_crFocused(RGB(0,255,0))
	{};
	CloseboxItem::CloseboxItem(int64 ID,
		COLORREF crBorder,
		COLORREF crNormal,
		COLORREF crFocused)
		:CVSpace2(ID),
		m_crBorder(crBorder),
		m_crNormal(crNormal),
		m_crFocused(crFocused)
	{
	};
	CloseboxItem::~CloseboxItem(){
	
	};

	void CloseboxItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		RECT rc = GetArea();	
		rc.left  += 3;
		rc.right -=3;
		rc.top   +=3;
		rc.bottom -=3;
		
		COLORREF crDraw = m_crNormal;
		if(m_State&SPACE_FOCUSED){
			crDraw = m_crFocused;
			m_State &=~SPACE_FOCUSED; 
		}
		
		HPEN   hPen    = ::CreatePen(PS_SOLID, 1, crDraw);
		HPEN   hOldPen = (HPEN)::SelectObject(hDC,hPen);
		
		// draw the cross
		::MoveToEx(hDC,rc.left, rc.top,NULL);
		::LineTo(hDC,rc.right-1, rc.bottom);
		::MoveToEx(hDC,rc.left+1, rc.top,NULL);
		::LineTo(hDC,rc.right, rc.bottom);
		
		::MoveToEx(hDC,rc.left, rc.bottom-1,NULL);
		::LineTo(hDC,rc.right-1, rc.top-1);
		::MoveToEx(hDC,rc.left+1, rc.bottom-1,NULL);
		::LineTo(hDC,rc.right, rc.top-1);
		
		::SelectObject(hDC,hOldPen);
		::DeleteObject(hPen);
		DrawEdge(hDC,GetArea(),m_crBorder);
	}

	DockboxItem::DockboxItem(int64 ID,COLORREF crBorder,
		COLORREF crNormal,
		COLORREF crFocused)
		:CVSpace2(ID),
		m_crBorder(crBorder),
		m_crNormal(crNormal),
		m_crFocused(crFocused),
		m_IsDock(false)
	{
	};
	DockboxItem::~DockboxItem(){
	
	};
	void DockboxItem::Draw(HDC hDC,ePipeline* Pipe){
		RECT rc = GetArea();	
		
		COLORREF crDraw = m_crNormal;
		if(m_State&SPACE_FOCUSED){
			crDraw = m_crFocused;
			m_State &=~SPACE_FOCUSED; 
		}
			
		HPEN   hPen   = ::CreatePen(PS_SOLID, 1, crDraw);
		HPEN   OldPen = (HPEN)::SelectObject(hDC,hPen);
		
		if(m_IsDock){
			RECT rcbox;
			::SetRect(&rcbox,rc.left+3,rc.top+1,rc.right-3,rc.bottom-4);
			DrawEdge(hDC,rcbox,crDraw);
			::MoveToEx(hDC,rc.right-5, rc.top+1,NULL);
			::LineTo(hDC,rc.right-5, rc.bottom-4);
			::MoveToEx(hDC,rc.right-6, rc.top+1,NULL);
			::LineTo(hDC,rc.right-6, rc.bottom-4);
			
			::MoveToEx(hDC,rc.left, rc.bottom-5,NULL);
			::LineTo(hDC,rc.right, rc.bottom-5);
			int x = rc.left+RectWidth(rc)/2;
			::MoveToEx(hDC,x, rc.bottom-4,NULL);
			::LineTo(hDC,x, rc.bottom);
		}
		else{
			RECT rcbox;
			::SetRect(&rcbox,rc.left+4,rc.top+3,rc.right-1,rc.bottom-3);
			DrawEdge(hDC,rcbox,crDraw);
			::MoveToEx(hDC,rc.left+4, rc.bottom-5,NULL);
			::LineTo(hDC,rc.right-1, rc.bottom-5);
			::MoveToEx(hDC,rc.left+4, rc.bottom-6,NULL);
			::LineTo(hDC,rc.right-1, rc.bottom-6);
			
			::MoveToEx(hDC,rc.left+4, rc.top,NULL);
			::LineTo(hDC,rc.left+4, rc.bottom);
			
			int y = rc.top+RectHeight(rc)/2;
			::MoveToEx(hDC,rc.left,y,NULL);
			::LineTo(hDC,rc.left+4,y);		
		}
		
		::SelectObject(hDC,OldPen);
		::DeleteObject(hPen);
	}
	
	SplitterItem::SplitterItem():
	m_crBorder(RGB(100,100,100)),
		m_crNormal(RGB(245,245,243)),
		m_crFocused(RGB(100,100,100)),
		m_crClose(RGB(0,0,128)),
		m_LeftView(NULL),
		m_RightView(NULL),
		m_imagSkin(NULL),
		m_BeginPos(0),
		m_EndPos(0)
	{};
	SplitterItem::SplitterItem(int64 ID,HANDLE Imag):
	CVSpace2(ID),
		m_crBorder(RGB(100,100,100)),
		m_crNormal(RGB(245,245,243)),
		m_crFocused(RGB(100,100,100)),
		m_crClose(RGB(0,0,128)),
		m_LeftView(NULL),
		m_RightView(NULL),
		m_imagSkin(Imag),
		m_BeginPos(0),
		m_EndPos(0)
	{};
	SplitterItem::SplitterItem(int64 ID,
		COLORREF crBorder,
		COLORREF crNormal,
		COLORREF crFocused,
		COLORREF crClose)
		:CVSpace2(ID),
		m_crBorder(crBorder),
		m_crNormal(crNormal),
		m_crFocused(crFocused),
		m_crClose(crClose),
		m_LeftView(NULL),
		m_RightView(NULL),
		m_imagSkin(NULL),
		m_BeginPos(0),
		m_EndPos(0)
	{
	};
	SplitterItem::~SplitterItem(){
	
	};
	void SplitterItem::Draw(HDC hDC,ePipeline* Pipe/*=NULL*/){
		if (m_imagSkin)
		{
			RECT rc = GetArea();
			FillTile(hDC,m_imagSkin,rc);
			
			int x1 = RectWidth(rc)+rc.left;
			int y1 = RectHeight(rc)+rc.top;
			
			if(m_State & SPACE_FOCUSED){
				int w = RectWidth(rc);
				int h = RectHeight(rc);
				if(w>h){
					rc.top = m_AreaTop-1;
					rc.bottom = m_AreaBottom+1;
					for(int x=m_AreaLeft; x<x1; x+=90){
						rc.left = x+30;
						rc.right = x+60;
						::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
						FillRectGlass(hDC,rc, RGB(255,0,0));
					}
				}else {
					rc.left = m_AreaLeft-1;
					rc.right = m_AreaRight+1;
					for(int y=m_AreaTop; y<y1; y+=90){
						rc.top = y+30;
						rc.bottom = y+60;
						::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
						FillRectGlass(hDC,rc, RGB(255,0,0));
					}
				}
			}	
		}else{
			RECT rc = GetArea();
			if(m_State & SPACE_FOCUSED){
				FillRect(hDC,rc,m_crFocused);
			}
			else if(m_State & SPLITTER_CLOSE){
				FillRect(hDC,rc,m_crClose);
			}
			else {
				FillRect(hDC,rc,m_crNormal);
			}
		}
		for(uint32 i=0; i<m_ChildList.size(); i++){
			m_ChildList[i]->Draw(hDC); 	
		};
	}
	

void Draw3DEdge(HDC hDC,RECT* Area){
		//3D边框
	RECT rc;

    ::CopyRect(&rc,Area);     //left-top
	rc.right = rc.left+1;
	FillRect(hDC,rc,RGB(19,19,19));
	rc.bottom = rc.top+1;
	rc.right = Area->right;
    FillRect(hDC,rc,RGB(19,19,19));

    ::CopyRect(&rc,Area);     //right-bottom
	rc.left = rc.right-1;
	FillRect(hDC,rc,RGB(137,137,137));
	rc.top = rc.bottom-1;
	rc.left = Area->left;
    FillRect(hDC,rc,RGB(137,137,137));
	/*
    ::CopyRect(&rc,Area); 
	rc.top++;
	rc.left++;
	rc.right = rc.left+1;     
	FillRect(hDC,rc,RGB(32,32,32));
	rc.bottom = rc.top+1;
	rc.right = Area->right;
    FillRect(hDC,rc,RGB(32,32,32));
 
*/

	/*
    ::CopyRect(&rc,Area); 
	rc.right--;
	rc.bottom--;
	rc.left = rc.right-1;
	FillRect(hDC,rc,RGB(96,96,96));
	rc.top = rc.bottom-1;
	rc.left = Area->left;
    FillRect(hDC,rc,RGB(96,96,96));
*/
}
void Draw3DEdge(HDC hDC,RECT& Area){
    Draw3DEdge(hDC,&Area);
}
	
	
} // namespace VISUALSPACE
