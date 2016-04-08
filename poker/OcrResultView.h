// OcrReusltView.h: interface for the COcrResultView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OCRREUSLTVIEW_H__DF9305FF_2107_4FC5_ADAF_ED323C7E8BDA__INCLUDED_)
#define AFX_OCRREUSLTVIEW_H__DF9305FF_2107_4FC5_ADAF_ED323C7E8BDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "VisibleSpace.h"
#include "WSListView.h"
#include "EditWin.h"

using namespace VISUALSPACE;

#define CM_KILLFOCUS     100
#define CM_ADDITEM       101
#define CM_SETOCR        102
#define CM_OCRTEXT_SET   1203

class COcrItem : public CVSpace2
{
public:
	tstring     m_Name;
    HBITMAP     m_hBMP; 
	uint32      m_Crc32;
	tstring     m_OcrText; //图像识别的文字
public:
	COcrItem();	
	COcrItem(int64 ID,tstring Name,HBITMAP hBMP,uint32 Crc32,tstring& OcrText);	
	virtual ~COcrItem();
	
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
};

class COcrEdit : public CEditWin 
{
protected:
	virtual BOOL ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );
};

class CMainFrame;
class COcrResultView : public CWSListView  
{
protected:
	CMainFrame*     m_MainFrame;
	COcrEdit        m_OcrEdit;
public:
	COcrResultView(CMainFrame* Frame);
	virtual ~COcrResultView();

	void AddItem(tstring Name,HBITMAP hBMP,uint32 Crc32,tstring OcrStr);

protected:
	virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);	
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);	

	virtual void Layout(bool Redraw = true);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDBCLK(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_OCRREUSLTVIEW_H__DF9305FF_2107_4FC5_ADAF_ED323C7E8BDA__INCLUDED_)
