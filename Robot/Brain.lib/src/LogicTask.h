/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LOGICTASK_H__
#define _LOGICTASK_H__


#include "PhysicSpace.h"
#include "Element.h"

#include <map>
#include <set>

using namespace std;


#define CM_TASKREFRESH    1

class CLogicThread;
class CLogicDialog;
class CText;
class CSentence;

void GetLogicName(tstring& SrcLoigcName, tstring& LogicName,tstring& LogicInstanceName,tstring& LogicComment);

typedef map <tstring,Mass *>  LOGIC_OBJECT; 

class CLogicTask : public CSeries
{
	friend class CLogicDialog;
public:
	
	int64                  m_UserDialogID;

	tstring                m_CompileError;

	int64				   m_BeginTime;        //用来标记每一次执行

	tstring                m_LogicText;        //逻辑的描述
	ePipeline              m_LogicData;        //逻辑的理解结果
    int32                  m_State;            //逻辑行为的类型
    int32                  m_bDepend;           
		
    int64                  m_ElementCount;     //编译任务得到的串联或并联的数目
	int64                  m_MassCount;        //编译任务得到的所有可执行的Particle数目,同时也作为识别ID  
	int64                  m_ExecuteCount;     //纪录本task已经被执行次数
	
	uint32                 m_ActionType;

	map<tstring,Mass*>     m_TempElementList;  

protected:
	tstring                m_CurThinkLogicName;
	Mass* GetInstinctInstance(CLogicDialog* Dialog,int64 InstinctID, Energy* Param);
	
public:

	CLogicTask();
	CLogicTask(CLogicTask* Parent);
	CLogicTask(int64 TaskID,int64 UserDialogID);
	virtual ~CLogicTask();
	  
	virtual Energy*  ToEnergy();	
	virtual bool     FromEnergy(CLogicDialog* Dialog,Energy* E);

	void ResetTime(){
		m_BeginTime = AbstractSpace::CreateTimeStamp();
	}; 	
	
	bool IsValid(){
		return m_LogicData.Size()>0;
	}


	CLogicTask* GetRootTask();

	
	bool Compile(CLogicDialog* Dialog,ePipeline* Sentence);
	CElement* CompileSentence(CLogicDialog* Dialog,tstring& LogicText,ePipeline* Sentence);
	CElement* CompileSentenceForTest(CLogicDialog* Dialog,tstring& LogicText,ePipeline* Sentence);
	bool CompileOK(){ return m_ActomList.size() !=0;};

	CLogicTask& operator<<(CLogicTask& Task);

	void Reset(int64 TaskID,int64 UserDialogID);
	
	void Clear();

	bool  SetBreakPoint(ePipeline& Path,BOOL bEnable);

    //用于调试VIEW的显示 
	void  GetDebugItem(ePipeline& ParentPipe,Mass* m=NULL);

};



#endif // !defined(_LOGICTASK_H__)
