#ifndef _CREATE_DLL_H
#define _CREATE_DLL_H
#include <stdio.h>
#include <string>

#include "TheorySpace.h"
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
		   1 cmd：open_port\n\
		   Function：打开一个串行端口\n\
		   Input：字符串（端口名字），缺省打开端口COM3\n\
		   Output：null\n\
		   2 cmd：pos\n\
		   Function：设置机械臂的姿态数据\n\
		   Input：输入姿态数据，可以按顺序输入一项或多项:臂旋转(-90 ~ 90),伸缩(0 ~ 210),高低(-180 ~ 150),手旋转(-90 ~ 90), \n\
		   Output：null\n\
		   3 Cmd：grap\n\
		   Function：手抓紧\n\
		   Input：null\n\
		   Output：null\n\
		   4 Cmd：release\n\
		   Function：手松开\n\
		   Input：null\n\
		   Output：null\n\
		   5 Cmd：left\n\
		   Function：让机械臂左旋转\n\
		   Input：一个整数（旋转角度）\n\
		   Output：null\n\
		   6 Cmd：right\n\
		   Function：让机械臂右旋转\n\
		   Input：一个整数（旋转角度）\n\
		   Output：null\n\
		   7 Cmd：up\n\
		   Function：升高机械臂\n\
		   Input：一个整数（升高角度）\n\
		   Output：null\n\
		   8 Cmd：down\n\
		   Function：降低机械臂\n\
		   Input：一个整数（降低角度）\n\
		   Output：null\n\
		   9 Cmd：goahead\n\
		   Function：伸出机械臂\n\
		   Input：一个整数（伸展角度）\n\
		   Output：null\n\
		   10 Cmd：goback\n\
		   Function：缩回机械臂\n\
		   Input：一个整数（缩回角度）\n\
		   Output：null\n\
		   11 Cmd：hand_left\n\
		   Function：手向左选择\n\
		   Input：一个整数（旋转角度）\n\
		   Output：null\n\
		   12 Cmd：hand_right\n\
		   Function：手像右旋转\n\
		   Input：一个整数（旋转角度）\n\
		   Output：null\n\
		   "\
		   );

	return Text;
}
#endif // _CREATE_DLL_H
