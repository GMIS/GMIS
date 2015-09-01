/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _LOGICELEMENT_H__
#define _LOGICELEMENT_H__

#include "AbstractSpace.h"
#include "Pipeline.h"
#include "Element.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

class CBrainTask;

//比如： use capacitor cp:循环变量
//其中“循环变量”为注释，使用GetLogicElementName()则去掉注释返回实际名字“cp”
tstring GetLogicElementName(tstring& Name);

/*电容也是一个特殊的能量容器，当电流经过它，如果电容为空则充电，电流清零。
否则电容放电。

也就是：我有，我给你，我没有，你给我（不管你有没有）
*/
class  CCapacitor : public Mass  
{
public:
	CBrainTask*   m_Task;
	ePipeline     m_Temp;      //用来暂时存储一段能量；
	tstring       m_Name;
	int64         m_TaskBeginTime;
public:
	CCapacitor(CBrainTask* Task,const TCHAR* Name,int64 ID=0)
		:m_Task(Task),m_Name(Name),m_Temp(),Mass(ID),m_TaskBeginTime(0){};	
	
	virtual ~CCapacitor(){};
	virtual tstring GetName(){ return m_Name;};
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
	virtual Energy*  ToEnergy();
	
	virtual bool     FromEnergy(Energy* E);
	
	void Reset();
	virtual bool Do(Energy* E);
	
};
/* 二极管用来模拟单向导通，如果等于设定的数则管道数据不变，否则管道数据清空,并设置软断路

*/
class  CDiode  : public Mass
{
	int64  m_Num;	
public:
	CDiode(int64 n,int64 ID)
		:m_Num(n),Mass(ID){
		
	};
	virtual ~CDiode(){
	};
	virtual tstring  GetName(){
		return Format1024(_T("(D)%d"),m_Num);
	};
	
	virtual TypeAB  GetTypeAB(){ return 0x10000000;} 
	
	virtual Energy*  ToEnergy();
	
	virtual bool     FromEnergy(Energy* E);
	
	
	virtual bool Do(Energy* E);
};

/*电感也是一个特殊的能量容器，当电流经过它，如果电流不为零则替换电感数据，
如果电流为零则取得数据。

也就是：你有，你给我，你没有，我给你（不管我有没有）
*/
class  CInductor : public Mass  
{
private:
	CBrainTask*   m_Task;
	ePipeline     m_Temp;      //用来暂时存储一段能量；
	tstring       m_Name;
	int64         m_TaskBeginTime;
public:
	CInductor(CBrainTask* Task,const TCHAR* Name,int64 ID=0)
		:m_Task(Task),m_Name(Name),m_Temp(),Mass(ID),m_TaskBeginTime(0){};
	virtual ~CInductor(){};
	
	virtual tstring   GetName(){ return m_Name.c_str();};
	virtual TypeAB    GetTypeAB(){ return 0x00000000;} 
	void Reset(){ 
		m_Temp.Clear();
	};
	
	virtual Energy*  ToEnergy();
	virtual bool     FromEnergy(Energy* E);
	virtual bool	 Do(Energy* E);
	
};
/* 零电阻，电流直接通过
-1表示极大电阻，电流在此阻断清零
否则清除指定数目的数据
*/
class   CResistor  : public Mass
{
	int32  m_Num;	
public:
	CResistor(int32 n=1,int64 ID=0)
		:m_Num(n),Mass(ID)
	{
	};
	virtual ~CResistor(){};
	
	virtual tstring      GetName(){ return Format1024(_T("(r) %d"),m_Num);}
	virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
	
	virtual Energy*  ToEnergy();
	
	virtual bool     FromEnergy(Energy* E);
	virtual bool Do(Energy* E);
};


/*引用其他质量体
*/
class  CMassRef: public Mass
{
private:
	Mass*   m_MassRef;  
	
	tstring      m_Name;
	CMassRef():m_MassRef(NULL){}; //不允许空引用；
public:
	CMassRef(Mass* Object,const TCHAR* Name,int64 ID=0)
		:m_Name(Name),m_MassRef(Object),Mass(ID)
	{
		assert(Object != this);
		assert(Object != NULL);
	};
	virtual ~CMassRef(){};
	virtual TypeAB GetTypeAB(){ return m_MassRef->GetTypeAB();};
	virtual tstring      GetName(){ return m_Name;};
	
	virtual Energy*  ToEnergy();
	virtual bool     FromEnergy(Energy* E);
	
	virtual bool  Do(Energy* E){
		assert(m_MassRef != NULL);
		return m_MassRef->Do(E);
	}
};


class  CEncodePipe : public Mass  
{
public:
	CEncodePipe(int64 ID)
		:Mass(ID){
	};	
	
	virtual ~CEncodePipe(){};
	
	virtual tstring GetName(){ return _T("Encode Pipe");};
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
	
	virtual bool Do(Energy* E);
};

class  CDecodePipe : public Mass  
{
public:
	CDecodePipe(int64 ID)
		:Mass(ID){
	};	
	
	virtual ~CDecodePipe(){};
	
	virtual tstring GetName(){ return _T("Encode Pipe");};
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
	
	virtual bool Do(Energy* E);
};	




		
//什么也不做，用Name object 或者focus这类仅仅面向大脑内部的操作
class  CEmptyMass : public Mass  
{
public:
	tstring   m_Name;
public:
	CEmptyMass(int64 ID,tstring Name)
		:Mass(ID),m_Name(Name){
	};	
	
	virtual ~CEmptyMass(){};
	
	virtual tstring GetName(){ return m_Name;};
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
	virtual bool Do(Energy* E)
	{
		return true;
	}
};	




#endif // !defined(_LOGICELEMENT_H__)
