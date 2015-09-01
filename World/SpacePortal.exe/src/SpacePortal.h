/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SPACEPORTAL_H__
#define _SPACEPORTAL_H__


#include "PhysicSpace.h"
#include "PhysicSpace.h"
#include "WorldLog.h"
#include "StatusTip.h"
#include "WSFrameView.h"
#include <map>
#include <set>

#include "space.h"
#include "LogDatabase.h"

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


//#define _WriteLogDB(Frame,Format, ...) Frame->WriteLogDB(Format, __VA_ARGS__)

#define  TIMER_HIDEWIN 99
class CSpacePortal : public System, public CWSFrameView  
{
public:

	CStatusTip     m_Status;
	BOOL           m_Created;
	HANDLE         m_InitThread;

	int            m_SecondHide;   //defualt = 10; 
	int            m_Second;
    
	ePipeline      m_CmdLine;  //Command line parsing results
		
	CWorldLog      m_Log;

	bool           m_bWriteLogToDatabase;
	CLogDatabase   m_LogDB;
protected:
	CUserMutex		                 m_Mutex;
	map<int64,map < int64,int64 > >  m_ExecuterEventList;  //<ExecuterID,<EventID,int64ID>>
	map<int64 ,set < int64 > >       m_ExecuterUserList;   //<RobotID,set < ExecuterID >>

	vector<int64>                    m_ExecuterPool;       //Prior started executers

	void  PushExecuterEvent(int64 ExecuterID,int64 RobotID,int64 EventID);
	int64 PopExecuterEvent(int64 ExecuterID,int64 EventID);

	void  RegisterExecuterUser(int64 RobotID,int64 ExecuterID);


	//CUserMutex                    m_ExeMutex;            
public:
	CSpacePortal(CSystemInitData* InitData);
	virtual ~CSpacePortal();
	virtual tstring MsgID2Str(int64 MsgID);
	
	//Delete all space's ROBOT_VISIBLE
	void    CheckWorldDB();
	static unsigned __stdcall InitThreadFunc (LPVOID pParam);

	virtual bool Activation();
	virtual void Dead();
	virtual bool Do(Energy* E=0);

	void AddRTInfo(tstring& s);
    void AddRTInfo(const TCHAR* Text);

	void WriteLogDB(TCHAR* Format, ...){
		if (!m_bWriteLogToDatabase)
		{
			return;
		}
		TCHAR Buffer [512] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
		_vsntprintf(Buffer, 512, Format, ArgList) ;		
		va_end (ArgList) ;

		int64 TimeStamp = AbstractSpace::CreateTimeStamp();
		m_LogDB.WriteItem(TimeStamp,_T("SpacePortal"),Buffer);
	};

    void    CloseLinker(int64 ID);

    void PrintMsg(int64 ID,bool Send = false); // send or get msg

	//if=-1 not hide
	void SetHideSecond(int Second){ m_SecondHide = Second;}  
protected:  //World work
	
	void    OnAppOnline(CMsg& Msg);
	void    OnObjectFeedback(CMsg& Msg);
	void    OnLinkerError(CMsg& Msg);   
	void    OnI_AM(CMsg& Msg);
    void    OnGotoSpace(CMsg& Msg);
    void    OnAskforSpaceBody(CMsg& Msg);
	void    OnCreateSpace(CMsg& Msg);
	void    OnDeleteSpace(CMsg& Msg);
    void    OnExportObject(CMsg& Msg);
	void    OnTaskRequest(CMsg& Msg);
		void    OnRequestStartObject(int64 SourceID,ePipeline& RequestInfo);
		void    OnRequestUseObject(int64 SourceID,ePipeline& RequestInfo);
		void    OnRequestCloseObject(int64 SourceID,ePipeline& RequestInfo);
		void    OnRequestGetObjectDoc(int64 SourceID, ePipeline& RequestInfo);

protected:
	bool    StartExecuter(int64 ExecuterID,tstring FileName);

	void    UserLinkerClosedProc(int64 SourceID);
	void    ExecuteLinkerClosedProc(int64 SourceID);

	void    CreateObject(int64 ParentID,ePipeline& Letter);
	void    CreateRoom(int64 ParentID,SPACETYPE RoomType,ePipeline& Letter);
  	

	virtual void  OutputLog(uint32 Type,const TCHAR* text);
	virtual void  NotifyLinkerState(CLinkerPipe* Linker,int64 NotifyID,ePipeline& Data);
	virtual Model::CCentralNerveWork* CreateCentralNerveWorker(int64 ID, Model* Parent,uint32 Reason);
	virtual System::CNerveWork*	      CreateNerveWorker(int64 ID,System* Parent,uint32 Reason);

protected:
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void  Layout(bool Redraw=true);

	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);

};



#endif // _SPACEPORTAL_H__
