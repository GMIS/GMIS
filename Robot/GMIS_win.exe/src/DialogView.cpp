// LogView.cpp: implementation of the CLogView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "GMIS.h"
#include "DialogView.h"
#include "UserTimer.h"
#include "MainFrame.h"
#include "LogicDialog.h"



CDialogView::InfoItem::InfoItem(int64 ID,const TCHAR* text)
:CVSpace2(ID),m_Text(text)
{	
	
};

void CDialogView::InfoItem::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){
	//输出文字
    RECT rc = GetArea();
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
		DT_NOPREFIX|DT_VCENTER);	
	::SetTextColor(hDC,Oldcr);	
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDialogView::CDialogView()
{
	m_crViewBkg =  RGB(255,255,255);
	m_HeaderWidth = 8;
	//m_State &= ~SPACE_SHOWWINBORDER;
}

CDialogView::~CDialogView()
{


}
/*
ePipeline{ 历史对话数据
	ePipeline{  m_ID=时间戳， m_Label=发言者名字 
		int32   信息状态
		string  信息文本
	} 
	...
	ePipeline{}
}	
*/
void CDialogView::SetDialogList(ePipeline& Pipe){
	SendChildMessage(GetHwnd(),SET_DIALOG,(int64)&Pipe,0);	
}

void CDialogView::AddDialogItem(ePipeline& Item){
	int64 ID = Item.GetID();
	tstring Time = AbstractSpace::GetTimer()->GetHMSM(ID);
	tstring& Name = Item.GetLabel();
	int32    State = *(int32*)Item.GetData(0);
	tstring text  = *(tstring*)Item.GetData(1);

/*
	if (Item.Size()==2)
	{
*/	
		//tstring str = tformat(_T("%s   %s\n%s"),Time.c_str(),Name.c_str(),text.c_str());

	    tstring str = Format1024(_T("%s   %s\n"),Time.c_str(),Name.c_str());
		str += text;

		InfoItem* Info = new InfoItem(ID,str.c_str()); 
		SIZE size = CalcuTextSize(str.c_str());
		size.cy+=8;
		Info->SetAreaSize(size.cx,size.cy);
		SendChildMessage(GetHwnd(),ADD_INFO,(int64)Info,0);
/*	
	}else{
		tstring str = tformat(_T("%s   %s\n%s"),Time.c_str(),Name.c_str(),text.c_str());
		ImageItem* Info   = new ImageItem(ID,str.c_str());
		SIZE  size=CalcuTextSize(str.c_str());
		size.cx=size.cx<250?250:size.cx+10;
		size.cy +=40;
		size.cy=size.cy<60?60:size.cy;
		Info->SetAreaSize(size.cx,size.cy);
		
		ePipeline* Data = (ePipeline*)Item.GetData(2);
		while(Data->Size()){
			int32 card = Data->PopInt();
			Info->m_CardList.push_back(card);
		}		
		SendChildMessage(GetHwnd(),ADD_INFO,(int64)Info,0);
	}
*/
};

void CDialogView::AddMoreDialogItem(ePipeline& Pipe){
	SendChildMessage(GetHwnd(),ADD_MORE,(int64)&Pipe,0);	
}


void CDialogView::ClearAllItem(){
	SendChildMessage(GetHwnd(),CLR_INFO,0,0);	
};

	
LRESULT CDialogView::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case ADD_INFO:
		{
			InfoItem* Info = (InfoItem*)SRM->wParam;
			PushChild(Info);
			Layout(FALSE);
			EnsureVisible(Info,FALSE); 
			break;
		}
	case CLR_INFO:	
		Clear();
		break;	
    case SET_DIALOG:
		{
			Clear();
			Layout();

			ePipeline* Pipe = (ePipeline*)SRM->wParam;
			if (Pipe->Size()==0)
			{
				return 0;
			}

			InfoItem* Info = NULL;
			for (int i=0; i<Pipe->Size(); i++)
			{
				ePipeline* Item = (ePipeline*)Pipe->GetData(i);
                int64 ID = Item->GetID();
                if (ID == -1) //生成一个more item,点击后可以得到更多历史对话记录
                {
					int32 n = Item->PopInt();
					tstring str = Format1024(_T("...More(%d)..."),n);
					Info = new InfoItem(0,str.c_str());
					SIZE size = CalcuTextSize(str.c_str());
					Info->SetAreaSize(size.cx+150,size.cy);
					PushChild(Info);
                }else{
					
					tstring Time = AbstractSpace::GetTimer()->GetHMSM(ID);
					tstring& Name = Item->GetLabel();
					int32    State = *(int32*)Item->GetData(0);
					tstring& text  = *(tstring*)Item->GetData(1);
					tstring str = Format1024(_T("%s   %s\n"),Time.c_str(),Name.c_str());
					str +=text;
					Info = new InfoItem(ID,str.c_str()); 
					SIZE size = CalcuTextSize(str.c_str());
					Info->SetAreaSize(size.cx,size.cy);
					PushChild(Info);
				}
			}
			Layout(FALSE);

			if (Info)
			{
				EnsureVisible(Info,FALSE); 
			}
		}
		break;
	case ADD_MORE:
		{
			ePipeline* Pipe = (ePipeline*)SRM->wParam;
			assert(Pipe->Size()>0);
		
			int p=0;
			int i=0;
			ePipeline* FirstItem = (ePipeline*)Pipe->GetData(i);
            if (FirstItem->GetID() == 0)
            {
				int32 n = FirstItem->PopInt();
				tstring str = Format1024(_T("...More(%d)..."),n);
				InfoItem* v = (InfoItem*)m_ChildList[0];
                v->m_Text = str;

				p = 1;  //插在more item之后
				i = 1;
            } 
            else  //取消more item
            {
				CVSpace2* v = m_ChildList[0];
                delete v;
				m_ChildList.pop_front();
            }
 
			InfoItem* Info = NULL;
			for (i; i<Pipe->Size(); i++)
			{
				ePipeline* Item = (ePipeline*)Pipe->GetData(i);
                int64 ID = Item->GetID();
                assert(ID > 0);
 				
				tstring Time = AbstractSpace::GetTimer()->GetHMSM(ID);
				tstring& Name = Item->GetLabel();
				int32    State = *(int32*)Item->GetData(0);
				tstring& text  = *(tstring*)Item->GetData(1);
				tstring str = Format1024(_T("%s   %s\n%s"),Time.c_str(),Name.c_str(),text.c_str());
				Info = new InfoItem(ID,str.c_str()); 
				SIZE size = CalcuTextSize(str.c_str());
				Info->SetAreaSize(size.cx,size.cy);
				PushChild(Info,p++);
			}
			Layout();
			CVSpace2* v = m_ChildList[0];
			EnsureVisible(v,TRUE); 
		}
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CDialogView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
 	
    POINTS* p = (POINTS*)(&lParam);

	if(m_ScrollSelected){
		//if(m_SpaceSelected)m_SpaceSelected->m_State &= ~SPACE_SELECTED;
		//m_SpaceSelected = NULL; //点击了Scrollbar不能再点击其它
		return 0;
	}

	POINT point;
	point.x =p->x;
	point.y =p->y;

	CVSpace2* SpaceSelected = Hit(point);

	if (SpaceSelected == NULL)
	{
		return 0;
	}

	if(SpaceSelected->m_Alias != 0){
		if(m_SpaceSelected)m_SpaceSelected->m_State &= ~SPACE_SELECTED;
		SpaceSelected->m_State |= SPACE_SELECTED;
		m_SpaceSelected = SpaceSelected;
	}else{
		assert(m_ChildList.size()>1);

		CVSpace2* Item = m_ChildList[1];

		ePipeline Msg(GUI_GET_DIALOG_MORE_ITME);
		Msg.PushInt(Item->m_Alias);		
	    GetGUI()->SendMsgToBrainFocuse(Msg);

	}
	Invalidate();
	return 0;
}

LRESULT CDialogView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}
	return CWSListView::Reaction(message,wParam,lParam);
}
