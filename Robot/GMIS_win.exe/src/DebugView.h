/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _DEBUGWIN_H__
#define _DEBUGWIN_H__



#include "LogicTask.h"
#include "WSTreeView.h"
//#include "hightime.h"

using namespace VISUALSPACE;
BOOL    SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName); 

#define CM_TASK_CONTROL   123

#define  SPACE_PAUSE  0x00004000  
#define  SPACE_BREAK  0x00008000  

#define  IDM_CAPTURE_WIN          131
#define  IDM_CAPTURE_TREE         132
#define  IDM_CAPTURE_TREE_OPENED  133

class CDebugView : public CWSTreeView   
{

	class MassItem : public CVSpace2{
	public:
		tstring          m_Name;
		int32            m_Type;
		uint32           m_MassState;
	public:
		MassItem(){
			SetAreaSize(250,25);
		};
		MassItem(int64 ID,tstring Name):CVSpace2(ID),m_Name(Name){
			SetAreaSize(250,25);
		};
		virtual ~MassItem(){};
		
		void Init(ePipeline& Pipe);

		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
		
	};

	void  Item2Path(CVSpace2* Item,ePipeline& Path);
    CVSpace2* Path2Item(ePipeline& Path);


	void OpenAllTree(CVSpace2* Parent=NULL);
public:	

	int64		  m_TaskTimeStamp;  //标识视图是否需要更新

protected:
	void Layout(bool Redraw =true);
	void Draw(HDC hDC, ePipeline* Pipe=NULL);

	void Reset(ePipeline& Pipe);
	void MarkPauseItem(int64 PauseItemID);

	void InsertDebugItem(ePipeline& InsertAddress,ePipeline& DebugItemList);
	void RemoveDebugItem(ePipeline& RemoveAddress,int64 ChildID);
	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);

	LRESULT OnCaptrueWin();
	LRESULT OnCaptrueTree();
	LRESULT OnCaptrueTreeOpened();
protected:
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	LRESULT OnInfoProc(WPARAM wParam, LPARAM lParam);

	//设置按钮状态
	void SetStepBnt(bool bStep);
	void SetRunBnt(bool bRun);

	//BreakFlag 不改变=0 设置BreakPoint=1 取消BreakPoint=2
	void SetBreakBnt(bool bBreak,int32 BreakFlag);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam); 
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
public:
	CDebugView();
	virtual ~CDebugView();

	void InfoProc(ePipeline& Pipe);


};




#endif // !defined(_DEBUGWIN_H__)
