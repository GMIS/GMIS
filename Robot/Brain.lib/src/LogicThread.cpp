// LogicThread.cpp: implementation of the CLogicThread class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "LogicThread.h"
#include "VisibleSpace.h"
#include <iostream>
#include <string>
#include "NotifyMsgDef.h"
#include "LogicDialog.h"
#include "Brain.h"

NumType IsNum(tstring& Text)
{
	int32 len=Text.size();
	int32 i = 0,end = len-1;
	while(i<len && _istspace(Text[i]))i++;
    while(end>-1 && _istspace(Text[end]))end--;
	
	if(end-i<0)return NOT_NUM;
	if(!_istdigit(Text[i])){
		if ( Text[i] == '-' )
		{
			i++;
		}else if (Text[i] == '+' )
		{
			i++;
		}else{
			return NOT_NUM;
		}
	};
	bool floatflag = false;  // 控制一个数里只允许有一个小数点
	end++;
	for(i; i<end; ++i)
	{	
		// 需要一次性处理完一个数,每个数用引号引起来，避免TOKEN重复处理。
		if(_istdigit(Text[i]))continue;
		else if(!floatflag && Text[i] =='.' && (i+1)<end && _istdigit(Text[i+1]))
		{
			floatflag = true;
		}
		else return	NOT_NUM;
	}
	if(floatflag)return FLOAT_NUM;
	return INT_NUM;
};



CLocalLogicCell::CLocalLogicCell(CLogicTask& Task)
{
    m_Task<<Task;
};
CLocalLogicCell::~CLocalLogicCell(){

}


ePipeline* CLocalLogicCell::GetLogicItem(){ 

	ePipeline* Item = new ePipeline;
	Item->PushString(m_Task.m_Name);
	Item->PushInt(m_Task.m_bDepend);
	Item->PushInt(m_Task.m_State);
	Item->PushString(m_Task.m_LogicText);
	Item->PushString(m_LogicMemo);
	return Item;
}

void CLocalLogicCell::GetRefList(ePipeline& List){
	map<tstring,tstring>::iterator it =  m_RefList.begin();
	while(it != m_RefList.end()){
		ePipeline* Item = new ePipeline;
		Item->PushString(m_Task.m_Name);
        Item->PushString(it->first);
		Item->PushString(it->second);
		List.Push_Directly(Item);
		it++;
	}
}

Energy*  CLocalLogicCell::ToEnergy(){
	ePipeline* Pipe = new ePipeline;
    if(!Pipe)return NULL;
	
	Energy* e = m_Task.ToEnergy();
	if (!e)return NULL;
	
	Pipe->Push_Directly(e);
	Pipe->PushString(m_LogicMemo);
	
	map<tstring,tstring>::iterator it =  m_RefList.begin();
	while(it != m_RefList.end()){
		Pipe->PushString(it->first);
		Pipe->PushString(it->second);
		it++;
	}
	return Pipe;
} 

bool     CLocalLogicCell::FromEnergy(Energy* E){
	ePipeline* Pipe  = (ePipeline*)E;
	eElectron e;   
	Pipe->Pop(&e);
	ePipeline* TaskData = (ePipeline*)e.Value();
	bool ret = m_Task.FromEnergy(NULL,TaskData);
    if (!ret)return false;
 
	m_LogicMemo = Pipe->PopString();

	while (Pipe->Size())
	{
		tstring s1 = Pipe->PopString();
		tstring s2 = Pipe->PopString();
	    m_RefList[s1] = s2;
	}
	return true;
}

CElementCell::CElementCell(tstring Name, tstring CreateBy)
:m_Name(Name),m_CreateBy(CreateBy)
{

}

ePipeline* CElementCell::GetElementItem(){
	ePipeline* Item = new ePipeline;
	Item->PushString(m_Name);
	Item->PushString(m_CreateBy);
	Item->PushInt(m_RefList.size());
	return Item;
}


void CElementCell::GetRefList(ePipeline& List){
	map<tstring,tstring>::iterator it =  m_RefList.begin();
	while(it != m_RefList.end()){
		ePipeline* Item = new ePipeline;
		Item->PushString(m_Name);
        Item->PushString(it->first);
		Item->PushString(it->second);
		List.Push_Directly(Item);
		it++;
	}
}


Energy*  CElementCell::ToEnergy(){
	ePipeline* Pipe = new ePipeline;
    if(Pipe){
		Pipe->PushString(m_Name);
		Pipe->PushString(m_CreateBy);
		
		map<tstring,tstring>::iterator it =  m_RefList.begin();
		while(it != m_RefList.end()){
            Pipe->PushString(it->first);
			Pipe->PushString(it->second);
			it++;
		}
		return Pipe;
	}
	return NULL;
} 
bool     CElementCell::FromEnergy(Energy* E){
	ePipeline* Pipe  = (ePipeline*)E;
	m_Name = Pipe->PopString();
	m_CreateBy = Pipe->PopString();
	
	while (Pipe->Size())
	{
		tstring s1 = Pipe->PopString();
		tstring s2 = Pipe->PopString();
		m_RefList[s1] = s2;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogicThread::CLogicThread(){

	m_ForecastSpace = NULL;

	Reset(0,0);
}
CLogicThread::CLogicThread(int64 ID,int64 UserDialogID)
{
	m_ForecastSpace = NULL;
	Reset(ID,UserDialogID);
};

void CLogicThread::Reset(int64 ThreadID,int64 UserDialogID){
	m_ID = ThreadID;
	m_InErrorState = false;	
	m_Text.m_ParentThread = this;

	m_UserDialogID = UserDialogID;

	m_Text.Clear();

	if(m_ForecastSpace){
		delete m_ForecastSpace;
		m_ForecastSpace = NULL;
	}
	m_ForecastSpace = new ForecastSpace();
	//BrainDB.close();

	m_LastMsgTimeStamp = 0;
	m_LogicPos = -1;

    if (ThreadID == 0)
    {
		return; //只是简单的释放掉内存就返回
    }

	Open();
	
};

CLogicThread::~CLogicThread()
{
	if(m_ForecastSpace){
		delete m_ForecastSpace;
		m_ForecastSpace = NULL;
	}

	ClearAnalyse();
}


Energy*  CLogicThread::ToEnergy(){
	ePipeline* Pipe = new ePipeline;
	eElectron e(Pipe);
	if (Pipe)
	{
		Pipe->PushInt(m_ID);
		Pipe->PushInt(m_UserDialogID);
		return e.Release();
	}
	return NULL;
};

bool     CLogicThread::FromEnergy(Energy* e){
	ePipeline* Pipe = (ePipeline*)e;

    m_ID = Pipe->PopInt();
	m_UserDialogID = Pipe->PopInt();
	return true;
};

ePipeline* CLogicThread::GetOneLogic(){

	int32 n = m_Text.m_SentenceList.size();
    while (++m_LogicPos< n)
    {
		CSentence* Sentence = m_Text.m_SentenceList[m_LogicPos];

		if(Sentence->m_AnalyseResult.Size() ==0){
			continue;
		}
		return &Sentence->m_AnalyseResult;	
    }
	m_LogicPos = -1;
    return NULL;
};

//[idle/run/think/debug]task name	

int64 CLogicThread::SelectMeaning(ClauseLogicSense* cls)
{        
	return 0;
}

	
void CLogicThread::ClearAnalyse()
{
    m_Text.Clear();
	m_ForecastType = NULL_TEXT;
	m_ForecastText = NULL;
}

	
bool CLogicThread::CanBeExecute(CLogicDialog* Dialog){

	tstring error;

	if(m_Text.m_SentenceList.size()==0 || m_TextMsgQueue.size() || !m_Text.IsComplete() ){
		Dialog->m_ThinkError = m_Text.PrintThinkResult();
		return false;
	}
	/*检查以下几个方面
	 - think logic是否与其要生成的逻辑成对出现
     - 引用的逻辑原件名是否存在,对于引用逻辑此时要获得其真实含义，放防止自我引用
	 - 只允许最后一句可执行任务
	 - 检查每个句子的行为属性是否冲突。
    */

    int64  ThinkID = 0;// 表示不在Think命令状态

	map<tstring, CSentence*>  RefList;
	tstring  ThinkName;
	int n = m_Text.m_SentenceList.size();
	for(int i=0;i<n; i++)
	{
		CSentence* Sentence = m_Text.m_SentenceList.at(i);
		if(!Sentence->IsOK()){
			Dialog->m_ThinkError = m_Text.PrintThinkResult();
			return false;
		}
		
		if(Sentence->m_State & INDE_INTER_ACTION){
			
			if(n != 1 && Sentence->m_AnalyseResult.Size() != 1){
				Dialog->m_ThinkError = Format1024(_T("ERROR: Inner command must be used alone."));
				return false;
			}
		}

        if (Sentence->m_ClauseNum==1)
        {
			CClause* Clause = m_Text.m_ClauseList[Sentence->m_BeginPos];
			if (Clause->m_MemoryID == INSTINCT_THINK_LOGIC )
			{
				if(ThinkID == 0){
					ThinkID = Clause->m_MemoryID;
					ThinkName = *(tstring*)Clause->m_Param->Value();
					Sentence->m_State = INDE_INTER_ACTION;
					continue;
				}
				else{
					//不能连续两句都是think
					Dialog->m_ThinkError = _T("Error: Action type Conflict !");
					//error = m_Text.PrintThinkResult();
					Dialog->SetThinkState(THINK_ERROR);
					return false;
				} 
			}
			
        }

		if(ThinkID==0){//如果当前句子不在思考状态，则必须是最后一句
			if(i != m_Text.m_SentenceList.size()-1){
				Dialog->m_ThinkError = _T("Error: Task sentence more than one or not be in last !");
				return  false;
			}
		}else{
			RefList[ThinkName]=Sentence;
			ThinkID = 0;
		}

		//检查参数
		int32   EndPos  = Sentence->m_BeginPos+Sentence->m_ClauseNum;
		for(int32 i = Sentence->m_BeginPos; i<EndPos; i++)
		{
			CClause* Clause = m_Text.m_ClauseList[i];
			
			int64 InstinctID = Clause->m_MemoryID<0?-Clause->m_MemoryID:Clause->m_MemoryID;

			switch(InstinctID)
			{
			case INSTINCT_USE_CAPACITOR:
				{
					tstring Name = *(tstring*)Clause->m_Param->Value();	
					if (CheckNameInInputed(INSTINCT_USE_CAPACITOR,Name,Clause,false))
					{
						Dialog->m_ThinkError = Format1024(_T("Error: capacitor name [%s] be existed!\n"),Name.c_str());
						return false;
					}
				
				}
				break;
			case INSTINCT_USE_INDUCTOR:
				{
					tstring Name = *(tstring*)Clause->m_Param->Value();	
					if (CheckNameInInputed(INSTINCT_USE_INDUCTOR,Name,Clause,false))
					{
						Dialog->m_ThinkError = Format1024(_T("Error: Inductor name [%s] be existed!\n"),Name.c_str());
						return false;
					}
				}
				break;
			case INSTINCT_REFERENCE_CAPACITOR:
				{
					tstring Name = *(tstring*)Clause->m_Param->Value();	
					
					if(!Dialog->FindCapacitor(Name) && !CheckNameInInputed(INSTINCT_USE_CAPACITOR,Name,Clause,true)){
						//没有找到引用对象
						Dialog->m_ThinkError = Format1024(_T("Error: lose reference capacitor [%s] !\n"),Name.c_str());
						return false;		    
					}
				} 
				break;
			case INSTINCT_REFERENCE_INDUCTOR:
				{
					tstring Name = *(tstring*)Clause->m_Param->Value();	
					
					if(!Dialog->FindInductor(Name) && !CheckNameInInputed(INSTINCT_USE_INDUCTOR,Name,Clause,true)){
						//没有找到引用对象
						Dialog->m_ThinkError = Format1024(_T("Error: lose reference inductor [%s] !\n"),Name.c_str());
						return false;		    
					}	
				}
				break;
			default:
				break;
			}

		};
		
	}

	//如果可以执行，则重置此值，以便开始下一个信息理解序列。
	m_LastMsgTimeStamp = 0; 
	return true;
}

tstring CLogicThread::GetUserInput(){
	tstring s;
	for (int i=0; i<m_Text.m_SentenceList.size(); i++)
	{
		s  += m_Text.GetSentenceText(i);
	}
	return s;
};




/*
void CLogicThread::TestAnalyse(CProgressThreadDlg* Control){
	int n=m_TestText.size();
	m_AnlayseCount=1;
    while(n>1)m_AnlayseCount *= n--;

//	Analyse(0,m_TestText);	
//	Think();
//	if(!CanExecute())return;

	m_CompletedCount = 0;
	vector<_InputInfo>  SelectResult;
	AnlyseRecursive(m_TestText,SelectResult,0,Control);
}

void CLogicThread::AnlyseRecursive(tstring SelectFlag, vector<_InputInfo>& SelectResult,int32 NestNum,CProgressThreadDlg* Control){
			
	if (Control->HasBeenCancelled())return ;
			
    if(NestNum == m_TestText.size()){
		m_CompletedCount++;
		if(m_CompletedCount<12)return;

		tstring log;
		try{
			for(int32 i=0; i<SelectResult.size(); i++){
				_InputInfo& Info = SelectResult[i];
				log += format("%d%c ",Info.pos,Info.ch);		  
				m_Text.Analyse(Info.pos,Info.ch);
				ThinkAllAffectedToken();
				ThinkAllAffectedClause();
				ThinkAllAffectedSentence();       	
			}
		}catch (...) {
			cout<<"Error["<<m_CompletedCount<<"] "<<log<<endl;	
			Control->OnCancel();
		}
         //比较分析答案 ,出错则输出输入方式   
		if(!CanExecute()){
	        cout<<"Error["<<m_CompletedCount<<"] "<<log<<endl;
			Control->OnCancel();
		}
		if (Control)
		{
			//othewise update the current position
			long nPercentage =m_CompletedCount*100 / m_AnlayseCount;
			CString WaitText;
			WaitText.Format("%d/%d",m_CompletedCount, m_AnlayseCount);
			Control->SetWaitText(WaitText);
			Control->SetPercentageDone(nPercentage);
		}

		ClearAnalyse();
        return;
    } 
    tstring s;
    //任意取一个还没被选择的字符
    for(int i=0; i<m_TestText.size();i++){
        if(SelectFlag[i] != '$') {
			char ch = m_TestText[i];
			SelectFlag[i] = '$'; //表示已经取了
			int32 pos = 0; //计算插入位置(找之前已经有多少字符被选）
			for(int j = 0; j< i; j++){
				if(SelectFlag[j]=='$')pos++;
			}
			
			_InputInfo info(pos,ch);
			SelectResult.push_back(info);
			
			AnlyseRecursive(SelectFlag,SelectResult,NestNum+1,Control);                            
			SelectFlag[i]=ch; //恢复为为取状态 
			SelectResult.pop_back();
		}    
		if (Control->HasBeenCancelled())return ;
    } 
}

void CLogicThread::TestDel(CProgressThreadDlg* Control){
	int32 n=m_TestText.size();
	int Count = 0;
	CString WaitText;
	
	for(int i=0; i<n; i++){
		int32 SelectNum = i+1;
		int32 t=n-i;
		for(int32 j=0; j<t; j++){
			Count = i*100+j;
			//			if(Count<3)continue;
			WaitText.Format("%d",Count);
			Control->SetWaitText(WaitText);
			try{				
				TaskThink(0,m_TestText,false); //先正确分析
				assert(CanExecute());
				
				int32 k;
				for(k=0; k<SelectNum;k++){
					//	m_Text.AddChar(j,VK_DELETE);
					m_Text.Analyse(j,VK_DELETE);
					ThinkAllAffectedToken();
					ThinkAllAffectedClause();
					ThinkAllAffectedSentence();   
				}
				for(k=0; k<SelectNum;k++){
					//m_Text.AddChar(j+k,m_TestText[j+k]);
					m_Text.Analyse(j+k,m_TestText[j+k]);
					ThinkAllAffectedToken();
					ThinkAllAffectedClause();
					ThinkAllAffectedSentence();   
				}
				if(!CanExecute()){				
					tstring log = format("Del Pos: %d n:%d",j,SelectNum);
					cout<<"Error["<<Count<<"] "<<log<<endl;	
					Control->OnCancel();
				}
			}catch (...) {
				tstring log = format("Del Pos: %d n:%d",j,SelectNum);
				cout<<"Error["<<Count<<"] "<<log<<endl;	
				Control->OnCancel();
			}				

			ClearAnalyse();
			if (Control->HasBeenCancelled())return ;
		}
		
	}
	
	cout<<"TestDel() OK"<<endl;	
}

ULONG ExecuteAnlayseTest(void* pData, CProgressThreadDlg* pProgressDlg){
	CLogicThread* Thread= (CLogicThread*)pData;
	Sleep(20);
	Thread->m_TestText = "e \"3\";g;"; 
	Thread->m_Text.m_IsDebug = false;
	pProgressDlg->SetWindowText(Thread->m_TestText.c_str());
	//Thread->TestAnalyse(pProgressDlg);
    Thread->TestDel(pProgressDlg);
	return 0;
}
*/