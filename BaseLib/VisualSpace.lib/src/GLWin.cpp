// GLWin.cpp: implementation of the CGLWin class.
//
//////////////////////////////////////////////////////////////////////

#include "GLWin.h"


namespace VISUALSPACE{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CGLWin::CGLWin()
{
  m_RC = NULL;
  m_DC = NULL;
  m_PauseRender = false;
}

CGLWin::~CGLWin()
{

}
	
bool CGLWin::Activation(){ 
	m_PauseRender = false;
	return true;
}
	
void CGLWin::Dead(){ 
	m_PauseRender = true;
}

void CGLWin::IdleRender(){
	::SetWindowPos(GetHwnd(),NULL,0,0,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_FRAMECHANGED);
	//Invalidate();
}

void  CGLWin::Draw3D(float32* ParentMatrix/*= NULL*/,ePipeline* Pipe /*= NULL*/){
	for(uint32 i=0; i<m_ChildList.size(); i++){
		CVSpace3* Space = (CVSpace3*)m_ChildList[i];	
		Space->Draw3D(ParentMatrix,Pipe);
	}
};

bool CGLWin::InitScene(){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// This Will Clear The Background Color To Black
	glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
    return true;
}

void CGLWin::CloseScene(){
	wglDeleteContext(m_RC); 
}

LRESULT CGLWin::Reaction(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT){
		OnPaint(wParam,lParam);
	}
	else if(message == WM_SIZE){
		return OnSize(wParam,lParam);
	}else if (message == WM_ERASEBKGND){
		return 1;
    }else if (message == WM_CREATE){
		OnCreate(wParam,lParam);
    }else if (message == WM_DESTROY)
    {
		CloseScene();
    }else{ 
		return CWinSpace2::Reaction(message,wParam,lParam);
	}

	return 0;
}

LRESULT CGLWin::OnCreate(WPARAM wParam, LPARAM lParam){
	HWND hWnd = GetHwnd();	
	
	HDC hDC = GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 
	    sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
	    1,                                // version number 
	    PFD_DRAW_TO_WINDOW |              // support window 
	    PFD_SUPPORT_OPENGL |              // support OpenGL 
	    PFD_DOUBLEBUFFER,                 // double buffered 
	    PFD_TYPE_RGBA,                    // RGBA type 
	    16,                               0, 0,                 // color bits ignored 
	    0,                                // no alpha buffer 
	    0,                                // shift bit ignored 
	    0,                                // no accumulation buffer 
	    0, 0, 0, 0,                       // accum bits ignored 
	    16,                               // 32-bit z-buffer     
	    0,                                // no stencil buffer 
	    0,                                // no auxiliary buffer 
	    PFD_MAIN_PLANE,                   // main layer 
	    0,                                // reserved 
	    0, 0, 0                           // layer masks ignored 
	}; 
	int  iPixelFormat = ChoosePixelFormat(hDC, &pfd); 
 
	// get the device context's best, available pixel format match 
	if(iPixelFormat == 0)
	{
		MessageBox(NULL, _T("ChoosePixelFormat Failed"), NULL, MB_OK);
		return -1;
	}
	 
	// make that match the device context's current pixel format 
	if(SetPixelFormat(hDC, iPixelFormat, &pfd) == FALSE)
	{
		MessageBox(NULL, _T("SetPixelFormat Failed"), NULL, MB_OK);
		return -1;
	}

	m_DC = hDC;
	
	m_RC = wglCreateContext(hDC);

	if(m_RC==NULL)return -1;
	   
	wglMakeCurrent(m_DC, m_RC);
    if (!InitScene())
    {
		MessageBox(NULL, _T("Init 3D Scene Failed"), NULL, MB_OK);
		return -1;
    }
	wglMakeCurrent(NULL, NULL);

	return 0;
};

LRESULT CGLWin::OnPaint(WPARAM wParam, LPARAM lParam){
	  if(m_PauseRender){
		   SLEEP_MILLI(20);
		   return ::DefWindowProc(GetHwnd(),WM_PAINT,wParam,lParam);
	  }

 	  wglMakeCurrent(m_DC, m_RC);
      Draw3D(NULL,NULL);
	  glFinish();
	  SwapBuffers(m_DC);
	  wglMakeCurrent(NULL,NULL);
	 
	  PAINTSTRUCT ps;				
	  HDC hDC = BeginPaint(m_hWnd, &ps);

	  if(m_State & SPACE_SHOWWINBORDER){
		 RECT rcWin; 
		 GetClientRect(m_hWnd,&rcWin);
		 DrawEdge(hDC,rcWin,m_crWinBorder);// Border
	  }	

	  ::EndPaint(m_hWnd, &ps);


	  return 0;
};

LRESULT CGLWin::OnSize(WPARAM wParam, LPARAM lParam){
	
	int width  = LOWORD(lParam);
	int height = HIWORD(lParam);

	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}
    wglMakeCurrent(m_DC, m_RC);

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	

	Layout(true);

	wglMakeCurrent(NULL,NULL);
	Invalidate(TRUE);

	return 0;
};

}//namespace VISUALSPACE