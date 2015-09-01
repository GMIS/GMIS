/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LOCALLOGICVIEW_H__
#define _LOCALLOGICVIEW_H__


#include "WSTreeView.h"

using namespace VISUALSPACE;

#define ELEMENT_PROC    101
#define ELEMENT_RESET   201

#define  SPACE_DELETE  0x80000000


#define CM_SELECT_LOGIC 20
#define CM_DELETE_LOGIC 21
#define CM_CLEAR_LOGIC  22


#define TYPE_LOGIC  0x001
#define TYPE_OBJECT 0x002
#define TYPE_TASK   0x004
#define TYPE_PEOPLE 0x008

#define CM_LOGICVIEW       10006


class CLogicView : public CWSTreeView
{
public:
	class CLogicLabel : public CVSpace2{
	public:
		tstring        m_Name;
		HICON          m_hIcon; 	
	public:
		CLogicLabel(int32 TypeID,tstring Name,HICON hIcon=NULL);
		virtual ~CLogicLabel(){if(m_hIcon)::DeleteObject(m_hIcon);};
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
	};

	class CRefItem : public CVSpace2{
	public:
		tstring    m_RefName;
		tstring    m_WhoRef;	
	public:
		CRefItem();
		CRefItem(tstring RefName, tstring WhoRef);
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
		
	};

	class CLogicItem :public CVSpace2{
	public:
		int32         m_Depend; //是否独立，即是否引用其他逻辑
		uint32        m_ActionType;
		tstring       m_LogicName;
		tstring       m_LogicText;
		tstring       m_LogicMemo;
		SIZE          m_MemoSize;
	public:	
		CLogicItem(ePipeline* ItemData);
		virtual ~CLogicItem();
		
		CRefItem*  AddRef(tstring RefName,tstring WhoRef);
		CRefItem*  DelRef(tstring RefName,tstring WhoRef);
		
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
	};



public:
	CLogicView();
	virtual ~CLogicView();

	void ResetLocalElement(ePipeline& List);
	void LogicElementProc(ePipeline* Item);

protected:
	virtual void Layout(bool Redraw =true);
	void Draw(HDC hDC, ePipeline* Pipe=NULL);
	
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);
	
	LRESULT OnElementProc(WPARAM wParam, LPARAM lParam);
	LRESULT OnResetLocalElement(WPARAM wParam, LPARAM lParam);


	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam); 
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(_LOCALLOGICVIEW_H__)
