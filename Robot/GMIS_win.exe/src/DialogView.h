/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LOGVIEW_H__
#define _LOGVIEW_H__

#include "PhysicSpace.h"
#include "WSListView.h"
#include <map>


using namespace VISUALSPACE;

enum MSG_STATE{
	BEGIN_PROCESS,
	END_PROCESS,
	WAIT_SEND,
	SENDING,
	SEND_DONE
};

#define ADD_INFO     111
#define CLR_INFO     112
#define SET_DIALOG   113
#define ADD_MORE     114

class CDialogView : public CWSListView  
{
public:
	class InfoItem : public CVSpace2{
	public:
		tstring  m_Text;
	public:
		InfoItem(){
		};
		InfoItem(int64 ID,const TCHAR* text);
		virtual ~InfoItem(){};
		
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};


protected:
    virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);

	void DrawItemHeader( HDC hDC, CVSpace2* Space){};

public:
	CDialogView();
	virtual ~CDialogView();

	void SetDialogList(ePipeline& Pipe);
	void AddDialogItem(ePipeline& Item);
	void AddMoreDialogItem(ePipeline& Pipe);

	void ClearAllItem();

};

#endif // !defined(AFX_LOGVIEW_H__6C815E08_3961_4421_9A54_1643B87C15ED__INCLUDED_)
