/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _CREATE_DLL_H
#define _CREATE_DLL_H

#include <stdio.h>
#include <string>

#include "MyObject.h"

#define _LinkDLL __declspec(dllexport)

/*
使用方法：
  - 首先你在MyObject.h中用一个类来封装自己的任务。
  - 修改下面的导出函数GetObject，new一个自己设计的类。类的名字并不重要，
    如果你愿意，你甚至直接使用MyObject作为类名，那么这步都省了。

*/
//定义导出函数指针

extern "C"  _LinkDLL Mass* _cdecl CreateObject(tstring Name,int64 ID=0){
	return new MyObject(Name,ID);
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
//这个函数给出一个物体最简单的使用说明,请自行替换
extern "C"  _LinkDLL const TCHAR*   _cdecl GetObjectDoc(){
	static const TCHAR* Text = _T("This is  MyObject's document");
	return Text;
}
#endif // _CREATE_DLL_H
