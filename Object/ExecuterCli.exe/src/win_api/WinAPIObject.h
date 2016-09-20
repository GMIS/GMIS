/*
* 封装windows操作系统中常用的命令
* author: ZhangHongBing(hongbing75@gmail.com) 
*
*/

#pragma once

#include "..\ActiveObject.h"

#pragma warning(disable: 4786)

#if defined _COMPILE_WIMAPI_OBJECT


class CWinAPIObject: public CActiveObject
{
public:

	tstring m_Name;
protected:
	virtual void			OnObjectRun(CMsg& Msg);
	virtual void			OnObjectGetDoc(CMsg& Msg);
public:
	CWinAPIObject(int argc, _TCHAR* argv[])
		:CActiveObject(argc,argv){

	};
    
	virtual ~CWinAPIObject(){};

	virtual tstring GetName(){ return _T("WinAPI Object");};
	
	bool DoOpen(ePipeline& ExePipe,tstring& Error);
};

#endif //_COMPILE_WIMAPI_OBJECT

