// LinkerView.cpp: implementation of the CLinkerView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786) 

#include "GMIS.h"
#include "LinkerView.h"
#include "MainFrame.h"
#include "LogicDialog.h"


CLinkerView::CLinkerItem::CLinkerItem(int64 ID,const tstring& Name)
:CTreeItem(ID,Name.c_str())
{

}

void CLinkerView::CLinkerItem::Draw(HDC hDC,ePipeline* Pipe ){
	//输出文字
    RECT rc = GetArea();
	
	if (m_State & SPACE_SELECTED)
	{
		FillRect(hDC,rc,RGB(0,0,255));
		
		COLORREF Oldcr = ::SetTextColor(hDC,RGB(255,255,255));
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);
			
		::SetTextColor(hDC,Oldcr);	
	}else if(m_State & SPACE_DISABLE)
	{
		FillRect(hDC,rc,RGB(128,128,128));

		COLORREF Oldcr = ::SetTextColor(hDC,RGB(255,255,255));
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);

		::SetTextColor(hDC,Oldcr);	
	}
	else if (m_State & SPACE_WARNING)
	{
		FillRect(hDC,rc,RGB(255,0,0));
		COLORREF Oldcr = ::SetTextColor(hDC,RGB(255,255,255));
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);
		
		::SetTextColor(hDC,Oldcr);	
	}
	else{
		
		COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);
		
		::SetTextColor(hDC,Oldcr);
	}
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLinkerView::CLinkerView()
{

    m_crViewBkg     = RGB(255,255,255);   
	m_DialogCounter = 0;

	m_Toolbar.m_Height = 0;  //目前没有工具条
};

CLinkerView::~CLinkerView()
{

};

tstring CLinkerView::GetDefaultName(){
	tstring Name = Format1024(_T("Dialog(%d)"),++m_DialogCounter);
	return Name;
}


CLinkerView::CLinkerItem* CLinkerView::FindLinker(int64 SourceID){
	deque<CVSpace2*>::iterator it = m_ChildList.begin();
	while (it != m_ChildList.end())
	{
		CVSpace2* v = *it;
		if (v->m_Alias == SourceID)
		{
			return (CLinkerItem*)v;
		}
		it++;
	}
	return NULL;
}


void CLinkerView::AddDialog(int64 SourceID,int64 DialogID,int64 ParentID,tstring Name){
	
	if(Name.size()==0){
 		Name =GetDefaultName();
	};

	CLinkerItem* LinkerItem = new CLinkerItem(DialogID,Name); 
   
    SIZE s = CalcuTextSize(Name.c_str());
	s.cx = s.cx<100?100:s.cx;
	
	LinkerItem->SetAreaSize(s.cx,s.cy);

	ePipeline  Param;
	Param.PushInt(SourceID);
	Param.PushInt(ParentID);
	SendChildMessage(GetHwnd(),ADD_DIALOG,(int64)&Param,(int64)LinkerItem);	
};

void CLinkerView::DeleteDialog(int64 SourceID,int64 DialogID){
	SendChildMessage(GetHwnd(),DEL_DIALOG,SourceID,DialogID);	
};

void CLinkerView::NotifyDialogHasNew(int64 SourceID,int64 DialogID){
	SendChildMessage(GetHwnd(),NOTIFY_DIALOG,SourceID,DialogID);	
}

void CLinkerView::EnableDialog(int64 SourceID,int64 DialogID,bool bEnable){
	CLinkerItem* LinkerItem = FindLinker(SourceID);
	if (DialogID == DEFAULT_DIALOG)
	{
		if(bEnable){
			LinkerItem->m_State &= ~SPACE_DISABLE;
		}else{
			LinkerItem->m_State |= SPACE_DISABLE;
		}
	}else {
		CLinkerItem* DialogItem = (CLinkerItem*)LinkerItem->FindSpace(DialogID);
		if (DialogItem)
		{
			if(bEnable){
				DialogItem->m_State &= ~SPACE_DISABLE;
			}else{
				DialogItem->m_State |= SPACE_DISABLE;
			}
		}
	}
	Invalidate();
}
void CLinkerView::ClearAll(){
	SendChildMessage(GetHwnd(),CLR_LINKER,0,0);	
};

CLinkerView::CLinkerItem* CLinkerView::GetCurDialog(){
	return (CLinkerItem*)m_SpaceSelected;
};

void  CLinkerView::SetCurDialog(int64 SourceID, int64 DialogID){

	SendChildMessage(GetHwnd(),SET_FOCUSDIALOG,SourceID,DialogID);	
}

LRESULT CLinkerView::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{

	case ADD_DIALOG:
		{
			CLinkerItem* DialogItem = (CLinkerItem*)SRM->lParam;
			ePipeline& Param = *(ePipeline*)SRM->wParam;
			
			int64 SourceID = Param.PopInt();
			int64 ParentID = Param.PopInt();
            
			if (ParentID == NO_PARENT)
			{
				CLinkerItem* LinkerItem = DialogItem;
				LinkerItem->m_Alias = SourceID;
				PushChild(LinkerItem);
				LinkerItem->m_State |= SPACE_OPEN;
				
				Layout(true);
			}else{
				CLinkerItem* LinkerItem = FindLinker(SourceID);
				if (LinkerItem)
				{
					if (ParentID)
					{
						if(ParentID != SourceID){ 
							LinkerItem = (CLinkerItem*)LinkerItem->FindSpace(ParentID);
						}
					}

					if (LinkerItem)
					{
						LinkerItem->m_State |= SPACE_OPEN;
						LinkerItem->PushChild(DialogItem);		
						Layout(true);
					}else{
						delete DialogItem;
					}
				}else{
					delete DialogItem;
				}
			}
		}
		break;
	case DEL_DIALOG:
		{
			int64 SourceID = SRM->wParam;
			int64 DialogID = SRM->lParam;
			
			CLinkerItem* LinkerItem = FindLinker(SourceID);
			if (LinkerItem)
			{
				if (DialogID == DEFAULT_DIALOG)
				{
					CancelAllWarning();

					RemoveChild(LinkerItem);				
					
					int64 rSourceID, rDialogID;
					GetGUI()->GetFocusDialog(rSourceID,rDialogID);

					if(rSourceID == SourceID){
						m_SpaceFocused = NULL;
						m_SpaceSelected = NULL;
						//自动把焦点重置为缺省的主系统对话
						ePipeline Msg(TO_BRAIN_MSG::GUI_SET_FOUCUS_DIALOG); 
						GetGUI()->SendMsgToBrainFocuse(Msg);
					}

					delete LinkerItem;					
					Layout(true);

				}else{
					CLinkerItem* DialogItem = (CLinkerItem*)LinkerItem->FindSpace(DialogID);
					if (DialogItem)
					{
						SetSpaceWarning(DialogItem,FALSE);
						CVSpace2* Parent = DialogItem->m_Parent; 
						Parent->RemoveChild(DialogItem);
						
						m_SpaceFocused = NULL;
	
						if (DialogItem  == m_SpaceSelected)
						{
							m_SpaceSelected = Parent;

							ePipeline Msg(TO_BRAIN_MSG::GUI_SET_FOUCUS_DIALOG);  //本地焦点输入，写信和收信为同一个人		
							GetGUI()->SendMsgToBrainFocuse(Msg);
						}					
						delete DialogItem;
						Layout(true);	
					}
				}
			}

			//对话只剩系统对话时，重置对话计数
			if (m_ChildList.size()==1)
			{
				m_DialogCounter = 0;
			}
		}
		break;
	case CLR_LINKER:
		CancelAllWarning();
		Clear();
		m_SpaceFocused = NULL;
		m_SpaceSelected = NULL;
		Layout(true);
		break;	
	case SET_FOCUSDIALOG:
		{
			int64 SourceID = SRM->wParam;
			int64 DialogID = SRM->lParam;

			CLinkerItem* LinkerItem = FindLinker(SourceID);
			if (LinkerItem)
			{

				CLinkerItem* DialogItem = LinkerItem;
				if(DialogID != DEFAULT_DIALOG ){
					DialogItem = (CLinkerItem*)LinkerItem->FindSpace(DialogID);
				}
				if (DialogItem)
				{
					if (IsWarning(DialogItem))
					{
						SetSpaceWarning(DialogItem,FALSE);
					}

					if(m_SpaceSelected){
						m_SpaceSelected->m_State &= ~SPACE_SELECTED;
					}
					m_OldSelected = NULL;
					
					m_SpaceSelected = DialogItem;
					m_SpaceSelected->m_State |= SPACE_SELECTED;
					m_SpaceFocused = m_SpaceSelected;
					
					Invalidate();
				}

			}
		}
		break;
	case NOTIFY_DIALOG:
		{
			int64 SourceID = SRM->wParam;
			int64 DialogID = SRM->lParam;

			CVSpace2* Linker = FindSpace(SourceID);
			if(Linker==NULL)return 0;
			CVSpace2* DialogItem = Linker;
			if (DialogID != DEFAULT_DIALOG )
			{
				DialogItem = (CLinkerItem*)Linker->FindSpace(DialogID);
			}

			if (DialogItem)
			{
				if(IsWarning(DialogItem))return 0;
				SetSpaceWarning(DialogItem,TRUE);
				SetTimer(GetHwnd(),(UINT_PTR)DialogItem,2500,NULL); //只闪烁3次
			}
		}
		break;
	default:
		break;
	}
	return 0;
}
LRESULT CLinkerView::OnTimer(WPARAM wParam, LPARAM lParam){
	long nIDEvent = wParam;
	CVSpace2* Item = (CVSpace2*)nIDEvent;
	SetSpaceWarning(Item,FALSE);  //取消警告
	::KillTimer(m_hWnd,nIDEvent);
	return 0;
}


LRESULT CLinkerView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
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
			//m_SpaceSelected->SetArea(rc);
			//m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			//m_SpaceSelected = NULL;
			//m_Toolbar.m_Owner = NULL;
			//Layout();
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

		if (m_OldSelected == m_SpaceSelected)
		{
			return 0;
		};

		SetSpaceWarning(m_SpaceSelected,FALSE);
		m_OldSelected = m_SpaceSelected;

		ePipeline Msg(TO_BRAIN_MSG::GUI_SET_FOUCUS_DIALOG);  //本地焦点输入，写信和收信为同一个人
		GetGUI()->SendMsgToBrainFocuse(Msg);


		Layout();	
	}
	
	return 0;
}

LRESULT CLinkerView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}else if(message == WM_TIMER){
		return OnTimer(wParam,lParam);
	}
	return CWSTreeView::Reaction(message,wParam,lParam);
}
