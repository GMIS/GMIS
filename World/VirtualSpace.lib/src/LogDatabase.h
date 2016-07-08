/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _LOGDATABASE_H__
#define _LOGDATABASE_H__

#include "Pipeline.h"
#include "sqlite\CppSQLite3.h"

#define  TABLENAME  "Log"

class CLogDatabase  
{
protected:
	CppSQLite3DB   m_LogDatabase;
public:
	CLogDatabase();
	virtual ~CLogDatabase();

	bool IsOpen(){
		return m_LogDatabase.IsOpen();
	}
	void Open(tstring DBFile);
	void WriteItem(int64 TimeStamp, tstring Who, tstring Say);

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

#endif // !defined(_LOGDATABASE_H__)
