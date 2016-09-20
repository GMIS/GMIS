// CBrainMemory.cpp : implementation of the CBrainMemory class
//
#pragma warning(disable: 4786)

#include "PhysicSpace.h"
#include "BrainMemory.h"
#include "Ipclass.h"
#include "InstinctDefine.h"

tstring Instinct2Str(int64 InstinctID){
	static map<int64,tstring>  InstinctTextList;
	if (InstinctTextList.size()==0)
	{
		int n = ( sizeof( (_InstinctTextList) ) / sizeof( (_InstinctTextList[0]) ) ) ;
		
		for (int i=0; i<n; i++)
		{
			InstinctTextList[_InstinctTextList[i].msg_id] =_InstinctTextList[i].msg_text;
		}		
	}
    tstring Str;
	map<int64,tstring>::iterator it = InstinctTextList.find(InstinctID);
	if(it!=InstinctTextList.end()){
		Str = it->second;		
	}
    return Str;
}


/////////////////////////////////////////////////////////////////////////////
// CBrainMemory construction/destruction
 
CUserMutex                CBrainMemory::m_MemoryMutex;
ePipeline				  CBrainMemory::m_GlobalLogicList;
map<tstring,ePipeline>	  CBrainMemory::m_GlobalObjectList;
map<tstring,ePipeline>    CBrainMemory::m_GlobalPeopleList;
CppSQLite3DB			  CBrainMemory::BrainDB;  

CBrainMemory::CBrainMemory()
{
	// TODO: add one-time construction code here
	//Open();
}

CBrainMemory::~CBrainMemory()
{

}

void CBrainMemory::Open(const char* DB /*= NULL*/){

	if(DB!=NULL){
		if(BrainDB.IsOpen()){
			BrainDB.close();
		}
		BrainDB.open(DB);
	}
	else{
		if (BrainDB.IsOpen())
		{
			return;
		}
		string dir ;//= GetCurrentDir();
		dir += "Brain.db";
		BrainDB.open(dir.c_str());
	}   

	if (!BrainDB.IsOpen())
	{
		throw tstring(_T("BrainDB not open"));
	}

	//首先生成ROOT_SPACE，似乎没有必要，当生成第一个空间时，会自动生成。  
	int64 SpaceID = ROOT_SPACE;   
	if(!RBrainHasTable(SpaceID))CreateRBrainTable(SpaceID);
	SpaceID = ROOM_SYSTEM;
	if(!HasTable(SpaceID)){
		CppSQLite3Buffer SQL;
		char TableName[30];
		int64toa(SpaceID,TableName);
		SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b TEXT NOT NULL);",TableName);
		BrainDB.execDML(SQL);
	}
};
void CBrainMemory::Close(){
    if (BrainDB.IsOpen())
    {
		BrainDB.close();
    }
}
	

void   CBrainMemory::SetSystemItem(int64 Item,AnsiString Info){
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	char a[30],b[30];
    int64toa(ROOM_SYSTEM,a);
	int64toa(Item,b);

	assert(BrainDB.tableExists(a));
	
    SQL.format("select b from \"%s\" where a = \"%s\";",a,b);				
	Result = BrainDB.execQuery(SQL);
	bool Find = !Result.eof();
    Result.finalize();

	if(!Find){
		SQL.format("insert into \"%s\" values (\"%s\", ?)",
			a,
			b);
	}else{
		SQL.format("update \"%s\" set b = ? where a = \"%s\";",
			a,
			b
			);
	}

	CppSQLite3Statement State = BrainDB.compileStatement(SQL);
	State.bind(1,Info.c_str()); //替换第一个问号
	State.execDML();

}

AnsiString CBrainMemory::GetSystemItem(int64 Item){

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	char Root[30],Name[30];
   
	assert(Item > 0 );
    AnsiString s;
	int64toa(ROOM_SYSTEM,Root);
	
	assert(BrainDB.tableExists(Root));

	int64toa(Item,Name);
	SQL.format("select b from \"%s\" where a = \"%s\";",Root,Name);
	try
	{
		Result = BrainDB.execQuery(SQL);
		if(!Result.eof()) s = Result.getStringField(0);
	
	}
	catch (...)
	{
		::MessageBox(NULL,_T("Brain.db may be damaged!"),_T("Warnning"),MB_OK);
	}

	return s;
};


void CBrainMemory::ClearMemory(){
	_CLOCK(&m_MemoryMutex);
	
	m_GlobalLogicList.Clear();	
    

	m_GlobalObjectList.clear();
	m_GlobalPeopleList.clear();

}

ePipeline* CBrainMemory::FindGlobalLogic(tstring& Name){
	tstring TrueName = Name;
	if(TrueName[0]==_T('"')){
		TrueName = Name.substr(1,Name.size()-2);
	}

	_CLOCK(&m_MemoryMutex);
	for(int i=0; i<m_GlobalLogicList.Size(); i++){
        ePipeline* LogicItem = (ePipeline*)m_GlobalLogicList.GetData(i);
		tstring& LogicName = LogicItem->GetLabel(); 
		if(LogicName == TrueName){
			return LogicItem;
		}
	}
	return NULL;
}


void CBrainMemory::RegisterGlobalLogic(tstring& Name, const tstring& LogicText, ePipeline& LogicData, uint32 State,
									    tstring LogicMeaning /*=""*/, tstring InputDescrip /*=""*/, tstring OutputDescrip /*=""*/ )
{	
	ePipeline LogicItem;
	LogicItem.SetLabel(Name.c_str());
	LogicItem.PushInt(State);
	LogicItem.PushString(LogicText);
	LogicItem.PushString(LogicMeaning);
	LogicItem.PushString(InputDescrip);
	LogicItem.PushString(OutputDescrip);
    LogicItem.Push_Directly(LogicData.Clone());
	ePipeline RefList;
	LogicItem.PushPipe(RefList); //引用名：谁引用，字符串成对保存

	_CLOCK(&m_MemoryMutex);
	m_GlobalLogicList.PushPipe(LogicItem);
};


ePipeline* CBrainMemory::ReferenceGlobalLogic(tstring& scrName,const tstring& refName,tstring& WhoRef,bool Add){

    ePipeline* LogicItem = FindGlobalLogic(scrName);
	if(LogicItem == NULL)return NULL;
   
	_CLOCK(&m_MemoryMutex);
	
	ePipeline* RefList = (ePipeline*)LogicItem->GetLastData();
	if(Add){
		for(int i=0; i<RefList->Size(); i+=2){
			tstring& refName1 = *(tstring*)RefList->GetData(i);
			tstring& WhoRef1 = *(tstring*)RefList->GetData(i+1);

			if (refName1 == refName && WhoRef1 == WhoRef)
			{
				RefList->EraseEnergy(i,2);
				return RefList;
			}
		}
	}else{
		RefList->PushString(refName);
		RefList->PushString(WhoRef);
	}
	return RefList;
}

/*
把本逻辑生成的Element标记删除
把本逻辑引用的Element和逻辑标记删除
把本逻辑标记删除

如果删除失败，通常是因为本逻辑被其它逻辑使用，则恢复所有删除标记
*/
ePipeline* CBrainMemory::DeleteGlobalLogic(tstring& Name)
{
	_CLOCK(&m_MemoryMutex);
	int i;
	ePipeline* LogicItem = NULL;
    for(i=0; i<m_GlobalLogicList.Size(); i++){
		LogicItem = (ePipeline*)m_GlobalLogicList.GetData(i);
		if(LogicItem->GetLabel() == Name)break;
	}

    if(i == m_GlobalLogicList.Size())return NULL;

	assert(LogicItem != NULL);
	
	ePipeline* RefList = (ePipeline*)LogicItem->GetLastData();

	if(RefList->Size()>0 ) return NULL;

	m_GlobalLogicList.MoveEnergy(i);
	
	return LogicItem;
}
	
ePipeline* CBrainMemory::FindGlobalObject(tstring& Name){
	_tcsupr(&Name[0]);
	_CLOCK(&m_MemoryMutex);
    map<tstring,ePipeline>::iterator It = m_GlobalObjectList.find(Name);
	while(It!=m_GlobalObjectList.end()){
		ePipeline& ObjectData = It->second;
		return &ObjectData;
	}
	return NULL;	
};

void  CBrainMemory::RegisterGlobalObject(ePipeline& ObjectData){
	tstring& Name = *(tstring*)ObjectData.GetData(0);
//    tstring& Fingerprint = *(tstring*)ObjectData.GetData(2);
	_tcslwr(&Name[0]);
//	_tcsupr(&Fingerprint[0]);
	
	_CLOCK(&m_MemoryMutex);
	map<tstring,ePipeline>::iterator It = m_GlobalObjectList.find(Name);
	if(It!=m_GlobalObjectList.end()){
		return;
	};

	m_GlobalObjectList[Name] = ObjectData;
};



void CBrainMemory::DeleteGlobalObject(tstring& CrcName){
	_tcsupr(&CrcName[0]);
	_CLOCK(&m_MemoryMutex);
	m_GlobalObjectList.erase(CrcName);
};


ePipeline* CBrainMemory::FindGlobalPeople(tstring& Name){
	
	_tcsupr(&Name[0]);
	_CLOCK(&m_MemoryMutex);
    map<tstring,ePipeline>::iterator It = m_GlobalPeopleList.find(Name);
	while(It!=m_GlobalPeopleList.end()){
		ePipeline& Item = It->second;
		return &Item;
	}
	return NULL;	
};

void  CBrainMemory::RegisterGlobalPeople(ePipeline& PeopleData){
	tstring& Name = *(tstring*)PeopleData.GetData(0);
	_tcsupr(&Name[0]);

	_CLOCK(&m_MemoryMutex);
	m_GlobalPeopleList[Name] = PeopleData;	
};


void CBrainMemory::DeleteGlobalPeople(tstring& Name){
	_tcsupr(&Name[0]);
	_CLOCK(&m_MemoryMutex);
    m_GlobalPeopleList.erase(Name);
};



//数据库操作
/////////////////////////////////////////////////////////////////////////////////////
 
bool CBrainMemory::HasTable(int64 ID)
{
	char TableName[30];
	int64toa(ID,TableName);
	return BrainDB.tableExists(TableName);
}

void CBrainMemory::CreateTable(int64 ID)
{
	CppSQLite3Buffer SQL;
	char TableName[30];
    int64toa(ID,TableName);
/*
关于：【记忆空间主键冲突问题】 16:54 2012/8/30
	索引空间的第三栏原本设置为主键唯一，但是当两个不同语言的单词指向同一个意义空间时，并且大家都是NOUN_MEANING时，
	此时意义空间值=意义空间ID，而意义空间值又是索引空间表名，这样，索引表的第三个数据就会相同，产生主键冲突。

     目前取消主键唯一，还不清楚会产生何种影响，大概会影响信息回取结果的唯一性

    if(ID<0)
		SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b INTEGER  NOT NULL,c INTEGER NOT NULL PRIMARY KEY ON CONFLICT FAIL);",TableName);
    else 
*/
	SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b INTEGER  NOT NULL,c INTEGER NOT NULL);",TableName);
	BrainDB.execDML(SQL);
}


CppSQLite3Query CBrainMemory::Query(const char* Select,const char* From, const char* Where, const char* Value )
{
    CppSQLite3Buffer SQL;
	if(!strcmp(Select,"*"))SQL.format("select * from \"%s\" where \"%s\" = \"%s\";",From,Where,Value);
	else SQL.format("select \"%s\" from \"%s\" where \"%s\" = \"%s\";",Select,From,Where,Value);
	
	return BrainDB.execQuery(SQL);
}

CppSQLite3Query CBrainMemory::LBrainQuery(const char* Select,int64 From, const char* Where, int64 Value )
{
	char bufa[30],bufb[30];
	ToLBrain(From);
	int64toa(From,bufa);
	int64toa(Value,bufb);
	return Query(Select,bufa,Where,bufb);
}

CppSQLite3Query CBrainMemory::RBrainQuery(const char* Select,int64 From, const char* Where, int64 Value )
{
	char bufa[30],bufb[30];
	ToRBrain(From);
	int64toa(From,bufa);
	int64toa(Value,bufb);
	return Query(Select,bufa,Where,bufb);
}



void CBrainMemory::InsertRow(int64 TableName,int64 one, int64 two, int64 three)
{
    CppSQLite3Buffer SQL;
	char a[30],b[30],c[30],d[30];
    int64toa(TableName,a);
	int64toa(one,b);
	int64toa(two,c);
	int64toa(three,d);
    SQL.format("insert into \"%s\" values (\"%s\", \"%s\", \"%s\")",a,b,c,d);

    BrainDB.execDML(SQL);
}


//空间操作，与数据库操作相比需要处理索引
/////////////////////////////////////////////////////////////////////////////////////
/*之所以需要父空间的逻辑明文,是因为给出的父空间可能还没有生成单独的表.
*/ 



void CBrainMemory::InsertSpace(int64 ParentTable,int64 ParentSpaceValue,int64 SpaceID,int64 SpaceValue,int64 SpaceType)
{
	ToRBrain(ParentTable);
	if(!RBrainHasTable(ParentTable)){
		CreateRBrainTable(ParentTable);
	}
     
	assert(SpaceValue != 0);
	RBrainInsertRow(ParentTable,SpaceID,SpaceValue,SpaceType);
	
	//制作索引
	if(!LBrainHasTable(SpaceValue)){
		CreateLBrainTable(SpaceValue);
	}
	LBrainInsertRow(SpaceValue,ParentTable,ParentSpaceValue,SpaceID);
}


int64 CBrainMemory::InsertMultiSpace(
				                    vector<int64>& SpaceValueList,
								    int64 SpaceType, 
									int64 StartParentID,// = ROOT_SPACE,
									int64 StartParentSpaceValueID,// = ROOT_LOGIC_TEXT
								    bool  IsToken// = false 
									) 
{
	if(SpaceValueList.size()==0)return 0;
	
	int64 ParentSpace = StartParentID;
	int64 ParentSpaceValue = StartParentSpaceValueID;
	int64 SpaceID = 0,MeaningID =0;


	int32 End = SpaceValueList.size();
	for(int i= 0; i<End; i++)
	{	
		MeaningID = SpaceValueList[i];
		assert(MeaningID != 0);
		SpaceID = GetChildID(ParentSpace,MeaningID, SpaceType);      
		if(SpaceID == 0){ //如果搜索途中提前发现空缺,则直接开始生成新空间.
			for(i; i<End; i++){
				SpaceID = AbstractSpace::CreateTimeStamp();
				MeaningID = SpaceValueList[i];
				InsertSpace(ParentSpace,ParentSpaceValue,SpaceID,MeaningID,SpaceType);					
				if(!IsToken){ 
					InsertEndSpace(MeaningID,MEANING_SENSE_OK,MEMORY_LOGIC_END);
				}
				ParentSpace = SpaceID;
				ParentSpaceValue = MeaningID;
			}
			return SpaceID;
		}else{ //如果不是token则在路过的空间的意义ID下插入一个结尾空间，
			//表示某个时间又记忆过此空间,这对搜索运算很有帮助
			if(!IsToken){ 
				InsertEndSpace(MeaningID,MEANING_SENSE_OK,MEMORY_LOGIC_END);
			}
		}
		ParentSpace = SpaceID;
		ParentSpaceValue = MeaningID;
	}

	return SpaceID;
}

int64 CBrainMemory::HasMeaningSpace(int64 ParentTable,int64 Meaning,int64 MeaningType){
	CppSQLite3Buffer SQL;
	char a[30],b[30];
	   
	if(!RBrainHasTable(ParentTable))return 0;
	ToRBrain(ParentTable);

	int64toa(ParentTable,a);
	int64toa(MeaningType,b);
	SQL.format("select %s, %s  from \"%s\" where %s = \"%s\";",
		   RB_SPACE_ID,
		   RB_SPACE_VALUE,
		   a,
		   RB_SPACE_TYPE,
		   b
		   );
	CppSQLite3Table t = BrainDB.getTable(SQL);

	if(Meaning == 0){
		for (int row = 0; row < t.numRows(); row++)
		{
			t.setRow(row);
			int64 ID   = t.getInt64Field(0);
			int64 Value = t.getInt64Field(1);
            if(ID == Value){
				return ID; //已经记忆过了，直接返回
			}
		}
	}else{
		for (int row = 0; row < t.numRows(); row++)
		{
			t.setRow(row);
			int64 ID   = t.getInt64Field(0);
			int64 Value = t.getInt64Field(1);
            if(Value == Meaning){
				return ID; //已经记忆过了，直接返回
			}
		}	
	}
	return 0;
}

int64 CBrainMemory::InsertMeaningSpace(int64 ParentTable,int64 ParentSpaceValue,int64 MeaningValue,int64 MeaningType,int64 ReturnID/* = 0*/){

	//首先检查，不能重复记忆相同的意义
	int64 MeaningID = HasMeaningSpace(ParentTable,MeaningValue,MeaningType);
    if(MeaningID)return MeaningID;

    //没有记忆过 
	int64 ChildID = ReturnID;
	if(ReturnID==0)ChildID = AbstractSpace::CreateTimeStamp();
     
	//特殊规定
    if(MeaningValue==NULL_MEANING) MeaningValue = ChildID;


	//生成指定意义空间
	InsertSpace(ParentTable,ParentSpaceValue,ChildID,MeaningValue,MeaningType);
	
	//插入一个缺省的结尾空间，其逻辑内容等于父空间逻辑内容
	InsertEndSpace(ChildID,MeaningValue,MEMORY_NULL_END);
	
	return ChildID;
}      

void CBrainMemory::InsertEndSpace(int64 ParentTable,int64 SpaceValue,int64 SpaceType)
{
	
	if(!RBrainHasTable(ParentTable)){
		CreateRBrainTable(ParentTable);
	}
	
	int64 ChildID = AbstractSpace::CreateTimeStamp(); 
	RBrainInsertRow(ParentTable,ChildID,SpaceValue,SpaceType);
	
};

int64 CBrainMemory::GetSpaceValue(int64 SpaceID){
	assert(SpaceID != ROOT_SPACE);

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	char a[30],b[30];

	ToRBrain(SpaceID);

    if(!RBrainHasTable(SpaceID))return 0;

	int64toa(SpaceID,a);
	int64toa(MEMORY_NULL_END,b);

	//首先找无意义结尾子空间，其空间值就是父空间的空间值
	SQL.format("select %s from \"%s\" where  %s = %s;",
				RB_SPACE_VALUE,
				a,
				RB_SPACE_TYPE,
				b
	);

	int64 SpaceValue;		
	
	Result = BrainDB.execQuery(SQL);
	if(!Result.eof()){  
		SpaceValue = Result.getInt64Field(0);
	    assert(SpaceValue);
		return SpaceValue;
	}

	//如果没有再找任意一个非结尾子空间的逻辑明文
	int64toa(SpaceID,a);
	int64toa(MEMORY_LOGIC_END,b);

	SQL.format("select %s from \"%s\" where  %s > \"%s\";",
		        RB_SPACE_VALUE,
				a,
				RB_SPACE_TYPE,
				b
				);
				
	Result = BrainDB.execQuery(SQL);
    if(Result.eof())return 0;   
	SpaceValue = Result.getInt64Field(0);

	ToLBrain(SpaceValue);

	int64toa(SpaceValue,a);
	int64toa(SpaceID,b);
	//通过左脑，根据子空间值以及SpaceID得到当前空间的逻辑明文
	//也就是当前空间的ID和逻辑明文,以便能向上漫游		
	SQL.format("select %s from \"%s\" where  %s = \"%s\";",
		        LB_FATHER_VALUE,
				a,
				LB_FATHER_ID,
				b
			);
				
	Result = BrainDB.execQuery(SQL);

    if(Result.eof())return 0;    		
	return Result.getInt64Field(0);
}


bool CBrainMemory::GetSpaceInfo(int64 SpaceID, int64& SpaceValue, int64& SpaceType){
    
	assert(SpaceID != ROOT_SPACE);

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;

	SpaceValue = GetSpaceValue(SpaceID);

	if(SpaceValue==0)return false;

	//但要得到逻辑暗文需要在向上走一次，得到父空间的ID
    Result = LBrainQuery(LB_FATHER_ID,SpaceValue,LB_CHILD_ID,SpaceID);
	if(Result.eof())return false;
		
	int64 FatherID = Result.getInt64Field(0);

	//回过头来,找当前空间的逻辑暗文
    Result = RBrainQuery(RB_SPACE_TYPE,FatherID,RB_SPACE_ID,SpaceID);
	if(Result.eof())return false;
    
	SpaceType =  Result.getInt64Field(0);
	return true;
}

int64 CBrainMemory::GetChildID(int64 ParentSpace,int64 ChildSpaceValue,int64 ChildSpaceType)
{
	ToRBrain(ParentSpace);
    int64 ChildID;
	 
	
	if(!RBrainHasTable(ParentSpace))return 0;

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	char a[30],b[30],c[30];
   
	assert(ChildSpaceValue != 0 || ChildSpaceType != 0);

	int64toa(ParentSpace,a);
	int64toa(ChildSpaceValue,b);
	int64toa(ChildSpaceType,c);

	SQL.format("select \"%s\" from \"%s\" where \"%s\" = \"%s\" and \"%s\" = %s;",
				RB_SPACE_ID,
				a,
				RB_SPACE_VALUE,
				b,				
				RB_SPACE_TYPE,
				c
				);
				
	Result = BrainDB.execQuery(SQL);
    if(Result.eof())return 0;  

	ChildID = Result.getInt64Field(0);
	return ChildID;
}

int64 CBrainMemory::GetChildType(int64 ParentSpaceID,int64 ChildSpaceID){
	ToRBrain(ParentSpaceID);
 	 	
	if(!RBrainHasTable(ParentSpaceID))return 0;

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	char a[30],b[30];
   
	assert(ChildSpaceID != 0 );
	int64toa(ParentSpaceID,a);
	int64toa(ChildSpaceID,b);

	SQL.format("select \"%s\" from \"%s\" where \"%s\" = \"%s\" ",
				RB_SPACE_TYPE,
				a,
				RB_SPACE_ID,
				b
				);
				
	Result = BrainDB.execQuery(SQL);
    if(Result.eof())return 0;  

	int64 ChildType = Result.getInt64Field(0);
	return ChildType;
};

//根据子空间找父空间
int64 CBrainMemory::GetFatherID(int64 Child)
{
	assert(Child != ROOT_SPACE);

	//首先找到它的逻辑明文
	
	int64 ChildSpaceValue = GetSpaceValue(Child);
	if(!ChildSpaceValue)return 0;

	//得到父空间的ID
    CppSQLite3Query  Result = LBrainQuery(LB_FATHER_ID,ChildSpaceValue,LB_CHILD_ID,Child);
	if(Result.eof())return false;
		
	return  Result.getInt64Field(0);
}

	
void CBrainMemory::DeleteChild(int64 ParentTable,int64 ChildSpaceValue,int64 ChildSpaceType){
	int64 ChildID = GetChildID(ParentTable,ChildSpaceValue,ChildSpaceType);
    if(ChildID==0)return ;

    CppSQLite3Buffer SQL;
	char buf[30];
	int64toa(ChildID,buf);
   
	//先删除子表
	if(BrainDB.tableExists(buf)){		
		SQL.format("DROP TABLE \"%s\"",buf);
		BrainDB.execDML(SQL);
	}

	//然后删除子条目
	char Father[30];
	int64toa(ParentTable,Father);
    SQL.format("delete from \"%s\" where %s = \"%s\" ",Father,RB_SPACE_ID,buf);
    BrainDB.execDML(SQL);

	//最后删除索引
	ToLBrain(ChildSpaceValue);
	int64toa(ChildSpaceValue,Father);
    SQL.format("delete from \"%s\" where %s = \"%s\" ",Father,LB_CHILD_ID,buf);
}

int64 CBrainMemory::CheckMemory(
		                   vector<int64>& SpaceValueList,  //形空间的值
						   int64 Meaning,        //指定意义空间的值
						   int64 MeaningType,    //意义空间类型
						   int64 MeaningSense,   //如果=0则无效，否则给指定的意义一个评价
						   int64 StartParentID /*= ROOT_SPACE*/,              //指定插入的起始空间   
						   int64 StartParentSpaceValueID /*= ROOT_LOGIC_TEXT*///生成索引需要
						   ){


	if(SpaceValueList.size()==0)return 0;

	int64 ParentSpace = StartParentID;
	int64 ParentSpaceValue = StartParentSpaceValueID;
	int64 SpaceID = 0,SpaceValue =0;

	try{
			    
		int32 End = SpaceValueList.size();
		for(int i= 0; i<End; i++)
		{	
			SpaceValue = SpaceValueList[i];
			assert(SpaceValue != 0);
			SpaceID = GetChildID(ParentSpace,SpaceValue, MEMORY_BODY);      
			if(SpaceID == 0){ 
				return 0;
			}
			ParentSpace = SpaceID;
			ParentSpaceValue = SpaceValue;
		}

	
		//检查指定的意义
		int64 MeaningID = HasMeaningSpace(ParentSpace,Meaning,MeaningType);
        if(MeaningID == 0)return 0;

		//检查效评价
		if(MeaningSense){
			int64 ID = GetChildID(MeaningID,MeaningSense,MEMORY_LOGIC_END);
		    if(ID==0)return 0;
		}
		return MeaningID;
		
	}catch(CppSQLite3Exception e ){
		return 0;
	}
	return 0;
}

//记忆存取
/////////////////////////////////////////////////////////////////////////////


/*
int32 CBrainMemory::GetAllMeaning(int64 ID, map<int64,int64> 
&MeaningList){

	   CppSQLite3Buffer SQL;
	   CppSQLite3Query  Result;
       char a[30];
	   char b[30];
      
       MeaningList.clear();
	
//	   if(!RBrainHasTable(ID))return 0;
	   
	   ToRBrain(ID);
	   SQL.format("select %s, %s  from \"%s\" where %s=\"%s\" ;",
						RB_SPACE_ID,
		                RB_SPACE_VALUE,
			            _i64toa(ID,a,10),
						RB_SPACE_TYPE,
						_i64toa(MEMORY_TYPE_MEANING,b,10)                  
						);
	   Result = BrainDB.execQuery(SQL);
       
	   while(!Result.eof())
	   {
          MeaningList[Result.getInt64Field(1)] = Result.getInt64Field(0);
		  Result.nextRow();
	   }
	   return MeaningList.size();
}
*/	
int32 CBrainMemory::GetAllMeaningSpaceID(int64 ID, deque<int64>& MeaningSpaceIDList){
	   CppSQLite3Buffer SQL;
	   CppSQLite3Query  Result;
       char a[30],b[30];      
       MeaningSpaceIDList.clear();
	
//	   if(!RBrainHasTable(ID))return 0;
	   
	   ToRBrain(ID);

	   int64toa(ID,a);
	   int64toa(MEMORY_BODY,b);
	   SQL.format("select %s  from \"%s\" where %s > \"%s\";",
						RB_SPACE_ID,
			            a, 
						RB_SPACE_TYPE,
						b                   
						);
	   Result = BrainDB.execQuery(SQL);
       
	   while(!Result.eof())
	   {
          MeaningSpaceIDList.push_back(Result.getInt64Field(0));
		  Result.nextRow();
	   }
	   return MeaningSpaceIDList.size();
}

uint32 CBrainMemory::GetAllPartOfSpeech(int64 ParentID){
	   CppSQLite3Buffer SQL;
       char a[30],b[30],c[30];
	   
	   uint32 PartOfSpeech = 0;
//	   if(!RBrainHasTable(ID))return 0;

	   ToRBrain(ParentID);
	   int64toa(ParentID,a);
	   int64toa(PARTOFSPEECH_START,b);
	   int64toa(PARTOFSPEECH_END,c);

	   SQL.format("select %s from \"%s\" where %s > \"%s\" AND %s< \"%s\";",
						RB_SPACE_TYPE,
			            a,
						RB_SPACE_TYPE,
						b, 
                        RB_SPACE_TYPE,
						c
						);
	   CppSQLite3Table t = BrainDB.getTable(SQL);

	   for (int row = 0; row < t.numRows(); row++)
	   {
			t.setRow(row);
			int64 type = t.getInt64Field(0);
            PartOfSpeech |= (uint32)((type-PARTOFSPEECH_START));  
	   }
	   return PartOfSpeech;
}

int32 CBrainMemory::GetAllPartOfSpeechSpace(int64 ParentID,map<int64,int64>& SpaceList)
{
	   CppSQLite3Buffer SQL;
       char a[30],b[30],c[30];
	   
	   uint32 PartOfSpeech = 0;
//	   if(!RBrainHasTable(ID))return 0;

	   ToRBrain(ParentID);

	   int64toa(ParentID,a);
	   int64toa(PARTOFSPEECH_START,b); 
	   int64toa(PARTOFSPEECH_END,c);

	   SQL.format("select %s,%s from \"%s\" where %s > \"%s\" AND %s< \"%s\";",
						RB_SPACE_ID,
						RB_SPACE_TYPE,
			            a,
						RB_SPACE_TYPE,
						b,
                        RB_SPACE_TYPE,
						c
						);
	   CppSQLite3Table t = BrainDB.getTable(SQL);

	   for (int row = 0; row < t.numRows(); row++)
	   {
			t.setRow(row);
			int64 ID = t.getInt64Field(0);
			int64 Type = t.getInt64Field(1);
            SpaceList[ID] = Type;
	   }
	   return SpaceList.size();
}
	

/* 
int64 CBrainMemory::GetMemoryType(int64 MeaningSpaceID,int64 MeaningValue){
	   
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
    char a[30],b[30];
	   
	if(MeaningValue == MEANING_TYPE_LAN || MeaningValue == MEANING_TYPE_INSTINCT){
		//向上漫游，得到父空间的值
		SQL.format("select %s from \"%s\" where  %s = \"%s\";",
			LB_FATHER_VALUE,
			_i64toa(MeaningValue,a,10),
			LB_CHILD_ID,
			_i64toa(MeaningSpaceID,b,10)
			);
		
		Result = BrainDB.execQuery(SQL);	
		if(Result.eof())return 0;    		
		return Result.getInt64Field(0);			
	}
	else if(IsMeaningValue(MeaningValue) ){
		return MeaningValue;
	}
	else if(MeaningValue){  //此时MeaningValue是其它意义空间的识别ID，取得这个空间的空间值，递归处理
		int64 SpaceValue = GetSpaceValue(MeaningValue);
		return GetMemoryType(MeaningValue, SpaceValue);
	}

  return 0;
}
*/
/*
bool CBrainMemory::GetMemoryBody(int64 MeaningID,int64 MeaningValue,ePipeline& Pipe){
     assert(0);
	 return false;

    int64  CurrentID = MeaningID;
    for(;;){
		
		//向上漫游,根据本空间的ID和逻辑明文,找到父空间空间的ID和逻辑明文,
        CppSQLite3Query Result = LBrainQuery("*",MeaningValue,LB_CHILD_ID,CurrentID);
		assert(!Result.eof());
		
		CurrentID = Result.getInt64Field(0);
		//如果得到的空间ID为根空间，则表示找到顶了。
  		if(CurrentID == ROOT_SPACE)return true;
		MeaningValue = Result.getInt64Field(1);   
		
		int64 Meaning = GetMemoryType(CurrentID,MeaningValue);
		if(MeaningValue == MEANING_TYPE_LAN ){
			Pipe.Push_Directly(new eINT64(Meaning));
		}
		else{
			ePipeline* Space = new ePipeline;
			MeaningID = MeaningValue;
			MeaningValue = GetSpaceValue(MeaningID);
			if(!GetMemoryBody(MeaningID,MeaningValue,*Space)){
				delete Space;
				return false;
			}
			Pipe.Push_Directly((Space));
		}
	}

	return true;	
}
*/

/*
bool  CBrainMemory::IsChar(int64 MeaningSpaceID){
	map<int64,int64>::iterator It = MemoryIDToValue.find(MeaningSpaceID);
	if(It != MemoryIDToValue.end()){
		return true;
	}
	return false;
}

bool  CBrainMemory::IsInstinct(int64 MeaningSpaceID){
	map<int64,int64>::iterator It = MemoryIDToValue.find(MeaningSpaceID);
	if(It != MemoryIDToValue.end()){
		return BelongInstinct(It->second);
	}
	return false;
};
*/

/*
bool  CBrainMemory::IsAction(int64 MeaningSpaceID,int64 MeaningValue){
	
	//检查MeaningValue是否为另一段逻辑的空间ID
    //太昂贵
	//int64 Value = GetSpaceValue(MeaningValue);
	//if(Value == MEANING_TYPE_SERIES || Value == MEANING_TYPE_SHUNT)return true;
	
	if (MeaningSpaceID == MeaningValue)//这时记忆自定义命令的一个小窍门     
	{  
		return true;
	}
	if(IsInstinct(MeaningSpaceID))return true;

	return false;
};

void CBrainMemory::InitTempMemory()
{
	int64 MemoryID = CHAR_BEGIN_MEMORYID+1;
	
	//常用ASSIC字符
	for(int i=1; i<128; i++){
		//if(isalnum(i)){
			ValueToMemoryID[i] = MemoryID;
			MemoryIDToValue[MemoryID] = i;
			MemoryID ++;
		//}
	}

	//所有本能
    MemoryID = INSTINCT_BEGIN_MEMORYID;

	int64 ValueID = INSTINCT_DEFINE_BYTE;
	//由于本能ID没有连续定义，因此分段填充
	for(ValueID; ValueID<(INSTINCT_VIEW_PIPE+1); ValueID++){
		ValueToMemoryID[ValueID] = MemoryID;
		MemoryIDToValue[MemoryID] = ValueID;
		MemoryID ++;
	}

	ValueID = INSTINCT_THINK_TASK;
	for(ValueID; ValueID<(INSTINCT_TECH+1); ValueID++){
		ValueToMemoryID[ValueID] = MemoryID;
		MemoryIDToValue[MemoryID] = ValueID;
		MemoryID ++;
	}
	
	ValueID = INSTINCT_RUN_TASK;
	for(ValueID; ValueID<(INSTINCT_FIND_SET_PRICISION+1); ValueID++){
		ValueToMemoryID[ValueID] = MemoryID;
		MemoryIDToValue[MemoryID] = ValueID;
		MemoryID ++;
	}

	ValueID = INSTINCT_USE_OBJECT;
	for(ValueID; ValueID<(INSTINCT_ASK_PEOPLE+1); ValueID++){
		ValueToMemoryID[ValueID] = MemoryID;
		MemoryIDToValue[MemoryID] = ValueID;
		MemoryID ++;
	}
}
*/
//Retrieve
////////////////////////////////////////////////////////////////////


tstring CBrainMemory::RetrieveToken(int64 SpaceID){
	int64 CurrentSpaceValue,SpaceType; 
	int64  CurrentID = SpaceID;

	//首先得到意义空间的信息
	if(!GetSpaceInfo(SpaceID,CurrentSpaceValue,SpaceType)){
		return NULL;
	}

	deque<int64> BodyList;
	while(1)
	{
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return 0;
		CurrentID = Result.getInt64Field(0);  //fatherID
		if(CurrentID == ROOT_SPACE)break;;
		CurrentSpaceValue = Result.getInt64Field(1); //fatherValuse
		BodyList.push_front(CurrentSpaceValue);
	}	

	tstring s;
	for (int i=0; i<BodyList.size(); i++)
	{
		int64 ID = BodyList[i];
		TCHAR ch = (TCHAR) IDToChar(ID);
		s += ch;
	}
	return s;
}
int32 CBrainMemory::RetrieveText(int64 MeaningID,tstring& Text,bool Nest){

	int64 CurrentSpaceValue,SpaceType; 
    int64  CurrentID = MeaningID;

	//首先得到意义空间的信息
	if(!GetSpaceInfo(MeaningID,CurrentSpaceValue,SpaceType))return 0;

	if(!Nest){
		//首次外部调用时检查MeaningID代表记忆是否为可读的文字信息，嵌套调用则忽略
		//if(UnReadable(SpaceType))return 0;
	}

	//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
    CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
	if(Result.eof())return 0;
    
	CurrentID = Result.getInt64Field(0);
  	if(CurrentID == ROOT_SPACE)return 0;
	CurrentSpaceValue = Result.getInt64Field(1);  //总是其他空间的空间识别ID
	
    
	//如果是字符,则可以确定所取文本应该是token
	int ch = IDToChar(CurrentSpaceValue);
    if(ch>0 && ch<100000){ //简单判断是字符
		TCHAR buf[100]; //暂存token，一个单词99字符应该足够了
		int p = 0;
		buf[p++] = ch;
        
		//继续向上漫游,应该全部都是字符
		for(;;){
			//根据本空间的ID和逻辑明文,找到父空间空间的ID和逻辑明文,
			CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
			if(Result.eof())return 0;
			
			CurrentID = Result.getInt64Field(0);
			//如果得到的空间ID为根空间，则表示找到顶了。
			if(CurrentID == ROOT_SPACE){
				buf[p]='\0';
				_tcsrev(buf);
				
				Text = buf;
				//如果是形容词则加上引号
				if(SpaceType == MEMORY_REFERENCE){
					Text.insert(Text.begin(),1,_T('\"'));
					Text+=_T('\"');
				}
				return 1;  //表示得到一个token
			}
			CurrentSpaceValue = Result.getInt64Field(1);   
			
			if(p<100) buf[p++]  = IDToChar(CurrentSpaceValue);
		}
	}
		
    //不是字符，则需要嵌套处理，然后根据返回值添加标点符号
	int32 n = 0;

	vector<tstring> StrList;
	for(;;){
		
		tstring s;
		n = RetrieveText(CurrentSpaceValue,s,true);
		
		StrList.push_back(s);
		
		//继续向上漫游,根据本空间的ID和逻辑明文,找到父空间空间的ID和逻辑明文,
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return 0;
		
		CurrentID = Result.getInt64Field(0);
		//如果得到的空间ID为根空间，则表示找到顶了。
		if(CurrentID == ROOT_SPACE)break;
		
		CurrentSpaceValue = Result.getInt64Field(1);   		
	}

	TCHAR flag=_T(' ');
	if(n==1){ //子句，token之间应该有空格
			flag  = _T(' ');
	}
	else if(n==2){ //句子，子句之间有逗号
			flag = _T(',');
	}
	else if(n ==3){ //段落，句子之间有句号
			flag = _T('.');
	} 
    else if(n== 4){//文本，段落之间要分行
            flag = _T('\n');			
	}
	assert(n<5);
	

	vector<tstring>::reverse_iterator It = StrList.rbegin();
	while(It != StrList.rend()){
		Text += *It;
		if(Text.size()){ 
			TCHAR ch =  Text[Text.size()-1];
            if(ch<256 && !ispunct(ch)){
				Text += flag;
			}else if(Text.size()>2) {
				ch = Text[Text.size()-2];
				if (ch<256 && isspace(ch))
				{
					Text.erase(Text.size()-2,1);
				}
			}
		}
		It++;
	}	     
	
	if(SpaceType == MEMORY_REFERENCE)
	{
		Text.insert(Text.begin(),1,_T('\"'));
		Text+='\"';
		assert(n==1 || n==0); //引用被看作是一个token，应该只出现在子句里
	    return 1;
	}
	return  ++n;
} 

//返回一个Ojbect/people的存储路径,不包括CRC32
uint32  CBrainMemory::RetrieveObject(int64 SpaceID,tstring& Text){
	int64 CurrentSpaceValue,SpaceType; 
    int64  CurrentID = SpaceID;

	//首先得到意义空间的信息
	GetSpaceInfo(SpaceID,CurrentSpaceValue,SpaceType);
	if (SpaceType == MEMORY_OBJECT){
		Text = _T("Object: ");
	}else if(SpaceType == MEMORY_PEOPLE)
	{
        Text = _T("People: ");
	}else return 0;
	uint32 crc = CurrentSpaceValue;

	deque<int64> MeaingList;
	while(1)
	{
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return 0;
 		CurrentID = Result.getInt64Field(0);  //fatherID
		if(CurrentID == ROOT_SPACE)break;;
	    CurrentSpaceValue = Result.getInt64Field(1); //fatherValuse
        MeaingList.push_front(CurrentSpaceValue);
	} 

	deque<int64>::iterator It = MeaingList.begin();
	deque<int64>::iterator ItEnd = MeaingList.end();
	int64 ID = *It;
	IP ip((uint32)ID);

	AnsiString s = ip.Get();
	tstring ws = UTF8toWS(s);
	Text += ws;

	It++;
	tstring Name;
	while (It <ItEnd){
		ID = *It;
  	    Text += _T("\\");
		if(!RetrieveText(ID,Name))return 0;
		Text +=Name;
		Name=_T("");
		It++;
	};
	return crc;
}
	
bool CBrainMemory::RetrieveAction(int64 SpaceID,int64 SpaceValue, tstring& Text,bool IsShunt){
    
	deque<int64> MeaingList;
	
	while(1){
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",SpaceValue,LB_CHILD_ID,SpaceID);
		if(Result.eof())return false;
    
	    SpaceValue = Result.getInt64Field(1); //fatherValuse
		SpaceID = Result.getInt64Field(0);  //fatherID
        if(SpaceID == ROOT_SPACE)break;
		MeaingList.push_front(SpaceValue);
	}
	
	assert(MeaingList.size() <3);
	int64 ActionID = MeaingList.front();
	int64 Param   = 0;
	if(MeaingList.size()==2)Param = MeaingList.back();

	GetCommandText(ActionID,Text);

	if(IsShunt)Text = _T("and ")+Text;
	if(Param){ //如果是本能还要返回参数
		assert(BelongInstinct(ActionID));
		tstring ParamText;
		RetrieveText(Param,ParamText);
		Text +=_T(' ');
		Text += ParamText;
	}
	return true;
}
	
bool CBrainMemory::RetrieveInstinctInstance(int64 SpaceID,int64 SpaceValue, ePipeline* Pipe){
	deque<int64> MeaingList;
 
	while(1){
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",SpaceValue,LB_CHILD_ID,SpaceID);
		if(Result.eof())return false;
    
	    SpaceValue = Result.getInt64Field(1); //fatherValuse
		SpaceID = Result.getInt64Field(0);  //fatherID
        if(SpaceID == ROOT_SPACE)break;
		MeaingList.push_front(SpaceValue);
	}
	

	assert(MeaingList.size() <3);
	int64 InstinctID = MeaingList.front();
	
	tstring Text;
	GetCommandText(InstinctID,Text);

	Pipe->m_Label=Text;

	int64 Param   = 0;
	if(MeaingList.size()==2)Param = MeaingList.back();

	assert(BelongInstinct(InstinctID));

	Pipe->PushInt(InstinctID);
	if (Param)
	{

		if(INSTINCT_DEFINE_FLOAT64 == InstinctID){
			tstring ParamText = RetrieveToken(Param);
			TCHAR* endptr = NULL;
			float64 f = _tcstod(ParamText.c_str(),&endptr);
			assert(endptr == NULL);
			Pipe->PushFloat(f);				
		}
		else if(INSTINCT_DEFINE_INT64 == InstinctID){
			tstring ParamText = RetrieveToken(Param);
			int64 t = _ttoi64(ParamText.c_str());
			Pipe->PushInt(t);
		}
		else if (INSTINCT_DEFINE_STRING == InstinctID)
		{
			tstring ParamText;
			RetrieveText(Param,ParamText,false);
			Pipe->PushString(ParamText);
		}
		else if( INSTINCT_USE_RESISTOR == InstinctID){
			tstring ParamText = RetrieveToken(Param);

			int64 t = _ttoi64(ParamText.c_str());
			Pipe->PushInt(t);
		}
		else if( INSTINCT_USE_DIODE == InstinctID){
			tstring ParamText = RetrieveToken(Param);

			int64 t = _ttoi64(ParamText.c_str());
			Pipe->PushInt(t);     
		}
		else if (INSTINCT_USE_OBJECT == InstinctID)
		{
			ePipeline* ObjectInfo = (ePipeline*)RetrieveEnergy(Param);
			assert(ObjectInfo);
			Pipe->Push_Directly(ObjectInfo);
		}
		else if (INSTINCT_WAIT_TIME == InstinctID)
		{
			tstring ParamText = RetrieveToken(Param);
			TCHAR* endptr = NULL;
			float64 f = _tcstod(ParamText.c_str(),&endptr);
			Pipe->PushFloat(f);			
		}
		else{
			tstring ParamText = RetrieveToken(Param);
			Pipe->PushString(ParamText);
		}
	}
	return true;
}


Energy* CBrainMemory::RetrieveEnergy(int64 SpaceID){
	int64 CurrentSpaceValue,SpaceType; 
    int64  CurrentID = SpaceID;
	
	//首先得到意义空间的信息
	if(!GetSpaceInfo(SpaceID,CurrentSpaceValue,SpaceType)){
		return NULL;
	}
	
	deque<int64> MeaingList;
	while(1)
	{
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return 0;
		CurrentID = Result.getInt64Field(0);  //fatherID
		if(CurrentID == ROOT_SPACE)break;;
		CurrentSpaceValue = Result.getInt64Field(1); //fatherValuse
        MeaingList.push_front(CurrentSpaceValue);
	}
	
	if (MeaingList.size()!=2)
	{
		return NULL;
	}

	int64 Type = MeaingList[0];
	int64 ValueID = MeaingList[1];

	Energy* E=NULL;
	switch(Type){
	   case TYPE_NULL:
		   {
			   E = new eNULL();
		   }
		   break;
	   case TYPE_INT:
		   {
			   E = new eINT(ValueID);
		   }
		   break;
	   case TYPE_FLOAT:
		   {
			   assert(0); //暂且这样简化
			   E = new eFLOAT(ValueID);
		   }
		   break;
	   case TYPE_STRING:
		   {
			   
			   tstring s;
			   if(!RetrieveText(ValueID,s,false)){
				   return NULL;
			   }
			   E = new eSTRING(s);
		   }
		   break;
	   case TYPE_PIPELINE:
		   {
			   E = RetrievePipe(ValueID);
		   }      
		   break;
	   default:
		   break;
	}
	
	return E;
}



ePipeline* CBrainMemory::RetrievePipe(int64 SpaceID){
	int64 CurrentSpaceValue,SpaceType; 
    int64  CurrentID = SpaceID;
	
	//首先得到意义空间的信息
	if(!GetSpaceInfo(SpaceID,CurrentSpaceValue,SpaceType)){
		return NULL;
	}
	
	deque<int64> BodyList;
	while(1)
	{
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return 0;
		CurrentID = Result.getInt64Field(0);  //fatherID
		if(CurrentID == ROOT_SPACE)break;;
		CurrentSpaceValue = Result.getInt64Field(1); //fatherValuse
        BodyList.push_front(CurrentSpaceValue);
	}

	ePipeline* Pipe = new ePipeline;
	eElectron E(Pipe);

	for (int i=0; i<BodyList.size(); i++)
	{
		int64 ID = BodyList[i];
		Energy* e = RetrieveEnergy(ID);
		if (!e)
		{
			return NULL;
		}
		Pipe->Push_Directly(e);
	}

	return (ePipeline*)E.Release();
}

    
//返回一段逻辑,存储为可编译状态
bool  CBrainMemory::RetrieveLogic(int64 SpaceID,ePipeline* Pipe){
	int64 CurrentSpaceValue,SpaceType; 
    int64  CurrentID = SpaceID;

	//首先得到意义空间的信息
	GetSpaceInfo(SpaceID,CurrentSpaceValue,SpaceType);
	if(SpaceType ==  MEMORY_ACT){  //递归终止
		return RetrieveInstinctInstance(SpaceID,CurrentSpaceValue,Pipe);
	}

	if(SpaceType != MEMORY_SERIES &&  SpaceType != MEMORY_SHUNT )return false;
  
    int64 LogicType = SpaceType;

	deque<int64> MeaingList;
    while(1)
	{
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return false;
		CurrentID = Result.getInt64Field(0);  //fatherID
		if(CurrentID == ROOT_SPACE)break;
		CurrentSpaceValue = Result.getInt64Field(1); //fatherValuse
        MeaingList.push_back(CurrentSpaceValue);
	} 
	
	deque<int64>::reverse_iterator It = MeaingList.rbegin();
	while (It != MeaingList.rend())
	{
		CurrentSpaceValue = *It;

        ePipeline ChildPipe(SpaceType==MEMORY_SHUNT?-1:0);
		
		if (!RetrieveLogic(CurrentSpaceValue,&ChildPipe)){
			return false;
		}

		Pipe->PushPipe(ChildPipe);

	    It++;	
	}
	return true;
}
    
//返回一段逻辑,表达为文字
bool  CBrainMemory::RetrieveLogic(int64 SpaceID,deque<tstring>& LogicList,tstring* Text/*=NULL*/,bool IsShunt /* = false*/){
	int64 CurrentSpaceValue,SpaceType; 
    int64  CurrentID = SpaceID;

	//首先得到意义空间的信息
	GetSpaceInfo(SpaceID,CurrentSpaceValue,SpaceType);

	if(SpaceType ==  MEMORY_ACT){//递归终止
		assert(Text != NULL);
		return RetrieveAction(SpaceID,CurrentSpaceValue,*Text,IsShunt);
	}
 
	if(SpaceType == MEMORY_INSTINCT){
		tstring s;
	    if(!RetrieveText(SpaceID,s))return false;
		s = _T("Custom Command : ") + s;
        s+=_T(';');
		LogicList.push_back(s);
		return true;
	}

	if(SpaceType != MEMORY_SERIES && 
		SpaceType != MEMORY_SHUNT)return false;
  
    int64 LogicType = SpaceType;

	deque<int64> MeaingList;
    while(1)
	{
		//向上漫游,找到父空间空间的ID和逻辑明文，得记忆的形ID
		CppSQLite3Query Result = LBrainQuery("*",CurrentSpaceValue,LB_CHILD_ID,CurrentID);
		if(Result.eof())return false;
		CurrentID = Result.getInt64Field(0);  //fatherID
		if(CurrentID == ROOT_SPACE)break;
		CurrentSpaceValue = Result.getInt64Field(1); //fatherValuse
        MeaingList.push_front(CurrentSpaceValue);
	} 

    if(Text) *Text = Format1024(_T("use logic %d"),LogicList.size());

	tstring LogicText = Format1024(_T("Logic %d: "),LogicList.size());
	LogicList.push_back(LogicText);

	tstring& NestLogicText = LogicList.back();

    IsShunt = SpaceType == MEMORY_SHUNT;
	deque<int64>::reverse_iterator It = MeaingList.rbegin();

	while (It != MeaingList.rend())
	{
		CurrentSpaceValue = *It;
		LogicText= _T("");
		if (!RetrieveLogic(CurrentSpaceValue,LogicList,&LogicText,IsShunt))return false;
	    LogicText +=_T(',');
		NestLogicText += LogicText;
		It++;	
	}
  
	*NestLogicText.rbegin() =_T(';');
     
	return true;
}

void  CBrainMemory::GetMemo(int64 SpaceID, deque<tstring>& MemoList){
	int64 FatherID,ChildID,SpaceType;
	char buf[30];
	CppSQLite3Buffer SQL;
	
	if(LBrainHasTable(SpaceID)){
		//索引空间是否有表，有则表示其为其它空间的空间值
		//根据索引表得到其作为空间值时的空间存储ID，存入种子表
		ToLBrain(SpaceID);
		int64toa(SpaceID,buf);
		SQL.format("select %s,%s from  \"%s\"  ;",LB_FATHER_ID,LB_CHILD_ID,buf);
		CppSQLite3Table t0 = BrainDB.getTable(SQL);
		for (int row = 0; row < t0.numRows(); row++)
		{
			t0.setRow(row);
			FatherID = t0.getInt64Field(0);
			ChildID = t0.getInt64Field(1);
			if(ChildID == -SpaceID)continue;
			SpaceType = GetChildType(FatherID,ChildID);
			if(SpaceType == MEMORY_LAN){ //看是否文本空间修饰SpaceID
                tstring Text;
				if(RetrieveText(ChildID,Text)){
					MemoList.push_back(Text);
				}
			}
		}
	}
}

void  CBrainMemory::GetCommandText(int64 LogicID,tstring& CmdText){
	int64 FatherID,ChildID,SpaceType;
	char buf[30];
	CppSQLite3Buffer SQL;
	
	if(LBrainHasTable(LogicID)){
		//索引空间是否有表，有则表示其为其它空间的空间值
		//根据索引表得到其作为空间值时的空间存储ID，存入种子表
		ToLBrain(LogicID);
		int64toa(LogicID,buf);
		SQL.format("select %s,%s from  \"%s\"  ;",LB_FATHER_ID,LB_CHILD_ID,buf);
		CppSQLite3Table t0 = BrainDB.getTable(SQL);
		for (int row = 0; row < t0.numRows(); row++)
		{
			t0.setRow(row);
			FatherID = t0.getInt64Field(0);
			ChildID  = t0.getInt64Field(1);
			if(ChildID == -LogicID)continue;
			SpaceType = GetChildType(FatherID,ChildID);
			if(SpaceType == MEMORY_INSTINCT){ //看是否文本空间修饰SpaceID
			    tstring cmd;
				if(RetrieveText(ChildID,cmd,false)){
					CmdText = cmd;
					return;
				}
			}
		}
	}
}

//待删除
///////////////////////////////////////////////////////////////////
/*

void CBrainMemory::GetAllChildSpaceValue(int64 ParentSpace,int64 SpaceType,map<int64,int64>& SpaceValueList)
{
	   CppSQLite3Buffer SQL;
	   CppSQLite3Query  Result;
       char a[30],b[30];

	   ToRBrain(ParentSpace);
	   SQL.format("select %s,%s from \"%s\" where %s= \"%s\" ;",
						RB_SPACE_ID,
		                RB_SPACE_VALUE,
			            _i64toa(ParentSpace,a,10),
						RB_SPACE_TYPE,
						_i64toa(SpaceType,b,10)
						);
	   Result = BrainDB.execQuery(SQL);

	   while(!Result.eof())
	   {
          SpaceValueList[Result.getInt64Field(0)] = Result.getInt64Field(1);
		  Result.nextRow();
	   }
}
*/
