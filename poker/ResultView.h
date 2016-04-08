// ResultView.h: interface for the CResultView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESULTVIEW_H__E64C78B5_0E31_4AF2_A6AF_FE8D861EAD97__INCLUDED_)
#define AFX_RESULTVIEW_H__E64C78B5_0E31_4AF2_A6AF_FE8D861EAD97__INCLUDED_

#include "TheorySpace.h"
#include "WSListView.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define RESULT_ADD_INFO       100
#define RESULT_ADD_POKER      200

using namespace VISUALSPACE;

class CResultView : public CWSListView  
{
public:
	/*显示格式：
	  得分：1234
    */
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

	/*显示格式
	 1手：牌面  系数  操作
          牌面图像
	*/
	class ImageItem : public CVSpace2{
	public:
		static  HANDLE  hPokerImage;		
		tstring         m_Text;
		vector<int32>   m_CardList;
	public:
		ImageItem(){
		};
		ImageItem(int64 ID,const TCHAR* text);
		virtual ~ImageItem();
		
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};	

protected:
	int32  m_MaxItemCount; //DEFUALT=200
public:
	CResultView();
	virtual ~CResultView();

    void AddInfo(ePipeline* Msg);
protected:
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);	

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(AFX_RESULTVIEW_H__E64C78B5_0E31_4AF2_A6AF_FE8D861EAD97__INCLUDED_)
