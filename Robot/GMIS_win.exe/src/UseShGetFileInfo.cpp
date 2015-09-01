
////////////////////////////////////////////////////////////
//                                                        //
// ClassName.cpp: implementation of the CClassName class. //
//                                                        //
////////////////////////////////////////////////////////////

#include "UseShGetFileInfo.h"


#include <shellapi.h>
#include <tchar.h>
#include <shlwapi.h> //path manipulation functions
#include <objbase.h>
#pragma comment( lib,"shlwapi") // link with respective library

/*
#if defined (_MT)
	#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	#endif
#else 
  CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
#endif //defined(_MT)
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUseShGetFileInfo::CUseShGetFileInfo()
{

}



HIMAGELIST  CUseShGetFileInfo::GetSystemImageListHandle( BOOL bSmallIcon )
{
	TCHAR tcSystemRoot[MAX_PATH+1];
	GetWindowsDirectory( tcSystemRoot, MAX_PATH);
	PathStripToRoot(tcSystemRoot);
	
	HIMAGELIST  hSystemImageList; 
	SHFILEINFO    ssfi; 
	
	if (bSmallIcon)
	{
		hSystemImageList = 
			(HIMAGELIST)SHGetFileInfo( 
			tcSystemRoot, 
			0, 
			&ssfi, 
			sizeof(SHFILEINFO), 
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON|SHGFI_ICON); 
	}
	else
	{
		hSystemImageList = 
			(HIMAGELIST)SHGetFileInfo( 
			(LPCTSTR)_T("c:\\"), 
			0, 
			&ssfi, 
			sizeof(SHFILEINFO), 
			SHGFI_SYSICONINDEX | SHGFI_LARGEICON); 
	}
	return hSystemImageList;
}


int CUseShGetFileInfo::GetFileIconIndex( tstring strFileName , BOOL bSmallIcon )
{
	SHFILEINFO    sfi;
	
	if (bSmallIcon)
	{
        SHGetFileInfo(
           (LPCTSTR)strFileName.c_str(), 
           FILE_ATTRIBUTE_NORMAL,
           &sfi, 
           sizeof(SHFILEINFO), 
           SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	}
	else
	{
		SHGetFileInfo(
           (LPCTSTR)strFileName.c_str(),
           FILE_ATTRIBUTE_NORMAL,
           &sfi, 
           sizeof(SHFILEINFO), 
           SHGFI_SYSICONINDEX | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	}

    return sfi.iIcon;
	
}

int CUseShGetFileInfo::GetDirIconIndex(BOOL bSmallIcon )
{
	SHFILEINFO    sfi;
	if (bSmallIcon)
	{
		 SHGetFileInfo(
		 _T(""),          //make it UNICODE compatible---empty string suffices 
		 FILE_ATTRIBUTE_DIRECTORY,
		 &sfi, 
		 sizeof(SHFILEINFO), 
		 SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES|SHGFI_ICON);
	}
	else
	{
		 SHGetFileInfo(
		 (LPCTSTR)_T("Doesn't matter"), 
		 FILE_ATTRIBUTE_DIRECTORY,
		 &sfi, 
		 sizeof(SHFILEINFO), 
		 SHGFI_SYSICONINDEX | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);

	}
    return sfi.iIcon;

}
HICON CUseShGetFileInfo::GetFileIconHandle(const TCHAR* strFileName, BOOL bSmallIcon)
{
	
	SHFILEINFO    sfi;
	if (bSmallIcon)
	{
		SHGetFileInfo(
		   (LPCTSTR)strFileName, 
		   FILE_ATTRIBUTE_NORMAL,
		   &sfi, 
		   sizeof(SHFILEINFO), 
		   SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	}
	else
	{
		SHGetFileInfo(
		   (LPCTSTR)strFileName, 
		   FILE_ATTRIBUTE_NORMAL,
		   &sfi, 
		   sizeof(SHFILEINFO), 
		   SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	}
	return sfi.hIcon;


}
HICON CUseShGetFileInfo::GetFolderIconHandle(BOOL bSmallIcon )
{
	SHFILEINFO    sfi;
	if (bSmallIcon)
	{
		 SHGetFileInfo(
		 (LPCTSTR)_T("Doesn't matter"), 
		 FILE_ATTRIBUTE_DIRECTORY,
		 &sfi, 
		 sizeof(SHFILEINFO), 
		 SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	}
	else
	{
		 SHGetFileInfo(
		 (LPCTSTR)_T("Does not matter"), 
		 FILE_ATTRIBUTE_DIRECTORY,
		 &sfi, 
		 sizeof(SHFILEINFO), 
		 SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	}
        return sfi.hIcon;
}

tstring CUseShGetFileInfo::GetFileType(tstring strFileName)
{
	SHFILEINFO    sfi;
	
	SHGetFileInfo(
     (LPCTSTR)strFileName.c_str(), 
     FILE_ATTRIBUTE_NORMAL,
     &sfi, 
     sizeof(SHFILEINFO), 
     	SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
     
	tstring type; 
	type =*(LPCTSTR)(&sfi.szTypeName[0]);
	return type;

}







