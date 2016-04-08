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

    DLL_TYPE GetDllType();
   
    bool IsValid();  
	
};
class CSpaceEvent{
public:
	enum {EVENT_INVALID,EVENT_PENDING,EVENT_DONE };
	int32       m_State;
	tstring     m_ObjectFilePath;
	int64       m_ClientEventID;
	int64       m_ClientLinkerID; //事件客户的地址
	ePipeline   m_ClientExePipe;  //临时保存事件客户的执行管道数据
public:
	CSpaceEvent()
		:m_State(EVENT_INVALID),m_ClientEventID(0),m_ClientLinkerID(-1)
	{
	};
	CSpaceEvent(tstring ObjectPath,int32 State,int64 ClientEventID,int64 ClientLinkerID,ePipeline& ExePipe)
	:m_ObjectFilePath(ObjectPath),m_State(State),m_ClientEventID(ClientEventID),m_ClientLinkerID(ClientLinkerID),m_ClientExePipe(ExePipe)
	{
	};
	bool IsValid(){return m_State != EVENT_INVALID;}
};

class CExecuter{
public:
	CUserMutex               m_Mutex;
	int64                    m_ID;   //对应Executer Linker ID
	DLL_TYPE                 m_Type;
	tstring                  m_ExecuterPath;
	map<int64, CSpaceEvent>  m_EventList;

public:
	CExecuter()
	:m_ID(0)
	{

	}
	CExecuter(DLL_TYPE Type,int64 ExecuterLinkerID)
	:m_ID(ExecuterLinkerID),m_Type(Type){
	}
	~CExecuter(){

	}


	bool IsValid(){
	    _CLOCK(&m_Mutex);
		return m_ID!=0;
	};

	void PushEvent(int64 SpaceEventID,CSpaceEvent& Event){
		_CLOCK(&m_Mutex);
		assert(m_EventList.find(SpaceEventID) == m_EventList.end());
		CSpaceEvent& SpaceEvent = m_EventList[SpaceEventID];
		SpaceEvent = Event;
	}
	bool PopEvent(int64 SpaceEventID, CSpaceEvent& Event){
		_CLOCK(&m_Mutex);
		map<int64, CSpaceEvent>::iterator it = m_EventList.find(SpaceEventID);
		if(it == m_EventList.end())return false;
		CSpaceEvent& SpaceEvent = it->second;
		Event = SpaceEvent;
		m_EventList.erase(it);
		return true;
	}
	void ProcessPendingEvent(CLinker& Linker){
		_CLOCK(&m_Mutex);
		assert(Linker().GetSourceID() == m_ID);
		map<int64, CSpaceEvent>::iterator it = m_EventList.begin();
		while(it != m_EventList.end()){
			CSpaceEvent& Event = it->second;

			CMsg NewMsg(MSG_OBJECT_START,DEFAULT_DIALOG,it->first);
			ePipeline& NewLetter = NewMsg.GetLetter();
			NewLetter.PushString(Event.m_ObjectFilePath);
			NewLetter.PushPipe(Event.m_ClientExePipe);

			Linker().PushMsgToSend(NewMsg);  

			Event.m_State = CSpaceEvent::EVENT_DONE;
			it++;
		}
	}
};
//#define _WriteLogDB(Frame,Format, ...) Frame->WriteLogDB(Format, __VA_ARGS__)


class CSpacePortal : public System
{
public:

	bool           m_bWriteLogToDatabase;
	CLogDatabase   m_LogDB;
protected:
	CUserMutex		                m_Mutex;
	map<int32,CExecuter*>           m_ExecuterList;

	CExecuter*  AddExecuter(DLL_TYPE Type,int64 ExecuterLinkerID);
	CExecuter*  FindExecuter(DLL_TYPE Type);
	CExecuter*  FindExecuterByLinker(int64 ExecuterLinkerID);

	void DeleteExecuter(int64 ExecuterLinkerID);
	void DeleteAllExecuter();
	/*
	map<int64,map < int64,int64 > >  m_ExecuterEventList;  //<ExecuterID,<EventID,int64ID>>
	map<int64 ,set < int64 > >       m_ExecuterUserList;   //<RobotID,set < ExecuterID >>

	deque<int64>                     m_ExecuterPool;       //Prior started executers


	void  PushExecuterEvent(int64 ExecuterID,int64 RobotID,int64 EventID);
	int64 PopExecuterEvent(int64 ExecuterID,int64 EventID);

	void  RegisterExecuterUser(int64 RobotID,int64 ExecuterID);
	*/
	map<int64,People*>               m_VisitorList;         //int64 = linker souceID
	deque<People*>                   m_VisitorPool;
	People*  CheckinVisitor(int64 SourceID,tstring& Name,tstring& CryptText);
	People*  GetVisitor(int64 SourceID);
	void           CheckoutVisitor(int64 SourceID);
	//CUserMutex                    m_ExeMutex;            
public:
	CSpacePortal(CUserTimer* Timer,CUserSpacePool* Pool);
	virtual ~CSpacePortal();
	virtual tstring MsgID2Str(int64 MsgID);
	
	virtual bool Activate();

	virtual void  OutputLog(uint32 Type,const TCHAR* text);

	void WriteLogDB(TCHAR* Format,tstring& s);

	void GetLinker(int64 SourceID,CLinker& Linker);
 
    void PrintMsg(int64 ID,bool Send ); // send or get msg


protected:  //World work
	virtual void    CentralNerveMsgProc(CMsg& Msg);
	void    OnAppOnline(CMsg& Msg);
	void    OnObjectFeedback(CMsg& Msg);
	void    OnI_AM(CMsg& Msg);
    void    OnGotoSpace(CMsg& Msg);
    void    OnAskforSpaceBody(CMsg& Msg);
	void    OnCreateSpace(CMsg& Msg);
	void    OnDeleteSpace(CMsg& Msg);
    void    OnExportObject(CMsg& Msg);
	void    OnTaskRequest(CMsg& Msg);
		void    OnRequestStartObject(int64 SourceID,int64 EventID,ePipeline& RequestInfo);
		void    OnRequestUseObject(int64 SourceID,int64 EventID,ePipeline& RequestInfo);
		void    OnRequestCloseObject(int64 SourceID,int64 EventID,ePipeline& RequestInfo);
		void    OnRequestGetObjectDoc(int64 SourceID,int64 EventID, ePipeline& RequestInfo);

protected:
	//bool    StartExecuter(int64 ExecuterID,tstring FileName);

	void    UserLinkerClosedProc(int64 SourceID);
	void    ExecuteLinkerClosedProc(int64 SourceID);

	void    CreateObject(int64 ParentID,ePipeline& Letter);
	void    CreateRoom(int64 ParentID,SPACETYPE RoomType,ePipeline& Letter);
  	

	virtual void  NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Data);
	
	virtual void    NerveMsgProc(CMsg& Msg);

};



#endif // _SPACEPORTAL_H__
