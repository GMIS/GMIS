// LogicThread.cpp: implementation of the CLogicThread class.
// 预测部分
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "LogicThread.h"
#include "InstinctDefine.h"
#include "NotifyMsgDef.h"
#include "TaskDialog.h"
#include "TaskDialog.h"

int64 CLogicThread::GetForecastResult(int64 RoomValue){ //根据空间值得到空间识别ID
	assert(m_ForecastRoom != NULL);

	int64 RoomID = 0;
	map<int64,ForecastRoom*>::iterator It = m_ForecastRoom->ChildList.find(RoomValue);
	if(It != m_ForecastRoom->ChildList.end()){
	    ForecastRoom* Room = It->second;
		assert(Room != NULL);
		RoomID = Room->ID;
		//替换一个无用的ROOM，避免解构时无物可Delete
		ForecastRoom* Empty = new ForecastRoom();
		It->second = Empty;
        
		//合并文本
		Room->RoomText = m_ForecastRoom->RoomText + Room->RoomText;
        Room->Parent   = NULL;
		//删除旧的预测空间
		delete m_ForecastRoom;		
		
		m_ForecastRoom = Room; //新的预测空间为当前选中的子空间,以便能继续预测
	}
	return RoomID;	
}
	
void  CLogicThread::GetAllChild(ForecastRoom* ParentRoom)
{
	   assert(ParentRoom != NULL);

	   CppSQLite3Buffer SQL;
	   CppSQLite3Query  Result;
       char a[30];
      
	   ParentRoom->ChildList.clear();
	   ParentRoom->MeanlingList.clear();

       if(!RBrainHasTable(ParentRoom->ID))return;
	   
	   ToRBrain(ParentRoom->ID);
	   int64toa(ParentRoom->ID,a);
	   SQL.format("select *  from \"%s\" ;",a);
	   Result = BrainDB.execQuery(SQL);
       
	   while(!Result.eof())
	   {
		   int64 RoomID = Result.getInt64Field(0);
		   int64 RoomValue = Result.getInt64Field(1);
           int64 Type = Result.getInt64Field(2);

		   if(IsMeaningRoom(Type)){
			   ParentRoom->MeanlingList.push_back(RoomValue);
		   }
		   else if(Type == MEMORY_BODY){
			   ForecastRoom* ChildRoom = new ForecastRoom(ParentRoom,RoomID);
			   ParentRoom->ChildList[RoomValue] = ChildRoom; 	
		   }
		   Result.nextRow();
	   }
}

//深度优先，如果穷尽，则返回Parent尝试下一个
CLogicThread::ForecastRoom* CLogicThread::GetCurForecastRoom(ForecastRoom* Parent){
    if(Parent->MeanlingList.size())return Parent; 
	if(Parent->ChildList.size()){
		if(Parent->It != Parent->ChildList.end())
			return GetCurForecastRoom(Parent->It->second);
		else {
			Parent = Parent->Parent;
			if(Parent){
				Parent->It++;
				return GetCurForecastRoom(Parent);
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
	// 1 准备预测 m_ForecastRoom == NULL;
	// 2 继续预测 m_ForecastRoom != NULL;
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
void  CLogicThread::SetForecast(CTaskDialog* Dialog,TextType Type,void* Text){ 
	
	if(m_ForecastText != Text || m_ForecastType != Type){ 
		if(m_ForecastRoom){
			delete m_ForecastRoom;
		    m_ForecastRoom = NULL;
		}
	}
	m_ForecastType = Type;
	m_ForecastText = Text;


	if(Type==NULL_TEXT ){
	    CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
		nf.PushInt(FORECAST_INFO);
		nf.PushString(_T(""));
		nf.Notify(Dialog);
	}
};

void CLogicThread::Forecast(CTaskDialog* Dialog,CToken* Token){

	if(m_ForecastRoom == NULL){
		if(Token->isPunct()){ 
            return;  
		}
		else if(Token->m_MemoryID == ERROR_2){
			//出错就不用再预测了,因为如果之前输入正确已经预测过
			return;
		}
		else{ //token正确			assert(Token->m_Type == COMMON);
			m_ForecastRoom = new ForecastRoom;
			m_ForecastRoom->ID = Token->m_MemoryID;
			m_ForecastRoom->RoomText = Token->m_Str;
		}

		CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
		nf.PushInt(FORECAST_INFO);
		nf.PushString(_T(""));
		nf.Notify(Dialog);
		return;
	}
	
	//深度预测所有可能
	//找到当前正在处理那个嵌套ForecastRoom
	ForecastRoom* CurRoom = GetCurForecastRoom(m_ForecastRoom);
	if(CurRoom == NULL)return;


	//得到本空间文本，以便随后childi能组装意义的文本
	if(CurRoom->RoomText.size() ==0){
	    assert(CurRoom != m_ForecastRoom);
		//得到本空间文本,对于Token总是字符
		int64 RoomID = CurRoom->Parent->It->first; //=此空间的Meaning
	    int64 ch = IDToChar(RoomID);
		assert(ch<255);
		CurRoom->RoomText = (TCHAR)ch;
		return;		
	}
	
	//把已经得到的意义空间全部回取成文本
	if(CurRoom->MeanlingList.size()){
		//取出一个，返回其文本
		//int64 MeaningID = Parent->MeanlingList.back();
        CurRoom->MeanlingList.pop_back();

		tstring Word;				
		while(CurRoom){ //向父空间漫游回取完整的文本
			Word = CurRoom->RoomText+Word;
			CurRoom = CurRoom->Parent;
		}
		if(Word.size()){
			CNotifyState nf(NOTIFY_DIALOG_OUTPUT);
			nf.PushInt(FORECAST_INFO);
			nf.PushString(Word);
			nf.Notify(Dialog);
		}
		return;
	}
	
 
    //继续往深处挖掘 			
	assert(CurRoom->ChildList.size() == 0 && CurRoom->MeanlingList.size()==0);
	
	//得到Token->m_MemoryID的所有子空间  
	GetAllChild(CurRoom);
	if(CurRoom->ChildList.size()){
		CurRoom->It = CurRoom->ChildList.begin();
	}
	else {//表示当前空间没有子空间，虚置一个表示结尾
		ForecastRoom* Empty = new ForecastRoom();
		CurRoom->ChildList[0] = Empty;
		CurRoom->It = CurRoom->ChildList.end(); //表示已经处理过			
	}
	return;
};

void CLogicThread::Forecast(CClause* Clause){

};

void CLogicThread::Forecast(CSentence* Sentence){

	
};

