#ifndef _EXECUTER_H_
#define _EXECUTER_H_

#include "PhysicSpace.h"
#include "Model.h"
#include "MainFrame.h"

tstring GetCurrentDir();
tstring GetTempDir();



class CSetCurWorkDir{
public:
	static tstring  m_DefaultCurDir;
protected:	
	CUserMutex      m_Mutex;

public:
	CSetCurWorkDir(tstring& NewDir){
		m_Mutex.Acquire();
		::SetCurrentDirectoryW(NewDir.c_str());
	};
	~CSetCurWorkDir(){
		::SetCurrentDirectoryW(m_DefaultCurDir.c_str());
		m_Mutex.Release();
	}

};

class CExecuter;

class CExObject
{
	class AutoBool{
		bool&  m_Flag;
	public:
		AutoBool(bool& Flag):m_Flag(Flag){
			m_Flag = true;
		};
		~AutoBool(){
			m_Flag = false;
		}
	};
public:	

	tstring     m_FileAddress;
	Mass*       m_Object;
	HINSTANCE   m_DLLHandle;
	bool        m_bRuning;  //指示是否正在执行，避免多线程下被随意删除

public:
	CExObject();
	virtual ~CExObject();

	tstring  GetName(){
		return m_FileAddress;
	};

	Mass* LoadObject(tstring DllFileName);
	void  DeleteObject();
	bool  GetObjectDoc(tstring& Doc);

	bool IsValid(){
		return m_Object != NULL;
	}
	bool IsRuning(){
		return m_bRuning;
	}

	bool WaitForStop(int64 Count);
	virtual TypeAB   GetTypeAB();  

	void Start(CExecuter* Executer,int64 EventID,ePipeline* ExePipe);
	void Run(CExecuter* Executer,int64 EventID,ePipeline* ExePipe);
	void GetDoc(CExecuter* Executer,int64 EventID,ePipeline* ExePipe);

};

class CExecuter : public Model
{
protected:
	tstring               m_CryptText;
	CUserMutex            m_ObjectListMutex;
	map<int64,CExObject>  m_ObjectList;

	CExObject& CreateNewObject(int64 EventID);
	CExObject& FindObject(int64 EventID);
	void DeleteObject(int64 EventID);
	void DeleteAllObject();
public:
	CExecuter(int64 ID,tstring CryptText,CUserTimer* Timer,CUserSpacePool* Pool);
	virtual ~CExecuter();

	virtual tstring MsgID2Str(int64 MsgID);

	void UnitTest();

	void GetSuperior(int64 ID,CLinker& Linker);
	
	virtual void	OutputLog(uint32 Type,const TCHAR* text);
	void OutputLog(uint32 Flag,TCHAR* Format, ...);

	virtual void NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info);

	void  SendFeedback(int64 EventID,ePipeline& Param);
public:
	virtual void    CentralNerveMsgProc(CMsg& Msg);

	void OnWhoAreYou(CMsg& Msg);
	void OnConnectTo(CMsg& Msg);
	void OnConnectOK(CMsg& Msg);
	void OnLinkerNotify(CMsg& Msg);


	void OnObjectStart(CMsg& Msg);
	void OnObjectRun(CMsg& Msg);
	void OnObjectClose(CMsg& Msg);
	void OnObjectGetDoc(CMsg& Msg);
	void OnAppReset(CMsg& Msg);
	void OnBroadcasgMsg(CMsg& Msg);

};

#endif   