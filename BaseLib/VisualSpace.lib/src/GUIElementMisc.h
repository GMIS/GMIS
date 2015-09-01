/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _GUIELEMENTMISC_H_
#define _GUIELEMENTMISC_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include "WinSpace2.h"
#include <string>

//预定义一下常用子空间

#define ID_TITLE        -1
#define ID_TOPSIDE      -2
#define ID_LEFTSIDE     -3
#define ID_RIGHTSIDE    -4
#define ID_BOTTOMSIDE   -5
#define ID_ICON         -6
#define ID_TITLETEXT    -7
#define ID_MAXBNT       -8
#define ID_MINBNT       -9
#define ID_CLOSE        -10
#define ID_DOCKBNT      -11  
#define ID_SPLITTER     -50

#ifndef GET_X_LPARAM
	#define GET_X_LPARAM(lp)	((int)(short)LOWORD(lp))
	#define GET_Y_LPARAM(lp)	((int)(short)HIWORD(lp))
#endif

namespace VISUALSPACE{
	

	//用与滚动棒那种小箭头,用户可以重载Draw()绘制自己的
	enum  ARROW_DIRECTION{ ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT};
	class  ArrowItem : public CVSpace2
	{
	public:
		ARROW_DIRECTION   m_ArrowDirection;
		COLORREF          m_crFg;
		COLORREF          m_crBk;
		COLORREF          m_crEdge;   
    public:	
		ArrowItem();
		ArrowItem(int64 ID,ARROW_DIRECTION ArrowDir,
			      COLORREF crFg   = RGB(0,255,0),
                  COLORREF crBk   = RGB(245,245,243),
                  COLORREF crEdge = RGB(128,128,128)
			);
		virtual ~ArrowItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};
	
	class  ThumbItem : public CVSpace2{
	public:
		bool       m_bVertical;
		COLORREF   m_crFg; 
        COLORREF   m_crBk; 
        COLORREF   m_crEdge; 
	public: 
		ThumbItem();
		ThumbItem(int64 ID,bool bVertical,
			      COLORREF crFg   = RGB(0,255,0),
                  COLORREF crBk   = RGB(245,245,243),
                  COLORREF crEdge = RGB(128,128,128)
			);
		virtual ~ThumbItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);     
	};
	

	class  BorderItem : public CVSpace2{
	public:
		COLORREF  m_crActivate;
		COLORREF  m_crInactivate;
		COLORREF  m_crFocused;
	public:
		BorderItem();
		BorderItem(int64 ID,
			COLORREF crAct   = RGB(0,127,255),
			COLORREF crInact = RGB(0,97,186),		
			COLORREF crFocused= RGB(255,127,127) 
			);
		void SetColor(COLORREF crAct,COLORREF crInact,COLORREF crFocused){
			m_crActivate = crAct; m_crInactivate = crInact; m_crFocused = crFocused;
		}
		virtual ~BorderItem();
		virtual CVSpace2*  HitTest(int32 x, int32 y);   //根据point计算自己是否被触动并进一步计算是那一个具体的child
		virtual void      Draw(HDC hDC,ePipeline* Pipe=NULL);    		
	};
	
	
	class  ButtonItem : public CVSpace2{
	public:	
		tstring          m_Name;
		bool            m_bVerDrawText; //不是垂直就是水平
		HANDLE          m_Image;   
		CWinSpace2*     m_AttachSpace;
	public:
		ButtonItem();
		ButtonItem(int64 CmdID,const TCHAR* text,CWinSpace2* Win,bool VerticalText,HANDLE Image=NULL);	
		virtual ~ButtonItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};
	
	//m_SizeX和m_SizeY决定icon大小，default = 16
	class  IconItem : public CVSpace2{
	public:
		HICON      m_hIcon; 
	public:
		IconItem();
		IconItem(int64 ID,HICON Icon,int32 size=16);
		virtual ~IconItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);     
	};
	
#define TEXT_LEFT   0x00001000
#define TEXT_RIGHT  0x00002000
#define TEXT_CENTER 0x00004000

	class  TextItem:public CVSpace2{
	public:
		COLORREF  m_crText;
		tstring    m_Text;
		HFONT     m_TextFont;     //default =SYSTEM_FONT
	public:
		TextItem();
		TextItem(int64 ID,tstring Text);
		virtual ~TextItem();
		virtual   void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};
	
	class  MaxboxItem : public CVSpace2{
	public:
		bool      m_bMax;

		COLORREF  m_crBorder;
		COLORREF  m_crNormal;
		COLORREF  m_crFocused;
	public:	
		MaxboxItem();
		MaxboxItem(int64 ID,
			COLORREF crBorder  = RGB(255,255,255),
			COLORREF crNormal  = RGB(255,255,255),
	        COLORREF crFocused = RGB(0,255,0) 
			);
		virtual ~MaxboxItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);     
	};
	
	class  MinboxItem : public CVSpace2{
	public:
		COLORREF  m_crBorder;
		COLORREF  m_crNormal;
		COLORREF  m_crFocused;
	public:
		MinboxItem();
		MinboxItem(int64 ID,
			COLORREF crBorder  = RGB(255,255,255),
			COLORREF crNormal  = RGB(255,255,255),
			COLORREF crFocused = RGB(0,255,0) 
			);
		virtual ~MinboxItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);     
	};
	class  CloseboxItem : public CVSpace2{
	public:
		COLORREF  m_crBorder;
		COLORREF  m_crNormal;
		COLORREF  m_crFocused;
	public:
		CloseboxItem();
		CloseboxItem(int64 ID,
			COLORREF crBorder  = RGB(255,255,255),
			COLORREF crNormal  = RGB(255,255,255),
			COLORREF crFocused = RGB(0,255,0) 
			);

		virtual ~CloseboxItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);     
	};
		
	class  DockboxItem : public CVSpace2{
	public:
		COLORREF  m_crBorder;
		COLORREF  m_crNormal;
		COLORREF  m_crFocused;		
		bool      m_IsDock;
	public:	
		DockboxItem(int64 ID,
			COLORREF crBorder  = RGB(192,192,192),
			COLORREF crNormal  = RGB(192,192,192),
	        COLORREF crFocused = RGB(0,255,0)
			);
		~DockboxItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);     
	};

#define  SPLITTER_CLOSE   0x00000100 
	class  SplitterItem:public CVSpace2
	{
	public:
		COLORREF  m_crBorder;
		COLORREF  m_crNormal;
		COLORREF  m_crFocused;
		COLORREF  m_crClose;    //即两个Splitter夹紧

		HANDLE    m_imagSkin;

		int32     m_BeginPos;  //Splitter左右或上下移动范围
		int32     m_EndPos;
		
		CWinSpace2* m_LeftView;
		CWinSpace2* m_RightView;
		
	public:
		SplitterItem();
		SplitterItem(int64 ID,HANDLE Imag);
		SplitterItem(int64 ID,
			COLORREF crBorder = RGB(100,100,100), 
			COLORREF crNormal = RGB(245,245,243),
	        COLORREF crFocused= RGB(100,100,100),
			COLORREF crClose  = RGB(0,0,128)
		);
		virtual ~SplitterItem();
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	    

	};
	

	void Draw3DEdge(HDC hDC,RECT* Area);
	void Draw3DEdge(HDC hDC,RECT& Area);
	
} //namespace VISUALSPACE
#endif // !defined(_GUIELEMENTMISC_H_)
