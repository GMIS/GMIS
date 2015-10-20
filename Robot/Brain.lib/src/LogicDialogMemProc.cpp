// TaskDialogMemProc.cpp: implementation of the CLogicThread class.
// 处理临时逻辑
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "InstinctDefine.h"
#include "NotifyMsgDef.h"
#include "LogicDialog.h"

CLocalLogicCell* CLogicDialog::FindLogic(const tstring& Name){

    vector<CLocalLogicCell*>::iterator It = m_LogicList.begin();
	while(It<m_LogicList.end()){
		CLocalLogicCell* lg = *It;
		if(lg->LogicName() == Name){
			return lg;
		}
		It++;
	}
	return NULL;
}

CElementCell* CLogicDialog::FindCapacitor(const tstring& Name){

    vector<CElementCell*>::iterator It = m_CapaList.begin();
	while(It<m_CapaList.end()){
		if((*It)->m_Name == Name){
			return *It;
		}
		It++;
	}
	return NULL;
}

CElementCell* CLogicDialog::FindInductor(const tstring& Name){

    vector<CElementCell*>::iterator It = m_InduList.begin();
	while(It<m_InduList.end()){
		if((*It)->m_Name == Name){
			return *It;
		}
		It++;
	}
	return NULL;
}


CObjectData* CLogicDialog::FindObject(int64 ObjectID){
    //优先在本地找
	vector<CObjectData*>::iterator It = m_ObjectList.begin();
	while(It<m_ObjectList.end()){
		if((*It)->m_ID == ObjectID){
			return *It;
		}
		It++;
	}

	//没找到则在父对话中找
	if (m_ParentDialogID)
	{
		CLogicDialog* ParentDialog = m_Brain->GetBrainData()->GetDialog(m_ParentDialogID);
		if (ParentDialog)
		{
			It = ParentDialog->m_ObjectList.begin();
			while(It<ParentDialog->m_ObjectList.end()){
				if((*It)->m_ID == ObjectID){
					return *It;
				}
				It++;
			}
		}
	}

    //最后在系统对话中找
	CLogicDialog* SysDialog = m_Brain->GetBrainData()->GetDialog(DEFAULT_DIALOG);
	if (SysDialog)
	{
		It = SysDialog->m_ObjectList.begin();
		while(It<SysDialog->m_ObjectList.end()){
			if((*It)->m_ID == ObjectID){
				return *It;
			}
			It++;
		}
	}

	return NULL;
}


int32 CLogicDialog::FindObject(tstring Name,vector<CObjectData>& ObjectList){


	vector<CObjectData*>::iterator It = m_ObjectList.begin();
	while(It<m_ObjectList.end()){
		tstring ObjectName = (*It)->m_Name;
		if(ObjectName == Name){
			CObjectData* Obj = *It;
			ObjectList.push_back(*Obj);
		}
		It++;
	}

	if (ObjectList.size())
	{
		return ObjectList.size();
	}

	//没找到则在父对话中找
	if (m_ParentDialogID)
	{
		CLogicDialog* ParentDialog = m_Brain->GetBrainData()->GetDialog(m_ParentDialogID);
		if (ParentDialog)
		{
			It = ParentDialog->m_ObjectList.begin();
			while(It<ParentDialog->m_ObjectList.end()){
				tstring ObjectName = (*It)->m_Name;
				if(ObjectName == Name){
					CObjectData* Obj = *It;
					ObjectList.push_back(*Obj);
				}
				It++;
			}
		}
	}
	if (ObjectList.size())
	{
		return ObjectList.size();
	}
    //最后在系统对话中找
	CLogicDialog* SysDialog = m_Brain->GetBrainData()->GetDialog(DEFAULT_DIALOG);
	if (SysDialog)
	{
		It = SysDialog->m_ObjectList.begin();
		while(It<SysDialog->m_ObjectList.end()){
			tstring ObjectName = (*It)->m_Name;
			if(ObjectName == Name){
				CObjectData* Obj = *It;
				ObjectList.push_back(*Obj);
			}
			It++;
		}
	}

	return ObjectList.size();
}


bool CLogicDialog::RegisterLogic(CLogicTask* Task)
{	
	CLocalLogicCell* lg  = new CLocalLogicCell(*Task);
    
	Task = &lg->m_Task; //因为数据已经转给了此TASK

	//记录本次逻辑生成的元件，用来判断本逻辑是否有依赖（即引用其他逻辑生成的元件）；
    vector<tstring>  ElementList;

	bool IsDepend = false;
	int n = Task->m_LogicData.Size();
	for(int32 i = 0; i<n; i++)
	{
		ePipeline* Clause = (ePipeline*)Task->m_LogicData.GetData(i);
		int64  Cmd = *(INT64*)Clause->GetData(0);
        if(Cmd<0)Cmd=-Cmd; 

		//增加引用计数,如果引用的元件不在是否应该生成呢？这种情况不应该出现
		switch(Cmd){
        case INSTINCT_USE_CAPACITOR:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring&  ElementName = *(tstring*)Clause->GetData(1);
				ElementList.push_back(ElementName); //这里的ElementName已经检查过是唯一
				RegisterCapacitor(ElementName,lg);
			}
			break;
		case INSTINCT_USE_INDUCTOR:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring&  ElementName = *(tstring*)Clause->GetData(1);
				ElementList.push_back(ElementName);
				RegisterInductor(ElementName,lg);
			}
			break;
		case INSTINCT_REFERENCE_CAPACITOR:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring SrcCapacitor = *(tstring*)Clause->GetData(1);
				
				tstring ReferenceName = SrcCapacitor;
				tstring::size_type pos = SrcCapacitor.find(':');
				if(pos != tstring::npos ){
					ReferenceName = SrcCapacitor.substr(pos+1,SrcCapacitor.size()-pos);
					SrcCapacitor.resize(pos);
				}
				
				ReferenceName = TriToken(ReferenceName);	
				SrcCapacitor  = TriToken(SrcCapacitor);
				
				if (ReferenceName.size())
				{
					ReferenceName = _T("Ref");
				};
				
				//引用其他逻辑生成的Element则本逻辑为依赖型
				if(!IsDepend && find(ElementList.begin(),ElementList.end(),SrcCapacitor) == ElementList.end())
				{
					IsDepend = true;
				};
				
				ReferenceCapacitor(SrcCapacitor,ReferenceName,lg);
			}
			break;
			
		case INSTINCT_REFERENCE_INDUCTOR:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring SrcInductor = *(tstring*)Clause->GetData(1);
				tstring::size_type pos = SrcInductor.find(':');	      
				
				tstring ReferenceName  = SrcInductor;
				if(pos != tstring::npos ){
					ReferenceName = SrcInductor.substr(pos+1,SrcInductor.size()-pos);
					SrcInductor.resize(pos);
				}

				ReferenceName = TriToken(ReferenceName);			
				SrcInductor = TriToken(SrcInductor);
				
				if(!IsDepend && find(ElementList.begin(),ElementList.end(),SrcInductor) == ElementList.end())
				{
					IsDepend = true;
				}
				ReferenceInductor(SrcInductor,ReferenceName,lg);
			}
			break;
		case INSTINCT_USE_LOGIC:
			{
				IsDepend = true;
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring srcName = *(tstring*)Clause->GetData(1);

				tstring LogicName,LogicInstanceName,LogicComment;
				GetLogicName(srcName,LogicName,LogicInstanceName,LogicComment);

				if(Task->m_Name == LogicName){
					m_CompileError = Format1024(_T("Error: 在Think [%s]逻辑中递归调用自身[%s]"),LogicName.c_str(),LogicName.c_str());
					return false;
				};
				/*
				tstring refName = srcName;
				tstring::size_type pos = srcName.find(':');	 
				if (pos != tstring::npos)
				{
					tstring refName = srcName.substr(pos+1,srcName.size()-pos);
					refName = TriToken(refName);
					srcName.resize(pos);	
					srcName = TriToken(srcName);
				}
				
				ReferenceLogic(srcName,refName,lg);
				*/
			}
			/*       无用了 
			else if(Cmd == INSTINCT_USE_OBJECT){
			assert(Clause->GetDataType(1) == TYPE_PIPELINE);
			ePipeline* Param = (ePipeline*)Clause->GetData(1);
			tstring srcName = Param->GetLabel();  
			tstring::size_type pos = srcName.find(':');	      
			tstring refName = srcName.substr(pos+1,srcName.size()-pos);
			refName = TriToken(refName);
			srcName.resize(pos);	
			srcName = TriToken(srcName);
			ReferenceObject(srcName,refName,lg);
			}
			*/			break;
		case INSTINCT_SET_LABEL:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring  LabelName = *(tstring*)Clause->GetData(1);
				LabelName+=_T("@");  //避免与其它名字意外混淆
				ElementList.push_back(LabelName);
			}
			break;
		case INSTINCT_GOTO_LABEL:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring  LabelName = *(tstring*)Clause->GetData(1);
				LabelName+=_T("@");
				if(!IsDepend && find(ElementList.begin(),ElementList.end(),LabelName) == ElementList.end())
				{
					IsDepend = true;
				}
			}
			break;
		case INSTINCT_TABLE_CREATE:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring MemoryName = *(tstring*)Clause->GetData(1);
				
				tstring LogicName = lg->LogicName();
				m_NamedTableList.RegisterNameByLogic(MemoryName,LogicName);
			}
			break;
		default:
			break;
		}
	}
	
	Task->m_bDepend = IsDepend;
	m_LogicList.push_back(lg);
	
	//通知GUI界面，有逻辑条目
	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(ADD_LOGIC);
	nf.Push_Directly(lg->GetLogicItem());
	nf.Notify(this);

	return true;
};

void CLogicDialog::DeleteLogic(const tstring& Name){
	
	CLocalLogicCell* lg = NULL;

	vector<CLocalLogicCell*>::iterator It = m_LogicList.begin();
	while(It<m_LogicList.end()){
		CLocalLogicCell* lg1 = *It;
		if(lg1->LogicName() == Name){
			lg = lg1;
			break;
		}
		It++;
	}
 
    if (!lg)
    {
		return;
    }
        
	CLogicTask* Task = &lg->m_Task; //因为数据已经转给了此TASK

	//记录本次逻辑生成的元件，用来判断本逻辑是否有依赖（即引用其他逻辑生成的元件）；
    vector<tstring>  ElementList;

	bool IsDepend = false;
	int n = Task->m_LogicData.Size();
	for(int32 i = 0; i<n; i++)
	{
		ePipeline* Clause = (ePipeline*)Task->m_LogicData.GetData(i);
		int64  Cmd = *(INT64*)Clause->GetData(0);
        if(Cmd<0)Cmd=-Cmd; 

		//增加引用计数,如果引用的元件不在是否应该生成呢？这种情况不应该出现
		switch(Cmd){
        case INSTINCT_USE_CAPACITOR:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring&  ElementName = *(tstring*)Clause->GetData(1);
				DeleteCapacitor(ElementName);
			}
			break;
		case INSTINCT_USE_INDUCTOR:
			{
				assert(Clause->GetDataType(1) == TYPE_STRING);
				tstring&  ElementName = *(tstring*)Clause->GetData(1);
				DeleteInductor(ElementName);
			}
			break;
		default:
			break;
		}
	}
	
	//通知GUI界面，有逻辑条目
	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(DELETE_LOGIC);
	nf.Push_Directly(lg->GetLogicItem());
	nf.Notify(this);

	delete lg;
	m_LogicList.erase(It);
	
}

void CLogicDialog::ReferenceLogic(const tstring& scrName,const tstring& refName,CLocalLogicCell* WhoRef,bool Add){

    CLocalLogicCell* lg = FindLogic(scrName);
	if(lg == NULL)return;
 		
	if(Add){
		lg->AddRef(refName,WhoRef);	
	}
	else{
		lg->DelRef(refName);
	} 
	

	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(REF_LOGIC);

	ePipeline* Pipe = new ePipeline;
	Pipe->PushInt(Add);
	Pipe->PushString(scrName);
	Pipe->PushString(refName);
	Pipe->PushString(WhoRef->LogicName());
    nf.Push_Directly(Pipe);
	nf.Notify(this);
}


void CLogicDialog::RegisterCapacitor(const tstring& Name, CLocalLogicCell* CreatedBy){
		
	CElementCell* Cp = new CElementCell(Name,CreatedBy->LogicName());
	m_CapaList.push_back(Cp);
	
	ePipeline* Item = Cp->GetElementItem();

	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(ADD_CAPA);
    nf.Push_Directly(Item);
	nf.Notify(this);
}

void CLogicDialog::ReferenceCapacitor(const tstring& scrName,const tstring& refName, CLocalLogicCell* WhoRef,bool Add)
{
    CElementCell* Cp = FindCapacitor(scrName);
    if(Cp == NULL)return ;
	 
	if(Add){
		 Cp->AddRef(refName,WhoRef);
	}else{
		 Cp->DelRef(refName);
	}

	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(REF_CAPA);
	
	ePipeline* Item = new ePipeline;
    Item->PushInt(Add);  
	Item->PushString(scrName);
	Item->PushString(refName);
	Item->PushString(WhoRef->LogicName());

	nf.Push_Directly(Item);
	nf.Notify(this);
}


void CLogicDialog::RegisterInductor(const tstring& Name, CLocalLogicCell* CreatedBy){

	CElementCell* Indu = new CElementCell(Name,CreatedBy->LogicName());

	m_InduList.push_back(Indu);
	ePipeline* Item = Indu->GetElementItem();

	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(ADD_INDU);
    nf.Push_Directly(Item);
	nf.Notify(this);
}

void CLogicDialog::RegisterObject(ePipeline& ObjectData){
	if (ObjectData.Size()==0)
	{
		return;
	}
	tstring& Name = *(tstring*)ObjectData.GetData(0);
    transform(Name.begin(),Name.end(),Name.begin(),tolower);

	vector<CObjectData> ObjectList; //暂时不允许同名物体登记
	if(FindObject(Name,ObjectList)){
		return;	
	};

	CObjectData* Ob = new CObjectData(ObjectData);

	Ob->m_ID = AbstractSpace::CreateTimeStamp();
	m_ObjectList.push_back(Ob);

	CNotifyState nf(NOTIFY_OBJECT_VIEW);
	nf.PushInt(ADD_OBJECT);
    nf.Push_Directly(ObjectData.Clone());
	nf.Notify(this);
};

void CLogicDialog::DeleteCapacitor(const tstring& Name)
{
	vector<CElementCell*>::iterator It = m_CapaList.begin();
	while(It<m_CapaList.end()){
		if((*It)->m_Name == Name){
			m_CapaList.erase(It);
			break;
		}
		It++;
	}
};
void CLogicDialog::DeleteInductor(const tstring& Name)
{
	vector<CElementCell*>::iterator It = m_InduList.begin();
	while(It<m_InduList.end()){
		if((*It)->m_Name == Name){
			m_InduList.erase(It);
			break;
		}
		It++;
	}
}
void CLogicDialog::DeleteObject(ePipeline& ObjectData){
	CObjectData ob(ObjectData);

	vector<CObjectData*>::iterator It = m_ObjectList.begin();
	while(It<m_ObjectList.end()){
		CObjectData* ob1 = *It;
		if(ob1->m_Name == ob.m_Name && ob1->m_Fingerprint == ob.m_Fingerprint){
			m_ObjectList.erase(It);
			return;
		}
		It++;
	}
	CNotifyState nf(NOTIFY_OBJECT_VIEW);
	nf.PushInt(DEL_OBJECT);
    nf.Push_Directly(ObjectData.Clone());
	nf.Notify(this);
}

void CLogicDialog::ReferenceInductor(const tstring& scrName,const tstring& refName, CLocalLogicCell* WhoRef,bool Add)
{
    CElementCell* Indu = FindInductor(scrName);
    if(Indu == NULL)return;

	if(Add){
		Indu->AddRef(refName,WhoRef);
	}else{
		Indu->DelRef(refName);
	}


	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(REF_INDU);

	ePipeline* Item = new ePipeline;
	Item->PushInt(Add);  
	Item->PushString(scrName);
	Item->PushString(refName);
	Item->PushString(WhoRef->LogicName());
	nf.Push_Directly(Item);
	nf.Notify(this);

}

ePipeline*  CLogicDialog::FindTempMemory(int64 InstanceID){
	map<int64,ePipeline>::iterator it = m_TableInstanceList.find(InstanceID);
	if (it == m_TableInstanceList.end())
	{
		return NULL;
	}
	ePipeline& Pipe = it->second;
	return &Pipe;
};

CElement*  CLogicDialog::FindLogicInstance(int64 InstanceID){
	map<int64,CElement*>::iterator it = m_LogicInstanceList.find(InstanceID);
	if (it == m_LogicInstanceList.end())
	{
		return NULL;
	}
	CElement* E = it->second;
	return E;
};

ePipeline* CLogicDialog::FindObjectInstance(int64 InstanceID){
	map<int64,ePipeline>::iterator it = m_ObjectInstanceList.find(InstanceID);
	if (it == m_ObjectInstanceList.end())
	{
		return NULL;
	}
	ePipeline& Pipe = it->second;
	return &Pipe;
}


void CLogicDialog::AddObjectInstance(int64 InstanceID,ePipeline& Pipe){
	m_ObjectInstanceList[InstanceID] = Pipe;

}
void CLogicDialog::AddMemoryInstance(int64 InstanceID, ePipeline& Pipe){
	m_TableInstanceList[InstanceID] = Pipe;
}

void CLogicDialog::CloseObjectInstance(int64 InstanceID){
	map<int64,ePipeline>::iterator it = m_ObjectInstanceList.find(InstanceID);
	if (it != m_ObjectInstanceList.end())
	{
		m_ObjectInstanceList.erase(it);
	}
}


void CLogicDialog::CloseMemoryInstance(int64 InstanceID){
	map<int64,ePipeline>::iterator it = m_TableInstanceList.find(InstanceID);
	if (it != m_TableInstanceList.end())
	{
		m_TableInstanceList.erase(it);
	}
}


void CLogicDialog::ClearLogicSence(){
    vector<CLocalLogicCell*>::iterator It = m_LogicList.begin();
	while(It!=m_LogicList.end()){
		delete *It;
		*It = NULL;
		It++;
	}
	m_LogicList.clear();
	
    vector<CElementCell*>::iterator Ita = m_CapaList.begin();
	while(Ita!=m_CapaList.end()){
		delete *Ita;
		*Ita = NULL;
		Ita++;
	}
	m_CapaList.clear();

    vector<CElementCell*>::iterator Itb = m_InduList.begin();
	while(Itb!=m_InduList.end()){
		delete *Itb;
		*Itb = NULL;
		Itb++;
	}
	m_InduList.clear();

	CNotifyState nf(NOTIFY_LOGIC_VIEW);
	nf.PushInt(CLEAR_ALL);
	nf.Notify(this);
}

void CLogicDialog::ClearObject(){
	vector<CObjectData*>::iterator Itc = m_ObjectList.begin();
	while(Itc != m_ObjectList.end()){
		delete *Itc;
		*Itc = NULL;
		Itc ++;
	}
	m_ObjectList.clear();
	
	CNotifyState nf(NOTIFY_OBJECT_VIEW);
	nf.PushInt(CLEAR_ALL);
	nf.Notify(this);
}


void CLogicDialog::GetLocalLogicData(ePipeline& List){
	ePipeline  Pipe;
	vector<CLocalLogicCell*>::iterator It = m_LogicList.begin();
	while(It!=m_LogicList.end()){
		CLocalLogicCell*  Item = *It;
		List.Push_Directly(Item->GetLogicItem());
        Item->GetRefList(Pipe);
		List.PushPipe(Pipe);
		It++;
	}
}



void CLogicDialog::GetLocalCapaData(ePipeline& List){
	ePipeline Pipe;
    vector<CElementCell*>::iterator Ita = m_CapaList.begin();
	while(Ita!=m_CapaList.end()){
		CElementCell* Item = *Ita;
		List.Push_Directly(Item->GetElementItem());
		Item->GetRefList(Pipe);
		List.PushPipe(Pipe);
		Ita++;
	}
}

void CLogicDialog::GetLocalInduData(ePipeline& List){
	ePipeline Pipe;
    vector<CElementCell*>::iterator Ita = m_InduList.begin();
	while(Ita!=m_InduList.end()){
		CElementCell* Item = *Ita;
		List.Push_Directly(Item->GetElementItem());
		Item->GetRefList(Pipe);
		List.PushPipe(Pipe);
		Ita++;
	}
}

void CLogicDialog::GetLocalObjectData(ePipeline& List){
    vector<CObjectData*>::iterator Ita = m_ObjectList.begin();
	while(Ita!=m_ObjectList.end()){
		CObjectData* Item = *Ita;
		List.Push_Directly(Item->GetItemData());
		Ita++;
	}
}

void CLogicDialog::GetTableInstanceData(ePipeline& List){
	map<int64, ePipeline>::iterator Ita = m_TableInstanceList.begin();
	while(Ita!=m_TableInstanceList.end()){
		List.PushInt(Ita->first);
		List.Push_Directly(Ita->second.Clone());
		Ita++;
	}
}
