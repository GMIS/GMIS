/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _BRAINFINDVIEW_H__
#define _BRAINFINDVIEW_H__


#include <map>
#include "BrainMemory.h"
#include "WSTreeView.h"

using namespace VISUALSPACE;

class FindItem : public CVSpace2{
public:
	tstring m_Text;
public:
	FindItem(){};
	FindItem(int64 ID,const TCHAR* text);
	virtual ~FindItem(){};

	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};


class ArrowBnt : public CVSpace2{
public:
	tstring  m_Text;
public:
	ArrowBnt(int64 ID,const TCHAR* Text):CVSpace2(ID),m_Text(Text){};
	virtual ~ArrowBnt(){};
	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};



class PageBnt : public CVSpace2{
public:
	tstring m_Text;
public:
	PageBnt(){};
	PageBnt(int64 ID,const TCHAR* text);
	virtual ~PageBnt(){};

	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
};

class CFindToolbar: public CVSpace2{

public:
	CFindToolbar();
	virtual ~CFindToolbar(){};
 	virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
   
	void Reset();  //让所有Bnt无效
};

#define FIND_ADDRESULT         0
#define FIND_CLRRESULT         1
#define FIND_END               3
#define FIND_NOTIFYUPDATE      4

class CFindView  :public CWSTreeView 
{
protected:
	int64             m_SourceID;
	int64             m_LocalID;
	
	tstring           m_FindTip;
    HFONT             m_Font;
	COLORREF          m_crText;
	int32             m_Padding;
	
	CFindToolbar      m_Toolbar;
	int32             m_ItemNumPerPage;
	int32             m_PageCount;
    int32             m_CurPage;
    BOOL              m_bFindEnd;
protected:
    virtual void     Draw(HDC hDC, ePipeline* Pipe=NULL);

	virtual CVSpace2* Hit(int32 x, int32 y); //TreeViewport只处理HotList里Item

	LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void  Layout(bool Redraw =true);
	void    UpdateToolbar();
protected:
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	LRESULT OnAddResult(WPARAM wParam, LPARAM lParam);
	LRESULT OnClearResult(WPARAM wParam, LPARAM lParam);
	LRESULT OnFindEnd(WPARAM wParam, LPARAM lParam);

public:
	CFindView();
	virtual ~CFindView();
	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName );
	void SetFindTip(tstring Tip);

	void AddText(int32 Index, int64 RoomID,const TCHAR* Text);
	void AddObjectOrPeople(int32 Index, int64 RoomID,const TCHAR* Text, const TCHAR* Memo);
	void AddCommandOrLogic(int32 Index, int64 RoomID,const TCHAR* Text, const TCHAR* Memo);
	void ClearAll();

	void FindEnd(BOOL IsAllEnd); //当前搜索已经全部完成
};


#endif // !defined(_BRAINFINDVIEW_H__)
