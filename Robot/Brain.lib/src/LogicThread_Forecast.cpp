// LogicThread.cpp: implementation of the CLogicThread class.
// 预测部分
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "Brain.h"
#include "LogicThread.h"
#include "InstinctDefine.h"
#include "NotifyMsgDef.h"
#include "LogicDialog.h"

int64 CLogicThread::GetForecastResult(int64 SpaceValue){ //根据空间值得到空间识别ID
	assert(m_ForecastSpace != NULL);

	int64 SpaceID = 0;
	map<int64,ForecastSpace*>::iterator It = m_ForecastSpace->ChildList.find(SpaceValue);
	if(It != m_ForecastSpace->ChildList.end()){
	    ForecastSpace* Space = It->second;
		assert(Space != NULL);
		SpaceID = Space->ID;
		//替换一个无用的ROOM，避免解构时无物可Delete
		ForecastSpace* Empty = new ForecastSpace();
		It->second = Empty;
        
		//合并文本
		Space->SpaceText = m_ForecastSpace->SpaceText + Space->SpaceText;
        Space->Parent   = NULL;
		//删除旧的预测空间
		delete m_ForecastSpace;		
		
		m_ForecastSpace = Space; //新的预测空间为当前选中的子空间,以便能继续预测
	}
	return SpaceID;	
}
	
void  CLogicThread::GetAllChild(ForecastSpace* ParentSpace)
{
	   assert(ParentSpace != NULL);

	   CppSQLite3Buffer SQL;
	   CppSQLite3Query  Result;
       char a[30];
      
	   ParentSpace->ChildList.clear();
	   ParentSpace->MeanlingList.clear();

       if(!RBrainHasTable(ParentSpace->ID))return;
	   
	   ToRBrain(ParentSpace->ID);
	   int64toa(ParentSpace->ID,a);
	   SQL.format("select *  from \"%s\" ;",a);
	   Result = BrainDB.execQuery(SQL);
       
	   while(!Result.eof())
	   {
		   int64 SpaceID = Result.getInt64Field(0);
		   int64 SpaceValue = Result.getInt64Field(1);
           int64 Type = Result.getInt64Field(2);

		   if(IsMeaningSpace(Type)){
			   ParentSpace->MeanlingList.push_back(SpaceValue);
		   }
		   else if(Type == MEMORY_BODY){
			   ForecastSpace* ChildSpace = new ForecastSpace(ParentSpace,SpaceID);
			   ParentSpace->ChildList[SpaceValue] = ChildSpace; 	
		   }
		   Result.nextRow();
	   }
}

//深度优先，如果穷尽，则返回Parent尝试下一个
CLogicThread::ForecastSpace* CLogicThread::GetCurForecastSpace(ForecastSpace* Parent){
    if(Parent->MeanlingList.size())return Parent; 
	if(Parent->ChildList.size()){
		if(Parent->It != Parent->ChildList.end())
			return GetCurForecastSpace(Parent->It->second);
		else {
			Parent = Parent->Parent;
			if(Parent){
				Parent->It++;
				return GetCurForecastSpace(Parent);
			}
			else return NULL;
		}
	}     
	return Parent;
}

/*
void CLogicThread::ExecuteForecast(){

	if(m_CurrentTask)//对于嵌套执行，为了避免破坏当前输出的完整性也不再输出预测
         return;

	//此时处理有两种情况：
	// 1 准备预测 m_ForecastSpace == NULL;
	// 2 继续预测 m_ForecastSpace != NULL;
	switch(m_ForecastType){
	case TOKEN:
		Forecast((CToken*)m_ForecastText);
		break;
	case CLAUSE:
		Forecast((CClause*)m_ForecastText);
		break;
	case SENTENCE:
		Forecast((CSentence*)m_ForecastText);
	default: //==NULL
		return;
	}
}
*/
void  CLogicThread::SetForecast(CLogicDialog* Dialog,TextType Type,void* Text){ 
	
	if(m_ForecastText != Text || m_ForecastType != Type){ 
		if(m_ForecastSpace){
			delete m_ForecastSpace;
		    m_ForecastSpace = NULL;
		}
	}
	m_ForecastType = Type;
	m_ForecastText = Text;


	if(Type==NULL_TEXT ){
	    CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
		nf.PushInt(FORECAST_INFO);
		nf.PushString(_T(""));
		nf.Notify(Dialog);
	}
};

void CLogicThread::Forecast(CLogicDialog* Dialog,CToken* Token){

	if(m_ForecastSpace == NULL){
		if(Token->isPunct()){ 
            return;  
		}
		else if(Token->m_MemoryID == ERROR_2){
			//出错就不用再预测了,因为如果之前输入正确已经预测过
			return;
		}
		else{ //token正确			assert(Token->m_Type == COMMON);
			m_ForecastSpace = new ForecastSpace;
			m_ForecastSpace->ID = Token->m_MemoryID;
			m_ForecastSpace->SpaceText = Token->m_Str;
		}

		CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
		nf.PushInt(FORECAST_INFO);
		nf.PushString(_T(""));
		nf.Notify(Dialog);
		return;
	}
	
	//深度预测所有可能
	//找到当前正在处理那个嵌套ForecastSpace
	ForecastSpace* CurSpace = GetCurForecastSpace(m_ForecastSpace);
	if(CurSpace == NULL)return;


	//得到本空间文本，以便随后childi能组装意义的文本
	if(CurSpace->SpaceText.size() ==0){
	    assert(CurSpace != m_ForecastSpace);
		//得到本空间文本,对于Token总是字符
		int64 SpaceID = CurSpace->Parent->It->first; //=此空间的Meaning
	    int64 ch = IDToChar(SpaceID);
		assert(ch<255);
		CurSpace->SpaceText = (TCHAR)ch;
		return;		
	}
	
	//把已经得到的意义空间全部回取成文本
	if(CurSpace->MeanlingList.size()){
		//取出一个，返回其文本
		//int64 MeaningID = Parent->MeanlingList.back();
        CurSpace->MeanlingList.pop_back();

		tstring Word;				
		while(CurSpace){ //向父空间漫游回取完整的文本
			Word = CurSpace->SpaceText+Word;
			CurSpace = CurSpace->Parent;
		}
		if(Word.size()){
			CNotifyDialogState nf(NOTIFY_DIALOG_OUTPUT);
			nf.PushInt(FORECAST_INFO);
			nf.PushString(Word);
			nf.Notify(Dialog);
		}
		return;
	}
	
 
    //继续往深处挖掘 			
	assert(CurSpace->ChildList.size() == 0 && CurSpace->MeanlingList.size()==0);
	
	//得到Token->m_MemoryID的所有子空间  
	GetAllChild(CurSpace);
	if(CurSpace->ChildList.size()){
		CurSpace->It = CurSpace->ChildList.begin();
	}
	else {//表示当前空间没有子空间，虚置一个表示结尾
		ForecastSpace* Empty = new ForecastSpace();
		CurSpace->ChildList[0] = Empty;
		CurSpace->It = CurSpace->ChildList.end(); //表示已经处理过			
	}
	return;
};

void CLogicThread::Forecast(CClause* Clause){

};

void CLogicThread::Forecast(CSentence* Sentence){

	
};

