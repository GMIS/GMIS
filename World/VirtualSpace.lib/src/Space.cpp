// Space.cpp: implementation of the CSpace class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "Space.h"
#include "shellapi.h"

#include <iostream>



CppSQLite3DB& GetWorldDB(const TCHAR* DB)
{
	static CppSQLite3DB  __World;
	if(DB){
		__World.close();
		tstring DBDir = DB;
		__World.open(DBDir);
	}
	return __World;
}


People& GetHost(People* p){
	static People*      __Host = NULL; 
    if (p)
    {
		__Host = p;
    }

	assert(__Host != NULL);
	return *__Host;
};

ROOM_SPACE& GetRootRoom(ROOM_SPACE* r){
	static ROOM_SPACE*	__RootRoom = NULL;
	if(r){
		__RootRoom = r;
	}else if (__RootRoom && !__RootRoom->IsValid())
	{
		__RootRoom->Load(ROOT_SPACE, LOCAL_SPACEID);
	}
    assert (__RootRoom != NULL);
	return *__RootRoom;
}


void CreateTable(int64 TableID)
{
	char     TableName[30]; 
    int64toa(TableID,TableName);
	CppSQLite3Buffer  SQL;          // 空间ID             空间名字         空间类型            空间指纹   空间属性   空间拥有者信息        
    SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b TEXT NOT NULL, c INTEGER NOT NULL,  d TEXT,   e TEXT,    f TEXT );",TableName);
    GetWorldDB().execDML(SQL);
}


bool HasTable(int64 TableID)
{
 	char    TableName[30]; 
    int64toa(TableID,TableName);
	return GetWorldDB().tableExists(TableName);;
    
}


bool HasChild(int64 ParentID)
{
	 if(!HasTable(ParentID))return false;
	
	 char       TableName[30]; 
    int64toa(ParentID,TableName);

     CppSQLite3Buffer SQL;
	 SQL.format("select %s from \"%s\" ",ITEM_ID,TableName);
	
	 CppSQLite3Query Result;
	 Result = GetWorldDB().execQuery(SQL);
     if(Result.eof())return false;
	 return true;
};

bool HasChild(int64 ParentID,int64 ChildID)
{
	if(!HasTable(ParentID))return false;
	
	char  Parent[30],Child[30]; 
    int64toa(ParentID,Parent);
    int64toa(ChildID,Child);
	
	CppSQLite3Buffer SQL;
	SQL.format("select * from \"%s\" where %s =  \"%s\" ",Parent,ITEM_ID,Child);
	
	CppSQLite3Query Result;
	Result = GetWorldDB().execQuery(SQL);
	if(Result.eof())return false;
	return true;
}
int32 GetAllChildList(int64 ParentID, ePipeline& ChildList,int64 NotIncludeChildID /*=0*/){
    assert(ParentID >0);
	ChildList.Clear();

	char TableName[30];
	int64toa(ParentID,TableName);
	if(!GetWorldDB().tableExists(TableName)){
		return 0;
	}

	//ChildList.SetID(ParentID);

    CppSQLite3Buffer SQL;
	SQL.format("select %s,%s,%s,%s from \"%s\"",
		     ITEM_ID,
		     ITEM_NAME,
			 ITEM_TYPE,
			 ITEM_FINGERPRINT,
		     TableName
	);

	CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
	
	while(!Result.eof()){
		int64      ChildID     = Result.getInt64Field(0);
		
		if (ChildID !=NotIncludeChildID)
		{
			AnsiString s    = Result.getStringField(1,"");
			
			tstring Name = UTF8toWS(s);
			Name = GetFileNoPathName(Name);
			Name = GetFileName(Name);
			
			int32      Type        = Result.getIntField(2);
			AnsiString FingerPrint = Result.getStringField(3,"");

			//ChildList.PushInt64(ChildID);
			ChildList.PushString(Name);
			ChildList.PushInt(Type);
			ChildList.PushString(FingerPrint);

		}
		Result.nextRow();
	}
	return ChildList.Size()/3;
}



/*
-注意：由于是递归，外部调用着应该考虑用事物回滚
*/
void DeleteAllChild(int64 ParentID)
{
	char     ParentName[30];   	
    int64toa(ParentID,ParentName);
    if(!GetWorldDB().tableExists(ParentName))return;  //指定空间没有子空间
	
	CppSQLite3Buffer SQL;
	
	//首先找到所有子空间的地址
	vector<int64> AddressList;
	
	SQL.format("select %s from \"%s\" ",
		ITEM_ID,
		ParentName
		);
	CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
	while(!Result.eof()){
		int64 ChildID  = Result.getInt64Field(0);
		AddressList.push_back(ChildID);
		Result.nextRow();
	}
	
	   
	//根据地址，执行递归操作
	for(uint32 i=0; i<AddressList.size();i++)
	{
        int64 ID = AddressList[i];
		DeleteAllChild(ID);
	}
	
	//最后删除子空间的所在的容器（表）
    if(AddressList.size() != 0){
		SQL.format("drop table \"%s\" ",ParentName);
		GetWorldDB().execDML(SQL);
	}
	
}

void DeleteChild(int64 ParentID,int64 ChildID)
{

	DeleteAllChild(ChildID);

	char     FatherName[30], ChildName[30]; 
	int64toa(ParentID,FatherName);
	int64toa(ChildID,ChildName);   

	CppSQLite3Buffer SQL;

	//在父空间里删除本空间条目
	SQL.format("delete from \"%s\" where %s = \"%s\" ",
		FatherName,
		ITEM_ID,
		ChildName);
	GetWorldDB().execDML(SQL);
}

SpaceAddress  FindChildSpace(ePipeline& ParentAddress,tstring& Fingerprint){
   SpaceAddress Addr;

   if (ParentAddress.Size()==0)
   {
	   Addr.ParentID = ROOT_SPACE;
	   Addr.ChildID  = LOCAL_SPACEID;
	   return Addr;
   }

   CppSQLite3Buffer SQL;

   int64 ParentID = 0;
   int64 ChildID  = ROOT_SPACE;

   char       ParentName[30]; 
   while (ParentAddress.Size())
   {
	   tstring Name = ParentAddress.PopString();
	   AnsiString AnsiName = WStoUTF8(Name);
	
	   ParentID = ChildID;
       ChildID  = 0;

	   int64toa(ParentID,ParentName);

	   SQL.format("select %s from \"%s\" where %s=\"%s\"",
		   ITEM_ID,
		   ParentName,
		   ITEM_NAME,
		   AnsiName.c_str()
		   );
	   CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
	   
	   if(!Result.eof()){
		   ChildID  = Result.getInt64Field(0);
		   Result.nextRow();
	   }else{
			return  Addr;
	   }
   }

   assert(ParentAddress.Size()==0);

   ParentID = ChildID;
   int64toa(ParentID,ParentName);

   AnsiString AnsiFingerprint = WStoUTF8(Fingerprint);

   SQL.format("select %s from \"%s\" where %s=\"%s\"",
	   ITEM_ID,
	   ParentName,
	   ITEM_FINGERPRINT,
	   AnsiFingerprint.c_str()
	   );

   CppSQLite3Query Result = GetWorldDB().execQuery(SQL);

   if(!Result.eof()){
	   ChildID  = Result.getInt64Field(0);
	   Result.nextRow();
   }

   Addr.ParentID = ParentID;
   Addr.ChildID  = ChildID;
   return Addr;
};
SpaceAddress  FindSpace(ePipeline& Path){
	SpaceAddress Addr;
	
	if (Path.Size()==0)
	{
		Addr.ParentID = ROOT_SPACE;
		Addr.ChildID  = LOCAL_SPACEID;
		return Addr;
	}
	
	CppSQLite3Buffer SQL;
	
	int64 ParentID = 0;
	int64 ChildID  = ROOT_SPACE;
	
	char       ParentName[30]; 
	while (Path.Size())
	{
		tstring Name = Path.PopString();
		AnsiString AnsiName = WStoUTF8(Name);
		
		ParentID = ChildID;
		ChildID  = 0;
		
		int64toa(ParentID,ParentName);
		
		SQL.format("select %s from \"%s\" where %s=\"%s\"",
			ITEM_ID,
			ParentName,
			ITEM_NAME,
			AnsiName.c_str()
			);
		CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
		
		if(!Result.eof()){
			ChildID  = Result.getInt64Field(0);
			Result.nextRow();
		}else{
			return  Addr;
		}
	}
	
	assert(Path.Size()==0);
	
	Addr.ParentID = ParentID;
	Addr.ChildID  = ChildID;
    return Addr;	
}

tstring GetFileName(tstring s)
{
	tstring::size_type n = s.find(_T('.'));
	if(n == tstring::npos)return s;
	return s.substr(0,n);
};

tstring GetFilePath(tstring FileName){
	tstring::size_type n = FileName.find_last_of(_T('\\'));
	if(n != tstring::npos){
		return FileName.substr(0,n);
	}
	n = FileName.find_last_of(_T('/'));
	if(n == tstring::npos){
		return FileName;
	}
	return FileName.substr(0,n);
};

tstring GetFileNoPathName(tstring s)
{
	tstring::size_type n = s.find_last_of(_T('\\'));
	if(n != tstring::npos){
		return s.substr(n+1);
	}
	n = s.find_last_of(_T('/'));
	if(n == tstring::npos){
		return s;
	}
	return s.substr(n);
}

tstring GetObjectType(tstring s)
{
	tstring Result;
	tstring::size_type n = s.find('.');
	if(n == tstring::npos)return Result; 
    n++;

	tstring::size_type n1 =s.find(' ',n);
	if(n1==tstring::npos)n1 = s.size();
    tstring ext = s.substr(n,n1-n);

	if(_tcsncmp(ext.c_str(),_T("ROOM"),4)==0)return _T("ROOM");
	else if(_tcsncmp(ext.c_str(),_T("PEOPLE"),6))return _T("PEOPLE");
	else if(_tcsncmp(ext.c_str(),_T("CONTAINER"),9))return _T("CONTAINER");	
	else {
		SHFILEINFO    sfi;	
		::SHGetFileInfo(
			(LPCTSTR)s.c_str(), 
			FILE_ATTRIBUTE_NORMAL,
			&sfi, 
			sizeof(SHFILEINFO), 
     		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
     
		Result = sfi.szTypeName;
		return Result;
	}
};



void  GetAllVisiter(int64 ParentID, vector<int64>& VisiterList,int64 NotIncludeChildID/*=0*/){

	char TableName[30];
	int64toa(ParentID,TableName);
	if(!GetWorldDB().tableExists(TableName)){
		return ;
	}

	CppSQLite3Buffer SQL;
	SQL.format("select %s from \"%s\" where  %s>%d",
		ITEM_ID,
		TableName,
		ITEM_TYPE,
		OBJECT_SPACE
		);

	CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
	if(Result.eof())return ;

	while(!Result.eof()){
		int64  ChildID = Result.getInt64Field(0);
		if (ChildID != NotIncludeChildID)
		{
			VisiterList.push_back(ChildID);			
		}
		Result.nextRow();
	}
}


tstring GetLocalSpaceName(){


	if(!GetWorldDB().tableExists(LOCAL_SPACE_STR)){
		return _T("");
	}

	CppSQLite3Buffer SQL;
	SQL.format("select %s from \"%s\" where %s = \"%s\" ",
		ITEM_NAME,
		ROOT_SPACE_STR,
		ITEM_ID,
		LOCAL_SPACE_STR
		);


	CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
	if(Result.eof())return _T("");

	AnsiString utf8Name = Result.getStringField(0,"");

	tstring Name = UTF8toWS(utf8Name);
	return  Name;
}


//////////////////////////////////////////////////////////////////////////
SpaceAddress::SpaceAddress()
:ParentID(0),ChildID(0){
};

SpaceAddress::SpaceAddress(int64 Father, int64 Child)
:ParentID(Father),ChildID(Child){

};

SpaceAddress::SpaceAddress(const SpaceAddress& Address)
:ParentID(Address.ParentID),ChildID(Address.ChildID){

};
bool SpaceAddress::IsValid(){
	return (ParentID >0 && ChildID >0 && ParentID != ChildID);
};
void SpaceAddress::Reset(){
	ParentID =0; ChildID = 0;
} 
SpaceAddress& SpaceAddress::operator =(const SpaceAddress& Address) {
	ParentID=Address.ParentID;
	ChildID=Address.ChildID;
	return *this;
};
bool SpaceAddress::operator < (const SpaceAddress& Address) {
	return (ParentID<Address.ParentID || ((ParentID == Address.ParentID)&&(ChildID<Address.ChildID) ));
};
bool SpaceAddress::operator > (const SpaceAddress Address){
	return  !(ParentID<Address.ParentID || ((ParentID == Address.ParentID)&&(ChildID<Address.ChildID)) );
};
bool SpaceAddress::operator == (const SpaceAddress Address){
	return (ParentID==Address.ParentID && ChildID == Address.ChildID);
};

//////////////////////////////////////////////////////////////////////////

CObjectData::CObjectData(ePipeline& ObjectData){
	if (ObjectData.Size()!=4)
	{
		return;
	}
	m_ID   = ObjectData.GetID();
	m_Name = *(tstring*)ObjectData.GetData(0);
	m_Type = *(SPACETYPE*)ObjectData.GetData(1);
	m_Fingerprint = *(tstring*)ObjectData.GetData(2);
	ePipeline* Path = (ePipeline*)ObjectData.GetData(3);
	m_Address << *Path;
}
CObjectData& CObjectData::operator =(const CObjectData& ob){
	if(this != &ob){
		m_ID   = ob.m_ID;
		m_Name = ob.m_Name;
		m_Type = ob.m_Type;
		m_Fingerprint = ob.m_Fingerprint;
		m_Address = ob.m_Address;
	}
	return *this;
}

ePipeline* CObjectData::GetItemData(){
	
	ePipeline* Item = new ePipeline(m_ID);
	Item->PushString(m_Name);
	Item->PushInt((int64)m_Type);
	Item->PushString(m_Fingerprint);
	Item->Push_Directly(m_Address.Clone());
	return Item;
}


//////////////////////////////////////////////////////////////////////////

CSpace::CSpace()
:m_ValidateModule(NULL),m_PrivateValidata(NULL)
{
	m_ID = -1;  //ParentID

	PushInt(-1);          //Space ID
	PushString(_T(""));  //Space Name
	PushInt(LOCAL_SPACE);//Space Type

	tstring Fingerprint;
    PushString(Fingerprint); //Space Fingerprint

	ePipeline Property;
	CreateDefaultProerty(Property); //Space Property;
	PushPipe(Property);

	ePipeline OwnerInfo;
	CreateDefaultOwnerInfo(OwnerInfo); //Space OwnerList
	PushPipe(OwnerInfo);

};

CSpace::CSpace(int64 ParentID, int64 ChildID)
:m_ValidateModule(NULL),m_PrivateValidata(NULL)
{

	m_ID = -1;  //ParentID
	
	PushInt(-1);          //Space ID
	PushString(_T(""));  //Space Name
	PushInt(LOCAL_SPACE);//Space Type
	
	tstring Fingerprint;
    PushString(Fingerprint); //Space Fingerprint
	
	ePipeline Property;
	CreateDefaultProerty(Property); //Space Property;
	PushPipe(Property);
	
	ePipeline OwnerInfo;
	CreateDefaultOwnerInfo(OwnerInfo); //Space OwnerList
	PushPipe(OwnerInfo);

	
	if(!Load(ParentID,ChildID)){
		m_ID = -1;	
	};
}

int64   CSpace::GetSpaceID(){
    assert(Size()>5);
	assert(GetDataType(0) == TYPE_INT);
	int64 SpaceID = *(int64*)GetData(0);
	return SpaceID;
};

tstring  CSpace::GetName(){
	assert(Size()>5);
	assert(GetDataType(1)==TYPE_STRING);
	tstring* Name = (tstring*)GetData(1);
	return *Name;
};

SPACETYPE    CSpace::GetSpaceType(){
	assert(Size()>5);
	assert(GetDataType(2) == TYPE_INT);
	int64 Type =*(int64*)GetData(2);
	return (SPACETYPE)Type;
};


tstring& CSpace::GetFingerprint(){
	assert(Size()>5);
	assert(GetDataType(3) == TYPE_STRING);
	tstring& Fingerprint = *(tstring*)GetData(3);
	return Fingerprint;	
}

ePipeline&   CSpace::GetProperty(){
	assert(Size()>5);
	assert(GetDataType(4) == TYPE_PIPELINE);
	ePipeline* Property = (ePipeline*)GetData(4);
	return *Property;
};

ePipeline&   CSpace::GetOwnerInfo(){
	assert(Size()>5);
	assert(GetDataType(5) == TYPE_PIPELINE);
	ePipeline* OwnerInfo = (ePipeline*)GetData(5);
	return *OwnerInfo;
}

void   CSpace::SetName(tstring Name){
	assert(Size()>5);
	assert(GetDataType(1)==TYPE_STRING);
	tstring* OldName = (tstring*)GetData(1);
	*OldName = Name;
};

void   CSpace::SetSpaceType(SPACETYPE Type){
	assert(Size()>5);
	assert(GetDataType(2) == TYPE_INT);
	int64* OldType = (int64*)GetData(2);
	*OldType = (int64)Type;
};

void   CSpace::SetFingerprint(tstring& Fingerprint){
	assert(Size()>5);
	assert(GetDataType(3)==TYPE_STRING);
	tstring* OldFingerprint = (tstring*)GetData(3);
	*OldFingerprint = Fingerprint;	
};


void   CSpace::SetProerty(ePipeline& Pipe){
	assert(Size()>5);
	assert(Pipe.Size());

	assert(GetDataType(4) == TYPE_PIPELINE);
	ePipeline* Property = (ePipeline*)GetData(4);
	Property->Clear();
	*Property << Pipe;
};

void   CSpace::SetOwnerInfo(ePipeline& Pipe){
	assert(Size()>5);
	assert(Pipe.Size());
	
	assert(GetDataType(5) == TYPE_PIPELINE);
	ePipeline* OwnerInfo = (ePipeline*)GetData(5);

	OwnerInfo->Clear();
	*OwnerInfo << Pipe;
};


bool CSpace::Load(int64 ParentID,int64 ChildID){
	m_ID = ParentID;
	
	char     ParentName[30], ChildName[30]; 
	int64toa(ParentID,ParentName);
	int64toa(ChildID,ChildName);
	
	CppSQLite3Buffer SQL;
	SQL.format("select * from \"%s\" where %s = \"%s\" ",
		ParentName,
		ITEM_ID,
		ChildName  
	);

	CppSQLite3Query Result = GetWorldDB().execQuery(SQL);
	if(Result.eof()){
		return false;
	}
	assert(Size()>5);
	assert(GetDataType(0) == TYPE_INT);
	int64& OldID = *(int64*)GetData(0);
	OldID = ChildID;

	AnsiString AnsiName = Result.getStringField(1,"");	
	tstring Name = UTF8toWS(AnsiName);
	SetName(Name);

	SPACETYPE Type = (SPACETYPE)Result.getIntField(2);
	SetSpaceType(Type);

	AnsiString Fingerprint = Result.getStringField(3);
	tstring Fingerprint1 = UTF8toWS(Fingerprint);
    SetFingerprint(Fingerprint1);
	
	AnsiString AnsiProperty = Result.getStringField(4,"");     	
	ePipeline Property;
	uint32 pos =0;
	bool ret = Property.FromString(AnsiProperty,pos);
	if(!ret){
		return false;
	}
    SetProerty(Property);

	AnsiString AnsiOwner = Result.getStringField(5,"");
	ePipeline OwnerInfo;

	pos=0;
	ret = OwnerInfo.FromString(AnsiOwner,pos);
	if(!ret){
		return false;
	}  
	SetOwnerInfo(OwnerInfo);

	return true;
}


SpaceAddress CSpace::GetAddress(){
	assert(IsValid());
	return 	SpaceAddress(GetParentID(),GetSpaceID());
};


void CSpace::CreateDefaultOwnerInfo(ePipeline& Pipe){
	Pipe.PushInt(NO_RIGHT);
    ePipeline Empty;
	Pipe.PushPipe(Empty);
}

void CSpace::CreateDefaultOwner(ePipeline& Pipe)
{
    tstring  Name = _T("Unkown");
	tstring  Cryptograhp = _T("");
	SPACE_RIGHT  Right = NO_RIGHT;
    tstring  Email = _T("Unkown");
    tstring  Memo = _T("Unkown");
   
    eBLOB* Validata = new eBLOB();   
	
    Pipe.PushString(Name);
    Pipe.PushInt(Right);
    Pipe.PushString(Cryptograhp);
    Pipe.PushString(Email);
    Pipe.PushString(Memo);  
    Pipe.Push_Directly(Validata);
};

void CSpace::CreateDefaultProerty(ePipeline& Pipe){
	Pipe.PushInt(0);
	Pipe.PushString(_T(""));
	ePipeline Empty;
	Pipe.PushPipe(Empty);
	//目前只有这些，其它后续再添加
}

void CSpace::UpdateAll(BOOL bInsertNew){
  
	if(!IsValid())return ;

	int64 ParentID = m_ID;
	
	AnsiString Name;
	Name = WStoUTF8(GetName());

	int64 ChildID  = GetSpaceID();
	if (bInsertNew)
    {
		if(ChildID== -1){
			ChildID = AbstractSpace::CreateTimeStamp();
		}
    }


	SPACETYPE Type = GetSpaceType();

	AnsiString Fingerprint = WStoUTF8(GetFingerprint());

	AnsiString Property;
	GetProperty().ToString(Property);

	AnsiString OwnerInfo;
	GetOwnerInfo().ToString(OwnerInfo);
	    
	CppSQLite3Buffer SQL;
	char     ParentName[30], ChildName[30]; 
    int64toa(ParentID,ParentName);
    int64toa(ChildID,ChildName);
	
	if (bInsertNew)
	{
		if (!HasTable(ParentID))
		{
			CreateTable(ParentID);
		}
		SQL.format(" insert into \"%s\" values(\"%s\",\"%s\",%d,\"%s\",\"%s\" ,\"%s\");",		         
			ParentName,
			ChildName,
			Name.c_str(),
			Type,
			Fingerprint.c_str(),
			Property.c_str(),
			OwnerInfo.c_str()
			);		
	}else{
		
		SQL.format("update \"%s\" set %s=\"%s\" %s=%d %s=\"%s\" %s=\"%s\" %s=\"%s\" where %s = \"%s\" ;",		         
			ParentName,
			ITEM_NAME,
			Name.c_str(),
			ITEM_TYPE,
			Type,
			ITEM_FINGERPRINT,
			Fingerprint.c_str(),
			ITEM_PROPERTY,
			Property.c_str(),
			ITEM_OWNER,
			OwnerInfo.c_str(),
			ITEM_ID,
			ChildName
			);
	}
    GetWorldDB().execDML(SQL);
};

void CSpace::UpdateOwnerInfo(){
	if(!IsValid())return;

	AnsiString OwnerInfo;
	GetOwnerInfo().ToString(OwnerInfo);
	
	int64 ParentID = GetParentID();
	int64 ChildID  = GetSpaceID();

	CppSQLite3Buffer SQL;
	char     ParentName[30], ChildName[30]; 
    int64toa(ParentID,ParentName);
    int64toa(ChildID,ChildName);
	
	SQL.format("update \"%s\" set %s=\"%s\" where %s = \"%s\" ;",		         
		ParentName,
		ITEM_OWNER,
		OwnerInfo.c_str(),
		ITEM_ID,
		ChildName
		);
    GetWorldDB().execDML(SQL);
};

void CSpace::UpdateProperty(){
	if(!IsValid())return;
	
	AnsiString Property;
	GetProperty().ToString(Property);
	
	int64 ParentID = GetParentID();
	int64 ChildID  = GetSpaceID();

	CppSQLite3Buffer SQL;
	char     ParentName[30], ChildName[30]; 
    int64toa(ParentID,ParentName);
    int64toa(ChildID,ChildName);
	
	SQL.format("update \"%s\" set %s=\"%s\" where %s = \"%s\" ;",		         
		ParentName,
		ITEM_OWNER,
		Property.c_str(),
		ITEM_ID,
		ChildName
		);
    GetWorldDB().execDML(SQL);
};


//////////////////////////////////////////////////////////////////////////
ROOM_SPACE::ROOM_SPACE(){
	
};

ROOM_SPACE::ROOM_SPACE(int64 ParentID, int64 ChildID)
	:CSpace(ParentID,ChildID){

};

ROOM_SPACE::ROOM_SPACE(int64 ParentID,int64 ID,tstring Name, int64 Size,SPACETYPE Type,SPACE_RIGHT r,tstring Fingerprint)
{
	assert(ParentID);
	assert(ID);

	m_ID  = ParentID;
	int64& ChilID = *(int64*)GetData(0);
	ChilID = ID;

	SetName(Name);
	SetSpaceType(Type);
    SetFingerprint(Fingerprint);

	*(int64*)GetProperty().GetData(PROPERTY_SIZE) = Size;
	*(SPACE_RIGHT*)GetOwnerInfo().GetData(REG_RIGHT) = r;

	UpdateAll(TRUE);
}


ROOM_SPACE::~ROOM_SPACE(){
}



bool ROOM_SPACE::AddOwner(const TCHAR* Name,tstring& Cryptograph,SPACE_RIGHT r)
{
	assert(IsValid());
    if (!IsValid())
    {
		return false;
    }

	ePipeline& OwnerInfo = GetOwnerInfo();
    ePipeline& OwnerList = *(ePipeline*)OwnerInfo.GetData(REG_OWNER_LIST);


	ePipeline Owner;
	CreateDefaultOwner(Owner);

	*(tstring*)Owner.GetData(0) = Name;
    *(int64*)Owner.GetData(1) = (int64)r;
    *(tstring*)Owner.GetData(2) = Cryptograph;
  
	OwnerList.PushPipe(Owner);

    UpdateOwnerInfo();

	return true;
}

void ROOM_SPACE::DeleteOwner(People& p)
{

	ePipeline& OwnerInfo = GetOwnerInfo();
    ePipeline& OwnerList = *(ePipeline*)OwnerInfo.GetData(REG_OWNER_LIST);

	for (int i=0; i<OwnerInfo.Size();i++)
	{
		ePipeline* Owner = (ePipeline*)OwnerList.GetData(i);
		tstring& Cryptograph = *(tstring*)Owner->GetData(2);
		if (Cryptograph == p.m_Cryptograhp)
		{
			OwnerList.EraseEnergy(i,1);
			return ;
		}
	}

	if(p.GetParentID() == GetSpaceID()){
		p.GoOut();
		
		//这里应该有更多处理，p被赶出此空间，那么应该退回到上一个空间
		assert(0);
		return;
	}

}


/*
 - 首先检查ROBOT是否已经注册为指定空间的Owner（成为其子空间）
   注册分为两种，一种是固定注册，即空间对此有固定的权利
   一种是临时注册，即被允许临时访问此空间，但访问完以后将被删除。
   
   Q：如何检测一个ROBOT是否在一个空间注册？
   A: 首先用ROBOT的指纹在空间里找子空间，然后对照权利，即可以确定身份。

   Q：如何区别一个ROBOT的注册类型？
   A: 临时ROBOT为ROBOT_VISIT
 

  - 得到指定空间的赋权
  - 比较当前两者的权利，确定ROBOT是否可以访问此空间,如果不能访问返回FALSE
  - 否则确定当前ROBOT的权利，让Robot离开原空间
  - 如果ROBOT不是指定空间固定注册用户，则注册其为当前空间的子空间（ROBOT_VISIT）
  - 修改ROBOT当前地址
*/

bool  ROOM_SPACE::AllowInto(People& p){
	if (!IsValid())
	{
		return false;
	}

    //如果已经在此空间则直接返回
	if(p.GetParentID() == m_ID)return true;
    
	int64 ChildID = 0;


	ePipeline& OwnerInfo = GetOwnerInfo();
	SPACE_RIGHT RoomRight = *(SPACE_RIGHT*)OwnerInfo.GetData(REG_RIGHT);

	ePipeline& OwnerList = *(ePipeline*)OwnerInfo.GetData(REG_OWNER_LIST);

	for (int i=0; i<OwnerList.Size();i++)
	{
		ePipeline* Owner = (ePipeline*)OwnerList.GetData(i);
		tstring& Cryptograph = *(tstring*)Owner->GetData(2);
		if (Cryptograph == p.m_Cryptograhp)
		{
			SPACE_RIGHT Right = *(SPACE_RIGHT*)Owner->GetData(1);
			if(Right >= RoomRight){
				return true;
			}
			break;
		}
	}

    //访问者不是固定注册的拥有者，但只要当前空间不禁止参观也可以进入
	bool Allow = false;

	switch(RoomRight)
	{
	case NO_RIGHT:
		{   //能够访问，robot的访问权限保持不变
			Allow = true;	
		}
		break;
	case VISIT_ONLY:
		{
			Allow = true;
		}
		break;
	case VISIT_NOTIFY:
		{
			//通知其它人，暂时没有实现
			assert(0);
		}
		break;
	case VISIT_APPLY:
		{
			//提出申请，暂时没有实现
			assert(0);
		}
	case USABLE:
		{
			Allow = true;
		}
		break;
	case USABLE_NOTIFY:
		{
			//通知其它人，暂时没有实现
			assert(0);
		}
		break;
	case USABLE_APPLY:
		{
			//提出申请，暂时没有实现
			assert(0);
		}
		break;
	case HOLD_NOTIFY:
		{
			//通知其它人，暂时没有实现
			assert(0);
		}
		break;
	case HOLD_APPLY:
		{
			//提出申请，暂时没有实现
			assert(0);
		}
		break;
	case RIGHT_HOLD:
		{
			//显然不允许
		}
		break;
	}
	return Allow;
}

bool  ROOM_SPACE::Logon(int64 SourceID,People& Who){
	if(AllowInto(Who)){
		Who.GoInto(SourceID,*this);
		return true;
	}
	else return false;
}
//////////////////////////////////////////////////////////////////////////

People::People()
{
	m_AutoLeave= true;
	SetSpaceType(ROBOT_VISITER);
};
People::People(tstring Name,tstring Cryptograhp)
	:m_AutoLeave(true){
		m_Cryptograhp=Cryptograhp;
		SetName(Name);
};



People::People(int64 ParentID,int64 ID,tstring Name,tstring& Cryptograph,SPACETYPE Type,SPACE_RIGHT r,tstring Fingerprint)
{   

	assert(ParentID);
	assert(ID);
	
	m_AutoLeave = true;

	m_ID  = ParentID;
	int64& ChilID = *(int64*)GetData(0);
	ChilID = ID;
	
	SetName(Name);
	SetSpaceType(Type);
    SetFingerprint(Fingerprint);
	
	*(SPACE_RIGHT*)GetOwnerInfo().GetData(REG_RIGHT) = r;
	
	UpdateAll(TRUE);
}

People::~People(){
	if(m_AutoLeave){
		GoOut();
	}
};

void People::GoOut(){
	if(IsValid()){
		int64 ParentID = GetParentID();
		int64 ChildID  = GetSpaceID();
		DeleteChild(ParentID,ChildID);
		m_ID = -1;
	}
}

void People::GoInto(int64 SourceID,ROOM_SPACE& Room){

	//先离开之前所在的空间,然后ROBOT设置新的地址
	GoOut();

	//如果没有注册 则注册为子空间，robot权利不变
	int64 RoomID = Room.GetSpaceID();

	SetID(RoomID); //ParentID
    
	int64& ChilID = *(int64*)GetData(0);
	ChilID = SourceID;

	SetSpaceType(ROBOT_VISITER);
		
	UpdateAll(TRUE);
}

void  People::AddKey(SpaceAddress& Address, AnsiString& Key){
	
	assert(m_KeyList.find(Address) == m_KeyList.end());
	m_KeyList[Address] = UTF8toWS(Key);
};

tstring People::GetKey(SpaceAddress& Address){
	tstring  Key;
	map<SpaceAddress,tstring>::iterator It = m_KeyList.find(Address);
	if(It!= m_KeyList.end())return  It->second;
	return Key;
}


//////////////////////////////////////////////////////////////////////////



