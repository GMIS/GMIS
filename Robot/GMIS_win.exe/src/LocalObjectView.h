/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LOCALOBJECTVIEW_H__
#define _LOCALOBJECTVIEW_H__

#include "VisibleSpace.h"
#include "WSTreeView.h"

using namespace VISUALSPACE;


#define ID_DEL_OBJECT    101
#define ID_CLR_OBJECT    102

#define OBJECT_PROC    101
#define OBJECT_RESET   201

class CLocalObjectView : public CWSTreeView  
{
public:
	class CObjectLabel : public CVSpace2{
	public:
		tstring        m_Name;
		HICON          m_hIcon; 	
	public:
		CObjectLabel(int32 TypeID,tstring Name,HICON hIcon=NULL);
		virtual ~CObjectLabel(){if(m_hIcon)::DeleteObject(m_hIcon);};
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
	};

	class CObjectItem: public CVSpace2{
	public:
		HICON		  m_hIcon;
		CObjectData   m_ObjectData;
	public:
		CObjectItem(ePipeline& ItemData);
		~CObjectItem();
		ePipeline* GetItemData();

		virtual void Draw(HDC hDC,ePipeline* Pipe = NULL);
	};

	int32  m_ObjectIDCount;

public:
	CLocalObjectView();
	virtual ~CLocalObjectView();
	
	void ResetLocalObject(ePipeline& ObjectList);
	
	void ObjectProc(ePipeline* Item);

protected:
	virtual void Layout(bool Redraw =true);
	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);

	void Draw(HDC hDC, ePipeline* Pipe=NULL);
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	LRESULT OnObjectProc(WPARAM wParam, LPARAM lParam);
	LRESULT OnResetLocalObject(WPARAM wParam, LPARAM lParam);
	
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_LOCALOBJECTVIEW_H__A9753947_4A29_4E9E_9E00_94D60E4F221C__INCLUDED_)
