// LogicThread.cpp: implementation of the CLogicThread class.
// 分析信号部分
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)
#pragma warning(disable: 4244)

#include "Brain.h"
#include "LogicThread.h"
#include "InstinctDefine.h"
#include "NotifyMsgDef.h"
#include "LogicDialog.h"
#include "TextAnalyse.h"



int32 CLogicThread::GetActionRoom(int64 ParentID,ClauseLogicSense* cls){
	   CppSQLite3Buffer SQL;
       char a[30],b[30];
	   
	   uint32 PartOfSpeech = 0;
//	   if(!RBrainHasTable(ID))return 0;

	   map<int64,MeaningPos>& RoomList = cls->ClauseMeaning;

	   ToRBrain(ParentID);

	   int64toa(ParentID,a);
	   int64toa(MEMORY_INSTINCT,b);
	   SQL.format("select %s,%s from \"%s\" where %s = \"%s\" ;",
						RB_SPACE_ID,
						RB_SPACE_VALUE,
			            a,
						RB_SPACE_TYPE,
						b
						);
	   CppSQLite3Table t = BrainDB.getTable(SQL);

	   for (int row = 0; row < t.numRows(); row++)
	   {
			t.setRow(row);
			int64 ID = t.getInt64Field(0);
			int64 Value = t.getInt64Field(1);
			MeaningPos Pos;
			Pos.RoomID = ID;
			Pos.ParamTokenPos = cls->NextTokenPos;
            RoomList[Value] = Pos;
	   }
	   return RoomList.size();	
}

void CLogicThread::ThinkProc(CLogicDialog* Dialog,CMsg& Msg){
	TASK_STATE State = Dialog->GetTaskState();
	int64 EventID = Msg.GetEventID();
	assert(EventID);
	
	_CLOCK(&m_Mutex);
	ePipeline& Letter = m_TextMsgQueue[EventID];
	assert(Letter.Size()==0);
	
	Letter << Msg.GetLetter();    
	
	if (State == TASK_THINK)
    {
		return;
    }

	map<int64,ePipeline>::iterator it = m_TextMsgQueue.begin();
	ePipeline& Letter1 = it->second;
	EventID = it->first;

	//下一个信息时间戳必须和上一个信息预先留下的一致
	//例外是上一个信息留下的=0；
	if (m_LastMsgTimeStamp !=0  && EventID != m_LastMsgTimeStamp ) 
	{
		return;
	};

	int64 Pos  = Letter1.PopInt();
	
	eElectron e;
	Letter1.Pop(&e);

	tstring text;

	if(e.EnergyType() == TYPE_INT){
		int64 n = e.Int64();
		text.assign(n,127); 
	}else {
		assert(e.EnergyType() == TYPE_STRING);
		text = e.String();
	}

	EventID    = Letter1.PopInt();

	m_TextMsgQueue.erase(it);

	ThinkProc(Dialog,Pos,text,true,EventID);
}
void CLogicThread::ThinkProc(CLogicDialog* Dialog,int32 Pos,tstring& Msg,bool Forecast ,int64 EventID){
 
	//_CLOCK(&m_Mutex);
	Dialog->SetTaskState(TASK_THINK);
	Dialog->NotifyTaskState();

	do{	
		m_LastMsgTimeStamp = EventID;

		int n = Msg.size();
					
		if (Msg.size()>1)
		{
			for(int i=0; i<Msg.size();i++){	
				int per = i*100/n;		
				
				TCHAR ch = Msg[i];
				if(ch == 127){
					m_Text.Analyse(Pos,ch);
				}
				else{
					m_Text.Analyse(Pos++,ch);
				}
				
				ThinkAllAffectedToken(Dialog);
				ThinkAllAffectedClause(Dialog);
				ThinkAllAffectedSentence(Dialog);
				
				if(Msg.size()>100){
					CNotifyDialogState nf(NOTIFY_PROGRESS_OUTPUT);
					nf.PushInt(THINK_PROGRESS);
					nf.PushInt(per);
					nf.Notify(Dialog);
				}	
			}
			
		}else{
			TCHAR ch = Msg[0];
			if(ch == 127){
				m_Text.Analyse(Pos,ch);
			}
			else{
				m_Text.Analyse(Pos++,ch);
			}
			ThinkAllAffectedToken(Dialog);
			ThinkAllAffectedClause(Dialog);
			ThinkAllAffectedSentence(Dialog);
		}
		
		map<int64,ePipeline>::iterator it = m_TextMsgQueue.begin();
		if (it != m_TextMsgQueue.end())
		{
			EventID = it->first;
			if (EventID != m_LastMsgTimeStamp)
			{
				break;  //如果当前首个信息的时间戳不是上一个信息指定的，则说明需要继续等待接收信息
			}

			ePipeline& Letter = it->second;
			
			Pos  = Letter.PopInt();
			Msg  = Letter.PopString();
			EventID = Letter.PopInt();

			m_TextMsgQueue.erase(it);
		}else{
			break;
		}
	}while(1);

	if(!Forecast)SetForecast(Dialog,NULL_TEXT,NULL);
	
	Dialog->SetTaskState(TASK_IDLE);
	Dialog->NotifyTaskState();
}

void CLogicThread::ThinkAllAffectedToken(CLogicDialog* Dialog){
	deque<InfectedText>::iterator It = m_Text.m_InfectedTokenList.begin();
	while(It != m_Text.m_InfectedTokenList.end()){
		InfectedText& Text = *It;
		Think(Dialog,(CToken*)Text.Text,Text.AlterPos);
		It++;
	}
	m_Text.m_InfectedTokenList.clear();
}
void CLogicThread::Think(CLogicDialog* Dialog,CToken* Token, int32 AlterCharPos)
{
    SetForecast(Dialog,NULL_TEXT,NULL);

	if(Token->m_Type != COMMON){
		return;
	}

	TokenLogicSense* tls = &m_TokenSense;
	
    int64 ParentID= ROOT_SPACE;	
	int pos=0;//字符分析起始位置，=0表示重新分析

	if(tls->BelongToken != Token){
		if(Token->IsOK() && AlterCharPos == Token->Size()-1){
			tls->BelongToken = Token;
			tls->NextCharPos = AlterCharPos;
			tls->LastCharID  = Token->m_MemoryID;
			pos = AlterCharPos;
			ParentID = tls->LastCharID;
		}
		else{
			tls->BelongToken = Token;
		}
	}
	else if(tls->NextCharPos){ //tls->BelongToken == Token 并且已经被正确分析过
		if(AlterCharPos == tls->NextCharPos){
			pos=AlterCharPos; //正常分析
            ParentID = tls->LastCharID;
		}
		else if(AlterCharPos > tls->NextCharPos){
			//因为前一个字符已经出错，不用分析也是错，
			//assert(!Token->IsOK());
	    	//throw "AlterCharPos > NextCharPos";
			return;
		}
	}

#ifdef _DEBUG    
	if(pos==0)assert(ParentID == ROOT_SPACE);//如果完全排除了错误不应该出现这种情况，以防万一，
#endif

	int32 CharNum = Token->m_Str.size();	
	for(pos; pos<CharNum;pos++){
		TCHAR ch = _totlower(Token->m_Str[pos]);
		int64 ChildID = CharToID(ch);
		
		//优先从预测表里找
		if(m_ForecastText == (void*)Token && ForecastOK()){		
			ParentID = GetForecastResult(ChildID);
		}
		else { //不行则直接从数据库里找			
			ParentID = GetChildID(ParentID,ChildID,MEMORY_BODY);
		}
		
		if(ParentID == 0){
			tls->NextCharPos = pos;
			Token->m_MemoryID = ERROR_2;    //不能理解，但对于token来说不一定是错误，比如实时数字
			//NotifyTokenError(Token,ERROR_2);
			return;		
		}
		tls->LastCharID = ParentID;
	}
	tls->NextCharPos = pos;
	Token->m_MemoryID = ParentID;
	
	assert(Token->m_Type == COMMON);
	SetForecast(Dialog,TOKEN,Token);
}

void CLogicThread::ThinkAllAffectedClause(CLogicDialog* Dialog){
	deque<InfectedText>::iterator It = m_Text.m_InfectedClauseList.begin();
	while(It != m_Text.m_InfectedClauseList.end()){
		InfectedText& Text = *It;
        CClause* Clause = (CClause*)Text.Text;
		Think(Dialog,Clause,Text.AlterPos);
		It++;
	}
	m_Text.m_InfectedClauseList.clear();
}

/*
如何避免重复分析?

输入token的任何一个字符都将引起Clause的改变和重新分析,实际上应该在确定
此token完全输入完毕后才分析token,确定token是否输入完毕有两种情况,
1 在Clause的中间修改token,即altertokenpos < cls.NextTokenPos
   则相应token在形式上肯定是已经完毕的,并应该重新分析字句 
2 在Clause的末尾按顺序输入,如果开始了新token则前一个token输入完毕,
  即altertokenpos > cls.NextTokenPos后取出cls.nexttoken分析
3 如果altertokenpos == cls.nexttokenpos,则两种情况,
   一种是altertokenpos<EndPos,表示中间修改应该从nexttokenpos出分析
   否则(altertokenpos==endpos)表示nexttokenpos还没有输入完毕,直接返回
*/
void CLogicThread::Think(CLogicDialog* Dialog,CClause* Clause, int32 AlterTokenPos)
{

	assert(m_Text.m_TokenList.size() != 0);

	ClauseLogicSense* cls = &m_ClauseSense;
	
	int32 EndPos  = Clause->m_BeginPos+Clause->m_TokenNum;
	
    EndPos--;//忽略Clause最后的标点
	
	bool  IsCompleted = false;
	bool  bNeedAnalyse = true;	
	bool  bChange = false;

	CToken* Token = m_Text.m_TokenList[EndPos];
	if(Token->IsClauseFlag()){
		IsCompleted = true;
	} 

	
	//回取分析逻辑场景
	if(cls->BelongClause != Clause){  //只能从头分析，不能象Token那样可以简单的恢复场景
		cls->BelongClause = Clause;  
		cls->NextTokenPos = Clause->m_BeginPos;
		cls->FirstWord = true;
		cls->ClauseMeaning.clear();
		cls->FatherList.clear();
	}
	else{
	    if(AlterTokenPos == cls->NextTokenPos){
            if(AlterTokenPos==EndPos && !Token->IsOK()){
				return ;  //表示目标token还没有输入完毕
			}
            //else  从NextTokenPos开始分析  
		}
		else if(AlterTokenPos < cls->NextTokenPos){ //修改现有结果,只能重头分析
		     cls->BelongClause = Clause;
		     cls->NextTokenPos = Clause->m_BeginPos;
		     cls->FirstWord = true;
		     cls->ClauseMeaning.clear();
		     cls->FatherList.clear();
		}
		else if((AlterTokenPos - cls->NextTokenPos)>1){ 
			 bNeedAnalyse = false; 
		}else if(IsCompleted && EndPos==AlterTokenPos && cls->ClauseMeaning.size()){
			 bNeedAnalyse = false; 		
		}
		//else 从NextTokenPos开始分析            	
	}		
       
	vector<int64>        TempList;       
	//从最后一个正确的token pos开始分析
	if(bNeedAnalyse){
		int32 TokenCount = Clause->m_BeginPos+Clause->m_TokenNum;
		for(cls->NextTokenPos; cls->NextTokenPos<TokenCount;cls->NextTokenPos++){
			Token = m_Text.m_TokenList[cls->NextTokenPos];
			
			//无法继续理解，但还不能算错，因为Token可能是参数，先退出
			if (Token->m_Type != COMMON || !Token->IsOK())break; 
			
			//取得每一个单词的词性空间，
			map<int64,int64>     RoomList;
			if(GetAllPartOfSpeechRoom(Token->m_MemoryID, RoomList)==0)break;;
			
			if(cls->FirstWord){  
				
				map<int64,int64>::iterator It = RoomList.begin();
				
				if(cls->NextTokenPos == Clause->m_BeginPos){
					//提前处理两个特殊的关系代词,确定此子句的逻辑关系
					if(It->second == MEMORY_LOGIC_AND){
						Clause->m_LogicRelation = SHUNT_RELATION;
						continue;
					}
					else if(It->second == MEMORY_LOGIC_THEN){
						Clause->m_LogicRelation = SERIES_RELATION;
						continue;
					}	
					else Clause->m_LogicRelation = SERIES_RELATION;
				}
				//尝试用每一个词性打头，看是否有可能的句子
				while(It != RoomList.end()){				
					int64 ID = GetChildID(ROOT_SPACE,It->first,MEMORY_BODY);
					if(ID){
						cls->FatherList.push_back(ID);
						bChange = true;
					}
					It++;
				}
				cls->FirstWord = false;
			}
			else{ //让前后TOKEN的每一个词性相互修饰
				vector<int64>::iterator FatherIt = cls->FatherList.begin();
				while(FatherIt != cls->FatherList.end()){
					int64 ParentID = *FatherIt;
					map<int64,int64>::iterator ChildIt = RoomList.begin();
					while(ChildIt != RoomList.end()){
						int64 ChildID = ChildIt->first;
						//优先从预测表里找
						if(m_ForecastText == (void*)Clause && ForecastOK()){		
							ChildID = GetForecastResult(ChildID);
						}
						else { //不行则直接从数据库里找			
							ChildID = GetChildID(ParentID,ChildID,MEMORY_BODY);
						}				
						
						if(ChildID) TempList.push_back(ChildID);						
						ChildIt++;
					}
					FatherIt++;
				}
				
				if(TempList.size() == 0){			
					break; //无法继续理解，非正常退出
				}
				else {
					cls->FatherList.assign(TempList.begin(),TempList.end());
					TempList.clear();
					bChange = true;
				}
				
			}
		}//end for
	}
    

	//不管当前理解是以正常退出，还是因为错误退出，只要修改了cls->FatherList含有的条
	//目，就以当前修饰结果为形，重新取得其含有的意义,如果
	//是行为则暂存到clause意义列表中
	if(bChange){
		cls->ClauseMeaning.clear();
		vector<int64>::iterator It = cls->FatherList.begin();
		while(It != cls->FatherList.end()){
			GetActionRoom(*It, cls);
			It++;
		}
	};
	
	//现在以行为识别优先来处理所得到的意义，即使非正常退出。
	//因为非正常退出可能是本能带有动态参数，或语句倒装。
	//但行为识别的前提是本子句已经是一个完整句（以逗号或句号结尾）
	if(!IsCompleted){
		//如果本子句还没有输入完毕，则暂存现有分析结果
		Clause->m_MemoryID = INPUT;
		return ;
	}

	//行为识别可能出现多个情况
	//理想的情况是只识别出一个行为，此时检查此行为的格式即可
	//如识别出多个行为则需要用户选择
	//如果一个都没有识别出，则考虑用户是否需要搜索有关记忆
	//否则是输入错误。

	MeaningPos MPos;
    if(cls->ClauseMeaning.size() == 0 ){ //一个行为意义都没有则询问是否执行搜索
		if(QueryFind()){ //似乎应该取消，因为会自动预测
			//ExecuteFind(Clause);
			//CutClause(Clause);
		}
		else{
			Clause->m_MemoryID = ERROR_2;
		}
		return;
	}  
    else if(cls->ClauseMeaning.size() >1){
		//要求用户选择一个
		int64 ID = SelectMeaning(cls);
		if(ID ==0){  //用户放弃选择，
			Clause->m_MemoryID = ERROR_2;
			return;
		}
		Clause->m_MemoryID = ID;
        
	}	//只有一个行为意义 注意：此时m_MemoryID不是空间ID，而是空间值
    else{
		Clause->m_MemoryID =  cls->ClauseMeaning.begin()->first; 
		MPos = cls->ClauseMeaning.begin()->second; 
	} 

	
	//现在确定唯一的行为后，确定可能含有的参数
	vector<tstring> ParamList;
    for(int pos=MPos.ParamTokenPos; pos<EndPos;pos++){
		Token = m_Text.m_TokenList[pos];
		if(Token->IsReference()){ //如果是引号，则合并两个引号之间的token为一个token
			tstring Param;
			int32 insertpos =0;
			++pos;
			int32 CharPos = Token->m_BeginPos+1;
			for(pos; pos<EndPos;pos++){
				Token = m_Text.m_TokenList[pos];
                if(Token->IsReference())break;		
				//Param += Token->m_Str.c_str();
				//Param +=' '; //合并的token之间加一个空格，但会导致token失真，当作为Name使用时需注意
			    insertpos = Token->m_BeginPos-CharPos;
				Param.resize(insertpos,' ');
				Param += Token->m_Str.c_str();
			}
            ParamList.push_back(Param);
			continue;
		}
		ParamList.push_back(Token->m_Str);
	}

	if(BelongInstinct(Clause->m_MemoryID)){
		//检查本能参数是否能够通过
		if(!CheckInstinctParam(Dialog,Clause,ParamList) ){
			//Clause->m_MemoryID = ERROR_20;
			assert(Clause->m_MemoryID>1 && Clause->m_MemoryID<LAST_ERROR);
			NotifyClauseError(Dialog,Clause,Clause->m_MemoryID);
			return ;
		}
	}//IsInstinct 		
	else{ // not Instinct
		//对于非本能遇到一个reference后，将被视作倒装语句
		//前提是后续的所有Token应该都应该是引用类型
		ePipeline Pipe;
		for(int j=0; j<ParamList.size(); j++){
			eSTRING* Param = new eSTRING(ParamList[j]);
			Pipe.Push_Directly(Param);
		}
		
		//暂存参数，处理句子时才真正还原倒装
		if(Pipe.Size()){
			ePipeline* Param = new ePipeline;
			*Param << Pipe;
			Clause->m_Param = Param; 
		}
		
	}	
	
    SetForecast(Dialog,CLAUSE,Clause);   
	return ;			  	
}

void CLogicThread::ThinkAllAffectedSentence(CLogicDialog* Dialog){
	deque<InfectedText>::iterator It = m_Text.m_InfectedSentenceList.begin();
	while(It != m_Text.m_InfectedSentenceList.end()){
		InfectedText& Text = *It;
		Think(Dialog,(CSentence*)Text.Text,Text.AlterPos);
		It++;
	}
    m_Text.m_InfectedSentenceList.clear();
};

void CLogicThread::Think(CLogicDialog* Dialog,CSentence* Sentence, int32 AlterClausePos)
{
 	assert(m_Text.m_ClauseList.size() != 0);
    
	vector<int64>  TempList;

	//如果修改的子句还没有输入完毕则不做处理
	assert(AlterClausePos<m_Text.m_ClauseList.size());
    CClause* Clause = m_Text.m_ClauseList[AlterClausePos];
	int32 EndPos  = Clause->m_BeginPos+Clause->m_TokenNum -1;
	if(EndPos<0)return ; //空句子? 可能吗?
	CToken* Token = m_Text.m_TokenList[EndPos];
	if(!Token->IsClauseFlag())return;
	
	//回取逻辑场景
	SentenceLogicSense* sls = &m_SentenceSense;
		
	//如果当前句子不是之前修改的句子，或者不能继续使用之前的分析结果，则重置
    int32 pos = Sentence->m_BeginPos;
	if(sls->BelongSentence != Sentence){
		sls->BelongSentence = Sentence;
	}
	else{ //sls->BelongSentence == Sentence
		if(AlterClausePos == sls->NextClausePos){
			pos=AlterClausePos; //正常分析
		}
		else if(AlterClausePos > sls->NextClausePos){
			//因为前一个Clause已经出错，不用分析也是错，
			assert(!Sentence->IsOK());
			return;
		}
	}

   	if(pos==Sentence->m_BeginPos){
		sls->FatherList.clear();
		Sentence->m_AnalyseResult.Clear();
	}

	EndPos  = Sentence->m_BeginPos+Sentence->m_ClauseNum;
	bool HasError = false;
	
	//检查句子含有的子句之间是否有逻辑冲突
	//如果没有冲突，则试图把这些子句联系起来，推测下一句
	for(pos; pos<EndPos; pos++)
	{
		 Clause = m_Text.m_ClauseList[pos];
         assert(!Clause->IsBlank());
		 if(!Clause->IsOK()){
			 HasError = true;
			 break;
		 }

	     //并联关系词的前一子句不能是set label 和 goto label操作
		 if(Clause->m_LogicRelation == SHUNT_RELATION){
			 
			 if(pos==Sentence->m_BeginPos) //如果第一个子句是并联，警告，但继续执行
			 {
				//NotifyTokenWarning(m_Text.m_TokenList.at(0),i,SentenceIndex);
				//return false;
			 }
			 else{
				 CClause* PreClause =  m_Text.m_ClauseList.at(pos-1);
				 if(PreClause->m_MemoryID == INSTINCT_SET_LABEL ||
					 PreClause->m_MemoryID == INSTINCT_GOTO_LABEL)
				 {
					 Clause->m_MemoryID = ERROR_12;
					 NotifyClauseError(Dialog,Clause,ERROR_12);
					 HasError = true;
					 break;
				 }	
			 }
		 }
		 
		 Energy* Param = NULL;
		 if( BelongInstinct(Clause->m_MemoryID) ){
			 
			 if(BelongIndeInterAction(Clause->m_MemoryID) ){
				 Clause->m_ActionType = INDE_INTER_ACTION;
			 }	 
			 else if(BelongInterAction(Clause->m_MemoryID)){ //外部
				 Clause->m_ActionType = INTER_ACTION;
			 }
			 else Clause->m_ActionType = COMMON_ACTION;
            
			 
			 //如果本能是USE LOGIC, 需要检查临时逻辑是否和目前行为类型冲突
			 //用法: use logic srcName:NewlogicName
			 //注意:USE_LOGIC不属于IsInterOperator()的检查	
			 
			 if( INSTINCT_USE_LOGIC == Clause->m_MemoryID ){
				 eSTRING* s = (eSTRING*)Clause->m_Param;
				 assert(s!= NULL);
				
				 tstring LogicName = *(tstring*)s->Value();      				 
				 
				 //重用逻辑的格式为 原逻辑名:新逻辑名
				 
				 /*不强制要求
				 tstring::size_type n = LogicName.find(':');
				 
				 if(n == tstring::npos || n==LogicName.size()-1){
					 Clause->m_MemoryID = ERROR_23;
					 NotifyClauseError(srcMsg,Clause,ERROR_23);
					 return ;
				 }
				 */
				/* 转到执行阶段
				 LogicName.resize(n);
				 LogicName=TriToken(LogicName);

				 CLogicItem* lg = FindLogic(LogicName);
				 if(lg == NULL) {
					 Clause->m_MemoryID = ERROR_10;
					 NotifyClauseError(Clause,ERROR_10);
					 return;
				 }

				 if(!lg->IsValid()) {
					 Clause->m_MemoryID = ERROR_14;
					 NotifyClauseError(Clause,ERROR_14);
				     HasError = true;
					 break;
				 }
				 Clause->m_ActionType = lg->m_ActionState;
				 
				 ePipeline NewLogic(lg->m_LogicData);
				 *Pipe<< NewLogic;
				 Param = Pipe;
				 */
			 }
			 
			 if(Clause->m_Param){
				 Param = Clause->m_Param->Clone();
			 } 
		 }
		 else {//处理非本能，也就是逻辑行为
			 Clause->m_ActionType =  OUTER_ACTION;

			 //检查是否此子句表达的意思是否正确(思考：需要这样吗？）
			 //if(MEMORY.IsOK(ClauseID))
			 //  throw tstring("cannot understand you inputed clause,Please learn it first");
			 			 
			 //如果非本能有补语 则在本逻辑之前加入一个或多个定义字符串的行为。
			 if(Clause->m_Param != NULL){
				 assert(Clause->m_Param->EnergyType()==TYPE_PIPELINE);
				 ePipeline* Pipe = (ePipeline*)Clause->m_Param;
				 for(int i=0; i<Pipe->Size(); i++)
				 {
					 tstring* str = (tstring*)Pipe->GetData(i);
					 ePipeline*  Clause =  new ePipeline;
					 Clause->PushInt(INSTINCT_DEFINE_STRING);
					 Clause->PushString(*str);
					 Sentence->m_AnalyseResult.Push_Directly(Clause);     
				 }
			 }
			 
		 }	

		 if(Clause->m_LogicRelation == SHUNT_RELATION ){
			 Clause->m_MemoryID = -Clause->m_MemoryID;	
		 }
		 ePipeline*  ClausePipe =  new ePipeline;
		 ClausePipe->PushInt(Clause->m_MemoryID);
		 if(Param != NULL)ClausePipe->Push_Directly(Param);
		 Sentence->m_AnalyseResult.Push_Directly(ClausePipe);	 
	}
    
    sls->NextClausePos = pos; 
	Sentence->m_IsError = HasError;
	if(!HasError)SetForecast(Dialog,SENTENCE,Sentence);
	return;			
}


bool CLogicThread::CheckNameInInputed(int64 InstinctID,tstring Name,CClause* Clause,bool IsRef){
    if (InstinctID<0)
    {
		InstinctID = -InstinctID;
    }

	deque<CClause*>::iterator ClauseIt = m_Text.m_ClauseList.begin();	
	while(ClauseIt != m_Text.m_ClauseList.end()){
		CClause* c = *ClauseIt;
		int64 ID = c->m_MemoryID;
		if (ID<0)
		{
			ID = -ID;
		}

		ClauseIt ++;
		if(ID< 0xffff)continue;
		
		
		if( ID == InstinctID && c != Clause) 
		{ 
			CToken* Token = m_Text.m_TokenList[c->m_BeginPos+c->m_TokenNum-2];
			tstring LogicElementName = GetLogicElementName(Token->m_Str);
			if(LogicElementName == Name){
				return true;
			}
		}

		if (IsRef && c == Clause) //只能引用Clause之前的
		{
			break;
		}
	}
    return false;
}

tstring GetParam(vector<tstring>& ParamList){
	tstring Param;
	if(ParamList.size()==0){
		return Param;
	}
	
	vector<tstring>::iterator it = ParamList.begin();	
	while(it != ParamList.end()){
		if (*it != _T("\"") && *it != _T("“") && *it != _T("”"))
		{
			Param += *it;
		}
		it++;
	}

	return Param;
}


//原本想只做检查,不转换成实际数据,但要做检查,同样需要检查实际值,所以干脆全部做完
bool  CLogicThread::CheckInstinctParam(CLogicDialog* Dialog,CClause* Clause, vector<tstring>& ParamList){
	
	int64 InstinctID = Clause->m_MemoryID;
	assert(InstinctID !=0);
    
	if(Clause->m_Param != NULL){
		delete Clause->m_Param;
		Clause->m_Param = NULL;
	}
	
	switch(InstinctID){

	case INSTINCT_DEFINE_INT64:
	case INSTINCT_DEFINE_FLOAT64:
		{
			tstring s = GetParam(ParamList);

			NumType type = IsNum(s);
			if(type == NOT_NUM){
				//Error = "The param is not a num.";
				Clause->m_MemoryID = ERROR_5;
				return false;
			}
			if(type == INT_NUM){
				int64 t = _ttoi64(s.c_str());
				Clause->m_Param = new eINT(t);
			}
			else {
				TCHAR* endptr = NULL;
				float64 f = _tcstod(s.c_str(),&endptr);
				Clause->m_Param = new eFLOAT(f);
			}
		}
		break;
	case  INSTINCT_DEFINE_STRING :
		{ 
			if( ParamList.size()!=1 ){
				//Error = "the param  more than one or lose param";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring s = ParamList[0];
			//int i=0;
			//while(isspace(s[i]) && ++i < s.size()); //提出空格
			//s = s.substr(i,s.size()-1);
			Clause->m_Param = new eSTRING(s);
		}
		break;
	case INSTINCT_USE_OPERATOR:
		{
			if(ParamList.size() == 1){
				tstring s = ParamList[0];
				s=TriToken(s);
				if (s.size()==1)
				{
					TCHAR ch = s[0];
					switch(ch){
					case _T('+'):
					case _T('-'):
					case _T('*'):	
					case _T('/'):
					case _T('%'):
					case _T('>'):
					case _T('<'):
					case _T('!'):
					case _T('&'):
					case _T('~'):
					case _T('|'):
					case _T('^'):
						Clause->m_Param = new eSTRING(s);
						return true;
					default:
						break;
					}
				}else{
					if( s == _T(">=") || 
						s == _T("<=") || 
						s == _T("!=") || 
						s == _T("&&") || 
						s == _T("==") || 
						s == _T("<<") ||
						s == _T(">>") ||
						s == _T("||")  )
					{
						Clause->m_Param = new eSTRING(s);	
						return true;
					}
				}
			}else if (ParamList.size() == 2)//==如果没有被加引号会被视为两个字符串
			{
				tstring s = ParamList[0];
				s += ParamList[1];
				s=TriToken(s);
				if( s == _T(">=") || 
					s == _T("<=") || 
					s == _T("!=") || 
					s == _T("&&") || 
					s == _T("==") || 
					s == _T("<<") ||
					s == _T(">>") ||
					s == _T("||")  )
				{
					Clause->m_Param = new eSTRING(s);	
					return true;
				}
			}
			
			//Error = "the param  more than one or lose param";
			Clause->m_MemoryID = ERROR_3;
			return false;				
		}
		break;
	case INSTINCT_USE_RESISTOR:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when use resistor";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring s = ParamList[0];
			NumType type = IsNum(s);
			if(type == NOT_NUM || type == FLOAT_NUM){
				//Error = "The param must be a int num.";
				Clause->m_MemoryID = ERROR_6;
				return false;
			}
			int64 t = _ttoi64(s.c_str());
			if(t<0){
				//Error = "The param  must be > 0 ";
				Clause->m_MemoryID = ERROR_7;
				return false;
			}
			Clause->m_Param = new eINT(t);     
		}
		break;
	case INSTINCT_USE_INDUCTOR:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when use inductor/capacitor";
				Clause->m_MemoryID = ERROR_3;
				return false;
			};

			tstring Name = ParamList[0];
			Name = GetLogicElementName(Name);
			
			if (Dialog->FindInductor(Name) || CheckNameInInputed(INSTINCT_USE_INDUCTOR,Name,Clause,false))
			{
				Clause->m_MemoryID = ERROR_22;
				return false;
			}
			
			Clause->m_Param = new eSTRING(Name);
			
		}
		break;
	case INSTINCT_USE_CAPACITOR:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when use inductor/capacitor";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring Name = ParamList[0];
			Name = GetLogicElementName(Name);
			
			if (Dialog->FindInductor(Name) || CheckNameInInputed(INSTINCT_USE_CAPACITOR,Name,Clause,false))
			{
				Clause->m_MemoryID = ERROR_22;
				return false;
			}
			Clause->m_Param = new eSTRING(Name);
		}
		break;
	case INSTINCT_USE_DIODE:
		{
			if( ParamList.size()!=1 ){
				//Error = "the param  more than one or lose param when use diode";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring s = ParamList[0];
			NumType type = IsNum(s);
			if(type == NOT_NUM || type == FLOAT_NUM){
				//Error = "The param must be a int num.";
				Clause->m_MemoryID = ERROR_6;
				return false;
			}
			int64 t = _ttoi64(s.c_str());
			Clause->m_Param = new eINT(t);     
		}
		break;
	case INSTINCT_REFERENCE_CAPACITOR:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring Name = ParamList[0];
			
			if (!Dialog->FindCapacitor(Name) && !CheckNameInInputed(INSTINCT_USE_CAPACITOR,Name,Clause,true))
			{
				Clause->m_MemoryID = ERROR_21;
				return false;
			}
			Clause->m_Param = new eSTRING(Name); 	
		}
		break;
	case INSTINCT_REFERENCE_INDUCTOR:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring Name = ParamList[0];

			if (!Dialog->FindInductor(Name) && !CheckNameInInputed(INSTINCT_USE_INDUCTOR,Name,Clause,true))
			{
				Clause->m_MemoryID = ERROR_21;
				return false;
			}
			Clause->m_Param = new eSTRING(Name); 				
		}		
		break;

	case INSTINCT_SET_LABEL:
	case INSTINCT_GOTO_LABEL:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when using SET LABEL or GOTO LABEL command";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring LabelName = ParamList[0];
			LabelName = GetLogicElementName(LabelName);
			Clause->m_Param = new eSTRING(LabelName);     
		}
		break;
	case INSTINCT_VIEW_PIPE:
		break;
	case INSTINCT_INPUT_NUM:
	case INSTINCT_INPUT_TEXT:
		{
			if( ParamList.size() !=1 ){
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring LabelName = ParamList[0];
			LabelName = TriToken(LabelName);
			Clause->m_Param = new eSTRING(LabelName);     
		}
		break;
	case INSTINCT_WAIT_SECOND:
		{
			if(ParamList.size()==0){
				Clause->m_Param = NULL;
			}
			else if( ParamList.size()==1 ){
				tstring s = ParamList[0];
				NumType type = IsNum(s);
				if(type == NOT_NUM){
					//Error = "The param must be a  num.";
					Clause->m_MemoryID = ERROR_5;
					return false;
				}
				TCHAR* endptr = NULL;
				float64 f = _tcstod(s.c_str(),&endptr);

				Clause->m_Param = new eFLOAT(f);
			}else{
				//Error = "the param  more than one or lose param when use diode";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
		}
		break;
	case INSTINCT_CREATE_MEMORY:
		{
			if( ParamList.size() !=1 ){
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			};
			
			tstring Name = ParamList[0];
			Name = GetLogicElementName(Name);
			if (Dialog->m_NamedMemoryList.HasMemoryInstance(Dialog,Name) || CheckNameInInputed(INSTINCT_CREATE_MEMORY,Name,Clause,false)) 
			{
				Clause->m_MemoryID = ERROR_22;
				return false;
			}
			
			Clause->m_Param = new eSTRING(Name);     		
		}
		break;
	case INSTINCT_FOCUS_MEMORY:
        {
			if (ParamList.size() == 0) //动态版本
			{
				return true;
			}
			else if( ParamList.size() ==1 ){ //静态版本
				tstring Name = ParamList[0];
				Clause->m_Param = new eSTRING(Name);   
			}else{
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
		}
		break;

	case INSTINCT_SET_MEMORY_ADDRESS:
	case INSTINCT_GET_MEMORY_ADDRESS:
	case INSTINCT_CREATE_MEMORY_NODE:
	case INSTINCT_IMPORT_MEMORY:
	case INSTINCT_EXPORT_MEMORY:
	case INSTINCT_INSERT_MEMORY:
	case INSTINCT_GET_MEMORY:
	case INSTINCT_REMOVE_MEMORY:
	case INSTINCT_GET_MEMORY_SIZE:
	case INSTINCT_CLOSE_MEMORY:
	case INSTINCT_MODIFY_MEMORY:
	case INSTINCT_GET_MEMORY_FOCUS:
		{
			if (ParamList.size()!=0)
			{
				Clause->m_MemoryID = ERROR_3;
				return false;
			}	
		}
		break;
	case INSTINCT_USE_LOGIC:
		{
			tstring Name; 
			if (ParamList.size()==0)
			{
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			else if(ParamList.size()==1){
				Name = ParamList[0];
			}
			else{
				Name = GetParam(ParamList);
			}
				
			Name = TriToken(Name);
			tstring::size_type pos = Name.find(_T(':'));
			if (pos== tstring::npos)
			{
				pos = Name.find(_T('：'));
			}
			
			if (pos != tstring::npos)
			{
				Clause->m_Param = new eSTRING(Name.substr(0,pos)); 
			}else{
				Clause->m_Param = new eSTRING(Name); 
			}
		}		
		break;
	case INSTINCT_FOCUS_LOGIC:
		{
			if (ParamList.size() == 0) //动态版本
			{
				return true;
			}
			else if( ParamList.size() ==1 ){ //静态版本
				tstring Name = ParamList[0];
				Clause->m_Param = new eSTRING(Name);   
			}else{
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
		}
		break;
	case INSTINCT_INSERT_LOGIC:
		{
			if( ParamList.size() !=1 ){
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring InsertLogicName = ParamList[0];
			
			Clause->m_Param = new eSTRING(InsertLogicName); 
		}
		break;
	case INSTINCT_REMOVE_LOGIC:
	case INSTINCT_SET_LOGIC_ADDRESS:
	case INSTINCT_GET_DATE:
	case INSTINCT_GET_TIME:
	case INSTINCT_OUTPUT_INFO:
		{   if (ParamList.size() !=0)
			{
				Clause->m_MemoryID = ERROR_3;
				return false;
			}		
		}
		break;
	case INSTINCT_SET_LOGIC_BREAKPOINT:
	case INSTINCT_TEST_EXPECTATION:
		{
			if( ParamList.size()!=1 ){
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring s = ParamList[0];
			NumType type = IsNum(s);
			if(type == NOT_NUM){
				//Error = "The param must be a int num.";
				Clause->m_MemoryID = ERROR_6;
				return false;
			}
			int64 t = _ttoi64(s.c_str());
			Clause->m_Param = new eINT(t);     
		}
		break;
	case INSTINCT_START_OBJECT:
		{
			tstring Name = GetParam(ParamList);	
			Name = TriToken(Name);
			
			if (Name.size()==0)
			{
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			vector<CObjectData> ObjectList;
			int n = Dialog->FindObject(Name,ObjectList);
            if (!n)
            {
				Dialog->RuntimeOutput(0,_T("Warning: not find object(%s)"),Name.c_str());
            }
			Clause->m_Param = new eSTRING(Name);
			return true;
		}
		break;
	case INSTINCT_FOCUS_OBJECT:
		{		
			if (ParamList.size() == 0) //动态版本
			{
				return true;
			}
			else if( ParamList.size() ==1 ){ //静态版本
				tstring Name = ParamList[0];
				Clause->m_Param = new eSTRING(Name);   
			}else{
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
		}
		break;
	case INSTINCT_NAME_OBJECT:
		{
			/* 
			if( ParamList.size() !=1 ){
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			tstring Name = ParamList[0];
			Name = TriToken(Name);
			
			//检查名字是否唯一
			if (Dialog->m_NamedObjectList.HasName(Dialog,Name) || CheckNameInInputed(INSTINCT_NAME_OBJECT,Name,Clause,false))
			{
				Clause->m_MemoryID = ERROR_24;
				return false;
			}
			Clause->m_Param = new eSTRING(Name);
			*/
			if (ParamList.size() ==1)
			{
				tstring Name = ParamList[0];
				Name = TriToken(Name);
				Clause->m_Param = new eSTRING(Name);
			}
			else if( ParamList.size() !=0 ){
				//Error = "the param  more than one or lose param when reference element";
				Clause->m_MemoryID = ERROR_3;
				return false;
			} 

		}
		
		break;
	case INSTINCT_USE_OBJECT:
	case INSTINCT_CLOSE_OBJECT:
	case INSTINCT_GET_OBJECT_DOC:
	case INSTINCT_ASK_PEOPLE:
		break;
	case INSTINCT_THINK_LOGIC :
		{
			tstring LogicName;
			if (ParamList.size()==0)
			{
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			else if(ParamList.size()==1){
				LogicName= ParamList[0];
			}
			else{
				LogicName = GetParam(ParamList);
			}
	
			LogicName = TriToken(LogicName);				
			tstring::size_type pos = LogicName.find(_T(':'));
			if (pos== tstring::npos)
			{
				pos = LogicName.find(_T('：'));
			}
			if (pos != tstring::npos)
			{
				LogicName = LogicName.substr(0,pos); 
			}

			if(Dialog->FindLogic(LogicName) || CheckNameInInputed(INSTINCT_THINK_LOGIC,LogicName,Clause,false)){  //逻辑名重复
				Clause->m_MemoryID = ERROR_8;
				return false;
			}
			Clause->m_Param = new eSTRING(LogicName);			
		}
		break;
		
	case INSTINCT_RUN_TASK:
	case INSTINCT_DEBUG_TASK:	
	case INSTINCT_STOP_TASK:
	case INSTINCT_PAUSE_TASK:
	case INSTINCT_STEP_TASK:
	case INSTINCT_TEST_TASK:
		{
			if( ParamList.size()!=0 ){
				//Error = "param error" 
				Clause->m_MemoryID = ERROR_20;
				return false;
			}
		}
		break;
	case INSTINCT_GOTO_TASK:
		{
			if( ParamList.size()!=1 ){
				//Error = "the param  more than one or lose param";
				Clause->m_MemoryID = ERROR_3;
				return false;
			}
			
			tstring s = ParamList[0];
			NumType type = IsNum(s);
			if(type != INT_NUM){
				//Error = "The param must be a int num.";
				Clause->m_MemoryID = ERROR_6;
				return false;
			}
			int64 t = _ttoi64(s.c_str());
			Clause->m_Param = new eINT(t);     
		}
		break;
	case INSTINCT_SET_GLOBLELOGIC:
		break;
	case INSTINCT_CLOSE_DIALOG:
		break;
	case INSTINCT_LEARN_TOKEN:
	case INSTINCT_LEARN_PRONOUN:
	case INSTINCT_LEARN_ADJECTIVE:
	case INSTINCT_LEARN_NUMERAL:  
	case INSTINCT_LEARN_VERB:  
	case INSTINCT_LEARN_ADVERB: 
	case INSTINCT_LEARN_ARTICLE: 
	case INSTINCT_LEARN_PREPOSITION:
	case INSTINCT_LEARN_CONJUNCTION:
	case INSTINCT_LEARN_INTERJECTION:
	case INSTINCT_LEARN_NOUN: 
		{
			//只能有一个参数
			if(ParamList.size() != 1){
				Clause->m_MemoryID = ERROR_3;
				return false;
			}	
			tstring Word = ParamList[0];
			Clause->m_Param = new eSTRING(Word);	
		}
		break;
	case INSTINCT_LEARN_TEXT:
		{
			if(ParamList.size() == 1){
				tstring Text = ParamList[0];
				Clause->m_Param = new eSTRING(Text);				
			}else if(ParamList.size() == 0){
				tstring Text ;
				Clause->m_Param = new eSTRING(Text);				
			}else{
				//Error = "param error" 
				Clause->m_MemoryID = ERROR_20;
				return false;
			}
		}
		break;
	case INSTINCT_LEARN_LOGIC:
	case INSTINCT_LEARN_OBJECT:
	case INSTINCT_LEARN_ACTION:
		{
			//只能有一个参数
			if(ParamList.size() != 1){
				Clause->m_MemoryID = ERROR_3;
				return false;
			}	
			tstring Text = ParamList[0];
			Text = TriToken(Text);
			//这里应该对text进行相应的检查
			Clause->m_Param = new eSTRING(Text);		
		}
		break;
	case INSTINCT_FIND_SET_PRICISION:
		{
			if( ParamList.size()!=1 ){
				//Error = "param error" 
				Clause->m_MemoryID = ERROR_20;
				return false;
			}
			
			tstring s = ParamList[0];
			NumType type = IsNum(s);
			if(type != INT_NUM){
				//Error = "The param must be a int num.";
				Clause->m_MemoryID = ERROR_6;
				return false;
			}
			int32 t = _ttoi(s.c_str());
			Clause->m_Param = new eINT(t); 
		}
		break;
	case INSTINCT_FIND_SET_STARTTIME:
	case INSTINCT_FIND_SET_ENDTIME:
		{
			//				if( ParamList.size()!=0 ){
			//Error = "param error" 
			Clause->m_MemoryID = ERROR_20;
			return false;
			//				}
		}
		break;
	case INSTINCT_FIND:
	case INSTINCT_FIND_LOGIC:
	case INSTINCT_FIND_OBJECT:
		{
			if( ParamList.size()!=1 ){
				//Error = "param error" 
				Clause->m_MemoryID = ERROR_20;
				return false;
			}

			tstring s = ParamList[0];
			s=TriToken(s);
			Clause->m_Param = new eSTRING(s); 
		}
		break;
	case INSTINCT_USE_ARM:
		{
			if( ParamList.size()!=0 ){
				//Error = "param error" 
				Clause->m_MemoryID = ERROR_20;
				return false;
			}
		}
		break;
	case INSTINCT_CREATE_ACCOUNT:
	case INSTINCT_DELETE_ACCOUNT:
		{
			if( ParamList.size()!=0 ){
				//Error = "param error" 
				Clause->m_MemoryID = ERROR_20;
				return false;
			}
		};
		break;
	default:
		assert(0);	
		return false;
	}

	return true;
}


void CLogicThread::NotifyTokenError(CLogicDialog* Dialog,CToken* Token,uint32 ErrorID)
{
	 assert(ErrorID<LAST_ERROR);
	 tstring error = ANALYSE_ERROR[ErrorID];
	 tstring s = Format1024(_T("Error token = %s : %s\n"),Token->m_Str.c_str(),error.c_str());
	 
	 CNotifyDialogState nf(NOTIFY_DIALOG_ERROR);
	 nf.PushInt(THINK_ERROR);
	 nf.PushString(s);
	 nf.Notify(Dialog);

	 m_InErrorState = true;
};


void CLogicThread::NotifyClauseError(CLogicDialog* Dialog,CClause* Clause,uint32 ErrorID)
{
     
	 assert(ErrorID<LAST_ERROR);
	 if(ErrorID>LAST_ERROR)ErrorID= ERROR_0;
	 tstring error = ANALYSE_ERROR[ErrorID];
	 tstring s = Format1024(_T("Clause error: %s\n"),error.c_str());

	 
	 CNotifyDialogState nf(NOTIFY_DIALOG_ERROR);
	 nf.PushInt(THINK_ERROR);
	 nf.PushString(s);
	 nf.Notify(Dialog);

	 m_InErrorState = true;
};



tstring  CLogicThread::GetErrorText(int32 CharPos)
{
	tstring s;
    deque<CSentence*>::reverse_iterator SentenceIt = m_Text.m_SentenceList.rbegin(); 	
	while(SentenceIt!=m_Text.m_SentenceList.rend() && CharPos< (*SentenceIt)->m_BeginPos )SentenceIt++; 
	
	if(SentenceIt == m_Text.m_SentenceList.rend())return s;

    CSentence* Sentence = *SentenceIt;
		
	//再找到当前逻辑场景所包含的句子，子句，Token
	CharPos  = CharPos - Sentence->m_BeginPos;
	
    deque<CClause*>::reverse_iterator ClauseIt = m_Text.m_ClauseList.rbegin();
	while(ClauseIt!=m_Text.m_ClauseList.rend() && CharPos< (*ClauseIt)->m_BeginPos )ClauseIt++; 
	if(ClauseIt == m_Text.m_ClauseList.rend())return s;

	CClause* Clause = *ClauseIt;

	CharPos  = CharPos- Clause->m_BeginPos;
    deque<CToken*>::reverse_iterator It = m_Text.m_TokenList.rbegin();
	while(It!=m_Text.m_TokenList.rend() && CharPos< (*It)->m_BeginPos )It++; 
		
	if(It == m_Text.m_TokenList.rend())return s;
	CToken* Token  = *It;
 
	int i = 0;
	if(!Token->IsOK()){
		i = Token->m_MemoryID;
	}
	else if(!Clause->IsOK()){
	    i = Clause->m_MemoryID;
	}
	
	assert(i<LAST_ERROR);
	s = ANALYSE_ERROR[i];
	return s;
}
    

/*
int64  CLogicThread::SelectMeaning(map<int64,int64>& ClauseMeaning){
	return 0;
}
*/

bool CLogicThread::CheckActionConflict(CSentence* Sentence){
    
	int32 EndPos  = Sentence->m_BeginPos+Sentence->m_ClauseNum;
	uint32  ActionType = 0;
	for(int32 i = Sentence->m_BeginPos; i<EndPos; i++)
	{
		 CClause* Clause = m_Text.m_ClauseList[i];
		 ActionType |= Clause->m_ActionType;
	};

	if(ActionType & OUTER_ACTION){
		if(ActionType&INTER_ACTION  || ActionType&INDE_INTER_ACTION)return true;
	}
	else if(ActionType & INDE_INTER_ACTION){
        if(Sentence->m_ClauseNum != 1)return true;
	}
	Sentence->m_State = ActionType;
	return false;
}





