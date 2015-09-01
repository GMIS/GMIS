// GuiGadget.cpp: implementation of the GuiGadget class.
//
//////////////////////////////////////////////////////////////////////
#include "SystemSetting.h"
#include "GuiGadget.h"

void ImageBorderItem::Draw(HDC hDC, ePipeline* Pipe){

	if (m_Image)
	{
		FillTile(hDC,m_Image,GetArea());

		int x1 = AreaWidth()+m_AreaLeft;
		int y1 = AreaHeight()+m_AreaTop;

		RECT rc;
		if(m_State & SPACE_FOCUSED){
            int w = AreaWidth();
			int h = AreaHeight();
			if(w>h){
              rc.top = m_AreaTop-1;
			  rc.bottom = m_AreaBottom+1;
			  for(int x=m_AreaLeft; x<x1; x+=90){
			      rc.left = x+30;
				  rc.right = x+60;
				  FillRect(hDC,rc, RGB(255,0,0));
				  ::DrawEdge(hDC,&rc,EDGE_SUNKEN,BF_RECT);
			  }
			}else {
              rc.left = m_AreaLeft-1;
			  rc.right = m_AreaRight+1;
			  for(int y=m_AreaTop; y<y1; y+=90){
			      rc.top = y+30;
				  rc.bottom = y+60;
				  FillRect(hDC,rc, RGB(255,0,0));
				  ::DrawEdge(hDC,&rc,EDGE_SUNKEN,BF_RECT);
			  }
			}
		}	
		for(int i=0; i<m_ChildList.size(); i++){
			m_ChildList[i]->Draw(hDC);
		}
	}else{
		BorderItem::Draw(hDC,Pipe);
	}
	

}
ImageTitleBar::ImageTitleBar(int64 ID, HANDLE Image):
ImageBorderItem(ID,Image){

};

ImageTitleBar::ImageTitleBar(int64 ID, COLORREF crActive,COLORREF crInActive,COLORREF crBorder):
ImageBorderItem(ID,NULL){
	m_crActivate = crActive;
	m_crInactivate = crInActive;
	m_crBorder   = crBorder;
};

void ImageTitleBar::Draw(HDC hDC, ePipeline* Pipe){
	if (m_Image)
	{
		FillTile(hDC,m_Image,GetArea());

	}else{
		//BorderItem::Draw(hDC);
		FillRectGlass(hDC,GetArea(),m_crActivate);
	//	DrawEdge(hDC,GetArea(),m_crBorder);
	}
	
	for(int i=0; i<m_ChildList.size(); i++){
		m_ChildList[i]->Draw(hDC);
	}    
}


ControlBnt::ControlBnt(int32 ID,const TCHAR* text,
		      bool IsMutilLine /*=false*/)
		:ButtonItem(ID,text,NULL,false){
		if(IsMutilLine)m_State |= SPACE_MUTILLINETEXT;
};
void ControlBnt::Draw(HDC hDC, ePipeline* Pipe){
	
    COLORREF crText       = SS.m_crBntFg;

 	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	

	HFONT OldFont = (HFONT)::SelectObject(hDC,SS.m_SystemFont);	

	RECT rc = GetArea();

	if(m_State & SPACE_SELECTED){
		FillRectGlass(hDC,&rc,RGB(0,127/3,255/2),FALSE,80);
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		Draw3DEdge(hDC,&rc);
	}
    else if(m_State & SPACE_WARNING || m_State & SPACE_FOCUSED){	
		FillRectGlass(hDC,&rc,RGB(0,127/3,255/2),FALSE,80);
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		//Draw3DEdge(hDC,&rc);
	}
	else {
		//Draw3DEdge(hDC,&rc);
		//::InflateRect(&rc,-1,-1);
		FillRectGlass(hDC,&rc,RGB(0,0,20),FALSE,80);
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}	

	::SelectObject(hDC,OldFont ); 

}

CAddNewBnt::CAddNewBnt(int32 ID,const TCHAR* text,
		      bool IsMutilLine /*=false*/)
		:ControlBnt(ID,text,IsMutilLine){
};
void CAddNewBnt::Draw(HDC hDC, ePipeline* Pipe){
	
	COLORREF crBorder    = SS.m_crBntBorder;
    COLORREF crFkg       = RGB(255,255,255);
	COLORREF crBkg       = SS.m_crBntBg;
	COLORREF crFocused   = SS.m_crBntFocused;

 	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	

	HFONT OldFont = (HFONT)::SelectObject(hDC,SS.m_Font12B);	

	RECT rc = GetArea();

	if(m_State & SPACE_SELECTED){
		FillRectGlass(hDC,&rc,RGB(0,127/3,255/2),FALSE,80);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		Draw3DEdge(hDC,&rc);
	}
    else if(m_State & SPACE_WARNING || m_State & SPACE_FOCUSED){	
		FillRectGlass(hDC,&rc,RGB(0,127/3,255/2),FALSE,80);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		//Draw3DEdge(hDC,&rc);
	}
	else {
		//Draw3DEdge(hDC,&rc);
		//::InflateRect(&rc,-1,-1);
		FillRectGlass(hDC,&rc,RGB(0,0,20),FALSE,80);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}	

	::SelectObject(hDC,OldFont ); 

}

MainBnt::MainBnt(int32 ID,const TCHAR* text,bool IsMutilLine /*=false*/)
		:ButtonItem(ID,text,NULL,false){
		if(IsMutilLine)m_State |= SPACE_MUTILLINETEXT;
};
void MainBnt::Draw(HDC hDC, ePipeline* Pipe){
    COLORREF crText       = SS.m_crBntFg;

 	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	

	HFONT OldFont = (HFONT)::SelectObject(hDC,SS.m_SystemFont);	

	COLORREF Oldcr; 

	RECT rc = GetArea();
 
	if(m_State & SPACE_SELECTED){
		Draw3DEdge(hDC,&rc);
  	    ::InflateRect(&rc,-1,-1);
       //::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
		FillRectGlass(hDC,&rc,RGB(0,150,0),FALSE,80);
		Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);
		::SetTextColor(hDC,Oldcr);	
	}
	else {
		if(m_State & SPACE_WARNING || m_State & SPACE_FOCUSED){	
			//Draw3DEdge(hDC,&rc);
			//::InflateRect(&rc,-1,-1);
			FillRectGlass(hDC,&rc,RGB(0,150,0),FALSE,80);
			Oldcr = ::SetTextColor(hDC,crText);
			::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);
			::SetTextColor(hDC,Oldcr);
		}else{
			//Draw3DEdge(hDC,&rc);
			//::InflateRect(&rc,-2,-2);
			FillRectGlass(hDC,&rc,RGB(0,20,0),FALSE,80);
			COLORREF Oldcr = ::SetTextColor(hDC,RGB(192,192,192));
			::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
			::SetTextColor(hDC,Oldcr);
		}
	}

	::SelectObject(hDC,OldFont ); 
}
	
SeparatorLine::SeparatorLine(int32 ID,const TCHAR* text,int32 nline /*= 1*/)
		:ButtonItem(ID,text,NULL,false),m_NumLine(nline){

	m_crDark  = RGB(13,13,13);
	m_crLight = RGB(120,120,120);
};
void SeparatorLine::Draw(HDC hDC, ePipeline* Pipe){
	
	RECT rc = GetArea();
	int32 pading = 2;
	if (m_bVerDrawText)
	{
		rc.top   +=pading;
		rc.bottom-=pading;

		int w = m_NumLine*(4);

		rc.left  = rc.left+(RectWidth(rc)-w)/2;
		
		for (int i=0; i<m_NumLine; i++)
		{
			rc.right = rc.left+1;
			//::DrawEdge(hDC,&rc,BDR_RAISEDINNER,BF_RECT);	
			FillRect(hDC,rc,m_crDark);
			rc.left = rc.right;
            rc.right = rc.left+1;
	        FillRect(hDC,rc,m_crLight);
			rc.left = rc.right+2;
		}
		

	}else{
		rc.left   += pading;
		rc.right  -=pading;

		int h = m_NumLine*4;

		int d = (RectHeight(rc)-h)/2;
		rc.top  = rc.top+d;
		
		for (int i=0; i<m_NumLine; i++)
		{
			rc.bottom = rc.top+1;
			//::DrawEdge(hDC,&rc,BDR_RAISEDINNER,BF_RECT);
			FillRect(hDC,rc,m_crDark);
			//rc.top = rc.bottom;
            //rc.bottom = rc.top+1;
	        //FillRect(hDC,rc,RGB(64,64,64));
			rc.top = rc.bottom;
            rc.bottom = rc.top+1;
	        FillRect(hDC,rc,m_crLight);
			rc.top = rc.bottom+2;
		}
	}	
}



VertiacalProgressItem::VertiacalProgressItem(){
	m_Per = 0;	
	m_crProgress = RGB(0,255,0);
	m_crBk = RGB(0,128,0);
};

VertiacalProgressItem::~VertiacalProgressItem(){

};

void VertiacalProgressItem::Draw(HDC hDC, ePipeline* Pipe){

	int w = AreaWidth();
	int h = AreaHeight();
    if(h<=0)return;
 	int i;
	int32 Pading = 1;
	int32 ItemHeight = 2;
  	
	int n ;
	
	h -=25;
	n = h/(Pading+ItemHeight); //要位置的进度条个数
	
	int n2 = (n*m_Per+50)/100; //当前进度所需绘制的条数
	int n1 = n-n2;        //剩余条数，与前一个绘制方法不同
	
	RECT rc;
	rc.left = m_AreaLeft;
	rc.right = m_AreaRight;
	rc.top = m_AreaTop+5;

	//先绘制不处于进度范围内的（暗绿色）
	for (i=0; i<n1; i++)
	{ 
		rc.bottom = rc.top+ItemHeight;
		FillRect(hDC,rc,m_crBk);
		rc.top = rc.bottom+Pading; 
	}
	
	HBRUSH hBrush = CreateSolidBrush(m_crProgress);
	//绘制当前进度，明绿色
	for (i=0; i<n2; i++)
	{ 
		rc.bottom = rc.top+ItemHeight;
		::FillRect(hDC,&rc,hBrush);
		rc.top = rc.bottom+Pading; 
	}
	DeleteObject(hBrush);

	rc.bottom = m_AreaBottom;
	tstring buf= Format1024(_T("%d%%"),m_Per);
	COLORREF crOld = ::SetTextColor(hDC,m_crProgress);
	::DrawText(hDC,buf.c_str(),buf.size(),&rc,DT_CENTER|DT_VCENTER);	
	::SetTextColor(hDC,crOld);

	//中间画一黑色背景竖线把进度条分成两栏，而不用独立绘制
    rc.top = m_AreaTop+5;
	rc.bottom -=20;
	rc.left = m_AreaLeft+w/2;
	rc.right = rc.left+1;
    FillRect(hDC,rc,RGB(0,0,0));
	
};


VertiacalProgressItem1::VertiacalProgressItem1(){
	m_IsBottomToTop = true;
	m_Per = 0;	
	m_crProgress = RGB(0,0xbf,0xff);
	m_crBk = RGB(0,0x8b,0x8b);
};

VertiacalProgressItem1::~VertiacalProgressItem1(){

};

void VertiacalProgressItem1::Draw(HDC hDC, ePipeline* Pipe){

	int w = AreaWidth();
	int h = AreaHeight();

	int i;
	
	int32 Pading = 1;
	int32 ItemSize = w;

	int n ;
    RECT rc;

	n = h/(Pading+ItemSize); //要位置的进度条个数
	
	int n1 = (n*m_Per+50)/100; //当前进度所需绘制的条数
	int n2 = n-n1;        //剩余条数，与前一个绘制方法不同
	
	rc.left = m_AreaLeft;
	rc.right = m_AreaRight;
	rc.top = m_AreaTop;

	COLORREF crDraw  = m_crBk;

	if (m_IsBottomToTop)
	{
		//先绘制不处于进度范围内的（暗色）
		for (i=0; i<n2; i++)
		{ 
			rc.bottom = rc.top+ItemSize;
			FillRect(hDC,&rc,crDraw);
			Draw3DEdge(hDC,rc);
			//DrawEdge(hDC,&rc,crDraw);
			rc.top = rc.bottom+Pading; 
		}
		HBRUSH hBrush = CreateSolidBrush(m_crProgress);
		SelectObject(hDC,hBrush);
		for (i=0; i<n1; i++)
		{ 
			rc.bottom = rc.top+ItemSize;
			FillRect(hDC,&rc,m_crProgress);
			Draw3DEdge(hDC,rc);
			rc.top = rc.bottom+Pading; 
		}	
       
	}else{ //right to left


		for (i=0; i<n1; i++)
		{ 
			rc.bottom = rc.top+ItemSize;
			FillRect(hDC,&rc,m_crProgress);
			Draw3DEdge(hDC,rc);
			rc.top = rc.bottom+Pading; 
		}	
		
		//再绘制不处于进度范围内的（暗色）
		for (i=0; i<n2; i++)
		{ 
			rc.bottom = rc.top+ItemSize;
			FillRect(hDC,&rc,crDraw);
			Draw3DEdge(hDC,rc);
			rc.top = rc.bottom+Pading; 
		}
	}	
};

HorizontalProgressItem::HorizontalProgressItem(){
	m_IsLeftToRight = true;
	m_Per = 0;	
	m_crProgress = RGB(0,255,0);
};

HorizontalProgressItem::~HorizontalProgressItem(){

};

void HorizontalProgressItem::Draw(HDC hDC, ePipeline* Pipe){

	int w = AreaWidth();
	int h = AreaHeight();
    if(w<=0)return;
 	int i;
    int16 Pattern[8];
	for (i = 0; i < 8; i++)Pattern[i] = (int16)(0x5555 << (i & 1));
	HBITMAP Bitmap = ::CreateBitmap(4, 4, 1, 1, &Pattern);
	if (Bitmap == NULL)return;
	
	 HBRUSH HalfBrush = ::CreatePatternBrush(Bitmap);
	::DeleteObject(Bitmap);
	
	if (HalfBrush == NULL)return;
	

	int32 Pading = 1;
	int32 ItemSize = h;

	int n ;
    RECT rc;

	n = w/(Pading+ItemSize); //要位置的进度条个数
	
	int n1 = (n*m_Per+50)/100; //当前进度所需绘制的条数
	int n2 = n-n1;        //剩余条数，与前一个绘制方法不同
	
	rc.top = m_AreaTop;
	rc.bottom = m_AreaBottom;
	rc.left = m_AreaLeft;
	COLORREF crOld = ::SetTextColor(hDC,RGB(0,128,0));
	
	if (m_IsLeftToRight)
	{
		HBRUSH hBrush = CreateSolidBrush(m_crProgress);
		HBRUSH hOldBrush= (HBRUSH)SelectObject(hDC,hBrush);
		for (i=0; i<n1; i++)
		{ 
			rc.right = rc.left+ItemSize;
			::FillRect(hDC,&rc,hBrush);
			DrawEdge(hDC,rc,RGB(128,128,128));
			rc.left = rc.right+Pading; 
		}	

		(HBRUSH)SelectObject(hDC,HalfBrush);
		//先绘制不处于进度范围内的（暗绿色）
		for (i=0; i<n2; i++)
		{ 
			rc.right = rc.left+ItemSize;
			::FillRect(hDC,&rc,HalfBrush);
			DrawEdge(hDC,rc,RGB(128,128,128));
			rc.left = rc.right+Pading; 
		}
		::SelectObject(hDC, hOldBrush);	
		DeleteObject(hBrush);

	}else{ //right to left

		HBRUSH hOldBrush= (HBRUSH)SelectObject(hDC,HalfBrush);
		//先绘制不处于进度范围内的（暗绿色）
		for (i=0; i<n2; i++)
		{ 
			rc.right = rc.left+ItemSize;
			::FillRect(hDC,&rc,HalfBrush);
			DrawEdge(hDC,rc,RGB(128,128,128));
			rc.left = rc.right+Pading; 
		}
		HBRUSH hBrush = CreateSolidBrush(m_crProgress);
		SelectObject(hDC,hBrush);
		for (i=0; i<n1; i++)
		{ 
			rc.right = rc.left+ItemSize;
			::FillRect(hDC,&rc,hBrush);
			DrawEdge(hDC,rc,RGB(128,128,128));
			rc.left = rc.right+Pading; 
		}	
		::SelectObject(hDC, hOldBrush);	
		DeleteObject(hBrush);

	}	
	::DeleteObject(HalfBrush);	
};

BrainProgressStruct::BrainProgressStruct(){
	
};

BrainProgressStruct::~BrainProgressStruct(){

};
	
void BrainProgressStruct::SetPer(int32 Per){
	m_ProgressBar.m_Per = Per;
}

void BrainProgressStruct::Draw(HDC hDC, ePipeline* Pipe){
	m_ProgressBar.m_AreaLeft = m_AreaLeft+5;
	m_ProgressBar.m_AreaRight= m_AreaRight-5;
	m_ProgressBar.m_AreaTop  = m_AreaTop+5;
	m_ProgressBar.m_AreaBottom = m_AreaBottom-5;
	
	RECT rc = GetArea();
	FillRect(hDC,&rc,RGB(0,0,0));
	m_ProgressBar.Draw(hDC);

    int32 w = AreaWidth()/2;

	//::InflateRect(&rc,-1,-1);
	rc.right = rc.left+w;
	AlphaBlendGlass(hDC,rc.left,rc.top,RectWidth(rc),RectHeight(rc),RGB(255,255,255),80);
	
	rc.left = rc.right;
	rc.right = m_AreaRight;
	AlphaBlendGlass(hDC,rc.left,rc.top,RectWidth(rc),RectHeight(rc),RGB(192,192,192),80);
 	
	//rc = GetArea();
    //Draw3DEdge(hDC,&rc);
};


OtherBnt::OtherBnt(int64 ID,CVSpace2* Parent){
	 m_Alias = ID;
	 m_Parent = Parent;
     m_Image = NULL;
	 m_Name = _T("...");
};
OtherBnt::~OtherBnt(){
     
};
	
void OtherBnt::Draw(HDC hDC, ePipeline* Pipe){
    if(m_Image){
		FillTile(hDC,m_Image,GetArea());
	};
	
	RECT rc = GetArea();

	if(m_State & SPACE_FOCUSED){
      FillRectGlass(hDC,rc,RGB(0xb0,0xc4,0xde),FALSE);
	} 

	::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
	

STDToolbar::STDToolbar(HANDLE Image):
    m_BntOther(BNT_OTHER,this){
    m_Image = Image;
	m_IsVertiacal = false;
}
STDToolbar::~STDToolbar(){
  
};
		
CVSpace2*  STDToolbar::HitTest(int32 x,int32 y) //改为测试m_visibleBntList
{
	POINT point;
	point.x = x;
	point.y = y;
	RECT rc = GetArea();
	if(!::PtInRect(&rc,point))return NULL;	
	CVSpace2* Ret = NULL;
	vector<CVSpace2*>::iterator It = m_VisibleBntList.begin();
	while(It != m_VisibleBntList.end()){
		Ret = (*It)->HitTest(x,y); 	
		if(Ret)return Ret;
		It++;
	}
	return this;

}
void STDToolbar::GetInvisibleBnt(vector<CVSpace2*>& InvisibleList){
    CVSpace2* Space = m_VisibleBntList.back();
	if(Space != &m_BntOther)return ;

	int n = m_VisibleBntList.size()-1;
    for(int i=n; i<m_ChildList.size(); i++){
         CVSpace2* Space = m_ChildList[i];
		 InvisibleList.push_back(Space);
	}
};

void STDToolbar::Layout(bool Redraw){
   RECT rc = GetArea();

   int w = RectWidth(rc);
   int h = RectHeight(rc);
   int pading = 2;
   m_VisibleBntList.clear();
   if (m_IsVertiacal)
   {
       rc.left  = m_AreaLeft;
	   rc.right = m_AreaRight;
	   rc.top   = m_AreaTop+3;
	   for(int i=0; i<m_ChildList.size(); i++){
           CVSpace2* Space = m_ChildList[i];
		   rc.bottom = rc.top + Space->m_SizeY;
           Space->SetArea(rc);
		   rc.top = rc.bottom+pading;

		   if(rc.top<m_AreaBottom){
		       m_VisibleBntList.push_back(Space);
		   }else{
               CVSpace2* PreSpace = m_VisibleBntList.back();
			   m_VisibleBntList.pop_back();
			   rc = PreSpace->GetArea();
			   m_BntOther.SetArea(rc);
			   m_VisibleBntList.push_back(&m_BntOther);
			   break;
		   }
	   }
   }else{
       rc.top  = m_AreaTop;
	   rc.bottom = m_AreaBottom;
	   rc.left   = m_AreaLeft+3;
	   for(int i=0; i<m_ChildList.size(); i++){
           CVSpace2* Space = m_ChildList[i];
		   rc.right = rc.left + Space->m_SizeX;
		
		   if(rc.right<=m_AreaRight){
	           Space->SetArea(rc);
		       m_VisibleBntList.push_back(Space);
			   rc.left = rc.right+pading;
		   }else{
			   rc.right = m_AreaRight;
			   w = RectWidth(rc);
			   if(w<12 && m_VisibleBntList.size()){
                   CVSpace2* PreSpace = m_VisibleBntList.back();
				   m_VisibleBntList.pop_back();
				   rc = PreSpace->GetArea();
				   rc.right = m_AreaRight;
				   w = RectWidth(rc);
				   if(w>20){
					  rc.right = rc.left+20;
				   }
				   m_BntOther.SetArea(rc);
			   }
			   else m_BntOther.SetArea(rc);
			   m_VisibleBntList.push_back(&m_BntOther);
			   break;
		   }
	   }
   }
}

void STDToolbar::Draw(HDC hDC, ePipeline* Pipe){
	if(m_Image)FillTile(hDC,m_Image,GetArea());
	RECT rc = GetArea();

	rc.bottom +=2;
	rc.right = rc.left+2;
	DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
	rc.left = rc.right+1;
	rc.right = rc.left+2;
//	DrawEdge(hDC,&rc,BDR_RAISEDINNER,BF_RECT);

	for(uint32 i=0; i<m_VisibleBntList.size(); i++){
		m_VisibleBntList[i]->Draw(hDC);
	}
};



//3D PART
///////////////////////////////////////////////////////////////////////
