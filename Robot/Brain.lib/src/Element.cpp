#pragma warning (disable:4786)

#include "Brain.h"
#include "Element.h"
#include "Pipeline.h"
#include "TaskDialog.h"
#include "UserTimer.h"
#include "LinkerPipe.h"
#include "LogicElement.h"


CLocalInfoAuto::CLocalInfoAuto(CTaskDialog* Dialog,CElement* Elt,ePipeline& Pipe)
	: m_LocalPipe(Pipe),m_Dialog(Dialog){
	assert(Elt);
	int64 ID = Elt->m_ID;
	m_LocalPipe.PushInt(ID);

	if (Dialog->m_Brain->GetLogFlag() & LOG_MSG_PROC_PATH ) //可用于调试时跟踪执行路径
	{
		if (Dialog->m_CurTaskMsgID != MSG_EVENT_TICK)
		{
			Dialog->m_Brain->OutputLog(LOG_MSG_PROC_PATH,_T("+%I64ld %s ExePipe Alive:%d State:%I64ld "),ID,Elt->GetName().c_str(),m_Dialog->m_ExePipe.IsAlive(),m_Dialog->m_ExePipe.GetID());
		}
	}
}

CLocalInfoAuto::~CLocalInfoAuto (){
	eElectron E;
	m_LocalPipe.PopBack(&E);
	int64 ID = E.Int64();
	if (m_Dialog->m_Brain->GetLogFlag() & LOG_MSG_PROC_PATH  )
	{
		if (m_Dialog->m_CurTaskMsgID != MSG_EVENT_TICK)
		{
			m_Dialog->m_Brain->OutputLog(LOG_MSG_PROC_PATH,_T("-%I64ld  ExePipe Alive:%d State:%I64ld "),ID,m_Dialog->m_ExePipe.IsAlive(),m_Dialog->m_ExePipe.GetID());
		}
	}
}   


CElement::CElement(int64 ID,const TCHAR* Name)
:Mass(ID), m_Name(Name),m_Parent(NULL),m_EventID(0)
{
};

CElement::~CElement(){		

	Clear();
};

void CElement::Clear()
{
	ActomPtr It = m_ActomList.begin();
	while(It != m_ActomList.end()){
		delete *It;
		*It = NULL;
		It ++;
	};
	m_ActomList.clear();
}
Energy*  CElement::ToEnergy(){
	ePipeline* Pipe = new ePipeline(m_ID);
	if (Pipe)
	{
		ActomPtr It = m_ActomList.begin();
		while(It != m_ActomList.end()){
			Mass* m = *It;
			Energy* e = m->ToEnergy();
			if (e == NULL)
			{
				return NULL;
			}
			Pipe->Push_Directly(e);
			It ++;
		};
		return Pipe;
	}
	return NULL;
};

bool   CElement::FromEnergy(Energy* E){
	if(E->EnergyType() != TYPE_PIPELINE)return false;
	ePipeline& Pipe = *(ePipeline*)E;
	if (Pipe.Size() == m_ActomList.size())
	{
		int n=0; 
		ActomPtr It = m_ActomList.begin();
		while(It != m_ActomList.end()){
			Mass* m = *It;
			Energy* e = Pipe.GetEnergy(n++);
			bool ret = m->FromEnergy(e);
			if (!ret)
			{
				return false;
			}
			It ++;
		};
		return true;
	}
	return false;
}		


tstring CElement::GetName(){		
	if(RealtionType() == SERIES_RELATION){
		return _T("[-] ")+m_Name;	
	}else if (RealtionType() == SHUNT_RELATION)
	{
		return _T("[|] ")+m_Name;
	}
	return m_Name;
}

void CElement::GetAddress(ePipeline& Address){
	eINT* e = new eINT(m_ID);
    
	Address.Push_Front(e);
	if(m_Parent){ 
		m_Parent->GetAddress(Address);
	}
}

void CElement::Push(Mass* Object){
	assert(Object != NULL);

	m_ActomList.push_back(Object);
	if (Object->MassType() == MASS_ELEMENT)
	{
		CElement* e = (CElement*)Object;
		e->m_Parent = this;
	}
};

void CElement::InsertLogic(int64 Index,CElement* e)
{
	ActomPtr It = m_ActomList.begin();
	if (Index<0 || Index > m_ActomList.size())
	{
		Push(e);
	}else{
        It +=Index;
		m_ActomList.insert(It,e);

		if (e->MassType() == MASS_ELEMENT)
		{
			e->m_Parent = this;
		}
	}
}

Mass* CElement::GetChild(int64 ChildID){
	ActomPtr It = m_ActomList.begin();
	while(It != m_ActomList.end()){
		Mass* m = *It;
		if (m->m_ID == ChildID)
		{
						
			return m;
		}
		It++;
	}
	return NULL;
}
bool CElement::RemoveLoigc(int64 ChildID)
{
	ActomPtr It = m_ActomList.begin();
	while(It != m_ActomList.end()){
		Mass* m = *It;
		if (m->m_ID == ChildID)
		{
			delete m;
			m_ActomList.erase(It);

			return true;
		}
		It++;
	}
	return false;
}



CElement* CElement::FindFocusLogic(const tstring& FocusName){

	tstring LogicName;
	tstring LogicInstanceName;
	tstring LogicComment;
			
	GetLogicName(m_Name,LogicName,LogicInstanceName,LogicComment);			
	if (LogicInstanceName == FocusName)
	{
		return this;
	}

	ActomList::iterator it = m_ActomList.begin();
	while(it != m_ActomList.end())
	{
		Mass* m = *it;
		if (m->MassType() == MASS_ELEMENT)
		{
			CElement* E = (CElement*)m;

			GetLogicName(E->m_Name,LogicName,LogicInstanceName,LogicComment);			
			if (LogicInstanceName == FocusName)
			{
				return E;
			}
		}
		it++;
	}


	CElement* Elt = m_Parent;	
	if (Elt==NULL)
	{
		return NULL;
	}else{
		return Elt->FindFocusLogic(FocusName);
	}
}

Mass* CElement::FindMass(ePipeline& Address){
	int64 ID = Address.PopInt();  
    
	ActomPtr it = m_ActomList.begin();
	while(it != m_ActomList.end()){
		Mass* m = *it;
		if (m->m_ID==ID)
		{
			if (Address.Size())
			{
				if (m->MassType() == MASS_ELEMENT)
				{
					CElement* e = (CElement*)m;
					return e->FindMass(Address);
				}else{
					return NULL;
				}
			}else{
				return m;
			}
		}
	}
	return NULL;
}


MsgProcState CElement::MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex){
	MsgProcState ret = Dialog->m_Brain->ElementMsgProc(Dialog,this,ChildIndex,Msg,ExePipe,LocalAddress); //缺省处理
	return ret;
}

bool CElement::TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress)
{
	return true;
}

bool CElement::Do(CTaskDialog* Dialog,ePipeline& ExePipe, ePipeline& LocalAddress,CMsg& Msg){

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);
    
	int32 ChildIndex = -1 ;
	if (!Msg.IsReaded()) //先处理信息
	{

		MsgProcState ret= MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex);
		if (ret == RETURN_DIRECTLY)
		{
			return true;
		}
	}
	

	bool ret = TaskProc(Dialog,Msg,ChildIndex,ExePipe,LocalAddress);
	return ret;
}

//////////////////////////////////////////////////////////////////////////

bool CSeries::Do(CTaskDialog* Dialog,ePipeline& ExePipe, ePipeline& LocalAddress,CMsg& Msg){

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);
    
	int32 ChildIndex =0 ;

	ActomList::iterator It = m_ActomList.begin();

	if (!Msg.IsReaded()) //先处理信息
	{		
		ePipeline& ObjectAddress = Msg.GetReceiver();
		if (ObjectAddress.Size()==0) //该此Element处理信息,在处理自己的Child之前
		{
			ChildIndex = -1;  //-1表示自身,而不是某个child
			MsgProcState ret = CElement::MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex); 
			if (ret==RETURN_DIRECTLY)
			{
				return true;
			}

			It = m_ActomList.begin();
		} 
		else //根据地址在自己的子物体里寻找
		{
			int64 ID = ObjectAddress.PopInt();			
			while (It != m_ActomList.end())
			{
				Mass* Child = *It;
				if (Child->m_ID == ID)
				{
					break;
				}
				It++;
			}
			
			if (It == m_ActomList.end())
			{
				//先交给系统缺省处理(有时需要关闭无效事件)
			    int64 MsgID = Msg.GetMsgID();
				tstring MsgStr = Dialog->m_Brain->MsgID2Str(MsgID);

				ChildIndex = -1;
				CElement::MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex);
				
				ExePipe.SetID(RETURN_ERROR);
				ExePipe.GetLabel() = Format1024(_T("Error: Msg(%s) Address(%I64ld) Invalid"),MsgStr.c_str(),ID);
				return true;
			}else{
				
				Mass* Child = *It;
				if(Child->MassType() != MASS_ELEMENT){  //对于非MASS_ELEMENT，由父Element代为处理信息
					if (ObjectAddress.Size()!=0) //对于非MASS_ELEMENT,则肯定是最后一个目标地址
					{
						int64 MsgID = Msg.GetMsgID();
						tstring MsgStr = Dialog->m_Brain->MsgID2Str(MsgID);

						ExePipe.SetID(RETURN_ERROR);
						ExePipe.GetLabel() = Format1024(_T("Error: Msg(%s) Address(%I64ld) Invalid"),MsgStr.c_str(),ID);
						return true;
					};
					
					ChildIndex = It-m_ActomList.begin();
					MsgProcState ret = CElement::MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex);
					if (ret == RETURN_DIRECTLY)
					{
						return true;
					}
				}
			}
		}
	}

	//然后依据漫游产生的迭代器位置，继续正常的逻辑执行。
	while(ExePipe.IsAlive() && It!=m_ActomList.end()){ 

		Mass* Child = *It++;
	
		if(Child->MassType() == MASS_ELEMENT){
			CElement* Elt = (CElement*)Child;
			if(!Elt->Do(Dialog,ExePipe,LocalAddress,Msg)){
				ExePipe.SetID(RETURN_ERROR);
				if (ExePipe.GetLabel().size()==0)
				{
					ExePipe.GetLabel() = Format1024(_T("%I64ld: %s execute failure."),Child->m_ID,Child->GetName().c_str());
				}
				return true;
			};
		}
		else {					
			ExePipe.AutoTypeAB();   
			uint32 Type = Child->GetTypeAB();   

			if( !ExePipe.HasTypeAB(Type))  
			{
				ExePipe.SetID(RETURN_ERROR);
				ExePipe.GetLabel() = Format1024(_T("%I64ld: %s input data type checking failure:(%x,%x)"),Child->m_ID,Child->GetName().c_str(),Type,ExePipe.GetTypeAB());
				return true;
			}else{								
				if(!Child->Do(&ExePipe)){
					ExePipe.SetID(RETURN_ERROR);
					if (ExePipe.GetLabel().size()==0){
						ExePipe.GetLabel() = Format1024(_T("%I64ld: %s execute failure."),Child->m_ID,Child->GetName().c_str());
					}
					return true;
				};     
			}			
		};
		
		//根据情况作出不同处理
		
		int64 State = ExePipe.GetID();
		if (State == RETURN_NORMAL)//让正常执行拥有最大效率
		{
			continue;
		}
		switch(State){
		case RETURN_PIPE_BREAK:  //应该并连体去处理 
		case RETURN_DEBUG_PIPE_BREAK:		
			return true;
		case RETURN_GOTO_LABEL:
			{
				tstring& Label = ExePipe.GetLabel(); 

				tstring Name = GetLogicElementName(m_Name);
				if(Name==Label){
					ExePipe.SetID(RETURN_NORMAL);
					ExePipe.m_Label = _T("");
					It = m_ActomList.begin(); 
				}else{
					return true;
				}
			}
			break;
		case RETURN_DEBUG_GOTO_LABEL:
			{
				tstring& Label = ExePipe.GetLabel(); 

				tstring Name = GetLogicElementName(m_Name);
				if(Name==Label){
					ExePipe.m_Label = _T("");	
					It = m_ActomList.begin();
					Mass* NextChild = *It;
					LocalAddress.PushInt(NextChild->m_ID);
					Dialog->NotifyPause(ExePipe,LocalAddress);
				}	
				return true;
			}
		case RETURN_BREAK:
			{				
				if (It == m_ActomList.end()) //让父空间去处理暂停
				{
					return true;
				}else{
					Mass* NextChild = *It;
					LocalAddress.PushInt(NextChild->m_ID);
					Dialog->NotifyPause(ExePipe,LocalAddress);
				}
				return true;
			}
			break;
/*
		case RETURN_WAIT:
		case RETURN_DEBUG_WAIT:
		case RETURN_PAUSE:
		case RETURN_ERROR:
		    return true;
*/		
		default:			
			return true;
		}
		
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
void CShunt::InsertLogic(int64 Index,CElement* e){

	ActomPtr It = m_ActomList.begin();
	if (Index<0 || Index > m_ActomList.size())
	{
		Push(e);
		ePipeline Empty;
		m_TempResultList.PushPipe(Empty);
	}else{
        It +=Index;
		m_ActomList.insert(It,e);
		
		ePipeline Empty;
		m_TempResultList.InsertEnergy(Index,Empty.Clone());

		if (e->MassType() == MASS_ELEMENT)
		{
			e->m_Parent = this;
		}
	
	}

};

bool CShunt::RemoveLoigc(int64 ChildID){
	int n=0;
	ActomPtr It = m_ActomList.begin();
	while(It != m_ActomList.end()){
		Mass* m = *It;
		if (m->m_ID == ChildID)
		{
			delete m;
			m_ActomList.erase(It);
			
			m_TempResultList.EraseEnergy(n,1);
			return true;;
		}
		n++;
		It++;
	}
	return false;
}


void CShunt::ShuntBegin(ePipeline& ExePipe)
{	
	ExePipe.AutoTypeAB();
    m_TempPipe = ExePipe;
    m_TempPipe.SetID(1); //表示已经初始化

	int32 n = m_ActomList.size();
	while(n--!=0)
	{
		ePipeline Empty;
		m_TempResultList.PushPipe(Empty);
	}

	m_TempResultList.SetID(0); //表示已经完成执行的分支数目
}

bool CShunt::GetExePipe(int32 n,ePipeline& ExePipe){
	assert(m_TempPipe.GetID()); //已经初始化
	ePipeline* TempResult = (ePipeline*)m_TempResultList.GetData(n);

	if (TempResult->m_ID != 0) //已经执行过的忽略
	{
	   return false;
	}
	
	TempResult->SetID(1);    //表示执行过

	ExePipe = m_TempPipe;
	return true;
};

void CShunt::SaveTempResult(int32 n,ePipeline& ExePipe)
{
	assert(n<m_TempResultList.Size());
	ePipeline& Pipe = *(ePipeline*)m_TempResultList.GetData(n);
    assert(Pipe.GetID()==1); //指示已经执行过    
	assert(Pipe.Size()==0);
	Pipe << ExePipe;
    m_TempResultList.m_ID ++;
	assert(m_TempResultList.m_ID <= m_ActomList.size());
}

bool CShunt::IsInited(){
	return m_TempPipe.GetID() !=0;
};

bool CShunt::IsCompleted(){
	return m_TempResultList.m_ID == m_ActomList.size();
};

void CShunt::ShuntEnd(ePipeline& ExePipe){
	ExePipe.Clear();
	for (int i=0; i<m_TempResultList.Size(); i++)
	{
		ePipeline& Pipe = *(ePipeline*)m_TempResultList.GetData(i);
		ExePipe<<Pipe;
	}
	Reset();
}
void CShunt::Reset()
{
	m_TempPipe.Clear();
	m_TempPipe.SetID(0); //指示是否初始化
	m_TempResultList.Clear();
	m_TempResultList.SetID(0); //已完成分支计数

	UpdateEventID();
}

bool CShunt::Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);
	
	int64 MsgID = Msg.GetMsgID();
	
	int ChildIndex =0;
	bool  bUseDefaultExePipe = false;

	//如果指定了目标地址，则优先根据目标地址寻找具体的执行者
	if (!Msg.IsReaded())
	{	
		//信息已经过期，直接返回,出现这种情况一般是RESET后收到之前的信息反馈	
		int64 MsgEventID = Msg.GetEventID();
		if (MsgEventID !=0 && MsgEventID< GetEventID())  
		{
			Dialog->m_Brain->OutputLog(LOG_MSG_RUNTIME_TIP,_T("WARNING: (%I64ld)%s Msg:%I64ld Event:%I64ld < Shunt EventID:%I64ld"),m_ID,GetName().c_str(),Msg.GetMsgID(),MsgEventID,GetEventID());	
			ExePipe.Break();
			return true;
		}
		
		ePipeline& ObjectAddress = Msg.GetReceiver();
		if (ObjectAddress.Size()==0) //该此Element处理信息,在处理自己的Child之前
		{     


			ChildIndex =-1;
			MsgProcState ret = MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex);
            if (ret == RETURN_DIRECTLY)
            {
				return true;
            }
			ChildIndex =0;
			
		} 
		else //根据地址在自己的子物体里寻找
		{
			int64 ID = ObjectAddress.PopInt();
			assert(ID>0);
		
			ActomList::iterator It = m_ActomList.begin();
			while (It != m_ActomList.end())
			{
				Mass* Child = *It;
				if (Child->m_ID == ID)
				{
					break;
				}
				It++;
			}
			
			if (It == m_ActomList.end())
			{
				//先交给系统缺省处理
				ChildIndex =-1;
				MsgProcState ret = MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex);
				if (ret == RETURN_DIRECTLY)
				{
					return true;
				}
                
				//与并联不同，不视为错误，而是忽略，检查是否所有分支都完成，如果没有完成则继续等待
				ChildIndex = m_TempResultList.Size(); //确保直接检查分支是否都完成
				
			}
			else{
				/*
				得到目标MASS的序列号，优先执行这个，然后会接着寻找没有执行的继续执行
				可能有人会问，这样i之前的岂不是没有机会执行？
				当一个并联体接收到指定目标的信息时，意味着i之前的MASS肯定都执行过，
				虽然不一定都执行完成，没完成的同样会接收到指定地址的信息。
				*/
				ChildIndex = It-m_ActomList.begin();		
				
				Mass* Child = *It;
				if(Child->MassType() != MASS_ELEMENT){
					if (ObjectAddress.Size()!=0) //对于非MASS_ELEMENT,则肯定是最后一个目标地址
					{
						int64 MsgID = Msg.GetMsgID();
						tstring MsgStr = Dialog->m_Brain->MsgID2Str(MsgID);

						ExePipe.SetID(RETURN_ERROR);
						ExePipe.GetLabel() = Format1024(_T("Error: Msg(%s) Address(%I64ld) Invalid"),MsgStr.c_str(),ID);
						return true;
					};

									
					MsgProcState ret = MsgProc(Dialog,Msg,ExePipe,LocalAddress,ChildIndex);
					if (ret == RETURN_DIRECTLY)
					{
						return true;
					}

				}

				//重入都使用缺省管道
				bUseDefaultExePipe = true;
			}
		}	
	}
	
	if (!IsInited())
	{
		ShuntBegin(ExePipe);
	}

	int n = m_TempResultList.Size();
	assert(n==m_ActomList.size());

	int64 State = ExePipe.GetID();

	while(ExePipe.IsAlive() && ChildIndex<n){ 
		
		if (!bUseDefaultExePipe) //如果不使用缺省的数据管道则取出一个未执行的分支
		{		
			if (!GetExePipe(ChildIndex,ExePipe)) //已经执行过的忽略
			{
				ChildIndex++;
				continue;
			}
		}else{
			bUseDefaultExePipe = false;
		}
		
		Mass* Child = m_ActomList[ChildIndex];
	
		ExePipe.SetID(State);

		if(Child->MassType() == MASS_ELEMENT){
			CElement* Elt = (CElement*)Child;
			if(!Elt->Do(Dialog,ExePipe,LocalAddress,Msg)){
				ExePipe.SetID(RETURN_ERROR); 
				if (ExePipe.GetLabel().size()==0){
					ExePipe.GetLabel() = Format1024(_T("%I64ld: %s execute failure."),Child->m_ID,Child->GetName().c_str());
				}
				return true;
			};

			//处理信息后有可能直接返回，比如MSG_EVENT_TICK
			//但如果恰好是最后一个分支，不检查这个会被错误的视为Completed
			if (!ExePipe.IsAlive()) 
			{
				return true;
			}
		}
		else {					
			uint32 Type = Child->GetTypeAB();             		
			if( !ExePipe.HasTypeAB(Type))  
			{
				ExePipe.SetID(RETURN_ERROR);
				ExePipe.GetLabel() = Format1024(_T("%I64ld: %s input data type checking failure:(%x,%x)"),Child->m_ID,Child->GetName().c_str(),Type,ExePipe.GetTypeAB());
				return true;
			}else{
				
				if(!Child->Do(&ExePipe)){
					Reset();			
					ExePipe.SetID(RETURN_ERROR); 
					if (ExePipe.GetLabel().size()==0){
						ExePipe.GetLabel() = Format1024(_T("%I64ld: %s execute failure."),Child->m_ID,Child->GetName().c_str());
					}
					return true;
				};     
			}		
		};

		State = ExePipe.GetID();
		if (State == RETURN_NORMAL)//让正常执行拥有最大效率
		{
			SaveTempResult(ChildIndex++,ExePipe);
			continue;
		}

		switch(State){
		case RETURN_PIPE_BREAK: 
			{
				ExePipe.SetID(RETURN_NORMAL);
				State = RETURN_NORMAL;
				ExePipe.Clear(); //软中断，此分支无效，可以继续执行下一个
				SaveTempResult(ChildIndex,ExePipe);
			}
			break;
		case RETURN_DEBUG_PIPE_BREAK:
			{
				ExePipe.SetID(RETURN_BREAK);
				State = RETURN_BREAK;
				ExePipe.Clear(); //软中断，此分支无效，可以继续执行下一个
				SaveTempResult(ChildIndex,ExePipe);
			}
			break;
		case RETURN_GOTO_LABEL:			
		case RETURN_DEBUG_GOTO_LABEL:
			assert(ExePipe.GetLabel() != m_Name); //理论上不存在GOTO并联体的问题
												  /*发生跳转意味着此并联体执行完毕,
												  只取此分支的执行结果返回
    		                                      */
			Reset();  
			return true;
		case RETURN_BREAK:
			{
				SaveTempResult(ChildIndex,ExePipe);
				
				if (IsCompleted()) //如果已经完成,则直接返回,调试中断留给父空间处理
				{
					ShuntEnd(ExePipe);
				}else{
					ChildIndex++;
					if (ChildIndex<n)
					{
						Mass* NextChild = m_ActomList[ChildIndex];
						LocalAddress.PushInt(NextChild->m_ID);
                        	
						GetExePipe(ChildIndex,ExePipe);//预先得到管道数据，这样重入时分支才会在缺省管道得到正确的数据
						Dialog->NotifyPause(ExePipe,LocalAddress);
					}else{
						State = ExePipe.GetID();
						if(State==RETURN_NORMAL)
						{
							ExePipe.SetID(RETURN_WAIT);
						}else{
							ExePipe.SetID(RETURN_DEBUG_WAIT);
						}
					}
				}	
				return true;
			}
		case RETURN_WAIT:
			{
				//注意：并联意味着当一个child需要等待回复时，可以无视，继续执行下一个
				State = RETURN_NORMAL;
			}
			break;
		case RETURN_DEBUG_WAIT:
			{
				State = RETURN_BREAK;
				
			}
			break;
		case RETURN_PAUSE:
			return true;
		case RETURN_ERROR:
			return true;
		default:
			Reset();
			return true;
		}
		
		ChildIndex++;
	}
	
	if(IsCompleted()){//所有并联体执行完以后在把结果返回给管道
		ShuntEnd(ExePipe);
	}else{
		State = ExePipe.GetID();
		if (State == RETURN_NORMAL)
		{
			ExePipe.SetID(RETURN_WAIT);
		}
		assert(ExePipe.GetID() == RETURN_WAIT || ExePipe.GetID() == RETURN_DEBUG_WAIT);
	}

	return true;
}


