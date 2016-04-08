// CPokerDatabase.cpp: implementation of the CPokerDatabase class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "MainFrame.h"
#include "PokerDatabase.h"
#include <memory>



CObjectData::CObjectData(){

}

CObjectData::~CObjectData(){
	Reset();
}

void CObjectData::Reset()
{
	if (Size()==10)
	{
		uint32& handle = *(uint32*)GetData(9);
		if (handle)
		{
			HBITMAP hbitmap = (HBITMAP)handle;
			::DeleteObject(hbitmap);
		}
	}
	Clear();
}

void CObjectData::SetImage(HBITMAP hBitmap)
{
	if (Size()==10)
	{
		uint32& handle = *(uint32*)GetData(9);
		if (handle)
		{
			HBITMAP hbitmap = (HBITMAP)handle;
			::DeleteObject(hbitmap);
		}
		handle = (uint32)hBitmap;
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPokerDatabase::CPokerDatabase()
{



}

CPokerDatabase::~CPokerDatabase()
{

}

BOOL CPokerDatabase::Open(AnsiString DBFileName){
  
	m_Database.close();
	m_Database.open(DBFileName.c_str());

	if(!m_Database.tableExists(POKER_TABLE)){
		CppSQLite3Buffer SQL;
		SQL.format("CREATE TABLE \"%s\" (Name TEXT NOT NULL UNIQUE,Type INTEGER,x INTEGER,y INTEGER,w INTEGER,h INTEGER,crc32 INTEGER, Time TEXT, Image BLOB);",POKER_TABLE);
		m_Database.execDML(SQL);
	}

	return TRUE;
}

void CPokerDatabase::GetObjectList(tstring FileName,vector<CObjectData>& ObjectList){

	FILE *fp;
	
	int ID = 0;
		
	if((fp=_tfopen(FileName.c_str(),_T("rb")))==NULL)
	{
		return;
	}
	

	//要求文本编码为UTF-8
	char buf[256];	
	while(fgets(buf,255,fp)){
		
		AnsiString s = buf;		
		tstring str = eSTRING::UTF8TO16(s);
		
		tstring::size_type n = str.find_first_of(_T('='));
		if(n==str.npos){
			continue;
		}
		
		tstring Name = str.substr(0,n);
		
		if (Name.size()==0)
		{
			break;
		}
		
		n++;
		int32 p=n;
		n = str.find_last_of(_T(';'));
		tstring TypeStr = str.substr(p,n-p);
		
		OBJECTTYPE ObjectType;
		if (TypeStr == _T("NUM"))
		{			
			ObjectType = NUM_OBJECT;
		}else if (TypeStr == _T("TXT"))
		{			
			ObjectType = TEXT_OBJECT;			
		}
		else if (TypeStr == _T("LABEL"))
		{
			ObjectType = LABEL_OBJECT;
		}
		else if(TypeStr == _T("BUTTON")){
			ObjectType = BUTTON_OBJECT;
		}

		CObjectData ObjectData;		
		GetObject(ID,Name,ObjectData);
		
		if (ObjectData.Size()==0) //数据库没有记录则表示是新物体
		{
			ObjectData.PushInt(ID);
			ObjectData.PushString(Name);
			ObjectData.PushInt(ObjectType);	
		}else
		{
		    int32& Type = *(int32*)ObjectData.GetData(2);
			if (Type != ObjectType)
			{
				Type = ObjectType;
				SaveObject(ObjectData);
			}
		}
		
		assert(ObjectData.Size());
		ObjectList.push_back(ObjectData);

		ID++;
	}
	fclose(fp);
}

void CPokerDatabase::DeleteObject(tstring& Name){
	AnsiString ansiName;
	eSTRING temp1(Name);
	temp1.ToString(ansiName);

	CppSQLite3Buffer SQL;
	SQL.format("delete  from \"%s\" where Name = \"%s\";",POKER_TABLE,ansiName.c_str());	
	m_Database.execDML(SQL);
}

void CPokerDatabase::SaveObject(CObjectData& ObjectData){

    tstring Name = *(tstring*)ObjectData.GetData(1);
	AnsiString ansiName = eSTRING::UTF16TO8(Name);
	
	int32 Type = *(int32*)ObjectData.GetData(2);
	int32 x    = *(int32*)ObjectData.GetData(3);
	int32 y    = *(int32*)ObjectData.GetData(4);
	int32 w    = *(int32*)ObjectData.GetData(5);
	int32 h    = *(int32*)ObjectData.GetData(6);

	if (w==0 || h==0)
	{
		return;
	};

	uint32 crc32 = *(uint32*)ObjectData.GetData(7);
	
	tstring TimeStr = ObjectData.GetLabel();
	AnsiString ansiTime = eSTRING::UTF16TO8(TimeStr);
	CppSQLite3Buffer SQL;

	SQL.format("select * from \"%s\" where Name = \"%s\";",POKER_TABLE,ansiName.c_str());
	
	CppSQLite3Query Result = m_Database.execQuery(SQL);
	
	if(!Result.eof()){
		SQL.format("update \"%s\" set Type=%d,x=%d,y=%d,w=%d,h=%d,crc32=%u,Time=\"%s\",Image=? where Name=\"%s\" ",
			POKER_TABLE,
			Type,
			x,y,w,h,
			crc32,
		    ansiTime.c_str(),
  		    ansiName.c_str());
	}else{
		SQL.format("insert into \"%s\" values (\"%s\",%d,%d,%d,%d,%d,%u,\"%s\",?)",
			POKER_TABLE,
			ansiName.c_str(),
			Type,
			x,y,w,h,
			crc32,
			ansiTime.c_str());
	}
    CppSQLite3Statement State = m_Database.compileStatement(SQL);

	HBITMAP hBitmap = ObjectData.GetImage();

	CBitmapEx Image;
	Image.Load(hBitmap);

	long ImageSize = Image.GetSaveSize();
	LPBYTE ImageData = new BYTE[ImageSize];
	Image.Save(ImageData,ImageSize);
	assert(ImageSize);
	std::auto_ptr<BYTE> ptr(ImageData);
	State.bind(1,ImageData,ImageSize); //替换第一个问号

    State.execDML();

	//delete ImageData;   
};

BOOL CPokerDatabase::GetObject(int64 ID,tstring& Name,CObjectData& ObjectData){


	AnsiString ansiName = eSTRING::UTF16TO8(Name);

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
		

    try{
		//	m_LogDatabase.open("d:\\Log.DB");
		
		SQL.format("select * from \"%s\" where Name = \"%s\";",POKER_TABLE,ansiName.c_str());
		
		Result = m_Database.execQuery(SQL);
		
		while(!Result.eof()){
			int32     Type = Result.getIntField(1);
			int32      x    = Result.getIntField(2);
			int32      y    = Result.getIntField(3);
			int32      w    = Result.getIntField(4);
			int32      h    = Result.getIntField(5);
			uint32   crc32  = Result.getInt64Field(6);
			AnsiString ansiTime = Result.getStringField(7);
			
		    ObjectData.GetLabel() = eSTRING::UTF8TO16(ansiTime);

			int32 ImagLen;
			const unsigned char* ImagData = Result.getBlobField(8,ImagLen);
			
		    ObjectData.PushInt(ID);  
			ObjectData.PushString(Name);
			ObjectData.PushInt(Type);
			ObjectData.PushInt(x);
			ObjectData.PushInt(y);
			ObjectData.PushInt(w);
			ObjectData.PushInt(h);
			ObjectData.PushInt(crc32);
			
			tstring ocrTxt ;
			ObjectData.PushString(ocrTxt);

			CBitmapEx Image;
			Image.Load((LPBYTE)ImagData);
			HBITMAP hBitmap = NULL;
			Image.Save(hBitmap);
			
		    ObjectData.PushInt((uint32)hBitmap);

			Result.nextRow();
			assert(Result.eof());
		}
		
	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}	
	return ObjectData.Size()>0;
};

void CPokerDatabase::GetObjectList(vector<CObjectData>& ObjectList)
{

	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;

	int ID = 0;

	AnsiString s;
	try{

		SQL.format("select * from \"%s\" ;",POKER_TABLE);

		Result = m_Database.execQuery(SQL);

		while(!Result.eof()){
			AnsiString Name = Result.getStringField(0);
			int32     Type = Result.getIntField(1);
			int32      x    = Result.getIntField(2);
			int32      y    = Result.getIntField(3);
			int32      w    = Result.getIntField(4);
			int32      h    = Result.getIntField(5);
			uint32   crc32  = Result.getInt64Field(6);
			AnsiString ansiTime = Result.getStringField(7);

			CObjectData ObjectData;
			ObjectData.GetLabel() = eSTRING::UTF8TO16(ansiTime);

			int32 ImagLen;
			const unsigned char* ImagData = Result.getBlobField(8,ImagLen);

			ObjectData.PushInt(ID++);  
			ObjectData.PushString(Name);
			ObjectData.PushInt(Type);
			ObjectData.PushInt(x);
			ObjectData.PushInt(y);
			ObjectData.PushInt(w);
			ObjectData.PushInt(h);
			ObjectData.PushInt(crc32);

			tstring ocrTxt ;
			ObjectData.PushString(ocrTxt);

			CBitmapEx Image;
			Image.Load((LPBYTE)ImagData);
			HBITMAP hBitmap = NULL;
			Image.Save(hBitmap);

			ObjectData.PushInt((uint32)hBitmap);

			assert(ObjectData.Size());
			ObjectList.push_back(ObjectData);

			Result.nextRow();
		}

	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}	
}
void CPokerDatabase::ClearObject(){
	try{
				
		CppSQLite3Buffer SQL;

		SQL.format("drop table %s",POKER_TABLE);
		m_Database.execDML(SQL);	
	
		SQL.format("CREATE TABLE \"%s\" (Name TEXT NOT NULL UNIQUE,Type INTEGER,x INTEGER,y INTEGER,w INTEGER,h INTEGER,crc32 INTEGER, Time TEXT, Image BLOB);",POKER_TABLE);
		m_Database.execDML(SQL);

		
	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}
};

void CPokerDatabase::SaveCrc32(uint32 crc32, tstring ocrStr,HBITMAP hBitmap){
    const char* TableName2 = "Crc32List";
	if(!m_Database.tableExists(TableName2)){
		CppSQLite3Buffer SQL;
		SQL.format("CREATE TABLE \"%s\" ( crc32 INTEGER NOT NULL UNIQUE,  ocr TEXT, Image BLOB);",TableName2);
		m_Database.execDML(SQL);
	}

	if(crc32==0)return;
	if(hBitmap==0)return;
	
	AnsiString ansiOcrStr;
	eSTRING temp1(ocrStr);
	temp1.ToString(ansiOcrStr);

	char buf[20];
	ultoa(crc32,buf,10);
	
	const char* TableName = "Crc32List";
	
	CppSQLite3Buffer SQL;
	
	SQL.format("select * from \"%s\" where crc32 = \"%s\";",TableName,buf);
	
	CppSQLite3Query Result = m_Database.execQuery(SQL);
	
	if(!Result.eof()){
		if(ocrStr.size()==0){ //不在重复存储
			return;
		}
		SQL.format("update \"%s\" set ocr=\"%s\",Image=? where crc32=\"%s\";",
			TableName,
			ansiOcrStr.c_str(),
			buf);
	}else{
		SQL.format("insert into \"%s\" values (\"%s\",\"%s\",?)",
			TableName,
			buf,	
			ansiOcrStr.c_str());
	}
    CppSQLite3Statement State = m_Database.compileStatement(SQL);
	
	CBitmapEx Image;
	Image.Load(hBitmap);
	
	long ImageSize = Image.GetSaveSize();
	LPBYTE ImageData = new BYTE[ImageSize];
	if (!ImageData)
	{
		return;
	}

	::memset(ImageData,0,ImageSize);
	std::auto_ptr<BYTE> ptr(ImageData);

    ImageSize = Image.Save(ImageData,ImageSize);
    assert(ImageSize);
	if (ImageSize==0)
	{
		return;
	}
	
    State.bind(1,ImageData,ImageSize); //替换第一个问号
    
    State.execDML();
}
void CPokerDatabase::UpdateCrc32(uint32 crc32,  tstring ocrStr){
	if(ocrStr.size()==0){
		return;
	}
	char buf[20];
	ultoa(crc32,buf,10);
	
	AnsiString ansiOcrStr;
	eSTRING temp1(ocrStr);
	temp1.ToString(ansiOcrStr);
	
	const char* TableName = "Crc32List";
	
	CppSQLite3Buffer SQL;
	
	SQL.format("select * from \"%s\" where crc32 = \"%s\";",TableName,buf);
	
	CppSQLite3Query Result = m_Database.execQuery(SQL);
	
	if(!Result.eof()){
		
		SQL.format("update \"%s\" set ocr=\"%s\" where crc32=\"%s\";",
			TableName,
			ansiOcrStr.c_str(),
			buf
			);
	}
    CppSQLite3Statement State = m_Database.compileStatement(SQL);
	State.execDML();
}

tstring CPokerDatabase::GetCrc32(uint32 crc32){
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	
	const char* TableName = "Crc32List";
	char buf[20];
	ultoa(crc32,buf,10);
	
	tstring ocrStr; 
    try{
		//	m_LogDatabase.open("d:\\Log.DB");
		
		SQL.format("select * from \"%s\" where crc32=\"%s\";",TableName,buf);
		
		Result = m_Database.execQuery(SQL);
		
		while(!Result.eof()){
					
			AnsiString ansiocrTxt = Result.getStringField(1);	
			eSTRING s2; 
			s2.FromString(ansiocrTxt,0);
			tstring ocrStr = s2();		
			
			Result.nextRow();
			assert(Result.eof());
			
			ocrStr;
		}
		
	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}	
	return ocrStr;
}

void CPokerDatabase::GetCrc32List(ePipeline& Pipe){
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	
	const char* TableName = "Crc32List";
	if(!m_Database.tableExists(TableName)){
		CppSQLite3Buffer SQL;
		SQL.format("CREATE TABLE \"%s\" ( crc32 INTEGER NOT NULL UNIQUE,  ocr TEXT, Image BLOB);",TableName);
		m_Database.execDML(SQL);
	}	
    try{
		
		SQL.format("select * from \"%s\";",TableName);
		
		Result = m_Database.execQuery(SQL);
		
		while(!Result.eof()){
			uint32   crc32  = Result.getInt64Field(0);	
			AnsiString ansiocrTxt = Result.getStringField(1);
			
			eSTRING s2; 
			s2.FromString(ansiocrTxt,0);
			tstring ocrTxt = s2();
			
			Pipe.PushInt(crc32);
			Pipe.PushString(ocrTxt);
			
			Result.nextRow();
		}
		
	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}	
}

void CPokerDatabase::ClearCrc32(){
	try{
		
		CppSQLite3Buffer SQL;
		const char* TableName = "Crc32List";
		SQL.format("drop table %s",TableName);
		m_Database.execDML(SQL);	
		
		//建立新表
		SQL.format("CREATE TABLE \"%s\" ( crc32 INTEGER NOT NULL UNIQUE, ocr TEXT, Image BLOB);",TableName);
		m_Database.execDML(SQL);
		
	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}
}
