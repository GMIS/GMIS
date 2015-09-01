/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _SPLITTER_H__
#define _SPLITTER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GUIElementMisc.h"

namespace VISUALSPACE{

//封装给窗口安装Splitter所需的一些基本行为	 
class  SplitterStruct : public CVSpace2{
public:
	CWinSpace2*   OwnerWin;
	HANDLE        m_ImageSkin;

    //注意：m_Area包含所有splitter view所占面积
	bool          IsVertical;       //splitter bar是垂直还是水平放置,缺省为垂直放置
	int32         SplitterWidth;    //水平放置时为高度
	int32         MinViewWidth;
	RECT          rcAllView;     
	SplitterItem  TempDragItem;     //拖动时作为原始条目的替身
private:
	SplitterStruct(){};
	void DrawTracker(CVSpace2* Space);

public:
	SplitterStruct(CWinSpace2* Win,int32 W, int32 MinW,bool Vertical=true,HANDLE Image=NULL)
		:OwnerWin(Win),SplitterWidth(W),MinViewWidth(MinW),IsVertical(Vertical),m_ImageSkin(Image){Clear();};

	virtual SplitterItem* CreateSplitterItem(){
		if(m_ImageSkin)return new SplitterItem(ID_SPLITTER,m_ImageSkin);
		return new SplitterItem(ID_SPLITTER);
	}
	void OnMouseMove(int32 x,int32 y,SplitterItem* Item); //OwnerWin.WM_MOUSEMOVE
	void OnLButtonDown(SplitterItem* Item);                 //OwnerWin.WM_LBUTONDOWN
	void OnLButtonUp(SplitterItem* Item);                 //OwnerWin.WM_LBUTONUP
	
	//bOrderInsert决定是否按View->m_Alias顺序插入
	void AddView(CWinSpace2* View,bool bOrderInsert=false,DWORD Style=WS_CHILD|WS_VISIBLE); //push_back
	CWinSpace2*  DeleteView(int32 ViewID);  //返回被删除的那个（Windows没有被destroy）
	//返回被替换的VIEW
    CWinSpace2* ReplaceView(int32 ViewID,CWinSpace2* NewView,DWORD Style=WS_CHILD|WS_VISIBLE); 
	void  Clear();
	void Reset();                   //每个View均分OwnerSpace面积     
    void Layout(RECT& rcClient);    //改变rcClient后调用此函数按比例调整各view 
    void Hide();  //隐藏所有窗口
	void SetFirstSplitterPos(int32 FirsPos);  //第一个Splitter的起始位置,只有窗口位置已经分配好后才能用此改变splitter位置
	int32 GetFirstSplitterPos();


};


}//namespace VISUALSPACE

#endif // !defined(_SPLITTER_H__)
