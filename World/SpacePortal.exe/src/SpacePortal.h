/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SPACEPORTAL_H__
#define _SPACEPORTAL_H__


#include "PhysicSpace.h"
#include <map>
#include <set>

#include "space.h"
#include "LogDatabase.h"
#include "SpaceMsgList.h"

tstring GetCurrentDir();
tstring GetTempDir();
tstring SpacePath2FileAddress(ePipeline& Path);

class Dll_Object{
public:
	int64       m_ID;
	tstring     m_FilePath;
	HINSTANCE   m_hDll; 

public:
	Dll_Object();
	Dll_Object(int64 ObjectID,tstring& FileName);
	~Dll_Object();

	Dll_Object& operator=(const Dll_Object& dll);

    int32 GetDllType();
   
    bool IsValid();  
	
};
enum CLIENT_TYPE{
	EXE_SELF =	1,			
	EXE_OTHERSPACE,		
	EXE_OBJECT,			
	EXE_ROBOT			
};
class CSpaceEvent{
public:
	int64				m_ClientEventID;
	CLIENT_TYPE	  	    m_ClientType;
	int64               m_ClientLinkerID;
 	int64               m_ExecuterLinkerID;   //=0 表示客户和执行者的链接还没完全建立
	int64               m_ExecuterEventID;
	int64               m_ExecuterType;
	bool                m_bIsBusy;            //一个事件不能同时被多次调用       
	ePipeline			m_EventData;

public:
	CSpaceEvent()
	:m_ClientEventID(0),m_ClientLinkerID(0),m_ClientType(EXE_SELF),
	 m_ExecuterLinkerID(0),m_ExecuterEventID(0),m_ExecuterType(EXE_SELF),m_bIsBusy(false)
	{

	};
};


//#define _WriteLogDB(Frame,Format, ...) Frame->WriteLogDB(Format, __VA_ARGS__)


class CSpacePortal : public System
{
public:

	bool           m_bWriteLogToDatabase;
	CLogDatabase   m_LogDB;

protected:
	CUserMutex		                m_Mutex;
	map<int64, CSpaceEvent>         m_EventList;

	map<int64,People*>              m_VisitorList;         //int64 = linker souceID
	deque<People*>                  m_VisitorPool;

	int32                           m_MaxEventNumPerLinker;   //每一个链接允许发起的事件数目，缺省=10；

protected:

	People*		CheckinVisitor(int64 SourceID,tstring& Name,tstring& CryptText);
	People*		GetVisitor(int64 SourceID);
	int64       GetVisitorLinkerID(tstring& Name,tstring& Fingerprint);
	void        CheckoutVisitor(int64 SourceID);
	
public:
	CSpacePortal(CUserTimer* Timer,CUserSpacePool* Pool);
	virtual ~CSpacePortal();
	virtual tstring MsgID2Str(int64 MsgID);
	
	virtual bool	Activate();
	virtual void	OutputLog(uint32 Type,const TCHAR* text);

	void			WriteLogDB(TCHAR* Format,tstring& s);
	void			GetLinker(int64 SourceID,CLinker& Linker);
    void			PrintMsg(CMsg& msg,bool Send ); // send or get msg

	int64  PushEvent(CLIENT_TYPE Type,int64 ClientLinkerID,int64 ClientEventID,ePipeline& EventData,int64 ExecuterType,int64 ExecuterLinkerID,int64 ExecuterEventID);

	bool GetEvent(int64 SpaceEventID, CSpaceEvent& Event);
	void PopEvent(int64 SpaceEventID);
	void ModifyEvent(int64 SpaceEventID, CSpaceEvent& Event);
protected:  //World work

	virtual void    CentralNerveMsgProc(CMsg& Msg);
	
	void    OnTaskFeedback(CMsg& Msg);
	void    OnI_AM(CMsg& Msg);
    void    OnGotoSpace(CMsg& Msg);
    void    OnAskforSpaceBody(CMsg& Msg);
	void    OnCreateSpace(CMsg& Msg);
	void    OnDeleteSpace(CMsg& Msg);
    void    OnExportObject(CMsg& Msg);

	void    OnStartObject(CMsg& Msg);
	void    OnUseObject(CMsg& Msg);
	void    OnCloseObject(CMsg& Msg);
	void    OnGetObjectDoc(CMsg& Msg);

	void    OnStartRequest(CMsg& Msg);
	void    OnRequestRuntime(CMsg& Msg);
	void    OnExecuteRequest(CMsg& Msg);
	void    OnCloseRequest(CMsg& Msg);


protected:

	void    LinkerBreakEventProc(int64 SourceID);
	int32   CountingClientEventNum(int64 SourceID);  
	void    CreateObject(int64 ParentID,ePipeline& Letter);
	void    CreateSpace(int64 ParentID,SPACETYPE SpaceType,ePipeline& Letter);
  	
	virtual void  NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Data);
	virtual void  NerveMsgProc(CMsg& Msg);

};



#endif // _SPACEPORTAL_H__
