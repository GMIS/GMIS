/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LOGICTHREAD_H__
#define _LOGICTHREAD_H__


#pragma warning(disable: 4786)

#include "PhysicSpace.h"
#include <set>
#include <deque>
#include "Brain.h"
#include "BrainTask.h"
#include "BrainMemory.h"
#include "InstinctDefine.h"
#include "TextAnalyse.h"

/* 根据输入的文字信息线索，实时找到对应的记忆
系统允许与每一个对话者分别对应一个线索分析。
*/

class  CWord;

enum NumType{ 
	NOT_NUM = 0,
	INT_NUM,
	FLOAT_NUM
};

NumType IsNum(tstring& Text);



enum FindTypeExpected {FIND_ALL,FIND_OBJECT,FIND_PEOPLE,FIND_TEXT,FIND_LOGIC, FIND_COMMAND}; 

//逻辑条目的非显示版
class CLocalLogicCell{
public:
	CBrainTask           m_Task;
    tstring              m_LogicMemo;
	map<tstring,tstring> m_RefList;         //<引用名，引用者>

public:	
	CLocalLogicCell(){};
	CLocalLogicCell(CBrainTask& Task);
	virtual ~CLocalLogicCell();
	void AddRef(const tstring& refName,CLocalLogicCell* WhoRef){ 
		m_RefList[refName]=WhoRef->LogicName();
	};
	void DelRef(const tstring& refName){
		m_RefList.erase(refName);
	};
	tstring LogicName(){
		return m_Task.m_Name;
	}
	int32 RefNum(){ return m_RefList.size();}	
	bool  IsValid(){ return m_Task.IsValid();}
	
	ePipeline* GetLogicItem(); //转换成逻辑条目显示版
	void GetRefList(ePipeline& List);

	Energy*  ToEnergy(); 
	bool     FromEnergy(Energy* E);

};

class CElementCell{
public:	
	tstring				 m_Name;
	tstring				 m_CreateBy;
	map<tstring,tstring> m_RefList;         //<引用名，引用者>
public:
	CElementCell(tstring Name, tstring CreateBy);
	virtual ~CElementCell(){};
	
	void AddRef(const tstring& refName,CLocalLogicCell* WhoRef){ 
		m_RefList[refName]=WhoRef->LogicName();
	};
	void DelRef(const tstring& refName){
		m_RefList.erase(refName);
	};
	
	ePipeline* GetElementItem();
	void GetRefList(ePipeline& List);

	
	Energy*  ToEnergy(); 
	bool     FromEnergy(Energy* E);

};


class CTaskDialog;

class CLogicThread :public CBrainMemory
{

	CUserMutex               m_Mutex;

public:
        
	int64                    m_ID;
    int64                    m_UserDialogID;

	tstring                  m_LastError;
public:
	CLogicThread();
	CLogicThread(int64 ThreadID,int64 UserDialogID);
	virtual ~CLogicThread();
    
	void Reset(int64 ThreadID,int64 UserDialogID);

	//[idle/run/think/debug]task name	
    tstring GetStateText();

    tstring GetLastError(){return m_LastError;};

	Energy*  ToEnergy(); 
	bool     FromEnergy(Energy* E);

//分析信息
///////////////////////////////////////////////
public: 

	CText  m_Text;		

	int64                 m_LastMsgTimeStamp; //确保接收到的字符在多线程环境下是按用户输入的顺序理解
	map<int64,ePipeline>  m_TextMsgQueue;  //用户输入的文字信息在处理之前暂存为有序队列

	int32  m_LogicPos;


	void ClearAnalyse(); 

	//以下理解分割好的token\clause\sentence时所产生的临时逻辑场景信息
	struct TokenLogicSense{
		CToken*             BelongToken;
		int32               NextCharPos;   //指出下一个字符应该出现的位置
		int64               LastCharID;    //最后一个正确理解字符得到的ID(作为预测线索)
		TokenLogicSense():BelongToken(NULL),NextCharPos(0),LastCharID(0){};
	};
	TokenLogicSense             m_TokenSense;
	
	struct MeaningPos{
		int64 RoomID;
		int64 ParamTokenPos;
	};
	struct ClauseLogicSense{
		CClause*                BelongClause;    //以下数据属于当前那个子句
		int32                   NextTokenPos;    //指出当前正确分析到子句的那个token  
		bool                    FirstWord;       //指出当前token是否算本子句的第一个token（不算and和then）
		map<int64,MeaningPos>   ClauseMeaning;   //当前子句理解到的行为表 〈RoomValue,MeaningPos〉
		vector<int64>           FatherList;      //暂存token之间修饰得到的形ID
		ClauseLogicSense():BelongClause(NULL),NextTokenPos(0),FirstWord(true){};
	};

    ClauseLogicSense        m_ClauseSense;  //保存最后一个正常输入子句的理解结果

	struct SentenceLogicSense{
		CSentence*		BelongSentence;
		int32           NextClausePos;
		vector<int64>	FatherList;             //暂存子句之间修饰得到的形ID  
		SentenceLogicSense():BelongSentence(NULL),NextClausePos(0){};
	};

	SentenceLogicSense     m_SentenceSense;

	//具体执行每一个受影响的语法成分
	void Think(CTaskDialog* Dialog,CToken* Token, int32 AlterPos);
    void Think(CTaskDialog* Dialog,CClause* Clause, int32 AlterPos);
	void Think(CTaskDialog* Dialog,CSentence* Sentence, int32 AlterPos);

    //执行所有受影响的语法成分
	void ThinkAllAffectedToken(CTaskDialog* Dialog);
    void ThinkAllAffectedClause(CTaskDialog* Dialog);
	void ThinkAllAffectedSentence(CTaskDialog* Dialog);

	void ThinkProc(CTaskDialog* Dialog,CMsg& Msg);
    void ThinkProc(CTaskDialog* Dialog, int32 Pos,tstring& Msg,bool Forecast,int64 EventID=0);	
	
	int32 GetActionRoom(int64 ParentID,ClauseLogicSense* cls);

	//在当前输入中检查InstinctID的名字是否为已经有Name
	//如果IsRef=true只检查clause之前的，否则在整个输入中检查所有不属于Clause的
	bool CheckNameInInputed(int64 InstinctID,tstring Name,CClause* Clause,bool IsRef=false);

	/*
	bool CheckFocusObjectIsValid();
    bool ObjectNameIsExisted(CTaskDialog* Dialog,tstring Name);

	bool CheckFocusMemoryIsValid();
	*/

	//根据本能ID,检查参数的合法性,顺便生成可直接传递的参数, 错误返回false
	bool CheckInstinctParam(CTaskDialog* Dialog,CClause* Clause, vector<tstring>& ParamList);

	//冲突返回TRUE，否则FALSE
	bool CheckActionConflict(CSentence* Sentence);
	
    /*错误标记原则
	  当一个单词输入完毕以后，被分析，可能出现错误，但是单个错误
	  放到子句总去理解又可能是正确的，因此此单词应该被存疑（m_MomeryID = ERROR_2)，

      当一个子句的所有单词输入完毕，首先标记整个子句为正常，然后逐个分析每个单词，
	  此时如果发现某个单词错误则标记为错误，用法不规范则警告，如果所有单词正确，但
	  子句不能被理解则标记整个子句为错误
	*/
	
	/*任何信号分析完毕后都应该给对方一个反馈，反馈信息用对方发来的ePipeline包装，
	  第一个数据为反馈ID，其他内容则根据具体的内容而定
	*/
	void NotifyTokenError(CTaskDialog* Dialog,CToken* Token,uint32 ErrorID);
	void NotifyClauseError(CTaskDialog* Dialog,CClause* Clause,uint32 ErrorID);
	void AnalyseMsg(int64 RoomID,const char* Comment=NULL);

	//检查输入的逻辑是否可以执行
	bool CanBeExecute(CTaskDialog* Dialog);

	//如果可以执行，则给出第一条逻辑，
	ePipeline* GetOneLogic();

	tstring GetUserInput();

	int64  SelectMeaning(ClauseLogicSense* cls);

//处理联想
/////////////////////////////////////////////////////////

public: 
    friend class CText;
	//处理联想，即预测下一步
    TextType                m_ForecastType;  //针对谁展开联想
	void*                   m_ForecastText;

	void  SetForecast(CTaskDialog* Dialog,TextType Type,void* Text);

	struct  ForecastRoom{
		ForecastRoom*                      Parent;         //父空间
		int64                              ID;             //本空间是识别ID；
        tstring							   RoomText;       //回取本空间(不含父空间部分）所得的文本
        map<int64,ForecastRoom*>::iterator It;             //指出已经递归预测那个子空间
        map<int64,ForecastRoom*>           ChildList;      //找到的非意义子空间，用来做进一步预测 <意义值,ForecastRoom>
		vector<int64>					   MeanlingList;   //已经找到的意义，需要翻译成文本输出
		

		ForecastRoom():Parent(NULL),ID(0){};
		ForecastRoom(ForecastRoom* pParent, int64 RoomID):Parent(pParent),ID(RoomID){};
		~ForecastRoom(){ //递归解构
			map<int64,ForecastRoom*>::iterator It = ChildList.begin();
            while(It != ChildList.end()){
				delete It->second;
				It->second = NULL;
				It++;
			}
		}
	};
     
	int32                   m_ForecastDepth; //default = 20, 应该可以改变
	ForecastRoom*           m_ForecastRoom; 
	
	void  GetAllChild(ForecastRoom* ParentRoom);
	ForecastRoom* GetCurForecastRoom(ForecastRoom* Parent);
	void  Forecast(CTaskDialog* Dialog,CToken* Token);
	void  Forecast(CClause* Clause);
	void  Forecast(CSentence* Sentence);	
public:
	bool  ForecastOK(){ return m_ForecastRoom && m_ForecastRoom->ChildList.size();};
	/*根据空间值得到空间识别ID,没有找到对应的子空间返回0
	  如果找到，
	*/
	int64 GetForecastResult(int64 RoomValue); 
	 void ExecuteForecast();


//内部本能
//////////////////////////////////////////////////////////////////////////
public:
	tstring       m_CurLogicName;

	//向WORLD请求得到address的object data，然后以SaveName文件名存储到temp dir
	bool         LoadObject(SpaceAddress& Address);
	//向传递Executer进程FileName和Pipe，执行结果返回到Pipe
	bool         ExecuteObject(CUseObject* Ob,tstring FileName,ePipeline* Pipe);
	//通知Executer删除所有含有此Task的所有外部Object
	void         DeleteObject(int64 ExecuterID);
 

//////////////////////////////////////////////////////////////////////////
public: 

	bool   QueryFind(){return false;};

//处理聊天
//////////////////////////////////////////
	void ExecuteChat();


//处理学习
/////////////////////////////////////////////////////////////////
public:
	    	
	/*
	  记忆一个字符或token
	  分解输入的token为字符，嵌套存储，只记忆COMMON和OTHER_FALG类型,NUM按COMMON处理
	  对于REFERENCE和SINGLE_REF需要嵌套分析，先调用LearnText()存储，然后按按引用词性存储
	  Meaing确定了嵌套字符空间后意义空间的逻辑明文，缺省=NULL_MEANING，表示死记硬背
	  MeaningSense则决定在意义空间里生成何种结尾空间, ==0则不做处理
	  ReturnID指定生成意义空间时的逻辑空间ID，如果=0则由当时时间戳代替
	  返回这个意义空间的ID
    */ 
    int64 LearnToken(CToken* Token,int64 Meaning=NULL_MEANING, int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0);    
	
	/*
	  存储clause
	  只按逗号分割，不考虑语法，分解成token嵌套存储
	  逻辑明文：LearnToken返回的空间ID or root
      逻辑类型：TYPE_LEARN_TOKEN
	  返回最后空间ID
	*/
	int64 LearnClause(CClause* Clause,int64 Meaning=NULL_MEANING, int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0);

	/*
	  存储Sentence
	  按分号或句号分割的句子，分解成子句嵌套存储
      逻辑明文： LearnClause返回的空间ID or ROOT
	  逻辑类型： TYPE_LEARN_CLAUSE
	*/
    int64 LearnSentence(CSentence* Sentence,int64 Meaning=NULL_MEANING, int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0);

	/*存储段落
	  有时会把一个文本分成几段，嵌套存储
      逻辑明文：LearnSentence返回的空间ID or Root
	  逻辑类型：TYPE_LEARN_SENTENCE

      BeginPos为m_SentenceList的位置
	*/
    int64 LearnParagraph(int32 BeginPos,int32 SentenceNum,int64 Meaning=NULL_MEANING, int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0);
		
	CToken*    JustOneToken(); //仅含一个token且type = COMMON,不是返回NULL
	CClause*   JustOneClause(); //仅含一个Clause
	CSentence* JustOneSentence();

public:
	/*通用文字记忆操作，只适用与死记硬背
	  如果text只是一个token则返回token的记忆ID，
	  如果是子句则返回子句的ID，依次类推，
	*/
    int64 LearnText(int64 Meaning=NULL_MEANING, int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0);
  
	int64 LearnText(tstring& Text,int64 Meaning=NULL_MEANING, int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0);
	
	//根据词性检查语法，确定每一个单词的实际词性，检查是否符合一个抽象行为短语
	//如果符合把Clause转换成WordList,并返回true
	bool SyntaxCheck(CClause* Clause,list<CWord>& WordList);



	//学习单词的词性,返回学习后的记忆ID，HasbeenLearned非零表示此单词的形（token)已经被记忆过，
	////////////////////////////////////////////////////
	int64 LearnWord(tstring Text,int64 PartOfSpeech,int64 MeaningSense=MEANING_SENSE_OK,int64 ReturnID = 0,bool IsCheck=false);
    int64 CheckWord(tstring Text,int64 PartOfSpeech,int64 MeaningSense=MEANING_SENSE_OK){
		return LearnWord(Text,PartOfSpeech,MeaningSense,0,true);
	}
	int64 LearnWordFromWord(tstring NewWord,int64 PartOfSpeech,tstring OldWord);

	/*用Comment描述记忆存储ID=MemoryID,即
	  在Comment的形空间下生成一个空间值=MemoryID的意义空间
  	*/
	int64 CommentMemory(tstring& Comment, int64 MemoryID){
		return  LearnText(Comment,MemoryID);
	}

	//Note: crc32将作为意义空间的空间值
    int64 LearnObject(int64 IP,SpaceAddress& Address,int64 crc32,tstring Comment,bool IsPeople = false);

	//use object时Robot会自动记忆
    int64 LearnObjectAuto(int64 IP,tstring AddressText,int64 crc32,bool IsPeople = false);

	

	/* Command为行为的抽象描述，比如"define int32"，必须通过语法检查，
	   存储相应词性的单词，然后再让其意义空间值=ActionID
	   ActionID如果不是预定义的本能，就是行为逻辑的存储ID
	*/
	int64 LearnAction(CTaskDialog* Dialog,tstring& Command,int64 ActionID,int64 MeaningSense = MEANING_SENSE_OK,bool IsCheck = false);

	//和前一个相比，Command已经被分析
	int64 LearnAction(CClause* Clause,int64 ActionID,tstring& Error,int64 MeaningSense = MEANING_SENSE_OK,bool IsCheck=false);
    
	int64 CheckAction(CTaskDialog* Dialog,tstring& Command,int64 ActionID,int64 MeaningSense = MEANING_SENSE_OK ){
		return LearnAction(Dialog,Command,ActionID,MeaningSense,true);
	}

	/*存储具体的行为，即带有参数，比如："define int32 234"
       格式:
	   int64  本能存储ID（实际上是行为命令的形ID）
       int64  宾语存储ID
	   注意 ：内部状态本能不可以转化成具体的行为存储。
	*/
	int64 LearnInstinctInstance(int64 InstinctID,Energy* Param);	

    int64 LearnLogic(ePipeline* LogicPipe);
//z    int64 LearnLogic(CGlobalLogicItem* Logic);

	int64  LearnEnergy(Energy* E);
	int64  LearnPipe(ePipeline& Pipe);

	int64  LearnToken(tstring& s);


	
//其他
/////////////////////////////////////////////////////////
public: 
	//仅仅表示分析完毕    
	bool IsEnd(){ 
		return m_Text.m_SentenceList.size() == 0;
	};

	//用户双击Edit控件的某个字符，将查询这个字符所在Token或子句的错误。
    tstring  GetErrorText(int32 CharPos);
	
  	//错误处理
	bool                   m_InErrorState;

/*
public:// 仅用于测试
	struct _InputInfo{
		int32 pos;
		char  ch;
		_InputInfo(int32 p,char c):pos(p),ch(c){}
	};	
    tstring  m_TestText;            
	int32   m_AnlayseCount;        //待分析总次数
    int32   m_CompletedCount;      //已经完成的分析总数

    //对于一个字符串，无论字符以何种顺序插入，最终都能得到正确的理解结果。
	void TestAnalyse(CProgressThreadDlg* Control);
    void TestDel(CProgressThreadDlg* Control); 

//    static ULONG TestProcess(void* pData, CProgressThreadDlg* pProgressDlg);
	void AnlyseRecursive(tstring SelectFlag,vector<_InputInfo>& SelectResult,int32 NestNum,CProgressThreadDlg* Control);
*/
};
//具体执行测试并显示进度
//ULONG ExecuteAnlayseTest(void* pData, CProgressThreadDlg* pProgressDlg);

class  CWord{
public:
	tstring            m_Text;      
	int64             m_ID;           //记忆ID

	uint32            m_AllPart;      //所有候选词性	
	int64             m_PartOfSpeech; //已选定词性
	
	CWord(CToken& Token)
		:m_Text(Token.m_Str),m_ID(Token.m_MemoryID),m_AllPart(0),m_PartOfSpeech(Token.m_Type){};
	CWord():m_ID(0),m_PartOfSpeech(0),m_AllPart(0){};
    ~CWord(){
	};
    	
	CWord& operator=(const CWord& w ){
		if(this != &w){
			m_Text = w.m_Text;
			m_ID = w.m_ID;
			m_PartOfSpeech = w.m_PartOfSpeech;
			m_AllPart = w.m_AllPart;
		}
		return *this;
	}
    void  AddPartOfSpeech(int64 Part){
		  assert(IsPartOfSpeech(Part));
          m_AllPart |= (uint32)((Part-PARTOFSPEECH_START));  		
	}
	uint32  HasPartOfSpeech(int64 Part){
		  assert(IsPartOfSpeech(Part));
          return m_AllPart & (uint32)((Part-PARTOFSPEECH_START));  
	}
};


#endif // _LOGICTHREAD_H__
