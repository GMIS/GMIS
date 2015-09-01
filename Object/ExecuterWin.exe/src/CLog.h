// CLog.h: interface for the CLog class.
// 修改自codeproject 原author:Aex Kucherenko
//////////////////////////////////////////////////////////////////////

#ifndef _CLOG_H__
#define _CLOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable: 4786 4290 ) // disable warning

#include <windows.h>
#include <string>
#include <stdio.h> 

#ifndef MAX_STRING_LENGTH
  #define MAX_STRING_LENGTH 1024
#endif

enum PRINT_TIME{ NoPrintTime = 0, PrintTime};
class  CLog  
{
    std::string    m_FileName;
    HANDLE         m_FileHandle;
    size_t         m_BufSize;
    char*          m_Buffer;
    size_t         m_BufPos;
public:
	CLog();
	CLog( const std::string FileName, size_t Size = MAX_STRING_LENGTH ) throw ( std::string );
	virtual ~CLog();
	
	void Open(const TCHAR* FileName, size_t Size = MAX_STRING_LENGTH);
	HANDLE GetHandle(){
		return m_FileHandle;
	}
	inline std::string CurrentTime()
    {
      SYSTEMTIME	SystemTime;
      GetLocalTime( &SystemTime );
      
	  char buffer[100];  
	  sprintf(buffer,"%02u:%02u:%02u ms:%03u", 
        SystemTime.wHour, SystemTime.wMinute, 
        SystemTime.wSecond, SystemTime.wMilliseconds );
      std::string Time(buffer);
      return Time; 
    };

    void Commit();
    
	void PrintHeader(const char* Tile="Application Log");
	void PrintSection(const char* Tile);
	
	void Printf(const char* fmt ...);
	void PrintLn(const std::string& s,PRINT_TIME PrintTime = NoPrintTime);
	void Print(const std::string& s, PRINT_TIME PrintTime = NoPrintTime);

    void SetBufferSize( long lSize );

};

#endif // _CLOG_H__



