// Win32Tool.h: interface for the Win32Tool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WIN32TOOL_H__9E7DB47F_B025_4E4F_A59B_E8B02B8591D9__INCLUDED_)
#define AFX_WIN32TOOL_H__9E7DB47F_B025_4E4F_A59B_E8B02B8591D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <tchar.h>     
#include <stdio.h> 

typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // must be 0x1000
  LPCSTR szName; // pointer to name (in user addr space)
  DWORD dwThreadID; // thread ID (-1=caller thread)
  DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);


BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
int CDECL MBPrintf (TCHAR * szCaption, TCHAR * szFormat, ...);

//Function to convert unsigned char to string of length 2
void Char2Hex(unsigned char ch, char* szHex);

//Function to convert string of length 2 to unsigned char
void Hex2Char(char const* szHex, unsigned char& rch);

//Function to convert string of unsigned chars to string of chars
void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize);

//Function to convert string of chars to string of unsigned chars
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize);


BOOL DeleteDir( CONST TCHAR* lpcszSrcPath );


#endif // !defined(AFX_WIN32TOOL_H__9E7DB47F_B025_4E4F_A59B_E8B02B8591D9__INCLUDED_)
