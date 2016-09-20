// Executer.cpp : Defines the entry point for the application.
//
#pragma warning(disable: 4786)

#include "ActiveObject.h"
#include "UserTimer.h"
#include "UserMutex.h"
#include "SpaceMsgList.h"
#include <conio.h >
#include <locale>  
#include <codecvt> 
#include "ObjectList.h"

#define MAX_LOADSTRING 100

#define USING_GUI 

using namespace ABSTRACT;

#pragma comment(lib, "comctl32.lib")


std::wstring UTF8ToUTF16(const std::string &source)  
{  

	try  
	{  
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;  
		return cvt.from_bytes(source);  
	}  
	catch (std::range_error &e)  
	{  
		return std::wstring();  
	}

}  

std::string UTF16ToUTF8(const std::wstring &source)  
{  
	try  
	{  
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;  
		return cvt.to_bytes(source);  
	}  
	catch (std::range_error &)  
	{  
		return std::string();  
	}  
}  


int _tmain(int argc, _TCHAR* argv[])
{

	//实例一个物体
	CTestObject  MyObject(argc,argv);
	
	if(!MyObject.Activate()){
		MyObject.OutputLog(LOG_TIP,_T("Object activate fail. Press any key to quit"));
		getch();
		return 0;
	}

	MyObject.Do(NULL);

	MyObject.Dead();

	return 0;
}


