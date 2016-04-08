// TexasPoker.h: interface for the CTexasPoker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASKRUN_H__A3CEF3B9_3225_4573_93F4_845B17915A9E__INCLUDED_)
#define AFX_TASKRUN_H__A3CEF3B9_3225_4573_93F4_845B17915A9E__INCLUDED_


#include "Tesseract.h"
#include "TheorySpace.h"
#include <map>
#include "UserTimeStamp.h"
#include "UserMutex.h"
#include "BitmapEx.h"
#include "CLog.h"
#include "OptionPipe.h"

/*
#pragma comment(lib,"cximage.lib")
#pragma comment(lib,"Jpeg.lib")
#pragma comment(lib,"zlib.lib")
#pragma comment(lib,"Tiff.lib")
#pragma comment(lib,"png.lib")
#pragma comment(lib,"mng.lib")
#pragma comment(lib,"libdcr.lib")
#pragma comment(lib,"jbig.lib")
#pragma comment(lib,"jasper")
*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std;

#define  POKER_SCROLL_WEB     10001
#define  POKER_UPDATE_OBJECT  10002
#define  POKER_CLICK_OBJECT   10003
#define  POKER_GAME_OVER      10005
#define  POKER_GET_LABLEL     10007
#define  POKER_GAME_BEGIN     10009
#define  POKER_GET_SCENE      10010
#define  POKER_OUTPUT_LOG     20001
#define  POKER_OCR_FAIL       20002
#define  POKER_OCR_OK         20003
#define  POKER_GET_EVALUATE   20004
#define  POKER_RESET_CARDPOS  20005 
#define  POKER_OPEN_WEB       20006
#define  POKER_LOGIN          20007
#define  POKER_INPUT_TEXT     20008


tstring RunTesseractExe(tstring& ImageFileName,tstring& ObjectName,AnsiString&LangName);

int32   GetCardFaceNum(tstring& Face);
int32   GetCardSuitNum(tstring& Suit);


enum PLAY_FACTOR_0 { PF_NORMAL,PF_ACTIVE,PF_PASSIVE };

class CObjectData : public ePipeline
{
public:
	CObjectData();
	virtual ~CObjectData();
	void Reset();
};

class CMainFrame;


class CTesseract;

class CTexasPoker : public Object  
{
protected:
	int64                   m_Alias;      //������ʱ�ļ�������
	tstring					m_PokerName;  //ָ�������˻�������

	CUserMutex              m_Mutex;
	CLockPipe               m_LockPipe;

	CMainFrame*             m_MainFrame;
	HANDLE                  m_hThread;


	AnsiString              m_TessDataDir;
	CTesseract*             m_Tesseract;
    tstring                 m_TempDir;


	map<tstring,ePipeline>  m_PokerObjectList;	

	int64                   m_LastMsgTimeStamp;
	eElectron               m_LastRevMsg;

	int32                   m_WaitPokerTime; //DEFAULT=500ms

	HBITMAP                 m_Scene;  //�Ƴ���ͼ

	POINT                   m_OffsetPos; //��Ϸ�����ڳ����е�ƫ��λ��

	BOOL                    m_bPause;
	BOOL                    m_bDebug;
	BOOL                    m_bRecordGame;
	BOOL                    m_bBisai;
	BOOL                    m_bTaojingSai;

	BOOL                    m_bLingFen; //���
	int64                   m_LastLingFenTime;
    int32                   m_LingfenDaoQiTime;    //���ʱ�䣨�룩

	COptionPipe             m_Option;

	CLog                    m_Log;

	map<uint32,tstring>     m_OcrObjectList;

	int32                   m_StartTotal;  //��ʼʱ���ܷ�
	int32                   m_CurTotal;
    int32                   m_SitPos;      //��λ
	int32                   m_RoomNum;     //�����
	int32                   m_MinBet;      //��Сäע
	int32                   m_HandRoll;     //��ǰ���Ϸ�
	vector<int32>           m_CardList;    //�Ѿ�������

	ePipeline               m_PreTaskPipe;

public:
    CBitmapEx               m_BaseImage;   //��׼��ͼ������У���Ƴ�λ��
public:
	CTexasPoker(CMainFrame* MainFrame);
	virtual ~CTexasPoker();

	virtual bool Activation();
	virtual void Dead();
	
	void  SetPreTaskParam(ePipeline& Pipe){
		m_PreTaskPipe << Pipe;
	}

	BOOL PreTaskDo();

	tstring GetName(){ return m_PokerName;};
	POINT GetOffsetPos(){return m_OffsetPos;};
    
	void InitObjectList(tstring PokerName,ePipeline& Pipe);
	void SetObject(ePipeline* Object); //��ʼ�����û�����½�ͼ�õ�������

	void  UpdateScene();

	void  SetOption(ePipeline& Pipe);
	void  GetOption(ePipeline& Pipe);

	int32 Name2ID(tstring Name);

	void SendMsg(int64 ID,ePipeline& Data);
	void ReceiveMsg(ePipeline* Msg){
		m_LockPipe.Push(Msg);
	}
	tstring OCR(tstring ImageFileName,AnsiString LanName);
    tstring OCR(ePipeline* Object,BOOL IsThresholded);

	
	virtual bool Do(Energy* E);	

	void SetOcrText(uint32 Crc32,tstring& OcrText);
	tstring GetOcrText(uint32 Crc32);

	//tools
    int32  Str2Num(tstring& str);
    int32  Str2Time(tstring& str);

	void  Stop();
	void  Pause(BOOL);
    BOOL  IsPause();


    void CaptureCardPosSign(); //�����Զ���ȡ���������Ʊ�־

	void AllIn();
	void FindBlankSitList(ePipeline& SitList);

	BOOL PlayNormal(PLAY_FACTOR_0 f);
	BOOL PlayShuaPeng(BOOL bActivity);

protected:

	BOOL TestPlayerPos();    //�������pos  
	BOOL TestOtherCardPos(); //���Թ�����pos

protected:

	void Init();
	BOOL JoinGame(tstring GameType);
	BOOL PlayGame();
    
	BOOL HasLabel(tstring Name,float32 f=95.0f);
	BOOL HasLabel(tstring Name,CObjectData& ob,float32 f=95.0f);

	BOOL  CalculateOffsetPos();

	ePipeline* WaitForFeedback(int64 StartTime,int32 Sec);

	void ClickBnt(ePipeline* ob,BOOL WaitFeedback);

	void NotifyOCRFail(ePipeline* ob);
	void NotifyOCROK(ePipeline* ob);
	void NotifyTip(tstring Tip);
	void NotifCard(tstring& Tip,vector<int32>& MyPorker,vector<int32>& OtherPorker);

	tstring OcrNumObject(CBitmapEx& Image);
    /*
	ePipeline(m_Label=txt){
		int  type
		uint crc32
		int  x
		int  y
		int  w
		int  h
		.......�������ݲ�ͬ�����в�ͬ������
	}
	*/

	BOOL    CaptureObject(tstring ObjectName,CObjectData& ob);
	BOOL    CaptureObjectSet(ePipeline& ObjectNameList,ePipeline& ObjectList);
	BOOL    WaitObject(CBitmapEx& srcImage, tstring Name,CObjectData& ob);

	BOOL    IsTaoJingSai();
   
	void   LingFen();

	int32  CheckPlayerNum();

	int32 CheckMySitPos(int32 PrePos=-1);  //�õ��ҵ���λ��,�������PrePos����ֻ������λ��
	BOOL  CheckHasCard(tstring Name);  //��鹫����λ�����Ƿ�����
	int32 CheckOtherCardNum(); //�õ���������
    BOOL  CheckPoketCard(vector<int32>& MyCard,int32 SitPos); //�õ��Լ����ϵ���
	BOOL  CheckOtherCard(vector<int32>& OtherCard); //�õ�������
    int32 CheckTotal();   //�õ��ҵ��ܷ�
	int32 CheckMyRoll(int32 SitPos);  //�õ��ҵĳ���
    int32 CheckMinRoll(); //�õ���Сäע
    int32 CheckBetRoll(); //�õ�Ҫ�����ע

    float32 CheckHand(vector<int32>& MyPorker,vector<int32>& OtherPorker,tstring& Memo);
};

#endif // !defined(AFX_TASKRUN_H__A3CEF3B9_3225_4573_93F4_845B17915A9E__INCLUDED_)