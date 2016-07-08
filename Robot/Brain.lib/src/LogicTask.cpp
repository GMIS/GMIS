// BrainTask.cpp: implementation of the CBrainTask class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)
#pragma warning(disable: 4244)

#include "Brain.h"
#include "LogicTask.h"
#include "Element.h"
#include "LogicDialog.h"
#include "InstinctDefine.h"

//#include "Arm.h"


void GetLogicName(tstring& SrcLoigcName, tstring& LogicName,tstring& LogicInstanceName,tstring& LogicComment)
{
	tstring::size_type n;
	if ( (n=SrcLoigcName.find(_T(":")))  != string::npos || (n=SrcLoigcName.find(_T("："))) != string::npos)
	{
		LogicName = SrcLoigcName.substr(0,n);
		LogicComment = SrcLoigcName.substr(n+1);
	}else{
		LogicName = SrcLoigcName;
	}

	n = LogicName.find(_T("@"));
	if (n  != string::npos)
	{
		LogicInstanceName = LogicName.substr(n+1);
		LogicName = LogicName.substr(0,n);
	}

}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



CLogicTask::CLogicTask()
:CSeries(0,_T("Logic"))
{
	Reset(0,-1);
}

CLogicTask::CLogicTask(CLogicTask* Parent)
:CSeries(0,_T("Logic")){
	Reset(0,-1);
	m_Parent = Parent;
}

CLogicTask::CLogicTask(int64 TaskID,int64 UserDialogID)
:CSeries(0,_T("Task"))
{
	Reset(TaskID,UserDialogID);
}

CLogicTask::~CLogicTask()
{

}


	
void CLogicTask::Reset(int64 TaskID,int64 UserDialogID){
	
	m_Parent = NULL;
	m_ID = TaskID;
	m_UserDialogID = UserDialogID;
    
	m_BeginTime = 0;

	m_LogicText = _T("");
	m_LogicData.Clear();
	m_State = COMMON_ACTION;
	m_bDepend = FALSE;

	m_ElementCount = 0;
	m_MassCount = 0;
	m_ExecuteCount = 0;	
	
	Clear();
}

void CLogicTask::Clear(){

	ActomPtr It = m_ActomList.begin();
	while(It != m_ActomList.end()){
		delete *It;
		*It = NULL;
		It ++;
	};
	m_ActomList.clear();
}


CLogicTask* CLogicTask::GetRootTask(){
	if (m_Parent==NULL)
	{
		return this;
	}

	CLogicTask* Task = (CLogicTask*)m_Parent;
	return Task->GetRootTask();
}


Energy*  CLogicTask::ToEnergy(){
	ePipeline* Pipe = new ePipeline;
	eElectron e(Pipe);
    if(Pipe){
	    Pipe->PushString(m_Name);
		Pipe->PushInt(m_UserDialogID);
		Pipe->PushInt(m_BeginTime);
		Pipe->PushString(m_LogicText);
		Pipe->PushPipe(m_LogicData);
		Pipe->PushInt(m_State);
		Pipe->PushInt(m_bDepend);
		Pipe->PushInt(m_ExecuteCount);
		Pipe->PushInt(m_MassCount);
		Pipe->PushInt(m_ActionType);

		Energy* E = CSeries::ToEnergy();
		if (E==NULL)
		{
			return NULL;
		}
		Pipe->Push_Directly(E);
		
		return e.Release();
	}
	return NULL;
}

bool     CLogicTask::FromEnergy(CLogicDialog* Dialog,Energy* E){
	
	ePipeline* Pipe  = (ePipeline*)E;
	m_Name = Pipe->PopString();
	m_UserDialogID = Pipe->PopInt();
	m_BeginTime = Pipe->PopInt();
	m_LogicText = Pipe->PopString();
	eElectron e;
	Pipe->Pop(&e);
	ePipeline* p = (ePipeline*)e.Get();
	m_LogicData << *p;
	m_State = Pipe->PopInt();
	m_bDepend = Pipe->PopInt();
	m_ExecuteCount = Pipe->PopInt();
	m_MassCount  = Pipe->PopInt();
	m_ActionType = Pipe->PopInt();

	if(!Dialog){
		return true;
	}

	Pipe->Pop(&e);
	ePipeline* MassData = (ePipeline*)e.Get();

	if(m_LogicData.Size()){
			            
		ePipeline LogicData = m_LogicData;
		CElement* Elt = CompileSentence(Dialog,m_LogicText,&LogicData);
		if(!Elt){
			return false;
		}
		Push(Elt);
	}
    bool ret = CSeries::FromEnergy(MassData);
	return ret;
}


CLogicTask& CLogicTask::operator<<(CLogicTask& Task){
	if (&Task == this)
	{
		return *this;
	}

	Reset(0,-1);
	m_ID = Task.m_ID;
	m_Name = Task.m_Name;
	m_ActomList.swap(Task.m_ActomList);
    
	m_BeginTime = Task.m_BeginTime;

	m_LogicText = Task.m_LogicText;
	m_LogicData <<Task.m_LogicData;
	
	m_State = Task.m_State;
	m_bDepend = Task.m_bDepend;

	m_ElementCount = Task.m_ElementCount;
	m_MassCount    = Task.m_MassCount;
	m_ExecuteCount = Task.m_ExecuteCount;

	
	Task.Reset(0,-1);

	return *this;
}

bool  CLogicTask::SetBreakPoint(ePipeline& Path,BOOL bEnable ){
	
	if (Path.Size()==0 || m_ActomList.size()==0)
	{
		return false;
	}
	int64 ID = Path.PopInt();  //第一个ID不是真实的
	assert(ID == 1);

	CElement* Parent = this;
	while(Path.Size()){
		int64 ID = Path.PopInt();

		bool bFind = false;
		ActomList::iterator it = Parent->m_ActomList.begin();

		while(it != Parent->m_ActomList.end())
		{
			Mass* Child = *it;
			if (Child->m_ID == ID)
			{
				bFind = true;
				if (Path.Size()==0)
				{
					if (!bEnable)
					{
						if (Child->MassType() == MASS_USER)
						{
							CDebugBreak* b = (CDebugBreak*)Child;
							*it = b->Release();
							delete b;
							return true;
						}
					} 
					else
					{
						//已经是BreakMass了
						if (Child->MassType() == MASS_USER)
						{
							return true;
						}
						CDebugBreak* b = new CDebugBreak(Child);
						*it = b;
						return true;
					}
					return false;
				}else{
					assert(Child->MassType() == MASS_ELEMENT);
					Parent = (CElement*)Child;
				}
				break;
			}
			it++;
		
		}
		if (!bFind)
		{
			return false;
		}

	}
	return false;
}
	
void  CLogicTask::GetDebugItem(ePipeline& ParentPipe,Mass* m){
	
	if (m==NULL)
	{
		m = this;
		assert(ParentPipe.GetID() ==m_BeginTime);
	}

	ePipeline* MassPipe = new ePipeline(m->GetName().c_str(),m->m_ID);
	ParentPipe.Push_Directly(MassPipe);

	MASS_TYPE Type = m->MassType();
	MassPipe->PushInt(Type);
    
	if (Type == MASS_ELEMENT)
	{
		CElement* E = (CElement*)m;
		vector<Mass*>::iterator it = E->m_ActomList.begin();
		while (it != E->m_ActomList.end())
		{
			Mass* ChildMass = *it;
			GetDebugItem(*MassPipe,ChildMass);
			it++;
		}
	}
}



/////////////////////////////////////////////////


bool CLogicTask::Compile(CLogicDialog* Dialog,ePipeline* Sentence){

	if( Sentence->Size() == 0 ){
		m_CompileError = _T("Invalid  Sentence !");
		return NULL;
	}
	
	Clear();
	
	m_ActionType = 0;
	
	m_CompileError = _T("");
	m_CurThinkLogicName = _T("");

	m_LogicData = *Sentence;
	
	m_TempElementList.clear();
	
	CElement* E= NULL;
	if (Dialog->GetWorkMode() == WORK_TEST)
	{
		E = CompileSentenceForTest(Dialog,m_LogicText,Sentence);
	}else{
		E = CompileSentence(Dialog,m_LogicText,Sentence);
	}

	if (E == NULL)
	{
		if (m_CompileError.size()==0)
		{
			m_CompileError = _T("Task compile error");
		}
		return false;
	}

	Push(E);
	//检查引用是否有效已经行为类型是否冲突ACTION
	if(m_ActionType & OUTER_ACTION){
		if(m_ActionType & INTER_ACTION  || m_ActionType & INDE_INTER_ACTION){
			m_CompileError = _T("错误: 外部行为不能和内部行为混合使用!");
			return false;
		}
	}
	else if(m_ActionType & INDE_INTER_ACTION){
		if(Sentence->Size() != 1){
			m_CompileError = _T("错误: 内部独立行为只能单独执行");
			return false;
		}
	}

	return true;
};

CElement* CLogicTask::CompileSentenceForTest(CLogicDialog* Dialog,tstring& LogicText,ePipeline* Sentence)
{
	CBrainTestElt* E = new CBrainTestElt(m_MassCount++);
	std::auto_ptr<CElement> ptr(E); //如果编译错误则自动删除E

	for (int i=0; i<Sentence->Size();i++)
	{
		ePipeline* Clause = (ePipeline*)Sentence->GetEnergy(i);	
		int64 InstinctID  = *(int64*)Clause->GetData(0);

		Energy* Param = NULL;
		if(Clause->Size()>1)Param = Clause->GetEnergy(1);

		if (InstinctID<0)
		{
			InstinctID = -InstinctID;
		}

		if (BelongCommanAction(InstinctID))
		{
			m_ActionType |= COMMON_ACTION;
		}else if (BelongIndeInterAction(InstinctID)) //忽略内部命令
		{
			continue;
		}
		else if (BelongInterAction(InstinctID))
		{
			continue;
		}else{
			m_ActionType |= OUTER_ACTION;
		}

		Mass* m = NULL;

		if(BelongInstinct(InstinctID)){
			if(InstinctID == INSTINCT_SET_LABEL)
			{
				continue;
			}
			else if(InstinctID == INSTINCT_USE_LOGIC)
			{
				assert(Param != NULL); 
				tstring Name = *(tstring*)Param->Value();  

				tstring LogicName;
				tstring LogicInstanceName;
				tstring LogicComment;

				GetLogicName(Name,LogicName,LogicInstanceName,LogicComment);

				if (LogicName == m_CurThinkLogicName || LogicName == Dialog->m_CurLogicName)
				{
					m_CompileError = Format1024(_T("Error: Think [%s]递归调用逻辑[%s]"),m_CurThinkLogicName.c_str(),m_CurThinkLogicName.c_str());
					return NULL;
				}
				if (LogicInstanceName.size())
				{
					CElement* e = E->FindFocusLogic(LogicInstanceName);
					if (e)
					{
						m_CompileError = Format1024(_T("Error: 逻辑实例名[%s]已经被使用"),LogicInstanceName.c_str());
						return NULL;
					}
				}


				if (LogicName[0] == _T('<')) //全局
				{
					ePipeline* lg = CBrainMemory::FindGlobalLogic(LogicName);
					if(lg == NULL) {
						m_CompileError = Format1024(_T("Error: 没有找到全局逻辑[%s]"),LogicName.c_str());
						return NULL;
					}
					if(lg->Size() != 7) {
						m_CompileError = Format1024(_T("Error: 找到的全局逻辑[%s]无效"),LogicName.c_str());
						return NULL;
					}				

					int32 ActionState = *(int32*)lg->GetData(0);

					m_ActionType |= ActionState;

					ePipeline* LogicData = (ePipeline*)lg->GetData(5);

					CLogicTask* RootTask = GetRootTask();

					CLogicTask NestTask(RootTask);

					bool ret = NestTask.Compile(Dialog,LogicData);
					if (!ret)
					{
						m_CompileError = Format1024(_T("Error: 编译全局逻辑[%s]失败"),LogicName.c_str());
						return NULL;
					}

					assert(NestTask.m_ActomList.size()==1);
					CBrainTestElt* s = new CBrainTestElt(m_MassCount++ ,LogicName.c_str());
					s->m_ActomList.swap(NestTask.m_ActomList);
					m = s;

				}else{

					CLogicTask* LogicTask;
					CLocalLogicCell* lg = Dialog->FindLogic(LogicName);
					if(lg == NULL) {
						m_CompileError = Format1024(_T("Error: Not find Local logic[%s]"),LogicName.c_str());
						return NULL;
					}		
					else if(!lg->IsValid()) {
						m_CompileError = Format1024(_T("Error: Local logic[%s] not valid"),LogicName.c_str());
						return NULL;
					}else{
						LogicTask = &lg->m_Task;		
					}

					ePipeline LogicPipe(LogicTask->m_LogicData);

					m = CompileSentenceForTest(Dialog,Name,&LogicPipe);

				}	
			}
			else{
				m = GetInstinctInstance(Dialog,InstinctID, Param);
			}
		}else{ //客户自定义命令
			ePipeline Pipe;
			CLogicThread* Think = Dialog->GetThink();
			if(!Think->RetrieveLogic(InstinctID,&Pipe,false))
			{
				deque<tstring> CommandList;
				Think->GetCustomCommandText(InstinctID,CommandList);
				deque<tstring>::iterator It = CommandList.begin();
				m_CompileError = _T("can't compile custom commond : \n");
				while (It != CommandList.end())
				{
					tstring& Command = *It;
					Command+=_T('\n');
					m_CompileError += Command;
					It++;
				}				
				return NULL;
			}

			CLogicTask NestTask(this);
			bool ret = NestTask.Compile(Dialog,&Pipe);
			if (ret)
			{
				assert(NestTask.m_ActomList.size()==1);
				m = NestTask.m_ActomList.back();
				NestTask.m_ActomList.clear();
			}
		}

		if(m==NULL){
			return NULL;
		}
		E->Push(m);		

		Dialog->SentenceCompileProgress(Sentence->Size(),i);
	}


	ptr.release(); //编译没发生错误，则释放控制权

	CElement* ExecuteBody = E;
	ExecuteBody->m_Name = LogicText;


	return ExecuteBody;
}

CElement* CLogicTask::CompileSentence(CLogicDialog* Dialog,tstring& LogicText,ePipeline* Sentence)
{	
	
	CSeries* E = new CSeries(m_MassCount++); 
	
	std::auto_ptr<CElement> ptr(E); //如果编译错误则自动删除E


	deque<CElement*>  ElementStack;
	ElementStack.push_front(E);

	bool bShunt = false;
	
	for (int i=0; i<Sentence->Size();i++)
	{
		ePipeline* Clause = (ePipeline*)Sentence->GetEnergy(i);	
		int64 InstinctID  = *(int64*)Clause->GetData(0);
		
		Energy* Param = NULL;
		if(Clause->Size()>1)Param = Clause->GetEnergy(1);

		if (InstinctID<0)
		{
			bShunt = true;
			InstinctID = -InstinctID;
		}else{
			bShunt = false;
		}
		
		if (BelongCommanAction(InstinctID))
		{
			m_ActionType |= COMMON_ACTION;
		}else if (BelongIndeInterAction(InstinctID))
		{
			m_ActionType |= INDE_INTER_ACTION;
		}
		else if (BelongInterAction(InstinctID))
		{
			m_ActionType |= INTER_ACTION;
		}else{
			m_ActionType |= OUTER_ACTION;
		}
		
	
		CElement* E = ElementStack.front();
		
		if(bShunt){
			if (E->RealtionType()== SERIES_RELATION)
			{	
				Mass* e = E->m_ActomList.back();
				E->m_ActomList.pop_back();
				bool bIsLabel = E->m_Name.find(_T(":(label)")) != tstring::npos;
				if (!bIsLabel && E->m_ActomList.size()==0)
				{				
					CElement* Shunt = new CShunt(E->m_ID); //取代当前E
					
					std::auto_ptr<CElement> ptr1(Shunt);
					ptr = ptr1;

					ElementStack.pop_front();
					ElementStack.push_front(Shunt);

					E = Shunt;
                    E->Push(e);
				}
				else{
					CElement* Shunt = new CShunt(e->m_ID); //交换ID是因为e将作为Shunt的子空间，但它比shunt先生成，搞乱了ID的顺序
					e->m_ID = m_MassCount++;
					
					Shunt->Push(e);				
					E->Push(Shunt);
					
					ElementStack.push_front(Shunt);
					E = Shunt;
				}
			}
		}
		else {
			if (E->RealtionType()== SHUNT_RELATION)
			{	
				//弹出并联体				
				ElementStack.pop_front();

				if (ElementStack.size())
				{
					E = ElementStack.front();		
				}else{
					CElement* Series = new CSeries(E->m_ID);
					E->m_ID = m_MassCount++;
					Series->Push(E);
					ElementStack.push_front(Series);
					E = Series;
				}
			}
		}
		
		
		Mass* m = NULL;
				
		if(BelongInstinct(InstinctID)){
			if(InstinctID == INSTINCT_SET_LABEL)
			{
				tstring Label = *(tstring*)Param->Value();
				
				if (E->RealtionType()== SHUNT_RELATION)
				{
					m_CompileError =Format1024(_T("Set label %s 不能用于并联 \n"),Label.c_str());
					return NULL;
				}
				
				Label = Label+_T(":(label)");
				CElement* Series = new CSeries(m_MassCount++,Label.c_str());
				ElementStack.push_front(Series);
				
				m = Series;	
			}
			else if(InstinctID == INSTINCT_USE_LOGIC)
			{
				assert(Param != NULL); 
				tstring Name = *(tstring*)Param->Value();  
				
				tstring LogicName;
				tstring LogicInstanceName;
				tstring LogicComment;

				GetLogicName(Name,LogicName,LogicInstanceName,LogicComment);

				if (LogicName == m_CurThinkLogicName || LogicName == Dialog->m_CurLogicName)
				{
					m_CompileError = Format1024(_T("Error: Think [%s]递归调用逻辑[%s]"),m_CurThinkLogicName.c_str(),m_CurThinkLogicName.c_str());
					return NULL;
				}
				if (LogicInstanceName.size())
				{
					CElement* e = E->FindFocusLogic(LogicInstanceName);
					if (e)
					{
						m_CompileError = Format1024(_T("Error: 逻辑实例名[%s]已经被使用"),LogicInstanceName.c_str());
						return NULL;
					}
				}
				
				
				if (LogicName[0] == _T('<')) //全局
				{
					ePipeline* lg = CBrainMemory::FindGlobalLogic(LogicName);
					if(lg == NULL) {
						m_CompileError = Format1024(_T("Error: 没有找到全局逻辑[%s]"),LogicName.c_str());
						return NULL;
					}
					if(lg->Size() != 7) {
						m_CompileError = Format1024(_T("Error: 找到的全局逻辑[%s]无效"),LogicName.c_str());
						return NULL;
					}				
					
					int32 ActionState = *(int32*)lg->GetData(0);
					
					m_ActionType |= ActionState;
					
					ePipeline* LogicData = (ePipeline*)lg->GetData(5);
					
					CLogicTask* RootTask = GetRootTask();

					CLogicTask NestTask(RootTask);
					
					bool ret = NestTask.Compile(Dialog,LogicData);
					if (!ret)
					{
						m_CompileError = Format1024(_T("Error: 编译全局逻辑[%s]失败"),LogicName.c_str());
						return NULL;
					}
					
					assert(NestTask.m_ActomList.size()==1);
					CSeries* s = new CSeries(m_MassCount++ ,LogicName.c_str());
					s->m_ActomList.swap(NestTask.m_ActomList);
					m = s;
					
				}else{
					
					CLogicTask* LogicTask;
					CLocalLogicCell* lg = Dialog->FindLogic(LogicName);
					if(lg == NULL) {
						m_CompileError = Format1024(_T("Error: Not find Local logic[%s]"),LogicName.c_str());
						return NULL;
					}		
					else if(!lg->IsValid()) {
						m_CompileError = Format1024(_T("Error: Local logic[%s] not valid"),LogicName.c_str());
						return NULL;
					}else{
						LogicTask = &lg->m_Task;		
					}
					
					ePipeline LogicPipe(LogicTask->m_LogicData);
					
					m = CompileSentence(Dialog,Name,&LogicPipe);
					
				}	
			}
			else{
				m = GetInstinctInstance(Dialog,InstinctID, Param);
			}
		}else{ //客户自定义命令
			ePipeline Pipe;
			CLogicThread* Think = Dialog->GetThink();
			if(!Think->RetrieveLogic(InstinctID,&Pipe,false))
			{
				deque<tstring> CommandList;
				Think->GetCustomCommandText(InstinctID,CommandList);
				deque<tstring>::iterator It = CommandList.begin();
				m_CompileError = _T("can't compile custom commond : \n");
				while (It != CommandList.end())
				{
					tstring& Command = *It;
					Command+=_T('\n');
					m_CompileError += Command;
					It++;
				}				
				return NULL;
			}
            
			CLogicTask NestTask(this);
		    bool ret = NestTask.Compile(Dialog,&Pipe);
			if (ret)
			{
				assert(NestTask.m_ActomList.size()==1);
				m = NestTask.m_ActomList.back();
				NestTask.m_ActomList.clear();
			}
		}
		
		if(m==NULL){
			return NULL;
		}
		E->Push(m);		

		Dialog->SentenceCompileProgress(Sentence->Size(),i);
	}
	
	
	ptr.release(); //编译没发生错误，则释放控制权
	
	assert(ElementStack.size()!=0);
	CElement* ExecuteBody = ElementStack.back();
	ExecuteBody->m_Name = LogicText;
	

	return ExecuteBody;
}



Mass* CLogicTask::GetInstinctInstance(CLogicDialog* Dialog,int64 InstinctID, Energy* Param){

	CLogicTask* RootTask = GetRootTask();
		
	int64 ID = RootTask->m_MassCount++;

    Mass* M = NULL;

	switch(InstinctID){

	case INSTINCT_DEFINE_INT64:
		{
			assert(Param);
			int64& n= *(int64*)Param->Value();
			M = new CDefineInt(n,ID);
		}
		break;
	case INSTINCT_DEFINE_FLOAT64:
		{
			assert(Param);
			float64& n= *(float64*)Param->Value();
			M = new CDefineFloat(n,ID);
		}
		break;
	case INSTINCT_DEFINE_STRING:
		{
			assert(Param);
			tstring& n= *(tstring*)Param->Value();
			M = new CDefineString(n,ID);
		}
		break;
	case INSTINCT_USE_OPERATOR:
		{
			assert(Param);
			tstring Operator = (*(tstring*)Param->Value());
			if(Operator.size() ==1){
				wchar_t ch = Operator[0];
				switch(ch){
				case _T('+'):
					{
						M = new CAddition(ID);
					}
					break;
				case _T('-'):
					{
						M = new CSubtraction(ID);
					}
					break;
				case _T('*'):
					{
						M = new CMultiplication(ID);
					}
					break;
				case _T('/'):
					{
						M = new CDivision(ID);
					}
					break;
				case _T('%'):
					{
						M = new CModulus(ID);
					}
					break;
				case _T('>'):
					{
						M = new CGreaterThan(ID);
					}
					break;
					
				case _T('<'):
					{
						M = new CLessThan(ID);
					}
					break;
				case _T('!'):
					{
						M = new CLogical_NOT(ID);
					}
					break;
					
				case _T('&'):
					{
						M = new CBitwise_AND(ID);
					}
					break;
				case _T('~'):
					{
						M = new CBitwise_NOT(ID);
					}
					break;
				case _T('|'):
					{
						M = new CBitwise_OR(ID);
					}
					break;
				case _T('^'):
					{
						M = new CBitwise_XOR(ID);
					}
					break;
				default:
					m_CompileError = Format1024(_T("operator '%s' error"),Operator.c_str());
					return NULL;
				}
			}
			else if(Operator == _T(">="))
			{
				M = new CGreaterThanOrEqual(ID);
			}
			else if(Operator == _T("<="))
			{
				M = new CLessThanOrEqual(ID);
			}
			else if(Operator == _T("!="))
			{
				M = new CNotEquivalent(ID);
			}
			else if(Operator == _T("&&"))
			{
				M = new CLogical_AND(ID);
			}
			else if(Operator == _T("=="))
			{
				M = new CEquivalent(ID);
			}
			else if(Operator == _T("<<"))
			{
				M = new CLeft_Shift(ID);
			}
			else if(Operator == _T(">>"))
			{
				M = new CRight_Shift(ID);
			}
			else if(Operator == _T("||"))
			{
				M = new CLogical_OR(ID);
			}else{
				m_CompileError = Format1024(_T("operator '%s' error"),Operator.c_str());
			}
			
			return M;
		}
		break;
	case INSTINCT_USE_RESISTOR:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_INT);
			int64& n= *(int64*)Param->Value();
			M = new CResistor(n,ID);
		}
		break;
	case INSTINCT_USE_INDUCTOR:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& Name = *(tstring*)Param->Value();
			/*		
			if(m_TempElementList.find(Name) != m_TempElementList.end()){
			m_CompileError = tformat(_T("use inductor fail!  %s be exist"),Name.c_str()).c_str();
			return NULL;
			}
			*/
			tstring NewName(_T("(i) "));
			NewName += Name;
			M = new CInductor(RootTask,NewName.c_str(),ID);
			m_TempElementList[Name] = M;
		}
		break;
	case INSTINCT_USE_CAPACITOR:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& Name = *(tstring*)Param->Value();
			/* 不再检查，否则引用逻辑时会出现冲突
			if(m_TempElementList.find(Name) != m_TempElementList.end()){
			m_CompileError = tformat(_T("use capacitor fail!  %s be exist"),Name.c_str()).c_str();
			return NULL;
			}
			*/
			tstring NewName(_T("(c) "));
			NewName += Name;
			M = new CCapacitor(RootTask,NewName.c_str(),ID);
			m_TempElementList[Name] = M;  
		}
		break;
	case INSTINCT_USE_DIODE:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_INT);
			int64& n= *(int64*)Param->Value();
			M = new CDiode(n,ID);
		}
		break;
	case INSTINCT_REFERENCE_CAPACITOR:
	case INSTINCT_REFERENCE_INDUCTOR:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring* Name = (tstring*)Param->Value(); 
			
			tstring srcName  = *Name;
			tstring ::size_type n = Name->find(_T(":"));
			if (n  != string::npos)
			{
				srcName = Name->substr(0,n);
			}
			
			srcName = TriToken(srcName);
			
			Mass* RefMass = NULL;
			
			map<tstring,Mass*>::iterator It = m_TempElementList.find(srcName);
			if(It == m_TempElementList.end()){
				m_CompileError  = Format1024(_T("can't find element '%s' that will be reference "),srcName.c_str()).c_str();
				return NULL;
			}else{
				RefMass = It->second;
			}

			tstring NewName;
			
			if(InstinctID==INSTINCT_REFERENCE_INDUCTOR){
				NewName = _T("Ref(i) ");
				NewName += *Name;
			}else{
				NewName = _T("Ref(c) ");
				NewName += *Name;
			}
			M = new CMassRef(RefMass,NewName.c_str(),ID);
		}
		break;

	case INSTINCT_GOTO_LABEL:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& Name = *(tstring*)Param->Value();
			
			tstring NewName = Format1024(_T("Return to [%s]"),Name.c_str());
			M = new CGotoLabel(NewName.c_str(),ID,Name);
		}
		break;
	case INSTINCT_VIEW_PIPE:
		{
			assert(Param == NULL);
			m_CompileError  = Format1024(_T("WARNING: Can't use view pipe command in intertask,will be ignore.")).c_str();
			M = new CPipeViewMass(ID);	
		}
		break;
    case INSTINCT_INPUT_TEXT:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& Tip = *(tstring*)Param->Value();
			
			M = new CInputElement(ID,Tip,false);
		}
		break;
    case INSTINCT_INPUT_NUM:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& Tip = *(tstring*)Param->Value();
			
			M = new CInputElement(ID,Tip,true);
		}
		break;
    case INSTINCT_WAIT_SECOND:
		{
			if(Param){
				assert(Param->EnergyType() == TYPE_FLOAT);
				float64 Second = *(float64*)Param->Value();
				M = new CWaitSecond_Static(ID,Second);
			}else{
				M = new CWaitSecond(ID);
			}
		}
        break;
	case INSTINCT_CREATE_MEMORY:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& Name = *(tstring*)Param->Value();
			
			M = new CCreateMemory(ID,Name);
			
		}
		break;
	case INSTINCT_FOCUS_MEMORY:
		{
			if (Param)
			{
				assert(Param->EnergyType() == TYPE_STRING);
				tstring& Name = *(tstring*)Param->Value();
				M = new CFocusMemoryStatic(ID,Name);
			} 
			else
			{
				M = new CFocusMemory(ID);
			}
		}
		break;
	case INSTINCT_SET_MEMORY_ADDRESS:
		{
			M = new CSetMemoryAddress(ID);
		}
		break;
	case INSTINCT_GET_MEMORY_ADDRESS:
		{
			M = new CGetMemoryAddress(ID);
		}
		break;
	case INSTINCT_CREATE_MEMORY_NODE:
		{
			M = new CCreateMemoryNode(ID);
		}
		break;
	case INSTINCT_IMPORT_MEMORY:
		{			
			M = new CImportMemory(ID);
		}
		break;
	case INSTINCT_EXPORT_MEMORY:
		{
			M = new CExportMemory(ID);
		}
		break;
	case INSTINCT_GET_MEMORY:
		{
			M = new CGetMemory(ID);
		}
		break;
	case INSTINCT_INSERT_MEMORY:
		{
			M = new CInsertMemory(ID);		
		}
		break;
	case INSTINCT_MODIFY_MEMORY:
		{
			M = new CModifyMemory(ID);
		}
		break;
	case INSTINCT_REMOVE_MEMORY:
		{
			M = new CRemoveMemory(ID);
			
		}
		break;
	case INSTINCT_GET_MEMORY_SIZE:
		{
			M = new CGetMemorySize(ID);	
		}
		break;
	case INSTINCT_CLOSE_MEMORY:
		{
			M = new CCloseMemory(ID);		
		}
		break;
	case INSTINCT_GET_MEMORY_FOCUS:
		{
			M = new CGetMemoryFocus(ID);
		}
		break;
	case INSTINCT_FOCUS_LOGIC:
		{
			if (Param)
			{
				assert(Param->EnergyType() == TYPE_STRING);
				tstring& Name = *(tstring*)Param->Value();
				M = new CFocusLogic_Static(ID,Name);
			} 
			else
			{
				M = new CFocusLogic(ID);
			}
		}
		break;

	case INSTINCT_INSERT_LOGIC:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			tstring& InsertLogicName = *(tstring*)Param->Value();
			
			
			CLocalLogicCell* lg = Dialog->FindLogic(InsertLogicName);
			if(lg == NULL) {					
				m_CompileError = Format1024(_T("Error: Not find Local logic[%s]"),InsertLogicName.c_str());
				return NULL;
			}		
			else if(!lg->IsValid()) {
				m_CompileError = Format1024(_T("Error: Local logic[%s] not valid"),InsertLogicName.c_str());
				return NULL;
			}
			
			M = new CInsertLogicStatic(ID,InsertLogicName);
			
		}
		break;
	case INSTINCT_REMOVE_LOGIC:
		{
			M = new CRemoveLogicStatic(ID);	
		}
		break;
	case INSTINCT_SET_LOGIC_ADDRESS:
		{
			M = new CSetLogicAddress(ID);
		}
	case INSTINCT_SET_LOGIC_BREAKPOINT:
		{
			M = new CSetLogicBreakpoint(ID);
		}
	case INSTINCT_GET_DATE:
		{
			M = new CGetDate(ID);
		}
		break;
	case INSTINCT_GET_TIME:
		{
			M = new CGetTime(ID);
		}
		break;
	case INSTINCT_OUTPUT_INFO:
		{
			M = new COutputInfo(ID);
		}
		break;
	case INSTINCT_TEST_EXPECTATION:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_INT);
			int64& bExpectation = *(int64*)Param->Value();


			M = new CTestExpectation(ID,bExpectation>0);
		}
		break;
	case INSTINCT_START_OBJECT:
		{
			assert(Param);
			assert(Param->EnergyType() == TYPE_STRING);
			
			tstring Name = *(tstring*)Param->Value();

			//优先在临时物体列表里找
			vector<CObjectData> ObjectList;
			int n = Dialog->FindObject(Name,ObjectList);

			ePipeline* ObjectInfo = NULL;

			if( n == 1 ){
				CObjectData& Ob = ObjectList.front();
				
				int64 InstanceID = AbstractSpace::CreateTimeStamp();
				Ob.m_ID = InstanceID;
				
				ObjectInfo = Ob.GetItemData();	
				Dialog->m_ObjectFocus = InstinctID;

				M = new CStartObject(ID, ObjectInfo);

			}
			else if( n > 0 )
			{
				//原本应该让用户选择，这里暂时简化处理
				m_CompileError = Format1024(_T("object(%s) not unique"),Name.c_str());
				return false;
			}

			if(ObjectInfo == NULL){
				m_CompileError = Format1024(_T("not find object(%s)"),Name.c_str());
				return false;
			}	
		}
		break;
	case INSTINCT_FOCUS_OBJECT:
		{
			if (Param)
			{
				if(Param->EnergyType() == TYPE_STRING)
				{
					tstring& Name = *(tstring*)Param->Value();
					M = new CFocusObject_Static(ID,Name);
				}else if(Param->EnergyType() == TYPE_INT)
				{
					int64 InstanceID = *(int64*)Param->Value();
					M = new CFocusObject_Inter(ID,InstanceID);
				}
			} 
			else
			{
				M = new CFocusObject(ID);
			}
		}
		break;
	case INSTINCT_NAME_OBJECT:
		{
			if (Param)
			{
				if(Param->EnergyType() == TYPE_STRING)
				{
					tstring& Name = *(tstring*)Param->Value();
					M = new CNameObject_Static(ID,Name);
				}
			}else{
				M = new CNameObject(ID);
			} 
		}
		break;
	case INSTINCT_USE_OBJECT:
		{
			M = new CUseObject(ID);
			
		}
		break;
	case INSTINCT_CLOSE_OBJECT:
		{
			
			M = new CCloseObject(ID);
			
		}
		break;
	case INSTINCT_GET_OBJECT_DOC:
		{
			M = new CGetObjectDoc(ID);
		}
		break;
	case INSTINCT_ASK_PEOPLE:
		{

		}
		break;
	case INSTINCT_BRAIN_INIT:
		{
			M = new CBrainInitElt(ID);
		}
		break;
	case INSTINCT_GET_SPACECATALOG:
		{

			M = new CGetSpaceCatalog(ID);
		}
/*
	case INSTINCT_USE_ARM:
		{
			M = new CUseArm(ID,CBrain::m_LeftArm);
		}
		break;
*/
	default:
		if (BelongInterAction(InstinctID))
		{
			ePipeline Clause;
			if(Param)Clause.Push_Directly(Param->Clone());
			M = new CInterBrainObject(ID,_T("InterBrainObject"),InstinctID,Clause);

			if (InstinctID == INSTINCT_THINK_LOGIC)
			{
				tstring Name = *(tstring*)Param->Value();
				
				tstring LogicName;
				tstring LogicInstanceName;
				tstring LogicComment;
				GetLogicName(Name,LogicName,LogicInstanceName,LogicComment);

				m_CurThinkLogicName = LogicName;
			}
		}else{
			assert(0);
		}

	}
	
	return M;
	
};