/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BRAINOBJECTVIEW_H
#define _BRAINOBJECTVIEW_H


#include "VisibleSpace.h"
#include "WSListView.h"
#include "space.h"

using namespace VISUALSPACE;

class ObjectItem : public CVSpace2
{
public:
	tstring    m_Name;
	int32      m_Index;     //显示序号
	SPACETYPE  m_Type;
	tstring    m_Fingerprint;
    HICON      m_hIcon; 
public:
	ObjectItem(){
	};
	ObjectItem(int64 ID,tstring& Name,SPACETYPE Type,tstring& Fingerprint,HICON hIcon);	
	virtual ~ObjectItem(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
};

//显示标题栏
class ObjectHeader: public ObjectItem{
public:
	tstring    m_Title;
	COLORREF   m_crText;
public:
	ObjectHeader();
	virtual ~ObjectHeader(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
};


#define OBJECT_ADD       1
#define OBJECT_DEL       2
#define OBJECT_CLR       3
#define OBJECT_FIND      4
#define OBJECT_SETTITLE  5
#define OBJECT_ENABLE    6

class CObjectView : public CWSListView   
{
public:
	tstring                 m_Name;
public:
	CObjectView();
	virtual ~CObjectView(){};

	
	void AddItem(int64 ID,tstring Name,HICON Type);
	void DeleteItem(int64 ID); 
	void ClearAllItem();
	
    void  ConnectSpaceFail(tstring Reason);
	void  SetHeaderText(tstring Title,BOOL Enable=TRUE);
	void  AddObject(int64 ID,tstring& Name,SPACETYPE Type,tstring& Fingerprint,HICON hIcon);
	void  DeleteObject(int64 ID); 
    int32 FindObject(tstring& Name, vector<ObjectItem>& SpaceList);
	void  ClearAllObject();

	virtual void    Layout(bool Redraw =true);

protected:  	
    virtual void    Draw(HDC hDC, ePipeline* Pipe=NULL);
	virtual void    DrawItemHeader(HDC hDC, CVSpace2* Space);
	virtual RECT    GetHeaderBox(CVSpace2* Space); //根据Space->Area计算TreeBox坐标	
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);	
	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);
public:
	LRESULT OnAddObject(int64 wParam, int64 lParam);
	LRESULT OnDeleteObject(int64 wParam, int64 lParam);
	LRESULT OnClearAll(int64 wParam, int64 lParam);
	LRESULT OnFindObject(int64 wParam, int64 lParam);
	LRESULT OnSetTitle(int64 wParam, int64 lParam);
};



#endif // _BRAINOBJECTVIEW_H
