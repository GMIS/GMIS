/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/
#ifndef _EXECUTERFRAME_H__
#define _EXECUTERFRAME_H__

#include "space.h"
#include "VisibleSpace.h"
#include "CLog.h"
#include "EditLog.h"
#include "editlog_stream.h"
#include "WSFrameView.h"
#include "PhysicSpace.h"


using namespace VISUALSPACE;

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
class CMainFrame;

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
	int64       m_ID;
	tstring     m_FileAddress;
	Mass*       m_Object;
	HINSTANCE   m_DLLHandle;
	bool        m_bRuning;  //指示是否正在执行，避免多线程下被随意删除

public:
	CExObject();
	CExObject(int64 ID,tstring& File);
	virtual ~CExObject();
	
    tstring  GetName(){
		return m_FileAddress;
	};

	Mass* LoadObject(tstring DllFileName);
	void  DeleteObject();
	bool  GetObjectDoc(tstring& Doc);
	
	bool IsRuning(){
		return m_bRuning;
	}

	bool WaitForStop(int64 Count);
	virtual TypeAB   GetTypeAB();  

	void Start(CMainFrame* Frame,int64 EventID,ePipeline* ExePipe);
	void Run(CMainFrame* Frame,int64 EventID,ePipeline* ExePipe);
	void GetDoc(CMainFrame* Frame,int64 EventID,ePipeline* ExePipe);
};



class CMainFrame : public Model, public CWSFrameView   
{
protected:
	int64         m_Alias;

    CExObject     m_Object;
		
protected: //GUI

	BOOL          m_Created;
	HWND          m_hEdit;
	CEditLog      m_AppLog;
	
	// ANSI and UNICODE stream buffers for the EditLogger
	std::editstreambuf	m_EditStrBuf;
	std::weditstreambuf m_EditStrBufW;
	
	// Used to save the previos values for cout and wcout
	std::basic_streambuf<char>*		m_OldBuf;
	std::basic_streambuf<wchar_t>*	m_OldBufW;
	
	HBRUSH                          m_BKBrush;
	
public:
	CMainFrame(int64 Alias,CModelInitData* InitData);
	virtual ~CMainFrame();
	
	virtual tstring MsgID2Str(int64 MsgID);

	void GetSuperior(int64 ID,CLinker& Linker);
	void	SendMsg(CMsg& Msg);

	virtual void	OutputLog(uint32 Type,const TCHAR* text);
	void OutputLog(uint32 Flag,TCHAR* Format, ...);

	//承继的虚函数
	virtual void NotifySysState(int64 NotifyID,ePipeline* Data = NULL);	
	virtual void NotifyLinkerState(CLinkerPipe* LinkerPipe,int64 NotifyID,ePipeline& Info);


	void  SendFeedback(int64 EventID,ePipeline& Param);
public:
	virtual bool Do(Energy* E=NULL);
	
	void OnWhoAreYou(CMsg& Msg);
	void OnConnectTo(CMsg& Msg);
    void OnConnectOK(CMsg& Msg);
	void OnLinkerError(CMsg& Msg);
    void OnLinkerNotify(CMsg& Msg);


	void OnObjectStart(CMsg& Msg);
    void OnObjectRun(CMsg& Msg);
	void OnObjectClose(CMsg& Msg);
	void OnObjectGetDoc(CMsg& Msg);
	void OnAppReset(CMsg& Msg);
	void OnBroadcasgMsg(CMsg& Msg);

protected:

	virtual void Layout(bool Redraw  = true );
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(AFX_EXECUTERFRAME_H__0D5A163B_8A1B_4FC3_87F4_92CFB2FCDE36__INCLUDED_)
