// Executer.cpp : Defines the entry point for the application.
//
#pragma warning(disable: 4786)

#include "MainFrame.h"
#include "Executer.h"
#include "UserTimer.h"
#include "UserMutex.h"
#include "SpaceMsgList.h"
#include <conio.h >

#define MAX_LOADSTRING 100

#define USING_GUI 

using namespace ABSTRACT;

#pragma comment(lib, "comctl32.lib")

/*
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
*/

int _tmain(int argc, _TCHAR* argv[])
{

	ePipeline CmdPipe;	
	for(int i=1; i<argc; i++)
	{
		tstring s = argv[i];
		CmdPipe.PushString(s);
	}

	if (CmdPipe.Size()==0)
	{
		CmdPipe.PushString(_T("executer"));
		CmdPipe.PushString(_T("101"));
	}

	assert(CmdPipe.Size()==2);
	if (CmdPipe.Size()!=2)return 0;

	
	tstring CryptText = CmdPipe.PopString();

    tstring s = CmdPipe.PopString();
	int32 DllType = _ttoi(s.c_str());
	assert(DllType);
	
	static CUserMutex  UIMutex;
	CWinSpace2::InitUIMutex(&UIMutex);
	

	static CUserSpacePool SpacePool;	
	static CUserTimer  Timer;

	//实例一个Model
	tstring AppName = Format1024(_T("Executer(Type:%d)"),DllType);

	CExecuter  Executer(DllType,CryptText,&Timer,&SpacePool);

	if(!Executer.Activate()){
		return 0;
	}
	Executer.OutputLog(LOG_TIP,_T("Activation oK"));


	tstring ip = _T("127.0.0.1");
	CMsg Msg(MSG_CONNECT_TO,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter();
	Letter.PushInt(SYSTEM_SOURCE);
	Letter.PushString(ip);
	Letter.PushInt(SPACE_PORT);
	Letter.PushInt(5);  //等待5秒

	Executer.PushCentralNerveMsg(Msg,false,true);

	Executer.Do(NULL);

	Executer.Dead();

	getch();
	return 0;
}


