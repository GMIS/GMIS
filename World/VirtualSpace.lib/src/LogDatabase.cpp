// LogDatabase.cpp: implementation of the CLogDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "LogDatabase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogDatabase::CLogDatabase()
{

}

CLogDatabase::~CLogDatabase()
{
	m_LogDatabase.close();
}

void CLogDatabase::Open(tstring DBFile){
	
	m_LogDatabase.open(DBFile);
	
	const char* TableName = TABLENAME;
	if(!m_LogDatabase.tableExists(TableName)){
		CppSQLite3Buffer SQL;
		SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b TEXT NOT NULL,c TEXT NOT NULL);",TableName);
		m_LogDatabase.execDML(SQL);
	}
}

void CLogDatabase::WriteItem(int64 TimeStamp, tstring Who, tstring Say){
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;

	const char* TableName = TABLENAME;

	AnsiString s1, s2;
	eSTRING temp1(Who);
	temp1.ToString(s1);

	eSTRING temp2(Say);
	temp2.ToString(s2);

	char a[30];
	int64toa(TimeStamp,a);
	
	SQL.format("insert into \"%s\" values (\"%s\", \"%s\",\"%s\")",
		TableName,
		a,
		s1.c_str(),
		s2.c_str());

	m_LogDatabase.execDML(SQL);	
}

void CLogDatabase::ReadItem(int64 TimeStamp,ePipeline& ResultPipe){
	CppSQLite3Buffer SQL;
	CppSQLite3Query  Result;
	
	const char* TableName = TABLENAME;
	
	char a[30];
	int64toa(TimeStamp,a);

    AnsiString s;

	SQL.format("select * from \"%s\" where a > \"%s\";",TableName,a);
				
	Result = m_LogDatabase.execQuery(SQL);
    
	uint32 pos=0;
	while(!Result.eof()){
		int64 t = Result.getInt64Field(0);
		AnsiString who = Result.getStringField(1);
		AnsiString Say = Result.getStringField(2);

		ResultPipe.PushInt(t);

		eSTRING ss;
		bool ret = ss.FromString(who,pos);
		assert(ret);
		ResultPipe.Push_Directly(ss.Clone());
		pos=0;
		ret = ss.FromString(Say,pos);
		assert(ret);

		ResultPipe.Push_Directly(ss.Clone());
			
		Result.nextRow();
	}
}

void CLogDatabase::Clear(){
	CppSQLite3Buffer SQL;
	const char* TableName = TABLENAME;
    SQL.format("drop table %s",TableName);
	m_LogDatabase.execDML(SQL);	

	//建立新表
	SQL.format("CREATE TABLE \"%s\" ( a INTEGER NOT NULL ,b TEXT NOT NULL,c TEXT NOT NULL);",TableName);
	m_LogDatabase.execDML(SQL);
}
