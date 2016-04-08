// LogDatabase.cpp: implementation of the CLogDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "LogDatabase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogDatabase::CLogDatabase()
{
	m_LogDatabase.open("d:\\Log.DB");

	const char* TableName = TABLENAME;
	if(!m_LogDatabase.tableExists(TableName)){
		CppSQLite3Buffer SQL;
		SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b TEXT NOT NULL,c TEXT NOT NULL);",TableName);
		m_LogDatabase.execDML(SQL);
	}
//	m_LogDatabase.close();
}

CLogDatabase::~CLogDatabase()
{
	m_LogDatabase.close();
}

void CLogDatabase::WriteItem(int64 TimeStamp,const TCHAR* whosay, const TCHAR* text){
	assert(whosay);
	assert(text);
    try{

//	m_LogDatabase.open("d:\\Log.DB");
 
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;

	const char* TableName = TABLENAME;

	AnsiString s1, s2;
	eSTRING temp1(whosay);
	temp1.ToString(s1);

	eSTRING temp2(text);
	temp2.ToString(s2);

	char a[30];
	_i64toa(TimeStamp,a,10);
	
	SQL.format("insert into \"%s\" values (\"%s\", \"%s\",\"%s\")",
		TableName,
		a,
		s1.c_str(),
		s2.c_str());

  
		m_LogDatabase.execDML(SQL);	

	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}
//	m_LogDatabase.close();
}

void CLogDatabase::ReadItem(int64 TimeStamp,ePipeline& ResultPipe){
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	
	const char* TableName = TABLENAME;
	
	char a[30];
	_i64toa(TimeStamp,a,10);

    AnsiString s;

    try{
//	m_LogDatabase.open("d:\\Log.DB");

	SQL.format("select * from \"%s\" where a > \"%s\";",TableName,a);
				
	Result = m_LogDatabase.execQuery(SQL);
    
	while(!Result.eof()){
		int64 t = Result.getInt64Field(0);
		AnsiString who = Result.getStringField(1);
		AnsiString Say = Result.getStringField(2);

		ResultPipe.PushInt(t);

		eSTRING ss;
		ss.FromString(who,0);
		ResultPipe.Push_Directly(ss.Clone());
		ss.FromString(Say,0);
		ResultPipe.Push_Directly(ss.Clone());
			
		Result.nextRow();
	}

	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}
//	m_LogDatabase.close();
}

void CLogDatabase::Clear(){
    try{
		
//	m_LogDatabase.open("d:\\Log.DB");

	CppSQLite3Buffer SQL;
	const char* TableName = TABLENAME;
    SQL.format("drop table %s",TableName);
	m_LogDatabase.execDML(SQL);	

	//建立新表
	SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b TEXT NOT NULL,c TEXT NOT NULL);",TableName);
	m_LogDatabase.execDML(SQL);

	}catch (CppSQLite3Exception& e) {
		const char* s = e.errorMessage();
		assert(0);
	}
//	m_LogDatabase.close();
}
