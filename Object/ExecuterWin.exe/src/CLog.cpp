// CLog.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "CLog.h"

#include <time.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
:m_BufSize( 0 ),
m_BufPos( 0 ),
m_Buffer(NULL),
m_FileHandle(INVALID_HANDLE_VALUE)
{

}
CLog::CLog(const std::string FileName, size_t Size)
	:m_BufSize( Size ),
     m_BufPos( 0 ),
	 m_FileName(FileName)
{
    m_FileHandle = CreateFileA(
		FileName.c_str(), 
		GENERIC_WRITE|GENERIC_READ ,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL );

    if( m_FileHandle == INVALID_HANDLE_VALUE ) 
       throw std::string( "Create Log file fail" );
  
    m_Buffer = new char[m_BufSize];
}


CLog::~CLog()
{
  PrintSection("ShutDown");
  if( m_FileHandle != INVALID_HANDLE_VALUE )
    CloseHandle( m_FileHandle );

  delete []m_Buffer;

}

void CLog::Open(const TCHAR* FileName, size_t Size /*= MAX_STRING_LENGTH*/){
	if( m_FileHandle != INVALID_HANDLE_VALUE ){
		CloseHandle( m_FileHandle );
		delete []m_Buffer;	
	};

	m_BufSize = Size;
	m_FileHandle = CreateFile(
		FileName, 
		GENERIC_WRITE ,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL );
	
    if( m_FileHandle == INVALID_HANDLE_VALUE ) 
		throw std::string( "Create Log file fail" );
	
    m_Buffer = new char[m_BufSize];
	m_BufPos = 0;
}


//////////////////////////////////////////////////////////////////////////
// Store data from buffer to store class

void CLog::Commit()
{
  if (m_FileHandle == INVALID_HANDLE_VALUE)
  {
	return;
  }
  DWORD tmpWrite = 0;

  if( m_BufPos > 0 )
    WriteFile( m_FileHandle, m_Buffer, m_BufPos, &tmpWrite, NULL );

  FlushFileBuffers( m_FileHandle );

  m_BufPos = 0;
}

void CLog::PrintHeader(const char* Tile){
    time_t t;
    time(&t);
	Printf("              %s\n\n        Start: %s",Tile,ctime(&t));
	Printf("_________________________________________________________\n");
	Commit();
}

void CLog::PrintSection(const char* Tile){
    Printf("_________________________________________________________\n");
	Printf("              ###    %s    ###\n", Tile);

	Commit();
}

void CLog::Printf(const char* fmt ...)
{
	if (m_FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	char Buffer [1024] ;
	va_list pArgList ;		
	va_start (pArgList, fmt) ;
	_vsnprintf(Buffer, 1024, fmt, pArgList) ;		
	va_end (pArgList) ;
    
	std::string s(Buffer);
	DWORD tmpWrite = 0;
	long strLen = s.length();

	if( strLen >= m_BufSize ) 
	{
		WriteFile( m_FileHandle, s.c_str(), strLen, &tmpWrite, NULL );
		return ;
	}

	if( strLen+m_BufPos >= m_BufSize )
	{
		Commit();
		WriteFile( m_FileHandle, s.c_str(), strLen, &tmpWrite, NULL );
		return ;
	}

	memcpy( m_Buffer + m_BufPos, s.c_str(), strLen );
	m_BufPos += strLen;

}

void CLog::PrintLn(const std::string& s,PRINT_TIME PrintTime)
{
	if(PrintTime)
		Printf("%s     %s\n",CurrentTime().c_str(),s.c_str());
	else
		Printf("%s\n",s.c_str());
}

void CLog::Print(const std::string &s, PRINT_TIME PrintTime)
{
	if(PrintTime)
		Printf("%s     %s\n",CurrentTime().c_str(),s.c_str());
	else
		Printf("%s",s.c_str());
}

//////////////////////////////////////////////////////////////////////////
// Set maximum buffer length 

void CLog::SetBufferSize( long lSize )
{
  Commit(  );

  if( lSize == m_BufSize ) return ;
  
  delete []m_Buffer;

  m_Buffer = new char[ m_BufSize = lSize ];  
}
