/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _CWSFORMVIEW_H__
#define _CWSFORMVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <map>
#include "WSListView.h"

using namespace VISUALSPACE;


enum { 
	CONTROL_NULL,
	CONTROL_PUSHBNT,
	CONTROL_EDIT,
	CONTROL_CHECKBOX,
	CONTROL_RADIO,
	CONTROL_COMBOLIST
};
class CInterControl : public CVSpace2
{
public:
	HWND			m_hControl; 
	RECT			m_rcControl;    //放置控件，坐标相对于此空间
  	int32           m_ControlType;
public:
	CInterControl(int64 ID,CVSpace2* Parent,int32 Type);
	~CInterControl();
   
    void SetControlRect(int32 left,int32 top,int32 right,int32 bottom);	
	virtual void OnLButtonDown(POINT& point){}; //用户承继，决定是否生成对应控件窗口
    virtual void OnLoseFocus(){};

	virtual int32 GetType(){return m_ControlType;};
	virtual void Layout(bool Redraw = true ); //移动自己所含控件窗口到正确位置

	virtual BOOL FromPipe(ePipeline& Pipe){ return TRUE; };
	virtual void ToPipe(ePipeline& Pipe){};
};

class CWSFormView : public CWSListView  
{
public:
	/*显示：
	 xxxx
	 ---------------------------
	*/
	class  CHeadline : public CInterControl{
	public:
		tstring  m_Title;
	public:
		CHeadline(int64 ID,CVSpace2* Parent,tstring Title);
		virtual ~CHeadline();				
		virtual void Draw(HDC hDC,ePipeline* Pipe);
		
	};	

	/*显示：
	 xxxxx: [    ]
	*/
	class  CEditText : public CInterControl{
	public:
		tstring  m_Title;
		tstring  m_EditText;
		BOOL     m_bNumber;
	public:
		CEditText(int64 ID,CVSpace2* Parent,tstring Title,BOOL bNumber);
		virtual ~CEditText();				
		virtual void Draw(HDC hDC,ePipeline* Pipe);
		virtual void OnLButtonDown(POINT& point);
		virtual void OnLoseFocus();
		virtual BOOL FromPipe(ePipeline& Pipe);
		virtual void ToPipe(ePipeline& Pipe);

	};	

	/*显示
	xx [v] 
	*/
	class  CCheckBox : public CInterControl{
	public:
		tstring  m_Title;
		BOOL     m_bChecked;
	public:
		CCheckBox(int64 ID,CVSpace2* Parent,tstring Title,BOOL bChecked);
		virtual ~CCheckBox();				
		virtual void Draw(HDC hDC,ePipeline* Pipe);	
		virtual void OnLButtonDown(POINT& point);
		virtual BOOL FromPipe(ePipeline& Pipe);
		virtual void ToPipe(ePipeline& Pipe);
	};
	

	/*显示
	xx ( ) 
	xx (*) 
	xx ()
	m_rcControl为整个按钮组不包括名字的坐标
	*/
	class  CRadioSet: public CInterControl{
	public:
		map<tstring,BOOL>     m_RadioSet;
	public:
		CRadioSet(int64 ID,CVSpace2* Parent);
		virtual ~CRadioSet();
		void  PushRadioValue(tstring Name,BOOL Value);
		virtual void Draw(HDC hDC,ePipeline* Pipe);	
		virtual void OnLButtonDown(POINT& point);
		virtual BOOL FromPipe(ePipeline& Pipe);
		virtual void ToPipe(ePipeline& Pipe);
	};
	
	/*显示
	  xxxxx: [ ] 
	class  CSpinEdit : public CInterControl{
	public:
		tstring  m_Title;
		tstring  m_EditText;
	public:
		CSpinEdit(int64 ID,CVSpace2* Parent);
		virtual ~CSpinEdit();				
		virtual void Draw(HDC hDC,ePipeline* Pipe);	
	};
	*/

	class  CComboList : public CInterControl{
	public:
		tstring  m_Title;
		tstring  m_CurText;
	public:
		CComboList(int64 ID,CVSpace2* Parent,tstring Title,tstring CurText);
		virtual ~CComboList();				
		virtual void Draw(HDC hDC,ePipeline* Pipe);
		virtual void OnLButtonDown(POINT& point);
		virtual void OnLoseFocus();
		virtual BOOL FromPipe(ePipeline& Pipe);
		virtual void ToPipe(ePipeline& Pipe);
	};


public:
	CWSFormView();
	virtual ~CWSFormView();

	BOOL FromPipe(ePipeline& Pipe);
	void ToPipe(ePipeline& Pipe);

protected:
	virtual void  DrawItemHeader(HDC hDC, CVSpace2* Space){}; //取消编号
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);	
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
};

#endif // _CWSFORMVIEW_H__
