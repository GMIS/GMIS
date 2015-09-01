// WinSpace2.cpp: implementation of the CWinSpace2 class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "WinSpace2.h"
#include <algorithm>
#include <map>

#define      SPACE_CLASSNAME    _T("WinSpace")

namespace VISUALSPACE{

//////////////////////////////////////////////////////////////////////
// CWinSpace2 Class
//////////////////////////////////////////////////////////////////////


BOOL CWinSpace2::RegisterCommonClass(HINSTANCE hInstance)
{
    WNDCLASS WndClass;
	if (::GetClassInfo(hInstance, SPACE_CLASSNAME, &WndClass))return TRUE;
	
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	
	wcex.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW |CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)SpaceDefaultProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= SPACE_CLASSNAME;
	wcex.hIconSm		= NULL;
	
	return RegisterClassEx(&wcex);
};


void CWinSpace2::InitUIMutex(CABMutex* Mutex){
	m_UILock = Mutex;
};

LRESULT CALLBACK SpaceDefaultProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	class ActiveSpaceList  
	{
		CRITICAL_SECTION             m_CS;
		std::map<HWND,CWinSpace2*>   m_SpaceList;
	public:
		ActiveSpaceList(){::InitializeCriticalSection (&m_CS);};
		virtual ~ActiveSpaceList(){::DeleteCriticalSection (&m_CS); };
		
		CWinSpace2* GetSpace(HWND hwnd){
            ::EnterCriticalSection(&m_CS);
			map<HWND,CWinSpace2*>::iterator It = m_SpaceList.find(hwnd);
			CWinSpace2* Win = It==m_SpaceList.end()?NULL:(It->second);
			::LeaveCriticalSection (&m_CS);
			return Win;
		}
		void AddSpace(HWND hwnd,CWinSpace2* Space){
			::EnterCriticalSection(&m_CS);
			m_SpaceList[hwnd]=Space;
			::LeaveCriticalSection (&m_CS);
		}
		void     Delete(HWND hwnd){
			::EnterCriticalSection(&m_CS);
			map<HWND,CWinSpace2*>::iterator It = m_SpaceList.find(hwnd);
			if(It != m_SpaceList.end())m_SpaceList.erase(It);
			::LeaveCriticalSection (&m_CS);
		}
	};
	
	//线程不一致才使用，一致则不使用
	class UILock 
	{
		CABMutex * m_Mutex;
		BOOL       m_bUse;
	public:
		UILock( CABMutex* mutex,BOOL bUse): m_Mutex(mutex),m_bUse(bUse){

			assert(mutex);
			if(bUse){
				mutex->Acquire();
			} 
		}
		~UILock (){
			if(m_bUse){
				m_Mutex->Release();
			} 
		}
	};

	static ActiveSpaceList SPACELIST;
	
	CWinSpace2* ThisSpace = SPACELIST.GetSpace(hWnd);
	if (ThisSpace == NULL)
	{
		if (message == WM_NCCREATE)
		{	
			assert(ThisSpace == NULL);
			CREATESTRUCT* cst = (CREATESTRUCT* )lParam;
            ThisSpace = (CWinSpace2*)cst->lpCreateParams;  
			ThisSpace->m_hWnd = hWnd;
			assert(ThisSpace != NULL);
            SPACELIST.AddSpace(hWnd,ThisSpace);
		}else{
			return ::DefWindowProc(hWnd,message,wParam,lParam);		
		}
	}

	DWORD ThreadID = ::GetCurrentThreadId();
	
	UILock lk(ThisSpace->m_UILock,ThreadID != ThisSpace->m_CurThreadID);
    
	ThisSpace->m_CurThreadID = ThreadID; //登记当前使用资源的线程ID

	switch (message) 
	{

	case WM_MOUSEMOVE:
		{	
			POINT point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			
			CVSpace2 * HitSpace = ThisSpace->Hit(point);
			assert(ThisSpace != HitSpace);
			if(HitSpace != ThisSpace->m_SpaceFocused){
				CVSpace2* OldSpace = ThisSpace->m_SpaceFocused;
				ThisSpace->m_SpaceFocused = HitSpace;
				if(HitSpace){
					HitSpace->m_State |= SPACE_FOCUSED;				
					::KillTimer(ThisSpace->m_hWnd,TIMER_MOUSELEAVE);
					//当鼠标离开当前窗口后以便有机会通知鼠标已经离开原SpaceFocused
					::SetTimer(ThisSpace->m_hWnd,TIMER_MOUSELEAVE,250,NULL);
				}
				ThisSpace->FocuseChanged(OldSpace,point); 
			}
			return ThisSpace->Reaction(message,wParam,lParam);
		}
	case WM_TIMER:
		{
			
			long nIDEvent = wParam;
			if(nIDEvent == TIMER_MOUSELEAVE){
				if(ThisSpace->m_SpaceFocused){
					POINT pt;
					::GetCursorPos(&pt);
					::ScreenToClient(ThisSpace->m_hWnd,&pt);
					RECT rc = ThisSpace->GetWinArea(ThisSpace->m_SpaceFocused);

					if(::PtInRect(&rc,pt)){
						return 0;
					}
					CVSpace2* OldSpace = ThisSpace->m_SpaceFocused;
					ThisSpace->m_SpaceFocused = NULL;
					ThisSpace->FocuseChanged(OldSpace,pt);  
				} 
				else ::KillTimer(ThisSpace->m_hWnd,nIDEvent);
			}
			else if(nIDEvent == TIMER_SPACEWANING){
				vector<CVSpace2*>::iterator It = ThisSpace->m_SpaceWarningList.begin();
				while(It != ThisSpace->m_SpaceWarningList.end()){
					CVSpace2* WarningSpace = *It;	
					if(WarningSpace->m_State & SPACE_WARNING)WarningSpace->m_State &= ~SPACE_WARNING;
					else WarningSpace->m_State |= SPACE_WARNING;
					It++;
				}
				ThisSpace->Invalidate();
			}
			else return ThisSpace->Reaction(message,wParam,lParam);
			
		}
		break;
	case WM_SPACEWANING:
		{
			CVSpace2* WarningSpace  = (CVSpace2*)wParam;
			BOOL     Insert = (BOOL)lParam;
			
			if(Insert==0 && WarningSpace == 0){ //delete all
				vector<CVSpace2*>::iterator It = ThisSpace->m_SpaceWarningList.begin();
				while(It < ThisSpace->m_SpaceWarningList.end()){
					WarningSpace = *It;
					assert(WarningSpace);
					WarningSpace->m_State &= ~SPACE_WARNING;
					It++;
				}
				ThisSpace->m_SpaceWarningList.clear();
				::KillTimer(ThisSpace->m_hWnd,TIMER_SPACEWANING);
				return 0;
			}
			if(Insert){
				assert(WarningSpace);
				ThisSpace->m_SpaceWarningList.push_back(WarningSpace);
				if(ThisSpace->m_SpaceWarningList.size()==1){
					::SetTimer(ThisSpace->m_hWnd,TIMER_SPACEWANING,400,NULL);
				}
			}
			else{
				assert(WarningSpace);
				vector<CVSpace2*>::iterator It;
				It = find(ThisSpace->m_SpaceWarningList.begin(),ThisSpace->m_SpaceWarningList.end(),WarningSpace);
				if(It == ThisSpace->m_SpaceWarningList.end())return 0;
				WarningSpace->m_State &= ~SPACE_WARNING;
				ThisSpace->m_SpaceWarningList.erase(It); 
				if(ThisSpace->m_SpaceWarningList.size()==0){
					::KillTimer(ThisSpace->m_hWnd,TIMER_SPACEWANING);
				}
			}

			ThisSpace->Invalidate();
			return 0;
		}
	case WM_PARENTRECTION:
		{	
			return ThisSpace->ParentReaction((SpaceRectionMsg*)wParam);
		}
		break;
	case WM_CHILDRECTION:
		{
			return ThisSpace->ChildReaction((SpaceRectionMsg*)wParam);
		}
		break;
	case WM_CREATE:
		{
			ThisSpace->Reaction(message,wParam,lParam);
		}
        break;
	case WM_NCCREATE:
		{		
			return ThisSpace->Reaction(message,wParam,lParam);		
		}
		break;
//	case WM_CLOSE:
//		::PostQuitMessage(0);
//		break;
	case WM_DESTROY:
		{
			if (ThisSpace->GetHwnd()==NULL)
			{
				return 1;
			}
			assert(ThisSpace->GetHwnd() == hWnd);
			ThisSpace->CancelAllWarning();
			ThisSpace->Reaction(message,wParam,lParam);
			SPACELIST.Delete(hWnd);
			HWND Parent = GetParent(hWnd);
			if (Parent == NULL)
			{
				::PostQuitMessage(0);
			}else{
				ThisSpace->SendParentMessage(WM_DESTROY,0,0,ThisSpace);
			}
			ThisSpace->m_hWnd = NULL;
			return 1;
		}
		break;
	default:
		{
			return ThisSpace->Reaction(message,wParam,lParam);
			
		}
   }
   return 0;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinSpace2::CWinSpace2()
{
    m_crViewBkg      = RGB(0xf5,0xf5,0xf3);
    m_crWinBorder    = RGB(128,128,128);	
	
	m_State    |= SPACE_SHOWWINBORDER;
	m_State    |= SPACE_SHOWBKG;
	m_SpaceFocused   = NULL;
	m_hWnd           = NULL;
	m_CurThreadID    = NULL;
	m_XYBasedWindows = TRUE;
}

CWinSpace2::~CWinSpace2()
{
	
}

CABMutex* CWinSpace2::m_UILock = NULL;

CVSpace2*  CWinSpace2::Hit(POINT& point){		
	CVSpace2* Ret = NULL;
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while(It != m_ChildList.end()){
		Ret = (*It)->HitTest(point.x,point.y); 	
		if(Ret)return Ret;
		It++;
	};
	return Ret;
}		

RECT CWinSpace2::GetWinArea(CVSpace2* Space){
    RECT rc = Space->GetArea();;
	if(m_XYBasedWindows){
		return rc;
	};

	CVSpace2* Child = Space;
	while (Child != this)
	{
		CVSpace2* Parent = Space->m_Parent;
		rc.left   += Parent->m_AreaLeft;
		rc.right  += Parent->m_AreaLeft;
		rc.top    += Parent->m_AreaTop;
		rc.bottom += Parent->m_AreaTop;
		Child = Parent;
	}

	//int32 y = GetVScrollPos();
	//int32 x = GetHScrollPos();

    assert(0); //未完成
	return  rc;
}

     
BOOL CWinSpace2::Create(HINSTANCE AppInstance, LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, HWND pParentWnd, UINT nID, DWORD dwExStyle /*=0*/)
{
	m_hWnd = ::CreateWindowEx(dwExStyle, SPACE_CLASSNAME, lpszText, dwStyle,
		rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,
		pParentWnd,
		(HMENU)nID,
		AppInstance,
		this);
	int n = GetLastError();
	return m_hWnd!=NULL;
}

bool CWinSpace2::IsWarning(CVSpace2* Space){
	vector<CVSpace2*>::iterator It;
	It = find(m_SpaceWarningList.begin(),m_SpaceWarningList.end(),Space);
	return It != m_SpaceWarningList.end();
};

void CWinSpace2::FocuseChanged(CVSpace2* OldSpace,POINT& point){
	if(OldSpace){
		OldSpace->m_State &= ~SPACE_FOCUSED;
	}
	Invalidate();
}

LRESULT CWinSpace2::OnPaint(WPARAM wParam, LPARAM lParam){

	RECT rcWin; 
	GetClientRect(m_hWnd,&rcWin);

	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(rcWin.right==rcWin.left || rcWin.top==rcWin.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}

	if(m_State & SPACE_SHOWBKG){
		FillRect(hdc,rcWin,m_crViewBkg);
	}

	Draw(hdc);

	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(hdc,rcWin,m_crWinBorder);// Border
	}		

	::EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT CWinSpace2::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
    if (message == WM_PAINT)
    {
		return OnPaint(wParam,lParam);
    }
	else if(message ==WM_WINDOWPOSCHANGED){
		SendParentMessage(message,wParam,lParam,NULL);
	}
	return ::DefWindowProc(m_hWnd,message,wParam,lParam);
}


void FillTile(HDC hDC,HANDLE BmpImage,RECT* rcArea){
	    if(BmpImage==NULL)return;

        RECT rc = *rcArea;  
		HDC DCMem = ::CreateCompatibleDC(hDC);
		
		BITMAP bmp;
		::GetObjectW(BmpImage, sizeof(BITMAP), &bmp); 
		HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, BmpImage );
		
		
		int w = bmp.bmWidth;
		int h = bmp.bmHeight;
		
		int x1 = RectWidth(rc)+rc.left;
		int y1 = RectHeight(rc)+rc.top;

		for (int y = rc.top; y <y1; y += h)
		{
			if((y+h)>y1){
				h=y1-y;
			}
			for (int x = rc.left; x <x1; x += w)
			{
				if((x+w)>x1){
					w=x1-x;
				}
				::BitBlt(hDC,x, y, w,h, DCMem,0, 0, SRCCOPY);
			}
			w = bmp.bmWidth;
		}
		::SelectObject(DCMem,OldBitmap);
		::DeleteDC(DCMem);
}
	
void FillTile(HDC hDC,HANDLE BmpImage,RECT& rcArea){
		FillTile(hDC,BmpImage,&rcArea);	
};

bool AlphaBlendGlass(HDC dcDest, int x, int y, int cx, int cy,COLORREF crGlass, int alpha)
{
	alpha = (unsigned char)alpha;

    BITMAPINFOHEADER BMI;
    // Fill in the header info.
    BMI.biSize = sizeof(BITMAPINFOHEADER);
    BMI.biWidth = cx;
    BMI.biHeight = cy;
    BMI.biPlanes = 1;
    BMI.biBitCount = 32;
    BMI.biCompression = BI_RGB;   // No compression
    BMI.biSizeImage = 0;
    BMI.biXPelsPerMeter = 0;
    BMI.biYPelsPerMeter = 0;
    BMI.biClrUsed = 0;           // Always use the whole palette.
    BMI.biClrImportant = 0;
    


    BYTE * pDestBits;
    HBITMAP hbmDest;
    // Create DIB section in shared memory
    hbmDest = CreateDIBSection (dcDest, (BITMAPINFO *)&BMI,
        DIB_RGB_COLORS, (void **)&pDestBits, 0, 0l);
    
    BYTE * pGlassBits;
    HBITMAP hbmGlass;
    // Create DIB section in shared memory
    hbmGlass = CreateDIBSection (dcDest, (BITMAPINFO *)&BMI,
        DIB_RGB_COLORS, (void **)&pGlassBits, 0, 0l);

   
    //先把Dest的指定部分copy到hbmDest
    HDC dc = CreateCompatibleDC(NULL);
    
    HBITMAP dcOld = (HBITMAP) SelectObject(dc, hbmDest);
    
    if (!StretchBlt(dc, 0, 0, cx, cy, dcDest, x, y,cx, cy, SRCCOPY)){
        DeleteObject(hbmGlass);    
        DeleteObject(hbmDest);    
        return false;
	}

    SelectObject(dc, hbmGlass);
    //再把玻璃颜色填充到hbmGlass		
	::SetBkColor(dc, crGlass); 
	RECT rc;
	::SetRect(&rc,0,0,cx,cy);
	::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);  

	int a = 255-alpha;
    for (int j = 0; j < cy; ++j)
    {
        LPBYTE pbDestRGB = (LPBYTE)&((DWORD*)pDestBits)[j * cx];
        LPBYTE pbSrcRGB = (LPBYTE)&((DWORD*)pGlassBits)[j * cx];

        for (int i = 0; i < cx; ++i)
        {
            pbSrcRGB[0]=(pbDestRGB[0] * a +
                    pbSrcRGB[0] * alpha)>>8;
            pbSrcRGB[1]=(pbDestRGB[1] * a +
                    pbSrcRGB[1] * alpha)>>8;
            pbSrcRGB[2]=(pbDestRGB[2] * a +
                    pbSrcRGB[2] * alpha)>>8;
            pbSrcRGB += 4;
            pbDestRGB += 4;
        }
    }
    
    BitBlt(dcDest, x, y, cx, cy, dc, 0, 0, SRCCOPY);
     
	SelectObject(dc, dcOld);
    DeleteDC(dc);

    DeleteObject(hbmGlass);    
    DeleteObject(hbmDest);    
    
    return true;
}
    

void FillRectGlass(HDC hDC,RECT* rcArea,COLORREF crFill, BOOL IsVer /*=FALSE*/,int alpha /*=40*/)
{
	float f0 = (float)alpha/(float)255;
    float f1 = 1.0f-f0; 
 
	//用RGB(255,255,255)与crFill做透明运算
    COLORREF crFill0 = RGB(GetRValue(crFill)*f1+f0*255,GetGValue(crFill)*f1+f0*255,GetBValue(crFill)*f1+f0*255);
  
	//用RGB(192,192,192)与crFill做透明运算
	COLORREF crFill1 = RGB(GetRValue(crFill)*f1+f0*192,GetGValue(crFill)*f1+f0*192,GetBValue(crFill)*f1+f0*192);

    if (IsVer)
    {
		RECT rc = *rcArea;
		int w = RectWidth(rc)/2;
		rc.right = rc.left+w;
		FillRect(hDC,rc,crFill0);
		
		rc.left = rc.right;
		rc.right = rcArea->right;
		FillRect(hDC,rc,crFill1);
    } 
    else
    {
		RECT rc = *rcArea;
		int32 h = RectHeight(rc)/2;
		
		//::InflateRect(&rc,-1,-1);
		rc.bottom = rc.top+h;
		FillRect(hDC,rc,crFill0);
		
		rc.top = rc.bottom;
		rc.bottom = rcArea->bottom;
		FillRect(hDC,rc,crFill1);
    }
};

void FillRectGlass(HDC hDC,RECT& rcArea,COLORREF crFill, BOOL IsVer/*=FALSE*/,int alpha /*=40*/){
	FillRectGlass(hDC,&rcArea,crFill,IsVer,alpha);
}
} // namespace VISUALSPACE

