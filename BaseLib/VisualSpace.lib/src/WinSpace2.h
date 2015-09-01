/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _WINSPACE2_H__
#define _WINSPACE2_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include "ABMutex.h"

#define  TIMER_MOUSELEAVE   889         //检测光标位置，决定是否取消FocuseSpace的Focuse状态       
#define  TIMER_SPACEWANING  888         //实现子空间的警告闪烁


//整个GUI系统只占用这三个消息
#define  WM_SPACEWANING      WM_USER-1  
#define  WM_PARENTRECTION    WM_USER-2	
#define  WM_CHILDRECTION     WM_USER-3	

#define RectWidth(rc)    (rc.right-rc.left)
#define RectHeight(rc)   (rc.bottom-rc.top)

namespace VISUALSPACE{

static LRESULT CALLBACK	SpaceDefaultProc(HWND, UINT, WPARAM, LPARAM);

class CWinSpace2;

	struct  SpaceRectionMsg 
	{
		UINT                   Msg;
		int64                  wParam;
		int64                  lParam;
		CVSpace2*               ChildAffected; 
		CWinSpace2*            WinSpace; 
	};

	/*
	CWinSpace2对于它的父空间来说与CVSpace2一样，但对它的子空间来说它
	与普通空间有两点不同：
	1 它具有独自接受外力的能力，通过函数Rection()来实现
	2 其所有子空间的坐标将基于它，而CVSpace2的坐标及其子空间的坐标都基于其
    根部最近的那个CWinSpace2。
	
	  CWinSpace2缺省做以下几件事，它们在SpaceCommonProc里被实现
	  1 鼠标移动时确定Focus Space，即当前鼠标停留在那个子空间上（当然只可能是CVSpace2空间），
	  同时启动一个Timer检查鼠标是否离开这个子空间
	  2 允许用户指定一个或多个空间具有警告状态，CWinSpace2会启动一个Timer周期性的取消或
	  恢复这种状态，根据这种状态我们可以闪烁某个Space2或做其它周期性的事。
	  3 用ParentReaction()处理子空间传来的信息，用ChildReaction接受父空间传来的信息

    NOTE: 用户必须自己初始化 m_UILock
	*/

#define SPACE_SHOWWINBORDER    0x00000020
#define SPACE_SHOWCLIENTBORDER 0x00000040 
#define SPACE_SHOWBKG          0x00000080

	class  CWinSpace2 : public CVSpace2      
	{
		friend static LRESULT CALLBACK	SpaceDefaultProc(HWND, UINT, WPARAM, LPARAM);
	private:
		//user interface lock,由于我们统一使用SpaceDefaultProc,我们可以使其为线程安全的
		static CABMutex*     m_UILock;     

		//但为了避免SpaceDefaultProcc自我调用带来的死锁，我们登记线程ID，同一个线程可以忽略m_UILock
		DWORD                m_CurThreadID; 
	public:
		static BOOL	   RegisterCommonClass(HINSTANCE hInstance);

		COLORREF          m_crViewBkg;
		COLORREF          m_crWinBorder; 

	protected:
		HWND               m_hWnd;             

		
		CVSpace2*          m_SpaceFocused;      //鼠标经过某条目
		vector<CVSpace2*>  m_SpaceWarningList;
		
		BOOL               m_XYBasedWindows;  //default = true;
	public:
		CWinSpace2();
		virtual ~CWinSpace2();
		
		static void InitUIMutex(CABMutex* Mutex);
		
		BOOL Create(HINSTANCE AppInstance, LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, HWND pParentWnd, UINT nID =0, DWORD dwExStyle=0 );
		HWND GetHwnd(){ return m_hWnd;}
		void Destroy(){ assert(::IsWindow(m_hWnd)); ::DestroyWindow(m_hWnd); m_hWnd=NULL;}; 
		
		inline void Invalidate(BOOL bErase=TRUE){
			if(m_hWnd)::InvalidateRect(m_hWnd, NULL, bErase);
		};
		
		//把Space的坐标转换成基于窗口
		RECT GetWinArea(CVSpace2* Space);

		static HINSTANCE GetHinstance(){ return (HINSTANCE)::GetModuleHandle(NULL);}
        
		//处理WM_PARENTRECTION,空间之间的相互作用将统一由以下两个函数完成
		virtual LRESULT ParentReaction(SpaceRectionMsg* SRM){
			return 0;
		};
        
		//处理WM_CHILDRECTION
		virtual LRESULT ChildReaction(SpaceRectionMsg* SRM){
			return 0;
		};

       virtual LRESULT SendParentMessage(UINT Msg,int64 wParam, int64 lParam,CVSpace2* Space){
			HWND parent = GetParent(m_hWnd);
			if(parent==NULL)return 0;
			SpaceRectionMsg SRM;
			
			SRM.Msg = Msg;
			SRM.wParam = wParam;
			SRM.lParam = lParam;
			SRM.ChildAffected = Space;
			SRM.WinSpace = this;
			return ::SendMessage(parent,WM_PARENTRECTION,(WPARAM)&SRM,0);
		}

		virtual LRESULT SendChildMessage(HWND Child,UINT Msg,int64 wParam, int64 lParam){
			//assert(Child);
			SpaceRectionMsg SRM;
			
			SRM.Msg = Msg;
			SRM.wParam = wParam;
			SRM.lParam = lParam;
			SRM.ChildAffected = NULL;
			SRM.WinSpace = this;			
			return ::SendMessage(Child,WM_CHILDRECTION,(WPARAM)&SRM,0);
		}
		/*
		  HitTest()用于被测试，由于每一个窗口拥有独立坐标体系，因此不能用一个父窗口
		  的坐标去测试它的子窗口。此时应该由子窗口Hit()去做主动测试
		*/

		virtual CVSpace2*  HitTest(int32 x,int32 y){return NULL; }		
		virtual CVSpace2*  Hit(POINT& point);

		virtual void    FocuseChanged(CVSpace2* OldSpace,POINT& point); //NewSpace = m_FocuseSpace; 
	
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);

		void SetSpaceWarning(CVSpace2* Space,BOOL Warning){
			::SendMessage(m_hWnd,WM_SPACEWANING,(WPARAM)Space,(LPARAM)Warning);
		}
		bool IsWarning(CVSpace2* Space); //必须检查m_SpaceWaningList才知道
		void CancelAllWarning(){
			::SendMessage(m_hWnd,WM_SPACEWANING,(WPARAM)0,(LPARAM)0);
		}
		void Clear(){ DeleteAll(); Layout();}
	};
	

//draw  tools
////////////////////////////////////////////////////////////////////////////////
	
	inline  void FillRect(const HDC& dc,const RECT* rc, const COLORREF& cr)                                   
	{                                                            
		::SetBkColor(dc, cr);                                    
		::ExtTextOut(dc, 0, 0, ETO_OPAQUE, rc, NULL, 0, NULL);  
	}                                                            
	inline  void FillRect(const HDC& dc,const RECT& rc, const COLORREF& cr){
		FillRect(dc,&rc,cr);
	}
	
	inline void  DrawEdge(const HDC& dc,const RECT* rc, const COLORREF& cr)                                                                \
	{                                                                                         
		COLORREF old = ::SetBkColor(dc, cr);                                                                  
		RECT rect;                                                                             
		rect.left=rc->left; rect.top = rc->top; rect.right=rc->right; rect.bottom=rc->top+1;      
		::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);                                
		rect.left=rc->left; rect.top = rc->top; rect.right=rc->left+1; rect.bottom=rc->bottom;    
		::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);                             
		rect.left=rc->right-1; rect.top = rc->top; rect.right=rc->right; rect.bottom=rc->bottom;  
		::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);                                
		rect.left=rc->left; rect.top = rc->bottom-1;rect.right=rc->right; rect.bottom=rc->bottom; 
		::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);                             
		::SetBkColor(dc,old);
	}                                                                                         
	inline void  DrawEdge(const HDC& dc,const RECT& rc, const COLORREF& cr)                                                                \
	{
		DrawEdge(dc,&rc,cr);
	}
	inline void DeflateRect(RECT* rc,int32 l, int32 t,int32 r,int32 b){
		rc->left +=l; rc->right -=r;  rc->top +=t;  rc->bottom-=b;
	}
	
	void FillTile(HDC hDC,HANDLE BmpImage,RECT* rcArea);
	void FillTile(HDC hDC,HANDLE BmpImage,RECT& rcArea);
	/*
	类似于把指定透明率和指定颜色的玻璃覆盖到dcDest上,alpha=255时dcDest将被指定颜色完全填充

	修改与《TransparentBlt() and AlphaBlend() for Windows 95 》
	By Christian Graus,  Dr. Dobb's Journal 
	m. 13, 2002 
	URL:http://www.ddj.com/dept/windows/184416353 
	*/
	bool AlphaBlendGlass(HDC dcDest, int x, int y, int cx, int cy,COLORREF crGlass, int alpha);
    
	/*使填充的crFill颜色呈现明暗效果，IsVer表示是否垂直还是水平绘制，alpha是与白色和暗色部分混合因子
	*/
    void FillRectGlass(HDC hDC,RECT* rcArea,COLORREF crFill, BOOL IsVer=FALSE,int alpha =127);
    void FillRectGlass(HDC hDC,RECT& rcArea,COLORREF crFill, BOOL IsVer=FALSE,int alpha =127);

} //NAME VISUALSPACE;

#endif //_WINSPACE2_H__
