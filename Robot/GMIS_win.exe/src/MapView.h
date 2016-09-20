/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _BRAINMAPVIEW_H__
#define _BRAINMAPVIEW_H__


#include "PhysicSpace.h"
#include "WSTreeView.h"
#include "GuiGadget.h"
#include "TitleView.h"

using namespace VISUALSPACE;

/*显示项目：
ICON 名字
*/
class MapItem : public CWSTreeView::CTreeItem
{
public:
	static HICON  hWorld;
	static HICON  hContainer;
	static HICON  hPeople;
	static HICON  hSpace;

	HICON         m_hIcon;
public:
	MapItem(){
		m_hIcon = 0;
	};
	MapItem(int64 ID,const TCHAR* Name,HICON hIcon);
	virtual ~MapItem(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
};

class WorldStart : public MapItem{
public:
	HWND        m_IPAddress;
	ControlBnt  m_GotoBnt;
public:
	WorldStart(int64 ID,const TCHAR* Name,HICON hIcon);
	virtual ~WorldStart();
	virtual void Layout(bool Redraw  =true);
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);	
	virtual CVSpace2*  HitTest(int32 x,int32 y);
};

#define MAP_ADD   1
#define MAP_DEL   2
#define MAP_CLR   3
#define MAP_RESET 4
#define MAP_FIND  5

#define  BN_GOTO_SPACE         202    


class CMapView : public CWSTreeView   
{
	friend class CTitleMapView;
	
	MapItem*          m_OldFindItem;
public:
    Nerve*            m_Outport;
    
	void SetOuterSpaceName(tstring Name);
	void SetLocalSpaceName(tstring Name);
 
	void EnableNewConnect(BOOL bEnable);
public:
	CMapView();
	virtual ~CMapView(){};
	
	void GetSpacePath(ePipeline& Path);
	MapItem* Path2Item(ePipeline& Path);

	void InitLocalSpace(tstring LocalName); 
	
	MapItem* GetCurSpace(){
		return (MapItem*)m_SpaceSelected;
	}
	void SetCurSpace(int64 ChildID);

public:
	virtual void Layout(bool Redraw  =true);
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	LRESULT OnAddSpace(int64 wParam, int64 lParam);
	LRESULT OnDeleteSpace(int64 wParam, int64 lParam);
	LRESULT OnClearSpace(int64 wParam, int64 lParam);
	LRESULT OnReset();
    LRESULT OnFind(int64 wParam, int64 lParam);

protected:

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam); 
};



class CTitleMapView : public CTitleView  
{
public:
	bool           m_Created;
	CMapView       m_MapView;
public:
	CTitleMapView():m_Created(FALSE){};
	virtual ~CTitleMapView(){};

	void InitLocalSpace(tstring LocalName){
		m_MapView.InitLocalSpace(LocalName);
	};
	

	MapItem* AddItem(int64 ParentID,int64 ChildID,tstring Name,HICON Type);
	
	bool DeleteItem(ePipeline* Path,tstring& Name);
	
	MapItem* FindChildItem(int64 ID);
	void ClearAllItem();
	void Reset();
    
	void SetCurSpace(int64 ChildID);
	void  Layout(bool Redraw);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
 	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam); 

};


#endif // !defined(_BRAINMAPVIEW_H__)
