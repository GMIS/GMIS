#pragma warning (disable:4786)

#include "LogicElement.h"
#include "LogicTask.h"
#include "LogicDialog.h"


tstring GetLogicElementName(tstring& srcName)
{

	tstring::size_type n;
	if ( (n=srcName.find(_T(":")))  != string::npos || (n=srcName.find(_T("："))) != string::npos)
	{
		tstring Name = srcName.substr(0,n);
		return Name;
	}
	return TriToken(srcName);
}

Energy*  CCapacitor::ToEnergy(){
	ePipeline* Pipe = new ePipeline(m_ID);
	if (Pipe)
	{
		Pipe->Push_Directly(m_Temp.Clone());
		Pipe->PushString(m_Name);
		Pipe->PushInt(m_TaskBeginTime);
		return Pipe;
	}
	return NULL;
};

bool     CCapacitor::FromEnergy(Energy* E){
	ENERGY_TYPE Type = E->EnergyType();
	if(Type != TYPE_PIPELINE){
		return false;
	}
	ePipeline& Pipe = *(ePipeline*)E;
	if (Pipe.GetID() == m_ID)
	{
		eElectron e;
		Pipe.Pop(&e);
		ePipeline* Temp = (ePipeline*)e.Get();
		m_Temp << *Temp;
		m_Name = Pipe.PopString();
		m_TaskBeginTime = Pipe.PopInt();
		return true;
	}
	return false;
}		

void CCapacitor::Reset(){ 
	m_Temp.Clear();
};

bool CCapacitor::Do(Energy* E)
{
	ePipeline* Pipe = (ePipeline*)E;
	if (m_Task->m_BeginTime != m_TaskBeginTime) //初始执行,重复执行一个逻辑任务时必须执行这一步
	{
		Reset();
		m_TaskBeginTime = m_Task->m_BeginTime; //避免多次Reset
	}
	
	if( m_Temp.Size() == 0 ){       // 充电
		m_Temp << *Pipe;
	}
	else{                           // 放电 
		*Pipe << m_Temp; 
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

Energy*  CDiode::ToEnergy(){
	ePipeline* Pipe = new ePipeline(m_ID);
	if (Pipe)
	{
		Pipe->PushInt(m_Num);
		return Pipe;
	}
	return NULL;
};

bool     CDiode::FromEnergy(Energy* E){
	if(E->EnergyType() != TYPE_PIPELINE)return false;
	ePipeline& Pipe = *(ePipeline*)E;
	if (Pipe.GetID() == m_ID)
	{
		m_Num = Pipe.PopInt();
		return true;
	}
	return false;
}		

bool CDiode::Do(Energy* E)
{		
	ePipeline* Pipe = (ePipeline*)E;
		
	int64 n = Pipe->PopInt();
	if(m_Num == n )
	{
		return true;
	}

	//否则不允许通过
	Pipe->Clear();
				
	int64 ID = Pipe->GetID();
	if (ID == RETURN_BREAK)
	{
		Pipe->SetID(RETURN_DEBUG_PIPE_BREAK);
	}else{
		Pipe->SetID(RETURN_PIPE_BREAK); 
	}	
	return true;
}
//////////////////////////////////////////////////////////////////////////
Energy*  CInductor::ToEnergy(){
	ePipeline* Pipe = new ePipeline(m_ID);
	if (Pipe)
	{
		Pipe->Push_Directly(m_Temp.Clone());
		Pipe->PushString(m_Name);
		Pipe->PushInt(m_TaskBeginTime);
		return Pipe;
	}
	return NULL;
};

bool     CInductor::FromEnergy(Energy* E){
	if(E->EnergyType() != TYPE_PIPELINE)return false;
	ePipeline& Pipe = *(ePipeline*)E;
	if (Pipe.GetID() == m_ID)
	{
		eElectron e;
		Pipe.Pop(&e);
		ePipeline* Temp = (ePipeline*)e.Get();
		m_Temp << *Temp;
		m_Name = Pipe.PopString();
		m_TaskBeginTime = Pipe.PopInt();
		return true;
	}
	return false;
}		

bool CInductor::Do(Energy* E)
{
	
	ePipeline* Pipe = (ePipeline*)E;
	if (m_Task->m_BeginTime != m_TaskBeginTime) //初始执行,重复执行一个逻辑任务时必须执行这一步
	{
		Reset();
		m_TaskBeginTime = m_Task->m_BeginTime; //避免多次Reset
	}
	
	if(Pipe->Size() == 0 ){
		*Pipe<< m_Temp;
	}
	else{
		m_Temp.Clone(*Pipe);  
		Pipe->Clear();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
Energy*  CResistor::ToEnergy(){
	ePipeline* Pipe = new ePipeline(m_ID);
	if (Pipe)
	{
		Pipe->PushInt(m_Num);
		return Pipe;
	}
	return NULL;
};

bool     CResistor::FromEnergy(Energy* E){
	if(E->EnergyType() != TYPE_PIPELINE)return false;
	ePipeline& Pipe = *(ePipeline*)E;
	if (Pipe.GetID() == m_ID)
	{
		m_Num = Pipe.PopInt();
		return true;
	}
	return false;
}				

bool CResistor::Do(Energy* E)
{
	ePipeline* Pipe = (ePipeline*)E;
	
	if(m_Num == 0)return true;
	if(m_Num == -1 || m_Num>=Pipe->Length())Pipe->Clear();	  
	else{	
		eElectron  Data;
		for(int i=0; i<m_Num; i++)
		{	  			  
			Pipe->Pop(&Data);
		}
	}	  
	return true;
}

//////////////////////////////////////////////////////////////////////////
Energy*  CMassRef::ToEnergy(){
	ePipeline* Pipe = new ePipeline(m_ID);
	if (Pipe)
	{
		Pipe->PushString(m_Name);
		return Pipe;
	}
	return NULL;
};

bool     CMassRef::FromEnergy(Energy* E){
	if(E->EnergyType() != TYPE_PIPELINE)return false;
	ePipeline& Pipe = *(ePipeline*)E;
	if (Pipe.GetID() == m_ID)
	{
		m_Name = Pipe.PopString();
		return true;
	}
	return false;
}				
		

//////////////////////////////////////////////////////////////////////////

bool CEncodePipe::Do(Energy* E)
{
	ePipeline& Pipe = *(ePipeline*)E;
	ePipeline* NewPipe = new ePipeline;
	if (!NewPipe)
	{
		return false;
	}
	*NewPipe << Pipe;
	Pipe.Push_Directly(NewPipe);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CDecodePipe::Do(Energy* E)
{
	ePipeline& Pipe = *(ePipeline*)E;
	
	for (int i=0; i<Pipe.Size(); i++)
	{
		Energy* e = (Energy*)Pipe.GetData(i);
		if (e->EnergyType() == TYPE_PIPELINE)
		{
			ePipeline* p = (ePipeline*)e;  //把p含有的数据插入到p所在位置
			
			int32 n = p->Size();

			Pipe.MoveEnergy(i);
					
			Pipe.InsertEnergy(i,*p);
			delete p;					

			i+=n;
		}
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////

