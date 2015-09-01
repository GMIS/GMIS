// Splitter.cpp: implementation of the CSplitter class.
//
//////////////////////////////////////////////////////////////////////

#include "Splitter.h"

namespace VISUALSPACE{

void SplitterStruct::Reset()                   //每个View均分OwnerSpace面积     
{
	if(m_ChildList.size()==0)return;
	if(m_ChildList.size()==1){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[0];
		if(Splitter->m_RightView==NULL){
			assert(Splitter->m_LeftView != NULL);
			Splitter->m_LeftView->SetArea(this);
						
            ::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,m_AreaLeft,m_AreaTop,AreaWidth(),AreaHeight(),SWP_SHOWWINDOW);
			//此时Splitter没必要显示
			Splitter->SetArea(m_AreaRight,m_AreaTop,m_AreaRight,m_AreaBottom);
			

			return;
		}
	}
	
	int32 ViewCount = m_ChildList.size()+1;
	/*奇怪,为什么如此直接计算会得到奇怪的结果,难道是编译器BUG?
	  int d = (RectHeight(m_m_Area)-m_SplitterWidth*m_ChildList.size())/ViewCount;
      经检查map deque string都有同样的问题	
		string str("a");
		int n = -4*str.size()/2;   //n=2147483646
		int n = -3*str.size()/3;   //n=1431655764
		int n = -3*str.size()/1;   //n=-3
	*/
	int l = m_ChildList.size()*SplitterWidth;
	
	if (IsVertical)
	{
		int d = (AreaWidth()-l) / ViewCount;//每个View平分剩下的面积
		
		int32 Left = m_AreaLeft;
		if(d<=0){ //表示仅仅Splitterbar已经挤满了m_Area,现在要避免超出m_Area
			for(uint32 i=0; i<m_ChildList.size(); i++){
				SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
				int32 x = Left ;	
				Splitter->m_LeftView->SetArea(x,m_AreaTop,x,m_AreaBottom);		
				RECT rcView = Splitter->m_LeftView->GetArea();
				::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);

				int32 x2 = x+SplitterWidth;
				if(x2>m_AreaRight){
					x2 = m_AreaRight;
				}			
				Splitter->SetArea(x,m_AreaTop,x2,m_AreaBottom);
				Splitter->m_BeginPos = x;
				Splitter->m_EndPos   = x2;
				Left = x2;
			}
			SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
			LastSplitter->m_RightView->SetArea(m_AreaRight,
				LastSplitter->m_AreaTop, 
				m_AreaRight,
				LastSplitter->m_AreaBottom);
			RECT rcView = GetArea();
            ::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);
		
		}
		else{
			for(uint32 i=0; i<m_ChildList.size(); i++){
				SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
				int32 x = Left + i*(d+SplitterWidth);
				
				Splitter->m_LeftView->SetArea(x,m_AreaTop,x+d,m_AreaBottom);		
				RECT rcView = Splitter->m_LeftView->GetArea();
				::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);

				x+=d;
				Splitter->SetArea(x,m_AreaTop,x+SplitterWidth,m_AreaBottom);
				Splitter->m_BeginPos = Splitter->m_LeftView->m_AreaLeft;
				Splitter->m_EndPos   = Splitter->m_AreaRight+d;
			}   
			SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
			LastSplitter->m_RightView->SetArea(LastSplitter->m_AreaRight,
				m_AreaTop, 
				m_AreaRight,
				m_AreaBottom);
			RECT rcView = LastSplitter->m_RightView->GetArea();
            ::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);

		}
	}else{ //水平放置
		int d = (AreaHeight()-l) / ViewCount;//每个View平分剩下的面积
		
		int32 Top = m_AreaTop;
		if(d<=0){ //表示仅仅Splitterbar已经挤满了m_Area,现在要避免超出m_Area
			for(uint32 i=0; i<m_ChildList.size(); i++){
				SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
				int32 y = Top ;	
				Splitter->m_LeftView->SetArea(m_AreaLeft,y,m_AreaRight,y);		
				RECT rcView = Splitter->m_LeftView->GetArea();
				::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);
				
				int32 y2 = y+SplitterWidth;
				if(y2>m_AreaBottom){
					y2 = m_AreaBottom;
				}			
				Splitter->SetArea(m_AreaLeft,y,m_AreaRight,y2);
				Splitter->m_BeginPos = y;
				Splitter->m_EndPos   = y2;
				Top = y2;
			}
			SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
			LastSplitter->m_RightView->SetArea(m_AreaLeft,
				LastSplitter->m_AreaBottom, 
				m_AreaRight,
				LastSplitter->m_AreaBottom);
			RECT rcView = LastSplitter->m_RightView->GetArea();
            ::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);

		}
		else{
			for(uint32 i=0; i<m_ChildList.size(); i++){
				SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
				int32 y = Top + i*(d+SplitterWidth);
				
				Splitter->m_LeftView->SetArea(m_AreaLeft,y,m_AreaRight,y+d);		
				RECT rcView = Splitter->m_LeftView->GetArea();
				::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);
				
				y+=d;
				Splitter->SetArea(m_AreaLeft,y,m_AreaRight,y+SplitterWidth);
				Splitter->m_BeginPos = rcView.top;
				Splitter->m_EndPos   = Splitter->m_AreaBottom+d;
			}   
			SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
			LastSplitter->m_RightView->SetArea(m_AreaLeft,
				LastSplitter->m_AreaBottom, 
				m_AreaRight,
				m_AreaBottom);
			RECT rcView = LastSplitter->m_RightView->GetArea();
            ::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);

		}
	}
}
void SplitterStruct::Hide(){
	if (m_ChildList.size()==0)
	{
		return;
	}
	for(uint32 i=0; i<m_ChildList.size(); i++){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
		::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
	}
	SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
    if (LastSplitter->m_RightView)
    {
		::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
    }
}
void SplitterStruct::Layout(RECT& rcClient)    //改变rcClient后调用此函数按比例调整各view 
{
//	
	if(m_ChildList.size()==0){
		SetArea(rcClient);
		return;
	}
	if(m_ChildList.size()==1){
	    SplitterItem* Splitter = (SplitterItem*)m_ChildList[0];
		if(Splitter->m_RightView==NULL){
			assert(Splitter->m_LeftView != NULL);
			Splitter->m_LeftView->SetArea(
				rcClient.left,  
		        rcClient.top, 
			    rcClient.right,
			    rcClient.bottom);
			RECT rcView = Splitter->m_LeftView->GetArea();
			::SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);

			//此时Splitter没必要显示
            Splitter->SetArea(rcClient.right,rcClient.top,rcClient.right,rcClient.bottom);   
			SetArea(rcClient);
			return;
		}
	}

	if (IsVertical)
	{
		if((uint32)(rcClient.right-rcClient.left)<= SplitterWidth*m_ChildList.size()){
		    m_AreaRight = m_AreaLeft;
            Hide();
			return;
		}

        int w =  AreaWidth();
		if(w ==0){
			SetArea(rcClient);
			Reset();
			return;
		}
		int w0 =m_ChildList.size()*SplitterWidth; //这部分不参与缩放
		float f = (float)(RectWidth(rcClient)-w0)/(float)(w-w0);
		if (f<0)return;
		
		int32 Left = rcClient.left;
		SplitterItem* PreSplitter = NULL;
		for(uint32 i=0; i<m_ChildList.size(); i++){
			SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
			
			CWinSpace2* View    = Splitter->m_LeftView;
			w = Splitter->m_AreaLeft-Splitter->m_BeginPos;
			View->m_AreaTop    = rcClient.top;
			View->m_AreaBottom = rcClient.bottom;
			View->m_AreaLeft   = Left;
			View->m_AreaRight  = Left+ (uint32)((float)w*f);
			RECT rcView = View->GetArea();
			if(RectWidth(rcView)< SplitterWidth){
				View->m_AreaRight = View->m_AreaLeft;
				::SetWindowPos(View->GetHwnd(),NULL,0,0,0,0,0);
			}else{
				::SetWindowPos(View->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),0);
			}
		                
			Splitter->SetArea(View->m_AreaRight,rcClient.top,View->m_AreaRight+SplitterWidth,rcClient.bottom);   

			Splitter->m_BeginPos = View->m_AreaLeft;
			if(PreSplitter){
				PreSplitter->m_EndPos = Splitter->m_AreaLeft;
			}
			PreSplitter = Splitter;
			Left =Splitter->m_AreaRight; 
		}    
		
		SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
		LastSplitter->m_RightView->SetArea(LastSplitter->m_AreaRight,
			rcClient.top,
			rcClient.right,
			rcClient.bottom);
		RECT rcView = LastSplitter->m_RightView->GetArea();
		if(RectWidth(rcView) < SplitterWidth){
				LastSplitter->m_LeftView->m_AreaRight = rcClient.right-SplitterWidth;
			    LastSplitter->SetArea(LastSplitter->m_LeftView->m_AreaRight,rcClient.top,rcClient.right,rcClient.bottom);   	
			    ::SetWindowPos(LastSplitter->m_LeftView->GetHwnd(),NULL,LastSplitter->m_LeftView->m_AreaLeft,
								LastSplitter->m_LeftView->m_AreaTop,
								LastSplitter->m_LeftView->AreaWidth(),
								LastSplitter->m_LeftView->AreaHeight(),SWP_SHOWWINDOW);
				::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
		}else{
			::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);
		}
		LastSplitter->m_EndPos   = rcClient.right;
		
	}else{ //水平放置
        if((uint32)(rcClient.bottom-rcClient.top)<=SplitterWidth*m_ChildList.size()){
			m_AreaBottom = m_AreaTop;
			Hide();
			return;
		}

		int h = AreaHeight();
		if( h==0 ){
			SetArea(rcClient);
			Reset();
			return;
		}
        int h0 =m_ChildList.size()*SplitterWidth; //这部分不参与缩放
		float f = (float)(RectHeight(rcClient)-h0)/(float)(h-h0);
        if (f<0)return;

		int32 Top = rcClient.top;
		SplitterItem* PreSplitter = NULL;
		for(uint32 i=0; i<m_ChildList.size(); i++){
			SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
	
			CWinSpace2* View = Splitter->m_LeftView;
			h = Splitter->m_AreaTop-Splitter->m_BeginPos;

			View->m_AreaTop = Top;
			View->m_AreaBottom = Top+(uint32)((float)h*f);
			View->m_AreaLeft  = rcClient.left;
			View->m_AreaRight = rcClient.right;
			RECT rcView = View->GetArea();
				
			if(RectHeight(rcView) < 16){ //最小窗口
				View->m_AreaBottom = View->m_AreaTop; 
				::SetWindowPos(View->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
			}else{
			    ::SetWindowPos(View->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);
			}	
			
			Splitter->SetArea(rcClient.left,View->m_AreaBottom,rcClient.right,View->m_AreaBottom+SplitterWidth);   
	
			Splitter->m_BeginPos = View->m_AreaTop;
			if(PreSplitter){
				PreSplitter->m_EndPos = Splitter->m_AreaTop;
			}
			PreSplitter = Splitter;
			Top =Splitter->m_AreaBottom; 
		}    
		
		
		SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();
		LastSplitter->m_RightView->SetArea(rcClient.left,
			LastSplitter->m_AreaBottom, 
			rcClient.right,
			rcClient.bottom);
 		RECT rcView = LastSplitter->m_RightView->GetArea();

		if(RectHeight(rcView) < 16){
				LastSplitter->m_LeftView->m_AreaBottom = rcClient.bottom-SplitterWidth;
			    LastSplitter->SetArea(rcClient.left,LastSplitter->m_LeftView->m_AreaBottom,rcClient.right,rcClient.bottom);   	
			    ::SetWindowPos(LastSplitter->m_LeftView->GetHwnd(),NULL,
					LastSplitter->m_LeftView->m_AreaLeft,LastSplitter->m_LeftView->m_AreaTop,
					LastSplitter->m_LeftView->AreaWidth(),
					LastSplitter->m_LeftView->AreaHeight(),SWP_SHOWWINDOW);
			    ::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
		}else{
			::SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,rcView.left,rcView.top,RectWidth(rcView),RectHeight(rcView),SWP_SHOWWINDOW);
		}
		LastSplitter->m_EndPos   = rcClient.bottom;
	}
	
	SetArea(rcClient);
}
void SplitterStruct::DrawTracker(CVSpace2* Space){
	assert(OwnerWin->GetHwnd());
	HDC hDC = ::GetWindowDC(OwnerWin->GetHwnd());
    
    HBRUSH HalfBrush = NULL;
    int16 Pattern[8];
	for (int i = 0; i < 8; i++)Pattern[i] = (int16)(0x5555 << (i & 1));
	HBITMAP Bitmap = ::CreateBitmap(4, 4, 1, 1, &Pattern);
	if (Bitmap != NULL)
	{
		HalfBrush = ::CreatePatternBrush(Bitmap);
		::DeleteObject(Bitmap);
	}
	
	if (HalfBrush != NULL){
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,HalfBrush);
		if(SplitterWidth>16){ //画矩形
            RECT rc = Space->GetArea();
			rc.right=rc.left+2;
			::PatBlt(hDC,rc.left, rc.top, RectWidth(rc), RectHeight(rc), PATINVERT);
            rc.right = Space->m_AreaRight;
			rc.left = rc.right-2;
			::PatBlt(hDC,rc.left, rc.top, RectWidth(rc), RectHeight(rc), PATINVERT);
            rc = Space->GetArea();
			rc.bottom = rc.top+2;
			::PatBlt(hDC,rc.left, rc.top, RectWidth(rc), RectHeight(rc), PATINVERT);
            rc.bottom = Space->m_AreaBottom;
			rc.top = rc.bottom-2;
			::PatBlt(hDC,rc.left, rc.top, RectWidth(rc), RectHeight(rc), PATINVERT);
		}else{
			::PatBlt(hDC,Space->m_AreaLeft, Space->m_AreaTop, Space->AreaWidth(), Space->AreaHeight(), PATINVERT);
		}
		
		::SelectObject(hDC, hOldBrush);
		::DeleteObject(HalfBrush);	
	}
	::ReleaseDC(OwnerWin->GetHwnd(),hDC);
    OwnerWin->Invalidate();
}

void SplitterStruct::SetFirstSplitterPos(int32 FirsPos){
	if (m_ChildList.size()==0)
	{
		return;
	}
	SplitterItem* Item = (SplitterItem*)m_ChildList[0];
    if (Item->m_AreaLeft == Item->m_AreaRight || Item->m_AreaTop == Item->m_AreaBottom)
    {
		return;
    }

	TempDragItem.SetArea(Item);
	TempDragItem.m_BeginPos = Item->m_BeginPos;
	TempDragItem.m_EndPos   = Item->m_EndPos;

	SplitterItem* SItem = &TempDragItem;

	if (IsVertical)
	{
		int32 x = FirsPos;
		SItem->m_AreaLeft = x;
		SItem->m_AreaRight = x+ SplitterWidth;
		
		if(SItem->m_AreaLeft<SItem->m_BeginPos){
			SItem->m_AreaLeft = SItem->m_BeginPos;
			SItem->m_AreaRight= SItem->m_AreaLeft+SplitterWidth;
		}
		if(SItem->m_AreaRight>SItem->m_EndPos){
			SItem->m_AreaRight = SItem->m_EndPos;
			SItem->m_AreaLeft = SItem->m_EndPos-SplitterWidth;
		}  
	} 
	else
	{
		int32 y = FirsPos;
		SItem->m_AreaTop = y;
		SItem->m_AreaBottom = y+ SplitterWidth;
		
		if(SItem->m_AreaTop<SItem->m_BeginPos){
			SItem->m_AreaTop = SItem->m_BeginPos;
			SItem->m_AreaBottom= SItem->m_AreaTop+SplitterWidth;
		}
		if(SItem->m_AreaBottom>SItem->m_EndPos){
			SItem->m_AreaBottom = SItem->m_EndPos;
			SItem->m_AreaTop = SItem->m_EndPos-SplitterWidth;
		}  
	}
	Item->SetArea(&TempDragItem);
	Item->m_BeginPos = TempDragItem.m_BeginPos;
	Item->m_EndPos   = TempDragItem.m_EndPos;


	if (IsVertical)
    {
		SplitterItem* CurItem = Item;
		if((CurItem->m_AreaLeft -CurItem->m_BeginPos)<MinViewWidth)
		{    
			CurItem->m_AreaLeft = CurItem->m_BeginPos;
			CurItem->m_AreaRight = CurItem->m_AreaLeft +SplitterWidth;
			CurItem->m_State |= SPLITTER_CLOSE;
		}
		else if((CurItem->m_EndPos-CurItem->m_AreaRight )<MinViewWidth){
			CurItem->m_AreaLeft = CurItem->m_EndPos-SplitterWidth;
			CurItem->m_AreaRight = CurItem->m_EndPos;
			CurItem->m_State |= SPLITTER_CLOSE;				
		}
		else CurItem->m_State &= ~SPLITTER_CLOSE;
		
		//修改前后splitter的范围
		if(m_ChildList.size()>1){
			int Index =0;
			while(CurItem != m_ChildList[Index] && (uint32)Index<m_ChildList.size())++Index;
			assert((uint32)Index<m_ChildList.size());
			
			SplitterItem* PreItem = NULL;
			SplitterItem* BackItem = NULL;		          
			if(Index == 0){
				BackItem = (SplitterItem*)m_ChildList[1];
				BackItem->m_BeginPos = CurItem->m_AreaRight;
			}
			else if(Index == m_ChildList.size()-1){
				assert(m_ChildList.size()>1);
				PreItem = (SplitterItem*)m_ChildList[Index-1];
				PreItem->m_EndPos = CurItem->m_AreaLeft;
			}
			else {
				PreItem = (SplitterItem*)m_ChildList[Index-1];
				BackItem = (SplitterItem*)m_ChildList[Index+1];
				PreItem->m_EndPos = CurItem->m_AreaLeft;
				BackItem->m_BeginPos = CurItem->m_AreaRight;
			}
		}
		
    } 
    else
    {
		SplitterItem* CurItem = (SplitterItem*)Item;
		
		if((CurItem->m_AreaTop-CurItem->m_BeginPos)<MinViewWidth)
		{    
			CurItem->m_AreaTop = CurItem->m_BeginPos;
			CurItem->m_AreaBottom = CurItem->m_AreaTop + SplitterWidth;
			CurItem->m_State |= SPLITTER_CLOSE;
		}
		else if((CurItem->m_EndPos-CurItem->m_AreaBottom )<MinViewWidth){
			CurItem->m_AreaBottom = CurItem->m_EndPos;
			CurItem->m_AreaTop = CurItem->m_AreaBottom - SplitterWidth;
			CurItem->m_State |= SPLITTER_CLOSE;				
		}
		else CurItem->m_State &= ~SPLITTER_CLOSE;
		
		//修改前后splitter的范围
		if(m_ChildList.size()>1){				
			int Index =0;
			while(CurItem != m_ChildList[Index] && (uint32)Index<m_ChildList.size())++Index;
			assert((uint32)Index<m_ChildList.size());
			
			SplitterItem* PreItem = NULL;
			SplitterItem* BackItem = NULL;		          
			if(Index == 0){
				BackItem = (SplitterItem*)m_ChildList[1];
				BackItem->m_BeginPos = CurItem->m_AreaBottom;
			}
			else if(Index == m_ChildList.size()-1){
				assert(m_ChildList.size()>1);
				PreItem = (SplitterItem*)m_ChildList[Index-1];
				PreItem->m_EndPos = CurItem->m_AreaTop;
			}
			else {
				PreItem = (SplitterItem*)m_ChildList[Index-1];
				BackItem = (SplitterItem*)m_ChildList[Index+1];
				PreItem->m_EndPos = CurItem->m_AreaTop;
				BackItem->m_BeginPos = CurItem->m_AreaBottom;
			}
		}
    }
	RECT rc = GetArea();
	Layout(rc);
}

int32 SplitterStruct::GetFirstSplitterPos(){
	if (m_ChildList.size()==0)
	{
		return 0;
	}
	SplitterItem* Item = (SplitterItem*)m_ChildList[0];
    if (Item->m_AreaLeft == Item->m_AreaRight || Item->m_AreaTop == Item->m_AreaBottom)
    {
		return 0;
    }
	return Item->m_AreaLeft;

}
void SplitterStruct::OnLButtonDown(SplitterItem* Item){
	assert(OwnerWin->GetHwnd());
	::SetCapture(OwnerWin->GetHwnd());
	::SetFocus(OwnerWin->GetHwnd());
	::RedrawWindow(OwnerWin->GetHwnd(),NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);

	//不直接拖动原始Item，而把其位置信息赋予临时条目，以保证原始条目在拖动完成之前保持正确显示
    TempDragItem.SetArea(Item);
	TempDragItem.m_BeginPos = Item->m_BeginPos;
	TempDragItem.m_EndPos   = Item->m_EndPos;
    DrawTracker(&TempDragItem);	
}

void SplitterStruct::OnLButtonUp(SplitterItem* Item){
			
	::ReleaseCapture();
	Item->SetArea(&TempDragItem);
	Item->m_BeginPos = TempDragItem.m_BeginPos;
	Item->m_EndPos   = TempDragItem.m_EndPos;
	DrawTracker(Item);

    if (IsVertical)
    {
			SplitterItem* CurItem = Item;
			if((CurItem->m_AreaLeft -CurItem->m_BeginPos)<MinViewWidth)
			{    
				CurItem->m_AreaLeft = CurItem->m_BeginPos;
				CurItem->m_AreaRight = CurItem->m_AreaLeft +SplitterWidth;
				CurItem->m_State |= SPLITTER_CLOSE;
			}
			else if((CurItem->m_EndPos-CurItem->m_AreaRight )<MinViewWidth){
				CurItem->m_AreaLeft = CurItem->m_EndPos-SplitterWidth;
				CurItem->m_AreaRight = CurItem->m_EndPos;
				CurItem->m_State |= SPLITTER_CLOSE;				
			}
			else CurItem->m_State &= ~SPLITTER_CLOSE;
			
			//修改前后splitter的范围
			if(m_ChildList.size()>1){
				int Index =0;
				while(CurItem != m_ChildList[Index] && (uint32)Index<m_ChildList.size())++Index;
				assert((uint32)Index<m_ChildList.size());
				
				SplitterItem* PreItem = NULL;
				SplitterItem* BackItem = NULL;		          
				if(Index == 0){
					BackItem = (SplitterItem*)m_ChildList[1];
					BackItem->m_BeginPos = CurItem->m_AreaRight;
				}
				else if(Index == m_ChildList.size()-1){
					assert(m_ChildList.size()>1);
					PreItem = (SplitterItem*)m_ChildList[Index-1];
					PreItem->m_EndPos = CurItem->m_AreaLeft;
				}
				else {
					PreItem = (SplitterItem*)m_ChildList[Index-1];
					BackItem = (SplitterItem*)m_ChildList[Index+1];
					PreItem->m_EndPos = CurItem->m_AreaLeft;
					BackItem->m_BeginPos = CurItem->m_AreaRight;
				}
			}

    } 
    else
    {
			SplitterItem* CurItem = (SplitterItem*)Item;
		
			if((CurItem->m_AreaTop-CurItem->m_BeginPos)<MinViewWidth)
			{    
				CurItem->m_AreaTop = CurItem->m_BeginPos;
				CurItem->m_AreaBottom = CurItem->m_AreaTop + SplitterWidth;
				CurItem->m_State |= SPLITTER_CLOSE;
			}
			else if((CurItem->m_EndPos-CurItem->m_AreaBottom )<MinViewWidth){
				CurItem->m_AreaBottom = CurItem->m_EndPos;
				CurItem->m_AreaTop = CurItem->m_AreaBottom - SplitterWidth;
				CurItem->m_State |= SPLITTER_CLOSE;				
			}
			else CurItem->m_State &= ~SPLITTER_CLOSE;
			
			//修改前后splitter的范围
			if(m_ChildList.size()>1){				
				int Index =0;
				while(CurItem != m_ChildList[Index] && (uint32)Index<m_ChildList.size())++Index;
				assert((uint32)Index<m_ChildList.size());
				
				SplitterItem* PreItem = NULL;
				SplitterItem* BackItem = NULL;		          
				if(Index == 0){
					BackItem = (SplitterItem*)m_ChildList[1];
					BackItem->m_BeginPos = CurItem->m_AreaBottom;
				}
				else if(Index == m_ChildList.size()-1){
					assert(m_ChildList.size()>1);
					PreItem = (SplitterItem*)m_ChildList[Index-1];
					PreItem->m_EndPos = CurItem->m_AreaTop;
				}
				else {
					PreItem = (SplitterItem*)m_ChildList[Index-1];
					BackItem = (SplitterItem*)m_ChildList[Index+1];
					PreItem->m_EndPos = CurItem->m_AreaTop;
					BackItem->m_BeginPos = CurItem->m_AreaBottom;
				}
			}
    }
	RECT rc = GetArea();
	Layout(rc);

}

void SplitterStruct::OnMouseMove(int32 x,int32 y,SplitterItem* SItem){
	SItem = &TempDragItem;
	
	DrawTracker(SItem);
	
	if (IsVertical)
	{
		SItem->m_AreaLeft = x;
		SItem->m_AreaRight = x+ SplitterWidth;
		
		if(SItem->m_AreaLeft<SItem->m_BeginPos){
			SItem->m_AreaLeft = SItem->m_BeginPos;
			SItem->m_AreaRight= SItem->m_AreaLeft+SplitterWidth;
		}
		if(SItem->m_AreaRight>SItem->m_EndPos){
			SItem->m_AreaRight = SItem->m_EndPos;
			SItem->m_AreaLeft = SItem->m_EndPos-SplitterWidth;
		}  
	} 
	else
	{
		SItem->m_AreaTop = y;
		SItem->m_AreaBottom = y+ SplitterWidth;
		
		if(SItem->m_AreaTop<SItem->m_BeginPos){
			SItem->m_AreaTop = SItem->m_BeginPos;
			SItem->m_AreaBottom= SItem->m_AreaTop+SplitterWidth;
		}
		if(SItem->m_AreaBottom>SItem->m_EndPos){
			SItem->m_AreaBottom = SItem->m_EndPos;
			SItem->m_AreaTop = SItem->m_EndPos-SplitterWidth;
		}  
	}
	DrawTracker(SItem);
}

CWinSpace2* SplitterStruct::ReplaceView(int32 ViewID,CWinSpace2* NewView,DWORD Style/*=WS_CHILD|WS_VISIBLE*/){
    
	CWinSpace2* rtView = NULL;
	RECT rc;
	::SetRect(&rc,0,0,0,0);
	
	if (NewView->GetHwnd() == NULL)
	{
		HINSTANCE AppInstance = OwnerWin->GetHinstance();
		if(!NewView->Create(AppInstance,NULL,Style,rc,OwnerWin->GetHwnd(),(uint32)NewView->m_Alias))return NULL;		
	} 
	else
	{
		SetParent(NewView->GetHwnd(),OwnerWin->GetHwnd());
	}

	for(uint32 i=0; i<m_ChildList.size(); i++){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
        assert(Splitter->m_LeftView);
		if(Splitter->m_LeftView->m_Alias == ViewID){
			rtView = Splitter->m_LeftView;
			if(rtView == NewView)return rtView;

			Splitter->m_LeftView = NewView;
	        NewView->SetArea(rtView->GetArea());
			::MoveWindow(rtView->GetHwnd(),0,0,0,0,FALSE);
			::SetWindowPos(NewView->GetHwnd(),NULL,NewView->m_AreaLeft,NewView->m_AreaTop,NewView->AreaWidth(),NewView->AreaHeight(),SWP_SHOWWINDOW);

			break;
		}else if (Splitter->m_RightView && Splitter->m_RightView->m_Alias == ViewID)
		{
			rtView = Splitter->m_LeftView;

			if(rtView == NewView)return rtView;

			Splitter->m_LeftView = NewView;
	        NewView->SetArea(rtView->GetArea());
			::MoveWindow(rtView->GetHwnd(),0,0,0,0,FALSE);
			::SetWindowPos(NewView->GetHwnd(),NULL,NewView->m_AreaLeft,NewView->m_AreaTop,NewView->AreaWidth(),NewView->AreaHeight(),SWP_SHOWWINDOW);
			break;
		}
	}
	return rtView;
}


void SplitterStruct::AddView(CWinSpace2* View,bool bOrderInsert,DWORD Style/*=WS_CHILD|WS_VISIBLE*/) //push_back
{
	RECT rc;
	::SetRect(&rc,0,0,0,0);
	
	if (View->GetHwnd() == NULL)
	{
		HINSTANCE AppInstance = OwnerWin->GetHinstance();
	    if(!View->Create(AppInstance,NULL,Style,rc,OwnerWin->GetHwnd(),(uint32)View->m_Alias))return;		
	} 
	else
	{
		SetParent(View->GetHwnd(),OwnerWin->GetHwnd());
	}

	
	if(m_ChildList.size()==0){
		SplitterItem* Splitter = CreateSplitterItem();
    	PushChild(Splitter);
		Splitter->m_LeftView = View;
		Reset();
		return;
	}

	vector<CWinSpace2*> ViewList;
	uint32 i;
	//先把旧view集合起来,并检查是否重复插入
	for(i=0; i<m_ChildList.size(); i++){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
        assert(Splitter->m_LeftView);
		if (Splitter->m_LeftView == View)
		{
			return;
		}
		ViewList.push_back(Splitter->m_LeftView);
	}

    SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back(); 
	if(LastSplitter->m_RightView)ViewList.push_back(LastSplitter->m_RightView);
	
	if (LastSplitter->m_RightView == View)
	{
		return;
	}

	
	if (bOrderInsert)
	{
		//按ID顺序插入新view
		vector<CWinSpace2*>::iterator it = ViewList.begin();
		while(it != ViewList.end()){
			CWinSpace2* v = (*it);
			if(View->m_Alias < v->m_Alias)
			{			
				it = ViewList.insert(it,View);
				break;
			}
			it++;
		}
		if (it == ViewList.end())
		{
			ViewList.push_back(View);
		}
	}else{
		ViewList.push_back(View);
	}

	
	//预备足够的Splitter: SplitterCount = ViewCount -1
	for(i=m_ChildList.size(); i<ViewList.size()-1; i++){
		SplitterItem* Splitter = CreateSplitterItem();
		PushChild(Splitter);
	}
    
  
	//现在给每一个Splitter分配View
    for(i=0; i<m_ChildList.size(); i++){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
        Splitter->m_LeftView   = ViewList[i];
		Splitter->m_RightView = ViewList[i+1];
	}
    Reset();
}

CWinSpace2*  SplitterStruct::DeleteView(int32 ViewID){
	CWinSpace2*  View= NULL;
	if(m_ChildList.size()==0)return View;

	vector<CWinSpace2*> ViewList;
	uint32 i;
	
	//先把删除之外的view集合起来
	for(i=0; i<m_ChildList.size(); i++){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
        assert(Splitter->m_LeftView);
		if(Splitter->m_LeftView->m_Alias == ViewID){
			SetWindowPos(Splitter->m_LeftView->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
			View = (CWinSpace2*)Splitter->m_LeftView;
			continue;
		}
		ViewList.push_back(Splitter->m_LeftView);
	}
    SplitterItem* LastSplitter = (SplitterItem*)m_ChildList.back();  
	if(LastSplitter->m_RightView )
	{
		if(LastSplitter->m_RightView->m_Alias == ViewID){
			SetWindowPos(LastSplitter->m_RightView->GetHwnd(),NULL,0,0,0,0,SWP_HIDEWINDOW);
			View = LastSplitter->m_RightView;
		}	
		else ViewList.push_back(LastSplitter->m_RightView);
	}

	if(ViewList.size()==0){
		assert(m_ChildList.size()==1);
		delete m_ChildList.back();
		m_ChildList.clear();
		return View;
	}

	//删除多余的Splitter;
	uint32 n = m_ChildList.size()-(ViewList.size()-1);
	for(i=0; i<n; i++){
	   CVSpace2* Space = m_ChildList.back();
	   delete Space;
	   m_ChildList.pop_back();
	}
    
	if(ViewList.size()==1){
		assert(m_ChildList.size()==0);
		SplitterItem* Splitter =CreateSplitterItem();

		PushChild(Splitter);
		Splitter->m_LeftView = ViewList[0];
		Reset();
		return View;
	}
	//现在给每一个Splitter分配View
    for(i=0; i<m_ChildList.size(); i++){
		SplitterItem* Splitter = (SplitterItem*)m_ChildList[i];
        Splitter->m_LeftView   = ViewList[i];
		Splitter->m_RightView = ViewList[i+1];
	}
	Reset();

	return View;
} 
   
void  SplitterStruct::Clear(){
	DeleteAll();
	SetArea(0,0,0,0);
};


}//namespace VISUALSPACE