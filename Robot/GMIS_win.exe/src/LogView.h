/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "WSFrameview.h"
#include "WSFormView.h"
#include "LineNumEdit.h"
#include "LogDatabase.h"

#define BNT_UPDATE  297
#define BNT_CLOSE   298
#define BNT_CLEAR   299

#define LOG_TIMER 20001

class CLogView : public CWSFrameView
{

	class CFooterBar: public CWinSpace2
	{
	public:
		class ControlBnt : public ButtonItem
		{	
		public:
			ControlBnt(int64 ID,const TCHAR* Name)
				:ButtonItem(ID,Name,NULL,false){
			};
			virtual ~ControlBnt(){};
			virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
		};
	public:
		ControlBnt        m_UpdateBnt;
		ControlBnt        m_ClearBnt;
		ControlBnt        m_CloseBnt;

		CVSpace2*         m_SpaceSelected;

	public:
		CFooterBar();
		virtual ~CFooterBar();
		virtual  void    Layout(bool Redraw = true);
	protected:
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		LRESULT OnPaint();
		LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

	};
	void  UpdateItem();
public:

	uint32         m_LogFlag;
	int64          m_LastItemTimeStamp;
	CLogDatabase   m_LogDB;

	CLineNumEdit   m_LogEdit;
	CWSFormView    m_OptionView;
	CFooterBar     m_FooterBar;
public:
	CLogView(void);
	~CLogView(void);
	
	void ResetFlag();
	void UpdateFlag();

protected:

	virtual void  Layout(bool Redraw = true);
	virtual LRESULT  ParentReaction(SpaceRectionMsg* SRM);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);

};

