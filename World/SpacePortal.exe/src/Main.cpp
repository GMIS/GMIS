// SpacePortal.cpp : Defines the entry point for the application.
//

#pragma warning (disable:4786)

#include "resource.h"
#include "SpacePortal.h"
#include "MainFrame.h"
#include "Win32Tool.h"
#include "UserTimer.h"
#include "UserMutex.h"
#include "SpaceMutex.h"
#include "UserSpacePool.h"
#include <tchar.h>
#include <commctrl.h>

//#include "vld.h"
#pragma comment(lib, "comctl32.lib")

using namespace ABSTRACT;

CMainFrame*  AfxGUI   = NULL;

SPACE_ACCOUNT     AfxAccount;

BOOL CALLBACK SetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


CMainFrame*  GetGUI(){
	assert(AfxGUI != NULL);
	return AfxGUI;
}


class CCheckDBWork: public Object{
	System*  m_Parent;
public:
	CCheckDBWork(System* Parent):m_Parent(Parent){};
	~CCheckDBWork(){};
	 void CheckWorldDB(){
		CppSQLite3Buffer  SQL;
		char Name[30];
		int64 ID = 2;
		vector<int64> TableList;
		bool FindEnd = false;

		GetGUI()->SetStatus(_T("Checking Space Database...")); 

		SQL.format("select  count(*) from sqlite_master where type='table'");
		CppSQLite3Query q = GetWorldDB().execQuery(SQL);	        
		if (q.eof())return;
		int32 Count = q.getIntField(0);
		if(Count==0)return;

		int32 n=0; 

		SPACETYPE RoomType = ROBOT_VISITER;
		while(m_Alive) 
		{
			//Each time we take out the 100-space checking, delete the ROBOT_VISIT residues 
			TableList.clear();    
			int64toa(ID,Name); 	
			SQL.format("select name from sqlite_master where type='table' and name > \"%s\"",Name);
			q = GetWorldDB().execQuery(SQL);	        
			if (q.eof())break;

			while (m_Alive && !q.eof())
			{
				ID = q.getInt64Field(0);
				TableList.push_back(ID);
				if (TableList.size()== 100)break;
				q.nextRow();
			}

			vector<int64>::iterator It = TableList.begin();
			while (m_Alive && It != TableList.end())
			{
				int64toa(*It,Name); 	
				SQL.format("delete from \"%s\"  where %s = %d ",Name,ITEM_TYPE,RoomType);
				GetWorldDB().execDML(SQL);
				It++;
				n++;
				GetGUI()->m_Status.SetProgressPer(n*100/Count);
			}

		};
		GetGUI()->m_Status.SetProgressPer(0);
		GetGUI()->m_Status.SetTip(_T(""));
	};

	virtual bool Do(Energy* E){
		if (GetGUI()==NULL)
		{
			return false;
		}
		try{
			CheckWorldDB();
		}catch (CppSQLite3Exception e) {

#ifdef _UNICODE
			AnsiString s = e.errorMessage();
			tstring ws = UTF8toWS(s);
			GetGUI()->AddRTInfo(ws.c_str());
#else
			GetGUI()->AddRTInfo(e.errorMessage());
#endif
			GetGUI()->AddRTInfo(_T("WARNING:  Space database check fail."));
		}catch(...){
			GetGUI()->AddRTInfo(_T("WARNING:  unkown exception occur ,Space database check fail."));
		};
		GetGUI()->AddRTInfo(_T("SpacePortal check Ok !"));

		if(GetGUI()->m_SecondHide != -1){
			tstring s = Format1024(_T("Hide window after %d second..."),GetGUI()->m_SecondHide);	
			GetGUI()->AddRTInfo(s.c_str());	
			::SetTimer(GetGUI()->GetHwnd(),TIMER_HIDEWIN,1000,NULL);
		};
		return true;
	}
};


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	

    HANDLE  MutexOneInstance = ::CreateMutexW( NULL, FALSE,
	  _T("SPACEPORTAL_CREATED"));

    bool AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
                       ::GetLastError() == ERROR_ACCESS_DENIED);
    // The call fails with ERROR_ACCESS_DENIED if the Mutex was 
    // created in a different users session because of passing
    // NULL for the SECURITY_ATTRIBUTES on Mutex creation);

    if ( AlreadyRunning )return 0;
	
	ePipeline CmdLinePipe;
	char* token;
	char* str = (char*)lpCmdLine;
	char* flag = " ";
	token = strtok(str,flag);
	while(token)
	{
		AnsiString s = token;
		CmdLinePipe.PushString(s);
		token = strtok(NULL,flag);
	}


	INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ics.dwICC  = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&ics);
	     
	CWinSpace2::RegisterCommonClass(hInstance);
  	
	CUserMutex  UIMutex;
	CWinSpace2::InitUIMutex(&UIMutex);

	//Initialize
	static CUserSpacePool     SpacePool; //Must be static to guarantee it be destructed in the last
	static CUserTimer         Timer;
	
	People  Host; 
	GetHost(&Host);//initialize，due to the static variable  can not control the destructor order, in order to avoid Host destruct ahead of the WorldDB , Host to local variables

	ROOM_SPACE RootRoom;
	GetRootRoom(&RootRoom); //initialize
	

	CSpacePortal SpacePortal(&Timer,&SpacePool);	
	if(!SpacePortal.Activate()){
		return 0;
	}

	CMainFrame   MainFrame;
	AfxGUI = &MainFrame;

	RECT rc;
	::SetRect(&rc,250,150,740,540);
	if(!MainFrame.Create(hInstance,_T("World"),WS_CLIPCHILDREN,rc,NULL)){
		int n = ::GetLastError();
		return 0;
	}

	HICON hIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MYWORLD));
    
	assert(hIcon != NULL);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);

	MainFrame.SetIcon(hIcon);

	tstring Dir = GetCurrentDir();
    Dir += _T("\\World.db");

    MainFrame.SetTitle(Dir.c_str());


	CCheckDBWork  CheckDBWork(&SpacePortal);
    CheckDBWork.Activate();

	//CenterWindow(MainFrame.GetHwnd(),NULL);
	::ShowWindow(MainFrame.GetHwnd(), SW_SHOW);
	::UpdateWindow(MainFrame.GetHwnd());

	if(!HasChild(ROOT_SPACE)){

		if(::MessageBox(NULL,_T("No Found World Database,Create new ?"),_T("Query"),MB_YESNO)==IDNO)return false;

		int ret = DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_SETACCOUNT), MainFrame.GetHwnd(), SetPasswordDlgProc, 0);
		if(ret>0){
			try{

				tstring Cryptograph = AfxAccount.Name+AfxAccount.Password;


				SpaceAddress Address(ROOT_SPACE,OUTER_SPACEID);
				tstring OuterName = _T("Unkown Name");  //Should be according to the Outer IP to get the name

				tstring Fingerprint = Format1024(_T("%I64ld"),OUTER_SPACEID); //Current space eigenvalue replaced by its subspace's ID for simplifying 

				ROOM_SPACE OuterRoom(ROOT_SPACE,OUTER_SPACEID,OuterName,0,OUTER_SPACE,NO_RIGHT,Fingerprint);

				Fingerprint = Format1024(_T("%I64ld"),LOCAL_SPACEID);
				ROOM_SPACE LocalRoom(ROOT_SPACE,LOCAL_SPACEID,AfxAccount.LocalName, 0,LOCAL_SPACE,NO_RIGHT,Fingerprint);

				LocalRoom.AddOwner(AfxAccount.Name.c_str(),Cryptograph,USABLE);

				People& Host = GetHost();
				Host.SetName(AfxAccount.Name);
				Host.m_Cryptograhp = Cryptograph;
				Host.SetSpaceType(ROBOT_HOST);

				ROOM_SPACE& RootRoom = GetRootRoom();
				bool ret = RootRoom.Logon(0,Host);
				assert(ret);
			}    
			catch (...) {
				return false;		
			}			
		}
		else return false;		
	}

	else 
	{
		bool bHostValid = false;
		if(CmdLinePipe.Size()==2)
		{

			AfxAccount.Name = CmdLinePipe.PopString();
			tstring CryptText = CmdLinePipe.PopString();

			People& Host = GetHost();
			Host.SetName(AfxAccount.Name);
			Host.m_Cryptograhp = CryptText;
			Host.SetSpaceType(ROBOT_HOST);

			ROOM_SPACE& RootRoom = GetRootRoom();
			if(RootRoom.Logon(0,Host)){			                
				bHostValid = true;				
			}
		}

		int i=0;
		while(!bHostValid && i++<3){
			SPACE_ACCOUNT param;
			int ret = ::DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_PASSWORD), 
				MainFrame.GetHwnd(), GetPasswordDlgProc, (long)&param);
			if(ret==0)break;
			try{

				People& Host = GetHost();
				Host.SetName(AfxAccount.Name);
				Host.m_Cryptograhp = AfxAccount.Name+AfxAccount.Password; 
				Host.SetSpaceType(ROBOT_HOST);

				ROOM_SPACE& RootRoom = GetRootRoom();
				if(RootRoom.Logon(0,Host)){	
					bHostValid = true;
					break;				
				}
			}catch(exception& roException){
#ifdef _UNICODE
				AnsiString str = roException.what();
				tstring s = UTF8toWS(str);
				SpacePortal.OutputLog(LOG_WARNING,s.c_str());
#else
				SpacePortal.OutputLog(LOG_WARNING,roException.what());
#endif
			}
			catch (...) {
				SpacePortal.OutputLog(LOG_WARNING,_T("Unkown error"));		
			}
		}	
		if (!bHostValid)
		{
			PostQuitMessage(0);
		}
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	Host.GoOut(); //initiative leave to avoid  error after the database has been shut down
	
    SpacePortal.Dead();

	if(MutexOneInstance)CloseHandle(MutexOneInstance);

	return 0;
}


BOOL CALLBACK SetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message)
	{
	case WM_INITDIALOG:
		{
			CenterWindow(hDlg,GetParent(hDlg));
			SPACE_ACCOUNT* ac = &AfxAccount; 
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{
			SPACE_ACCOUNT* ac = &AfxAccount; 
			HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_SETPASSWORD);
			HWND hConfirm  = GetDlgItem(hDlg,IDC_SETCONFIRM);
			HWND hLocalName = GetDlgItem(hDlg,IDC_LOCALNAME);

			//目前不可用
			HWND hOuterIP  = GetDlgItem(hDlg,IDC_OUTER_IP);
			::EnableWindow(hOuterIP,FALSE);

			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			ac->Name = buf;
			::GetWindowText(hPassword,buf,99);
			ac->Password = buf;
			::GetWindowText(hConfirm,buf,99);
			ac->Confirm = buf;
			::GetWindowText(hLocalName,buf,99);
			ac->LocalName  = buf;
			if(ac->Name.size()>0 && ac->Confirm.size()>0 && ac->Confirm == ac->Password){
				::SetWindowText(hName,_T(""));
				::SetWindowText(hPassword,_T(""));		
				::SetWindowText(hConfirm,_T(""));
				::SetWindowText(hLocalName,_T(""));
				if (ac->LocalName.size() == 0)
				{ 
					ac->LocalName == _T("");
				}
				::EndDialog(hDlg, 1);
			}
			return TRUE;
		}else if(LOWORD(wParam) == IDCANCEL){
			::EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
};

BOOL CALLBACK GetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){

	switch (message)
	{
	case WM_INITDIALOG:
		{
			CenterWindow(hDlg,GetParent(hDlg));
			SPACE_ACCOUNT* ac = (SPACE_ACCOUNT*)lParam;
			AfxAccount = *ac;
			ac = &AfxAccount;
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{
			SPACE_ACCOUNT* ac = &AfxAccount; 
			HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_EDITPASSWORD);
			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			ac->Name = buf;
			::GetWindowText(hPassword,buf,99);
			ac->Password = buf;
			if(ac->Name.size()>0 && ac->Password.size()>0 ){
				::SetWindowText(hName,_T(""));
				::SetWindowText(hPassword,_T(""));
				::EndDialog(hDlg, 1);
			}
			return TRUE;
		}else if(LOWORD(wParam) == IDCANCEL){
			::EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
};

