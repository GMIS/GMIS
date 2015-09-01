/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _BRAINTASK_H__
#define _BRAINTASK_H__


#include "PhysicSpace.h"
#include "Element.h"
#include "BrainObject.h"
#include "InstinctDefine.h"
#include <map>
#include <set>

using namespace std;


#define CM_TASKREFRESH    1

class CLogicThread;
class CTaskDialog;
class CText;


void GetLogicName(tstring& SrcLoigcName, tstring& LogicName,tstring& LogicInstanceName,tstring& LogicComment);

typedef map <tstring,Mass *>  LOGIC_OBJECT; 

class CBrainTask : public CSeries
{
	friend class CTaskDialog;
public:
	
	int64                  m_UserDialogID;

	tstring                m_CompileError;

	int64				   m_BeginTime;        //用来标记每一次执行

	tstring                m_LogicText;        //逻辑的描述
	ePipeline              m_LogicData;        //逻辑的理解结果
    int32                  m_State;            //逻辑行为的类型
    int32                  m_bDepend;           
		
    uint32                 m_ElementCount;     //编译任务得到的串联或并联的数目
	uint32                 m_MassCount;        //编译任务得到的所有可执行的Particle数目,同时也作为识别ID  
	int32                  m_ExecuteCount;     //纪录本task已经被执行次数
	
	uint32                 m_ActionType;

	map<tstring,Mass*>     m_TempElementList;  
protected:
	tstring                m_CurThinkLogicName;
	Mass* GetInstinctInstance(CTaskDialog* Dialog,int64 InstinctID, Energy* Param);
	
public:

	CBrainTask();
	CBrainTask(CBrainTask* Parent);
	CBrainTask(int64 TaskID,int64 UserDialogID);
	virtual ~CBrainTask();
	  
	virtual Energy*  ToEnergy();	
	virtual bool     FromEnergy(CTaskDialog* Dialog,Energy* E);

	void ResetTime(){
		m_BeginTime = AbstractSpace::CreateTimeStamp();
	}; 	
	
	bool IsValid(){
		return m_LogicData.Size()>0;
	}

	CBrainTask* GetRootTask();

	bool Compile(CTaskDialog* Dialog,ePipeline* Sentence);
	CElement* CompileSentence(CTaskDialog* Dialog,tstring& LogicText,ePipeline* Sentence);

	bool CompileOK(){ return m_ActomList.size() !=0;};

	CBrainTask& operator<<(CBrainTask& Task);

	void Reset(int64 TaskID,int64 UserDialogID);
	
	void Clear();

	bool  SetBreakPoint(ePipeline& Path,BOOL bEnable);

    //用于调试VIEW的显示 
	void  GetDebugItem(ePipeline& ParentPipe,Mass* m=NULL);

};



#endif // !defined(AFX_BRAINTASK_H__843288B8_EBC2_49AB_A746_7CBDD879A5CF__INCLUDED_)
