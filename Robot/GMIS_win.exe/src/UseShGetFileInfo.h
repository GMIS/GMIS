// This file was created on February 5th 2003. By Doru Cioata
//
//


#if !defined(AFX_USESHGETFILEINFO_H__CE3A83A6_10C3_46BC_9D55_EFB612864292__INCLUDED_)
#define AFX_USESHGETFILEINFO_H__CE3A83A6_10C3_46BC_9D55_EFB612864292__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <string>
#include <commctrl.h>
#include <tchar.h>

using namespace std;

#ifndef tstring

#ifdef _UNICODE
#define  tstring  std::string
#else
#define  tstring  std::wstring
#endif

#endif


#pragma warning( push )  // better yet to not change it at all
#pragma warning (disable : 4089)

class CUseShGetFileInfo
{

private:   
	CUseShGetFileInfo();   // prevent instantiation
public:   
	// virtual ~CUseShGetFileInfo();   // not needed   
	//get the system's image list   
	static HIMAGELIST GetSystemImageListHandle( BOOL bSmallIcon );      
	
	//get the image's index in the system's image list   
	static int GetFileIconIndex( tstring strFileName , BOOL bSmallIcon); 
	static int GetDirIconIndex(BOOL bSmallIcon);   
	
	//get a handle to the icon  
	static HICON GetFileIconHandle(const TCHAR* strFileName,BOOL bSmallIcon); 
	static HICON GetFolderIconHandle(BOOL bSmallIcon );   
	
	//get file type 
	static tstring GetFileType(tstring tstrFileName);
};
#pragma warning( pop )   // see above


#endif 


