/*
*对大脑数据的一些基本操作，分4个层次：
*  1 数据库操作 
*  2 空间操作
*  3 记忆存取
*  4 常用记忆的初始化
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _CBASEMEMORY_H__
#define _CBASEMEMORY_H__



#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include "InstinctDefine.h"
#include "UserTimer.h"
#include "Sqlite\CppSQLite3.h"
#include "UserMutex.h"
#include "space.h"

using namespace std;

//NOTE: the minimum unit of timestamp is hundred nanosecond 
#define TIME_SEC    10*1000*1000  //1 second = 1 hundred nanosecond * TIME_SEC
#define TIME_MILlI  10*1000       //1 millisecond = 1 hundred nanosecond * TIME_MILlI

#define MIN_EVENT_INTERVAL 10*1000*1  //the minimum event interval = 1 millisecond


/*定义系统配置表名，位置：MyBrain.db
    用来存储诸如地图、好友名单等虚拟人的启动必须的数据
	表的结构为：项目名  项目数据
	其中项目数据采用XML格式，考虑加密处理

  note: 在生成SQL语句时，当数字做表名，表项时，需要使用双引号，
         如 select "127282417795627563" from “127282417795627560”
*/




#define SYSTEM_CONFIG  "127282417795627500"   
#define CONFIG_ID      "127282417795627501"
#define CONFIG_DATA    "127282417795627502"

//所有的配置ID将以CONFIG为基准降序
#define SYSTEM_MAP     "127282417795627499"



//可以看出左右脑的数据结构是一样的,
//所以这里给它们相同的条目
#define ITEM_A   "a"
#define ITEM_B   "b"
#define ITEM_C   "c"

/*定义空间索引，位置：MyLBrain.db
                             
		表名：欲查找的逻辑空间类型
	   ------------------------------------------------
	  | 父逻辑空间ID | 父逻辑空间意义ID | 子逻辑空间ID |
	   ------------------------------------------------
	  |  64 INT      |     64   INT     |    64 INT    |               
       ------------------------------------------------
*/

#define LB_FATHER_ID        ITEM_A
#define LB_FATHER_VALUE     ITEM_B
#define LB_CHILD_ID         ITEM_C


/*定义逻辑空间，位置：MyRBrain.db
	 表名：动态产生
		  -----------------------------------------
		 |子逻辑空间ID | 子逻辑意义ID | 逻辑类型ID |
		  -----------------------------------------
		 |____64_int___|____64 int____|___64 int___|
*/

#define RB_SPACE_ID         ITEM_A
#define RB_SPACE_VALUE      ITEM_B
#define RB_SPACE_TYPE       ITEM_C


#define  NULL_MEANING   0

//记忆细胞类型
#define       MEMORY_START             0x10000  //65536


//注意放在最前面是有原因的,有函数根据把大于MEMORY_LOGIC_END找非结尾空间
#define 	  MEMORY_NULL_END     MEMORY_START -2 //无意义结尾空间，每一个记忆都隐含
#define 	  MEMORY_LOGIC_END    MEMORY_START -1 //有意义结尾空间

#define       MEMORY_BODY         MEMORY_START -0

//用0x00001-0x000ff来表示,这样方便确定某个词同时含有什么意义
#define	    PARTOFSPEECH_START			    0x11000   // 抽象词性 
#define       MEMORY_ENERGY          PARTOFSPEECH_START+0x000
#define		  MEMORY_LAN             PARTOFSPEECH_START+0x001
#define		  MEMORY_REFERENCE       PARTOFSPEECH_START+0x002
#define		  MEMORY_VERB            PARTOFSPEECH_START+0x004
#define		  MEMORY_NOUN            PARTOFSPEECH_START+0x008
#define		  MEMORY_PRONOUN         PARTOFSPEECH_START+0x010
#define		  MEMORY_ADJECTIVE       PARTOFSPEECH_START+0x020
#define		  MEMORY_NUMERAL         PARTOFSPEECH_START+0x040
#define		  MEMORY_ADVERB          PARTOFSPEECH_START+0x080
#define		  MEMORY_ARTICLE         PARTOFSPEECH_START+0x100
#define		  MEMORY_PREPOSITION     PARTOFSPEECH_START+0x200
#define		  MEMORY_CONJUNCTION     PARTOFSPEECH_START+0x400
#define		  MEMORY_INTERJECTION    PARTOFSPEECH_START+0x800
#define       MEMORY_LOGIC_AND       PARTOFSPEECH_START+0x1000 //特殊词性
#define       MEMORY_LOGIC_THEN      PARTOFSPEECH_START+0x2000 //要求在记忆单词and和than使用
#define		PARTOFSPEECH_END                 MEMORY_LOGIC_THEN+1
   
                                                   
#define       MEMORY_INSTINCT     PARTOFSPEECH_END +11 //标准本能,以及后天学习的新行为

#define       MEMORY_ACT          PARTOFSPEECH_END +12 //行为实例
#define       MEMORY_PEOPLE       PARTOFSPEECH_END +13 //处理人
#define       MEMORY_OBJECT       PARTOFSPEECH_END +14 //处理外部物体


#define       MEMORY_SERIES       PARTOFSPEECH_END +15 //处理逻辑串联
#define       MEMORY_SHUNT        PARTOFSPEECH_END +16 //处理逻辑并联
	    
#define    WORD_OK               0x01c432af42dc0000
#define    WORD_ERROR            WORD_OK+1
#define    WORD_IMPEACH          WORD_OK+2   

//用于结尾空间评价，暂时规定一个,或应该是对应单词的ID？
#define    MEANING_SENSE_OK         WORD_OK  
#define    MEANING_SENSE_ERROR      WORD_ERROR
#define    MEANING_SENSE_IMPEACH    WORD_IMPEACH


//系统设置
#define  ROOM_SYSTEM       127282417795000000
#define  SYS_ACCOUNT       127282417795000001
#define  SYS_WORLDIP       127282417795000002
#define  SYS_DIALOG        127282417795000003



//使用同一个数据库来表达大脑后,作为表名,左脑ID必须是负值,右脑必须是正值
#define ToLBrain(ID) ID=(ID>0)?-ID:ID
#define ToRBrain(ID) ID=(ID<0)?-ID:ID
	
#define ConvToMinus(ID) ID=ID>0?-ID:ID
#define ConvToPSign(ID) ID=ID<0?-ID:ID

#define ConvToTable(ID) ID=ID<0?-ID:ID


#define    IsPartOfSpeech(ID) ID>PARTOFSPEECH_START&&ID<PARTOFSPEECH_END
#define    IsMeaningRoom(ID)  (ID>MEMORY_BODY)

//在此范围内的记忆不是可读的文字
#define    UnReadable(ID) (ID>PARTOFSPEECH_END&&ID<(MEMORY_SHUNT+1))

tstring Instinct2Str(int32 InstinctID);


class CBrainMemory 
{
public:
	// 大脑数据库 
    static CppSQLite3DB		           BrainDB;  

	tstring                            m_LastError;

	static CUserMutex                  m_MemoryMutex;       
	
	static ePipeline                   m_GlobalLogicList;
 
	//逻辑物体
	static map<tstring,ePipeline>      m_GlobalObjectList;

	//People,与Object分开显示
	static map<tstring,ePipeline>      m_GlobalPeopleList; 

public:

    static void ClearMemory(); //包括上述四者
	
	/*除了登记逻辑，还处理逻辑里的元件和其它逻辑的引用或生成
	*/
	static ePipeline* FindGlobalLogic(tstring& Name);
	
	static void RegisterGlobalLogic(tstring& Name, const tstring& LogicText, ePipeline& LogicData, uint32 State,
		                             tstring LogicMeaning=_T(""), tstring InputDescrip = _T(""), tstring OutputDescrip = _T("") );

	//返回引用列表 add==false unreference;
	static ePipeline* ReferenceGlobalLogic(tstring& scrName,const tstring& refName,tstring& WhoRef,bool Add = true); 

	/*正被引用不能被删除，返回NULL, 同样删除时要解除对其它逻辑的引用
	  返回被删除的逻辑数据，注意: 使用者需要delete it
	*/
	static ePipeline* DeleteGlobalLogic(tstring& Name); 

	static ePipeline* FindGlobalObject(tstring& Fingerprint);
	static void RegisterGlobalObject(ePipeline& ObjectData);
	static void DeleteGlobalObject(tstring& CrcName); 

	static ePipeline* FindGlobalPeople(tstring& Name);
	static void RegisterGlobalPeople(ePipeline& PeopleData);
	static void DeleteGlobalPeople(tstring& Name); 
 

//	CLogicItem&  GetCurrentLogic(); 得到当前选择的应该交给view
//	string  GetCurrentCapacitorName();
//	string  GetCurrentInductorName();


public:

    //数据库操作
	/////////////////////////////////////////////////////////////////////////////////////

	bool HasTable(int64 ID);
    void CreateTable(int64 ID); 
	
	//根据大脑type分别在左脑或右脑生成新表
	//NOTE:Create时并不检查给定的表是否已经存在
	void CreateLBrainTable(int64 ID){
	         ToLBrain(ID);
			 CreateTable(ID);
	};
    void CreateRBrainTable(int64 ID){
             ToRBrain(ID);
			 CreateTable(ID);	       
	};
	
	bool LBrainHasTable(int64 ID){
	    ToLBrain(ID);
		return HasTable(ID);
	}
	bool RBrainHasTable(int64 ID){
		ToRBrain(ID);
		return HasTable(ID);
	}

	//如果结果为空返回false
	CppSQLite3Query Query(const char* Select,const char* From, const char* Where, const char* Value ); 
    
	//查询索引
	CppSQLite3Query LBrainQuery(const char* Select,int64 From, const char* Where, int64 Value );
    CppSQLite3Query RBrainQuery(const char* Select,int64 From, const char* Where, int64 Value );

	void InsertRow(int64 TableName,int64 one, int64 two, int64 three);
	//在指定的表中插入一行数据
	void LBrainInsertRow(int64 TableName,int64 one, int64 two, int64 three){
		ToLBrain(TableName);
		InsertRow(TableName,one,two,three);
	}
	void RBrainInsertRow(int64 TableName,int64 one, int64 two, int64 three){
		ToRBrain(TableName);
		InsertRow(TableName,one,two,three);	
	}
	

public:
	//空间操作，与数据库操作相比需要处理索引
	////////////////////////////////////////////////////////////////////////////////////

	//在指定父空间(表)下插入一个新的空间条目,同时生成索引,
	//新的空间条目并没有实际生成一个单独的表,这个工作留待它有子空间时才做
	//如果指定父空间的数据表还不存在(比如它也只是一个空间条目)
	//则只是简单的根据ID生成一个新表,但假设它的索引已经被制造
	/*之所以需要父空间的逻辑明文,是因为给出的父空间可能还没有生成单独的表
	  而制作索引时需要
    */ 
	void InsertRoom(int64 ParentTable,int64 ParentLogicText,int64 ChildTable,int64 LogicText,int64 LogicType);

		
    //连续生成多个嵌套空间，通常用于生成某个记忆的形
	int64 InsertMultiRoom(
		                   vector<int64>& ValueList,
						   int64 RoomType, 
						   int64 StartParentID = ROOT_SPACE,              //指定插入的起始空间   
						   int64 StartParentLogicTextID = ROOT_LOGIC_TEXT, //生成索引需要
						   bool  IsToken = false //组成token的是字符而不是其它空间的意义空间ID,因此需要特别处理
						   );

	//返回指定的意义空间ID or  0
	int64 HasMeaningRoom(int64 ParentTable,int64 Meaning,int64 MeaningType);

	//在某个形下生成一个意义空间，同时在其空间下自动生成一个无意义的结尾空间,
	//如果有重复的意义空间，返回此ID，
	//如果之前有一个NULL意义空间，则删除这个空间（或许今后这应该由统一的遗忘程序来执行）
	//如果ReturnID!=0那么意义空间的逻辑空间ID将由ReturnID指定，否则自动生成一个当前时间戳。
	//返回意义空间的逻辑空间ID
	int64 InsertMeaningRoom(int64 ParentTable,int64 ParentLogicText,int64 Meaning,int64 MeaningType,int64 ReturnID = 0);      

	/*
	   在意义空间下生成一个结尾空间。缺省情况是有意义的，无意义空间通常是自动生成。
	   注意：所有结尾空间都不在左脑生成索引。
	*/
	void InsertEndRoom(int64 ParentTable,int64 LogicText,int64 Type=MEMORY_NULL_END);

	//根据空间的识别ID，得到其值，出错返回0
	int64 GetRoomValue(int64 RoomID);

	//根据空间的逻辑ID，得到此空间的值和类型
	bool GetRoomInfo(int64 RoomID, int64& RoomValue, int64 & RoomType); 
	
	
	//根据父空间的识别ID，得到指定子空间的识别ID，出错为0
	int64 GetChildID(int64 ParentRoomID,int64 ChildRoomValue,int64 ChildRoomType);
	int64 GetChildType(int64 ParentRoomID,int64 ChildRoomID);
	int64 GetFatherID(int64 Child);
	void DeleteChild(int64 ParentTable,int64 ChildLogicText,int64 ChildLogicType);

public:


	//记忆的存取
    ////////////////////////////////////////////////////////////////////////////////

	int64 LearnMemory(
		                   vector<int64>& ValueList,  //形空间的值
						   int64 MeaningValue,        //指定意义空间的值
						   int64 MeaningType,    //意义空间类型
						   int64 MeaningSense = MEANING_SENSE_OK,   //如果=0则无效，否则给指定的意义一个评价
						   int64 ReturnID = 0,   //非零则指定返回值（作为意义空间的识别ID）
						   int64 StartParentID = ROOT_SPACE,              //指定插入的起始空间   
						   int64 StartParentLogicTextID = ROOT_LOGIC_TEXT, //生成索引需要
						   bool  IsToken = false 
						   ){

		//记忆形     
        
		BrainDB.start_transaction();
		int64 ID = InsertMultiRoom(ValueList,MEMORY_BODY,StartParentID,StartParentLogicTextID,IsToken);
		if(!ID){
			BrainDB.rollback_transaction();
			return 0;
		}
		//记忆意义空间（自动隐含一个无评价结尾）
		ID = InsertMeaningRoom(ID,ValueList.back(),MeaningValue,MeaningType,ReturnID);
		if (!ID)
		{
			BrainDB.rollback_transaction();
			return 0;
		}

		//给一个有效评价的结尾
		if(MeaningSense)InsertEndRoom(ID,MeaningSense,MEMORY_LOGIC_END);
		
		BrainDB.commit_transaction();
		return ID;
	}	

	int64 CheckMemory(
		                   vector<int64>& ValueList,  //形空间的值
						   int64 MeaningValue,        //指定意义空间的值
						   int64 MeaningType,    //意义空间类型
						   int64 MeaningSense= MEANING_SENSE_OK,   //如果=0则无效，否则检查是否有此评价
						   int64 StartParentID = ROOT_SPACE,              
						   int64 StartParentLogicTextID = ROOT_LOGIC_TEXT 
						   );

	  


	//得到指定形ID下的所有意义项,结果存入MeaningLsit<意义值,意义空间识别ID>
	//返回找到意义的个数,注意：MeaningList使用之前会被自动Clear,
    /*
	GetAllMeaning采用map<意义值，意义识别ID>有问题，如果意义值相同，则不能得到所有意义空间	
	*/
//	int32 GetAllMeaning(int64 ID, map<int64,int64> &MeaningList);
	int32 GetAllMeaningRoomID(int64 ID, deque<int64>& MeaningRoomIDList);

	//把所有空间类型为词性的集合成一个UINT32
	uint32 GetAllPartOfSpeech(int64 TokenID);
    //得到所有词性意义空间的空间ID及词性 map<RoomID，RoomType>
	int32 GetAllPartOfSpeechRoom(int64 ParentID,map<int64,int64>& RoomList);


	//在指定形下返回指定意义值的意义空间ID
    int64 GetMeaningRoomID(vector<int64> BodyList,int64 MeaningValue,int64 MeaningType){
		int64 ParentRoom = ROOT_SPACE;
		int64 ParentRoomValue = ROOT_LOGIC_TEXT;
		int64 RoomID = 0;
		
		int32 End = BodyList.size();
		for(int32 i= 0; i<End; i++)
		{	
			int64 RoomValue = BodyList[i];
			assert(RoomValue != 0);
			RoomID = GetChildID(ParentRoom,RoomValue, MEMORY_BODY);      
			if(RoomID == 0){ 
				return 0;
			}
			ParentRoom = RoomID;
			ParentRoomValue = RoomValue;
		}

		int64 MeaningID = HasMeaningRoom(RoomID,MeaningValue,MeaningType);
		return MeaningID;
	}

	//得到指定的意义值所表达事物的类型，
	//如果是现成的意义预定义值返回自身（其中是字符，返回字符值，是本能返回本能ID）
	//否则是其它意义空间的识别ID，则递归直到返回其具体的意义类型，出错返回0
//	int64 GetMemoryType(int64 MeaningRoomID,int64 MeaningValue);
	
//	bool  IsChar(int64 MeaningRoomID);
//	bool  IsInstinct(int64 MeaningRoomID);
//	bool  IsAction(int64 MeaningRoomID,int64 MeaningValue);

	//根据指定的意义空间ID返回其形，如果不是特别的意义，则返回其具体的并按相同的嵌套存储层次保存在Pipe里：
	//ePipeline（父空间）{ eINT64(子空间值), or ePipeline{...}...}
//	bool  GetMemoryBody(int64 MeaningID,int64 MeaningValue,ePipeline& Pipe);

	
	//返回指定逻辑记忆ID是否OK or ERROR 
    int32 GetMeaningState(int64 ID){ return 1; }; //暂时如此
   
 
	/*通用文字回取操作
	 把RoomID代表的空间转换成文本,Nest指示是否是嵌套调用
	 返回值指示返回的文本层次，0= Error 1=token 2=Clause，依次类推
	 本操作自动还原标点符号。
	*/
	int32 RetrieveText(int64 RoomID,tstring& Text,bool Nest=true); 
    
	//某些字符串因为标点符合，不宜用RetrieveText，比如文件名存储，此时可用此函数
	tstring RetrieveToken(int64 RoomID);

	//把Ojbect/people的存储路径到Text,并返回crc，错误返回0
    uint32  RetrieveObject(int64 RoomID,tstring& Text);
   
	bool  RetrieveAction(int64 RoomID,int64 RoomValue, tstring& Text,bool IsShunt);
	bool  RetrieveAction(int64 RoomID,int64 RoomValue, ePipeline* Pipe,bool IsShunt);

	//返回一段逻辑,存储为可编译状态
	bool  RetrieveLogic(int64 RoomID,ePipeline* Pipe,bool IsShunt = false);
    
	/*把一段逻辑表达为文字，由于出现嵌套的子逻辑,显示格式如下：
	  Logic 0 : instinct a, logic 2, instinct b;
	  Logic 1 : ...

      如果是抽象的自定义命令（本能直接在显示是调用RetrieveText()）则显示格式：
	  Command : aaa bbb ; 
	*/
	bool  RetrieveLogic(int64 RoomID,deque<tstring>& LogicList,tstring* Text=NULL,bool IsShunt = false);
    
	//根据一个任意存储ID，返回描述这个ID的文字列表
	void  GetMemo(int64 RoomID, deque<tstring>& MemoList);
    
	//根据一个逻辑存储ID，返回使用这个逻辑的自定义命令表
	void  GetCustomCommandText(int64 LogicID, deque<tstring>& CommanTextList);




	Energy* RetrieveEnergy(int64 RoomID);
    ePipeline* RetrievePipe(int64 RoomID);

    //大脑初始化
	////////////////////////////////////////////////////////////////////////////////

public:
	CBrainMemory();
	virtual ~CBrainMemory();

    void Open(const char* DB = NULL);
    void Close();

	/*用于系统关闭时在数据库中保存当前逻辑场景系统参数等，启动时恢复。
	按下列格式存储:
	  项目名     项目数据
	*/
	static void       SetSystemItem(int64 Item,AnsiString Info);
    static AnsiString GetSystemItem(int64 Item);
};

/////////////////////////////////////////////////////////////////////////////

#endif // _CBASEMEMORY_H__