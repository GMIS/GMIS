// MainFrame.h: interface for the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRAME_H__36A02368_67BF_4C23_AE6D_DF4A037A2F57__INCLUDED_)
#define AFX_MAINFRAME_H__36A02368_67BF_4C23_AE6D_DF4A037A2F57__INCLUDED_

#include "Tesseract.h"
#include "TheorySpace.h"
#include <process.h>
#include "UserMutex.h"
#include "UserObject.h"
#include "BitmapEx.h"
#include "OptionPipe.h"
#include "CLog.h"

#include "resource.h"

#include "WSFrameView.h"
#include "ObjectView.h"
#include "StatusBar.h"
#include "Splitter.h"
#include "WebHost.h"
#include "PokerDatabase.h"
#include "ResultView.h"
#include "AddressBar.h"
#include "OcrResultView.h"
#include "SplitterView.h"
#include "PokerDatabase.h"
#include "Ipclass.h"
#include "TexasPokerEvaluate.h"
#include "OptionView.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct MY_ACCOUNT{
	tstring Name;
	tstring Password;
	tstring Confirm;
	tstring CrypText;
	IP      ip;
};

#define  AUTO_LOGIN_TIMER    1032
#define  WEB_OPEN_URL       10002
#define  WEB_CLICK_OBJECT   10003
#define  WEB_CLICK_POINT    10004
#define  WEB_GET_SCENE      10010
#define  WEB_LOGIN          20007
#define  WEB_REFESH         20008

using namespace VISUALSPACE;

uint32 Crc32Bitmap(HBITMAP hbitmap);
uint32 Crc32Bitmap(CBitmapEx& Image);
float  CompareBmp(CBitmapEx& Image1, CBitmapEx& Image2);

int32   GetCardFaceNum(tstring& Face);
int32   GetCardSuitNum(tstring& Suit);


enum PLAY_FACTOR_0 { PF_NORMAL,PF_ACTIVE,PF_PASSIVE };

tstring GetCurrentDir();

#ifdef _WINDLL

HANDLE GetDLLHandle();

#endif
class CMainFrame : public CUserObject, public CWSFrameView  
{

//GUI
public:
	CPokerDatabase      m_PokerDB;

public:
	BOOL                m_bCreated;

	CAddressBar         m_AddressBar;

	CSplitterView       m_SplitterView;
	CObjectView         m_CurObjectView;
	CResultView         m_ResultView;
	COcrResultView      m_OcrView;  

	SplitterStruct      m_Splitter;
	CVSpace2*           m_SpaceSelected;

	COptionView         m_OptionView;

	CWebHost            m_WebHost;
	CStatusBar          m_Status;

	//�����ڽ�ͼ״̬
	int64                  m_CaptureItemID;

//����
public: 
	tstring                 m_UserName;   //�ʺ�����������ʱ�ļ�������    
	tstring					m_PokerName;  //ָ�������˻�������

	CUserMutex              m_Mutex;

	AnsiString              m_TessDataDir;
	CTesseract*             m_Tesseract;
	tstring                 m_TempDir;
	
//	CBitmapEx               m_MaskNumImage;
//	CBitmapEx               m_MaskPokerImage;

	tstring                   m_OcrNumTempFileName;
	tstring                   m_OcrTxtTempFileName;

	map<tstring,CObjectData>  m_PokerObjectList;	
	map<uint32,tstring>       m_OcrObjectList;


	HBITMAP                 m_Scene;       //�Ƴ���ͼ
	CBitmapEx               m_BaseImage;   //��׼ͼ��

	POINT                   m_BasePos;     //��׼ͼ���ڳ����е����꣬������ͼ��������Ҫ���ڴ�����ת������ʵ����

	BOOL                    m_bPause;
	BOOL                    m_bDebug;
	BOOL                    m_bSaveDebugToLog;
	BOOL                    m_bSaveOcrImag;
	BOOL                    m_bBisai;
	BOOL                    m_bTaojingSai;

	BOOL                    m_bLingFen;     //���
	int32                   m_LingFenCount; //��ִ���
	

	int32                   m_PlayNum;        //ͳ�����ƴ���

	int32                   m_CardType;  //-1=δ���� 0=С�ְ棬1=���ְ�

	COptionPipe             m_Option;

	tstring                 m_CurPlayType;
	int32                   m_StartTotal;   //��ʼʱ���ܷ�
	int32                   m_CurTotal;
	int32                   m_SeatPos;      //��λ
	int32                   m_RoomNum;      //�����
	int32                   m_MinBet;       //��Сäע
	int32                   m_HandRoll;     //��ǰ���Ϸ�
	vector<int32>           m_OldCardList;     //�Ѿ�������

	ePipeline               m_UserAccount;

	HFONT                   m_Font;

	CTexasPokerEvaluate     m_TexasPokerEvaluate;

	CLog                    m_Log;

	CUserMutex              m_DoLock; //����DLL�棬ȷ��������ִ��
	bool                    m_IsRuning;
public:
	CMainFrame(CTimeStamp* Timer,CAbstractSpacePool* Pool);
	CMainFrame();

	virtual ~CMainFrame();
	void    FlushWindow();
	void    InitSplitterPos(int32 Pos){
		m_Splitter.SetFirstSplitterPos(Pos);
	};
	BOOL    IsPause();

	bool    Activation();
	void    Dead();
	BOOL    CalculateBaseImagePos(HBITMAP Image);

//tool
///////////////////////////////////////////////////////////////////////////
protected:
	/*
	ePipeline(m_Label=SaveFileName(��ѡ))
	{
	int64    ID
	string   Name
	int32    Type
	int32    x
	int32    y
	int32    w
	int32    h
	uint32   crc32
	string   ocrtxt
	uint32   hbitmap
	}
	*/
	BOOL    CaptureObject(tstring ObjectName,CObjectData& ob);
	void    CaptureCardPosSign(); //�����Զ���ȡ���������Ʊ�־

	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName );


	CObjectData* FindObject(tstring ObjectName);
	void    UpdateObjectImage(CObjectData& Object,BOOL bBaseImage); //��ʼ�����û�����½�ͼ�õ�������
	void    UpdateScene();	

	int32   Str2Num(tstring& str);
	int32   Str2Time(tstring& str); //���ض�����
	
	tstring OCR(tstring ImageFileName,AnsiString LanName);
	tstring OCR(ePipeline* Object,BOOL IsThresholded);

	void    RegisterOcredObject(uint32 Crc32,tstring& OcrText);
	tstring GetOcredObject(uint32 Crc32);
	
	tstring OcrObject(CBitmapEx& Image,BOOL bNegative,BOOL bPokerSuit,const char* lang);
	tstring OcrNumObject(CBitmapEx& Image);

	void    NotifyTip(tstring Tip);
	void	NotifyOCRFail(ePipeline* ob);
	void	NotifyOCROK(ePipeline* ob);
	void	NotifCard(tstring& Tip,vector<int32>& MyPorker,vector<int32>& OtherPorker);

	void    FindBlankSitList(ePipeline& SitList);

	void    ClickBnt(ePipeline* ob);
	void    ClickPoint(ePipeline& Point);
	BOOL    HasLabel(tstring Name,float32 f=93.0f,CBitmapEx* SrcImage=NULL);
	BOOL    HasLabel(tstring Name,CObjectData& ob,float32 f=93.0f,CBitmapEx* SrcImage=NULL);

	void    WebInputText(tstring& text);

	tstring CheckRoomID();
	BOOL    CheckBlankSeat();
	int32   CheckPlayerNum();
	int32   CheckMySitPos(int32 PrePos=-1);  //�õ��ҵ���λ��,�������PrePos����ֻ������λ��
	BOOL    CheckHasCard(tstring Name);  //��鹫����λ�����Ƿ�����
	int32   CheckOtherCardNum(); //�õ���������
	BOOL    GetSelfCard(vector<int32>& MyCard); //�õ��Լ����ϵ���
	BOOL    CheckOtherCard(vector<int32>& OtherCard); //�õ�������
	int32   CheckTotal();   //�õ��ҵ��ܷ�
	int32   CheckMyRoll(int32 SitPos);  //�õ��ҵĳ���
	int32   CheckMinRoll(); //�õ���Сäע
	int32   CheckBetRoll(); //�õ�Ҫ�����ע
	float32 CheckHand(vector<int32>& MyPorker,vector<int32>& OtherPorker,tstring& Memo);

protected:

	void ChangeAllObjectPos(int32 dx, int32 dy); //help func,������Ҫһ����ƫ�����н�ͼ������
	BOOL GetSelfCard(vector<int32>& MyCard,tstring& Name1,tstring& Name2,tstring& Name3,tstring& Name4);

//��������
//////////////////////////////////////////////////////////////////////////
public:
	virtual bool Do(Energy* E=NULL);

	bool InitPokerGame();

	void AllIn();

	BOOL PlayGame();

	BOOL PlayNormalRound(PLAY_FACTOR_0 f);
	BOOL PlayShuaPeng(BOOL bActivity);
	BOOL PlayDingShiRenWu();
	BOOL PlayShuaJu();
	BOOL PlayZhuanfen();
	BOOL SelectPlayType(tstring GameType);
	BOOL Kuaisuxunzhaozuowei();
	void LingFen();
	

//GUI
//////////////////////////////////////////////////////////////////////////
protected:
	virtual void Layout(bool Redraw =true);

	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	LRESULT OnWebClickObject(SpaceRectionMsg* SRM);
	LRESULT OnWebClickPoint(SpaceRectionMsg* SRM);
	LRESULT OnWebGetScene(SpaceRectionMsg* SRM);
	LRESULT OnWebLogin(SpaceRectionMsg* SRM);
	LRESULT OnWebOpenURL(SpaceRectionMsg* SRM);
	LRESULT OnWebRefresh(SpaceRectionMsg* SRM);

	virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);
	LRESULT OnParentWebCompleted(SpaceRectionMsg* SRM);
	LRESULT OnParentMouseMove(SpaceRectionMsg* SRM);
	LRESULT OnParentCaptrueObject(SpaceRectionMsg* SRM);
	LRESULT OnParentObjectSelected(SpaceRectionMsg* SRM);
	LRESULT OnParentOcrObject(SpaceRectionMsg* SRM);
	LRESULT OnParentConnect(SpaceRectionMsg* SRM);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);

	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
  
//�����ӿ�
/////////////////////////////////////////////////////////////////////////
public:
	bool WaitForCompleted();

	BOOL CmdProc(tstring Cmd,ePipeline& Param);
	BOOL CmdOutputTip(ePipeline& Param);
	BOOL CmdOpenGameWeb(ePipeline& Param);
	BOOL CmdRefresh(ePipeline& Param);
	BOOL CmdClickObject(ePipeline& Param);
	BOOL CmdDBClickObject(ePipeline& Param);
	BOOL CmdLogin(ePipeline& Param);
	BOOL CmdHasSeat(ePipeline& Param);
	BOOL CmdHasLabel(ePipeline& Param);
	BOOL CmdInputText(ePipeline& Param);
	BOOL CmdGetRoomID(ePipeline& Param);
	BOOL CmdGetTotalRoll(ePipeline& Param);
	BOOL CmdGetMySiteID(ePipeline& Param);
	BOOL CmdGetMinRoll(ePipeline& Param);
	BOOL CmdGetHandCard(ePipeline& Param);
	BOOL CmdGetCommonCard(ePipeline& Param);
	BOOL CmdGetBetRoll(ePipeline& Param);
	BOOL CmdPauseGame();
	BOOL CmdStopGame();
	BOOL CmdRunGame();
	BOOL CmdRunOnce(tstring& Output);
    BOOL CmdClearCookie(ePipeline& Param);
	BOOL CmdClickPoint(ePipeline& Param);

};

#endif // !defined(AFX_MAINFRAME_H__36A02368_67BF_4C23_AE6D_DF4A037A2F57__INCLUDED_)
