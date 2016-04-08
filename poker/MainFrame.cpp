// MainFrame.cpp: implementation of the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4786)

//#include "vld.h"
#include "MainFrame.h"
#include "BitmapEx.h"
#include "crc32.h"
#include <commctrl.h>
#include "MsgList.h"
#include "OcrResultView.h"
#include "UserLinkerPipe.h"
#include <Wininet.h>

MY_ACCOUNT     AfxAccount;

BOOL CALLBACK GetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){

	switch (message)
	{
	case WM_INITDIALOG:
		{
			MY_ACCOUNT* ac = (MY_ACCOUNT*)lParam;
			AfxAccount = *ac;
			ac = &AfxAccount;
			HWND hName = GetDlgItem(hDlg,IDC_EDITNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_EDITPASSWORD);
			::SetWindowText(hName,ac->Name.c_str());
			::SetWindowText(hPassword,ac->Password.c_str());

			HWND hIP   = GetDlgItem(hDlg,IDC_IPADDRESS1);
			::SendMessage(hIP,IPM_SETADDRESS,0,MAKEIPADDRESS(ac->ip.seg1,ac->ip.seg2,ac->ip.seg3,ac->ip.seg4));
			::SetFocus(GetDlgItem(hDlg,IDC_EDITNAME));
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{
			MY_ACCOUNT* ac = &AfxAccount; 
			HWND hName = GetDlgItem(hDlg,IDC_EDITNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_EDITPASSWORD);
			HWND hIP       = GetDlgItem(hDlg,IDC_IPADDRESS1);
			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			ac->Name = buf;
			::GetWindowText(hPassword,buf,99);
			ac->Password = buf;
			DWORD dwAddress;
			::SendMessage(hIP,IPM_GETADDRESS,0,(LPARAM)&dwAddress);
			ac->ip.seg1 = FIRST_IPADDRESS(dwAddress);
			ac->ip.seg2 = SECOND_IPADDRESS(dwAddress);
			ac->ip.seg3 = THIRD_IPADDRESS(dwAddress);
			ac->ip.seg4 = FOURTH_IPADDRESS(dwAddress);
			if(ac->Name.size()>0 && ac->Password.size()>0 ){
				::SetWindowText(hName,_T(""));
				::SetWindowText(hPassword,_T(""));
				EndDialog(hDlg, 1);
			}
			return TRUE;
		}else if(LOWORD(wParam) == IDCANCEL){
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
};




uint32 Crc32Bitmap(HBITMAP hbitmap){
	assert(hbitmap);
	CBitmapEx Image;
	Image.Load(hbitmap);

	uint32 crc = Crc32Bitmap(Image);
	return crc;
}
uint32 Crc32Bitmap(CBitmapEx& Image){
	Image.AutoThreshold();
	
	const int size = 512*512;
	BYTE buf[size];

	long ImageSize;	
	ImageSize = Image.Save(buf,size);

	if (ImageSize!=0)
	{
		uint32 crcValue =0;
		crcValue = crc32(crcValue,(LPBYTE)&buf,ImageSize);
		return crcValue;
	}
	return 0;
}

//注意：会改变第二图像
float CompareBmp(CBitmapEx& Image1, CBitmapEx& Image2){

	int w = Image1.GetWidth();
	int h = Image1.GetHeight();

	if(Image1.GetWidth() != Image2.GetWidth() || Image1.GetHeight() != Image2.GetHeight()){
		return 0.0f;
	}
	
	Image2.DrawCombined(0,0,w,h,Image1,0,0, CM_SRC_XOR_DST);
	
	long black=0, white=0;
	
	long crBlack = RGB(0,0,0);
	
	for (long i=0; i<h; i++)
	{
		for (long j=0; j<w; j++)
		{
			_PIXEL px = Image2.GetPixel(j, i);

			px &= 0x00ffffff;
            if (px  == crBlack)
            {
				++black;
            }else{
				++white;
			}
		}
	};
	
    if (white==0)
    {
		return 100.0f;
    }
	
	else if(black==0){
		return 0.0f;
	}
	
	float count = white+black;
	if (count == 0.0f)
	{
		return 0.0f;
	}
	
	float f = (1.0f-white/count)*100.0f;
	return f;		
}


int32  GetCardSuitNum(tstring& Suit){
	if(Suit == _T("H")) return 0; //红桃
	else if(Suit == _T("C")) return 100; //梅花
	else if(Suit == _T("D")) return 200; //方片
	else if(Suit == _T("S")) return 300; //黑桃
	else return -1;
}

int32 GetCardFaceNum(tstring& Face)
{
	if(Face.find(_T("2"))!= -1) return 2;
	else if(Face.find(_T("3"))!= -1) return 3;
	else if(Face.find(_T("4"))!= -1) return 4;
	else if(Face.find(_T("5"))!= -1) return 5;
	else if(Face.find(_T("6"))!= -1) return 6;
	else if(Face.find(_T("7"))!= -1) return 7;
	else if(Face.find(_T("8"))!= -1) return 8;
	else if(Face.find(_T("9"))!= -1) return 9;
	else if(Face.find(_T("10"))!= -1)return 10;
	else if(Face.find(_T("J"))!= -1) return 11;
	else if(Face.find(_T("Q"))!= -1) return 12;
	else if(Face.find(_T("K"))!= -1) return 13;
	else if(Face.find(_T("A"))!= -1) return 14;
	else if(Face.find(_T("0"))!= -1) return 12; //Q有时被识别为0
	else if(Face.find(_T("70"))!= -1) return 10; //10有时被识别70 
	else  return 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame(CTimeStamp* Timer,CAbstractSpacePool* Pool)
:CUserObject(Timer,Pool),
m_WebHost(this),
m_Splitter(this,4,20,TRUE),
m_SplitterView(this,FALSE),
m_SpaceSelected(NULL),
m_CaptureItemID(NULL),
m_OcrView(this),

/////
m_Option(&m_Mutex),
m_bPause(FALSE),
m_StartTotal(0),
m_CurTotal(0),
m_RoomNum(0),
m_SeatPos(-1),
m_CardType(-1),
m_MinBet(0),
m_HandRoll(5000),
m_Scene(NULL),
m_bDebug(FALSE),
m_bSaveDebugToLog(TRUE),
m_bTaojingSai(FALSE),
m_bLingFen(FALSE),
m_Font(NULL)
{
	m_PlayNum = 0;
	m_CurObjectView.m_Alias = 10;
	m_ResultView.m_Alias    = 20;
    m_OcrView.m_Alias       = 30;
	m_SplitterView.m_Alias  = 40;
	
	m_bSaveOcrImag = false;

	m_bCreated = FALSE;

	PushChild(&m_Splitter);	
	m_State &= ~SPACE_SHOWWINBORDER;

	m_Alias = CreateTimeStamp();   

	m_Tesseract = new CTesseract;

	
	m_BasePos.x = 0;
	m_BasePos.y = 0;

	TCHAR lpBuffer2[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,lpBuffer2);

	tstring dir = lpBuffer2;
	m_TessDataDir = eSTRING::UTF16TO8(dir);
	m_TessDataDir +="\\";

//	tstring filename = dir + _T("\\tessdata\\masknum.bmp");
//	m_MaskNumImage.Load((LPTSTR)filename.c_str());

//	filename = dir + _T("\\tessdata\\maskcard.bmp");
//	m_MaskPokerImage.Load((LPTSTR)filename.c_str()); 


	m_TempDir = tformat(_T("%s\\temp\\"),lpBuffer2);

	_tmkdir(m_TempDir.c_str());

	tstring LogFileName = m_TempDir+tformat(_T("Poker.log"));
	m_Log.Open(LogFileName.c_str());

	//bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"poker");
	//assert(m_Tesseract->IsValid());

	SetTextFont(16,TRUE,FALSE,_T("Arial"));
}
CMainFrame::CMainFrame()
:	m_WebHost(this),
	m_Splitter(this,4,20,TRUE),
	m_SplitterView(this,FALSE),
	m_SpaceSelected(NULL),
	m_CaptureItemID(NULL),
	m_OcrView(this),

	/////
	m_Option(&m_Mutex),
	m_bPause(FALSE),
	m_StartTotal(0),
	m_CurTotal(0),
	m_RoomNum(0),
	m_SeatPos(-1),
	m_CardType(-1),
	m_MinBet(0),
	m_HandRoll(5000),
	m_Scene(NULL),
	m_bDebug(FALSE),
	m_bSaveDebugToLog(TRUE),
	m_bTaojingSai(FALSE),
	m_bLingFen(FALSE),
	m_Font(NULL)
{
	m_PlayNum = 0;
	m_CurObjectView.m_Alias = 10;
	m_ResultView.m_Alias    = 20;
	m_OcrView.m_Alias       = 30;
	m_SplitterView.m_Alias  = 40;

	m_bSaveOcrImag = false;

	m_bCreated = FALSE;
	
	PushChild(&m_Splitter);	
	m_State &= ~SPACE_SHOWWINBORDER;

	m_Alias = CreateTimeStamp();   

	m_Tesseract = new CTesseract;

	//bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"poker");
	//assert(m_Tesseract->IsValid());

	m_BasePos.x = 0;
	m_BasePos.y = 0;

	TCHAR lpBuffer2[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,lpBuffer2);

	tstring dir = lpBuffer2;
	m_TessDataDir = eSTRING::UTF16TO8(dir);
	m_TessDataDir +="\\";

	//tstring filename = dir + _T("\\tessdata\\masknum.bmp");
	//m_MaskNumImage.Load((LPTSTR)filename.c_str());

	//filename = dir + _T("\\tessdata\\maskcard.bmp");
	//m_MaskPokerImage.Load((LPTSTR)filename.c_str()); 


	//m_TessDataDir = eSTRING::UTF16TO8(dir);

	m_TempDir = tformat(_T("%s\\temp\\"),lpBuffer2);

	_tmkdir(m_TempDir.c_str());

	tstring LogFileName = m_TempDir+tformat(_T("Poker.log"));
	m_Log.Open(LogFileName.c_str());

	SetTextFont(16,TRUE,FALSE,_T("Arial"));
}

CMainFrame::~CMainFrame()
{
	if(m_Tesseract){
		delete m_Tesseract;
	}
	if(m_Scene){
		::DeleteObject(m_Scene);
	}

	m_ChildList.clear(); //avoid delete m_Splitter

	if (m_OcrNumTempFileName.size())
	{
		::DeleteFile(m_OcrNumTempFileName.c_str());
	}
	if (m_OcrTxtTempFileName.size())
	{
		::DeleteFile(m_OcrTxtTempFileName.c_str());
	}
	if (m_Font)
	{
		::DeleteObject(m_Font);
	}
};


void CMainFrame::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName )
{
	LOGFONT	LogFont;
	LogFont.lfHeight			= -MulDiv( nHeight, GetDeviceCaps( GetDC(m_hWnd), LOGPIXELSY ), 72 );
	LogFont.lfWidth				= 0;
	LogFont.lfEscapement		= 0;
	LogFont.lfOrientation		= 0;
	LogFont.lfWeight			= ( bBold )? FW_BOLD:FW_DONTCARE;
	LogFont.lfItalic			= (BYTE)( ( bItalic )? TRUE:FALSE );
	LogFont.lfUnderline			= FALSE;
	LogFont.lfStrikeOut			= FALSE;
	LogFont.lfCharSet			= DEFAULT_CHARSET;
	LogFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality			= DEFAULT_QUALITY;
	LogFont.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;

	_tccpy( LogFont.lfFaceName, csFaceName );

	if( m_Font!= NULL )
		::DeleteObject(m_Font);

	m_Font = ::CreateFontIndirect( &LogFont );
}

void CMainFrame::FlushWindow(){
	FLASHWINFO fwi;
	fwi.cbSize    = sizeof(fwi);
	fwi.hwnd      = m_hWnd;
	fwi.dwFlags   = FLASHW_ALL | FLASHW_TIMER;
	fwi.uCount    = 5;
	fwi.dwTimeout = 500;
	::FlashWindowEx(&fwi);
}


void CMainFrame::Layout(bool Redraw /*=true*/){
	CWSFrameView::Layout(FALSE);		

	if (m_bCreated)
	{
		RECT rcClient = m_rcClient;	

#ifdef _WINDLL
		::MoveWindow(m_AddressBar.GetHwnd(),0,0,0,0,FALSE);
		rcClient.bottom-=24;

		m_Splitter.Layout(rcClient);
#else

		if(!IsAlive()){			

			rcClient.bottom = rcClient.top+28;
			::MoveWindow(m_AddressBar.GetHwnd(),rcClient.left,rcClient.top,RectWidth(rcClient),28,TRUE);

			rcClient = m_rcClient;	
			rcClient.top +=28;
			rcClient.bottom-=24;
		}else{
			UpdateScene();
			::MoveWindow(m_AddressBar.GetHwnd(),0,0,0,0,FALSE);
			rcClient.bottom-=24;
		}

		if (m_CaptureItemID)
		{
			m_WebHost.EndSelectObject();

		}
		m_Splitter.Layout(rcClient);		
			
#endif
		rcClient.top = rcClient.bottom;
		rcClient.bottom = m_rcClient.bottom;
		::MoveWindow(m_Status.GetHwnd(),rcClient.left,rcClient.top,RectWidth(rcClient),24,TRUE);
 	}
}


/*tools
批量修改数据库里的数据
*/

/*
void CMainFrame::CalibrateCard(){

	POINT GamePos;
	//m_WebHost.m_WebView.GetGamePos(GamePos);
	


	ePipeline ObjectList;
	m_PokerDB.GetObjectList(_T("PokerRobot.ini"),ObjectList);

	int basex=0,basey=0;
	for (int i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ObjectData = (ePipeline*)ObjectList.GetData(i);

		if (ObjectData->Size()<5)
		{
			continue;
		}
		int32& x = *(int32*)ObjectData->GetData(3);
		int32& y = *(int32*)ObjectData->GetData(4);

		//得到全局位置
		if(i==0){
            basex = x;
			basey = y;
		}else{
			x += basex;
			y += basey;
		}
        x -= GamePos.x;
		y -= GamePos.y;

		m_PokerDB.SaveObject(*ObjectData);
	}
}
*/



LRESULT CMainFrame::OnParentWebCompleted(SpaceRectionMsg* SRM){
	
	if (m_WebHost.GetHandle()==NULL)
	{
		BOOL ret = m_WebHost.SubclassWebWnd();
		assert(ret);
	}

	m_IsRuning = false;

	m_AddressBar.SetConnectState(FALSE);
	if (m_UserAccount.Size())
	{
		if (m_UserAccount.GetID()==1) //自动登录
		{
			tstring UserName = m_UserAccount.PopString();
			tstring Password = m_UserAccount.PopString();
			m_WebHost.AuotFillForm(UserName,Password);
			Sleep(500);
			m_WebHost.AuotFillForm(UserName,Password);

			SetTitle(UserName.c_str());

			if (m_UserAccount.Size())
			{
				m_UserAccount.SetID(2);
			}
		}else if (m_UserAccount.GetID()==2) //进入指定房间
		{

		}
	}
	return 0;
}
LRESULT CMainFrame::OnParentMouseMove(SpaceRectionMsg* SRM){

	int32 wParam = SRM->wParam;
	int32 lParam = SRM->lParam;

	POINT point;
	point.x = LOWORD(lParam);  //窗口坐标
	point.y = HIWORD(lParam);

	POINT ScrollPos;
	m_WebHost.GetWebScrollPos(ScrollPos);			

	POINT FramePos;
	m_WebHost.GetFramePos(FramePos);

	//转化成基于游戏Frame
	point.x -=FramePos.x;
	point.y -=FramePos.y;

	//再转化成基于BasePos
	point.x -= m_BasePos.x;
	point.y -= m_BasePos.y;

	tstring s = tformat(_T("X:%d Y:%d  FramePos:%d,%d  ScrollPos:%d,%d"),point.x,point.y,FramePos.x,FramePos.y,ScrollPos.x,ScrollPos.y);
	m_Status.SetTip(s);

	return 0;
}
LRESULT CMainFrame::OnParentCaptrueObject(SpaceRectionMsg* SRM){
	if (SRM->lParam)
	{
		m_CaptureItemID = SRM->wParam;
		m_WebHost.StartSelectObject();
		
	}else{
		m_CaptureItemID = 0;
		m_WebHost.EndSelectObject();
	}
	return 0;
};

LRESULT CMainFrame::OnParentObjectSelected(SpaceRectionMsg* SRM){
	CObjectData ObjectData;
	BOOL ret = m_WebHost.GetSelectedObject(m_CaptureItemID,ObjectData,FALSE);
	if (!ret)
	{
		m_Status.SetTip(_T("CaptureIamge Fail"));
		return 0;
	}
	
		
	int32& x = *(int32*)ObjectData.GetData(3); 	
	int32& y = *(int32*)ObjectData.GetData(4); 


	//转化成基于游戏Frame的坐标，注意，FramePos是也基于当前窗口，直接相减即可
	POINT FramePos;
	m_WebHost.GetFramePos(FramePos);
	x -= FramePos.x;
	y -= FramePos.y;


	ePipeline OldObject;

	m_CurObjectView.OnFindObject(m_CaptureItemID,(int64)&OldObject);


	tstring Name = *(tstring*)OldObject.GetData(1);
	int32 Type =  *(int32*)OldObject.GetData(2);
	*(int32*)ObjectData.GetData(2) = Type;
	*(tstring*)ObjectData.GetData(1) = Name;


	if (Name == _T("坐标基准"))
	{		
		m_BaseImage.Create(0,0);//使原图像无效
		UpdateObjectImage(ObjectData,true);    
	}else{
		UpdateScene();
		
		
	    //验证一下
		int32      w = *(int32*)ObjectData.GetData(5);
		int32      h = *(int32*)ObjectData.GetData(6);

		
		HBITMAP b1 = CaptureImageFromBmp(GetHwnd(),m_Scene,x,y,w,h); 
		CBitmapEx Image1;
		Image1.Load(b1);

		HBITMAP b2 = ObjectData.GetImage();
		CBitmapEx Image2;
		Image2.Load(b2);

		float f = CompareBmp(Image1,Image2); //应该 f=100.0
		
		assert(f==100.0f);

		assert(m_Scene);
		BOOL ret = CalculateBaseImagePos(m_Scene);
		assert(ret);

		x-= m_BasePos.x; 
		y-= m_BasePos.y;
		UpdateObjectImage(ObjectData,false);    
	}

	return 0;
};

LRESULT CMainFrame::OnParentOcrObject(SpaceRectionMsg* SRM){
	int64  ObjectID = SRM->wParam;
	
	ePipeline ObjectData;
	if(m_CurObjectView.GetHwnd()){
		SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_FIND,ObjectID,(int64)&ObjectData);
	}else{
		m_CurObjectView.OnFindObject(ObjectID,(int64)&ObjectData);
	}

	tstring ocrtxt = OCR(&ObjectData,TRUE);

	if (ocrtxt.size())
	{
		if(m_CurObjectView.GetHwnd()){
			SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_OCR_TXT,ObjectID,(int64)&ocrtxt);
		}else{
			m_CurObjectView.OnSetObjectOcrTxt(ObjectID,(int64)&ocrtxt);
		}
		//m_Status.SetTip(ocrtxt);
	}
	return 0;
}
LRESULT CMainFrame::OnParentConnect(SpaceRectionMsg* SRM)
{
	//tstring url = m_AddressBar.m_AddressEdit.GetEditText(false);
	TCHAR buf[255];
	::SendMessage(m_AddressBar.m_AddressEdit,WM_GETTEXT,255,(LPARAM)buf);
	tstring url = buf;		
	if (url.size()==0)
	{
		m_AddressBar.SetConnectState(FALSE);
		return 0;
	}
	m_WebHost.m_WebView.m_WebBrowser.DisplayHTMLPage(m_WebHost.m_WebView.GetHwnd(),url.c_str());
	return 0;
}

LRESULT CMainFrame::ParentReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg){
	case CM_WEB_COMPLETED:
		return OnParentWebCompleted(SRM);
	case CM_WEB_BEGIN:
		m_WebHost.Close();
		break;
	case CM_MOUSE_MOVE:
		return OnParentMouseMove(SRM);
	case CM_SET_CUR_OBJECT:
		m_CaptureItemID = SRM->wParam;
        break;
	case CM_CAPTURE_OBJECT:
		return OnParentCaptrueObject(SRM);
	case CM_OBJECT_SELECTED:
		return OnParentObjectSelected(SRM);
	case CM_OCR_OBJECT:
		return OnParentOcrObject(SRM);
	case BNT_TASK_RUN:
		{
			//m_WebHost.ScreenShot(TRUE);
			if(IsPause()){
				m_bPause = FALSE;
				m_Status.SetBntState(BNT_TASK_PAUSE,0);
				m_Status.SetBntState(BNT_TASK_RUN,SPACE_DISABLE);
			}else{

#ifndef _WINDLL
#ifdef _DEBUG
			  m_Splitter.ReplaceView(m_CurObjectView.m_Alias,&m_SplitterView);	
#endif
			 //m_Splitter.AddView(&m_OcrView);
				m_OptionView.m_OptionView.ToPipe((ePipeline&)m_Option);
				Activation();
#endif
			}
		}
		break;
	case BNT_TASK_PAUSE:
		CmdPauseGame();
        break;
	case BNT_TASK_STOP:
		//	assert(m_TexasPoker.IsAlive());		
		CmdStopGame();
		break;
	case CM_CONNECT:
		return OnParentConnect(SRM);
	case CM_DELETE_OBJECT:
		{
			tstring* Name = (tstring*)SRM->wParam;
			m_PokerDB.DeleteObject(*Name);
		}
		break;
	case CM_OCRTEXT_SET:
		{
			COcrItem* Item = (COcrItem*)SRM->ChildAffected;
			
			RegisterOcredObject(Item->m_Crc32,Item->m_OcrText);
			m_PokerDB.SaveCrc32(Item->m_Crc32,Item->m_OcrText,Item->m_hBMP); //保存在数据库里	
		}
		break;
	case BNT_TASK_BISAI:
		//m_TexasPoker.SwitchBisai();
        break;
	case BNT_TASK_DEBUG:
		//m_TexasPoker.SwitchDebug();
		break;
	case BNT_TASK_SHUPENG:
		//m_TexasPoker.SwitchShuaPeng();
		break;
	case BNT_TASK_JIAOZHENG:
		CaptureCardPosSign();
		break;

	case BNT_TASK_OPTION:
		{
			if (m_OptionView.GetHwnd())
			{
				m_OptionView.Destroy();
				m_Status.CloseOptionView();
			}else{
				RECT rc;
				rc.left = 100;
				rc.top  = 100;
				rc.right = 500;
				rc.bottom = 600;
				BOOL ret = m_OptionView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN|WS_EX_TOOLWINDOW,rc,GetHwnd());		
			}

		}
		break;
	case NEW_OPTION_RESULT:
	{
		ePipeline* OptionPipe = (ePipeline*)SRM->wParam;
		m_Option.FromPipe(*OptionPipe);
		m_Status.SetTip(_T("得到新的选项"));

	}
	case CM_WEB_NEWWINDOW:
		{
			tstring* url = (tstring*)SRM->wParam;
			m_AddressBar.SetConnectState(TRUE);
			m_WebHost.m_WebView.m_WebBrowser.DisplayHTMLPage(GetHwnd(),url->c_str());
			m_AddressBar.SetText(*url);
			//m_WebHost.Close();
		}
		break;
	default:
		break;
	}
	return 0;

}

LRESULT CMainFrame::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
//	case WM_ERASEBKGND:
//		return TRUE;
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_TIMER:
		return OnTimer(wParam,lParam);
	case WM_GETMINMAXINFO:
		return OnGetMinMaxInfo(wParam,lParam);
	case WM_CLOSE:
		//SaveObjectToDatabase();
		m_WebHost.Close();

		if (IsAlive())
		{
			CmdStopGame();
		}
	default:
		return CWSFrameView::Reaction(message,wParam,lParam);
	}
	return 0;
}

LRESULT CMainFrame::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rc={0};
	
	if(!m_AddressBar.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}


#ifdef _WINDLL
	m_Splitter.AddView(&m_ResultView);
	m_Splitter.AddView(&m_WebHost.m_WebView,false,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN);

#else

#ifdef _DEBUG
	if(!m_SplitterView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	m_SplitterView.AddView(&m_ResultView);
	m_SplitterView.AddView(&m_OcrView);


	m_Splitter.AddView(&m_CurObjectView);
	m_Splitter.AddView(&m_WebHost.m_WebView,false,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN);
#else 
	m_Splitter.AddView(&m_ResultView);
	m_Splitter.AddView(&m_WebHost.m_WebView,false,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN);
	
	//m_WebHost.SubclassWebWnd();
#endif
#endif

	if(!m_Status.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd(),20)){
		return -1;
	}
	m_bCreated = TRUE;

	::SetTimer(GetHwnd(),AUTO_LOGIN_TIMER,1000,NULL);
	//::SetTimer(GetHwnd(),AUTO_MOUSE_TIMER,30000,NULL);
	return 0;
}

LRESULT CMainFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
	CWSFrameView::OnLButtonDown(wParam,lParam);

    POINTS p = MAKEPOINTS(lParam);
	
	POINT point;
	point.x = p.x;
	point.y = p.y;	
	m_SpaceSelected = Hit(point);
	if(m_SpaceSelected){
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){	
			m_Splitter.OnLButtonDown((SplitterItem*)m_SpaceSelected);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{

	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER) {
			m_Splitter.OnLButtonUp((SplitterItem*)m_SpaceSelected);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
            m_SpaceSelected = NULL; 	
		}
	}
	return 0;
}

LRESULT CMainFrame::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{
	CWSFrameView::OnMouseMove(wParam,lParam);

	POINTS point = MAKEPOINTS(lParam);
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){
            m_Splitter.OnMouseMove(point.x,point.y,(SplitterItem*)m_SpaceSelected);
		}
	}		
	return 0;
}

LRESULT CMainFrame::OnTimer(WPARAM wParam, LPARAM lParam){
	if (wParam == AUTO_LOGIN_TIMER)
	{
		if (m_UserAccount.Size())
		{
			tstring URL = m_UserAccount.PopString();
			::SetWindowText(m_AddressBar.m_AddressEdit,URL.c_str());
			m_AddressBar.SetConnectState(TRUE);

			if(m_UserAccount.Size()){
				m_UserAccount.SetID(1); //指示网页加载完后自动登录
			}
			m_WebHost.m_WebView.m_WebBrowser.DisplayHTMLPage(GetHwnd(),URL.c_str());

		}
		::KillTimer(GetHwnd(),wParam);
	}
	else {
		return CWSFrameView::OnTimer(wParam,lParam);
	}
	return 0;
}

LRESULT CMainFrame::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
#ifdef LOCAL_USE	
	lpmmi->ptMinTrackSize.x = 320;
	lpmmi->ptMinTrackSize.y = 300;
#else 
	lpmmi->ptMinTrackSize.x = 100;
	lpmmi->ptMinTrackSize.y = 150;
#endif
	CWSFrameView::OnGetMinMaxInfo(wParam,lParam);
	return 0;
}


LRESULT CMainFrame::ChildReaction(SpaceRectionMsg* SRM){

	switch(SRM->Msg){
	case WEB_CLICK_OBJECT:
		return OnWebClickObject(SRM);
	case WEB_CLICK_POINT:
		return OnWebClickPoint(SRM);
	case WEB_GET_SCENE:
		return OnWebGetScene(SRM);
	case WEB_OPEN_URL:
		return OnWebOpenURL(SRM);
	case WEB_LOGIN:
		return OnWebLogin(SRM);
	case WEB_REFESH:
		return OnWebRefresh(SRM);
	default:
		break;
	}
	return 0;
}
LRESULT CMainFrame::OnWebRefresh(SpaceRectionMsg* SRM){
	m_AddressBar.SetConnectState(TRUE);
	IWebBrowser2* WebBrowser = m_WebHost.m_WebView.m_WebBrowser.GetWebBrowser();
	VARIANT state;
	state.vt = VT_I4;
	state.iVal = REFRESH_NORMAL;
	WebBrowser->Refresh2(&state);
	return 0;
}
LRESULT CMainFrame::OnWebClickObject(SpaceRectionMsg* SRM){
	ePipeline& ObjectData = *(ePipeline*)SRM->wParam;
	tstring*   Name = (tstring*)ObjectData.GetData(1);
	int32      x = *(int32*)ObjectData.GetData(3);
	int32      y = *(int32*)ObjectData.GetData(4);
	int32      w = *(int32*)ObjectData.GetData(5);
	int32      h = *(int32*)ObjectData.GetData(6);

	POINT p,p1,p2;


	x += m_BasePos.x;  //转换成基于FramePos的坐标
	y += m_BasePos.y;


	//转换成基于整个网页窗口的坐标
	POINT FramePos;
	m_WebHost.GetFramePos(FramePos);
	
	x += FramePos.x;  
	y += FramePos.y;


	p1.x = x;
	p1.y = y;

	p2.x = p1.x + w;
	p2.y = p1.y + h;

	//点击点坐标
	p.x = p1.x+w/2;
	p.y = p1.y+h/2;


	POINT ScrollPos;
	m_WebHost.GetWebScrollPos(ScrollPos);


	//	m_WebHost.Click(p);

	RECT rc;
	::GetClientRect(m_WebHost.m_WebView.GetHwnd(),&rc);

	int32 scroll_x = 0;
	int32 scroll_y = 0;


	//确保需要点击的点在当前窗口
	if(p.x<rc.left+30){
		int dx = rc.left+30 -p.x;
		scroll_x = ScrollPos.x-dx;
		//		p.x += dx;
	}
	if (p.y<rc.top+50)
	{
		int dy = rc.top+50 - p.y;
		scroll_y = ScrollPos.y-dy;
		//		p.y += dy;
	}
	if (p.x>rc.right-100)
	{
		int dx = p.x - rc.right +100; 
		scroll_x = ScrollPos.x+dx;
		//		p.x -= dx;
	}
	if (p.y > rc.bottom-100)
	{
		int dy = p.y - rc.bottom+100; //有时窗口会有状态条，遮住了点击点，因此多偏移一点
		scroll_y = ScrollPos.y+dy; 
		//		p.y -= dy;
	}

	if(scroll_x!=0 || scroll_y!=0){  //点击坐标超过当前窗口则移动窗口点击部位当当前窗口，点击完后恢复位置
	    POINT  OldScrollPos = ScrollPos;

		m_WebHost.ScrollTo(scroll_x,scroll_y);

		m_WebHost.GetWebScrollPos(ScrollPos);  

		p.x -= ScrollPos.x - OldScrollPos.x; //理论滚动和实际滚动距离可能不一致，所以必须以前后滚动条位置为准
		p.y -= ScrollPos.y - OldScrollPos.y;

	}
	
	LPARAM lparam = MAKELPARAM(p.x,p.y);

	HWND  hWnd = m_WebHost.GetHandle();
	if(hWnd==NULL){
		m_WebHost.SubclassWebWnd();
		 hWnd = m_WebHost.GetHandle();
	};

	::SendMessage(hWnd,WM_LBUTTONDOWN,MK_LBUTTON,lparam);
	::SendMessage(hWnd,WM_LBUTTONUP,MK_LBUTTON,lparam);
		
	return 0;
};

LRESULT CMainFrame::OnWebClickPoint(SpaceRectionMsg* SRM){
	ePipeline& PointData = *(ePipeline*)SRM->wParam;
	
	int32      x = PointData.PopInt();
	int32      y = PointData.PopInt();




	x += m_BasePos.x;  //转换成基于FramePos的坐标
	y += m_BasePos.y;


	//转换成基于整个网页窗口的坐标
	POINT FramePos;
	m_WebHost.GetFramePos(FramePos);

	x += FramePos.x;  
	y += FramePos.y;

	POINT p;

	p.x = x;
	p.y = y;


	POINT ScrollPos;
	m_WebHost.GetWebScrollPos(ScrollPos);

	RECT rc;
	::GetClientRect(m_WebHost.m_WebView.GetHwnd(),&rc);

	int32 scroll_x = 0;
	int32 scroll_y = 0;


	//确保需要点击的点在当前窗口
	if(p.x<rc.left+30){
		int dx = rc.left+30 -p.x;
		scroll_x = ScrollPos.x-dx;
		//		p.x += dx;
	}
	if (p.y<rc.top+50)
	{
		int dy = rc.top+50 - p.y;
		scroll_y = ScrollPos.y-dy;
		//		p.y += dy;
	}
	if (p.x>rc.right-100)
	{
		int dx = p.x - rc.right +100; 
		scroll_x = ScrollPos.x+dx;
		//		p.x -= dx;
	}
	if (p.y > rc.bottom-100)
	{
		int dy = p.y - rc.bottom+100; //有时窗口会有状态条，遮住了点击点，因此多偏移一点
		scroll_y = ScrollPos.y+dy; 
		//		p.y -= dy;
	}

	if(scroll_x!=0 || scroll_y!=0){  //点击坐标超过当前窗口则移动窗口点击部位当当前窗口，点击完后恢复位置
		POINT  OldScrollPos = ScrollPos;

		m_WebHost.ScrollTo(scroll_x,scroll_y);

		m_WebHost.GetWebScrollPos(ScrollPos);  

		p.x -= ScrollPos.x - OldScrollPos.x; //理论滚动和实际滚动距离可能不一致，所以必须以前后滚动条位置为准
		p.y -= ScrollPos.y - OldScrollPos.y;

	}

	LPARAM lparam = MAKELPARAM(p.x,p.y);

	HWND  hWnd = m_WebHost.GetHandle();
	if(hWnd==NULL){
		m_WebHost.SubclassWebWnd();
		hWnd = m_WebHost.GetHandle();
	};

	::SendMessage(hWnd,WM_LBUTTONDOWN,MK_LBUTTON,lparam);
	::SendMessage(hWnd,WM_LBUTTONUP,MK_LBUTTON,lparam);

	return 0;
}
LRESULT CMainFrame::OnWebGetScene(SpaceRectionMsg* SRM){
	if (m_Scene)
	{
		::DeleteObject(m_Scene);
	}
	HBITMAP hBitmap = m_WebHost.CaptureFrameFromWeb(950,800);
	m_Scene = hBitmap;
	return 0;
};

LRESULT CMainFrame::OnWebLogin(SpaceRectionMsg* SRM){
	ePipeline* Letter = (ePipeline*)SRM->wParam;
	tstring UserName = Letter->PopString();
	tstring Password = Letter->PopString();	
	m_WebHost.AuotFillForm(UserName,Password);
	Sleep(500);
	m_WebHost.AuotFillForm(UserName,Password);
	SetTitle(UserName.c_str());
	return 0;
};

LRESULT CMainFrame::OnWebOpenURL(SpaceRectionMsg* SRM){
	ePipeline* Letter = (ePipeline*)SRM->wParam;
	tstring URL = Letter->PopString();
	m_AddressBar.SetConnectState(TRUE);

	InternetSetOption(0, INTERNET_OPTION_END_BROWSER_SESSION, 0, 0);
	tstring NewCookie;
	InternetSetCookie(URL.c_str(),NULL,NewCookie.c_str());
	m_PlayNum = 0;

	m_WebHost.m_WebView.m_WebBrowser.DisplayHTMLPage(GetHwnd(),URL.c_str());
	return 0;
}

//公共接口
//////////////////////////////////////////////////////////////////////////

bool CMainFrame::WaitForCompleted(){
	int i=20;

	while(m_IsRuning && i-->0)
	{
		Sleep(500);		
	}

	return !m_IsRuning;
}
BOOL CMainFrame::CmdProc(tstring Cmd,ePipeline& Param){
	CLock lk(&m_DoLock);

	if (!InitPokerGame())
	{
		Param.PrintLabel(_T("init error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	m_Option.GetValue(PLAY_DEBUG,m_bDebug);
	m_Option.GetValue(PLAY_LINGFEN,m_bLingFen);

	if (Cmd == _T("玩一局"))
	{
		tstring Output;
		CmdRunOnce(Output);
		Param.PushString(Output);
		return TRUE;
	}else if (Cmd == _T("刷新"))
	{
		return CmdRefresh(Param);
	}
	else if (Cmd == _T("输出提示"))
	{
		return CmdOutputTip(Param);
	} 
	else if(Cmd == _T("打开网址"))
	{
		return CmdOpenGameWeb(Param);
	}else if(Cmd == _T("点击标签"))
	{
		return CmdClickObject(Param);
	}else if (Cmd == _T("双击标签"))
	{
		return CmdDBClickObject(Param);
	}else if (Cmd == _T("点击位置"))
	{
		return CmdClickPoint(Param);
	}
	else if(Cmd == _T("登录"))
	{
		return CmdLogin(Param);
	}
	else if(Cmd == _T("寻找标签"))
	{
		return CmdHasLabel(Param);
	}else if(Cmd == _T("输入字符串"))
	{
		return CmdInputText(Param);
	}else if (Cmd == _T("得到房间号"))
	{
		return CmdGetRoomID(Param);
	}else if (Cmd == _T("是否有座位"))
	{
		return CmdHasSeat(Param);
	}
	else if(Cmd == _T("得到总分"))
	{
		return CmdGetTotalRoll(Param);
	}else if(Cmd == _T("得到座位"))
	{
		return CmdGetMySiteID(Param);
	}else if(Cmd == _T("得到盲注"))
	{
		return CmdGetMinRoll(Param);
	}else if(Cmd == _T("得到底牌"))
	{
		return CmdGetHandCard(Param);
	}else if (Cmd == _T("得到公牌"))
	{
		return CmdGetCommonCard(Param);
	}else if (Cmd == _T("得到跟注"))
	{
		return CmdGetBetRoll(Param);
	}else if (Cmd == _T("暂停"))
	{
		return CmdPauseGame();
	}else if (Cmd == _T("停止"))
	{
		return CmdStopGame();
	}else if (Cmd == _T("运行"))
	{
		return CmdRunGame();
	}else if (Cmd == _T("清除历史"))
	{
		return CmdClearCookie(Param);
	}
	return TRUE;
}


BOOL CMainFrame::CmdOutputTip(ePipeline& Param){
	
	if (!Param.HasTypeAB(0x30000000))
	{
		Param.PrintLabel(_T("output tip() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}

	tstring Tip = Param.PopString();

	if (m_bDebug || m_bSaveDebugToLog)
	{
		AnsiString s = eSTRING::UTF16TO8(Tip);
		m_Log.PrintLn(s);
	}
	m_ResultView.AddInfo(&Param);
	return TRUE;
};

BOOL CMainFrame::CmdRefresh(ePipeline& Param){
	m_IsRuning = true;
	SendChildMessage(GetHwnd(),WEB_REFESH,(int64)&Param,0);

	WaitForCompleted();
	
	return  TRUE;
}


BOOL CMainFrame::CmdOpenGameWeb(ePipeline& Param){
	if (!Param.HasTypeAB(0x30000000))
	{
		Param.PrintLabel(_T("Open web() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	m_StartTotal = 0; //重置 
	m_RoomNum = 0;
	m_SeatPos = -1;
	m_MinBet = 0;
	m_HandRoll = 5000;
	
	m_IsRuning = true;
	ePipeline* Letter = &Param;

	SendChildMessage(GetHwnd(),WEB_OPEN_URL,(int64)Letter,0);

	WaitForCompleted();
	return  TRUE;
};

BOOL CMainFrame::CmdClickObject(ePipeline& Param){

	if (!Param.HasTypeAB(0x30000000))
	{
		Param.PrintLabel(_T("Click Object() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	UpdateScene();
	tstring Label = Param.PopString();
	CObjectData* Obj = FindObject(Label);
	if (Obj)
	{
		ClickBnt(Obj);
	}
	return TRUE;
};
BOOL CMainFrame::CmdDBClickObject(ePipeline& Param)
{
	if (!Param.HasTypeAB(0x30000000))
	{
		Param.PrintLabel(_T("Double Click Object() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	UpdateScene();
	tstring Label = Param.PopString();
	CObjectData* Obj = FindObject(Label);
	if (Obj)
	{
		ClickBnt(Obj);
		ClickBnt(Obj);
	}
	return TRUE;
}
BOOL CMainFrame::CmdLogin(ePipeline& Param){
	ePipeline* Letter = &Param;
	
	if (!Param.HasTypeAB(0x33000000))
	{
		Param.PrintLabel(_T("Login param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}

	//tstring UserName = Letter->PopString();
	//tstring Password = Letter->PopString();
	m_IsRuning = true;
	SendChildMessage(GetHwnd(),WEB_LOGIN,(int64)Letter,0);

	//WaitForCompleted();

	//m_WebHost.AuotFillForm(UserName,Password);

	return TRUE;
};

BOOL CMainFrame::CmdHasLabel(ePipeline& Param){
	if (!Param.HasTypeAB(0x32000000))
	{
		Param.PrintLabel(_T("Has label() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}

	UpdateScene();

	tstring ObjectName = Param.PopString();
	float32 f =  Param.PopFloat();
	BOOL ret = HasLabel(ObjectName,f);
	Param.PushInt(ret);
	return true;
};

BOOL CMainFrame::CmdInputText(ePipeline& Param){
	if (!Param.HasTypeAB(0x30000000))
	{
		Param.PrintLabel(_T("Input text() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	UpdateScene();
	tstring text = Param.PopString();
	WebInputText(text);
	return TRUE;
};
BOOL CMainFrame::CmdGetRoomID(ePipeline& Param){
	UpdateScene();
	tstring  RoomID  = CheckRoomID();

	Param.PushString(RoomID);
	return TRUE;
}
BOOL CMainFrame::CmdHasSeat(ePipeline& Param)
{
	UpdateScene();
	int n  = CheckBlankSeat();
	if(n>0)n=1;
	Param.PushInt(n);
	return TRUE;
}
BOOL CMainFrame::CmdGetTotalRoll(ePipeline& Param){
	UpdateScene();
	int32 n  = CheckTotal();
	Param.PushInt(n);
	return TRUE;
};

BOOL CMainFrame::CmdGetMySiteID(ePipeline& Param){
	UpdateScene();
	int32 n = CheckMySitPos();
	Param.PushInt(n);
	return TRUE;
};

BOOL CMainFrame::CmdGetMinRoll(ePipeline& Param){
	int32 n = CheckMinRoll();
	Param.PushInt(n);
	return TRUE;
};

BOOL CMainFrame::CmdGetHandCard(ePipeline& Param){
	if (!Param.HasTypeAB(0x00000000))
	{
		Param.PrintLabel(_T("Get Hand card() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	UpdateScene();
	vector<int32> MyCard;

	BOOL ret = GetSelfCard(MyCard); 
	for(int32 i=0; i<MyCard.size(); i++){
		Param.PushInt(MyCard[i]);
	}
	return ret;
};

BOOL CMainFrame::CmdGetCommonCard(ePipeline& Param){
	UpdateScene();
	vector<int32> OtherCard;
	BOOL ret = CheckOtherCard(OtherCard); 
	for(int32 i=0; i<OtherCard.size(); i++){
		Param.PushInt(OtherCard[i]);
	}
	return ret;
};

BOOL CMainFrame::CmdGetBetRoll(ePipeline& Param){
	UpdateScene();
	int32 n =  CheckBetRoll();
	Param.PushInt(n);
	return TRUE;
};

BOOL CMainFrame::CmdPauseGame(){
	m_Mutex.Acquire();
	m_bPause = TRUE;
	m_Mutex.Release();
	tstring tip = _T("暂停自动玩牌");
	NotifyTip(tip);
	
	m_Status.SetBntState(BNT_TASK_PAUSE,SPACE_DISABLE);
	m_Status.SetBntState(BNT_TASK_RUN,0);

	return TRUE;
};

BOOL CMainFrame::CmdStopGame(){
	if (!m_Alive)
	{
		return TRUE;
	}
	m_Alive = FALSE;
	m_bPause = FALSE;

#ifdef _DEBUG
	m_Splitter.ReplaceView(m_SplitterView.m_Alias,&m_CurObjectView);
//	::SetWindowPos(GetHwnd(),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	Layout();
#endif
	tstring tip = _T("停止自动玩牌");
	NotifyTip(tip);
	return TRUE;
};

BOOL CMainFrame::CmdRunGame(){
	m_Option.GetValue(PLAY_DEBUG,m_bDebug);
    
	if (!InitPokerGame())
	{
		
		CmdStopGame();
		return false;
	}
	
	m_Alive = true;
	m_CardType = -1;

//	UpdateScene();
//	CheckBlankSeat();

//	TestZoomClick();
//	TestOtherCardPos();


	for (int i=0; i<5 && m_Alive; i++) //重复5次进入游戏大厅
	{		
		tstring tip = tformat(_T("尝试进入大厅...第%d次"),i);
		NotifyTip(tip);

		UpdateScene();

		if (HasLabel(_T("大厅")))
		{
			NotifyTip(_T("当前在大厅，尝试自动进入房间"));
			if (!SelectPlayType(_T("游戏场")))
			{
				Sleep(2000);
				continue;
			}

			if (!Kuaisuxunzhaozuowei())
			{
				Sleep(2000);
				continue;
			}
					
		};

		if(!HasLabel(_T("牌场"),90.0f))
		{
			NotifyTip(_T("未能检测到房间"));
			Sleep(2000);
			continue;
		}


		//NotifyTip(_T("游戏启动"));


		CBitmapEx qipai,kuaisukaishi;

		CObjectData& Ob1 = m_PokerObjectList[_T("弃牌")];
		assert(Ob1.Size()>9);
		qipai.Load(Ob1.GetImage());

		CObjectData& Ob3 = m_PokerObjectList[_T("快速开始")];
		assert(Ob3.Size()>9);
		kuaisukaishi.Load(Ob3.GetImage());
		
		CObjectData& Ob4 = m_PokerObjectList[_T("关闭分享弹出窗口")];
		assert(Ob4.Size()>9);
	
		CObjectData QiPaiObject;	
		
		while(m_Alive)
		{	            
			NotifyTip(_T("等待新一轮..."));
			do{
								
				UpdateScene();
				if (
					!HasLabel(_T("弃牌"),QiPaiObject,93.0f,&qipai)
					//&&!WaitObject(qipai1,_T("弃牌(黄)"),QiPaiObject)
					)
				{
					CObjectData ZhiDongObject;
					if (HasLabel(_T("快速开始"),ZhiDongObject,93.0f,&kuaisukaishi)) 
					{
						ClickBnt(&ZhiDongObject);  //自动选座位
						
						Sleep(3000);
						CObjectData CloseBnt;	
						if(CaptureObject(_T("关闭买分弹出窗口"),CloseBnt)){
							ClickBnt(&CloseBnt);  
						}						
					}else{
						ClickBnt(&Ob4); //击关闭可能出现的分享窗口
						Sleep(200);
						LingFen();
						Sleep(300);
					}
				}else{
					NotifyTip(_T("开始玩牌"));	
					Sleep(1200); //等待1.5秒翻牌
					break;
				}
				
				BOOL bPause = IsPause();
				while(m_Alive && IsPause()){
					Sleep(20);
				}
				
			}while(m_Alive /*(&& CutTime<EndTime*/);

			if(!m_Alive ){
				break;
			}

			assert(QiPaiObject.Size());
			
			BOOL ret = PlayGame();
			if (!ret)
			{

				ClickBnt(&QiPaiObject);
				
				tstring Action = _T("操作: 弃牌");
				NotifyTip(Action);	

			
				m_OldCardList.clear();

				if (m_CurPlayType ==_T("2 刷局"))
				{
					Sleep(300); //等待弃牌按钮变化
				}else{
				    //得到手上筹码
					int32 MyHandRoll = CheckMyRoll(m_SeatPos);

					//如果赢的分大于一定数目，则站起来保存，之后会重新坐下
					if(!m_bBisai &&  MyHandRoll>m_MinBet*400){
						CObjectData* BntObject = FindObject(_T("站起"));	
						if(BntObject){
							ClickBnt(BntObject);
							NotifyTip(_T("已存分"));	
						}
					}
					Sleep(1000);
				}
				
			}


			BOOL bPause = IsPause();
			while(m_Alive && IsPause()){
				Sleep(20);
			}
	

		}//while
	}
	

	CmdStopGame();
	return true;
}

BOOL CMainFrame::CmdRunOnce(tstring& Output){


	tstring PlayActive;
	m_Option.GetValue(PLAY_FACTOR,PlayActive);
	PLAY_FACTOR_0 pf = PF_NORMAL;
	if (PlayActive == _T("2 保守"))
	{
		pf = PF_PASSIVE;
	}else if (PlayActive == _T("3 积极"))
	{
		pf = PF_ACTIVE;
		
	}
	
	UpdateScene();
/*
	if(!HasLabel(_T("牌场"),90.0f))
	{
		
		Output = _T("未能检测到房间");
		NotifyTip(Output);
		return TRUE;
	}
*/	
	CBitmapEx qipai,kuaisukaishi;
	
	CObjectData& Ob1 = m_PokerObjectList[_T("弃牌")];
	assert(Ob1.Size()>9);
	qipai.Load(Ob1.GetImage());
	
	CObjectData& Ob3 = m_PokerObjectList[_T("快速开始")];
	assert(Ob3.Size()>9);
	kuaisukaishi.Load(Ob3.GetImage());
	

	CObjectData& Ob4 = m_PokerObjectList[_T("关闭分享弹出窗口")];
	assert(Ob4.Size()>9);

	CObjectData QiPaiObject;
	
	bool bPlayOnceEnd = false;
	
	//先点击关闭可能出现的分享窗口
	ClickBnt(&Ob4);

	m_bPause = false;
	m_Status.SetBntState(BNT_TASK_PAUSE,0);
	m_Status.SetBntState(BNT_TASK_RUN,SPACE_DISABLE);

	while(m_Alive && !bPlayOnceEnd)
	{	            
		NotifyTip(_T("等待新一轮..."));
		do{
			if (!HasLabel(_T("弃牌"),QiPaiObject,93.0f,&qipai))
			{
				CObjectData ZhiDongObject;
				if (HasLabel(_T("快速开始"),ZhiDongObject,93.0f,&kuaisukaishi)) 
				{
					ClickBnt(&ZhiDongObject);  //自动选座位
					
					Sleep(3000);
					CObjectData CloseBnt;	
					if(CaptureObject(_T("关闭买分弹出窗口"),CloseBnt)){
						ClickBnt(&CloseBnt);  
					}						
				}else{
					ClickBnt(&Ob4); //击关闭可能出现的分享窗口
					Sleep(200);
					LingFen();
					Sleep(300);
				}
				UpdateScene();

			}else{
				NotifyTip(_T("开始玩牌"));
				break;
			}
			
			BOOL bPause = IsPause();
			while(m_Alive && IsPause()){
				Sleep(20);
			}
	
		}while(m_Alive);
		
		if(!m_Alive ){
			Output = _T("User stop");
			return true;
		}
		
		assert(QiPaiObject.Size());
		
		BOOL ret = PlayNormalRound(pf);
		
		if (!ret)
		{
			
			ClickBnt(&QiPaiObject);
			
			tstring Action = _T("操作: 弃牌");
			NotifyTip(Action);	
			
			
			if (m_CurPlayType ==_T("2 刷局"))
			{
				Sleep(300); //等待弃牌按钮变化
			}else{
				//得到手上筹码
				int32 MyHandRoll = CheckMyRoll(m_SeatPos);
				
				//如果赢的分大于一定数目，则站起来保存，之后会重新坐下
				if(!m_bBisai &&  MyHandRoll>m_MinBet*400){
					CObjectData* BntObject = FindObject(_T("站起"));	
					if(BntObject){
						ClickBnt(BntObject);
						NotifyTip(_T("已存分"));	
					}
				}
				Sleep(500);
			}
			bPlayOnceEnd = true;
			tstring tip = tformat(_T("已经玩 %d 局"),++m_PlayNum);
			NotifyTip(tip);

			Output = tformat(_T("座位:%d, 总分:%d  已赢:%d  "),m_SeatPos,m_CurTotal,m_CurTotal-m_StartTotal);
		}else{
			if (m_OldCardList.size()==7)
			{
				m_OldCardList.clear();
				bPlayOnceEnd = true;
				
				tstring tip = tformat(_T("已经玩 %d 局"),++m_PlayNum);
				NotifyTip(tip);

				Output = tformat(_T("座位:%d, 总分:%d  已赢:%d  "),m_SeatPos,m_CurTotal,m_CurTotal-m_StartTotal);
			};
		}
	}
	return true;
}


BOOL CMainFrame::CmdClearCookie(ePipeline& Param){
	if (!Param.HasTypeAB(0x30000000))
	{
		Param.PrintLabel(_T("ClearCookie() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	tstring Ulr = Param.PopString();

	InternetSetOption(0, INTERNET_OPTION_END_BROWSER_SESSION, 0, 0);
	tstring NewCookie;
	InternetSetCookie(Ulr.c_str(),NULL,NewCookie.c_str());
	return true;
}

BOOL CMainFrame::CmdClickPoint(ePipeline& Param){
	if (!Param.HasTypeAB(0x11000000))
	{
		Param.PrintLabel(_T("CmdClickPoint() param error"));
		Param.SetID(RETURN_ERROR);
		return FALSE;
	}
	UpdateScene();
	ClickPoint(Param);
	return true;
}