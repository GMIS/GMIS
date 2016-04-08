// LogDatabase.h: interface for the CLogDatabase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _LOGDATABASE_H__
#define _LOGDATABASE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TheorySpace.h"
#include "CppSQLite3.h"

#define  TABLENAME  "Log"

class CLogDatabase  
{
protected:
	CppSQLite3DB   m_LogDatabase;
public:
	CLogDatabase();
	virtual ~CLogDatabase();

	void WriteItem(int64 TimeStamp, const TCHAR* whosay, const TCHAR* text);

	/*读时间戳大于TimeStamp的所有条目，存放在Result
	  ePipeline{
			int64 TimeStamp
			string Who
			string Say
			....
			int64 TimeStamp
			string Who
			string Say
		}
	*/
	void ReadItem(int64 TimeStamp,ePipeline& Result);

	void Clear();
};

#endif // _LOGDATABASE_H__
