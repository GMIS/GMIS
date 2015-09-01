/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _OUPUTWIN_H__
#define _OUPUTWIN_H__


#include "PhysicSpace.h"
#include "VisibleSpace.h"
#include "SubclassWnd.h"
#include <richedit.h>
#include "DialogView.h"
#include "FindView.h"
#include "LogicView.h"
#include "ObjectView.h"
#include "GuiGadget.h"
#include "LineNumEdit.h"
#include "DebugView.h"
#include "MemoryView.h"

#include "LocalObjectView.h"

#define BNT_DIALOG    11
#define BNT_OUTPUT    12
#define BNT_MEMORY    13
#define BNT_DEBUG     14
#define BNT_LOGIC     15
#define BNT_OBJECT    16
#define BNT_FIND      17

#define BNT_VIEWLOCK  20

#define BNT_THINKRESULT   11
#define BNT_ANALYSERESULT 12

enum OUTWINTYPE {
		DIALOG_VIEW   = BNT_DIALOG,
   	    OUTPUT_VIEW   = BNT_OUTPUT,
		MEMORY_VIEW   = BNT_MEMORY,
		DEBUG_VIEW    = BNT_DEBUG,
		LOGIC_VIEW    = BNT_LOGIC,
		OBJECT_VIEW   = BNT_OBJECT,
		FIND_VIEW     = BNT_FIND,
};

#define CMD_SHOW_VIEW  100
#define CMD_DELALLVIEW 101

class COutputWin : public CWinSpace2
{
protected:
	BOOL            m_Created;

	CVSpace2*       m_SpaceSelected;
	SplitterStruct  m_Splitter;

public:

	CDialogView      m_DialogView;  //显示历史对话
	
 	CLineNumEdit     m_OutputView;  //显示逻辑执行结果

	CDebugView       m_DebugView;   //显示DEBUG执行树，与m_ThinkView共享按钮

//	CSceneWin        m_ThinkView;   //显示思考状态

	CMemoryView      m_MemoryView;  //显示数据和外部物体实例

    CLogicView       m_LogicView;   //显示逻辑

	CLocalObjectView m_ObjectView;  //物体

	CFindView        m_FindView;    //显示预测联想和搜索结果
	

public:
	COutputWin();
	virtual ~COutputWin();

	void HideAllView();
    void ShowView(int64 ViewID,BOOL bShow);
	BOOL HasView();
protected:
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);

	virtual void Layout(bool Redraw = true);
    virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);

	LRESULT OnCreate(WPARAM wParam,LPARAM lParam);
	LRESULT OnSize(WPARAM wParam,LPARAM lParam);

	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(_OUPUTWIN_H__)
