

#ifndef _BRAINOBJECTVIEW_H
#define _BRAINOBJECTVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include "WSListView.h"
#include "PokerDatabase.h"

using namespace VISUALSPACE;
/*
 编号 [类型] 图像
*/
class ObjectItem : public CVSpace2
{
public:
    OBJECTTYPE  m_Type;
	tstring     m_Name;
    HBITMAP     m_hBMP; 
	uint32      m_Crc32;
	int32       m_x;  //截图的原始位置
	int32       m_y;
	int32       m_w;
	int32       m_h;
	tstring     m_OcrText; //图像识别的文字
	tstring     m_Time;    //截图时间
public:
	ObjectItem(){

	};
	ObjectItem(int64 ID,tstring Name,OBJECTTYPE Type,HBITMAP hBMP,uint32 Crc32,tstring Time);
	void SetIamgePos(int32 x,int32 y, int32 w,int32 h);

	virtual ~ObjectItem();

	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
};


class ObjectViewToolbar: public CVSpace2{
public:
	CVSpace2*   m_Owner;
public:
	ObjectViewToolbar();
	virtual ~ObjectViewToolbar(){};
    
	virtual  void  Layout(bool Redraw = true); //根据Owner->m_Area
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

class ToolbarBnt : public ButtonItem
{	
	
public:
	ToolbarBnt(int64 ID,const TCHAR* text)
		:ButtonItem(ID,text,NULL,false){
	};
	virtual ~ToolbarBnt(){};
	virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
};

#define OBJECT_ADD       1
#define OBJECT_DEL       2
#define OBJECT_CLR       3
#define OBJECT_FIND      4
#define OBJECT_SETTITLE  5
#define OBJECT_ENABLE    6
#define OBJECT_SET       7
#define OBJECT_CAPTURE   8
#define OBJECT_OCR       9
#define OBJECT_OCR_TXT   10

class CObjectView : public CWSListView   
{
public:
	tstring                 m_Name;
    Nerve*                  m_Outport;
	ObjectViewToolbar       m_Toolbar;
    CVSpace2*               m_ToolbarItemSelected;

public:
	CObjectView();
	virtual ~CObjectView(){};
    
protected:  	
    virtual void    Draw(HDC hDC, ePipeline* Pipe=NULL);
	virtual void    Layout(bool Redraw =true);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void    DrawItemHeader(HDC hDC, CVSpace2* Space);
	virtual RECT    GetHeaderBox(CVSpace2* Space); //根据Space->Area计算TreeBox坐标	
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);	
	virtual LRESULT ToolbarReaction(ToolbarBnt* Bnt);

public:
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

	LRESULT OnAddObject(int64 wParam, int64 lParam);
	LRESULT OnSetObject(int64 wParam, int64 lParam);
	LRESULT OnDeleteObject(int64 wParam, int64 lParam);
	LRESULT OnClearAll(int64 wParam, int64 lParam);
	LRESULT OnFindObject(int64 wParam, int64 lParam);
	LRESULT OnSetObjectOcrTxt(int64 wParam, int64 lParam);
};

#endif // _BRAINOBJECTVIEW_H
