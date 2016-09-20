// LogicThread.cpp: implementation of the CLogicThread class.
// 学习部分
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "LogicThread.h"
#include "InstinctDefine.h"
#include "BrainMemory.h"
#include "LogicDialog.h"

//语法分析
///////////////////////////////////////////////////////////////////////

/*只对抽象行为的描述进行语法检查

  抽象行为的描述特点是：没有主语，必须有谓语，宾语可选。对于英语
  语法来说，谓语是一个动词，它必须是描述子句的第一个单词，紧接着
  可以有一个副词来形容。宾语是一个名词，它位于子句的最后一个单词，
  宾语可以被其它形容词修饰，如果有形容词，它位于宾语之前，谓语或
  副词之后。


  由于很多单词有多个词性，在检查时首先寻找第一个含有动词词性的单词，
  假设它是谓语，然后找最后一个含有名词词性的单词，假设它是宾语，据
  此检查单词，确定其词性（供存储时使用）。如果发现矛盾则报错。
  
  
 
  怎么处理's格式的形容词？
  名词+'s后变成形容词，即使语法检查通过，是按原来名词形式记忆还是
  按变形后的形容词记忆？如果按名词记忆，那么回取文字时就无法再还原
  成名词+'s形式，而记忆变形后的形式，又缺乏灵活性，当用户没有输入
  's时会理解出错。因此决定直接允许宾语前含有名词做修饰而忽略输入
  的's形式，

  语句的基本格式(对于英语)：
        [谓语] [副词] [形容词/名词] [宾语]
  其中：
  - 形容词也可以是名词加‘s
  - 副词最多有一个，如果有只能位于谓语的后面

*/
bool CLogicThread::SyntaxCheck(CClause* Clause,list<CWord>& WordList,tstring& Error){
	int32 WordNum = Clause->m_TokenNum;
	if(WordNum==0){
		Error = _T("no words");
		return false;
	}
	
	CToken* LastToken = m_Text.m_TokenList[Clause->m_BeginPos+WordNum-1]; 
	if(LastToken->IsClauseFlag())WordNum--;
	
	if(WordNum == 0){
		Error = _T("no available words");
		return false;
	}
	
    int32 i=Clause->m_BeginPos;
	int32 EndPos = i+WordNum;

	//构造所有token的词性表
	for(i; i<EndPos; i++)
	{
		CToken* Token = m_Text.m_TokenList[i];  
		CWord Word(*Token);

		//每一个单词都必须是正常可读的
		if(Token->m_Type != COMMON ){
			Error = Format1024(_T("the word '%s' is invalid"),Token->m_Str.c_str());
			return false;
		}
		 			
		if(IsNum(Token->m_Str)){
			Error = Format1024(_T("the word '%s' is invalid"),Token->m_Str.c_str());
            return false;   //抽象行为不应该含有数字   
			//Word.m_PartOfSpeech = NUM;
		} 
		else if(Token->isPunct() && Token->m_Str==_T("\'")){
			if(i==0 || i==EndPos){
				Error = Format1024(_T("the punct '%s' is invalid"),Token->m_Str.c_str());
				return false;
			}
			CToken* NextToken = m_Text.m_TokenList[++i];
			if(NextToken->m_Str != _T("s")){
				Error = Format1024(_T("the punct '%s%s' is invalid"),Token->m_Str.c_str(),NextToken->m_Str.c_str());
				return false;
			}	
			CWord& PreWord = WordList.back();
			PreWord.m_PartOfSpeech = MEMORY_NOUN;
		} //除去前述两种情况，Token应该已经被正确记忆过
		else if(!Token->IsOK()){
			//可能此单词没有在记忆中
			Error = Format1024(_T("the word '%s' maybe haven't be memory"),Token->m_Str.c_str());
			return false;
		}
		Word.m_AllPart = GetAllPartOfSpeech(Token->m_MemoryID);	  			
		WordList.push_back(Word);
	}

 	
	list<CWord>::iterator Ita = WordList.begin();
	//CWord* Word = (CWord*)Ita;
	//首先一头一尾确定为谓语和宾语
    if(!Ita->HasPartOfSpeech(MEMORY_VERB))return false;
	Ita->m_PartOfSpeech = MEMORY_VERB;

	if(WordNum == 1)return true;

    list<CWord>::reverse_iterator Itb = WordList.rbegin();
	if(!Itb->HasPartOfSpeech(MEMORY_NOUN))return false;
	Itb->m_PartOfSpeech = MEMORY_NOUN;  

	if(WordNum == 2)return true;
	
	//如果在谓语和宾语之前还有单词则确定副词和形容词
	Ita++;
    int32 p = 1;

	//第二个token优先识别为副词，忽略可能含有的形容词歧义
	if(Ita->HasPartOfSpeech(MEMORY_ADVERB)){
	 	Ita->m_PartOfSpeech = MEMORY_ADVERB;
	    Ita++;
		p=2;
	}

	//继续检查宾语之前，此token之后的单词是否为名词或形容词
	for(i=p; i<WordNum-1; i++){
		if(Ita->m_PartOfSpeech == MEMORY_NOUN)continue;
		else if(Ita->HasPartOfSpeech(MEMORY_NOUN) ){
			Ita->m_PartOfSpeech = MEMORY_NOUN;
		}else if(Ita->HasPartOfSpeech(MEMORY_ADJECTIVE) ){
			Ita->m_PartOfSpeech = MEMORY_ADJECTIVE;
		}
		else return false;
		Ita++;
	}
	return true;
	
};


//基本记忆操作
///////////////////////////////////////////////////////////////////////

int64 CLogicThread::LearnToken( CToken* Token,
								int64 Meaning /*=NULL_MEANING*/,
								int64 MeaningSense/*=0*/,
								int64 ReturnID /*= 0*/) 
{

	if(Token->Size()==0)return 0;
	               
	switch (Token->m_Type){
    case COMMON:
		{
			vector<int64> CharList(Token->Size());
			for(int i=0; i<Token->Size();i++){
				CharList[i] = CharToID(Token->m_Str[i]);
			}
			return LearnMemory(CharList,Meaning,MEMORY_LAN,MeaningSense,ReturnID, ROOT_SPACE,ROOT_LOGIC_TEXT,true);
		}
	case PUNCT:
        {
            if(isgraph(Token->m_Str[0])){ //可见字符
				vector<int64> CharList;
				CharList.push_back(CharToID(Token->m_Str[0]));
				return  LearnMemory(CharList,Meaning,MEMORY_LAN,MeaningSense,ReturnID, ROOT_SPACE,ROOT_LOGIC_TEXT,true);			
			}
		}
  	case NEWLINE:      ///r或/n
		return 0;	   
	default:
		assert(0);
	}
	return 0;				
}
	
int64 CLogicThread::LearnClause(CClause* Clause,
								int64 Meaning /*=NULL_MEANING*/,
								int64 MeaningSense/*=0*/,
								int64 ReturnID /*= 0*/) 
{
	 
	 int32 WordNum = Clause->m_TokenNum;
	 if(WordNum==0)return 0;

	 CToken* LastToken = m_Text.m_TokenList[Clause->m_BeginPos+WordNum-1]; 
	 if(Meaning !=NULL_MEANING && LastToken->IsClauseFlag())WordNum--;

   	 if(WordNum == 0)return 0;
  
	 vector<int64> WordList;
   	     
	 int32 j=Clause->m_BeginPos;
	 int32 EndPos = j+WordNum;

	 int64 ID = 0;
	 	
	 if(WordNum == 1){
	     CToken* Token = m_Text.m_TokenList[j];
		 ID = LearnToken(Token,Meaning,MeaningSense,ReturnID);
		 return ID;
	 }
	 for(j; j<EndPos; j++)
	 {
		CToken* Token = m_Text.m_TokenList[j];
		ID = LearnToken(Token);
		if(ID>0)WordList.push_back(ID);
		else return 0;
     }

	 if(WordNum == 1 && Meaning == NULL_MEANING)return ID;
	 return LearnMemory(WordList,Meaning,MEMORY_LAN,MeaningSense,ReturnID);

}

int64 CLogicThread::LearnSentence(CSentence* Sentence,
								int64 Meaning /*=NULL_MEANING*/,
								int64 MeaningSense/*=0*/,
								int64 ReturnID /*= 0*/) 
{
	 int32 ClauseNum = Sentence->m_ClauseNum;
   	 if(ClauseNum == 0)return 0;
  
	 vector<int64> ClauseList;

	 int32 j=Sentence->m_BeginPos;
	 int32 EndPos = j+ClauseNum;

	 int64 ID = 0;
	 if(ClauseNum == 1){
	     CClause* Clause = m_Text.m_ClauseList[j];
		 ID = LearnClause(Clause,Meaning,MeaningSense,ReturnID);
		 return ID;
	 }
	 for(j; j<ClauseNum; j++)
	 {
		CClause* Clause = m_Text.m_ClauseList[j];
		ID = LearnClause(Clause);
		if(ID>0)ClauseList.push_back(ID);
     }

	 return LearnMemory(ClauseList,Meaning,MEMORY_LAN,MeaningSense,ReturnID);
}


int64 CLogicThread::LearnParagraph(int32 BeginPos,int32 SentenceNum,
								int64 Meaning /*=NULL_MEANING*/,
								int64 MeaningSense/*=0*/,
								int64 ReturnID /*= 0*/) {

   	 if(SentenceNum == 0)return 0;
  
	 vector<int64> SentenceList;

	 int64 ID = 0;
	 if(SentenceNum == 1){
		 CSentence* Sentence = m_Text.m_SentenceList[BeginPos];
		 ID = LearnSentence(Sentence,Meaning,MeaningSense,ReturnID);		 
		 return ID;
	 }
	 for(int j=BeginPos; j<SentenceNum; j++)
	 {
		CSentence* Sentence = m_Text.m_SentenceList[j];
		ID = LearnSentence(Sentence);
		if(ID>0)SentenceList.push_back(ID);
     }

	 return LearnMemory(SentenceList,Meaning,MEMORY_LAN,MeaningSense,ReturnID);

}

int64 CLogicThread::LearnText(tstring& Text,
							  int64 Meaning/*=NULL_MEANING*/, 
							  int64 MeaningSense/*=0*/,
							  int64 ReturnID/*=0*/){
	
	CLogicThread Think(99,0); //避免破坏当前的思考结果

	for(int i=0; i<Text.size(); i++){
		Think.m_Text.Analyse(i,Text[i]);
	}
 
	return Think.LearnText(Meaning,MeaningSense,ReturnID);
}

int64 CLogicThread::LearnText( int64 Meaning /*=NULL_MEANING*/,
							   int64 MeaningSense /*=0*/,
							   int64 ReturnID /*=0*/
							 ){
    int64 r = 0; 
	if(m_Text.m_SentenceList.size()==0)return 0;
    if(m_Text.m_SentenceList.size()==1){
		r = LearnSentence(m_Text.m_SentenceList.back(),Meaning,MeaningSense,ReturnID);
	}
    else{
	    map<int32,int32> ParagraphList;
		int32 begin = 0;
		int64 ID ; 
		int32 i;
		for(i =1; i<m_Text.m_SentenceList.size(); i++){
			CSentence* Sentence = m_Text.m_SentenceList[i];
			if(Sentence == NULL){ //分割段落
				ParagraphList[begin] =i-begin;
				begin = i+1;
			}
		}
		
		if(begin <i){
			ParagraphList[begin] =i-begin;
		}

		vector<int64> ValueList;
		map<int32,int32>::iterator It = ParagraphList.begin();
		if(ParagraphList.size()==1){
			ID = LearnParagraph(It->first,It->second,Meaning,MeaningSense,ReturnID);
			return ID;
		}
		while (It != ParagraphList.end())
		{
			ID = LearnParagraph(It->first,It->second);
			if(ID == 0 )return 0;
			ValueList.push_back(ID);
			It++;
		}
		r = LearnMemory(ValueList,Meaning,MEMORY_LAN,MeaningSense,ReturnID);	
	}
	return r;
}


CToken* CLogicThread::JustOneToken(){
	
	if(m_Text.m_TokenList.size() == 1){
		return m_Text.m_TokenList.back();
	}
	return NULL;
}
bool  CLogicThread::IsFloatNum(){
	if(m_Text.m_TokenList.size() == 3){
		tstring s = m_Text.GetText();
		if(IsNum(s)==FLOAT_NUM)return true;
	}
	return false;
}	
CClause* CLogicThread::JustOneClause(){
	
	if( m_Text.m_ClauseList.size()==1){
		return m_Text.m_ClauseList.back();
	}
	return NULL;
}

CSentence* CLogicThread::JustOneSentence(){
	if(m_Text.m_SentenceList.size()==1){
		return m_Text.m_SentenceList.back();
	}
	return NULL;
};

int64 CLogicThread::LearnAction(CClause* Clause,int64 ActionID,tstring& Error,int64 MeaningSense,bool IsCheck){
   	
	list<CWord> WordList;
	if(!Clause){
		Error = _T("Not only one clause");
		return 0;
	}
	tstring CheckError;
	if(!SyntaxCheck(Clause,WordList,CheckError)){
		Error = _T("Syntax check fail: ")+CheckError;
		return 0;
	}
	//记忆子句

	int64 ID;
	vector<int64> MemoryIDList;
	list<CWord>::iterator It = WordList.begin();
	while(It != WordList.end())
	{
		CWord& Word = *It;
		ID = HasMeaningSpace(Word.m_ID,0,Word.m_PartOfSpeech);
		assert(ID>0);
		if(ID==0)return 0;
		MemoryIDList.push_back(ID);
		It++;
	}
				
	if(IsCheck)return CheckMemory(MemoryIDList,ActionID,MEMORY_INSTINCT,MeaningSense);
	
	return LearnMemory(MemoryIDList,ActionID,MEMORY_INSTINCT,MeaningSense);	

}

int64 CLogicThread::LearnAction(CLogicDialog* Dialog,tstring& Command,int64 ActionID,int64 MeaningSense,bool IsCheck){

	CLogicThread Think; //避免破坏当前的思考结果
		
	for(int i=0; i<Command.size();i++){		
		Think.m_Text.Analyse(i,Command[i]);
		Think.ThinkAllAffectedToken(Dialog);
	}
	CClause* Clause = Think.JustOneClause();
	m_LastError = _T("");
	return Think.LearnAction(Clause,ActionID,m_LastError,MeaningSense,IsCheck);
};
 

int64 CLogicThread::LearnInstinctInstance(int64 ActionID,Energy* Param){
	assert(ActionID>11655820);

	vector<int64> ValueList;
	ValueList.push_back(ActionID);
	int64 ParamID = 0;
	if(Param != NULL){
		TCHAR buf[50];
		tstring s;
		ENERGY_TYPE t = Param->EnergyType();
		switch(t)
		{

		case TYPE_INT:
			{
				int64 v = *(int64*)Param->Value();
				_i64tot(v,buf,10);
				s = buf;
				ParamID = LearnWord(s,MEMORY_NUMERAL);
			}
			break;
		case TYPE_FLOAT:
			{
				float64 v = *(float64*)Param->Value();
				_stprintf(buf,_T("%.6f"),v);
				s = buf;
				ParamID = LearnWord(s,MEMORY_NUMERAL);		
			}
			break;
		case TYPE_STRING:
			{
				s = *(tstring*)Param->Value();
				ParamID = LearnText(s);
			}
			break;
		case TYPE_PIPELINE:
			{
				if (ActionID == INSTINCT_USE_OBJECT)
				{
					ePipeline*  ObjectInfo = (ePipeline*)Param;										
					ParamID = LearnPipe(*ObjectInfo);
				}
			}
			break;
		default:
			return 0;
		}
		if (ParamID ==0 )
		{
			return 0;
		}
		ValueList.push_back(ParamID);
	}
    int64 ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ACT);
	return ID;
}
   
int64 CLogicThread::LearnWord(tstring Text,int64 PartOfSpeech,int64 MeaningSense/*=0*/,int64 ReturnID /*=0*/,bool IsCheck/*=0*/){
	
	CLogicThread Think; //避免破坏当前的思考结果

	for(int i=0; i<Text.size(); i++){
		Think.m_Text.Analyse(i,Text[i]);
		//ThinkToken();
	}

	bool Onetoken = false;
	if(PartOfSpeech==MEMORY_NUMERAL){
		if(IsNum(Text) != NOT_NUM) Onetoken = true;
	}

	if(!Onetoken){
		CToken* Token = Think.JustOneToken();
		if(!Token || Token->Size()==0)return 0;
	}

	vector<int64> CharList(Text.size());

	for(int j=0; j<Text.size();j++){
		CharList[j] = CharToID(Text[j]);
	}

	
	if(IsCheck){
		return CheckMemory(CharList,NULL_MEANING,PartOfSpeech,MeaningSense);
	}else{
		return LearnMemory(CharList,NULL_MEANING,PartOfSpeech,MeaningSense,ReturnID, ROOT_SPACE,ROOT_LOGIC_TEXT,true);	
	}

	return 0;
}

int64  CLogicThread::LearnEnergy(Energy* E){
	int64 ID = 0;
	vector<int64> ValueList;
	ENERGY_TYPE Type = E->EnergyType();
	
	switch(Type){
	   case TYPE_NULL:
		   {
			   ValueList.push_back(Type);
			   ValueList.push_back(0);
			   ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ENERGY,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);
		   }
		   break;
	   case TYPE_INT:
		   {
			   int64* value=(int64 *)E->Value();
			   ValueList.push_back(Type);
			   ValueList.push_back(*value);
			   ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ENERGY,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);
			   
		   }
		   break;
	   case TYPE_FLOAT:
		   {
			   assert(0); //暂且这样简化
			   float64* value=(float64 *)E->Value();
			   ValueList.push_back(Type);
			   ValueList.push_back(*value);
			   ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ENERGY,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);
		   }
		   break;
	   case TYPE_STRING:
		   {
			   tstring* value=(tstring *)E->Value();
			   ID = LearnText(*value);
			   if (ID==0)
			   {
				   return NULL;
			   }
			   ValueList.push_back(Type);
			   ValueList.push_back(ID);
			   ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ENERGY,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);
		   }
		   break;
	   case TYPE_PIPELINE:
		   {
			   ePipeline* value=(ePipeline *)E->Value();
			   ID = LearnPipe(*value);
		   }      
		   break;
	   default:
		   break;
	   }
	   return ID;
}

int64  CLogicThread::LearnPipe(ePipeline& Pipe){
	vector<int64> ValueList;

	int64 ID = 0;
	for (int i=0; i<Pipe.Size(); i++)
	{
		Energy* E = Pipe.GetEnergy(i);
		ID = LearnEnergy(E);
		if (ID==0)
		{
			return 0;
		}
		
        ValueList.push_back(ID);
	}
	
	ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ENERGY,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);

	if(!ID){
		return 0;
	}
	
	ValueList.clear();
	ValueList.push_back(TYPE_PIPELINE);
    ValueList.push_back(ID);

	ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_ENERGY,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);

	return ID;
}

int64  CLogicThread::LearnToken(tstring& s){
	vector<int64> ValueList;
	for (int i =0; i<s.size(); i++)
	{
		TCHAR ch = s[i];
		ValueList.push_back( CharToID(ch));
	}
	int64 ID = LearnMemory(ValueList,NULL_MEANING,MEMORY_LAN,MEANING_SENSE_OK,0, ROOT_SPACE,ROOT_LOGIC_TEXT,true);
	return ID;
}



int64 CLogicThread::LearnWordFromWord(tstring NewWord,int64 PartOfSpeech,tstring OldWord){
	CLogicThread Think; //避免破坏当前的思考结果
	
	//首先检测新单词是不是token
	for(int i=0; i<NewWord.size(); i++){
		Think.m_Text.Analyse(i,NewWord[i]);
		//ThinkToken();
	}
    CToken* Token = Think.JustOneToken();
	if(!Token || Token->Size()==0){
		return 0;
	}

	//根据已经记忆的单词，获得指定词性的意义空间ID
	vector<int64> CharList(OldWord.size());
	int j;
	for(j=0; j<OldWord.size();j++){
		CharList[j] = CharToID(OldWord[j]);
	}

	int64 MeaningID = GetMeaningSpaceID(CharList,NULL_MEANING,PartOfSpeech);
	if (!MeaningID)
	{
		return 0;

	}
	
	//记忆新单词的形,已经记忆的不会重复记忆
    CharList.resize(NewWord.size());
	for(j=0; j<NewWord.size();j++){
		CharList[j] = CharToID(NewWord[j]);
	}

	int64 SpaceID = InsertMultiSpace(CharList,MEMORY_BODY,ROOT_SPACE,ROOT_LOGIC_TEXT,true);
	if(!SpaceID)return 0;


	//在新单词里先寻找是否有指定词性，并且与旧单词意义空间ID相同	
	int64 MeaningSpaceID = HasMeaningSpace(SpaceID,NULL_MEANING,PartOfSpeech);
	if(MeaningID == MeaningSpaceID){
		return MeaningID;
	}else{
		//不相同则记忆一个相同的
		MeaningSpaceID = InsertMeaningSpace(SpaceID,CharList.back(),NULL_MEANING,PartOfSpeech,MeaningID);

		//给一个有效评价的结尾
		if(MeaningSpaceID)InsertEndSpace(MeaningSpaceID,MEANING_SENSE_OK,MEMORY_LOGIC_END);
		return MeaningSpaceID;
	}

	return 0;
}


int64 CLogicThread::LearnObject(int64 IP,SpaceAddress& Address,int64 crc32,tstring Comment,bool IsPeople /*= false*/){
 	vector<int64> ValueList;
	ValueList.push_back(IP);
    ValueList.push_back(Address.ParentID);
    ValueList.push_back(Address.ChildID);
	

	int64 ObjectType = IsPeople? MEMORY_PEOPLE : MEMORY_OBJECT;
	int64 ID = CheckMemory(ValueList,crc32,ObjectType);
	if(ID == 0){
		ID = LearnMemory(ValueList,crc32,ObjectType);
	}
	if(ID == 0)return 0;
	if (Comment.size())
	{
		CommentMemory(Comment,ID);
	}
	return ID;
}

int64 CLogicThread::LearnObjectAuto(int64 IP,tstring AddressText,int64 crc32,bool IsPeople /*= false*/){
 	vector<int64> ValueList;
	ValueList.push_back(IP);

	//AddressText 类似 Home\liveroom\my.dll 
	
	CLogicThread Think; //避免破坏当前的思考结果

	for(int i=0; i<AddressText.size(); i++){ 
		Think.m_Text.Analyse(i,AddressText[i]);
	}
    CClause* Clause = Think.JustOneClause();
	if(!Clause || Clause->m_TokenNum==0)return 0;
	
	int32 p = 0;
	vector<int64> TempList;
	for(int j=0; j<Think.m_Text.m_TokenList.size();j++){
		CToken* t = Think.m_Text.m_TokenList[j];
		if(t->m_Str == _T("\\") && TempList.size()){
			int64 ID = CheckMemory(TempList,NULL_MEANING,MEMORY_LAN);
			if(ID == 0){
				ID = LearnMemory(TempList,NULL_MEANING,MEMORY_LAN);
			}
			ValueList.push_back(ID);
			TempList.clear();
		}
		int64 TokenID = CheckWord(t->m_Str,MEMORY_LAN);
		if (TokenID)
		{
			TempList.push_back(TokenID);    	
		}else{
			int64 ID = LearnWord(t->m_Str,MEMORY_LAN);
			if(ID == 0)return 0;
			TempList.push_back(ID);    
		}	
	}
	
	if(TempList.size()){
		int64 ID = CheckMemory(TempList,NULL_MEANING,MEMORY_LAN);
		if(ID == 0){
			ID = LearnMemory(TempList,NULL_MEANING,MEMORY_LAN);
		}
		ValueList.push_back(ID);
		TempList.clear();
	};

	int64 ObjectType = IsPeople? MEMORY_PEOPLE : MEMORY_OBJECT;
	int64 ID = CheckMemory(ValueList,crc32,ObjectType);
	if(ID == 0){
		ID = LearnMemory(ValueList,crc32,ObjectType);
	}
	return ID;
}

/*
记忆逻辑的具体步骤

- 取出每一个子句的命令和参数，记忆成行为实例，得到存储ID
- 判断是并联还串联，如果是并联把前一个得到的行为存储ID存储到并联vector里，否则存储到串联vector里
- 当并联改为串联后，把并联vector记忆成逻辑，得到的存储ID存入串联vector
- 最后把串联vector存储为逻辑，返回存储ID

  **或许，编译任务也应该采用这种更简单的算法
*/
int64 CLogicThread::LearnLogic(CLogicDialog* Dialog,ePipeline* Sentence){
		
	if(Sentence->Size()==0){
		return 0;
	}
  
    vector<int64> SeriesList;
	vector<int64> ShuntList;		

	int64 ActionID = 0 ;
    Energy*  Param = NULL;
	for(int i=0; i<Sentence->Size(); i++){

		ePipeline* ClausePipe = (ePipeline*)Sentence->GetData(i);

		int64 Instinct = *(int64*)ClausePipe->GetData(0);

		if(Instinct<0){  //与前一个行为并联
			assert(SeriesList.size()>0);
            
			ActionID = SeriesList.back();

			SeriesList.pop_back();
			ShuntList.push_back(ActionID);

			do{ //处理所有并联
				Param = NULL;
				if(ClausePipe->Size()>1){
					Param = ClausePipe->GetEnergy(1);				
				}
				Instinct = -Instinct;

				//如果是嵌套逻辑
				if(INSTINCT_USE_LOGIC == Instinct )
				{
					assert(Param != NULL); 
					tstring* ChildLogicName = (tstring*)Param->Value();

					CLocalLogicCell* ChildLogic = Dialog->FindLogic(*ChildLogicName);

					if(ChildLogic == NULL){		
						return 0;
					}

					ePipeline ChildLogicData = ChildLogic->m_Task.m_LogicData;
					ActionID = LearnLogic(Dialog,&ChildLogicData);
					assert(ActionID!=0);
				}
				else {
					ActionID = LearnInstinctInstance(Instinct,Param);	
					assert(ActionID!=0);
				}	
				if(ActionID == 0)return 0;
				ShuntList.push_back(ActionID);
						
				i++;
			    if(i == Sentence->Size())break;

				ClausePipe = (ePipeline*)Sentence->GetData(i);
				Instinct = *(int64*)ClausePipe->GetData(0);	

			}while(Instinct<0);

			ActionID = LearnMemory(ShuntList,NULL_MEANING,MEMORY_SHUNT);
			assert(ActionID!=0);

			if(ActionID == 0)return 0;
			ShuntList.clear();

			//Sentence只是单一的并联体
			if(i == Sentence->Size() && SeriesList.size()==0){
				return ActionID;
			}
			//否则并联将作为串联的一部分
			SeriesList.push_back(ActionID);
	
		}
		 
		Param = NULL;
		if(ClausePipe->Size()>1){
			Param = ClausePipe->GetEnergy(1);				
		}

		if(INSTINCT_USE_LOGIC == Instinct)
		{
			assert(Param != NULL); 
			tstring* ChildLogicName = (tstring*)Param->Value();

			CLocalLogicCell* ChildLogic = Dialog->FindLogic(*ChildLogicName);

			if(ChildLogic == NULL){		
				return 0;
			}

			//预先记忆逻辑体
			ePipeline ChildLogicData = ChildLogic->m_Task.m_LogicData;
			ActionID = LearnLogic(Dialog,&ChildLogicData);
			assert(ActionID!=0);
		}
		else {
			ActionID = LearnInstinctInstance(Instinct,Param);				
			assert(ActionID!=0);
		}
		if(ActionID == 0)return 0;
		SeriesList.push_back(ActionID);

	}
    assert(SeriesList.size() >0);
	ActionID = LearnMemory(SeriesList,NULL_MEANING,MEMORY_SERIES,0);
	assert(ActionID!=0);
	return ActionID;
}
/*
int64 CLogicThread::LearnLogic(CGlobalLogicItem* Logic){
	if (Logic->m_LogicMemo.size()==0)
	{
		RuntimeOutput("The Logic [%s] lost useful description",Logic->m_LogicName.c_str());
	    return 0;
	}
	//存储每一个行为实例
    int64 LogicID = LearnLogic(&Logic->m_LogicData);
	//表达此逻辑的使用
	if(LogicID ){
        int64 ID = CommentMemory(Logic->m_LogicMemo,LogicID);
        if (ID == 0)
        { 
			RuntimeOutput("Error: when learning the logic description",Logic->m_LogicName.c_str());
            return 0;
		}	
	}
	return LogicID;
}
*/