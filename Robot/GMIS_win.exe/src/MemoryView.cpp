// MemoryView.cpp: implementation of the CMemoryView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786) 
#pragma warning (disable: 4244)

#include "GMIS.h"
#include "MemoryView.h"
#include "resource.h"
#include "SystemSetting.h"
#include "TextAnalyse.h"
#include "NotifyMsgDef.h"

CMemoryView::CTitle::CTitle(int64 ID, const tstring& Name)
:CVSpace2(ID),m_Text(Name.c_str())
{
	m_ObjectCount = 0;
	SetAreaSize(200,20);
}

void CMemoryView::CTitle::Draw(HDC hDC,ePipeline* Pipe ){
	//输出文字
    RECT rc = GetArea();
	
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	tstring s = Format1024(_T("%s		(%3d) "),m_Text.c_str(),m_ChildList.size());
	::DrawText(hDC,s.c_str(),s.size(),&rc,DT_LEFT|DT_EXPANDTABS|
		DT_NOPREFIX|DT_VCENTER);
	
	::SetTextColor(hDC,Oldcr);
	
}

CMemoryView::CTextItem::CTextItem(int Index,int64 ID,tstring Text)
:CVSpace2(ID),m_Index(Index),m_Text(Text){
	SetAreaSize(200,20);
}

void CMemoryView::CTextItem::Draw(HDC hDC, ePipeline* Pipe){
	//输出文字
    RECT rc = GetArea();
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));

	if (m_Alias<0xffffffff)
	{
		int32 ID = m_Alias;
		tstring Text = Format1024(_T("%s"),m_Text.c_str());
		::DrawText(hDC,Text.c_str(),Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);

	} 
	else
	{

		tstring Text = Format1024(_T("[%d]	%s"),m_Index,m_Text.c_str());
		::DrawText(hDC,Text.c_str(),Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);

	}

	::SetTextColor(hDC,Oldcr);	
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryView::CMemoryView()
{
	m_UseSystemScrollbar = false;
	m_crViewBkg = RGB(255,255,255);
    //m_State &= ~SPACE_SHOWWINBORDER;

	CTitle* ObjectTitle = new CTitle(-1,_T("Object Instance"));
	CTitle* DataTitle   = new CTitle(-2,_T("Data Instance"));
	
	ObjectTitle->m_State = SPACE_NOTOOLABR;
	DataTitle->m_State = SPACE_NOTOOLABR;

	m_ChildList.push_back(ObjectTitle);
	m_ChildList.push_back(DataTitle);

	ButtonItem* Bnt1 = new ButtonItem(BNT_CLOSE_ONE,_T("Close this"),NULL,false,NULL);
	ButtonItem* Bnt2 = new ButtonItem(BNT_CLOSE_ALL,_T("Close all"),NULL,false,NULL);
	
	m_Toolbar.PushChild(Bnt1);
	m_Toolbar.PushChild(Bnt2);

}

CMemoryView::~CMemoryView()
{

}

void CMemoryView::InfoProc(ePipeline& Info)
{
    SendChildMessage(GetHwnd(),INFO_PROC,(int64)&Info,0);
}

void CMemoryView::ResetData(ePipeline& DataList){
	
	CTitle* Title = (CTitle*)m_ChildList[1];
	Title->DeleteAll();
	Title->m_ObjectCount=0;

	for (int i=0; i<DataList.Size(); i++)
	{
		ePipeline*  InstanceItem = (ePipeline*)DataList.GetData(i);
		tstring InstanceName = InstanceItem->GetLabel();
		CreateDataInstance(InstanceName);

		CTextItem* ChildItemList = (CTextItem*)Title->m_ChildList[Title->m_ChildList.size()-1];
		
		for (int j=0; j<InstanceItem->Size(); j++)
		{
			Energy* Data = InstanceItem->GetEnergy(j);
			
			CMemoryView::CTextItem*  DataItem = Data2Item(j,Data);

			if (DataItem==NULL)
			{
				return;
			}	

			//DataItem->m_State = SPACE_NOTOOLABR;

			ChildItemList->PushChild(DataItem);
		}
	}
	Layout();
}
void CMemoryView::CreateObjectInstance(ePipeline* ObjData)
{
	CTitle* Title = (CTitle*)m_ChildList[0];
	
	Title->m_State |= SPACE_OPEN;

	int64 InstanceID = ObjData->GetID();
	tstring SrcObjectName = *(tstring*)ObjData->GetData(0);

	int i;
	CTextItem* ObjectItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		ObjectItem = (CTextItem*)Title->m_ChildList[i];
		
		if (ObjectItem->m_Text == SrcObjectName)
		{
			break;
		}
	}
	
	if (i==Title->m_ChildList.size()) //没找到生成一个项目
	{
		int32 ID = Title->m_ObjectCount++;		
		ObjectItem = new CMemoryView::CTextItem(0,ID,SrcObjectName);
		ObjectItem->SetAreaSize(200,18);
		Title->PushChild(ObjectItem);
		assert(ObjectItem);	
		ObjectItem->m_State = SPACE_OPEN|SPACE_NOTOOLABR;
	}
	

	//然后生成一个未命名的实例
    assert(ObjectItem);
	
	tstring Text = _T("Unnamed object instance");
	CMemoryView::CTextItem*  InstanceItem = new CMemoryView::CTextItem(0,InstanceID,Text);
	InstanceItem->SetAreaSize(200,18);
	ObjectItem->PushChild(InstanceItem);

};

void CMemoryView::NameObjectInstance(tstring SrcObjectName,int64 InstanceID,tstring InstanceName)
{
	CTitle* Title = (CMemoryView::CTitle*)m_ChildList[0];
	
	int i;
	CMemoryView::CTextItem* ObjectItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		ObjectItem = (CMemoryView::CTextItem*)Title->m_ChildList[i];
		
		if (ObjectItem->m_Text == SrcObjectName)
		{
			break;
		}
	}
	
	if (i==Title->m_ChildList.size()) //没找到
	{
		return ;
	}
	
	assert(ObjectItem);

	deque<CVSpace2*>::iterator it = ObjectItem->m_ChildList.begin();
	
	while (it != ObjectItem->m_ChildList.end())
	{
		CMemoryView::CTextItem* InstanceItem = (CMemoryView::CTextItem*)(*it);
		
		if (InstanceItem->m_Alias == InstanceID)
		{
			InstanceItem->m_Text = InstanceName;
			return ;
		}
		it++;
	}
};

void CMemoryView::CloseObjectInstance(tstring SrcObjectName,int64 InstanceID){

	CTitle* Title = (CMemoryView::CTitle*)m_ChildList[0];
		
	int i;
	CMemoryView::CTextItem* ObjectItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		ObjectItem = (CMemoryView::CTextItem*)Title->m_ChildList[i];
		
		if (ObjectItem->m_Text == SrcObjectName)
		{
			break;
		}
	}
	
	if (i==Title->m_ChildList.size()) //没找到
	{
		return;
	}
	
	assert(ObjectItem);
	
	CMemoryView::CTextItem* InstanceItem = NULL;
	
	deque<CVSpace2*>::iterator it = ObjectItem->m_ChildList.begin();
	
	while (it != ObjectItem->m_ChildList.end())
	{
		InstanceItem = (CMemoryView::CTextItem*)(*it);
		
		if (InstanceItem->m_Alias == InstanceID)
		{
			delete InstanceItem;

			ObjectItem->m_ChildList.erase(it);

			if (ObjectItem->m_ChildList.size()==0)
			{
				delete ObjectItem;
				Title->m_ChildList.erase(Title->m_ChildList.begin()+i);
			}

			m_Toolbar.m_Owner = NULL;
			m_SpaceFocused = NULL;

			return ;
		}
		it++;
	}
		
};

void CMemoryView::CreateDataInstance(tstring InstanceName){
	
	CTitle* Title = (CTitle*)m_ChildList[1];
	Title->m_State |= SPACE_OPEN;


	int i;
	
	CTextItem* InstanceItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		InstanceItem = (CTextItem*)Title->m_ChildList[i];
		
		if (InstanceItem->m_Text == InstanceName)
		{
			break;
		}
	}
	
	if (i < Title->m_ChildList.size()) //已经存在
	{
		return;
	}
	

	int32 ID = Title->m_ObjectCount++;

	InstanceItem = new CMemoryView::CTextItem(0,ID,InstanceName);
	InstanceItem->SetAreaSize(200,18);
	Title->PushChild(InstanceItem);

	InstanceItem->m_State |=SPACE_OPEN;

};


CMemoryView::CTextItem* CMemoryView::Data2Item(int Index,Energy* Data)
{
	   TCHAR buffer[255];
	   
	   int64 ID = AbstractSpace::CreateTimeStamp();
	   uint32 Type = Data->EnergyType();
	   switch(Type){
	   case TYPE_NULL:
		   {
			   _stprintf(buffer,_T("NULL:-"));
			   CMemoryView::CTextItem*  DataItem = new CMemoryView::CTextItem(Index,ID,buffer);  
			   return DataItem;
		   }
		   break;

	   case TYPE_INT:
		   {
			   int64* value=(int64 *)Data->Value();
			   _stprintf(buffer,_T("INT64: %I64ld"), *value);
			   CMemoryView::CTextItem*  DataItem = new CMemoryView::CTextItem(Index,ID,buffer);  
			   return DataItem;
		   }
		   break;
	   case TYPE_FLOAT:
		   {
			   float64* value=(float64 *)Data->Value();
			   _stprintf(buffer,_T("FLOAT64: %f"), *value);
			   CMemoryView::CTextItem*  DataItem = new CMemoryView::CTextItem(Index,ID,buffer);  
			   return DataItem;
		   }
		   break;
	   case TYPE_STRING:
		   {
			   tstring* value=(tstring *)Data->Value();
			   tstring s = Format1024(_T("STRING: %s"),value->c_str());
			   
			   CMemoryView::CTextItem*  DataItem = new CMemoryView::CTextItem(Index,ID,s);  
			   SIZE s1 = CalcuTextSize(buffer);    	
			   DataItem->SetAreaSize(s1.cx+4,s1.cy+4);
			   if(DataItem->m_SizeX<200)DataItem->m_SizeX = 200;
			   return DataItem;
		   }
		   break;
	   case TYPE_BLOB:
		   {
			   
			   _stprintf(buffer,_T("BLOB:-"));
			   CMemoryView::CTextItem*  DataItem = new CMemoryView::CTextItem(Index,ID,buffer);  
			   return DataItem;
		   }
		   break;	
	   case TYPE_PIPELINE:
		   {
			   ePipeline* Pipe=(ePipeline *)Data->Value();
			   
			   _stprintf(buffer,_T("PIPELINE: %d"),Pipe->Size());
			   CMemoryView::CTextItem*  ParentItem = new CMemoryView::CTextItem(Index,ID,buffer);  
			   
			   if(ParentItem==NULL)
			   {
				   return NULL;
			   }
			   for(int i=0; i<Pipe->Size();i++)
			   {
				   Data = Pipe->GetEnergy(i);
				   CMemoryView::CTextItem*  DataItem = Data2Item(i,Data);
				   if (!DataItem)
				   {
					   delete ParentItem;
					   return NULL;
				   }
				   ParentItem->PushChild(DataItem);
			   }
			   return ParentItem;
			   
		   }      
		   break;
	   default:
		   break;
		   
	   }
	   
	   return NULL;
}

void CMemoryView::InsertData(tstring InstanceName,ePipeline& Address,Energy* Data){
	
	CTitle* Title = (CTitle*)m_ChildList[1];
	
	int i;
	
	CTextItem* InstanceItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		InstanceItem = (CTextItem*)Title->m_ChildList[i];
		
		if (InstanceItem->m_Text == InstanceName)
		{
			break;
		}
	}
	
	if (i == Title->m_ChildList.size()) //不存在
	{
		return;
	}
	
	assert(InstanceItem);
	
	CVSpace2*  Parent = InstanceItem;
	CVSpace2*  Target = NULL;
	int64 AddressLength = Address.Size();

	if (AddressLength==0)
	{
		CMemoryView::CTextItem*  DataItem = Data2Item(0,Data);

		if (DataItem==NULL)
		{
			return;
		}	
		DataItem->m_State = SPACE_NOTOOLABR;
		Parent->PushChild(DataItem,0);
		for (int i=1; i<Parent->m_ChildList.size();i++)
		{
			CMemoryView::CTextItem* item = (CMemoryView::CTextItem*)Parent->m_ChildList[i];
			item->m_Index++;
		}
		return;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Address.GetData(i);
		if(Index<0 || Index>Parent->m_ChildList.size()){
			assert(0);
			return;
		}

		if(i==AddressLength-1){ //最后一个地址
			CMemoryView::CTextItem*  DataItem = Data2Item(Index,Data);

			if (DataItem==NULL)
			{
				return;
			}	
			DataItem->m_State = SPACE_NOTOOLABR;

			Parent->PushChild(DataItem,Index++);
			for (int i=Index; i<Parent->m_ChildList.size();i++)
			{
				CMemoryView::CTextItem* item = (CMemoryView::CTextItem*)Parent->m_ChildList[i];
				item->m_Index++;
			}
			return;
		}

		Target = Parent->m_ChildList.at(Index);	
		Parent = Target;	
	}

	return;

};

void CMemoryView::ImportData(tstring InstanceName,ePipeline& Address,ePipeline* DataList){
	CTextItem* InstanceItem = NULL;
	int i;
	CTitle* Title = (CTitle*)m_ChildList[1];

	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		InstanceItem = (CTextItem*)Title->m_ChildList[i];

		if (InstanceItem->m_Text == InstanceName)
		{
			break;
		}
	}

	if (i == Title->m_ChildList.size()) //不存在
	{
		return;
	}

	assert(InstanceItem);
	CVSpace2*  Parent = InstanceItem;
	CVSpace2*  Target = NULL;
	int64 AddressLength = Address.Size();

	if (AddressLength==0)
	{
		int Index=Parent->m_ChildList.size();
		int n = Index;
		while (DataList->Size())
		{
			eElectron E;
			DataList->Pop(&E);
			ePipeline* Data = (ePipeline*)E.Get();

			CMemoryView::CTextItem*  DataItem = Data2Item(Index++,Data);

			if (DataItem==NULL)
			{
				return;
			}	

			DataItem->m_State = SPACE_NOTOOLABR;
			Parent->PushChild(DataItem);
		}
		if(n==Index)return;

		for (int i=Index; i<Parent->m_ChildList.size();i++)
		{
			CMemoryView::CTextItem* item = (CMemoryView::CTextItem*)Parent->m_ChildList[i];
			item->m_Index++;
		}
		return;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Address.GetData(i);
		if(Index<0 || Index>Parent->m_ChildList.size()){
			assert(0);
			return;
		}

		if(i==AddressLength-1){ //最后一个地址
			int Index=Parent->m_ChildList.size();
			int n=Index;
			while (DataList->Size())
			{
				eElectron E;
				DataList->Pop(&E);
				ePipeline* Data = (ePipeline*)E.Get();

				CMemoryView::CTextItem*  DataItem = Data2Item(Index++,Data);

				if (DataItem==NULL)
				{
					return;
				}	

				DataItem->m_State = SPACE_NOTOOLABR;
				Parent->PushChild(DataItem);
			}
			if(n==Index)return;

			for (int i=Index; i<Parent->m_ChildList.size();i++)
			{
				CMemoryView::CTextItem* item = (CMemoryView::CTextItem*)Parent->m_ChildList[i];
				item->m_Index++;
			}
			return;
		}

		Target = Parent->m_ChildList.at(Index);	
		Parent = Target;	
	}

	return;

}

void CMemoryView::ModifyData(tstring InstanceName,ePipeline& Address,Energy* Data){

	CTitle* Title = (CTitle*)m_ChildList[1];


	int i;

	CTextItem* InstanceItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		InstanceItem = (CTextItem*)Title->m_ChildList[i];

		if (InstanceItem->m_Text == InstanceName)
		{
			break;
		}
	}

	if (i == Title->m_ChildList.size()) //不存在
	{
		return;
	}

	assert(InstanceItem);

	CVSpace2*  Parent = InstanceItem;
	CVSpace2*  Target = NULL;
	int64 AddressLength = Address.Size();

	if (AddressLength==0)
	{
		return;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Address.GetData(i);
		if(Index<0 || Index>=Parent->m_ChildList.size()){
			assert(0);
			return;
		}

		if(i==AddressLength-1){ //最后一个地址
			CMemoryView::CTextItem*  DataItem = Data2Item(Index,Data);

			if (DataItem==NULL)
			{
				return;
			}	
			DataItem->m_State = SPACE_NOTOOLABR;
			DataItem->m_Parent = Parent;
			deque<CVSpace2*>::iterator It = Parent->m_ChildList.begin()+Index;
			CVSpace2* Old = *It;
			delete Old;
			*It = DataItem;
			return;
		}

		Target = Parent->m_ChildList.at(Index);	
		Parent = Target;	
	}
};

void CMemoryView::RemoveData(tstring InstanceName,ePipeline& Address){
	CTitle* Title = (CTitle*)m_ChildList[1];
	
	
	int i;
	
	CTextItem* InstanceItem = NULL;
	for (i=0; i<Title->m_ChildList.size(); i++)
	{
		InstanceItem = (CTextItem*)Title->m_ChildList[i];
		
		if (InstanceItem->m_Text == InstanceName)
		{
			break;
		}
	}
	
	if (i == Title->m_ChildList.size()) //不存在
	{
		return;
	}
	
	assert(InstanceItem);


	CVSpace2*  Parent = InstanceItem;
	CVSpace2*  Target = NULL;
	int64 AddressLength = Address.Size();

	if (AddressLength==0) //空地址
	{
		return;
	}

	for (int i=0 ;i<AddressLength; i++){
		int64 Index = *(int64*)Address.GetData(i);
		if(Index<0 || Index>=Parent->m_ChildList.size()){
			assert(0);
			return;
		}

		if(i==AddressLength-1){ //最后一个地址
			CMemoryView::CTextItem* DataItem = (CMemoryView::CTextItem*)Parent->m_ChildList[Index];	

			if(m_Toolbar.m_Owner == DataItem)
			{
				m_Toolbar.m_Owner = NULL;
				m_SpaceFocused = NULL;
			}
			delete DataItem;

			Parent->m_ChildList.erase(Parent->m_ChildList.begin()+Index);

			for (i=Index; i<Parent->m_ChildList.size();i++)
			{
				DataItem = (CMemoryView::CTextItem*)Parent->m_ChildList[i];
				DataItem->m_Index--;
			}
			return;
		}

		Target = Parent->m_ChildList.at(Index);	
		Parent = Target;	
	}

	
};

void CMemoryView::CloseDataInstance(tstring InstanceName){
	CTitle* Title = (CTitle*)m_ChildList[1];
	
	CTextItem* InstanceItem = NULL;
	deque<CVSpace2*>::iterator it = Title->m_ChildList.begin();
	while(it != Title->m_ChildList.end())
	{
		InstanceItem = (CTextItem*)*it;
		
		if (InstanceItem->m_Text == InstanceName)
		{
			break;
		}
		it++;
	}
	
	if (it == Title->m_ChildList.end()) //不存在
	{
		return;
	}

	delete InstanceItem;
	Title->m_ChildList.erase(it);

	m_Toolbar.m_Owner = NULL;
    m_SpaceFocused = NULL;

}


void CMemoryView::Layout(bool Redraw /* =true */){
    
    CWSTreeView::Layout(false);   
	m_Toolbar.Layout();
	if(Redraw){
		Invalidate();
	}
};	
void CMemoryView::Draw(HDC hDC, ePipeline* Pipe){
	CWSTreeView::Draw(hDC,Pipe);
	if(m_Toolbar.m_Owner){
		m_Toolbar.Draw(hDC);
	}
};

LRESULT CMemoryView::ChildReaction(SpaceRectionMsg* SRM){
    switch(SRM->Msg)
    {
    case INFO_PROC:
		{
           return OnInfoProc(SRM->wParam,SRM->lParam);
		}
		break;
    }
    return 0;
};

LRESULT CMemoryView::OnInfoProc(WPARAM wParam, LPARAM lParam){
	
	
	ePipeline* Info = (ePipeline*)wParam;
	
	int64 Type = Info->PopInt();
	int64 Cmd  = Info->PopInt();
	
	if (Type == INSTANCE_OBJECT)
	{
		if(Cmd == CREATE_INSTANCE){
			ePipeline* ObjData = (ePipeline*)Info->GetData(0);
			CreateObjectInstance(ObjData);
		}else if (Cmd == NAME_INSTANCE)
		{
			tstring SrceObjectName = Info->PopString();
			int64 InstanceID = Info->PopInt();
			tstring InstanceName = Info->PopString();
			NameObjectInstance(SrceObjectName,InstanceID,InstanceName);

		}else if (Cmd == CLOSE_INSTANCE)
		{
			tstring SrceObjectName = Info->PopString();
			int64 InstanceID = Info->PopInt();
			CloseObjectInstance(SrceObjectName,InstanceID);
		}
	} 
	else if(Type == INSTANCE_DATA)
	{
		switch(Cmd){
		case CREATE_INSTANCE:
			{
				tstring InstanceName = Info->PopString();
				CreateDataInstance(InstanceName);
			}
			break;
		case INSERT_DATA:
			{
				tstring InstanceName = Info->PopString();
			
				ePipeline* Address = (ePipeline*)Info->GetData(0);
				Energy* Data = Info->GetEnergy(1);

				InsertData(InstanceName,*Address,Data);
			}
			break;
		case MODIFY_DATA:
			{
				tstring InstanceName = Info->PopString();
				ePipeline* Address = (ePipeline*)Info->GetData(0);
				Energy* Data = Info->GetEnergy(1);


				ModifyData(InstanceName,*Address,Data);

			}
			break;
		case REMOVE_DATA:
			{
				tstring InstanceName = Info->PopString();
				ePipeline* Address = (ePipeline*)Info->GetData(0);
				RemoveData(InstanceName,*Address);
			}
			break;
		case CLOSE_INSTANCE:
			{
				tstring InstanceName = Info->PopString();
				CloseDataInstance(InstanceName);
			}
			break;
		case IMPORT_DATA:
			{
				tstring InstanceName = Info->PopString();
				ePipeline* Address = (ePipeline*)Info->GetData(0);
				ePipeline* DataList = (ePipeline*)Info->GetData(1);
				ImportData(InstanceName,*Address,DataList);
			}
			break;
		default:
			assert(0);
			break;
		}
	}
	m_Toolbar.m_Owner = NULL;
	m_SpaceSelected = NULL;
	m_SpaceFocused  = NULL;
	Layout();
	return 0;	
}
LRESULT CMemoryView::ToolbarReaction(ButtonItem* Bnt){
	if (!m_SpaceSelected)
	{
		return 0;
	}
	CVSpace2* Parent = m_SpaceSelected->m_Parent;
	if (!Parent)
	{
		return 0;
	}

	int64 Type = INSTANCE_OBJECT;
	if(Parent == m_ChildList[1]){
		Type = INSTANCE_DATA;
	}

	CTextItem* Item = (CTextItem*)m_SpaceSelected;

	switch(Bnt->m_Alias){
	case BNT_CLOSE_ONE:
		{	
			ePipeline Msg(TO_BRAIN_MSG::GUI_MEMORY_OPERATE);		
			Msg.PushInt(Type);
			Msg.PushInt(CLOSE_INSTANCE);
			if (Type == INSTANCE_OBJECT)
			{
				int64 InstanceID = Item->m_Alias;
				Msg.PushInt(InstanceID);
			}else{
				tstring& InstanceName = Item->m_Text;
				Msg.PushString(InstanceName);
			}

			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
		break;
	case BNT_CLOSE_ALL:
		{
			ePipeline Msg(TO_BRAIN_MSG::GUI_MEMORY_OPERATE);		
			Msg.PushInt(Type);
			Msg.PushInt(CLOSE_INSTANCE);
			
			if (Type == INSTANCE_OBJECT)
			{
				deque<CVSpace2*>::iterator it = Parent->m_ChildList.begin();
				while(it != Parent->m_ChildList.end()){
					CVSpace2* InstanceItem = *it;
					Msg.PushInt(InstanceItem->m_Alias);
					it++;
				}		
			}else{
				deque<CVSpace2*>::iterator it = Parent->m_ChildList.begin();
				while(it != Parent->m_ChildList.end()){
					CTextItem* InstanceItem = (CTextItem*)*it;
					Msg.PushString(InstanceItem->m_Text);
					it++;
				}		
			}
			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
		break;
	}
	return 0;
}
LRESULT CMemoryView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
   if(message == WM_RBUTTONDOWN){
	   return OnRButtnDown(wParam,lParam);
   }
   else if (message == WM_COMMAND){
	   return OnCommand(wParam,lParam);
   }
   else return CWSTreeView::Reaction(message,wParam,lParam);
}


LRESULT CMemoryView::OnRButtnDown(WPARAM wParam, LPARAM lParam){
	//弹出菜单
	/*
	POINT pos;
	::GetCursorPos(&pos);
	
	HMENU hmenu;
	HMENU subMenu;
	
	HINSTANCE hInstance = CWinSpace2::GetHinstance();
	hmenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_TEMPLOGICMENU));
	if(hmenu == NULL){
		int32 n = ::GetLastError();
		return TRUE;
	}
				
	subMenu = GetSubMenu(hmenu, 0); 
	
	::SetForegroundWindow(GetHwnd()); 
	::TrackPopupMenu(subMenu,0, pos.x, pos.y,0,GetHwnd(),NULL);
	::DestroyMenu(hmenu); 
	::PostMessage(GetHwnd(),WM_NULL, 0, 0);
	*/
	  return 0;
}

LRESULT CMemoryView::OnCommand(WPARAM wParam, LPARAM lParam){

    if(wParam = ID_SELECT_LOGIC)
	{


	}
	return 0;
}
