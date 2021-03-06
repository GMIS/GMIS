﻿/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _ELEMENT_H__
#define _ELEMENT_H__


#include "PhysicSpace.h"
#include <vector>
#include <deque>
#include <map>
#include <tchar.h>
#include "MsgList.h"
#include "Brain.h"

/*
  Element是质量体的集合。

  集合只有两种形式：并联CSeries或串联CShunt，由此组合成万物。

  靠上述方式组装逻辑应该可以同时组装出空间外形，但目前没有实现
*/

typedef vector<Mass*>            ActomList;
typedef vector<Mass*>::iterator  ActomPtr;

enum LogicRelation { 
	UNKOWN_RELATION = 0, 
	SERIES_RELATION = 1,
	SHUNT_RELATION  = 2
};

class CTaskDialog;


class   CElement: public Mass
{
private:
	int64           m_EventID;
	//弃用
	virtual bool Do(Energy* E){return TRUE;};
public:
    tstring			m_Name; 
	CElement*       m_Parent;
	ActomList       m_ActomList;
public:
	CElement(int64 ID,const TCHAR* Name);
	virtual ~CElement();

	void Clear();

	void   UpdateEventID(){
		 m_EventID = AbstractSpace::CreateTimeStamp();
	}

	int64 GetEventID(){
		return m_EventID;
	}
	virtual Energy*  ToEnergy();	
	virtual bool     FromEnergy(Energy* E);

	virtual void InsertLogic(int64 Index,CElement* e);
    virtual bool RemoveLoigc(int64 ChildID);

	Mass* GetChild(int64 ChildID);


	void GetAddress(ePipeline& Address);

	CElement* FindFocusLogic(const tstring& FocusName);

	virtual tstring GetName();
	CElement& operator=(const CElement& Element);

	void  Push(Mass* Object);
    int32 MassNum(){return m_ActomList.size();};

	//Mass* FindMass(int64 ID); //假设所有ID都是有序的，采用二元搜索法完成
 
  
	Mass* FindMass(ePipeline& Address);

	virtual LogicRelation RealtionType(){return UNKOWN_RELATION;};

	virtual MASS_TYPE	  MassType(){ return MASS_ELEMENT;}; 
   	virtual TypeAB		  GetTypeAB(){ return 0x00000000;};

    /*
	ExePipe = 执行管道
	LocalAddress  负责跟踪记录执行物体的地址，当暂停时或则对外发信息时需要
	ObjectAddress 暂停恢复时，继续执行所需地址
	Msg 暂停恢复时可能需要处理的外部信息，只有Element具有处理外部信息的能力
	*/
	virtual bool Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);


protected:

	//处理外部信息，缺省：Dialog->m_Brain->TaskDialogProc(Dialog,SysMsg); 
	//返回CONTINUE_NEXT会继续处理TaskProc()，否则不处理TaskProc(),如果想直接返回系统，那么ExePipe.Break();
	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};


// 串联体
// 在表达行为逻辑时，集合里的行为会按顺序依次执行

class  CSeries : public CElement  
{
public:
public:
	CSeries(int64 ID,const TCHAR* Name=_T("Series"))
		:CElement(ID,Name){
	};
    
	virtual ~CSeries(){};
  	
    LogicRelation RealtionType(){ return SERIES_RELATION;};

	//由于此函数递归调用频繁，直接实现do()而不是分别实现MsgProc()和TaskProc()可以节约函数堆栈
	virtual bool Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

//并联体
//在表达行为逻辑时，集合里的行为被视为同时执行——赋予天然的分布式执行能力

class  CShunt : public CElement
{
private:
	ePipeline        m_TempPipe;   //临时保存执行管道，因为需要为每一个子物体复制一份

	/*临时保存每一个子物体执行返回的结果（执行管道数据）
	  m_ID=MassNum()表示已经完全执行，否则没有
	  每一个子ePipeline.m_ID !=0 表示执行过
	*/
	ePipeline        m_TempResultList; 
public:

	CShunt(int64 ID,const TCHAR* Name=_T("Shunt"))
		:CElement(ID,Name){};
    
	virtual ~CShunt(){};
    LogicRelation RealtionType(){ return SHUNT_RELATION;};

	void ShuntBegin(ePipeline& ExePipe);
	void ShuntEnd(ePipeline& ExePipe);
	void SaveTempResult(int32 n,ePipeline& ExePipe);
    bool IsCompleted();
	bool IsInited();
	void Reset();
	bool GetExePipe(int32 n,ePipeline& ExePipe);

	virtual void InsertLogic(int64 Index,CElement* e);
	virtual bool RemoveLoigc(int64 ChildID);

	bool Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

class  CLocalInfoAuto  //跟踪Mass地址
{
private:
	ePipeline&   m_LocalPipe;
	CTaskDialog* m_Dialog;
public:
    CLocalInfoAuto(CTaskDialog* Dialog,CElement* Elt,ePipeline& Pipe);
    ~CLocalInfoAuto ();
};



#endif // _ELEMENT_H__