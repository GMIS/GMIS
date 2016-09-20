/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once
#include "..\ObjectList.h"

#if defined _COMPILE_TEST_OBJECT

#include "Mass.h"
#include "Pipeline.h"
#include "..\ActiveObject.h"

class CTestObject: public CActiveObject
{
public:

	tstring m_Name;
protected:
	virtual void			OnObjectRun(CMsg& Msg);
	virtual void			OnObjectGetDoc(CMsg& Msg);
public:
	CTestObject(int argc, _TCHAR* argv[])
		:CActiveObject(argc,argv){

	};

	virtual ~CTestObject(){};

	virtual tstring GetName(){ return _T("Test Object");};

};

#endif //_COMPILE_TEST_OBJECT

