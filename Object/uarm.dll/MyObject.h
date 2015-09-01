// MyObject.h: interface for the MyObject class.
//
//////////////////////////////////////////////////////////////////////

#ifndef  _MY_OBJECT_H__
#define  _MY_OBJECT_H__

#pragma warning(disable: 4786)


#include "AbstractSpace.h"
#include "UserTimer.h"
#include "UserMutex.h"
#include "Object.h"
#include "Arm.h"

class MyObject : public Mass
{
	CArm        m_Arm;
	tstring     m_Name;
public:
	MyObject(tstring Name,int64 ID=0)
		:Mass(ID){
	
	};

	virtual ~MyObject(){
		
	};

	virtual tstring GetName(){
		return m_Name;
	};
	
	virtual TypeAB GetTypeAB(){
		return 0x00000000;
	}


	virtual bool Do(Energy* E){
		return m_Arm.Do(E);
	};
	
	/*如果Object的执行依赖其内部状态信息则应该实现下面两个函数
      virtual uint32  ToString(string& s,uint32 pos =0) 
	  virtual uint32  FromString(string& s,uint32 pos=0) 
    */
};

#endif // 