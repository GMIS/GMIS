// TexasPoker.cpp: implementation of the CTexasPoker class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)


#include "TexasPoker.h"
#include "MainFrame.h"
#include "webhost.h"
#include "ObjectView.h"
//#include "FORMAT.H"
#include "crc32.h"
#include <iostream>
#include <fstream>
#include<string>

#include "leptwin.h"



int32  GetCardSuitNum(tstring& Suit){
    if(Suit.find(_T("H"))!=-1) return 0;
	else if(Suit.find(_T("C"))!=-1) return 100;
	else if(Suit.find(_T("D"))!=-1) return 200;
	else if(Suit.find(_T("S"))!=-1) return 300;
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
	else  return 0;
}


CObjectData::CObjectData(){

}

CObjectData::~CObjectData(){
	Reset();
}

void CObjectData::Reset()
{
	if (Size()==10)
	{
		uint32 handle = *(uint32*)GetData(9);
		if (handle)
		{
			HBITMAP hbitmap = (HBITMAP)handle;
			::DeleteObject(hbitmap);
		}
	}
	Clear();
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTexasPoker::CTexasPoker(CMainFrame* MainFrame)
:m_LockPipe(&m_Mutex),
m_MainFrame(MainFrame),
m_Option(&m_Mutex),
m_WaitPokerTime(500),
m_bPause(FALSE),
m_LastMsgTimeStamp(0),
m_StartTotal(0),
m_CurTotal(0),
m_RoomNum(0),
m_SitPos(-1),
m_MinBet(0),
m_HandRoll(5000),
m_Scene(NULL),
m_bDebug(FALSE),
m_bRecordGame(TRUE),
m_bTaojingSai(FALSE),
m_bLingFen(FALSE),
m_LingfenDaoQiTime(0)
{
	m_Alias = CreateTimeStamp();   

	m_Tesseract = new CTesseract;

	char lpBuffer[MAX_PATH];
	::GetCurrentDirectoryA(MAX_PATH,lpBuffer);

	m_TessDataDir = lpBuffer;
	m_TessDataDir +="\\tessdata";

	m_Tesseract->SetLang(m_TessDataDir.c_str(),"poker");
	assert(m_Tesseract->IsValid());


	TCHAR lpBuffer2[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,lpBuffer2);
	m_TempDir = tformat(_T("%s\\temp%I64ld\\"),lpBuffer2,m_Alias);
	_tmkdir(m_TempDir.c_str());

	tstring LogFileName = m_TempDir+tformat(_T("Poker(%I64ld).log"),m_Alias);
	m_Log.Open(LogFileName.c_str());

	m_OffsetPos.x = 0;
	m_OffsetPos.y = 0;

}

CTexasPoker::~CTexasPoker()
{
	if(m_Tesseract){
		delete m_Tesseract;
	}
	if(m_Scene){
		::DeleteObject(m_Scene);
	}

}


int32 CTexasPoker::Name2ID(tstring Name){
	map<tstring,ePipeline>::iterator it = m_PokerObjectList.find(Name);
	if(it == m_PokerObjectList.end()){	 
		return -1;
	}
	ePipeline& Object = it->second;
	int64 ID = *(int64*)Object.GetData(0);
	assert(ID<200);
	return ID;
}

void CTexasPoker::InitObjectList(tstring PokerName,ePipeline& ObjectList){
	m_PokerName = PokerName;

	m_PokerObjectList.clear();

	for (int i=0; i<ObjectList.Size(); i++)
	{
        ePipeline& SrcObject = *(ePipeline*)ObjectList.GetEnergy(i); 
		tstring Name = *(tstring*)SrcObject.GetData(1);
         
		ePipeline& Object = m_PokerObjectList[Name];
		Object << SrcObject;

	}
   	
};    
  
void CTexasPoker::SetObject(ePipeline* ObjectData){
	
	uint32& handle       = *(uint32*)ObjectData->GetData(9);
	HBITMAP hBitmap      = (HBITMAP)handle;

	CBitmapEx Image;
	Image.Load(hBitmap);

	long ImageSize;
	LPBYTE ImageData;
	Image.Save(ImageData,ImageSize);
	assert(ImageSize);

	ePipeline* temp = (ePipeline*)ObjectData->Clone();
	temp->EraseBackEnergy();  //把最后一个HBITMAP句柄改为BLOB数据 

	eBLOB* Blob = new eBLOB((const char*)ImageData,ImageSize);
	temp->Push_Directly(Blob);

	delete ImageData;

	tstring Name = *(tstring*)ObjectData->GetData(1);
	ePipeline& Object = m_PokerObjectList[Name];
	Object.Clear();
	Object << *temp;
	delete temp;
}

void CTexasPoker::SendMsg(int64 ID,ePipeline& Data){

	m_LastMsgTimeStamp = AbstractSpace::CreateTimeStamp();
    Data.SetID(m_LastMsgTimeStamp);
	m_MainFrame->SendChildMessage(m_MainFrame->GetHwnd(),ID,(int64)&Data,0);
};

void  CTexasPoker::SetOption(ePipeline& Pipe){
	m_Option.FromPipe(Pipe);
};

void  CTexasPoker::GetOption(ePipeline& Pipe){
	m_Option.ToPipe(Pipe);	
};

ePipeline* CTexasPoker::WaitForFeedback(int64 StartTime, int32 Sec){
	int64 EndTime = StartTime +Sec*1000*1000*10; //秒=1秒*1000毫秒*1000微妙*10百纳秒
	
GETMSG:
	while(m_LockPipe.DataNum() == 0){
		if(StartTime<EndTime){
			Sleep(20);
			StartTime = AbstractSpace::CreateTimeStamp();
		}else{
			return NULL;
		};
	}

	m_LockPipe.Pop(&m_LastRevMsg);
	ePipeline* Msg = (ePipeline*)m_LastRevMsg.Value();
    int64 EventID = Msg->GetID();
	if (EventID == m_LastMsgTimeStamp){
		return Msg;
	}
	else{
		m_LastRevMsg.Reset(NULL);
		goto GETMSG;
    }
	return NULL;
}


BOOL CTexasPoker::CaptureObjectSet(ePipeline& ObjectNameList,ePipeline& ObjectList){
	while(ObjectNameList.Size())
	{
		tstring Name = ObjectNameList.PopString();
		CObjectData* ob = new CObjectData;
		ObjectList.Push_Directly(ob);
        if (!CaptureObject(Name,*ob))
        {
			ObjectList.Clear();
			return FALSE;
        }
	}
	return TRUE;
}


BOOL  CTexasPoker::CaptureObject(tstring ObjectName,CObjectData& ob){
	ob.Reset();

	map<tstring,ePipeline>::iterator it = m_PokerObjectList.find(ObjectName);
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object = it->second;
	
	if (Object.Size() <10)
	{
		return  FALSE;
	}

	for(int i=0; i<Object.Size()-1; i++){
		ob.Push_Copy(Object.GetEnergy(i));
	}
	
	int32      x = *(int32*)ob.GetData(3);
	int32      y = *(int32*)ob.GetData(4);
	int32      w = *(int32*)ob.GetData(5);
	int32      h = *(int32*)ob.GetData(6);
	
	x += m_OffsetPos.x;
	y += m_OffsetPos.y;

	HBITMAP hBitmap =CaptureImageFromBmp(m_MainFrame->m_WebHost.GetHandle(),m_Scene,x,y,w,h);
				
	uint32 handle = (uint32)hBitmap;
	ob.PushUint32(handle);

	return TRUE;
}

BOOL FindOffsetPos(CBitmapEx& SceneImage, int32 x1,int32 y1,int32 x2,int32 y2, CBitmapEx& BaseImage,POINT& p){

	int32 wScene = SceneImage.GetWidth();
	int32  wBase = BaseImage.GetWidth();
	int x =0;
	_PIXEL pxBase = BaseImage.GetPixel(0, 0);
	
	for (y1; y1<y2; y1++)
	{
		for (x1=0; x1<x2; x1++)
		{
			_PIXEL px = SceneImage.GetPixel(x1, y1);
			if(px == 0xffffffff)continue;

			DWORD d= px&pxBase;
			if ( d== pxBase) //开始横向对比
			{		
				BOOL bFind = TRUE;
				for (x=1; x<wBase && x+x1<wScene; x++)
				{
					_PIXEL px1 = BaseImage.GetPixel(x, 0);
                    px = SceneImage.GetPixel(x+x1, y1);
					d = px&px1;
				    if (d != px1)
					{
						bFind = FALSE;
						break;
					}        
				}

				if (bFind)
				{		
					p.x = x1;
					p.y = y1;
					return TRUE;
				}
			}
		}	
	}	
    return FALSE;
}

BOOL  CTexasPoker::CalculateOffsetPos(){
	if (m_Scene == NULL)
	{
		return  FALSE;
	}
	CBitmapEx SceneImage;
	SceneImage.Load(m_Scene);

	if (!m_BaseImage.IsValid())
	{
		map<tstring,ePipeline>::iterator it = m_PokerObjectList.find(_T("基准线"));
		if(it == m_PokerObjectList.end()){	 
			return FALSE;
		}
		ePipeline& Object = it->second;
		if(Object.Size()<10)return FALSE;

		eBLOB& Blob = *(eBLOB*)Object.GetEnergy(9);
		m_BaseImage.Load((LPBYTE)Blob().c_str());
	}

	int32  hScene = SceneImage.GetHeight();
	int32  wScene  = SceneImage.GetWidth();
	
		
	//优先按现有偏移点查找	
	if (FindOffsetPos(SceneImage, m_OffsetPos.x, m_OffsetPos.y, m_OffsetPos.x+10,m_OffsetPos.y+10,m_BaseImage,m_OffsetPos))
	{
		return FALSE;
	}

	//否则按现有基准点周围正负100的范围寻找
	int32 d = 300;
	int x1 = m_OffsetPos.x-d>-1? m_OffsetPos.x-d:0;
	int y1 =  m_OffsetPos.y-d>-1? m_OffsetPos.y-d:0;
	int x2 = m_OffsetPos.x+d<wScene? m_OffsetPos.x+d:wScene;
	int y2 = m_OffsetPos.y+d<hScene? m_OffsetPos.y+d:hScene;
	
	BOOL ret = FindOffsetPos(SceneImage,x1,y1,x2,y2,m_BaseImage,m_OffsetPos);
    return ret;		
}

BOOL CTexasPoker::WaitObject(CBitmapEx& srcImage, tstring Name, CObjectData& ob){

	ob.Clear();

	UpdateScene();

    if(!CaptureObject(Name,ob))return FALSE;

	uint32 handle  = *(uint32*)ob.GetData(9);
	HBITMAP Newbitmap = (HBITMAP)handle;

	CBitmapEx NewImage;
	NewImage.Load(Newbitmap);
	NewImage.AutoThreshold();
	
	float f = CompareBmp(srcImage,NewImage);
	
//	tstring s = tformat(_T("wait %s: f=%f"),Name.c_str(),f);
//	NotifyTip(s);

	if (f>99.5f)
	{
		return TRUE;
	}else{
/* 等待循环次数太多，显示意义不大
		tstring Name = *(tstring*)ObjectData->GetData(1);
		tstring FileName = tformat(_T("%s_wait_src.bmp"),Name.c_str());
		srcImage.Save((TCHAR*)FileName.c_str());
        CBitmapEx NewImage1; 
		NewImage1.Load(Newbitmap);
		FileName = tformat(_T("%s_wait_dst.bmp"),Name.c_str());
		NewImage1.Save((TCHAR*)FileName.c_str());
		FileName = tformat(_T("%s_wait_ocr.bmp"),Name.c_str());
		NewImage.Save((TCHAR*)FileName.c_str());

		tstring s = tformat(_T("wait %s fail: f=%.1f"),Name.c_str(),f);
		NotifyTip(s);
*/
  }
	
	return FALSE;
}

void CTexasPoker::ClickBnt(ePipeline* ob,BOOL WaitFeedback){
	tstring Name = *(tstring*)ob->GetData(1);
		
	ePipeline Letter; 
	Letter<< *ob;

	int32&   x = *(int32*)Letter.GetData(3);
	int32&   y = *(int32*)Letter.GetData(4);
	x += m_OffsetPos.x;
	y += m_OffsetPos.y;

	SendMsg(POKER_CLICK_OBJECT,Letter);

	ePipeline* Msg = WaitForFeedback(m_LastMsgTimeStamp,5);
	if (!Msg)
	{
		return;
	}


	while(WaitFeedback && m_Alive  && HasLabel(Name)){
		UpdateScene();
		Sleep(500);
	}; //等待按下的按钮消失（起作用）

	if (Msg->Size()==2)
    {
		//int32 Scroll_x = Msg->PopInt32();
		//int32 Scroll_y = Msg->PopInt32();
		//SendMsg(POKER_SCROLL_WEB,*Msg);
    }
}

int32  CTexasPoker::Str2Num(tstring& str){
	tstring numStr;
	for(int i=0; i<str.size(); i++)
	{
		TCHAR ch = str[i];
		if (_istdigit(ch))
		{
			numStr+=ch;
		}else if(ch ==_T(',') || ch==_T('$')){
			continue;
		}else if(!_istspace(ch) && _istprint(ch) )
		{
			return 0;
		}
	}

	int32  n =_ttoi(numStr.c_str());
	return n;
}

int32  CTexasPoker::Str2Time(tstring& str){
	if(str.size()<6)return 0;

	tstring s1 = str.substr(0,2);
	int32  n1 =_ttoi(s1.c_str());
	tstring s2  = str.substr(3,2);
	int32  n2 =_ttoi(s1.c_str());	
	return n1*60+n2;
}



BOOL CTexasPoker::JoinGame(tstring GameType){
	CObjectData ob1;
	if(HasLabel(GameType,ob1))
	{
		ClickBnt(&ob1,FALSE);
		Sleep(1000);
	}
	
	if(!HasLabel(GameType))
	{
		return FALSE;
	}
	
	CObjectData ob2;
	if(CaptureObject(_T("快速寻找座位"),ob2))
	{
		ClickBnt(&ob2,TRUE);
		Sleep(8000);
		LingFen();
		return TRUE;
	}

	return FALSE;
};

BOOL CTexasPoker::HasLabel(tstring Name,CObjectData& ob, float32 f)
{

	if(!CaptureObject(Name,ob)){
		return FALSE;
	}

    ePipeline& SrcObject = m_PokerObjectList[Name];
	assert(SrcObject.Size() != 0);

	if (SrcObject.Size()<10)
	{
		return FALSE;
	}

	CBitmapEx SrcImage;
	eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
	SrcImage.Load((LPBYTE)Blob().c_str());


	uint32 handle = *(uint32*)ob.GetData(9);
	HBITMAP NewBitmap = (HBITMAP)handle;

	CBitmapEx NewImage;
	NewImage.Load(NewBitmap);

	if (m_bDebug)
	{
		tstring FileName = m_TempDir+tformat(_T("%s_Label_dst.bmp"),Name.c_str());
		NewImage.Save((TCHAR*)FileName.c_str());
	}

	NewImage.AutoThreshold();

	float f0= CompareBmp(NewImage,SrcImage);

	if(m_bDebug){
		tstring s = tformat(_T("HasLabel %s: f=%f"),Name.c_str(),f0);
		NotifyTip(s);
	};

	if (f0> f)
	{
		return TRUE;
	}else if(m_bDebug){

		tstring Name = *(tstring*)SrcObject.GetData(1);

		tstring FileName = m_TempDir+tformat(_T("%s_Label_xor.bmp"),Name.c_str());
		SrcImage.Save((TCHAR*)FileName.c_str());		
		
		SrcImage.Load((LPBYTE)Blob().c_str()); //比较时被改变，重新Load
        FileName = m_TempDir+tformat(_T("%s_Label_src.bmp"),Name.c_str());
		SrcImage.Save((TCHAR*)FileName.c_str());        

	}
	
	return FALSE;
}

BOOL CTexasPoker::HasLabel(tstring Name,float32 f){
	CObjectData ob;
	BOOL ret = HasLabel(Name,ob,f);
	return ret;
}


void CTexasPoker::NotifyOCRFail(ePipeline* ob){
	tstring&  Name = *(tstring*)ob->GetData(1);
	uint32   crc32 = *(uint32*)ob->GetData(7);
	tstring ocrtxt = *(tstring*)ob->GetData(8);
	uint32& handle = *(uint32*)ob->GetData(9);

    if(m_bDebug){
		CBitmapEx Image;
		Image.Load((HBITMAP)handle);
		int64 t = AbstractSpace::CreateTimeStamp();
		tstring FileName = tformat(_T("%s%I64ld.bmp"),m_TempDir.c_str(),t);
		int32 Type = *(int32*)ob->GetData(2);
		Image.AutoThreshold();
		if (Type==NUM_OBJECT)
		{
			Image.Scale(200,200);
			Image.Save((TCHAR*)FileName.c_str());
		}
	}
	tstring s = tformat(_T("%s ocr fail"),Name.c_str());
	ePipeline Letter;
	Letter.PushString(Name);
    Letter.PushUint32(crc32);
	Letter.PushUint32(handle);
	Letter.PushString(s);
	handle = 0;
    
	SendMsg(POKER_OCR_FAIL,Letter);
}

void CTexasPoker::NotifyOCROK(ePipeline* ob){
	tstring&  Name = *(tstring*)ob->GetData(1);
	uint32   crc32 = *(uint32*)ob->GetData(7);
	tstring ocrtxt = *(tstring*)ob->GetData(8);
	uint32 handle = *(uint32*)ob->GetData(9);
    

	HBITMAP hbitmap = (HBITMAP)handle;
	CBitmapEx Image;
	Image.Load(hbitmap);
	HBITMAP hbitmap1 = NULL;
	Image.Save(hbitmap1);

/*
	int64 t = AbstractSpace::CreateTimeStamp();
	tstring FileName = tformat(_T("%s%I64ld.bmp"),m_TempDir.c_str(),t);
	int32 Type = *(int32*)ob->GetData(2);
	Image.AutoThreshold();
    if (Type==NUM_OBJECT)
    {
        Image.Scale(200,200);
		Image.Save((TCHAR*)FileName.c_str());
    }
*/

	ePipeline Letter ;
	Letter.PushString(Name);
    Letter.PushUint32(crc32);
	Letter.PushString(ocrtxt);
	Letter.PushUint32((uint32)hbitmap1);
    
	SendMsg(POKER_OCR_OK,Letter);
}

void CTexasPoker::NotifyTip(tstring Tip){
	if (m_bDebug || m_bRecordGame)
	{
		AnsiString s = eSTRING::UTF16TO8(Tip);
		m_Log.PrintLn(s);
	}
	ePipeline Letter;
	Letter.PushString(Tip);	
	SendMsg(POKER_OUTPUT_LOG,Letter);

}
	
void CTexasPoker::NotifCard(tstring& Tip,vector<int32>& MyPorker,vector<int32>& OtherPorker){
	
	
	ePipeline Letter;
	Letter.PushString(Tip);
	
	ePipeline CardList;
	int i=0; 
	for (i=0; i<MyPorker.size();i++)
	{
		int32 card = MyPorker[i];
		CardList.PushInt32(card);
	}
	
	for (i=0; i<OtherPorker.size();i++)
	{
		int32 card = OtherPorker[i];
		CardList.PushInt32(card);
	}
	
	if (m_bDebug || m_bRecordGame)
	{
		AnsiString s = eSTRING::UTF16TO8(Tip);
		m_Log.PrintLn(s);
		s="";
		for (int i=0; i<CardList.Size();i++)
		{
			int32 card = *(int32*)CardList.GetData(i);
			s+=format("%d ",card);
		}
		m_Log.PrintLn(s);
	}
	
	Letter << CardList;
	SendMsg(POKER_OUTPUT_LOG,Letter);
}

void CTexasPoker::SetOcrText(uint32 Crc32,tstring& OcrText){
	m_Mutex.Acquire();
	m_OcrObjectList[Crc32] = OcrText;
	m_Mutex.Release();
}

tstring CTexasPoker::GetOcrText(uint32 Crc32){
    tstring s;
	m_Mutex.Acquire();
	map<uint32,tstring>::iterator it = m_OcrObjectList.find(Crc32);
	if (it != m_OcrObjectList.end())
	{
		s = it->second;
	}
	m_Mutex.Release();
	return s; 
}


float32 CTexasPoker::CheckHand(vector<int32>& MyPorker, vector<int32>& OtherPorker,tstring& Memo){
		
	assert(MyPorker.size()==2);
    assert(OtherPorker.size()==0 ||OtherPorker.size()>2);

	ePipeline Pipe;
	int i=0;
	for (i=0; i<MyPorker.size(); i++)
	{
		int t = MyPorker[i];
		Pipe.PushInt32(t);
	}
	for (i=0; i<OtherPorker.size(); i++)
	{
		int t = OtherPorker[i];
		Pipe.PushInt32(t);
	}
	

	SendMsg(POKER_GET_EVALUATE,Pipe);

	ePipeline* Msg = WaitForFeedback(m_LastMsgTimeStamp,10);
	if (!Msg )return 0;

	assert(Msg->Size()==2);
	float32 f = Msg->PopFloat32();
	Msg->PopString(Memo);

	return f;

};

void CTexasPoker::Pause(BOOL bPause){
	m_Mutex.Acquire();
	m_bPause = bPause;
	m_Mutex.Release();
};

BOOL  CTexasPoker::IsPause(){
	m_Mutex.Acquire();
	BOOL ret = m_bPause;
	m_Mutex.Release();
	return ret;
};

tstring CTexasPoker::OcrNumObject(CBitmapEx& Image){
     

/*
	int32 x2=iWidth-1;
	for (x2; x2>-1; x1--)
	{
		int32 t=0;
		for (int32 y=0; y<iHeight; y++)
		{
			_PIXEL px = Image.GetPixel(x2, y);
			t += px;			
		}
		if (t != k)//判断一列是否全是白色
		{ 
			break;
		}
	}
    
    //然后把字符重组
	CBitmapEx Image1;
	Image1.Create(120,30);

    int x = x1;
	int p = 0;
	for (x; x<x2; x+=8)
	{
//		Image1.Draw(p,0,8,iHeight,Image,x,0,8,iHeight);
//		p+=10;
		
		for (int32 y=0; y<iHeight; y++)
		{
			//Image.SetPixel(x, y,RGB(255,255,255));

		}
	}
*/
	
	bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"num");
	assert(ret);
	
	Image.Scale(200,200);
	tstring FileName = m_TempDir+tformat(_T("numocr-%I64ld.bmp"),m_Alias);
	Image.Save((TCHAR*)FileName.c_str());

	AnsiString s = m_Tesseract->OCR(FileName);

	if (s.size()>1){
		tstring ocrtxt = eSTRING::UTF8TO16(s);	
		return ocrtxt;
	}
	
	//没有识别出来的则通过与标准数字组合以后识别
	int32  iHeight = Image.GetHeight();
	int32  iWidth  = Image.GetWidth();
	
	//纵向扫描得到数字字符的范围
	int32 x1=0;
	int32 k = iHeight*255; 
	
	for (x1; x1<iWidth; x1++)
	{
		int32 t=0;
		for (int32 y=0; y<iHeight; y++)
		{
			_PIXEL px = Image.GetPixel(x1, y);
			unsigned char cr = px;
			t += cr;			
		}
		if (t != k)//判断一列是否全是白色
		{ 
			break;
		}
	}	

	int w = iWidth-x1;
	
	CBitmapEx TempImag;
	TempImag.Create(120,30);
	
	CBitmapEx MaskImage;
	MaskImage.Load(_T("masknum.bmp"));
	TempImag.Draw(0,0,MaskImage.GetWidth(),MaskImage.GetHeight(),MaskImage,0,0);
	
	TempImag.Draw(MaskImage.GetWidth(),0,w,iHeight,Image,x1,0,w,iHeight);
	
	TempImag.Save((TCHAR*)FileName.c_str());
		
	s = m_Tesseract->OCR(FileName);	
	
	tstring ocrtxt = eSTRING::UTF8TO16(s);	
	if (ocrtxt.size()>2)
	{
		ocrtxt = ocrtxt.substr(2);
	}else {
		ocrtxt = _T("");
	}
	return ocrtxt;	
}

tstring CTexasPoker::OCR(ePipeline* Object,BOOL IsThresholded)
{

	tstring  ObjectName  = *(tstring*)Object->GetData(1);
	int32    Type        = *(int32*)  Object->GetData(2);
	uint32&  crcValue    = *(uint32*) Object->GetData(7);
	tstring& ocrtxt      = *(tstring*)Object->GetData(8);
	uint32   handle      = *(uint32*) Object->GetData(9);
	HBITMAP  hBitmap     = (HBITMAP)handle;

	if(hBitmap==NULL)return ocrtxt;

	CBitmapEx Image;
	Image.Load(hBitmap);
    //Image.Load(_T("test3.bmp"));
	//Image.Save(_T("ocr.bmp"));

	if (IsThresholded)
	{
		crcValue  = Crc32Bitmap(hBitmap);	
	}else{
		crcValue  = Crc32Bitmap(Image);  //AutoThreshold
	}

	//优先查表
	ocrtxt = GetOcrText(crcValue);
	if (ocrtxt.size())
	{
		return ocrtxt;
	}

	
//	tstring FileName = m_TempDir;
//	FileName += ObjectName;
//  FileName += _T(".bmp");

	switch(Type){
	case NUM_OBJECT:
		{
			if (ObjectName == _T("自动加注额"))
			{
			    Image.BlackAndWhite();
			}
			ocrtxt = OcrNumObject(Image);
		}
		break;
	case TEXT_OBJECT:
		{
			bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"poker");
			assert(ret);

		    Image.Negative();
			CBitmapEx TempImag;
			TempImag.Create(80,30);
	
			CBitmapEx MaskImage;
			if(ObjectName.find_last_of(_T("花色")) != -1){
			    MaskImage.Load(_T("maskcard.bmp"));
				TempImag.Draw(0,0,MaskImage.GetWidth(),MaskImage.GetHeight(),MaskImage,0,0);
				TempImag.Draw(MaskImage.GetWidth(),2,Image.GetWidth(),Image.GetHeight(),Image,0,2);
			}else {
				MaskImage.Load(_T("masknum.bmp"));
				TempImag.Draw(0,0,MaskImage.GetWidth(),MaskImage.GetHeight(),MaskImage,0,0);
				TempImag.Draw(MaskImage.GetWidth(),3,Image.GetWidth(),Image.GetHeight(),Image,0,0);
			}
            tstring FileName = m_TempDir+tformat(_T("txtocr-%I64ld.bmp"),m_Alias);
			TempImag.Save((TCHAR*)FileName.c_str());			
			AnsiString s = m_Tesseract->OCR(FileName);
			ocrtxt = eSTRING::UTF8TO16(s);

			if (ocrtxt.size()>2)
			{
				ocrtxt = ocrtxt.substr(2);
			}else {
				ocrtxt = _T("");
			}
		}
		break;
	case BUTTON_OBJECT:
		{
			//bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"chi_sim");
			//assert(ret);

			Image.Save(_T("txtocr.bmp"));
			tstring FileName = m_TempDir+_T("txtocr.bmp"); 
			AnsiString s = m_Tesseract->OCR(FileName);
			ocrtxt = eSTRING::UTF8TO16(s);
		}
		break;
    default:
		if (hBitmap)
		{
		//	tstring FileName = tformat(_T("%s.bmp"),ObjectName.c_str());
		//	Image.Save((TCHAR*)FileName.c_str());
		}
		return ocrtxt;
	}

//#if _MSC_VER < 1300  

	return ocrtxt;
}


tstring CTexasPoker::OCR(tstring ImageFileName,AnsiString LanName)
{	
	CTesseract  Tess;

	Tess.SetLang(m_TessDataDir.c_str(),LanName.c_str());

	AnsiString s = Tess.OCR(ImageFileName);

	tstring ocrTxt;	
    ocrTxt = eSTRING::UTF8TO16(s);
	
	return ocrTxt;
}


bool CTexasPoker::Activation(){
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	uint32 ThreadID = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, // Security
		0,							                     // Stack size - use default
		ObjectDefaultThreadFunc,     		
		(void*)this,	      
		0,					                			 // Init flag
		&ThreadID);					                     // Thread address
	
	assert( m_hThread != NULL );        
	
	if (!m_hThread)
	{
		m_Alive = FALSE;
	}

	return TRUE;
};

void CTexasPoker::Dead(){
	m_Alive = FALSE;
	m_bPause = FALSE;
	WaitForSingleObject (m_hThread, INFINITE);
}
void  CTexasPoker::Stop(){
	m_Alive = FALSE;
	m_bPause = FALSE;
}

void  CTexasPoker::UpdateScene(){
	
	if(m_Scene){
		::DeleteObject(m_Scene);
	}

	ePipeline Letter; 
	SendMsg(POKER_GET_SCENE,Letter);
	
	ePipeline* Msg = WaitForFeedback(m_LastMsgTimeStamp,5);
	if (!Msg)
	{
		return ;
	}

	uint32 handle = Msg->PopUint32();
	m_Scene = (HBITMAP)handle;

	CalculateOffsetPos();
}

void CTexasPoker::Init(){

	ePipeline Letter; 
	Letter.PushString(m_PokerName);
	Letter.PushInt32(m_PokerObjectList.size());

	SendMsg(POKER_GAME_BEGIN,Letter);
	

	if (m_Alive==FALSE) //如果没初始化，系统会终止玩牌
	{
		return;
	}
	
	UpdateScene();


/*
	tstring s;
	vector<int32> MyPoker,OtherPoker1;
	MyPoker.push_back(205);
	MyPoker.push_back(205);

	CheckHand(MyPoker,OtherPoker1,s);

	m_Alive = FALSE;
	return;
*/
	if (!HasLabel(_T("坐标基准")))
	{
		NotifyTip(_T("坐标基准出错"));
		m_Alive = FALSE;
		return;
	}

/*
	m_bTaojingSai = IsTaoJingSai();
	if (m_bTaojingSai)
	{
		NotifyTip(_T("检测出在淘金赛，如果错误请停止然后在开牌前重新开始"));
	}else{
		NotifyTip(_T("检测出不在淘金赛，如果错误请停止然后在开牌前重新开始"));
	}
*/
};



BOOL CTexasPoker::TestPlayerPos()
{
	ePipeline ObjectList;

	//牌位识别
	ePipeline NameList;
 
	NameList.PushString(_T("1号无牌"));
	NameList.PushString(_T("2号无牌"));
	NameList.PushString(_T("3号无牌"));
	NameList.PushString(_T("4号无牌"));
	NameList.PushString(_T("5号无牌"));
	NameList.PushString(_T("6号无牌"));
	NameList.PushString(_T("7号无牌"));
	NameList.PushString(_T("8号无牌"));
	NameList.PushString(_T("9号无牌"));

	
	//确定自己座位
    int MySitPos = -1;
		
	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取Pos Object失败"));
		return -1;
	};
	
	
	for (int i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
		
		NewImage.AutoThreshold();
		
		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());
		
		float f = CompareBmp(SrcImage,NewImage);
		
		if (f>95.0f)
		{
			continue;
		}else{
			
/*		
		SrcImage.Save(_T("MyPos_src.bmp"));
		CBitmapEx NewIamg1;
		NewIamg1.Load(hBitmap);
		NewIamg1.Save(_T("MyPos_dst.bmp"));
		NewImage.Save(_T("MyPos_xor.bmp"));
*/					
//		return FALSE;
		}				
	}
	return TRUE;	
}

BOOL CTexasPoker::TestOtherCardPos()
{
	ePipeline ObjectList;

	//牌位识别
	ePipeline NameList;
 
	NameList.PushString(_T("3号公牌无牌"));
	NameList.PushString(_T("4号公牌无牌"));
	NameList.PushString(_T("5号公牌无牌"));
	NameList.PushString(_T("6号公牌无牌"));
	NameList.PushString(_T("7号公牌无牌"));
	

	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取Pos Object失败"));
		return -1;
	};
		
	for (int i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
		
		NewImage.AutoThreshold();
		
		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());
		
		float f = CompareBmp(SrcImage,NewImage);
		
		if (f>95.0f)
		{
			continue;
		}else{
			
/*		
		SrcImage.Save(_T("MyPos_src.bmp"));
		CBitmapEx NewIamg1;
		NewIamg1.Load(hBitmap);
		NewIamg1.Save(_T("MyPos_dst.bmp"));
		NewImage.Save(_T("MyPos_xor.bmp"));
*/					
//		return FALSE;
		}				
	}
	return TRUE;	

}

int32 CTexasPoker::CheckMySitPos(int32 PrePos){
	ePipeline ObjectList;

	//牌位识别
	static ePipeline NameList;
	if (NameList.Size()==0)
	{		
		NameList.PushString(_T("1号无牌"));
		NameList.PushString(_T("2号无牌"));
		NameList.PushString(_T("3号无牌"));
		NameList.PushString(_T("4号无牌"));
		NameList.PushString(_T("5号无牌"));
		NameList.PushString(_T("6号无牌"));
		NameList.PushString(_T("7号无牌"));
		NameList.PushString(_T("8号无牌"));
		NameList.PushString(_T("9号无牌"));
	}

	//确定自己座位
    int MySitPos = -1;
	
	if (PrePos != -1)
	{
		tstring Name = *(tstring*)NameList.GetData(PrePos-1);		
		ePipeline NameList1;
		NameList1.PushString(Name);
		if (!CaptureObjectSet(NameList1,ObjectList))
		{
			NotifyTip(_T("获取Pos Object失败"));
			return -1;
		};
	}else{		
		if (!CaptureObjectSet(NameList,ObjectList))
		{
			NotifyTip(_T("获取Pos Object失败"));
			return -1;
		};
	}
	
	for (int i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
		
		NewImage.AutoThreshold();
		
		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());
		
		float f = CompareBmp(SrcImage,NewImage);
		
		if (f<80.0f)
		{
			if(PrePos != -1)return PrePos;

			MySitPos = i+1;
			return MySitPos;
		}else{	
		/*
		SrcImage.Save(_T("MyPos_src.bmp"));
		CBitmapEx NewIamg1;
		NewIamg1.Load(hBitmap);
		NewIamg1.Save(_T("MyPos_dst.bmp"));
		NewImage.Save(_T("MyPos_xor.bmp"));
			*/		
		}				
	}
	return MySitPos;
}

BOOL CTexasPoker::CheckHasCard(tstring Name){
	
    int CardNum = 0;
	CObjectData Ob;
	
	if (!CaptureObject(Name,Ob))
	{
		NotifyTip(_T("获取card pos Object失败"));
		return -1;
	};
	
	uint32  handle = *(uint32*)Ob.GetData(9);
	HBITMAP hBitmap = (HBITMAP)handle;
	CBitmapEx NewImage;
	NewImage.Load(hBitmap);
	NewImage.AutoThreshold();
	
	ePipeline& SrcObject = m_PokerObjectList[Name];
	assert(SrcObject.Size()!=0);
	eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
	CBitmapEx SrcImage;
	SrcImage.Load((LPBYTE)Blob().c_str());
	
	float f = CompareBmp(SrcImage,NewImage);
	
	if (f>95.0f)
	{
		return FALSE;
	}else{
		return TRUE;
	}
	
}

BOOL    CTexasPoker::IsTaoJingSai(){
	ePipeline ObjectList;
	
	ePipeline NameList;
	
	NameList.PushString(_T("3号淘公牌无牌"));
	NameList.PushString(_T("4号淘公牌无牌"));
	NameList.PushString(_T("5号淘公牌无牌"));
	NameList.PushString(_T("6号淘公牌无牌"));
	NameList.PushString(_T("7号淘公牌无牌"));
	
	
	
	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取card pos Object失败"));
		return FALSE;
	};
	
	for (int32 i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
        NewImage.AutoThreshold();
		
		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());
        
		float f = CompareBmp(SrcImage,NewImage);
		
		if (f>95.0f)
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

void CTexasPoker::CaptureCardPosSign(){
	ePipeline ObjectList;
	ePipeline NameList;
	
	if (m_bTaojingSai)
	{
		NameList.PushString(_T("3号淘公牌无牌"));
		NameList.PushString(_T("4号淘公牌无牌"));
		NameList.PushString(_T("5号淘公牌无牌"));
		NameList.PushString(_T("6号淘公牌无牌"));
		NameList.PushString(_T("7号淘公牌无牌"));	
	} 
	else
	{
		NameList.PushString(_T("3号公牌无牌"));
		NameList.PushString(_T("4号公牌无牌"));
		NameList.PushString(_T("5号公牌无牌"));
		NameList.PushString(_T("6号公牌无牌"));
		NameList.PushString(_T("7号公牌无牌"));
		
	}
	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取card pos Object失败"));
		return ;
	};

	SendMsg(POKER_RESET_CARDPOS,ObjectList);
}
int32  CTexasPoker::CheckPlayerNum(){
    int PlayerNum = 9;
	ePipeline ObjectList;
	
	ePipeline NameList;
	
	NameList.PushString(_T("1号无人"));
	NameList.PushString(_T("2号无人"));
	NameList.PushString(_T("3号无人"));
	NameList.PushString(_T("4号无人"));
	NameList.PushString(_T("5号无人"));
	NameList.PushString(_T("6号无人"));
	NameList.PushString(_T("7号无人"));
	NameList.PushString(_T("8号无人"));
	NameList.PushString(_T("9号无人"));
	
	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取card pos Object失败"));
		return 0;
	};
	
	for (int32 i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
        NewImage.AutoThreshold();
		
		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());
        
		float f = CompareBmp(SrcImage,NewImage);
		
		if (f>95.0f)
		{
			PlayerNum--;
		}
	}
	
	return PlayerNum;
}

void CTexasPoker::FindBlankSitList(ePipeline& SitList){
	SitList.Clear();
	int PlayerNum = 9;
	ePipeline ObjectList;

	ePipeline NameList;

	NameList.PushString(_T("1号无人"));
	NameList.PushString(_T("2号无人"));
	NameList.PushString(_T("3号无人"));
	NameList.PushString(_T("4号无人"));
	NameList.PushString(_T("5号无人"));
	NameList.PushString(_T("6号无人"));
	NameList.PushString(_T("7号无人"));
	NameList.PushString(_T("8号无人"));
	NameList.PushString(_T("9号无人"));

	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取card pos Object失败"));
		return;
	};

	for (int32 i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32&  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
		NewImage.AutoThreshold();

		handle = 0;
		::DeleteObject(hBitmap);

		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());

		float f = CompareBmp(SrcImage,NewImage);

		if (f>95.0f)
		{
		   CObjectData* Object = new CObjectData;
		   Object->SetID(i+1);
		   *Object<<*ob;
		   SitList.Push_Directly(Object);
		}
	}
	return ;
}

int32 CTexasPoker::CheckOtherCardNum(){

    int CardNum = 0;
	ePipeline ObjectList;
	
	static ePipeline NameList;
	//公共牌识别
	if(NameList.Size()==0){		
		NameList.PushString(_T("3号公牌无牌"));
		NameList.PushString(_T("4号公牌无牌"));
		NameList.PushString(_T("5号公牌无牌"));
		NameList.PushString(_T("6号公牌无牌"));
		NameList.PushString(_T("7号公牌无牌"));
	}


	if (!CaptureObjectSet(NameList,ObjectList))
	{
		NotifyTip(_T("获取card pos Object失败"));
		return -1;
	};
	
	for (int32 i=0; i<ObjectList.Size(); i++)
	{
		ePipeline* ob = (ePipeline*)ObjectList.GetData(i);
		uint32  handle = *(uint32*) ob->GetData(9);
		HBITMAP hBitmap = (HBITMAP)handle;
		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
        NewImage.AutoThreshold();

		tstring& Name = *(tstring*)ob->GetData(1);
		ePipeline& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		eBLOB& Blob = *(eBLOB*)SrcObject.GetEnergy(9);
		CBitmapEx SrcImage;
		SrcImage.Load((LPBYTE)Blob().c_str());
        
		float f = CompareBmp(SrcImage,NewImage);
		
		if (f<70.0f)
		{
			CardNum ++;
		}else{
			break;
		}
	}

	return CardNum;
}



BOOL  CTexasPoker::CheckPoketCard(vector<int32>& MyCard,int32 SitPos){
	MyCard.clear();
	ePipeline ObjectList;	
	static ePipeline NameList;
	//手上牌
	if(NameList.Size()==0){		
		NameList.PushString(_T("1号牌1牌面"));//0
		NameList.PushString(_T("1号牌1花色"));//1
		NameList.PushString(_T("1号牌2牌面"));//2
		NameList.PushString(_T("1号牌2花色"));//3
		
		NameList.PushString(_T("2号牌1牌面"));//4
		NameList.PushString(_T("2号牌1花色"));//5
		NameList.PushString(_T("2号牌2牌面"));//6
		NameList.PushString(_T("2号牌2花色"));//7
		
		NameList.PushString(_T("3号牌1牌面"));//8
		NameList.PushString(_T("3号牌1花色"));//9
		NameList.PushString(_T("3号牌2牌面"));//10
		NameList.PushString(_T("3号牌2花色"));//11
		
		NameList.PushString(_T("4号牌1牌面"));//12
		NameList.PushString(_T("4号牌1花色"));//13
		NameList.PushString(_T("4号牌2牌面"));//14
		NameList.PushString(_T("4号牌2花色"));//15
		
		NameList.PushString(_T("5号牌1牌面"));//16
		NameList.PushString(_T("5号牌1花色"));//17
		NameList.PushString(_T("5号牌2牌面"));//18
		NameList.PushString(_T("5号牌2花色"));//19
		
		NameList.PushString(_T("6号牌1牌面"));//20
		NameList.PushString(_T("6号牌1花色"));//21
		NameList.PushString(_T("6号牌2牌面"));//22
		NameList.PushString(_T("6号牌2花色"));//23
		
		NameList.PushString(_T("7号牌1牌面"));//24
		NameList.PushString(_T("7号牌1花色"));//25
		NameList.PushString(_T("7号牌2牌面"));//26
		NameList.PushString(_T("7号牌2花色"));//27
		
		NameList.PushString(_T("8号牌1牌面"));//28
		NameList.PushString(_T("8号牌1花色"));//29
		NameList.PushString(_T("8号牌2牌面"));//30
		NameList.PushString(_T("8号牌2花色"));//31
		
		NameList.PushString(_T("9号牌1牌面"));//32
		NameList.PushString(_T("9号牌1花色"));//33
		NameList.PushString(_T("9号牌2牌面"));//34
		NameList.PushString(_T("9号牌2花色"));//35
	}
	
	int32 p = (SitPos-1)*4;
    
	ePipeline CardNameList;
	int i=0; 
	for (i=0; i<4; i++)
	{
		tstring Name = *(tstring*)NameList.GetData(p+i);
        CardNameList.PushString(Name);
	}

	if (!CaptureObjectSet(CardNameList,ObjectList))
	{
		NotifyTip(_T("获取hand card Object失败"));
		return FALSE;
	};
	
	
	for (i=0; i<4; i+=2)
	{
		ePipeline* ob1 = (ePipeline*)ObjectList.GetData(i);
		ePipeline* ob2 = (ePipeline*)ObjectList.GetData(i+1);
		
		tstring ocrStr = OCR(ob1,FALSE); //牌面	
		int32 FaceNum = GetCardFaceNum(ocrStr);	
		if (FaceNum == 0)
		{
			NotifyOCRFail(ob1);
			return FALSE;
		}else{
			NotifyOCROK(ob1);
		}
		
		ocrStr = OCR(ob2,FALSE);//花色
		int32 SuitNum = GetCardSuitNum(ocrStr);	
		if (SuitNum==-1)
		{			
			NotifyOCRFail(ob2);
			return FALSE;
		}else{
			NotifyOCROK(ob2);
		}
		
		int32 Card =SuitNum + FaceNum;
		MyCard.push_back(Card);
	}
	
	return TRUE;
};

BOOL  CTexasPoker::CheckOtherCard(vector<int32>& OtherCard){

	vector<int32> temp;
	
	static ePipeline NameList;
	//手上牌
	if(NameList.Size()==0){		
		NameList.PushString(_T("3号公牌无牌"));
		NameList.PushString(_T("3号淘公牌无牌"));	
		NameList.PushString(_T("公牌3牌面"));//0
		NameList.PushString(_T("公牌3花色"));//1
		NameList.PushString(_T("4号公牌无牌"));
		NameList.PushString(_T("4号淘公牌无牌"));
		NameList.PushString(_T("公牌4牌面"));//2
		NameList.PushString(_T("公牌4花色"));//3
		NameList.PushString(_T("5号公牌无牌"));
		NameList.PushString(_T("5号淘公牌无牌"));
		NameList.PushString(_T("公牌5牌面"));//4
		NameList.PushString(_T("公牌5花色"));//5
		NameList.PushString(_T("6号公牌无牌"));
		NameList.PushString(_T("6号淘公牌无牌"));
		NameList.PushString(_T("公牌6牌面"));//6
		NameList.PushString(_T("公牌6花色"));//7
		NameList.PushString(_T("7号公牌无牌"));
		NameList.PushString(_T("7号淘公牌无牌"));
		NameList.PushString(_T("公牌7牌面"));//8
		NameList.PushString(_T("公牌7花色"));//9
	}
	

	CObjectData ob;
	int32 FaceNum,SuitNum;
	for (int i=OtherCard.size()*2; i<NameList.Size(); i+=4)
	{
		tstring Name0;
		if(!m_bTaojingSai){
			Name0 = *(tstring*)NameList.GetData(i);
		}else{
			Name0 = *(tstring*)NameList.GetData(i+1);
		}
		//牌面
		tstring Name1 = *(tstring*)NameList.GetData(i+2);
		int n=2;
		while(n--){ //尝试2次

			if (!CheckHasCard(Name0)) //表示确实无牌
			{
				if (n==0)
				{
					return TRUE;
				}
				Sleep(500);
				UpdateScene();					
				continue;
			}
			
			if(!CaptureObject(Name1,ob)){
				return FALSE;
			}

			tstring ocrStr = OCR(&ob,FALSE); //牌面			
			FaceNum = GetCardFaceNum(ocrStr);	
			if (FaceNum == 0)
			{
				Sleep(500);
				UpdateScene();

			}else{
				break;
			}
		};
		
		if (FaceNum == 0)
		{			
			NotifyOCRFail(&ob);
			return FALSE;
		}else{
			NotifyOCROK(&ob);
		}
				
		//花色
		tstring Name2 = *(tstring*)NameList.GetData(i+3);	
		if(!CaptureObject(Name2,ob)){
			return FALSE;
		}
			
		tstring ocrStr = OCR(&ob,FALSE); //牌面			
		SuitNum = GetCardSuitNum(ocrStr);	
		
		if (SuitNum==-1)
		{			
			NotifyOCRFail(&ob);
			return FALSE;
		}else{
			NotifyOCROK(&ob);
		}
		
		int32 Card =SuitNum + FaceNum;
		OtherCard.push_back(Card);
	}
	
	return TRUE;
};

int32 CTexasPoker::CheckTotal()   //得到我的总分
{
	CObjectData ob;
	
	if (!CaptureObject(_T("总分值"),ob))
	{
		NotifyTip(_T("获取总分object失败"));
		return FALSE;
	};

    tstring ocrStr = OCR(&ob,FALSE);

	int64 Total = Str2Num(ocrStr);
    if (Total == 0)
    {
		NotifyOCRFail(&ob);
    }
	return Total;
}

int32 CTexasPoker::CheckMyRoll(int32 SitPos)  //得到我的筹码
{
    if(SitPos<1 || SitPos>9)return 0;

	static ePipeline NameList;
	//手上牌
	if(NameList.Size()==0){		
		NameList.PushString(_T("1号手上筹码"));
		NameList.PushString(_T("2号手上筹码"));
		NameList.PushString(_T("3号手上筹码"));
		NameList.PushString(_T("4号手上筹码"));
		NameList.PushString(_T("5号手上筹码"));
		NameList.PushString(_T("6号手上筹码"));
		NameList.PushString(_T("7号手上筹码"));
		NameList.PushString(_T("8号手上筹码"));
		NameList.PushString(_T("9号手上筹码"));
	}
	  
	tstring Name = *(tstring*)NameList.GetData(SitPos-1);
	CObjectData ob;
	if (!CaptureObject(Name,ob))
	{
		NotifyTip(_T("获取Other card Object失败"));
		return 0;
	};
		
    tstring ocrStr = OCR(&ob,FALSE);
    int64 Num = Str2Num(ocrStr);
    if (Num == 0)
    {
		NotifyOCRFail(&ob);
    }else{
        NotifyOCROK(&ob);
	}
	return Num;	
}

int32 CTexasPoker::CheckMinRoll() //得到最小盲注
{
	CObjectData ob;
	if (!CaptureObject(_T("小盲注"),ob))
	{
		NotifyTip(_T("获取小盲注object失败"));
		return FALSE;
	};
		
    tstring ocrStr = OCR(&ob,FALSE);
	int p=1;

	while(p<ocrStr.size() && ocrStr[p]!=_T('/') && ++p); //正向查找数字
	
	tstring s1 = ocrStr.substr(0,p);
	
	//p=ocrStr.size()-1; 
	//while(p>-1&&_istdigit(ocrStr[p] && p--)); //反向查找数字,可能忽略p=0的情况，但这时肯定已经无意义
	//tstring s2 = ocrStr.substr(p);
	
	int32 t1 = Str2Num(s1);
	//int32 t2 = Str2Num(s2);
	
	if(t1 !=0){
		NotifyOCROK(&ob);
		return t1/2;
	}else{
		NotifyOCRFail(&ob);
	}
	return 0;

}

int32 CTexasPoker::CheckBetRoll() //得到要求的下注
{
	CObjectData ob;
	if (!CaptureObject(_T("跟注额"),ob))
	{
		NotifyTip(_T("获取跟注额object失败"));
		return FALSE;
	};
		
    tstring ocrStr = OCR(&ob,FALSE);
    int64 Num = Str2Num(ocrStr);
    if (Num == 0)
    {
		NotifyOCRFail(&ob);
    }else {
		NotifyOCROK(&ob);
	}
	return Num;
}

BOOL CTexasPoker::PreTaskDo(){

	//初始化数据,打开指定页面
	tstring Weburl = m_PreTaskPipe.PopString();

	NotifyTip(Weburl.c_str());

	if (Weburl.find(_T("renren")) != tstring::npos)
	{
		m_PokerName = _T("renren");
	}else if(Weburl.find(_T("weibo"))!= tstring::npos){
		m_PokerName = _T("weibo");
	}else{
		return FALSE;
	}   
	ePipeline Letter;
	Letter.PushString(m_PokerName);
	Letter.PushString(Weburl);
	SendMsg(POKER_OPEN_WEB,Letter);
   
	ePipeline* Msg = WaitForFeedback(m_LastMsgTimeStamp,10);
	if(Msg == NULL ){
		return FALSE;	
	};
	
	tstring url = Msg->PopString();
	
	 
	//输入用户名和密码
	if (m_PreTaskPipe.Size() < 2)return TRUE;
	tstring UserName = m_PreTaskPipe.PopString();
	tstring Password = m_PreTaskPipe.PopString();
	Letter.Clear();
	Letter.PushString(UserName);
	
	map<tstring,ePipeline>::iterator it = m_PokerObjectList.find(_T("用户名框"));
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object = it->second;
	
	if (Object.Size() <10)
	{
		return  FALSE;
	}
	Letter.Push_Directly(Object.Clone());


	Letter.PushString(Password);
	it = m_PokerObjectList.find(_T("密码框"));
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object1 = it->second;
	
	if (Object1.Size() <10)
	{
		return  FALSE;
	}

	Letter.Push_Directly(Object1.Clone());
	
	it = m_PokerObjectList.find(_T("登录按钮"));
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object2 = it->second;
	
	if (Object2.Size() <10)
	{
		return  FALSE;
	}
	
	Letter.Push_Directly(Object2.Clone());
	
	SendMsg(POKER_LOGIN,Letter);
	
	Msg = WaitForFeedback(m_LastMsgTimeStamp,15);
	if(Msg == NULL){
		return FALSE;	
	};

	url = Msg->PopString();


	//确定游戏场和房间	
	Letter.Clear();
	if (m_PreTaskPipe.Size() < 2)return TRUE;

	//先确定是在大厅
	int i=0;
	for (i; i<10; i++)
	{
		UpdateScene();	
		if (HasLabel(_T("大厅")))
		{
			break;
		}
		Sleep(2000);
	}
	if (i==10)
	{
		return FALSE;
	}

	tstring RoomType = m_PreTaskPipe.PopString();
	it = m_PokerObjectList.find(RoomType);
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object3 = it->second;
	
	if (Object3.Size() <10)
	{
		return  FALSE;
	}
	//Letter.Push_Directly(Object3.Clone());
	ClickBnt(&Object3,FALSE); //先点击选定游戏场
	Sleep(1000);



	it = m_PokerObjectList.find(_T("牌场输入框"));
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object4 = it->second;
	
	if (Object4.Size() <10)
	{
		return  FALSE;
	}

	//Letter.Push_Directly(Object4.Clone());
	ClickBnt(&Object4,FALSE); //点击牌场输入框
	Sleep(500);

	tstring RoomIDStr   = m_PreTaskPipe.PopString();	
	//Letter.PushString(RoomIDStr);
	for (int i=0; i<RoomIDStr.size(); i++) //模拟输入每一个字符
	{
		tstring s;
		s =RoomIDStr[i];
        Letter.PushString(s);
		SendMsg(POKER_INPUT_TEXT,Letter);
		Sleep(500);
	}

	it = m_PokerObjectList.find(_T("牌场选择"));
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	ePipeline& Object5 = it->second;
	
	if (Object5.Size() <10)
	{
		return  FALSE;
	}
	ePipeline Object55 = Object5;
	ClickBnt(&Object5,FALSE); //点击选定的牌场
	ClickBnt(&Object55,FALSE); //点击选定的牌场

	Sleep(500);
	//Letter.Push_Directly(Object5.Clone());

//	SendMsg(POKER_SELECT_ROOM,Letter);

	Msg = WaitForFeedback(m_LastMsgTimeStamp,10);
	if(Msg == NULL){
		return FALSE;	
	};

	return TRUE;
};

bool CTexasPoker::Do(Energy* E){
    m_LockPipe.Clear();

	m_Option.GetValue(PLAY_DEBUG,m_bDebug);

    if (m_PreTaskPipe.Size())
    {
		BOOL ret = PreTaskDo();
		if (!ret)
		{
			NotifyTip(_T("自动登录，请人工登录"));
			m_Alive = FALSE;
			ePipeline Letter;
			SendMsg(POKER_GAME_OVER,Letter);
			return FALSE;
		}
    }

	Init();

/*
	CBitmapEx Scene;
	Scene.Load(m_Scene);
    Scene.Save(_T("Scene.bmp"));
*/
//	TestZoomClick();
//	TestOtherCardPos();

	m_CardList.clear();
	for (int i=0; i<10 && m_Alive; i++) //重复10次进入游戏大厅
	{		

		UpdateScene();

		if (HasLabel(_T("大厅")))
		{
			if (!JoinGame(_T("游戏场")))
			{
				Sleep(2000);
				continue;
			}
		};

		UpdateScene();

		if(!HasLabel(_T("牌场")))
		{
			NotifyTip(_T("未能检测到游戏大厅或房间"));
			Sleep(2000);
			continue;
		}

		NotifyTip(_T("游戏启动"));


		CBitmapEx qipai,qipai1,kuaisukaishi;

		ePipeline& Ob1 = m_PokerObjectList[_T("弃牌")];
		assert(Ob1.Size()>9);
		eBLOB& Blob1 = *(eBLOB*)Ob1.GetEnergy(9);
		qipai.Load((LPBYTE)Blob1().c_str());

/*		ePipeline& Ob2 = m_PokerObjectList[_T("弃牌(黄)")];
		assert(Ob2.Size()>9);
		eBLOB& Blob2 = *(eBLOB*)Ob2.GetEnergy(9);
		qipai1.Load((LPBYTE)Blob2().c_str());
*/		
		ePipeline& Ob3 = m_PokerObjectList[_T("快速开始")];
		assert(Ob3.Size()>9);
		eBLOB& Blob3 = *(eBLOB*)Ob3.GetEnergy(9);
		kuaisukaishi.Load((LPBYTE)Blob3().c_str());
		

		CObjectData QiPaiObject;
		while(m_Alive)
		{	            
			NotifyTip(_T("等待新一轮..."));

			do{
				UpdateScene();
				if (!WaitObject(qipai,_T("弃牌"),QiPaiObject)
					//&&!WaitObject(qipai1,_T("弃牌(黄)"),QiPaiObject)
					)
				{
					CObjectData ZhiDongObject;
					if (WaitObject(kuaisukaishi,_T("快速开始"),ZhiDongObject)) 
					{
						ClickBnt(&ZhiDongObject,TRUE);  //自动选座位
						Sleep(3000);
						CObjectData CloseBnt;	
						if(CaptureObject(_T("关闭买分弹出窗口"),CloseBnt)){
							ClickBnt(&CloseBnt,TRUE);  
						}

						LingFen();

					}else{
						Sleep(500);
						//CutTime =  AbstractSpace::CreateTimeStamp();
					}

				}else{
					NotifyTip(_T("开始玩牌"));
					break;
				}
				
				BOOL bPause = IsPause();
				while(m_Alive && IsPause()){
					Sleep(20);
				}
				if(bPause){ //如果暂停过，那么所有牌的检测重头开始
					m_CardList.clear();
				}
			}while(m_Alive /*(&& CutTime<EndTime*/);

			if(!m_Alive ){
				break;
			}

			assert(QiPaiObject.Size());

			if (m_CardList.size()==7)
			{
				m_CardList.clear();
			};

			BOOL ret = PlayGame();
			if (!ret)
			{
				ClickBnt(&QiPaiObject,TRUE);
				m_CardList.clear();

				//得到手上筹码
				int32 MyHandRoll = CheckMyRoll(m_SitPos);

				//如果赢的分大于一定数目，则站起来保存，之后会重新坐下
				if(!m_bBisai && MyHandRoll>20000 && MyHandRoll>m_MinBet*400){
					CObjectData BntObject;
					if(CaptureObject(_T("站起"),BntObject)){
						ClickBnt(&BntObject,FALSE);
						NotifyTip(_T("已存分"));	
					}
				}
				
				LingFen();
			}				
						
			BOOL bPause = IsPause();
			while(m_Alive && IsPause()){
				Sleep(20);
			}
			if(bPause){
				m_CardList.clear();
			}

		}//while
	}
	m_Alive = FALSE;
	ePipeline Letter;
	SendMsg(POKER_GAME_OVER,Letter);
	return FALSE;
}

void   CTexasPoker::LingFen(){
	
	if (!m_bLingFen)
	{
		return ;
	}
	CObjectData BntObject;
	/*
	int64 CurTime = AbstractSpace::CreateTimeStamp();
	if(m_LingfenDaoQiTime !=0 ){
		int64 sec = (CurTime-m_LastLingFenTime)/10/1000/1000;
        assert(sec<1000);

		tstring s = tformat(_T("领分预计时间：%d 当前：%d"),m_LingfenDaoQiTime,sec);
		NotifyTip(s);
		if(sec<m_LingfenDaoQiTime)return;
	}

	CaptureObject(_T("领分"),BntObject);
	ClickBnt(&BntObject,FALSE);
	Sleep(1000);
	
	UpdateScene();
	
	CaptureObject(_T("领分时间"),BntObject);
	tstring ocrStr = OCR(&BntObject,FALSE); 
	m_LingfenDaoQiTime = Str2Time(ocrStr);

	if(m_LingfenDaoQiTime>5000){
		m_MainFrame->FlushWindow();
	}

	m_LastLingFenTime = AbstractSpace::CreateTimeStamp();

	if (m_LingfenDaoQiTime == 0)
	{
		NotifyOCRFail(&BntObject);
		return ;
	}else{
		NotifyOCROK(&BntObject);
	}
	m_LastLingFenTime = CurTime;
	
	*/

	if(HasLabel(_T("领分"),BntObject))
	{
		ClickBnt(&BntObject,FALSE);
		Sleep(2000);
		CaptureObject(_T("关闭领分"),BntObject);
		ClickBnt(&BntObject,FALSE);
	}; 
	Sleep(500);
}

void CTexasPoker::AllIn(){
	CObjectData ALLBnt;
	CObjectData BntObject;

	tstring Action;

	CaptureObject(_T("ALL牌"),ALLBnt);
	ClickBnt(&ALLBnt,FALSE);

	if( HasLabel(_T("加注"),BntObject) 
		//||HasLabel(_T("加注(黄)"),BntObject)
		){

			Sleep(500);
			ClickBnt(&BntObject,TRUE);

			Action += _T("ALL牌(加注）");
			NotifyTip(Action);

			m_CardList.clear();
	}
	else
	{ 			
		CaptureObject(_T("跟注"),BntObject);
		Sleep(500);
		ClickBnt(&BntObject,TRUE);
		m_CardList.clear();

		Action += _T("ALL牌");
		NotifyTip(Action);
	}
}
BOOL CTexasPoker::PlayShuaPeng(BOOL bActivity){
	
	if (bActivity)
	{
		CObjectData BntObject;

		AllIn();

		UpdateScene();
		while(m_Alive && HasLabel(_T("3号公牌无牌"),BntObject))
		{
			Sleep(200);
			UpdateScene();
		}; 
		
		if(!m_Alive){
			return TRUE;
		};

		if(CaptureObject(_T("站起"),BntObject)){
			ClickBnt(&BntObject,FALSE);
		}

		ePipeline SitList;
			
		int SitNum = -1;
		do{
			Sleep(500);			
			UpdateScene();
			FindBlankSitList(SitList);
			SitNum = SitList.Size();	
		}while(m_Alive && SitNum <8);

		if (!m_Alive)
		{
			return TRUE;
		}

		CObjectData* Ob = NULL;
		int32 NewSit=0;
		if (m_SitPos==-1 || m_SitPos==9)
		{
			Ob = (CObjectData*)SitList.GetData(0);

			m_SitPos = Ob->GetID();

			CObjectData* SitOb = (CObjectData*)Ob->Clone();
			eElectron e(SitOb);
			ClickBnt(SitOb,FALSE);	
		}else{ 
			//寻找当前座位的下一个座位
	
			//CObjectData* Ob= NULL;
			for(int i=0; i<SitList.Size();i++){
				Ob = (CObjectData*)SitList.GetData(i);
				if (Ob->GetID()>m_SitPos)
				{
					break;
				}
			}

			if (Ob == NULL)
			{
				Ob = (CObjectData*)SitList.GetData(0);
				m_SitPos = -1;
			}else{
				m_SitPos = Ob->GetID();
			}
			CObjectData* SitOb = (CObjectData*)Ob->Clone();
			eElectron e(SitOb);
			ClickBnt(SitOb,FALSE);	
			
		}
		
		Sleep(500);
		UpdateScene();		
		while(m_Alive && !HasLabel(_T("买分窗口标志"),BntObject))
		{
			Sleep(200);
			UpdateScene();
				
			CObjectData* SitOb = (CObjectData*)Ob->Clone();
			eElectron e(SitOb);
			ClickBnt(SitOb,FALSE);	
			
			/*
			if(CaptureObject(_T("站起"),BntObject)){
				ClickBnt(BntObject,FALSE);
			}
			*/
		}; 

		if (!m_Alive)
		{
			return TRUE;
		}
		
		CaptureObject(_T("最小买入值"),BntObject);
		ClickBnt(&BntObject,TRUE);	
		CaptureObject(_T("最小买入值"),BntObject);
		ClickBnt(&BntObject,FALSE);	

	}else{
		CObjectData BntObject;
		CaptureObject(_T("看牌"),BntObject);
		ClickBnt(&BntObject,TRUE);
	}
	return TRUE;
}

BOOL CTexasPoker::PlayNormal(PLAY_FACTOR_0 t){

	int32 Total = 0;
	int32 OtherCardNum = 0;
	vector<int32>  OtherCard; 

	Sleep(m_WaitPokerTime); //1秒时间翻牌

	//确定座位
	UpdateScene();
	int MySitPos = CheckMySitPos(m_SitPos);	
	int n=5;
	while (MySitPos == -1 && n-->0)
	{
		Sleep(m_WaitPokerTime);
		UpdateScene();
		MySitPos = CheckMySitPos();	
	}
	
	if(MySitPos ==-1){
		NotifyTip(_T("确认本人座位失败"));
		return FALSE;
	};

	//Sleep(m_WaitPokerTime);

	//确定桌上牌数,两轮都一样则视为真实
	/*
	for (int i=0; i<3; i++)
	{
		int OtherCardNum1 = CheckOtherCardNum();
		Sleep(1000);
		int OtherCardNum2 = CheckOtherCardNum();
		if (OtherCardNum1 == OtherCardNum2)
		{
			OtherCardNum = OtherCardNum1;
			break;
		}
		Sleep(1000);
        UpdateScene();  
	}
*/

	//确定手上两张牌
	vector<int32> SelfCard;
	if (OtherCardNum<=m_CardList.size()-2) //还没有底牌或只有底牌,只有底牌也要重新识别，因为可能是之前一局的残余结果
	{
		m_CardList.clear();
		
		n=5;
		while(!CheckPoketCard(SelfCard,MySitPos) && n-->0)
		{
			Sleep(m_WaitPokerTime*2);
			UpdateScene();
		}
		
		if (SelfCard.size()!=2)
		{
			tstring s = tformat(_T("底牌确认失败"));
			NotifyTip(s);
			return FALSE;
		}
		m_CardList.push_back(SelfCard[0]);
		m_CardList.push_back(SelfCard[1]);
		
	} 
	else
	{
		assert(m_CardList.size()>1);
		SelfCard.push_back(m_CardList[0]);
		SelfCard.push_back(m_CardList[1]);
		
	}

	int32 p = m_CardList.size()-2;
    for(int i=0; i<p;i++){
		OtherCard.push_back(m_CardList[i+2]);	//已经检测到的旧牌
	}			

	//当前总分
	Total = CheckTotal();
    if (Total == 0)
    {
		if (m_CurTotal!=0)
		{
			tstring s = tformat(_T("总分确认失败,使用之前总分"));
			NotifyTip(s);
			Total = m_CurTotal;
		} 
		else
		{
			tstring s = tformat(_T("总分确认失败"));
			NotifyTip(s);
			return FALSE;
		}
    }

	if (m_StartTotal==0 && Total)
	{
		m_StartTotal = Total;
	}

    //得到当前盲注
    int32  BaseRoll = CheckMinRoll();   
    if (BaseRoll == 0)
    {	
		if (m_MinBet !=0)
		{
			BaseRoll = m_MinBet;
			tstring s = tformat(_T("小盲注确认失败,使用之前结果"));
			NotifyTip(s);

		}else{
			tstring s = tformat(_T("小盲注确认失败"));
			NotifyTip(s);
			return FALSE;
		}
	}
	m_MinBet = BaseRoll;	
	m_SitPos = MySitPos;
	m_CurTotal = Total;

	
	tstring s = tformat(_T("总分:%d  已赢:%d  座位:%d  小盲注:%d "),Total,Total-m_StartTotal,MySitPos,BaseRoll);
	NotifyTip(s);

	//得到公共牌
	if (!CheckOtherCard(OtherCard))
	{
		tstring s = tformat(_T("公共牌确认失败"));
		NotifyTip(s);
		return FALSE;
		
	}else{
		tstring s = tformat(_T("公共牌数：%d"),OtherCard.size());
		NotifyTip(s);
		
		for (int i=m_CardList.size()-2; i<OtherCard.size();i++)
		{
			m_CardList.push_back(OtherCard[i]);
		}
	}

	//得到风险因子,计算可以下注
	tstring Memo;
	float32 f = CheckHand(SelfCard,OtherCard,Memo);
				
	NotifCard(Memo,SelfCard,OtherCard);
	
	tstring Action =_T("操作:");

	//f=10.0f; //test

	//决定如何下注
	if (t==PF_ACTIVE)
	{
		if (f>=9.0f)
		{
			f=10.0f;
		}
	}else if (t==PF_PASSIVE)
	{
		if (f<5.5)
		{
			f=4.9;
		}
	}

	if (f > 9.8f)
	{
		AllIn();
		return TRUE;
	}else if(f>1.0f){
		do{
			CObjectData BntObject;
			if (
			//	HasLabel(_T("看牌(黄)"),BntObject,90.0f) ||  //看牌不要钱，好坏都看
				HasLabel(_T("看牌",90.0f),BntObject)
				)
			{   

				ClickBnt(&BntObject,TRUE);

				Action += _T("看牌");
				NotifyTip(Action);

				return TRUE;
			}
			else if(
				//HasLabel(_T("跟注(黄)"),BntObject,90.0f) ||
				HasLabel(_T("跟注"),BntObject,90.0f)
				)
			{   	

				NotifyTip(_T("检测到跟注...."));


				//得到手上筹码
				int32 MyHandRoll = CheckMyRoll(MySitPos);
				if (MyHandRoll == 0)
				{ 
					MyHandRoll = m_HandRoll;

					tstring s = _T("手上筹码获取错误，使用之前结果");
					NotifyTip(s);				
				}

				Action += tformat(_T("手上筹码=%d "),MyHandRoll);		

				//得到要求的跟注大小
				int32  BetNum = CheckBetRoll();	
				if (BetNum == 0)
				{
					Action += _T("跟注获取错误，弃牌");
					NotifyTip(Action);
					return FALSE;
				}        

				//根据风险计算出可以下注的金额
				f=1+f-5;
				int32 Roll = f*f*f*BaseRoll*2;  

				if (BetNum <= Roll) //如果要求的金额小于可以下注的，则跟
				{
					int32 n = Roll - BetNum;
					/*暂时不考虑主动加注
					if (n != BaseRoll*2) //应该适当加注
					{
					CObjectData AddBnt;
					CObjectData AddBox;
					if (
					CaptureObject(_T("加注"),AddBnt,FALSE)&&
					CaptureObject(_T("加注输入框"),AddBox,FALSE))
					{

					}
					}else{
					ClickBnt(&BetBnt);
					Memo += _T(" 跟牌");
					}
					*/

					ClickBnt(&BntObject,TRUE);

					Action += tformat(_T("跟注=%d"),BetNum);
					NotifyTip(Action);

					return TRUE;		
				}else{
					Action += tformat(_T("要求下注:%d > 计划下注:%d，弃牌"),BetNum,Roll);
					NotifyTip(Action);
					return FALSE;
				}				
			}
		}while(m_Alive);
	/*	
		else{
			if (m_bDebug)
			{
				CBitmapEx Scene;
				Scene.Load(m_Scene);
				tstring FileName = m_TempDir+_T("ERROR_Scene.bmp");
				Scene.Save((TCHAR*)FileName.c_str());
			}

			Action += _T("没有发现看牌或跟注，弃牌");
			NotifyTip(Action);

			return FALSE;
		}
		*/
	}else{
		Action += _T("直接弃牌");
		NotifyTip(Action);		
		return FALSE;
	}

	return TRUE;
}

BOOL CTexasPoker::PlayGame(){

    tstring PlayType;
	BOOL ret = m_Option.GetValue(PLAY_TYPE,PlayType);
    assert(ret);

	m_Option.GetValue(PLAY_DEBUG,m_bDebug);
	m_Option.GetValue(PLAY_LINGFEN,m_bLingFen);


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

    if (PlayType ==_T("1 自由场"))
    {
		m_bBisai = FALSE;
		m_bTaojingSai = FALSE;
		return PlayNormal(pf);
    }
	else if(PlayType == _T("2 比赛")){
		m_bBisai = TRUE;
		m_bTaojingSai = FALSE;
        return PlayNormal(pf);
	}
	else if (PlayType == _T("3 淘金赛"))
	{
		m_bBisai = TRUE;
		m_bTaojingSai = TRUE;
		return PlayNormal(pf);
		
	}else if (PlayType == _T("4 刷盆"))
	{
		return PlayShuaPeng(FALSE);
	}
	else if (PlayType == _T("5 二人刷盆"))
	{
		return PlayShuaPeng(TRUE);		
	}
	return TRUE;
};



