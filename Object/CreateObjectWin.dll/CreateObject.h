/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _CREATE_DLL_H
#define _CREATE_DLL_H

#include <stdio.h>
#include <string>


#include "ObjectList.h"
#include "win_api\WinAPIObject.h"
#include "uarm\uArmObject.h"
#include "curl\CurlObject.h"

#define _LinkDLL __declspec(dllexport)

#ifdef _MSC_VER
#define EXECUTER_TYPE 1000
#endif
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
extern "C"  _LinkDLL int _cdecl GetDllType(){
	//不同的返回值帮助决定不同的C运行时库
#ifdef _DEBUG
	return EXECUTER_TYPE+1;	
#else
	return EXECUTER_TYPE;
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
