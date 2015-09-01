/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/



#ifndef _TEXTANALYSE_H__
#define _TEXTANALYSE_H__


#include "AbstractSpace.h"
#include "Element.h"


#define  ERROR_0          0 //"input not completed"
#define  ERROR_1          1 //ok
#define  ERROR_2          2 //"Cannot understand this word or clause"
#define  ERROR_3          3 //"The param incorrect";
#define  ERROR_4          4 //"The param is not properly formatted";
#define  ERROR_5          5 //"The param must be a num.";
#define  ERROR_6          6 //"The param must be an integer";
#define  ERROR_7          7 //"The param  must be > 0 ";
#define  ERROR_8          8 //"name unavailable "
#define  ERROR_9          9 //"This command is not a valid format"
#define  ERROR_10        10 //"Global or Local Logic name unavailable "    
#define  ERROR_11        11 //"Relative can't be first word of sentence,will be ignore"
#define  ERROR_12        12 //"Cannot make shunt logic relation with SET LABEL or GOTO LABEL command "
#define  ERROR_13        13 //"Cannot combine outer Action and inter Action in one logic"
#define  ERROR_14        14 //"Lost the temp logic or the temp logic not matching action type with current action"
#define  ERROR_15        15 //"Use nest think error!"
#define  ERROR_16        16 //"Use nest action error!"
#define  ERROR_17        17 //"Cannot understand this clause"
#define  ERROR_18        18 //"The param must be a reference"
#define  ERROR_19        19 //"The current temp object invalid"      
#define  ERROR_20        20 //"Param error" 
#define  ERROR_21        21 //"The element be referenced not exist"
#define  ERROR_22        22 //"The  name repeat"
#define  ERROR_23        23 //"Lose reference name when reference element"
#define  ERROR_24        24 //"Focus object error" 
#define  ERROR_25        25 //"Name not unique" 

#define  LAST_ERROR      26

static const TCHAR* ANALYSE_ERROR[] ={
	    _T("input not completed;"),  //input
		_T(""),  //ok
		_T("cannot understand the word or clause;"),
		_T("the param  num  incorrect;"),
		_T("the param  format incorrect;"),
		_T("The param not is a num;"),
		_T("The param must be a int num;"),
		_T("The param  must be > 0 ;"),
		_T("Name unavailable ;"),
		_T("the format incorrect use logic command;"),
		_T("TempLogic name unavailable ;"),    
		_T("Relative can't be first word of sentence,will be ignore;"),
		_T("Cannot make shunt logict relation with SET LABEL or GOTO LABEL command; "),
		_T("cannot combine OuterAction and InterAction in one logic;"),
		_T("Lost the Templogic or the type of templogic not matching with current action;"),
		_T("use nest think error!"),
		_T("use nest action error!"),
		_T("cannot understand this clause;"),
		_T("the param must be a reference;"),
		_T("the current temp object unvalid;"),
		_T("param error;"),
		_T("the element be referenced not exist;"),
		_T("the name repeat;"),
		_T("lose reference name when reference element or logic;"),
		_T("Focus object error;"),
		_T("Name not unique;")
};

enum TOKENTYPE{
	COMMON,         //字母+数字
	NUM,
	ADJECTIVE,
	PUNCT, 
	NEWLINE         ///r或/n
};  

#define  INPUT            0
#define  OK               1

class CToken{
public:
	uint32        m_BeginPos;    //相对于所属Clause里坐标位置
	TOKENTYPE     m_Type;
    tstring       m_Str;
	
	/*对于Common类型的Token，必须>0xFFFF表示正确，等于token的形ID，
	  否则>0 但 < 0xFFFF则表示为特别信息ID,主要是提示错误
	*/
	int64		 m_MemoryID;    
	CToken(TOKENTYPE type = COMMON):m_BeginPos(0),m_MemoryID(0),m_Type(type){};
    ~CToken(){}; 
	
	bool  IsOK(){ 
		return (m_Type==COMMON && m_MemoryID>0xFFFF) || m_MemoryID==OK;
	} 
	uint32 Size(){
		return m_Str.size();
	};  
	bool  isPunct(){ 
		return m_Type ==PUNCT;
	};
	bool  IsReference(){ 
		return m_Type == PUNCT && m_Str[0]==_T('\"');
	}
	bool  IsClauseFlag(){ 
		return m_Type == PUNCT && 
			   (
			    m_Str[0]==_T(',') || 
			    m_Str[0]==_T(';') || 
				m_Str[0]==_T('，')||
				m_Str[0]==_T('；')
			   );
	}
	bool  IsSentenceFlag(){
		return m_Type == PUNCT &&
			  (
			   m_Str[0]==_T('；') ||
			   m_Str[0]==_T(';') 
			  );
	}
};


class CClause{
public:	
	uint32           m_BeginPos;       //第一个Token位置
	uint32           m_TokenNum;       //本子句包含多少个token
    uint32    		 m_ActionType;
    //=0 表示还没有分析 <0xFFFF 为错误ID 
	int64            m_MemoryID;	 
	Energy*          m_Param;        //TokenList中作为参数的token数。
	LogicRelation    m_LogicRelation;

	CClause():m_BeginPos(0),m_TokenNum(0),m_MemoryID(0),m_Param(NULL),m_LogicRelation(UNKOWN_RELATION),m_ActionType(0){};
    ~CClause(){
		if(m_Param)delete m_Param;
	}

	bool  IsOK(){ return m_MemoryID>0xFFFF;};
	bool  IsBlank(){ return m_TokenNum==0;};
};

//保存嵌套THINK得到的临时逻辑
//指示行为逻辑的属性
enum ACTIONSTATE{
	COMMON_ACTION   =    0x0001,      //抽象命令，既可以和外部也可以和内部组合执行
	OUTER_ACTION    =    0x0002,      //需要依赖外部资源执行
	INTER_ACTION    =    0x0004,      //不意外外部资源执行的命令
	INDE_INTER_ACTION  = 0x0008,      //独立内部行为，一次只能执行一个
	EXE_COMPLETED   =    0x0010
};

class CSentence{
public:
    bool			       m_IsError; //只有所含子句或Token有任何错误都会让m_Error = true;
	uint32    		   	   m_State;

	//相对位置
	uint32                 m_BeginPos;  //第一个子句位置
	uint32                 m_ClauseNum;
  
	ePipeline              m_AnalyseResult;
 
	CSentence():m_BeginPos(0),m_ClauseNum(0),m_State(COMMON_ACTION),m_IsError(true){};
    ~CSentence(){
	}
	bool  IsOK(){ return !m_IsError;};
	bool  IsBlank(){ return m_ClauseNum==0;};	
};
	
enum TextType{NULL_TEXT,TOKEN, CLAUSE,SENTENCE};

struct InfectedText{
	TextType  Type;
	int32     AlterPos;
	void*     Text;
	InfectedText(CToken* T, int32 P):Type(TOKEN),Text(T),AlterPos(P){};
	InfectedText(CClause* C, int32 P):Type(CLAUSE),Text(C),AlterPos(P){};
	InfectedText(CSentence* S, int32 P):Type(SENTENCE),Text(S),AlterPos(P){};
	InfectedText(const InfectedText& In){
		Type     = In.Type;
		AlterPos = In.AlterPos;
		Text     = In.Text;
	}		
	InfectedText& operator=(const InfectedText& In ){
		if(this != &In){
			Type     = In.Type;
			AlterPos = In.AlterPos;
			Text     = In.Text;
		}
		return *this;
	}
};



class CLogicThread;

class CText  
{
public:

	/*仅仅在delete token/clause/sentence时需要同时取消Thread里的m_ForecastText
	  将来或许应该优化一下
	*/
	CLogicThread*    m_ParentThread; 

	//按顺序保存输入的多个句子
	deque<CToken*>         m_TokenList;    
	deque<CClause*>        m_ClauseList;
    deque<CSentence*>      m_SentenceList; 

	
	//分析信息时产生的临时变量
    uint32                  m_CurTokenSelect;
    uint32                  m_CurClauseSelect; 
	uint32                  m_CurSentenceSelect;
	

	deque<InfectedText>    m_InfectedTokenList;
    deque<InfectedText>    m_InfectedClauseList;
    deque<InfectedText>    m_InfectedSentenceList;

public:
	void  AddInfectedText(CToken* Token,uint32 AlterPos);
	void  AddInfectedText(CClause* Clause,uint32 AlterPos);
	void  AddInfectedText(CSentence* Sentence,uint32 AlterPos);

	//注意：Pos是字符位置，不是Token在m_TokenList的Index
	CToken*    FindToken(uint32 Pos);
    CClause*   FindClause(uint32 Pos);
    CSentence* FindSentence(uint32 Pos);
	
	//插入
	CToken*    InsertToken(uint32 Index,tstring Token,uint32 BeginPos,TOKENTYPE Type, int32 TokenState);
	CToken*    InsertToken(uint32 Index,TCHAR ch,uint32 BeginPos,TOKENTYPE Type, int32 TokenState)
	{
		tstring Token; Token = ch;
		return InsertToken(Index,Token,BeginPos,Type,TokenState);
	}
	CClause*   InsertClause(uint32 Index,uint32 BeginPos);
	CSentence* InsertSentence(uint32 Index,uint32 BeginPos);

	//删除
	void  DeleteToken(uint32 Index);
	void  DeleteClause(uint32 Index);
	void  DeleteSentence(uint32 Index);
	
	//更改位置
	void  ChangeTokenBeginPos(uint32 TokenPos,uint32 len); 
	void  ChangeClauseBeginPos(uint32 ClausePos,uint32 len); 
	void  ChangeSentenceBeginPos(uint32 SentencePos,uint32 len);

	//组装Token
    bool  InsertPunct(CToken* CurToken,uint32 Pos,TCHAR ch);
	bool  InsertSpace(CToken* CurToken,uint32 Pos,TCHAR ch);
    bool  InsertAlpha(CToken* CurToken,uint32 Pos,TCHAR ch);
    bool  InsertNewLineFlag(CToken* CurToken,uint32 Pos,TCHAR ch);
    bool  InsertBack(CToken* CurToken,uint32 Pos);	
	void  SplitClause();
	void  SplitSentence();
	int32 SplitParagraph(uint32 BeginPos); //目前没用
public:
	CText();
	virtual ~CText();
    
	//注意：每次分析会把结果保存在InfectedTextList里，下次分析会丢失上次结果
	void Analyse(uint32 CharPos,TCHAR ch);

	bool IsComplete();

	//每次重新分析前都应该调用，清空上次TEXT分析的痕迹
    void   Clear();
	
	CSentence* GetSentence(uint32 index);

	tstring GetSentenceText(uint32 index);
	tstring GetText(); //把分析好token再转换成text;

	tstring PrintAnlyseResult();

	//把当前的理解结果打印出来，用来检查输入与理解是否正确匹配
	tstring PrintThinkResult();
};


#endif // _TEXTANALYSE_H__
