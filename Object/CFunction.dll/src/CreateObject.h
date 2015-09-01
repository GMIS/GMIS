#ifndef _CREATE_DLL_H
#define _CREATE_DLL_H
#include <stdio.h>
#include <string>

#include "Mass.h"
#include "pipeline.h"
#include "MyObject.h"

#define _LinkDLL __declspec(dllexport)


//定义导出的函数指针

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
#else _MSG_VER<1800
	return DLL_VC13D;
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
#else _MSG_VER<1800
	return DLL_VC13D;
#endif

#endif
} 
//这个函数给出一个物体最简单的使用说明,请自行替换
extern "C"  _LinkDLL const TCHAR*   _cdecl GetObjectDoc(){
		static const TCHAR* Text = __TEXT(\
			"\
		   1 命令：sin\n\
		   功能：计算三角函数sin\n\
		   输入数据：一个整数或者小数\n\
		   输出数据：计算所得的值（小数）\n\
		   2 命令：cos\n\
		   功能：计算三角函数cos\n\
		   输入数据：一个整数或小数\n\
		   输出数据：计算所得的值（小数）\n\
		   3 命令：tan\n\
		   功能：计算三角函数tan\n\
		   输入数据：一个整数或小数\n\
		   输出数据：计算所得的值（小数）\n\
		   "\
		   );

	return Text;
}
#endif // _CREATE_DLL_H
