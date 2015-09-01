// TextAnalyse.cpp: implementation of the CText class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "TextAnalyse.h"
#include "LogicThread.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CText::CText()
{
	m_ParentThread  = 0;
    m_CurTokenSelect = 0;
    m_CurClauseSelect = 0; 
	m_CurSentenceSelect = 0;
}

CText::~CText()
{
	Clear();
}
	
	
bool CText::IsComplete(){
	if(m_TokenList.size() && m_TokenList.back()->IsSentenceFlag())return true;
	return false;
}
CToken* CText::InsertToken(uint32 Index,tstring Token,uint32 BeginPos,TOKENTYPE Type, int32 TokenState)
{
    CToken* T = new CToken;
	T->m_MemoryID = TokenState;
	T->m_BeginPos = BeginPos;
	T->m_Str     = Token;
	T->m_Type     = Type;

	deque<CToken*>::iterator It = m_TokenList.insert(m_TokenList.begin()+Index,T);
	return *It;
}

CClause* CText::InsertClause(uint32 Index,uint32 BeginPos)
{
    CClause* C = new CClause;
	C->m_BeginPos       = BeginPos;
	C->m_TokenNum       = 0;
	C->m_LogicRelation  = UNKOWN_RELATION;
	C->m_MemoryID       = 0;

	deque<CClause*>::iterator It = m_ClauseList.insert(m_ClauseList.begin()+Index,C);
	return *It;
}

CSentence* CText::InsertSentence(uint32 Index,uint32 BeginPos)
{
    CSentence* S = new CSentence;
	S->m_BeginPos       = BeginPos;
	S->m_ClauseNum      = 0;

	deque<CSentence*>::iterator It = m_SentenceList.insert(m_SentenceList.begin()+Index,S);
	return *It;
}


/*       |---|   当pos属于此范围，则选定此token返回 
     ... abc  abc ...
*/
CToken*  CText::FindToken(uint32 Pos)
{
 	//优先考察pos与上一次发生编辑行为的Token的关系
	CToken* Token = NULL;
	deque<CToken*>::reverse_iterator It = m_TokenList.rbegin();
	if(m_CurTokenSelect<m_TokenList.size()){
		Token = m_TokenList[m_CurTokenSelect];
		if (Pos==Token->m_BeginPos)
		{
			return Token;
		}
		else if(Pos>Token->m_BeginPos){
			CToken* NextToken = NULL;
			if(m_CurTokenSelect+1<m_TokenList.size()){
				NextToken = m_TokenList[m_CurTokenSelect+1];
			}
			if(NextToken == NULL)return Token;
		    if(Pos<NextToken->m_BeginPos)return Token;
			else {
				m_CurTokenSelect = m_TokenList.size() -1; 	//从尾部逆向查找
			}
		}//否则从当前TOKEN位置向前找		
		else{//Pos<Token->m_BeginPos
			It += (m_TokenList.size() - m_CurTokenSelect);
			m_CurTokenSelect--;
		}
		
	}
	else m_CurTokenSelect = m_TokenList.size()-1; //从尾部逆向查找

	
	while(It!=m_TokenList.rend() && Pos< (*It)->m_BeginPos )
	{ 
		It++; 
		--m_CurTokenSelect;
	}
		
	if(It == m_TokenList.rend()){
		//比第一个Token还小，则生成一个
		CToken* T = new CToken();  
		T->m_BeginPos = Pos;
		m_TokenList.push_front(T); //如果后面输入字母可以直接加入token，但如过输入逗号则可能需要额外处理
		m_CurTokenSelect = 0;
         
		//所有子句其始TokenPos顺延
		ChangeClauseBeginPos(0,1);
		return T;
	}
    return *It;
};


CClause*  CText::FindClause(uint32 Pos)
{
	m_CurClauseSelect = m_ClauseList.size()-1;
    deque<CClause*>::reverse_iterator It = m_ClauseList.rbegin();
	while(It != m_ClauseList.rend() && Pos< (*It)->m_BeginPos ){
		It++; 
		--m_CurClauseSelect;
	}
  
	if(It == m_ClauseList.rend()){
		assert(m_ClauseList.size()==0); //理论上只有如此才可能找不到相应子句
		CClause* C = new CClause;
		m_ClauseList.push_front(C);
		m_CurClauseSelect = 0;
		return C;
	}
    return *It;
};

CSentence*  CText::FindSentence(uint32 Pos)
{
	m_CurSentenceSelect = m_SentenceList.size()-1;
    deque<CSentence*>::reverse_iterator It = m_SentenceList.rbegin(); 
	
	while(It!=m_SentenceList.rend() && Pos< (*It)->m_BeginPos ){
		It++; 
		--m_CurSentenceSelect;
	};

	if(It == m_SentenceList.rend())
	{
		assert(m_SentenceList.size()==0); //理论上只有如此才可能找不到相应句子
		CSentence* S = new CSentence;
		m_SentenceList.push_front(S);
		m_CurSentenceSelect = 0;
		return S;
	};
    return *It;
};


//Token已经改变，只处理后续Token以及当前子句的size
void  CText::ChangeTokenBeginPos(uint32 TokenPos,uint32 len) 
{ 
	if(TokenPos+1>=m_TokenList.size())return;
	deque<CToken*>::iterator It = m_TokenList.begin();
	It += TokenPos+1;
    while(It < m_TokenList.end())
	{
		CToken* Token = *It;
		Token->m_BeginPos += len;
		It++;
	}
}

void CText::ChangeClauseBeginPos(uint32 ClausePos,uint32 len)
{
	if(ClausePos+1>=m_ClauseList.size())return;

	deque<CClause*>::iterator It = m_ClauseList.begin();
	It += ClausePos+1;
    while(It < m_ClauseList.end())	
	{
		CClause* Clause = *It;
		Clause->m_BeginPos+=len;
		It++;
	}
}

void CText::ChangeSentenceBeginPos(uint32 SentencePos,uint32 len)
{
	if(SentencePos+1>=m_SentenceList.size())return;

	deque<CSentence*>::iterator It = m_SentenceList.begin();
	It += SentencePos+1;
    while(It != m_SentenceList.end())	
	{
		CSentence* Sentence = *It;
		Sentence->m_BeginPos += len;
		It++;
	}
}
void   CText::DeleteToken(uint32 Index){
	assert(Index<m_TokenList.size());
	deque<CToken*>::iterator It = m_TokenList.begin()+Index;
	if(m_ParentThread && m_ParentThread->m_ForecastText == (*It)){
		m_ParentThread->m_ForecastType = NULL_TEXT;
	}
	delete *It;
	m_TokenList.erase(It);
}
void   CText::DeleteClause(uint32 Index){
	assert(Index<m_ClauseList.size());
	deque<CClause*>::iterator It = m_ClauseList.begin()+Index;
	if(m_ParentThread && m_ParentThread->m_ForecastText == (*It)){
		m_ParentThread->m_ForecastType = NULL_TEXT;
	}
	delete *It;
	m_ClauseList.erase(It);
}
void CText::DeleteSentence(uint32 Index){
	assert(Index<m_SentenceList.size());
	deque<CSentence*>::iterator It = m_SentenceList.begin()+Index;
	if(m_ParentThread && m_ParentThread->m_ForecastText == (*It)){
		m_ParentThread->m_ForecastType = NULL_TEXT;
	}
	delete *It;
	m_SentenceList.erase(It);
}
	
void  CText::AddInfectedText(CToken* Token,uint32 AlterPos){
	InfectedText In(Token,AlterPos);
	m_InfectedTokenList.push_back(In);
}	
void  CText::AddInfectedText(CClause* Clause,uint32 AlterPos){
	InfectedText In(Clause,AlterPos);
	m_InfectedClauseList.push_back(In);
}	
void  CText::AddInfectedText(CSentence* Sentence,uint32 AlterPos){
	InfectedText In(Sentence,AlterPos);
	m_InfectedSentenceList.push_back(In);
}

void CText::Clear(){
    deque<CToken*>::iterator TokenIt = m_TokenList.begin();
    deque<CClause*>::iterator ClauseIt = m_ClauseList.begin();
	deque<CSentence*>::iterator SentenceIt = m_SentenceList.begin();

	while(TokenIt<m_TokenList.end()){ 
		CToken* Token = (*TokenIt);
		delete Token;
		TokenIt++;
	};
	m_TokenList.clear();

	while(ClauseIt<m_ClauseList.end()){ 
		CClause* Clause = (*ClauseIt);
		delete Clause;
		ClauseIt++;
	};
	m_ClauseList.clear();

	while(SentenceIt<m_SentenceList.end()){ 
		CSentence* Sentence = (*SentenceIt);
		delete Sentence;
		SentenceIt++;
	};
    m_SentenceList.clear();
	    
	m_CurTokenSelect = 0;
    m_CurClauseSelect = 0; 
	m_CurSentenceSelect = 0;	
}


/*插入空格
	                      ' '
	 举例 define int32 23,  define int32 234;
	                       ^
	 在^处插入空格
*/

bool CText::InsertSpace(CToken* CurToken,uint32 OppPos,TCHAR ch){
	
	if(OppPos==0){ //在CurToken的头字符处插入		
		if(CurToken->Size() == 0){ //这种情况只可能发生在第一个字符处
			DeleteToken(m_CurTokenSelect);
            ChangeClauseBeginPos(m_CurClauseSelect,-1);
			return false;
		}
		else CurToken->m_BeginPos++;
	}
	else if(OppPos==CurToken->Size()){ //在CurToken的尾部处插入
		return false;
	}
    else if(OppPos>CurToken->Size()){
		return false;
	}
	else{ //中间插入,把一个token分成了两个		
		assert(CurToken->m_Type != PUNCT);		
		tstring s = CurToken->m_Str.substr(OppPos,CurToken->Size()-OppPos);	
		CurToken->m_Str.resize(OppPos);
		CurToken->m_MemoryID = INPUT;	
		//插入后半部分token;
		CToken* NextToken = InsertToken(m_CurTokenSelect+1,s,CurToken->m_BeginPos+OppPos+1,COMMON,INPUT);	
		ChangeClauseBeginPos(m_CurClauseSelect,1);

		AddInfectedText(CurToken,OppPos);	
		AddInfectedText(NextToken,0);	
	}	
	return true;
}

/*插入普通符号
	                       a
	 举例 define int32 23,  define int32 234;
	                       ^
	 在^处插入逗号
*/

bool CText::InsertAlpha(CToken* CurToken,uint32 OppPos,TCHAR ch){
	CToken* NewToken = NULL;
	if(OppPos==0){ //在CurToken的头字符处插入		
		if(CurToken->Size() == 0){
			CurToken->m_Str = ch;
			AddInfectedText(CurToken,0);
			return true;
		}
		
		if(CurToken->isPunct()){
			//检查新字符是否与前一个Token相连构成一个token
			if(m_CurTokenSelect>0){
				CToken* PreToken = m_TokenList[m_CurTokenSelect-1];
				if(!PreToken->isPunct() && PreToken->m_BeginPos+PreToken->Size() == CurToken->m_BeginPos){
					PreToken->m_Str +=ch;
					--m_CurTokenSelect;
					AddInfectedText(PreToken,PreToken->Size()-1);
				}
				else{//不相连则插入新TOKEN
					NewToken = InsertToken(m_CurTokenSelect,ch,CurToken->m_BeginPos,COMMON,INPUT);				
				    ChangeClauseBeginPos(m_CurClauseSelect,1);
					AddInfectedText(NewToken,0);
				}
			}
			else {
				NewToken = InsertToken(m_CurTokenSelect,ch,CurToken->m_BeginPos,COMMON,INPUT);				
			    ChangeClauseBeginPos(m_CurClauseSelect,1);
				AddInfectedText(NewToken,0);
			}
			CurToken->m_BeginPos++;
		}
		else{
			CurToken->m_Str.insert((int)0,1,ch);
			AddInfectedText(CurToken,0);
		}	
	}
	else if(OppPos==CurToken->Size()){ //在CurToken的尾部处插入
		if(CurToken->isPunct()){
            //根据FindToken规则，不会出现与后token相连的情况，只需插入一个新Token	
			CurToken = InsertToken(++m_CurTokenSelect,ch,CurToken->m_BeginPos+OppPos,COMMON,INPUT);
		    ChangeClauseBeginPos(m_CurClauseSelect,1);
			AddInfectedText(CurToken,0);
		}
		else{
			CurToken->m_Str+=ch;
			AddInfectedText(CurToken,OppPos);
		}		
	}
	else if(OppPos>CurToken->Size()){
		CurToken = InsertToken(++m_CurTokenSelect,ch,CurToken->m_BeginPos+OppPos,COMMON,INPUT);
		ChangeClauseBeginPos(m_CurClauseSelect,1);
		AddInfectedText(CurToken,0);	
	}
	else{ //中间插入		
		CurToken->m_Str.insert(OppPos,1,ch);
		CurToken->m_MemoryID = INPUT;
		AddInfectedText(CurToken,OppPos);	
	}
	return true;
}



/*删除一个字符(backspace将被看作是插入一个delete)
	                     d  
	 举例 define int32 23,  define int32 234;
	                     ^
	 在^处插入delete

*/

bool CText::InsertBack(CToken* CurToken,uint32 OppPos){

	if(OppPos==0){ 		
		if(CurToken->Size() == 0){
			DeleteToken(m_CurTokenSelect);
			ChangeClauseBeginPos(m_CurClauseSelect,-1);
			return false;
		}
		else {	
            //删除普通Token的第一个字符
			CurToken->m_Str.erase(OppPos,1);
			CurToken->m_MemoryID = INPUT;			
			if(CurToken->m_Str.size()==0){
				DeleteToken(m_CurTokenSelect);
				ChangeClauseBeginPos(m_CurClauseSelect,-1);

				//对于只有一个字符的token，要考虑与前后两个Token相连
				if(m_CurTokenSelect>0 && m_CurTokenSelect<m_TokenList.size()){
					CurToken = m_TokenList[m_CurTokenSelect];
					CToken* PreToken = m_TokenList[m_CurTokenSelect-1];
					if(!PreToken->isPunct() && !CurToken->isPunct() &&
						PreToken->m_BeginPos+PreToken->Size() == CurToken->m_BeginPos){
                        PreToken->m_Str +=CurToken->m_Str;
						DeleteToken(m_CurTokenSelect);
						ChangeClauseBeginPos(m_CurClauseSelect,-1);						
					}
				}
			}
			else{
				AddInfectedText(CurToken,OppPos);	
			}
		}		
	}
	else if(OppPos==CurToken->Size()){ //在CurToken的尾部处插入
		//删除发生在一个CurToken的尾部要考虑和后续Token相连
		uint32 NextTokenNum = m_CurTokenSelect+1;
		if(!CurToken->isPunct() && NextTokenNum < m_TokenList.size()){
			CToken* NextToken = m_TokenList[NextTokenNum];
			if(!NextToken->isPunct() && CurToken->m_BeginPos+CurToken->m_Str.size() == NextToken->m_BeginPos){
				if( CurToken->m_Type == NextToken->m_Type ){ //需要更多条件?
					CurToken->m_Str += NextToken->m_Str;
					DeleteToken(NextTokenNum);     //删除NextToken
					ChangeClauseBeginPos(m_CurClauseSelect,-1);
					AddInfectedText(CurToken,OppPos);	
				}
			}			
		}		
	}
	else if(OppPos>CurToken->Size()){
        return false;
	}
	else{ //中间插入
		assert(!CurToken->isPunct());
		CurToken->m_Str.erase(OppPos,1);
		CurToken->m_MemoryID = INPUT;
	    AddInfectedText(CurToken,OppPos);	
	}
	return true;
}

bool CText::InsertNewLineFlag(CToken* CurToken,uint32 OppPos,TCHAR ch){
	//暂时按空格处理
	return InsertSpace(CurToken,OppPos,_T(' '));
}
  
bool CText::InsertPunct(CToken* CurToken,uint32 OppPos,TCHAR ch){
	if(OppPos==0){ //在CurToken的头字符处插入分号		
		if(CurToken->Size() == 0){  //新token
			CurToken->m_Str = ch;
			CurToken->m_Type = PUNCT;
			CurToken->m_MemoryID = OK;	
		}
		else{ 
			CurToken->m_BeginPos++;
			InsertToken(m_CurTokenSelect,ch,CurToken->m_BeginPos-1,PUNCT,OK); 							    
		}
	}
	else if(OppPos==CurToken->Size()){ //在CurToken的尾部处插入
	  
		CurToken = InsertToken(++m_CurTokenSelect,ch,CurToken->m_BeginPos+OppPos,PUNCT,OK);			
	}
	else if(OppPos>CurToken->Size()){ //在CurToken之后		
		CurToken = InsertToken(++m_CurTokenSelect,ch,CurToken->m_BeginPos+OppPos,PUNCT,OK);					
	}
	else{ //在Token中间插入
		//把一个token分成了两个
		assert(!CurToken->isPunct());
		tstring s = CurToken->m_Str.substr(OppPos,CurToken->Size()-OppPos);	
		CurToken->m_Str.resize(OppPos);	
		CurToken->m_MemoryID = INPUT;
		AddInfectedText(CurToken,OppPos);	   
		CurToken = InsertToken(m_CurTokenSelect+1,ch,CurToken->m_BeginPos + OppPos,PUNCT,OK);
		//插入后半部分token;
		CurToken = InsertToken(m_CurTokenSelect+2,s,CurToken->m_BeginPos+CurToken->Size(),COMMON,INPUT);
		AddInfectedText(CurToken,0);
	}
    return true;
}
  



/*
  根据最早修改的Token位置找所属的子句,
  从当前子句的第一个Token开始，根据标点符号的优先级重新为子句分配token,
  如果一个子句的Token分配没有受到影响，则停止分配。
  记录受影响的子句及发生修改的Token位置，等待分析。

  标点符号的优先级：
  1 引号
  2 逗号，分号（句号），感叹号
  3 括号，大括号，方括号，单引号（不包括's）

  分配方法：
  - 依次取出每一个Token,当遇到一个引号token时，直到找到下一个引号为止，期间所有
    Token都属于当前子句。
  - 否则找下一个逗号，分号和感叹号结束一个子句
  - 如果遇到其他标点符号先作为普通token接受，留待以后分析解释
*/

void CText::SplitClause(){

	deque<CToken*>::iterator It = m_TokenList.begin();	
    assert(It != m_TokenList.end());

    m_InfectedClauseList.clear();	
    
	assert(m_CurClauseSelect<m_ClauseList.size());
	CClause* Clause = m_ClauseList[m_CurClauseSelect];
	uint32 CurClauseNum  = m_CurClauseSelect;
    uint32 AlterTokenPos = m_CurTokenSelect;

	//更新m_CurSentenceSelect，因为在插入或删除Clause是需要调整之后Sentence的BeginPos
	FindSentence(CurClauseNum);

	It += Clause->m_BeginPos;
	if(It < m_TokenList.end()){
		Clause->m_TokenNum = 0;
	}
	else{
		DeleteClause(CurClauseNum);
		return;
	}

	bool Complete = false;
    //int32 AlterPos = m_FirstTokenPos-Clause->m_BeginPos;
	while(It < m_TokenList.end()){
		CToken* Token = *It;
		Clause->m_TokenNum++;	
        if(Token->IsReference()){ //优先
			It++;
			while(It != m_TokenList.end()){
				Token = *It;
				Clause->m_TokenNum++;
				if(Token->IsReference())break;
				It++;
			}
			//所有token已经被本子句包括，删除之后的子句
			if(It == m_TokenList.end()){
				deque<CClause*>::iterator It = m_ClauseList.begin()+CurClauseNum+1;
				while(It != m_ClauseList.end()){
					delete *It;
					*It = NULL;
					It = m_ClauseList.erase(It);
				}
			}
		}
		else if(Token->IsClauseFlag()){	    		
		    uint32 NextPos = Clause->m_BeginPos+Clause->m_TokenNum;
			
			if(AlterTokenPos <= (NextPos-1)){
				//AlterTokenPos = min(AlterTokenPos,NextPos-1);
				AddInfectedText(Clause,AlterTokenPos);
			}else{
				m_CurClauseSelect++;
			} 

			Complete = true;
			++CurClauseNum; //下一个子句
REDO:		
		    if(CurClauseNum<m_ClauseList.size()){
				CClause* NextClause = m_ClauseList[CurClauseNum];								
				//012345678   
				//|---|-- |
				if(NextPos<NextClause->m_BeginPos){//小于紧邻的Token则需要填补一个子句
                    Clause = InsertClause(CurClauseNum,NextPos);
					AlterTokenPos = NextPos;
                    Complete = false;
				}
				else if(NextPos == NextClause->m_BeginPos)return;
				else if(NextPos < (NextClause->m_BeginPos+NextClause->m_TokenNum)){ //如果占用了下一个子句的部分
					NextClause->m_TokenNum  = NextClause->m_BeginPos+NextClause->m_TokenNum-NextPos;
                    NextClause->m_BeginPos  = NextPos;
					AddInfectedText(NextClause,0);
					return;			
				}
				else{
					DeleteClause(CurClauseNum);
					ChangeSentenceBeginPos(m_CurSentenceSelect,-1);
					goto REDO;
				}
			}
			else{ //没有下一个子句，
				if(NextPos<m_TokenList.size()){//但还有剩下的token没有分配完
					Clause = InsertClause(CurClauseNum,NextPos);

					AlterTokenPos = NextPos;
					Complete = false;
				}
				else return;
			} 
		}
		if(It<m_TokenList.end())It++;
	}
	if(!Complete){
		AddInfectedText(Clause,AlterTokenPos);
	}
}

/*
  找到最早受影响子句所属的Sentence，
  如果

*/
void CText::SplitSentence(){
		
  	deque<CClause*>::iterator It = m_ClauseList.begin();	
	assert(It != m_ClauseList.end());
    m_InfectedSentenceList.clear();

	assert(m_CurSentenceSelect<m_SentenceList.size());
	CSentence* Sentence = m_SentenceList[m_CurSentenceSelect];
	uint32 CurSentenceNum = m_CurSentenceSelect;
    uint32 AlterClausePos = m_CurClauseSelect;
  

	It += Sentence->m_BeginPos;
	if(It < m_ClauseList.end()){
		Sentence->m_ClauseNum = 0;
	}
	else{
		DeleteSentence(CurSentenceNum);
		return;
	}

	bool Complete = false;
	while(It != m_ClauseList.end()){
		CClause* Clause    = *It;
		Sentence->m_ClauseNum++;	
		
		CToken*  LastToken = m_TokenList[Clause->m_BeginPos+Clause->m_TokenNum-1];
		if(LastToken->IsSentenceFlag()){
	
		    uint32 NextPos = Sentence->m_BeginPos+Sentence->m_ClauseNum;
			if(AlterClausePos < NextPos){
			//	AlterClausePos = min(AlterClausePos,NextPos-1);
				AddInfectedText(Sentence,AlterClausePos);
			}
			Complete = true;
			++CurSentenceNum;
REDO:
			if(CurSentenceNum < m_SentenceList.size()){
				CSentence* NextSentence = m_SentenceList[CurSentenceNum];								
			//	012345678   
			//	|---|-- |
				if(NextPos<NextSentence->m_BeginPos){//小于紧邻的Token则需要填补一个子句
                    Sentence = InsertSentence(CurSentenceNum,NextPos);
					AlterClausePos = NextPos;
					Complete = false;
				}
				else if(NextPos == NextSentence->m_BeginPos)return;
				else if(NextPos < (NextSentence->m_BeginPos+NextSentence->m_ClauseNum)){ //如果占用了下一个子句的部分
					NextSentence->m_ClauseNum  = NextSentence->m_BeginPos+NextSentence->m_ClauseNum-NextPos;
                    NextSentence->m_BeginPos   = NextPos;
					AddInfectedText(Sentence,0);
					return;			
				}
				else{
					DeleteSentence(CurSentenceNum);
					goto REDO;
				}
			}
			else{ //没有下一个句子
				if(NextPos<m_ClauseList.size()){//但还有剩下的子句没有分配完
					Sentence = InsertSentence(CurSentenceNum,NextPos);
					AlterClausePos = NextPos;
					Complete = false;
				}
				else return;
			} 
		}
		It++;
	}
	if(!Complete){
		if(AlterClausePos>=(Sentence->m_BeginPos+Sentence->m_ClauseNum))return; //define int32 4,5 当删除5之后导致AlterClausePos的子句已经不存在
		AddInfectedText(Sentence,AlterClausePos);
	}
}

/*段落标准标准：
	
*/
int32  CText::SplitParagraph(uint32 BeginPos){
	deque<CSentence*>::iterator It = m_SentenceList.begin();
	It += BeginPos;
	while(It < m_SentenceList.end()){
			return 0;
	}
	return 0;
}


void CText::Analyse(uint32 Pos,TCHAR ch){
	assert(Pos>=0);
	m_InfectedTokenList.clear();

	TCHAR tempChar = _totlower(ch);

    CToken*     Token    = FindToken(Pos);
	
	//得到所属子句位置，因为在插入或删除token是需要调整之后子句的BeginPos
	FindClause(m_CurTokenSelect);

	int32 n = 1;
	if(ch==127/*VK_DELETE*/)n = -1;

	ChangeTokenBeginPos(m_CurTokenSelect,n);
    
	bool Continue = false; //插入字符如果导致Text没有任何改变（比如空格）则不再继续处理
	int32 OppPos  = Pos - Token->m_BeginPos;
	if(_istalnum(tempChar))
		Continue = InsertAlpha(Token,OppPos,ch);
	else if(_istspace(tempChar))
		Continue = InsertSpace(Token,OppPos,ch);
	else if(tempChar == 127 /*VK_DELETE*/) //必须在标点之前处理

		Continue = InsertBack(Token,OppPos);
	else if(tempChar == _T(':') || tempChar == _T('：') ||tempChar == _T('@'))
		Continue = InsertAlpha(Token,OppPos,ch);
	else if(_istpunct(tempChar))
		Continue = InsertPunct(Token,OppPos,ch);
	else  if(  tempChar == _T('\r') || tempChar == _T('\n'))//暂时如此
	    Continue = InsertNewLineFlag(Token,OppPos,ch);  
	else  
		Continue = InsertAlpha(Token,OppPos,ch);

	if(!Continue)return;		
	if(m_TokenList.size()==0){
		deque<CClause*>::iterator ClauseIt = m_ClauseList.begin();
		deque<CSentence*>::iterator SentenceIt = m_SentenceList.begin();
		
		while(ClauseIt<m_ClauseList.end()){ 
			CClause* Clause = (*ClauseIt);
			delete Clause;
			ClauseIt++;
		};
		m_ClauseList.clear();
		
		while(SentenceIt<m_SentenceList.end()){ 
			CSentence* Sentence = (*SentenceIt);
			delete Sentence;
			SentenceIt++;
		};
		m_SentenceList.clear();

		return;
	}

	SplitClause();
	SplitSentence();

}

tstring CText::PrintAnlyseResult(){
	
#ifdef _UNICODE
	wstringstream ss;
#else
	stringstream  ss;
#endif
	
	ss<<_T("Anlyse Result:    (")<<m_SentenceList.size()<<_T(" Sentence)\n");
	for(uint32 i=0; i<m_SentenceList.size(); i++){ 
		CSentence* Sentence  = m_SentenceList[i];
		ss<<_T("S")<<i;
		for(uint32 j=0;j<Sentence->m_ClauseNum; j++){
			CClause* Clause = m_ClauseList[Sentence->m_BeginPos+j];
			ss<<_T("\t--> C")<<j<<_T(": ");
			for(uint32 k=0; k<Clause->m_TokenNum; k++){
				CToken* Token = m_TokenList[Clause->m_BeginPos+k];
				ss<<Token->m_Str<<_T(" ");
			}
			ss<<_T('\n');
		}
	};
	return ss.str();
	//m_ParentThread->RuntimeOutput(ss.str());
}

tstring CText::PrintThinkResult(){
	
#ifdef _UNICODE
	wstringstream ss;
#else 
	stringstream  ss;
#endif
	
	ss<<_T("Think Result:    (")<<m_SentenceList.size()<<_T(" Sentence)\n");
	for(uint32 i=0; i<m_SentenceList.size(); i++){ 
		CSentence* Sentence  = m_SentenceList[i];
		ss<<_T("S")<<i;
		if(Sentence->IsOK()){
			ss<<_T("\t--> OK")<<endl;
			continue;
		}
		for(uint32 j=0;j<Sentence->m_ClauseNum; j++){
			CClause* Clause = m_ClauseList[Sentence->m_BeginPos+j];
			if(!Clause->IsOK()){
				if(Clause->m_MemoryID>=0 && Clause->m_MemoryID<LAST_ERROR){
					const TCHAR* ErrorText = ANALYSE_ERROR[Clause->m_MemoryID];
					ss<<_T("\t--> C")<<j<<_T("  ERROR: ")<<ErrorText<<endl;
				}else{
					ss<<_T("\t--> C")<<j<<_T("  ERROR: unkown clause")<<endl;
				}
			}
			else ss<<_T("\t--> C")<<j<<_T("  OK")<<endl;
		}
	};
	return ss.str();
	//m_ParentThread->RuntimeOutput(ss.str());
}

CSentence* CText::GetSentence(uint32 index){
	assert(index<m_SentenceList.size());
	CSentence* Sentence = m_SentenceList[index];
    return Sentence;
}

tstring CText::GetSentenceText(uint32 index){
	assert(index<m_SentenceList.size());
	CSentence* Sentence = m_SentenceList[index];
	CClause* BeginClause = m_ClauseList[Sentence->m_BeginPos];
	CClause* EndClause   = m_ClauseList[Sentence->m_BeginPos+Sentence->m_ClauseNum-1];
    
	int32 BeginToken = BeginClause->m_BeginPos;
	int32 EndToken  = EndClause->m_BeginPos+EndClause->m_TokenNum;
	
	CToken* Token = m_TokenList[BeginToken];
	int BeginCharPos = Token->m_BeginPos;
    Token = m_TokenList[EndToken-1];
	int len = Token->m_BeginPos+Token->Size() - BeginCharPos;
	tstring text(len,_T(' '));
	
	for(int i= BeginToken; i<EndToken; i++){
		Token = m_TokenList[i];
		text.insert(Token->m_BeginPos-BeginCharPos,Token->m_Str.c_str());
	}
	return text;
}
 
tstring CText::GetText(){
	if(m_TokenList.size()==0)return _T("");
	CToken* Token = m_TokenList.back();
	int len = Token->m_BeginPos+Token->Size();
	tstring text(len,_T(' '));
	deque<CToken*>::iterator It = m_TokenList.begin();
	while(It<m_TokenList.end()){
		Token = *It;
		text.insert(Token->m_BeginPos,Token->m_Str.c_str());
		It++;
	}
	return text;
}
	
	