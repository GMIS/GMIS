#include <winsock2.h>
#include "MainFrame.h"
#include "MsgList.h"
#include "UserLinkerPipe.h"
#include "format.h"
#include "Win32Tool.h"
#include "hightime.h"
#include "BrainObject.h"
//tool 
//////////////////////////////////////////////////////////////////////////

tstring GetCurrentDir(){
	static tstring Dir;
	if (Dir.size()==0)
	{
#ifdef _WINDLL
		HMODULE hHandle = (HMODULE)GetDLLHandle();
#else
		HMODULE hHandle = (HMODULE)GetModuleHandle(NULL);  
#endif
		TCHAR buf[512];
		::GetModuleFileName(   
			hHandle,         
			buf,       
			512                 
			);   

		Dir = buf;
		int n = Dir.find_last_of(_T("\\"),tstring::npos);
		if(n!=-1){
			Dir=Dir.substr(0,n);
		}
	}
	return Dir;
};

BOOL  CMainFrame::CaptureObject(tstring ObjectName,CObjectData& ob){
	ob.Reset();

	map<tstring,CObjectData>::iterator it = m_PokerObjectList.find(ObjectName);
	if(it == m_PokerObjectList.end()){	 
		return FALSE;
	}
	CObjectData& Object = it->second;

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

	x += m_BasePos.x;
	y += m_BasePos.y;

	HBITMAP hBitmap =CaptureImageFromBmp(m_WebHost.m_WebView.GetHwnd(),m_Scene,x,y,w,h);

	uint32 handle = (uint32)hBitmap;
	ob.PushInt(handle);

	return TRUE;
}



void CMainFrame::CaptureCardPosSign(){
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


	for (int i=0; i<NameList.Size(); i++)
	{
		tstring ObjectName = *(tstring*)NameList.GetData(i);
		CObjectData ObjectData;
		if (!CaptureObject(ObjectName,ObjectData))
		{
			NotifyTip(_T("获取公牌无牌失败"));
			return ;
		};


		//注意：由于共用数据，调用次序必须保证数据不被破坏
		UpdateObjectImage(ObjectData,FALSE);  //hBitmap1被m_ResultView所占有，但后面的还可以取得图像句柄

	}	
}


CObjectData*  CMainFrame::FindObject(tstring ObjectName){
	map<tstring,CObjectData>::iterator it = m_PokerObjectList.find(ObjectName);
	if(it == m_PokerObjectList.end()){	 
		return NULL;
	}
	CObjectData& Object = it->second;
	return &Object;
};


void CMainFrame::UpdateObjectImage(CObjectData& ObjectData,BOOL bBaseImage){

	//设置更新时间
	int64 TimeStamp = AbstractSpace::CreateTimeStamp();
	ObjectData.GetLabel() = ObjectData.GetTimer()->GetYMDHMS(TimeStamp);		

	HBITMAP hBitmap  = ObjectData.GetImage();

	CBitmapEx Image;
	Image.Load(hBitmap);


	uint32 crcValue = 0;

	if (!bBaseImage)
	{
		//Crc32Bitmap()会转换图像为黑白图
		crcValue = Crc32Bitmap(Image);
	}

	HBITMAP hBitmap1 = NULL;
	Image.Save(hBitmap1);	
	ObjectData.SetImage(hBitmap1);


	//会复制图像数据，但不影响原来图像句柄
	int64 ObjectID = *(int64*)ObjectData.GetData(0);

	if(m_CurObjectView.GetHwnd()){
		SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_SET,ObjectID,(int64)&ObjectData);
	}else{
		m_CurObjectView.OnSetObject(ObjectID,(int64)&ObjectData);
	}
	m_PokerDB.SaveObject(ObjectData);		


	tstring Name = *(tstring*)ObjectData.GetData(1);
	CObjectData& Object = m_PokerObjectList[Name];
	Object.Reset();
	Object << ObjectData;
}

void  CMainFrame::UpdateScene(){
	SendChildMessage(GetHwnd(),WEB_GET_SCENE,0,0);

	if(!m_Scene){
		NotifyTip(_T("没找到游戏页面"));
		return;
	}

	if (m_bDebug)
	{
		CBitmapEx Image;
		Image.Load(m_Scene);
		tstring Name = m_TempDir+_T("Scene.bmp");
		Image.Save((TCHAR*)Name.c_str());
	}

	BOOL ret = CalculateBaseImagePos(m_Scene);
	if (!ret)
	{
		NotifyTip(_T("游戏页面识别有误，将重新尝试"));
	}
}


BOOL  CMainFrame::CalculateBaseImagePos(HBITMAP Image){
	if(Image==NULL){
		return false;
	}


	if (!m_BaseImage.IsValid())
	{
		map<tstring,CObjectData>::iterator it = m_PokerObjectList.find(_T("坐标基准"));
		if(it == m_PokerObjectList.end()){	 
			return FALSE;
		}
		CObjectData& Object = it->second;
		if(Object.Size()<10)return FALSE;

		m_BasePos.x = *(int32*)Object.GetData(3);
		m_BasePos.y = *(int32*)Object.GetData(4);

		HBITMAP hBitmap = Object.GetImage();
		m_BaseImage.Load(hBitmap);
		//m_BaseImage.AutoThreshold();
	}


	int32 w = m_BaseImage.GetWidth(); 
	int32 h = m_BaseImage.GetHeight();

	//优先按现有偏移点查找	

	HBITMAP hDest = CaptureImageFromBmp(GetHwnd(),m_Scene,m_BasePos.x,m_BasePos.y,w,h);

	CBitmapEx  DestImage;
	DestImage.Load(hDest);
	//	DestImage.AutoThreshold();

	float f = CompareBmp(m_BaseImage,DestImage);

	//m_BaseImage.Save(_T("BaseImage.bmp"));
	//DestImage.Save(_T("BaseImage1.bmp"));

	::DeleteObject(hDest);

	if (f == 100.0f)
	{
		return  TRUE;
	}

	//否则按现有基准点周围正负100的范围寻找
	int32 d = 200;
	int x1 = m_BasePos.x-d>-1? m_BasePos.x-d:0;
	int y1 =  m_BasePos.y-d>-1? m_BasePos.y-d:0;
	int x2 = m_BasePos.x+d;
	int y2 = m_BasePos.y+d;

	for (y1; y1<y2; y1++)
	{
		for (x1=0; x1<x2; x1++)
		{
			HBITMAP hDest = CaptureImageFromBmp(m_WebHost.m_WebView.GetHwnd(),m_Scene,x1,y1,w,h);

			CBitmapEx  DestImage;
			DestImage.Load(hDest);

			float f = CompareBmp(m_BaseImage,DestImage);

			::DeleteObject(hDest);

			if (f == 100.0f)
			{
				m_BasePos.x = x1;
				m_BasePos.y = y1;
				return  TRUE;
			}
		}
	}
	return FALSE;		
}


int32  CMainFrame::Str2Num(tstring& str){
	tstring numStr;
	for(int i=0; i<str.size(); i++)
	{
		TCHAR ch = str[i];
		if (_istdigit(ch))
		{
			numStr+=ch;
		}
	}

	int32  n =_ttoi(numStr.c_str());
	return n;
}

int32  CMainFrame::Str2Time(tstring& str){
	if(str.size()<6)return 0;

	tstring s1 = str.substr(0,2);
	int32  n1 =_ttoi(s1.c_str());
	tstring s2  = str.substr(3,2);
	int32  n2 =_ttoi(s1.c_str());	
	return n1*60+n2;
}


tstring CMainFrame::OCR(ePipeline* Object,BOOL IsThresholded)
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
	ocrtxt = GetOcredObject(crcValue);
	if (ocrtxt.size())
	{
		return ocrtxt;
	}

	switch(Type){
	case NUM_OBJECT:
		{
			if (ObjectName == _T("自动加注额"))
			{
				ocrtxt = OcrObject(Image,TRUE,FALSE,"num");
			}else{
				ocrtxt = OcrObject(Image,FALSE,FALSE,"num");
			}
		}
		break;
	case TEXT_OBJECT:
		{
			if (ObjectName.find(_T("花色")) != -1)
			{
				ocrtxt = OcrObject(Image,TRUE,TRUE,"poker");	

			}else{
				ocrtxt = OcrObject(Image,TRUE,FALSE,"poker");	
			}
		}
		break;
	case BUTTON_OBJECT:
		{
			//bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"chi_sim");
			//assert(ret);

			Image.Save((TCHAR*)m_OcrTxtTempFileName.c_str());
			AnsiString s = m_Tesseract->OCR(m_OcrTxtTempFileName);
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


tstring CMainFrame::OCR(tstring ImageFileName,AnsiString LanName)
{	
	assert(ImageFileName.size()!=0);

	CTesseract  Tess;

	Tess.SetLang(m_TessDataDir.c_str(),LanName.c_str());

	AnsiString s = Tess.OCR(ImageFileName);

	tstring ocrTxt;	
	ocrTxt = eSTRING::UTF8TO16(s);

	return ocrTxt;
}


void CMainFrame::RegisterOcredObject(uint32 Crc32,tstring& OcrText){
	m_Mutex.Acquire();
	m_OcrObjectList[Crc32] = OcrText;
	m_Mutex.Release();
}

tstring CMainFrame::GetOcredObject(uint32 Crc32){
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

tstring CMainFrame::OcrObject(CBitmapEx& Image,BOOL bNegative,BOOL bPokerSuit,const char* lang){
	
	if(bNegative){
		Image.Negative();
	}

	if (m_bSaveOcrImag)
	{
		int64 Alias = AbstractSpace::CreateTimeStamp();
		tstring FileName = m_TempDir+tformat(_T("ocr-%I64ld.bmp"),Alias);
		Image.Save((TCHAR*)FileName.c_str());
	}

	bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),lang);
	assert(ret);

	//扫描图像去除文字之前的空白
	int32  iHeight = Image.GetHeight();
	int32  iWidth  = Image.GetWidth();

	if (iWidth>100)
	{
		//Image.Save((TCHAR*)m_OcrNumTempFileName.c_str());
		AnsiString s = m_Tesseract->OCR(Image);
		tstring ocrtxt;
		ocrtxt = eSTRING::UTF8TO16(s);	
		return ocrtxt;
	};


	//纵向扫描得到图像的范围（也就是去除两边空白）
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

	int32 x2=iWidth;
	for (x2; x2>0; x2--)
	{
		int32 t=0;
		for (int32 y=0; y<iHeight; y++)
		{
			_PIXEL px = Image.GetPixel(x2, y);
			unsigned char cr = px;
			t += cr;			
		}
		if (t != k)//判断一列是否全是白色
		{ 
			break;
		}
	}	

	int32 w = x2-x1;
	if (w<0)
	{
		w=0;
	};

	HDC hDC = GetDC(GetHwnd());

	HDC DCMem = ::CreateCompatibleDC(hDC);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hDC, 240,50);
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	RECT rc = {0,0,240,50};
	FillRect(DCMem,&rc,RGB(255,255,255));

	if (bPokerSuit)
	{
		HFONT OldFont = (HFONT)::SelectObject(DCMem,m_Font);
		COLORREF Oldcr = ::SetTextColor(DCMem,RGB(0,0,0));
		::DrawText(DCMem,_T("88"),2,&rc,DT_LEFT);	
		::SetTextColor(DCMem,Oldcr);
		::SelectObject(DCMem,OldFont);
	}

	//::BitBlt(DCMem, 35, 8, Image.GetWidth(),Image.GetHeight(), DCMem, 0, 0, SRCCOPY );

	CBitmapEx TempImag;
	TempImag.Load(bmpCanvas);

	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	ReleaseDC(GetHwnd(),hDC);

	//CBitmapEx TempImag;

	//TempImag.Create(180,50);
	//TempImag.Clear(RGB(255,255,255));
	//TempImag.DrawTextA(2,2,"88",_RGB(0,0,0),0,_T("Arial"),16);
	
	
	int n=1;
	if(iWidth<20)
	{
		n=6;
	}
	else if (iWidth<32)
	{
		n=4;
	}else if (iWidth<50)
	{
		n=2;
	}

	int dx=4;
	
	int x = 24;
	for (int i=0; i<n; i++)
	{	x+=dx;	
		TempImag.Draw(x,2,w,iHeight,Image,x1,0,w,iHeight);
		x  += w;
	}

	//TempImag.Save((TCHAR*)m_OcrNumTempFileName.c_str());
	//AnsiString s = m_Tesseract->OCR(m_OcrNumTempFileName);
 	
	AnsiString s = m_Tesseract->OCR(TempImag);
	tstring ocrtxt;

	ocrtxt = eSTRING::UTF8TO16(s);	

	tstring str;
	tstring::iterator it = ocrtxt.begin();
	while(it!=ocrtxt.end()){
		TCHAR ch = *it;
		if(ch!=_T(' ') && ch != _T('\n')){
			str+=ch;
		}
		it++;
	}
	if (str.size()>=n)
	{
		if (bPokerSuit)
		{
			tstring::size_type p = str.find(_T("88"));
			if (p != tstring::npos)
			{
				ocrtxt = str.substr(p+2);
				int t = ocrtxt.size()/n;
				ocrtxt = ocrtxt.substr(0,t);
			}else{
				int t = str.size()/n;
				ocrtxt = str.substr(0,t);
			}	
		}else{

			int t = str.size()/n;
			ocrtxt = str.substr(0,t);
		}
	}else {
		ocrtxt = _T("");
	}

	return ocrtxt;
}

tstring CMainFrame::OcrNumObject(CBitmapEx& Image){

	bool ret = m_Tesseract->SetLang(m_TessDataDir.c_str(),"poker");
	assert(ret);
	Image.Negative();
	Image.Scale(200,200);

	HDC hDC = GetDC(GetHwnd());

	HDC DCMem = ::CreateCompatibleDC(hDC);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hDC, 120,30);
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	RECT rc = {0,0,120,30};
	FillRect(DCMem,&rc,RGB(255,255,255));

	HFONT OldFont = (HFONT)::SelectObject(DCMem,m_Font);
	COLORREF Oldcr = ::SetTextColor(DCMem,RGB(0,0,0));
	::DrawText(DCMem,_T("88"),2,&rc,DT_LEFT);	
	::SetTextColor(DCMem,Oldcr);
	::SelectObject(DCMem,OldFont);

	//::BitBlt(DCMem, 35, 8, Image.GetWidth(),Image.GetHeight(), DCMem, 0, 0, SRCCOPY );
	
	CBitmapEx TempImag;
	TempImag.Load(bmpCanvas);

	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	ReleaseDC(GetHwnd(),hDC);

	//TempImag.DrawTextA(hDC,0,0,"8 8",_RGB(0,0,0),255,_T("Arial"),16,TRUE);
	//TempImag.Draw(0,0,m_MaskNumImage.GetWidth(),m_MaskNumImage.GetHeight(),m_MaskNumImage,0,0);	
	TempImag.Draw(24,0,Image.GetWidth(),Image.GetHeight(),Image,0,0);

	TempImag.Save((TCHAR*)m_OcrNumTempFileName.c_str());


	if (m_bSaveOcrImag)
	{
		int64 Alias = AbstractSpace::CreateTimeStamp();
		tstring FileName = m_TempDir+tformat(_T("numocr-%I64ld.bmp"),Alias);
		TempImag.Save((TCHAR*)FileName.c_str());
	}

	AnsiString s = m_Tesseract->OCR(m_OcrNumTempFileName);
	tstring ocrtxt;

	ocrtxt = eSTRING::UTF8TO16(s);	
	if (ocrtxt.size()>2)
	{
		ocrtxt = ocrtxt.substr(2);
	}else {
		ocrtxt = _T("");
	}

	return ocrtxt;
	/*
	//return  ocrtxt; 

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
	TempImag.Clear(RGB(255,255,255));
	TempImag.DrawTextW(5,5,_T("23"),0,0,_T("Arial"),14);
	//TempImag.Draw(0,0,m_MaskNumImage.GetWidth(),m_MaskNumImage.GetHeight(),m_MaskNumImage,0,0);	
	TempImag.Draw(30,0,w,iHeight,Image,x1,0,w,iHeight);

	TempImag.Save((TCHAR*)m_OcrNumTempFileName.c_str());

	s = m_Tesseract->OCR(m_OcrNumTempFileName);	

	ocrtxt = eSTRING::UTF8TO16(s);	
	if (ocrtxt.size()>2)
	{
		ocrtxt = ocrtxt.substr(2);
	}else {
		ocrtxt = _T("");
	}
	return ocrtxt;	
	*/
}

void CMainFrame::NotifyTip(tstring Tip){
	if (m_bDebug || m_bSaveDebugToLog)
	{
		AnsiString s = eSTRING::UTF16TO8(Tip);
		m_Log.PrintLn(s);
	}

	ePipeline Letter;
	Letter.PushString(Tip);	
	m_ResultView.AddInfo(&Letter);

}

void CMainFrame::NotifyOCRFail(ePipeline* ob){
	
#ifdef _WINDLL
	return;
#endif
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
	tstring s = tformat(_T("%s 识别失败（将再次尝试）"),Name.c_str());
	ePipeline Msg;
	Msg.PushString(s);
	m_ResultView.AddInfo(&Msg);

#ifdef _DEBUG
	HBITMAP hBitmap = (HBITMAP)handle;
	m_OcrView.AddItem(Name,hBitmap,crc32,_T(""));

	//m_PokerDB.SaveObject(Name,crc32,_T(""),(HBITMAP)Handle);	
	handle = 0;
#endif
}

void CMainFrame::NotifyOCROK(ePipeline* ob){
#ifdef _WINDLL
	return;
#endif

#ifdef _DEBUG
	tstring&  Name = *(tstring*)ob->GetData(1);
	uint32   crc32 = *(uint32*)ob->GetData(7);
	tstring ocrtxt = *(tstring*)ob->GetData(8);
	uint32& handle = *(uint32*)ob->GetData(9);

	HBITMAP hbitmap = (HBITMAP)handle;

	//显示识别结果
	m_OcrView.AddItem(Name,hbitmap,crc32,ocrtxt);       	

	//记录CRC32下次直接识别
	//m_PokerDB.SaveObject(Name,crc32,ocrtxt,hbitmap);
	handle = 0;
#endif
}


void CMainFrame::NotifCard(tstring& Tip,vector<int32>& MyPorker,vector<int32>& OtherPorker){


	ePipeline Letter;
	Letter.PushString(Tip);

	ePipeline CardList;
	int i=0; 
	for (i=0; i<MyPorker.size();i++)
	{
		int32 card = MyPorker[i];
		CardList.PushInt(card);
	}

	for (i=0; i<OtherPorker.size();i++)
	{
		int32 card = OtherPorker[i];
		CardList.PushInt(card);
	}

	if (m_bDebug || m_bSaveDebugToLog)
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
	m_ResultView.AddInfo(&Letter);
}

void CMainFrame::FindBlankSitList(ePipeline& SitList){
	SitList.Clear();
	int PlayerNum = 9;

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



	for (int32 i=0; i<NameList.Size(); i++)
	{
		tstring Name = *(tstring*)NameList.GetData(0);

		CObjectData ObjectData;
		if (!CaptureObject(Name,ObjectData))
		{
			NotifyTip(_T("寻找空座位失败"));
			return;
		};

		HBITMAP hBitmap = ObjectData.GetImage();

		CBitmapEx NewImage;
		NewImage.Load(hBitmap);
		NewImage.AutoThreshold();


		CObjectData& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);

		CBitmapEx SrcImage;
		SrcImage.Load(SrcObject.GetImage());

		float f = CompareBmp(SrcImage,NewImage);

		if (f>95.0f)
		{
			CObjectData* Object = new CObjectData;
			Object->SetID(i+1);
			*Object<<ObjectData;
			SitList.Push_Directly(Object);
		}
	}
	return ;
}

void CMainFrame::ClickBnt(ePipeline* ObjectData){
	SendChildMessage(GetHwnd(),WEB_CLICK_OBJECT,(int64)ObjectData,0);
}

void CMainFrame::ClickPoint(ePipeline& Point)
{
	SendChildMessage(GetHwnd(),WEB_CLICK_POINT,(int64)&Point,0);
}

BOOL CMainFrame::HasLabel(tstring Name,CObjectData& ob, float32 f,CBitmapEx* SrcImage)
{

	if(!CaptureObject(Name,ob)){
		return FALSE;
	}

	HBITMAP hBitmap = ob.GetImage();
	if (hBitmap==NULL)
	{
		return FALSE;
	}

	CBitmapEx NewImage;
	NewImage.Load(hBitmap);
	NewImage.AutoThreshold();
	
	CBitmapEx SrcTempImage;
	if (SrcImage==NULL)
	{
		CObjectData& SrcObject = m_PokerObjectList[Name];

		if (SrcObject.Size()<10)
		{
			return FALSE;
		}

		SrcTempImage.Load(SrcObject.GetImage());
		SrcImage = &SrcTempImage;
	}

	

	if (m_bDebug)
	{
		tstring FileName = m_TempDir+tformat(_T("%s_Label_dst.bmp"),Name.c_str());
		NewImage.Save((TCHAR*)FileName.c_str());
	}


	float f0= CompareBmp(*SrcImage,NewImage);

	if(m_bDebug){
		tstring s = tformat(_T("HasLabel %s: f=%f"),Name.c_str(),f0);
		NotifyTip(s);
	};

	if (f0 > f)
	{
		return TRUE;
	}else if(m_bDebug){

		tstring FileName = m_TempDir+tformat(_T("%s_Label_xor.bmp"),Name.c_str());
		NewImage.Save((TCHAR*)FileName.c_str());		

		FileName = m_TempDir+tformat(_T("%s_Label_src.bmp"),Name.c_str());
		SrcImage->Save((TCHAR*)FileName.c_str());        
	}

	return FALSE;
}

BOOL CMainFrame::HasLabel(tstring Name,float32 f,CBitmapEx* SrcImage){
	CObjectData ob;
	BOOL ret = HasLabel(Name,ob,f,SrcImage);
	return ret;
}
void CMainFrame::WebInputText(tstring& text){
	for (int i=0; i<text.size(); i++)
	{
	    HWND hWnd = m_WebHost.GetHandle();
		if (hWnd==NULL)
		{
			m_WebHost.SubclassWebWnd();
		}
		::SendMessage(hWnd,WM_CHAR,text[i],0);
	}
}



int32  CMainFrame::CheckPlayerNum(){
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



	for (int32 i=0; i<NameList.Size(); i++)
	{
		tstring Name = *(tstring*)NameList.GetData(i);

		CObjectData ObjectData;
		if (!CaptureObject(Name,ObjectData))
		{
			NotifyTip(_T("获取玩家人数失败"));
			return 0;
		};

		CBitmapEx NewImage;
		NewImage.Load(ObjectData.GetImage());
		NewImage.AutoThreshold();

		CObjectData& SrcObject = m_PokerObjectList[Name];

		CBitmapEx SrcImage;
		SrcImage.Load(SrcObject.GetImage());

		float f = CompareBmp(SrcImage,NewImage);

		if (f>95.0f)
		{
			PlayerNum--;
		}
	}

	return PlayerNum;
}

/*

int32 CMainFrame::CheckMySitPos(int32 PrePos){

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

	for (int i=0; i<NameList.Size(); i++)
	{
		tstring Name;
		CObjectData ObjectData;
		if (PrePos != -1)
		{
			Name = *(tstring*)NameList.GetData(PrePos-1);		

			if (!CaptureObject(Name,ObjectData))
			{
				NotifyTip(_T("获取座位位置失败"));
				return -1;
			};
			i=NameList.Size(); //不再检查其它位置
		}else{
			Name = *(tstring*)NameList.GetData(i);
			if (!CaptureObject(Name,ObjectData))
			{
				NotifyTip(_T("获取座位位置失败"));
				return -1;
			};
		}

		CBitmapEx NewImage;
		NewImage.Load(ObjectData.GetImage());

		NewImage.AutoThreshold();

		CObjectData& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);

		CBitmapEx SrcImage;
		SrcImage.Load(SrcObject.GetImage());

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
/*		}				
	}
	return MySitPos;
}
*/
int32 CMainFrame::CheckMySitPos(int32 PrePos){
	static ePipeline NameList;
	//手上牌
	if(NameList.Size()==0){		
		NameList.PushString(_T("1号牌1牌面"));
		NameList.PushString(_T("1号牌1花色"));

		NameList.PushString(_T("2号牌1牌面"));
		NameList.PushString(_T("2号牌1花色"));

		NameList.PushString(_T("3号牌1牌面"));
		NameList.PushString(_T("3号牌1花色"));

		NameList.PushString(_T("4号牌1牌面"));
		NameList.PushString(_T("4号牌1花色"));

		NameList.PushString(_T("5号牌1牌面"));
		NameList.PushString(_T("5号牌1花色"));

		NameList.PushString(_T("6号牌1牌面"));
		NameList.PushString(_T("6号牌1花色"));

		NameList.PushString(_T("7号牌1牌面"));
		NameList.PushString(_T("7号牌1花色"));

		NameList.PushString(_T("8号牌1牌面"));
		NameList.PushString(_T("8号牌1花色"));

		NameList.PushString(_T("9号牌1牌面"));
		NameList.PushString(_T("9号牌1花色"));
	}

	UpdateScene();

	CObjectData ob1,ob2;
	tstring ocrStr;
	int n=2;
	if(PrePos != -1 && m_CardType != -1){
		tstring Name1 = *(tstring*)NameList.GetData(PrePos);
		tstring Name2 = *(tstring*)NameList.GetData(PrePos+1);
		if (m_CardType==0)
		{
			Name1 += _T("0");
			Name2 += _T("0");
		}
		while (n--)
		{
			if (!CaptureObject(Name1,ob1) || !CaptureObject(Name2,ob2)){
				return -1;
			}
				
			ocrStr = OCR(&ob1,FALSE);//牌面
			int32 FaceNum = GetCardFaceNum(ocrStr);
			if(FaceNum !=0){
				ocrStr = OCR(&ob2,FALSE);//花色
				int32 SuitNum = GetCardSuitNum(ocrStr);	
				if (SuitNum !=-1){
					return PrePos;
				}
			}
				
			Sleep(500);
			UpdateScene();
		}

	}

	m_SeatPos = -1;

	n=2;
	while(n--){
		for(int i=0; i<NameList.Size()/2; i++)
		{
			int32 index1 = i*2;
			
			if (m_CardType==0)
			{
				tstring Name1 = *(tstring*)NameList.GetData(index1);
				tstring Name2 = *(tstring*)NameList.GetData(index1+1);
				
				Name1 += _T("0");
				Name2 += _T("0");

				if (!CaptureObject(Name1,ob1) || !CaptureObject(Name2,ob2)){
					return -1;
				}
				
				ocrStr = OCR(&ob1,FALSE);//牌面
				int32 FaceNum = GetCardFaceNum(ocrStr);
				if(FaceNum !=0){
					ocrStr = OCR(&ob2,FALSE);//花色
					int32 SuitNum = GetCardSuitNum(ocrStr);	
					if (SuitNum !=-1){
						return i+1;
					}
				}

			}else if (m_CardType==1)
			{
				tstring Name1 = *(tstring*)NameList.GetData(index1);
				tstring Name2 = *(tstring*)NameList.GetData(index1+1);

				if (!CaptureObject(Name1,ob1) || !CaptureObject(Name2,ob2)){
					return -1;
				}

				ocrStr = OCR(&ob1,FALSE);//牌面
				int32 FaceNum = GetCardFaceNum(ocrStr);
				if(FaceNum !=0){
					ocrStr = OCR(&ob2,FALSE);//花色
					int32 SuitNum = GetCardSuitNum(ocrStr);	
					if (SuitNum !=-1){
						return i+1;
					}
				}
			}else{ //m_CardType == -1;
				tstring Name1 = *(tstring*)NameList.GetData(index1);
				tstring Name2 = *(tstring*)NameList.GetData(index1+1);
	
				//优先测试小图案牌

				tstring Name11 = Name1+_T("0");
				tstring Name22 = Name2+_T("0");
				if (!CaptureObject(Name11,ob1) || !CaptureObject(Name22,ob2)){
					return -1;
				}

				ocrStr = OCR(&ob1,FALSE);//牌面
				int32 FaceNum = GetCardFaceNum(ocrStr);
				if(FaceNum !=0){
					ocrStr = OCR(&ob2,FALSE);//花色
					int32 SuitNum = GetCardSuitNum(ocrStr);	
					if (SuitNum !=-1){
						m_CardType = 0;
						return i+1;
					}
				}

				if (!CaptureObject(Name1,ob1) || !CaptureObject(Name2,ob2)){
					return -1;
				}

				ocrStr = OCR(&ob1,FALSE);//牌面
				FaceNum = GetCardFaceNum(ocrStr);
				if(FaceNum !=0){
					ocrStr = OCR(&ob2,FALSE);//花色
					int32 SuitNum = GetCardSuitNum(ocrStr);	
					if (SuitNum !=-1){
						m_CardType = 1;
						return i+1;
					}
				}

		
			}
		}
		Sleep(1000);
		UpdateScene();
	}

	return -1;
}

BOOL CMainFrame::CheckHasCard(tstring Name){

	int CardNum = 0;
	CObjectData Ob;

	if (!CaptureObject(Name,Ob))
	{
		NotifyTip(_T("获取失败"));
		return 0;
	};

	CBitmapEx NewImage;
	NewImage.Load(Ob.GetImage());
	NewImage.AutoThreshold();

	CObjectData& SrcObject = m_PokerObjectList[Name];
	assert(SrcObject.Size()!=0);

	CBitmapEx SrcImage;
	SrcImage.Load(SrcObject.GetImage());

	float f = CompareBmp(SrcImage,NewImage);

	if (f>95.0f)
	{
		return FALSE;
	}else if ((m_SeatPos==7||m_SeatPos==3) && f>50.0f) //这两个位置的图像会被动态阴影干扰，所以降低识别度
	{
		return FALSE;
	}
	{
		return TRUE;
	}

}


int32 CMainFrame::CheckOtherCardNum(){

	int CardNum = 0;

	static ePipeline NameList;

	//公共牌识别
	if(NameList.Size()==0){		
		NameList.PushString(_T("3号公牌无牌"));
		NameList.PushString(_T("4号公牌无牌"));
		NameList.PushString(_T("5号公牌无牌"));
		NameList.PushString(_T("6号公牌无牌"));
		NameList.PushString(_T("7号公牌无牌"));
	}



	for (int32 i=0; i<NameList.Size(); i++)
	{
		tstring Name = *(tstring*)NameList.GetData(i);

		CObjectData ObjectData;
		if (!CaptureObject(Name,ObjectData))
		{
			NotifyTip(_T("获取公共牌失败"));
			return -1;
		};

		CBitmapEx NewImage;
		NewImage.Load(ObjectData.GetImage());
		NewImage.AutoThreshold();

		CObjectData& SrcObject = m_PokerObjectList[Name];
		assert(SrcObject.Size()!=0);
		CBitmapEx SrcImage;
		SrcImage.Load(SrcObject.GetImage());

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

BOOL  CMainFrame::GetSelfCard(vector<int32>& MyCard){
	if(m_SeatPos ==-1 || m_CardType == -1){
		return FALSE;
	}

	MyCard.clear();
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



	int32 p = (m_SeatPos-1)*4;
    
	tstring Name1 = *(tstring*)NameList.GetData(p);
	tstring Name2 = *(tstring*)NameList.GetData(p+1);
	tstring Name3 = *(tstring*)NameList.GetData(p+2);
	tstring Name4 = *(tstring*)NameList.GetData(p+3);

	if (m_CardType==0) //小字版
	{
		Name1 +=_T("0");
		Name2 +=_T("0");
		Name3 +=_T("0");
		Name4 +=_T("0");
	}

	CObjectData ob1,ob2,ob3,ob4;

	if (
		!CaptureObject(Name1,ob1) ||
		!CaptureObject(Name2,ob2) ||
		!CaptureObject(Name3,ob3) ||
		!CaptureObject(Name4,ob4) 
		)
	{
		return FALSE;
	};

	//牌1
	tstring ocrStr = OCR(&ob1,FALSE); //牌面	
	int32 FaceNum = GetCardFaceNum(ocrStr);	
	if (FaceNum == 0)
	{
		NotifyOCRFail(&ob1);
		return FALSE;
	}else{
		NotifyOCROK(&ob1);
	}

	ocrStr = OCR(&ob2,FALSE);//花色
	int32 SuitNum = GetCardSuitNum(ocrStr);	
	if (SuitNum==-1)
	{	
		NotifyOCRFail(&ob2);
		return FALSE;
	}else{
		NotifyOCROK(&ob2);
	}


	int32 Card =SuitNum + FaceNum;
	MyCard.push_back(Card);

	//牌2
	ocrStr = OCR(&ob3,FALSE); //牌面	
	FaceNum = GetCardFaceNum(ocrStr);	
	if (FaceNum == 0)
	{
		NotifyOCRFail(&ob3);
		return FALSE;
	}else{
		NotifyOCROK(&ob3);
	}

	ocrStr = OCR(&ob4,FALSE);//花色
	SuitNum = GetCardSuitNum(ocrStr);	
	if (SuitNum == -1)
	{		
		NotifyOCRFail(&ob4);
		return FALSE;
	}else{
		NotifyOCROK(&ob4);
	}

	Card =SuitNum + FaceNum;
	MyCard.push_back(Card);

	assert(MyCard.size()==2);
	return TRUE;
};

/*
BOOL  CMainFrame::CheckOtherCard(vector<int32>& OtherCard){

	vector<int32> temp;

	static ePipeline NameList;
	//手上牌
	if(NameList.Size()==0){		
		NameList.PushString(_T("3号公牌无牌"));
		NameList.PushString(_T("3号淘公牌无牌"));	
		NameList.PushString(_T("公牌3牌面"));
		NameList.PushString(_T("公牌3花色"));
		NameList.PushString(_T("4号公牌无牌"));
		NameList.PushString(_T("4号淘公牌无牌"));
		NameList.PushString(_T("公牌4牌面"));
		NameList.PushString(_T("公牌4花色"));
		NameList.PushString(_T("5号公牌无牌"));
		NameList.PushString(_T("5号淘公牌无牌"));
		NameList.PushString(_T("公牌5牌面"));
		NameList.PushString(_T("公牌5花色"));
		NameList.PushString(_T("6号公牌无牌"));
		NameList.PushString(_T("6号淘公牌无牌"));
		NameList.PushString(_T("公牌6牌面"));
		NameList.PushString(_T("公牌6花色"));
		NameList.PushString(_T("7号公牌无牌"));
		NameList.PushString(_T("7号淘公牌无牌"));
		NameList.PushString(_T("公牌7牌面"));//8
		NameList.PushString(_T("公牌7花色"));//9
	}


	CObjectData ob;
	int32 FaceNum,SuitNum;

	int i= OtherCard.size()*4;

	int end = NameList.Size();
	if (i==0)
	{
		end = 3*4;
	}else if (i==3)
	{
		end = 4*4;
	}

	for (i; i<end; i+=4)
	{
		tstring Name0;
		if(!m_bTaojingSai){
			Name0 = *(tstring*)NameList.GetData(i);
		}else{
			Name0 = *(tstring*)NameList.GetData(i+1);
		}
		int n=2;
		while(n--){ //尝试2次

			if (!CheckHasCard(Name0)) //表示无牌
			{
				if (n==0)
				{
					return TRUE;
				}
				Sleep(500);
				UpdateScene();					
				continue;
			}else{
				break;
			}
		};
		n=2;
REPEAT:
		//牌面
		tstring Name1 = *(tstring*)NameList.GetData(i+2);
		if(!CaptureObject(Name1,ob)){
			NotifyTip(_T("获取公共牌牌面失败"));
			return FALSE;
		}

		tstring ocrStr = OCR(&ob,FALSE); //牌面			
		FaceNum = GetCardFaceNum(ocrStr);	
		if (FaceNum == 0 && n--==2)
		{
			Sleep(500);
			UpdateScene();
			goto REPEAT;
		}

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
			NotifyTip(_T("获取公共牌花色失败"));
			return FALSE;
		}

		ocrStr = OCR(&ob,FALSE); //花色
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
*/
BOOL  CMainFrame::CheckOtherCard(vector<int32>& OtherCard){

	vector<int32> temp;

	static ePipeline NameList;
	//手上牌
	if(NameList.Size()==0){		
		NameList.PushString(_T("公牌3花色"));//0
		NameList.PushString(_T("公牌3牌面"));//1
		NameList.PushString(_T("公牌4花色"));//2
		NameList.PushString(_T("公牌4牌面"));//3
		NameList.PushString(_T("公牌5花色"));//4
		NameList.PushString(_T("公牌5牌面"));//5
		NameList.PushString(_T("公牌6花色"));//6
		NameList.PushString(_T("公牌6牌面"));//7
		NameList.PushString(_T("公牌7花色"));//8
		NameList.PushString(_T("公牌7牌面"));//9
	}


	CObjectData ob;
	int32 FaceNum,SuitNum;

	int i= OtherCard.size()*2;

	int end = NameList.Size();
	
	if (OtherCard.size()==3)
	{
		end = 4*2;
	}

	for (i; i<end; i+=2)
	{

		//花色	
		int n=2;
        while (n--)
        {
			tstring Name1 = *(tstring*)NameList.GetData(i);	
			if (m_CardType==0) //小字版
			{
				Name1 += _T("0");
			}
			if(!CaptureObject(Name1,ob)){
				NotifyTip(_T("获取公共牌花色失败"));
				return FALSE;
			}

			tstring ocrStr = OCR(&ob,FALSE); //花色
			SuitNum = GetCardSuitNum(ocrStr);	
			
			if (SuitNum==-1 )
			{	
				Sleep(500);
				UpdateScene();
				continue ;
			}else{
				NotifyOCROK(&ob);
				break;
			}
        }

		if (SuitNum == -1)
		{
		    if(!(OtherCard.size()==0 || OtherCard.size()>2)){
				return FALSE;
			}
			return TRUE;  //只要有牌，花色识别一般不会出错，此时表明公共牌识别完成
		}

		//牌面
		tstring Name2 = *(tstring*)NameList.GetData(i+1);
			
		if (m_CardType==0) //小字版
		{
			Name2 += _T("0");
		}

		if(!CaptureObject(Name2,ob)){
			NotifyTip(_T("获取公共牌牌面失败"));
			return FALSE;
		}

		tstring ocrStr = OCR(&ob,FALSE); //牌面			
		FaceNum = GetCardFaceNum(ocrStr);	
		if (FaceNum == 0)
		{			
			NotifyOCRFail(&ob);
			return FALSE;
		}else{
			NotifyOCROK(&ob);
		}

		int32 Card =SuitNum + FaceNum;
		OtherCard.push_back(Card);
	}

	if(OtherCard.size()==0 || OtherCard.size()==3 || 
		OtherCard.size()==4||OtherCard.size()==5){
		return TRUE;
	}
	return FALSE;
};

tstring CMainFrame::CheckRoomID(){
	CObjectData ob;
	
	if (!CaptureObject(_T("牌场ID"),ob))
	{
		NotifyTip(_T("获取牌场ID失败"));
		return FALSE;
	};
	
	tstring ocrStr = OCR(&ob,FALSE);

	tstring numStr;
	for(int i=0; i<ocrStr.size(); i++)
	{
		TCHAR ch = ocrStr[i];
		if (_istdigit(ch))
		{
			numStr+=ch;
		}else{
			break;
		}
	}
	return numStr;
}

int CMainFrame::CheckBlankSeat(){

	ePipeline ObjectList;

	static ePipeline NameList;

	if (NameList.Size()==0)
	{
		NameList.PushString(_T("1号无人"));
		NameList.PushString(_T("2号无人"));
		NameList.PushString(_T("3号无人"));
		NameList.PushString(_T("4号无人"));
		NameList.PushString(_T("5号无人"));
		NameList.PushString(_T("6号无人"));
		NameList.PushString(_T("7号无人"));
		NameList.PushString(_T("8号无人"));
		NameList.PushString(_T("9号无人"));
	}

	int n=0;
	for (int32 i=0; i<NameList.Size(); i++)
	{
		tstring Name = *(tstring*)NameList.GetData(i);

		CObjectData ObjectData;
		if (!CaptureObject(Name,ObjectData))
		{
			NotifyTip(_T("获取玩家人数失败"));
			return 0;
		};

		CBitmapEx NewImage;
		NewImage.Load(ObjectData.GetImage());
		NewImage.AutoThreshold();

		CObjectData& SrcObject = m_PokerObjectList[Name];

		CBitmapEx SrcImage;
		SrcImage.Load(SrcObject.GetImage());

		float f = CompareBmp(SrcImage,NewImage);

		if (f>80.0f)
		{
			n++;
		}
	}

	return n;
}

int32 CMainFrame::CheckTotal()   //得到我的总分
{
	CObjectData ob;

	if (!CaptureObject(_T("总分值"),ob))
	{
		NotifyTip(_T("获取总分失败"));
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

int32 CMainFrame::CheckMyRoll(int32 SitPos)  //得到我的筹码
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
		NotifyTip(_T("获取手上筹码失败"));
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

int32 CMainFrame::CheckMinRoll() //得到最小盲注
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
		return t1;
	}else{
		NotifyOCRFail(&ob);
	}
	return 0;

}

int32 CMainFrame::CheckBetRoll() //得到要求的下注
{
	CObjectData ob;
	if (!CaptureObject(_T("跟注额"),ob))
	{
		NotifyTip(_T("获取跟注额失败"));
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

float32 CMainFrame::CheckHand(vector<int32>& MyPoker, vector<int32>& OtherPoker,tstring& Memo){

	assert(MyPoker.size()==2);
	assert(OtherPoker.size()==0 ||OtherPoker.size()>2);

	ePipeline Pipe;
	int i=0;
	for (i=0; i<MyPoker.size(); i++)
	{
		int t = MyPoker[i];
		Pipe.PushInt(t);
	}
	for (i=0; i<OtherPoker.size(); i++)
	{
		int t = OtherPoker[i];
		Pipe.PushInt(t);
	}

	float32 f = m_TexasPokerEvaluate.CheckHand(MyPoker,OtherPoker,Memo);

	return f;
};

//本地玩牌
//////////////////////////////////////////////////////////////////////////

bool CMainFrame::InitPokerGame(){


	/*
	tstring url = m_WebHost.GetUrl();

	AnsiString AnsiPokerName;
	tstring    tPokerName;
	if (url.find(_T("renren")) != tstring::npos)
	{
		AnsiPokerName ="renren";
		tPokerName = _T("renren");
	}else if(url.find(_T("weibo"))!= tstring::npos){
		AnsiPokerName = "weibo";
		tPokerName = _T("weibo");
	}else {
		tstring s = _T("Poker data init fail: 当前不在游戏页面");
		m_Status.SetTip(s);

		m_Status.StopTask();
		return false;
	}

	if (m_PokerObjectList.size()>0 && m_PokerName == tPokerName )
	{
	return true;
	}

	m_PokerName = tPokerName;

	*/

	if (m_PokerObjectList.size()>0){
		return true;
	} 

	tstring dir = GetCurrentDir();

	m_TessDataDir = eSTRING::UTF16TO8(dir);
	m_TessDataDir +="\\";

	/*
	tstring filename = dir + _T("\\tessdata\\masknum.bmp");
	m_MaskNumImage.Load((LPTSTR)filename.c_str());

	filename = dir + _T("\\tessdata\\maskcard.bmp");
	m_MaskPokerImage.Load((LPTSTR)filename.c_str()); 

	if (!m_MaskNumImage.IsValid())
	{
		tstring s = _T("Poker data init fail: MaskNumImage lost");
		m_Status.SetTip(s);
		return false;
	}

	if (!m_MaskPokerImage.IsValid())
	{
		tstring s = _T("Poker data init fail: MaskPokerImage lost");
		m_Status.SetTip(s);

		return false;
	}
	*/
	m_TempDir =dir+_T("\\temp\\");

	_tmkdir(m_TempDir.c_str());

	tstring LogFileName = m_TempDir+tformat(_T("Poker.log"));
	m_Log.Open(LogFileName.c_str());


	AnsiString DBFile = m_TessDataDir+ "PokerData.db";

	if(!m_PokerDB.Open(DBFile)){
		tstring s = _T("Poker database open error");
		m_Status.SetTip(s);
		return false;		
	};

	vector<CObjectData>  ObjectList;


#ifdef _WINDLL
	m_PokerDB.GetObjectList(ObjectList);
#else
	#ifdef _DEBUG
	tstring InitFile = dir + _T("\\PokerRobot.ini");
	m_PokerDB.GetObjectList(InitFile,ObjectList);	
	#else
	m_PokerDB.GetObjectList(ObjectList);
	#endif
#endif 

	if (ObjectList.size()==0)
	{
		tstring s = _T("Poker data init fail: PokerRobot.ini lost");
		m_Status.SetTip(s);

		return false;
	}

	ePipeline Crc32List;
	m_PokerDB.GetCrc32List(Crc32List);

	int64 TimeStamp = CreateTimeStamp();

	m_OcrNumTempFileName = m_TempDir+tformat(_T("txtocr-%I64ld.bmp"),TimeStamp);
	m_OcrTxtTempFileName = m_TempDir+tformat(_T("numocr-%I64ld.bmp"),TimeStamp);


#ifndef _WINDLL

#ifndef DEBUG
	HANDLE hFile = m_Log.GetHandle();
	assert(hFile);

	FILETIME  ft1,ft2,ft3; //分别为创建时间，上一次访问时间，上一次修改时间
	BOOL ret = GetFileTime(hFile,&ft1,&ft2,&ft3); 
	//::CloseHandle(hFile);

	if(!ret){
		int error = ::GetLastError();
		tstring s = _T("Poker log init fail: Log.txt lost");
		m_Status.SetTip(s);
		return false;
	}

	SYSTEMTIME st1;
	::GetSystemTime(&st1);

	CHighTime t1=ft2;
	CHighTime t2=st1;
	CHighTime t3;
	t3.SetDate(2014,11,30);


	if (t2<=t1 || t2>t3)
	{	
		tstring s = _T("Poker log init fail: 时间过期");
		m_Status.SetTip(s);
		return false;
	}
#endif
#endif

	//	DeleteDir(dir.c_str());
	//	m_Log.Open(LogFileName.c_str());

	int32 n = ObjectList.size();

	//删除OBJECT窗口的条目
	if(m_CurObjectView.GetHwnd()){
		SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_CLR,0,0);	
	}else{
		m_CurObjectView.OnClearAll(0,0);	
	}

	m_PokerObjectList.clear();

	for (int i=0; i<n; i++)
	{
		CObjectData& ObjectData = ObjectList[i];

		if(ObjectData.Size()==0)continue;

		int64     ID = *(int64*)ObjectData.GetData(0);
		tstring Name = *(tstring*)ObjectData.GetData(1);
		int32   Type = *(int32*)ObjectData.GetData(2);


		ObjectItem* Item = new ObjectItem(ID,Name,(OBJECTTYPE)Type,NULL,0,_T(""));
		if(m_CurObjectView.GetHwnd()){
			SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_ADD,(int64)Item,0);
		}else{
			m_CurObjectView.OnAddObject((int64)Item,0);
		}

		if (ObjectData.Size()>3)
		{
			if(m_CurObjectView.GetHwnd()){
				SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_SET,ID,(int64)&ObjectData);
			}else{
				m_CurObjectView.OnSetObject(ID,(int64)&ObjectData);
			}

		}

		CObjectData& Object = m_PokerObjectList[Name];
		Object << ObjectData;

		float32 per = (float32)i/(float32)n*100;
		m_Status.SetProgressPer((int32)per);
	}
	m_Status.SetProgressPer(0);


	while (Crc32List.Size())
	{
		uint32 Crc32 = Crc32List.PopInt();
		tstring ocrStr = Crc32List.PopString();
		RegisterOcredObject(Crc32,ocrStr);
	}

	/*
	//需要被人工识别的
	int p=0;
	for(int i=0; i<Pipe.Size();i++)
	{
	uint32 crc32 = *(uint32*)Pipe.GetData(i);
	tstring ocrStr = *(tstring*)Pipe.GetData(++i);
	if (ocrStr.size()==0)
	{
	HBITMAP hBitmap;
	tstring Name;
	m_PokerDB.GetObject(Name,crc32,ocrStr,hBitmap);
	m_OcrView.AddItem(Name,hBitmap,crc32,ocrStr);
	}
	}
	*/
	tstring s = _T("Poker data init ok!");
	m_Status.SetTip(s);
	//	m_Status.SetConnect(TRUE);
	return true;
}



BOOL CMainFrame::SelectPlayType(tstring GameType){

	CObjectData& Ob1 = m_PokerObjectList[GameType];

	ClickBnt(&Ob1);
	ClickBnt(&Ob1);

	Sleep(2000);

	UpdateScene();

	if(!HasLabel(GameType))
	{
		NotifyTip(_T("游戏类型未选定"));
		return FALSE;
	}else{
		tstring tip = tformat(_T("已经选定：%"),GameType.c_str());
		NotifyTip(tip);
	}

	return TRUE;
};


BOOL CMainFrame::Kuaisuxunzhaozuowei()
{
	
	CObjectData ob2;
	if(CaptureObject(_T("快速寻找座位"),ob2))
	{	
		NotifyTip(_T("快速寻找座位"));
		
		ClickBnt(&ob2);
		Sleep(8000);
		UpdateScene();
		return TRUE;
	}else{
		NotifyTip(_T("快速寻找座位失败"));
	}
	return FALSE;
}

BOOL  CMainFrame::IsPause(){
	m_Mutex.Acquire();
	BOOL ret = m_bPause;
	m_Mutex.Release();
	return ret;
};


bool CMainFrame::Activation(){
#ifdef _WINDLL
	m_Alive = TRUE;
	return true;
#else 
	return CUserObject::Activation();
#endif

}


void CMainFrame::Dead(){
	m_Alive = FALSE;
	m_bPause = FALSE;
	CUserObject::Dead();
}



void CMainFrame::ChangeAllObjectPos(int32 dx, int32 dy)
{
	map<tstring,CObjectData>::iterator it = m_PokerObjectList.begin();
	while(it != m_PokerObjectList.end()){
		tstring Name = it->first;
		CObjectData& ObjectData = it->second;

		if (Name != _T("坐标基准") && ObjectData.Size()>4 )
		{
			int32& x = *(int32*)ObjectData.GetData(3);
			int32& y = *(int32*)ObjectData.GetData(4);

			x += dx; 
			y += dy;

			int64 ObjectID = *(int64*)ObjectData.GetData(0);

			if(m_CurObjectView.GetHwnd()){
				SendChildMessage(m_CurObjectView.GetHwnd(),OBJECT_SET,ObjectID,(int64)&ObjectData);
			}else{
				m_CurObjectView.OnSetObject(ObjectID,(int64)&ObjectData);
			}
			m_PokerDB.SaveObject(ObjectData);		

		}
		it++;
	}
}



bool CMainFrame::Do(Energy* E){


#ifdef _WINDLL
	ePipeline* ParamPipe = (ePipeline*)E;
	if (E==NULL)
	{
		return CmdRunGame();
	}

	if (ParamPipe->Size())
	{
		eElectron E;
		ParamPipe->Pop(&E);
		if (E.EnergyType() != TYPE_STRING)
		{
			ParamPipe->SetID(RETURN_ERROR);
			return false;
		}
		tstring Cmd = *(tstring*)E.Value();
		BOOL ret = CmdProc(Cmd,*ParamPipe);
		if (!ret)
		{
			ParamPipe->SetID(RETURN_ERROR);
			return false;
		}
	}
	return true;

#else
	return CmdRunGame();
#endif

}

void   CMainFrame::LingFen(){

	if (!m_bLingFen)
	{
		return ;
	}
	CObjectData BntObject;	
	if(HasLabel(_T("领分"),BntObject,90.0f))
	{
		ClickBnt(&BntObject);
		Sleep(2000);
		CObjectData* Bnt = FindObject(_T("关闭领分"));
		ClickBnt(Bnt);
		Sleep(200);
		ClickBnt(Bnt);

		tstring tip = _T("领分一次");
		NotifyTip(tip);	
	}; 
}

void CMainFrame::AllIn(){


	tstring Action;

	CObjectData* ALLBnt = FindObject(_T("ALL牌"));
	ClickBnt(ALLBnt);

	CObjectData Bnt;
	if( HasLabel(_T("跟注"),Bnt) 
		//||HasLabel(_T("加注(黄)"),BntObject)
		){

			ClickBnt(&Bnt);

			Action += _T("ALL牌(跟注）");
			NotifyTip(Action);
	}
	else
	{ 			
		CObjectData* GenBnt =FindObject(_T("加注"));
		ClickBnt(GenBnt);

		Action += _T("ALL牌(加注)");
		NotifyTip(Action);
	}
}

BOOL CMainFrame::PlayZhuanfen()
{

	int32 Total = CheckTotal();

	if (Total<5000)
	{
		FlushWindow();
		return FALSE;
	}

	CObjectData Object;
	if (!HasLabel(_T("加注"),Object))
	{
		CObjectData* KanPaiBnt = FindObject(_T("看牌"));
		ClickBnt(KanPaiBnt);
		return TRUE;
	}

	AllIn();

	CObjectData* BntObject = FindObject(_T("站起"));

	assert(BntObject);
	ClickBnt(BntObject);
	Sleep(200);
	ClickBnt(BntObject);

	return TRUE;
}

BOOL CMainFrame::PlayShuaPeng(BOOL bActivity){

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
			ClickBnt(&BntObject);
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
		if (m_SeatPos==-1 || m_SeatPos==9)
		{
			Ob = (CObjectData*)SitList.GetData(0);

			m_SeatPos = Ob->GetID();

			CObjectData* SitOb = (CObjectData*)Ob->Clone();
			eElectron e(SitOb);
			ClickBnt(SitOb);	
		}else{ 
			//寻找当前座位的下一个座位

			//CObjectData* Ob= NULL;
			for(int i=0; i<SitList.Size();i++){
				Ob = (CObjectData*)SitList.GetData(i);
				if (Ob->GetID()>m_SeatPos)
				{
					break;
				}
			}

			if (Ob == NULL)
			{
				Ob = (CObjectData*)SitList.GetData(0);
				m_SeatPos = -1;
			}else{
				m_SeatPos = Ob->GetID();
			}
			CObjectData* SitOb = (CObjectData*)Ob->Clone();
			eElectron e(SitOb);
			ClickBnt(SitOb);	

		}

		Sleep(500);
		UpdateScene();		
		while(m_Alive && !HasLabel(_T("买分窗口标志"),BntObject))
		{
			Sleep(200);
			UpdateScene();

			CObjectData* SitOb = (CObjectData*)Ob->Clone();
			eElectron e(SitOb);
			ClickBnt(SitOb);	

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
		ClickBnt(&BntObject);	
		CaptureObject(_T("最小买入值"),BntObject);
		ClickBnt(&BntObject);	

	}else{
		CObjectData BntObject;
		CaptureObject(_T("看牌"),BntObject);
		ClickBnt(&BntObject);
	}
	return TRUE;
}


BOOL CMainFrame::PlayNormalRound(PLAY_FACTOR_0 t){

	vector<int32>  SelfCard;
	vector<int32>  OtherCard; 

	UpdateScene();
	//首先尝试确定手上两张牌
	int n=2;
	
	while(m_SeatPos != -1 && n--)
	{
		if(GetSelfCard(SelfCard)){
			break;
		};
		Sleep(1000);
		UpdateScene();
	}

	//如果无法确定，则重新确定座位
	if (SelfCard.size() != 2)
	{
		int MySitePos =  CheckMySitPos(m_SeatPos);

		if (MySitePos == -1)
		{
			tstring s = tformat(_T("座位确认失败"));
			NotifyTip(s);
			return FALSE;
		}else{
			m_SeatPos = MySitePos;
			assert(m_CardType != -1);
			tstring s = tformat(_T("座位: %d CardType=%d"),MySitePos,m_CardType);
			NotifyTip(s);
		}
		m_SeatPos = MySitePos;
		
		//再次确认底牌
		if(!GetSelfCard(SelfCard)){
			tstring s = tformat(_T("底牌确认失败"));
			NotifyTip(s);
			return FALSE;	
		}
	};

	//然后得到公共牌
	Sleep(1000);
	UpdateScene();
	if (!CheckOtherCard(OtherCard))
	{
		tstring s = tformat(_T("公共牌确认失败"));
		NotifyTip(s);
		return FALSE;
	}

	//如果是首轮重新检测盲注、总分
	if (OtherCard.size()==0 && m_OldCardList != SelfCard)
	{

		int32 Total = 0;
		int32 OtherCardNum = 0;

		//当前总分
		Total = CheckTotal();
		if (Total == 0)
		{
			Total = m_CurTotal;
			tstring s = tformat(_T("总分确认失败,使用之前总分"));
			NotifyTip(s);

		}else{
			m_CurTotal = Total;
		}

		if (m_StartTotal==0 && Total)
		{
			m_StartTotal = Total;
		}

		//得到当前盲注，因为如果是比赛，盲注会随时间增加
		int32  BaseRoll = CheckMinRoll();   
		if (BaseRoll == 0)
		{	
			//尝试使用之前的结果
			if (m_MinBet ==0){
				tstring s = tformat(_T("小盲注确认失败"));
				NotifyTip(s);
				return FALSE;
			}
		}else{
			m_MinBet = BaseRoll;	
		}

		tstring s = tformat(_T("第%3d局 总分:%d 小盲注:%d 已赢:%d    "),m_PlayNum++,Total,m_MinBet,Total-m_StartTotal);
		NotifyTip(s);

	}

	m_OldCardList.assign(SelfCard.begin(),SelfCard.end());
	m_OldCardList.insert(m_OldCardList.end(),OtherCard.begin(),OtherCard.end());

	//得到风险因子,计算可以下注
	tstring Memo;
	float32 f = CheckHand(SelfCard,OtherCard,Memo);

	NotifCard(Memo,SelfCard,OtherCard);

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
		int32 n=5;
		do{
			CObjectData BntObject;
			if (
				//	HasLabel(_T("看牌(黄)"),BntObject,90.0f) ||  //看牌不要钱，好坏都看
				HasLabel(_T("看牌",90.0f),BntObject)
				)
			{   

				ClickBnt(&BntObject);

				tstring Action = _T("操作: 看牌");
				NotifyTip(Action);

				Sleep(1000);
				return TRUE;
			}
			else if(
				//HasLabel(_T("跟注(黄)"),BntObject,90.0f) ||
				HasLabel(_T("跟注"),BntObject,90.0f)
				)
			{   	

				//得到手上筹码
				int32 MyHandRoll = CheckMyRoll(m_SeatPos);
				if (MyHandRoll == 0)
				{ 
					MyHandRoll = m_HandRoll;

					tstring s = _T("手上筹码获取错误，使用之前结果");
					NotifyTip(s);				
				}

				tstring  tip = tformat(_T("手上筹码=%d "),MyHandRoll);		

				//得到要求的跟注大小
				int32  BetNum = CheckBetRoll();	
				if (BetNum == 0)
				{
					tip += _T("跟注获取错误");
					NotifyTip(tip);
					return FALSE;
				}        

				//根据风险计算出可以下注的金额
				f=1+f-5;
				int32 Roll = f*f*f*m_MinBet*2;  

				if (BetNum <= Roll || BetNum == m_MinBet) //如果要求的金额小于可以下注的，或者是盲注则跟
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

					ClickBnt(&BntObject);

					NotifyTip(tip);

					tstring Action = tformat(_T("操作: 跟注=%d"),BetNum);
					NotifyTip(Action);

					Sleep(1000);
					return TRUE;		
				}else{
					tip += tformat(_T("操作：弃牌，要求下注:%d > 计划下注:%d"),BetNum,Roll);
					NotifyTip(tip);
					return FALSE;
				}				
			}else{
				n--;
				if (n==0)
				{
					tstring tip = _T("没发现看牌或跟注");
					NotifyTip(tip);	
#ifndef _WINDLL
					CBitmapEx Image;
					Image.Load(m_Scene);
					Image.Save(_T("没发现看牌或跟注.bmp"));
#endif				
					return FALSE;
				}
				Sleep(1000);
				UpdateScene();
			}
		}while(m_Alive && n);
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
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::PlayDingShiRenWu()
{
	CHighTime CurTime = CHighTime::GetPresentTime();
	CHighTime t1,t2;
	t1.SetDateTime(CurTime.GetYear(),CurTime.GetMonth(),CurTime.GetDay(),17,0,0);
	t2.SetDateTime(CurTime.GetYear(),CurTime.GetMonth(),CurTime.GetDay(),21,0,0);

	static BOOL bInit = FALSE;
	if (CurTime>=t1 && CurTime<t2)
	{	
		if (bInit == FALSE)
		{
			bInit = TRUE;
			m_PlayNum = 0;
			NotifyTip(_T("定时任务开始计数"));

			CObjectData* ob1  = FindObject(_T("定时玩牌按钮"));
			assert(ob1 && ob1->Size()>5);

			CObjectData* ob11 = FindObject(_T("领取任务"));
			assert(ob11 && ob11->Size()>5);

			CObjectData* ob6 = FindObject(_T("关闭定时任务2"));
			assert(ob6 && ob6->Size()>5);

			ClickBnt(ob1); 
			Sleep(1000);

			/*
			int n=5;
			while (n-->0 && !HasLabel(_T("关闭定时任务2")))
			{
			Sleep(1000);
			ClickBnt(ob1); 
			}
			*/

			ClickBnt(ob11);	
			Sleep(200);
			ClickBnt(ob11);
			Sleep(200);

			ClickBnt(ob6);
			Sleep(200);
			ClickBnt(ob6);
			/*
			n=5;
			while (n-- >0 && HasLabel(_T("关闭定时任务2")))
			{
			Sleep(500);
			ClickBnt(ob6); 
			}
			*/
		}
		if (m_PlayNum%10 ==0)
		{
			CObjectData* ob1 = FindObject(_T("定时玩牌按钮"));
			CObjectData* ob11 = FindObject(_T("领取任务"));
			CObjectData* ob2 = FindObject(_T("定时任务1"));
			CObjectData* ob3 = FindObject(_T("定时任务2"));
			CObjectData* ob4 = FindObject(_T("定时任务3"));

			CObjectData* ob5 = FindObject(_T("关闭定时任务1"));
			CObjectData* ob6 = FindObject(_T("关闭定时任务2"));

			ClickBnt(ob1); 
			Sleep(200);
			ClickBnt(ob1);
			/*
			int n=5;
			while (n-->0 && !HasLabel(_T("关闭定时任务2")))
			{
			Sleep(1000);
			ClickBnt(ob1); 
			}
			*/
			//ClickBnt(ob11);

			ClickBnt(ob2);
			Sleep(200);
			ClickBnt(ob3);
			Sleep(200);
			ClickBnt(ob4);
			Sleep(200);

			Sleep(1000);
			/*
			ClickBnt(ob5);

			Sleep(500);
			n=5;
			while (n-- >0 && HasLabel(_T("关闭定时任务1")))
			{
			Sleep(500);
			ClickBnt(ob5); 
			}
			*/
			ClickBnt(ob6);
			Sleep(200);
			ClickBnt(ob6);
			/*
			n=5;
			while (n-- >0 && HasLabel(_T("关闭定时任务2")))
			{
			Sleep(500);
			ClickBnt(ob6); 
			}
			*/
		}
		return FALSE;
	}else{
		return PlayNormalRound(PF_NORMAL);
	}
}
BOOL CMainFrame::PlayShuaJu(){


	//m_PlayNum ++;
	if (m_PlayNum > 88 && m_PlayNum<90)
	{
		FlushWindow();
	}

	return FALSE;
}


BOOL CMainFrame::PlayGame(){

	BOOL ret = m_Option.GetValue(PLAY_TYPE,m_CurPlayType);
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

	if (m_CurPlayType ==_T("1 自由场"))
	{
		m_bBisai = FALSE;
		m_bTaojingSai = FALSE;
		return PlayNormalRound(pf);
	}
	else if (m_CurPlayType == _T("2 刷局"))
	{
		return PlayShuaJu();
	}
	else if (m_CurPlayType == _T("3 定时任务"))
	{
		return PlayDingShiRenWu();
	}
	else if (m_CurPlayType == _T("4 转分"))
	{
		return PlayZhuanfen();
	}
	else if (m_CurPlayType == _T("5 二人刷盆"))
	{
		return TRUE;
		return PlayShuaPeng(TRUE);	

	}else if (m_CurPlayType == _T("6 刷盆"))
	{
		return TRUE;
		return PlayShuaPeng(FALSE);
	}	
	else if(m_CurPlayType == _T("7 比赛")){
		return TRUE;
		m_bBisai = TRUE;
		m_bTaojingSai = FALSE;
		return PlayNormalRound(pf);
	}
	else if (m_CurPlayType == _T("8 淘金赛"))
	{
		return TRUE;
		m_bBisai = TRUE;
		m_bTaojingSai = TRUE;
		return PlayNormalRound(pf);

	}
	return TRUE;
};



