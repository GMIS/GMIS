/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SPACE_H__
#define _SPACE_H__


//#include <vld.h>   //内存检测

#include "Mass.h"
#include "Pipeline.h"
#include "Sqlite\CppSQLite3.h"
#include <algorithm>
#include <vector>
#include <string>
#include <map>

using namespace std;


/* 物理空间

 1 存储项目          数据类型   备注
	 空间ID          INT64   
	 空间名字        TEXT       展现给用户的名字
	 空间类型        INT64      0=本地空间 1=外部空间 2=ROBOT（特殊的外部空间）
     空间指纹        TEXT       特征值，比如CRC32或MD5
     空间属性        TEXT       一个ePipeline数据结构，空间本地文件存储位置,3D几何位置等
     空间所有权信息  TEXT       一个ePipeline数据结构，本空间所有人的列表

 其中：
  空间属性
        存储项目			类型           备注
		大小                int64          空间大小
		空间本地位置        string         文件存储路径
		空间几何位置        ePipeline      相对于父空间的几何位置，由多个int32组成
		
  空间所有权信息
        存储项目			类型           备注
		权利级别            int64          对于ROBOT指其所拥有的权力级别，对于SPACE指其接受来访人的权力
        权利人列表（仅对SPACE有效，一个空间可以有多个权利人，每个权利人含有如下信息）
		
			姓名            string         注册本空间拥有者真实姓名
			权利            int
			注册密文        string         由对方提供，用来验证对方身份
			eMail           string         联系方式
			备注            string
			身份验证模块    BLOB           空间所有人自己设计的身份验证模块，是一个DLL
			

  2  数据库表设计
		表名：64位整数，统一为正值
		--------------------------------------------------------------------------------   
		|空间ID|空间名字|空间类型|空间指纹| 空间属性|空间所有权信息| 
		--------------------------------------------------------------------------------

*/


#define ITEM_ID		       "a"
#define ITEM_NAME		   "b"
#define ITEM_TYPE          "c" 
#define ITEM_FINGERPRINT   "d"    
#define ITEM_PROPERTY      "e"
#define ITEM_OWNER	       "f"


#define ROOT_SPACE             1
#define ROOT_LOGIC_TEXT        1
#define ROOT_LOGIC_TYPE        1
#define ROOT_SPACE_STR        "1"


#define OUTER_SPACEID          10000
#define OUTER_SPACE_STR       "10000"

#define LOCAL_SPACEID          20000
#define LOCAL_SPACE_STR       "20000"

#define ROBOT_SPACE            2

class People;
class ROOM_SPACE;



People&     GetHost(People* p=NULL); //当p!=NULL意味着初始化
ROOM_SPACE& GetRootSpace(ROOM_SPACE* r=NULL);//同上


struct SpaceAddress{
	int64  ParentID;
	int64  ChildID;
	
	SpaceAddress();
	SpaceAddress(int64 Father, int64 Child);
	SpaceAddress(const SpaceAddress& Address);
    bool IsValid();
    void Reset(); 
	SpaceAddress& operator =(const SpaceAddress& Address);
	bool operator < (const SpaceAddress& Address);
	bool operator > (const SpaceAddress Address);
	bool operator == (const SpaceAddress Address);
    
	//在map里使用
	friend bool operator < (const SpaceAddress& A,const SpaceAddress& B) { return true;};
};

//返回物理空间容器 当DB有效时，打开或重新打开指定数据库
CppSQLite3DB& GetWorldDB(const TCHAR* DB = NULL);

//按空间定义制造一个空表，表名为64位整数，总是正值

void CreateTable(int64  TableID);
    
bool HasTable(int64 TableID);
bool HasChild(int64 ParentID);
bool HasChild(int64 ParentID,int64 ChildID);

/*把所有当前空间地址的所有Child按下列格式存储：  
	  ePipeline(ID = ParentID,ParentName)
	     ChildID, eSTRING(Name), eINT32(Type)}
		 ....
		 ChildID,{ eSTRING(Name), eINT32(Type)}
    返回Child数目
*/
int32 GetAllChildList(int64 ParentID, ePipeline& ChildList,int64 NotIncludeChildID=0);

void  GetAllVisiter(int64 ParentID, vector<int64>& VisiterList,int64 NotIncludeChildID=0);

//删除所有子空间,包括其递归子空间
//提示：或许外部调用者应该考虑用事物回滚
void DeleteAllChild(int64 ParentID);

void DeleteChild(int64 ParentID,int64 ChildID);

SpaceAddress  FindChildSpace(ePipeline& Address,tstring& Fingerprint);
SpaceAddress  FindSpace(ePipeline& Address);


//排除文件名后缀部分
tstring GetFileName(tstring s);
tstring GetFilePath(tstring FileName);
tstring GetFileNoPathName(tstring s); 

//给出文件名后缀
tstring GetObjectType(tstring s);

tstring     GetLocalSpaceName();


/*
  LOCAL_SPACE和OUTER_SPACE允许ROBOT进入
  CONTAINER_SPACE不允许ROBOT进入
  ROBOT_HOST仅仅用于权利注册
  ROBOT_VISITER则表示当前在此空间停留的ROBOT
*/
enum  SPACETYPE{ 
	LOCAL_SPACE=1, 
	CONTAINER_SPACE,
	OUTER_SPACE,
	OBJECT_SPACE, 
	ROBOT_VISITER,
	ROBOT_HOST
};



/*注册信息存储在一个ePipeline里，我们可以通过定义数据保存次序
  可以象结构一样访问其数据成员
*/
enum RegInfoType{ 
	REG_RIGHT = 0,
    REG_OWNER_LIST
};

enum PropertyInfoType{
	PROPERTY_SIZE = 0,
	PROPERTY_FILE_PATH,
	PROPERTY_SPACE_POS
};

/*赋权策略：
   对物而言，指它的权利状态。对人而言，指它拥有的权利状态。

   1 对于空间的赋权安排
     - 赋权安排以一个空间为单位
     - 任何无主空间，由第一个访问它的人安排它的权利状态，比如宣布此空间由我独有。当一个ROBOT
	 宣布一个空间为其所有后，它将成为这个空间的一个固定子空间。一个空间可以拥有多个不同权限
	 的权利人。
     - 当一个空间拥有权利人后，权利人可以为此空间赋权，比如允许其他人访问，或使用此空间等
	 等等。
	 - 权利人可以把自己的权利授权给其它人，其它人在自己的授权范围内也可以再授权给其他人，
     - 不是所有的空间都需要权利安排。对于没有权利安排的空间，子空间的权利人从属于父空间，但
	   可具有单独的权利安排。当前访问它的人可以在不通知父空间权利人的情况下，可以临时宣布拥
	   有排它权利。所谓临时是指，当前访问人离开以后此空间的赋权状态即恢复原状，同时父空间权利
	   人可以随时撤消。而所谓的排它权限是指除父空间权利人以外，其它人不能在进入此空间。排它性
	   权利人临时拥有此空间的最高权限。
	 - 对于以有赋权安排的空间，父空间权利人也不能对抗本空间的权利人。
	 - 对于本地的第一个空间，如果还没有上级空间可以连接，即无法得知父空间的权利安排，可以通
	     过一个单独的管理员密码代替。管理员密码只对第一个空间有效，它不是超级用户，不能作用与
		 其它子空间。
     
   2 对于ROBOT的赋权安排：
     - 一个ROBOT成为一个空间的权利人后，将成为此空间的固定子空间。
	 - 每个ROBOT将拥有不同的权限，每一种权限将有不同的表决权。
	 - 每一个ROBOT可以根据自己的权限给空间赋权，当空间有不同的人试图赋权时，以最大权利人为准
	 - ROBOT想提高自己的权利，需要向具有同等权利的人申请。当拥有此权利的人不只一人时，需要它
	     们表决同意，通常半数以上同意为准。
	 

   3 权利验证程序
     - 权利注册
	     当前空间如果还没有权利人，则直接填写注册信息成为其第一个权利人。如果有了权利，则把注册
		 信息提供给此空间的权利人，由他来操作完成权利注册。只有批准人有权查看子空间权利人的注册
		 信息。
	 - 权利验证
	     对于有权利人的空间，当一个人连接如此空间以后，首先检查此人是否为此空间的权利人，得到其
		 权限，然后得到此空间的当前赋权安排，系统首先把这些信息提供一个缺省的鉴别程序，得到一个
		 初步的结果，然后检查此空间是否有权利的特殊身份鉴别模块（DLL），如果有，重复把这些信息
		 以及刚才得到的结果提供给由它执行鉴别，最后以他的验证结果为最终结果。

        
     - 权利人授权
	     权利人根据自己的权限可以改变当前空间的赋权状态，但如果权利人有多个人，则需表决。

*/

enum SPACE_RIGHT{   
	 UNKOWN = 0 ,     //仅编程需要，表示出错
	 FREE ,           //自由之地
     VISIT_ONLY,      //允许任何人参观
	 VISIT_NOTIFY,    //允许参观，但会向上级权利人通知
	 VISIT_APPLY,     //参观应得到上级权利人许可
	 USABLE,          //允许使用，无须申请
	 USABLE_NOTIFY,   //允许使用，但会向上级权利人通知
	 USABLE_APPLY,    //使用需得到上级权利人许可
     HOLD_NOTIFY,     //允许临时拥有，但会向上级权利人通知
	 HOLD_APPLY,      //允许临时拥有，需向上级权利人申请
	 RIGHT_HOLD       //拒绝其他人同时访问，但对高权利人无效  
};  

class CObjectData{
public:
	int64        m_ID;
	tstring      m_Name;
	SPACETYPE    m_Type;
	tstring      m_Fingerprint;
	ePipeline    m_Address; //空间路径
	int32        m_ExecuterType;
public:
	CObjectData();
	CObjectData(ePipeline& ObjectData);
	CObjectData& operator=(const CObjectData& ob);

	ePipeline* Clone();
};


//////////////////////////////////////////////////////////////////////////
/*
//Space.m_Lable      = 父空间名
  Space.m_ID         = 空间父空间ID
  Space.GetData(0)   = 空间ID
  Space.GetData(1)   = 空间名字
  Space.GetData(2)   = 空间类型
  Space.GetData(3)   = 空间指纹
  Space.GetData(4)   = 空间属性
  Space.GetData(5)   = 空间所有人

*/

class CSpace : public ePipeline{
public:
	HANDLE            m_ValidateModule;     //私有的身份验证模块(dll)，不一定有
    Mass*             m_PrivateValidata;    //从DLL中取得的执行验证的类

public:
	CSpace();
	CSpace(int64 ParentID, int64 ChildID);
		
	virtual ~CSpace(){
		if(m_PrivateValidata)delete m_PrivateValidata;
		m_PrivateValidata = NULL;
	}
	void Reset();

	virtual ENERGY_TYPE EnergyType(){ return TYPE_USER;}

	int64        GetParentID(){
		return m_ID;
	}
	
	int64        GetSpaceID();
	tstring      GetName();
	SPACETYPE    GetSpaceType();
	tstring&     GetFingerprint();
    ePipeline&   GetProperty();
	ePipeline&   GetOwnerInfo();


	void   SetName(tstring Name);
	void   SetSpaceType(SPACETYPE Type);
	void   SetFingerprint(tstring& Fingerprint);
    void   SetProerty(ePipeline& Pipe);
	void   SetOwnerInfo(ePipeline& Pipe);
public:
	bool IsValid(){return m_ID !=-1;};
	
	//根据指定的地址，从数据库找到信息填充本类
	bool Load(int64 ParentID,int64 ChildID);


	SpaceAddress GetAddress();

	static void CreateDefaultOwner(ePipeline& Pipe);
	static void CreateDefaultOwnerInfo(ePipeline& Pipe);
	static void CreateDefaultProerty(ePipeline& Pipe);

    void UpdateAll(BOOL bInsertNew);

	void UpdateOwnerInfo(); 
	void UpdateProperty();


protected:
	//标准验证,从Pipe中取得数据，返回结果给Pipe
	void DoDefaulteValidata(ePipeline* Pipe)
	{

	};
};

class People;
//物理空间 
class  ROOM_SPACE : public CSpace{
  
public:
 	ROOM_SPACE();
    ROOM_SPACE(int64 ParentID, int64 ChildID);
	ROOM_SPACE(int64 ParentID,int64 ID,tstring Name, int64 Size,SPACETYPE Type,SPACE_RIGHT r,tstring Fingerprint);
	~ROOM_SPACE();

	/*
	ePipeline{
		ID
		NAME
		TYPE
		...
		ID
		NAME
		TYPE
	}
	*/

	//在本空间加一个host，等于生成一个People，只加入最少的信息，
	//返回生成People后，用户可以再添加额外信息
	bool AddOwner(const TCHAR* Name,tstring& Cryptograph,SPACE_RIGHT r=FREE);
	
	//删除一个权利人，Who为权利人列表中的index,注意这里的所有删除仅是物理操作，没有考虑权利安排
	void DeleteOwner(People& Robot);
	
/*Robot尝试进入指定空间访问，访问必须符合以下条件：
	- 如果Robot已经在指定空间注册，则权利必须大于当前空间的赋权
	- 否则将根据本空间的赋权状态决定是否接受访问
	如果允许访问，将可能改变robot的当前权利，并把robot临时注册为指定空间的用户
*/
	bool  AllowInto(People& p);

	bool  Logon(int64 SourceID,People& Who);

	ROOM_SPACE operator = (const ROOM_SPACE& SPACE){
          ePipeline* thisPipe = (ePipeline*)this;
		  ePipeline* SpacePipe = (ePipeline*)&SPACE;

		  *thisPipe = *SpacePipe;

		  m_ValidateModule  = SPACE.m_ValidateModule;
          m_PrivateValidata = SPACE.m_PrivateValidata;

		  ROOM_SPACE& rm = const_cast<ROOM_SPACE&>(SPACE);

		  rm.m_PrivateValidata = NULL;
		  return *this;
	}

};

class People: public CSpace
{
public:
	bool                         m_AutoLeave;         //default = false
	tstring                      m_Cryptograhp;       //Logon()时生成，用来识别身份
	map<SpaceAddress,tstring>    m_KeyList;           //string是密文，是指定地址空间的解密密码

public:
	People();
	People(tstring Name,tstring Cryptograhp);
	People(int64 ParentID,int64 ID,tstring Name,tstring& Cryptograph,SPACETYPE Type,SPACE_RIGHT r,tstring Fingerprint);

	virtual ~People();

	void Reset(tstring Name,tstring Cryptograhp);
	People operator = ( People& p){
		ePipeline* thisPipe = (ePipeline*)this;
		ePipeline* PeoplePipe = (ePipeline*)&p;
		
		*thisPipe = *PeoplePipe;
	
		m_ValidateModule  = p.m_ValidateModule;
		m_PrivateValidata = p.m_PrivateValidata;

		m_Cryptograhp     = p.m_Cryptograhp;  

		People& Who = const_cast<People&>(p);
		m_KeyList.swap(Who.m_KeyList);

		Who.m_PrivateValidata = NULL;
		Who.m_Cryptograhp = _T("");
		return *this;
	};
public:

	void GoOut(); //离开当前所在空间，当前地址无效

	/*ROBOT进入一个新空间之前，会自动先离开当前的空间
	  这样如果Robot是visit将在原空间取消临时注册。
	*/
	void GoInto(int64 SourceID,ROOM_SPACE& SPACE);

	void  AddKey(SpaceAddress& Address, AnsiString& Key);
	tstring GetKey(SpaceAddress& Address);
};



#endif // _SPACE_H__
