/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _CREATE_DLL_H
#define _CREATE_DLL_H

#include <stdio.h>
#include <string>

#include "ObjectList.h"

#include "WinAPIObject.h"
#include "uArmObject.h"

#define _LinkDLL __declspec(dllexport)

/*
使用方法：
  - 首先用一个类来封装自己想要的功能。
  - 修改导出函数CreateObject，new一个自己设计的类。
  - 修改导出函数GetObjectDoc，输出你设计的功能的使用说明书

*/
//定义导出函数指针

extern "C"  _LinkDLL Mass* _cdecl CreateObject(tstring Name,int64 ID){
#if defined _COMPILE_WIMAPI_OBJECT
	return new CWinAPIObject(Name,ID);
#elif defined _COMPILE_UARM_OBJECT
	return new CUArmObject(Name,ID);
#elif defined _COMPILE_CURL_OBJECT
	return new CCurlObject(Name,ID);
#endif
}
  
extern "C"  _LinkDLL void _cdecl  DestroyObject(Mass* p){
	delete p;
	p = NULL;
}
extern "C"  _LinkDLL DLL_TYPE _cdecl GetDllType(){
	//不同的返回值帮助决定不同的C运行时库
#ifdef _DEBUG

#if _MSC_VER<1300
	return DLL_VC6D;
#elif _MSC_VER<1400
	return DLL_VC7D;
#elif _MSC_VER<1500
	return DLL_VC8D;
#elif _MSC_VER<1600
	return DLL_VC9D;
#elif _MSC_VER<1700
	return DLL_VC10D;
#endif

#else

#if _MSC_VER<1300
	return DLL_VC6;
#elif _MSC_VER<1400
	return DLL_VC7;
#elif _MSC_VER<1500
	return DLL_VC8;
#elif _MSC_VER<1600
	return DLL_VC9;
#elif _MSC_VER<1700
	return DLL_VC10;
#endif

#endif
} 

extern "C"  _LinkDLL const TCHAR*   _cdecl GetObjectDoc(){

#if defined _COMPILE_WIMAPI_OBJECT
	return CWinAPIObject::UserManual;
#elif defined _COMPILE_UARM_OBJECT
	return  CUArmObject::UserManual;
#elif defined _COMPILE_CURL_OBJECT
	return CCurlObject::UserManual;
#endif

}
#endif // _CREATE_DLL_H
