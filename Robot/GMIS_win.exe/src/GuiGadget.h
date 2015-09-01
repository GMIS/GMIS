/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _GUIGADGET_H__
#define _GUIGADGET_H__


#include "GUIElementMisc.h"
#include "Splitter.h"

using namespace VISUALSPACE;



#define BNT_OTHER    9999

//用图像贴图代替原来的单色背景
class ImageBorderItem : public BorderItem
{
public:
	HANDLE          m_Image;
public:
	ImageBorderItem(int64 ID,HANDLE Image):m_Image(Image){m_Alias = ID;};
	~ImageBorderItem(){};
		
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

//用图像贴图代替原来的单色背景
class ImageTitleBar: public ImageBorderItem
{
public: 
	COLORREF  m_crBorder;
public:
	ImageTitleBar(int64 ID, HANDLE Image);
	ImageTitleBar(int64 ID, COLORREF crActive,COLORREF crInActive,COLORREF crBorder);
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};


#define  SPACE_MUTILLINETEXT    0x00000020

class ControlBnt : public ButtonItem
{	
public:
	ControlBnt(int32 ID,const TCHAR* text,bool IsMutilLine=false);
	virtual ~ControlBnt(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

class CAddNewBnt : public ControlBnt
{	
public:
	CAddNewBnt(int32 ID,const TCHAR* text,bool IsMutilLine=false);
	virtual ~CAddNewBnt(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

class MainBnt : public ButtonItem
{	
public:
	MainBnt(int32 ID,const TCHAR* text,bool IsMutilLine=false);
	virtual ~MainBnt(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

//工具栏分割线
class SeparatorLine : public ButtonItem
{
public:
     int m_NumLine;       //default = 1
	 COLORREF  m_crDark;  //default RGB(13,13,13)
	 COLORREF  m_crLight; //default
public:
	SeparatorLine(int32 ID,const TCHAR* text,int32 nline = 1);
	virtual ~SeparatorLine(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};



/*类似windos程序管理器里的CPU使用率
  -- --
  -- --
   20%
*/
class VertiacalProgressItem : public CVSpace2{
public:
	COLORREF  m_crProgress;   //default = RGB(0,255,0)
	COLORREF  m_crBk;         //default = RGB(0,0x8b,0x8b)
	int32     m_Per;
public:
    VertiacalProgressItem();
	virtual ~VertiacalProgressItem();
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

/*显示类似： 
  口 
  口 
  口 
*/
class VertiacalProgressItem1 : public CVSpace2{
public:
	int32     m_Per;
	bool      m_IsBottomToTop;  //default = true;
	COLORREF  m_crProgress;   //default = RGB(0,0xbf,0xff)
	COLORREF  m_crBk;         //default = RGB(0,0x8b,0x8b)
public:
    VertiacalProgressItem1();
	virtual ~VertiacalProgressItem1();
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

//显示类似： 口 口 口 口 口 
class HorizontalProgressItem : public CVSpace2{
public:
	int32     m_Per;
	bool      m_IsLeftToRight;  //default = true;
	COLORREF  m_crProgress;     //default = RGB(0,255,0)
public:
    HorizontalProgressItem();
	virtual ~HorizontalProgressItem();
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

#define WM_SETPROGRESSPER    WM_USER+1

//用于神经中枢的信息处理
class BrainProgressStruct : public CVSpace2
{
public:
    VertiacalProgressItem   m_ProgressBar;
public:
    BrainProgressStruct();
	virtual ~BrainProgressStruct();

	void SetPer(int32 Per);
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};

//工具棒中如果有其他未完的则显示这个
class OtherBnt: public CVSpace2{
public:
	HANDLE          m_Image;
	tstring          m_Name;
public:
	OtherBnt(int64 ID,CVSpace2* Parent); //Parent所属toolbar
	~OtherBnt();
	
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};

/*
  一个标准工具条:
  可以拥有任意多个按钮；
  工具条只提供安置位置；
  根据按钮Size决定按钮是否可见；
  当按钮无法显示完时，提供一个缺省的未完按钮，用户点击这个按钮之后
  系统可要求获得其他没显示的做更多处理
   
*/
class STDToolbar : public CVSpace2{

public:
	HANDLE             m_Image;
	bool               m_IsVertiacal;   //default = false;
	vector<CVSpace2*>  m_VisibleBntList;
	OtherBnt           m_BntOther;
public:
	STDToolbar(HANDLE Image);
	~STDToolbar();

	virtual CVSpace2*  HitTest(int32 x,int32 y); //改为测试m_visibleBntList
	void GetInvisibleBnt(vector<CVSpace2*>& InvisibleList);
	void AddBnt(ButtonItem* Bnt){
		PushChild(Bnt);
	}
	ButtonItem* GetBnt(int ID){ return (ButtonItem*)FindSpace(ID);}
	virtual void Layout(bool Redraw);
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
};



//3D part 
////////////////////////////////////////////////////




#endif // !_GUIGADGET_H__
