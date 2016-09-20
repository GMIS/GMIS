/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LINKERVIEW_H__
#define _LINKERVIEW_H__

#include "WSListView.h"
#include "LogicDialog.h"


#define ADD_DIALOG         1
#define DEL_DIALOG         2
#define CLR_LINKER         3
#define SET_FOCUSDIALOG    4
#define NOTIFY_DIALOG      5



using namespace VISUALSPACE;


class CLinkerView : public CWSTreeView  
{
public:
	class CLinkerItem: public CWSTreeView::CTreeItem{
	public:
		CLinkerItem(int64 ID, const tstring& Name);
        virtual ~CLinkerItem(){};

		virtual void Draw(HDC hDC,ePipeline* Pipe /* = NULL */);
	};

private:
	int32     m_DialogCounter;
    CVSpace2* m_OldSelected;
public:
	CLinkerView();
	virtual ~CLinkerView();
	
	void AddDialog(int64 SourceID,int64 DialogID,int64 ParentID,tstring Name);
	void DeleteDialog(int64 SourceID,int64 DialogID);
    void NotifyDialogHasNew(int64 SourceID,int64 DialogID);
	void EnableDialog(int64 SourceID,int64 DialogID,bool bEnable);
	void  SetCurDialog(int64 SourceID, int64 DialogID);
	CLinkerItem* GetCurDialog();

	void ClearAll();

    CLinkerItem* FindLinker(int64 SourceID);

protected:

	tstring GetDefaultName();

	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(_LINKERVIEW_H__)
