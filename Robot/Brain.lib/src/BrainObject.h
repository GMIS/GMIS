/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BRAINOBJECT_H__
#define _BRAINOBJECT_H__

#include "Element.h"
#include "UserTimer.h"

#ifdef _WIN32
#include <WINSOCK2.H>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

//

//常用MASS
////////////////////////////////////

class CLogicThread;
class CTaskDialog;
class CObjectData;
class CArm;

class  CBreakMass: public Mass
{
	Mass*  m_SrcMass;
public:
	CBreakMass(Mass* srcMass)
		:m_SrcMass(srcMass)
	{
		assert(srcMass);
		m_ID = srcMass->m_ID;
	}

	virtual ~CBreakMass(){
		if (m_SrcMass)
		{
			delete m_SrcMass;
			m_SrcMass = NULL;
		}
	}
	virtual MASS_TYPE  MassType(){ return MASS_USER;}; 
	virtual TypeAB GetTypeAB(){ 
	    assert(m_SrcMass);
		return m_SrcMass->GetTypeAB();
	};
    virtual tstring GetName(){
		assert(m_SrcMass);
		return m_SrcMass->GetName();
	};
	virtual bool  Do(Energy* E){
		assert(m_SrcMass);
		ePipeline* Pipe = (ePipeline*)E;
		Pipe->SetID(RETURN_BREAK);
		bool ret = m_SrcMass->Do(E);
		return ret;
	};

	Mass* Release(){
		Mass* m = m_SrcMass;
		m_SrcMass = NULL;
		return m;
	}
};

//////////////////////////////////////////////////////////////////////////


class  CWaitSecond_Static : public CElement  
{
public:
	float64    m_Second;  //等待时间
	
	int64      m_StartTimeStamp;
    bool       m_bPause;
public:
	CWaitSecond_Static(int64 ID,float64 Second)
		:CElement(ID,_T("Wait")),m_Second(Second),m_bPause(false){
	};	
	
	virtual ~CWaitSecond_Static(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};	

class  CWaitSecond: public CWaitSecond_Static  
{
public:
	CWaitSecond(int64 ID)
		:CWaitSecond_Static(ID,0.0f){
	};	
	
	virtual ~CWaitSecond(){};
	
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};	

//GOTO LABLE在转向之前会向系统申请一次中断，这样可以避免在无限转向循环中其他并联分支没有机会执行
class CGotoLabel: public CElement
{
private:
	tstring      m_Name;
	tstring      m_GotoLabel;

public:
	CGotoLabel(const TCHAR* Name,int64 ID,tstring& Label)
		:CElement(ID,Name),m_GotoLabel(Label)
	{		
	};
	virtual ~CGotoLabel(){};
	
	
	virtual TypeAB GetTypeAB(){ return 0;};
	
	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class CGetDate: public Mass
{

public:
	CGetDate(int64 ID=0):Mass(ID){};
	virtual ~CGetDate(){};

	virtual tstring      GetName(){ return _T("GetDate");};
	virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
	virtual bool Do(Energy* E){
		int64 CurentTime = AbstractSpace::CreateTimeStamp();
		tstring s = AbstractSpace::GetTimer()->GetYMD(CurentTime);
		ePipeline* Pipe = (ePipeline*)E;
		Pipe->PushString(s);
		return true;
	};
};

class CGetTime: public Mass
{
	
public:
	CGetTime(int64 ID=0):Mass(ID){};
	virtual ~CGetTime(){};
	
	virtual tstring      GetName(){ return _T("GetTime");};
	virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
	virtual bool Do(Energy* E){
		int64 CurentTime = AbstractSpace::CreateTimeStamp();
		tstring s = AbstractSpace::GetTimer()->GetHMS(CurentTime);
		ePipeline* Pipe = (ePipeline*)E;
		Pipe->PushString(s);
		return true;
	};
};

class COutputInfo: public CElement
{
public:
	COutputInfo(int64 ID=0):CElement(ID,_T("Output Info")){};
	virtual ~COutputInfo(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x30000000;} 

    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//常用内部Element
////////////////////////////////////

class CInterBrainObject: public CElement
{
protected:

	int64           m_InstinctID;
    ePipeline       m_Param;
	
	CInterBrainObject();
public:
	CInterBrainObject(int64 ID,tstring Name,int64 InstinctID,ePipeline& Param);
	~CInterBrainObject();
	
	virtual bool  Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
	virtual TypeAB GetTypeAB(){ return 0;};
protected:
   	bool  DoThinkLogic(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoRunTask(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoDebugTask(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoStopTask(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoPauseTask(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoStepTask(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoGotoTask(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
    bool  DoLearnText(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnWord(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
    bool  DoLearnLogic(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnObject(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnAction(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnMemory(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFind(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFindLogic(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFindObject(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
	bool  DoSetGlobleLogic(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
	bool  DoFindSetStartTime(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFindSetEndTime(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoSetFindPricision(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
		
	bool  DoCloseDialog(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
};



class CInputElement: public CElement
{
public:   
	bool             m_bInputNum;  //输入的必须是数字
protected:
	
	CInputElement():CElement(0,_T("")),m_bInputNum(false){}; //不允许空引用；
public:
	CInputElement(int64 ID,tstring Tip,bool bInputNum);
	virtual ~CInputElement();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	
	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};




class CStartObject: public CElement
{
public:   
	ePipeline*       m_Obj;
protected:
	
	CStartObject():CElement(0,_T("")){}; //不允许空引用；
public:
	CStartObject(int64 ID,ePipeline* Obj);
	virtual ~CStartObject();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
};

class CNameObject: public CElement
{
	
public:
	CNameObject(int64 ID);  
	virtual ~CNameObject();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class CNameObject_Static: public CElement
{
	tstring  m_Name;	
public:
	CNameObject_Static(int64 ID,tstring Name);  
	virtual ~CNameObject_Static();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class CFocusObject: public CElement
{
public:
	CFocusObject(int64 ID)
		:CElement(ID,_T("Focus object"))
	{

	};
	virtual ~CFocusObject(){
	
	};
	
	virtual TypeAB  GetTypeAB(){ return 0x30000000;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

//静态版本，即focus object名字直接由参数提供
class CFocusObject_Static: public CElement
{
	tstring  m_FocusName;
public:
	CFocusObject_Static(int64 ID,tstring FocusName)
		:CElement(ID,_T("Focus object")),m_FocusName(FocusName)
	{
		
	};
	virtual ~CFocusObject_Static(){
		
	};
	
	virtual TypeAB  GetTypeAB(){ return 0;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

//内部版本，即focus object instance ID名字直接由参数提供
class CFocusObject_Inter: public CElement
{
	int64   m_InstanceID;
public:
	CFocusObject_Inter(int64 ID,int64 InstanceID)
		:CElement(ID,_T("Focus object")),m_InstanceID(InstanceID)
	{

	};
	virtual ~CFocusObject_Inter(){

	};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};
class CUseObject: public CElement
{
	int64   m_InstanceID;
public:
	CUseObject(int64 ID);
	virtual ~CUseObject();
	
	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class CGetObjectDoc: public CElement
{
public:
	CGetObjectDoc(int64 ID);
	virtual ~CGetObjectDoc();

	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};


class CCloseObject: public CElement
{
public:   
	tstring          m_ObjectName;
public:
	CCloseObject(int64 ID);

	virtual ~CCloseObject();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CTaskDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
};



class  CCreateTable : public CElement  
{
	tstring  m_MemoryName;
public:
	CCreateTable(int64 ID,tstring MemoryName)
		:CElement(ID,_T("Create Memory")),m_MemoryName(MemoryName){
	};
	virtual ~CCreateTable(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class CFocusTable: public CElement
{
public:
	CFocusTable(int64 ID)
		:CElement(ID,_T("Focus Memory")){
	};;
	virtual ~CFocusTable(){};
	
	virtual TypeAB  GetTypeAB(){ return 0;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};

//静态版本，即focus memory名字直接由参数提供
class CFocusMemory_Static: public CElement
{
	tstring m_FocusName;
public:
	CFocusMemory_Static(int64 ID,tstring FocusName)
		:CElement(ID,_T("Focus Memory")),m_FocusName(FocusName){
	};;
	virtual ~CFocusMemory_Static(){};
	
	virtual TypeAB  GetTypeAB(){ return 0;};
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class  CTable_ImportData : public CElement  
{
public:
	CTable_ImportData(int64 ID)
		:CElement(ID,_T("Insert memory data")){
	};	

	virtual ~CTable_ImportData(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};

class  CTable_ExportData : public CElement  
{
public:
	CTable_ExportData(int64 ID)
		:CElement(ID,_T("Insert memory data")){
	};	

	virtual ~CTable_ExportData(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};

class  CTable_InsertData : public CElement  
{
public:
	CTable_InsertData(int64 ID)
		:CElement(ID,_T("Insert memory data")){
	};	

	virtual ~CTable_InsertData(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};
class CTable_ModifyData: public CElement  
{
public:
	CTable_ModifyData(int64 ID)
		:CElement(ID,_T("Modify memory data")){
	};	

	virtual ~CTable_ModifyData(){
	};

	virtual TypeAB  GetTypeAB(){ return 0x10000000;};	
	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};

class  CTable_GetData : public CElement  
{
public:
	CTable_GetData(int64 ID)
		:CElement(ID,_T("Get memory data")){
	};	
	
	virtual ~CTable_GetData(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x10000000;};
	
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CTable_RemoveData : public CElement  
{
public:
	CTable_RemoveData(int64 ID)
		:CElement(ID,_T("Remove memory data")){
	};	
	
	virtual ~CTable_RemoveData(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x10000000;};
	
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CTable_GetSize : public CElement  
{
public:
	CTable_GetSize(int64 ID)
		:CElement(ID,_T("Get memory size")){
	};	
	
	virtual ~CTable_GetSize(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class  CCloseTable : public CElement  
{
public:
	CCloseTable(int64 ID)
		:CElement(ID,_T("Close memory ")){
	};	
	
	virtual ~CCloseTable(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};
//////////////////////////////////////////////////////////////////////////

static CElement* FindFocusLogic(CElement* Elt,tstring FocusName);

class CFocusLogic: public CElement
{
public:
	CFocusLogic(int64 ID)
		:CElement(ID,_T("Focus Logic")){
	};;
	
	virtual ~CFocusLogic(){
		
	};
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class CFocusLogic_Static: public CElement
{
	tstring   m_FocusName;
public:
	CFocusLogic_Static(int64 ID,tstring FocusName)
		:CElement(ID,_T("Focus Logic")),m_FocusName(FocusName){
	};;
	
	virtual ~CFocusLogic_Static(){
		
	};
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool    TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};

class  CInserLogic : public CElement  
{
public:
	tstring     m_InsertLogicName;
public:
	CInserLogic(int64 ID,tstring LogicName)
		:CElement(ID,_T("Insert Logic")),m_InsertLogicName(LogicName){
	};	
	
	virtual ~CInserLogic(){
		
	};
	
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};	


class  CRemoveLogic : public CElement  
{
public:
	CRemoveLogic(int64 ID)
		:CElement(ID,_T("Remove Logic")){
	};	
	
	virtual ~CRemoveLogic(){};
	
	virtual TypeAB  GetTypeAB(){ return 0x00000000;};
	
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};	




//////////////////////////////////////////////////////////////////////////
class CUseRobot: public CElement
{
public:
	CTaskDialog*   m_Dialog;
	int64          m_RobotID;
	ePipeline      m_TaskLogic;  //其中m_Label是此逻辑的明文
	
protected:
	CUseRobot():CElement(0,_T("")){}; //不允许空引用；
public:
	CUseRobot(int64 ID,CTaskDialog* Dialog, int64 RobotID,ePipeline& LogicPipe);
	virtual ~CUseRobot();
	
	virtual TypeAB GetTypeAB();
    virtual tstring GetName();

	virtual MsgProcState MsgProc(CTaskDialog* Dialog,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress,int32& ChildIndex);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class  CConnectSocket : public CElement  
{
	enum STATE { PHYSICAL_CON, LOGIC_CON,END_CON}; 
public:
	STATE        m_ConState;
	int64        m_LinkerID;
	
	int64        m_StopTime;
	SOCKET       m_Socket;
public:
	CConnectSocket(int64 ID): CElement(ID,_T("Connect")),m_ConState(PHYSICAL_CON),m_StopTime(0),m_Socket(NULL){};
	~CConnectSocket(){};
	
	virtual bool Do(ePipeline* Pipe,ePipeline* LocalAddress);
	
protected:
    bool  CheckConnected(ePipeline* Pipe, ePipeline* LocalAddress);
	virtual void SysMsgProc(CTaskDialog* Dialog,CMsg& SysMsg,ePipeline* ExePipe,ePipeline* LocalAddress);
};


// CPipeview是一个质量体，但与ePipeline密切相关所以放在一个目录
class  CPipeViewMass  : public CElement
{
protected:
	int32            m_TabNum;
	
	void PrintView(ePipeline& Result,ePipeline& Pipe);
public:
	CPipeViewMass(int64 ID)
		:CElement(ID,_T("PipeView")){
	};
	virtual ~CPipeViewMass(){};
	
	void GetPipeInfo(ePipeline* Pipe,tstring& text);
    virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};

/*
class CUseArm: public CElement
{
	CArm&    m_Arm;
public:
	CUseArm(int64 ID,CArm& Arm);
	virtual ~CUseArm();

	virtual TypeAB  GetTypeAB(){ return 0x00000000;} 

	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};
*/
#endif // !defined(AFX_BRAINOBJECT_H__48B86D08_4A8A_44A9_BEE5_F7AD3334BD3F__INCLUDED_)
