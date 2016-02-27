// WinAPIObject.h: interface for the WinAPIObject class.
// 封装windows操作系统的一些常用API
//////////////////////////////////////////////////////////////////////

#ifndef  _WIN_API_OBJECT_H__
#define  _WIN_API_OBJECT_H__

#pragma warning(disable: 4786)

#include "ObjectList.h"

#if defined _COMPILE_WIMAPI_OBJECT

#include "Mass.h"
#include "Pipeline.h"

class CWinAPIObject : public Mass
{
public:
	static const TCHAR* UserManual;/* = __TEXT(\
		"\
		1 cmd：open\n\
		Function：打开一个程序或链接\n\
		Input：字符串，程序名或链接地址\n\
		Output：null\n\
		"\
		);
		*/

	tstring m_Name;
public:
	CWinAPIObject(tstring Name,int64 ID=0)
		:m_Name(Name),Mass(ID){};
    
	virtual ~CWinAPIObject(){};
	virtual tstring GetName(){ return m_Name;};
	virtual TypeAB GetTypeAB(){ return 0x00000000;}

	virtual bool Do(Energy* E);
	
	/*如果Object的执行依赖其内部状态信息则应该实现下面两个函数来实现初始化
      virtual uint32  ToString(string& s,uint32 pos =0) 
	  virtual uint32  FromString(string& s,uint32 pos=0) 
    */
};

#endif //_COMPILE_WIMAPI_OBJECT

#endif // 