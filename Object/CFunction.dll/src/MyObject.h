// MyObject.h: interface for the MyObject class.
// 一个制造产品的标准模板
//////////////////////////////////////////////////////////////////////

#ifndef  _MY_OBJECT_H__
#define  _My_OBJECT_H__

#pragma warning(disable: 4786)

#include "Mass.h"
#include "pipeline.h"

class MyObject : public Mass
{
	tstring m_Name;
public:
	MyObject(tstring Name,int64 ID=0)
		:m_Name(Name),Mass(ID){};
    
	virtual ~MyObject(){};
	virtual tstring GetName(){ return m_Name;};
	virtual TypeAB GetTypeAB(){ return 0x00000000;}

	virtual bool Do(Energy* E);
	
	/*如果Object的执行依赖其内部状态信息则应该实现下面两个函数
      virtual uint32  ToString(string& s,uint32 pos =0) 
	  virtual uint32  FromString(string& s,uint32 pos=0) 
    */
};

#endif // 