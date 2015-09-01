// CTGLWin.cpp: implementation of the CTGLWin class.
//
//////////////////////////////////////////////////////////////////////
#include "TGLWin.h"

namespace VISUALSPACE{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTGLWin::CTGLWin()
:
 m_LocalNerve(NULL)
{
  
}

CTGLWin::~CTGLWin()
{

}

bool CTGLWin::InitScene(){

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	return true;
};


void CTGLWin::GLReaction(){
	SpaceRectionMsg SRM;

	if (GetLocalMessage(SRM))
	{
		if (SRM.Msg == WM_SIZE)
		{
			DWORD lParam = (DWORD)SRM.lParam;
			OnGLSceneSize(LOWORD(lParam),HIWORD(lParam));
		}
	}
};

void CTGLWin::OnGLSceneSize(int width, int height)	// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.01f,200.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
};


LRESULT CTGLWin::Reaction(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SIZE)
	{
		return OnSize(wParam,lParam);
	}else if (message == WM_ERASEBKGND){
		return 1;
    }else{ 
		return CWinSpace2::Reaction(message,wParam,lParam);
	}
}

bool CTGLWin::Do(Energy* E){
	HWND hWnd = GetHwnd();
	if(hWnd==NULL)return false;
	
	
	HDC hDC = GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 
	    sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
	    1,                                // version number 
	    PFD_DRAW_TO_WINDOW |              // support window 
	    PFD_SUPPORT_OPENGL |              // support OpenGL 
	    PFD_DOUBLEBUFFER,                 // double buffered 
	    PFD_TYPE_RGBA,                    // RGBA type 
	    16,                               // 24-bit color depth 
	    0, 0, 0, 0, 0, 0,                 // color bits ignored 
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
		return false;
	}
	 
	// make that match the device context's current pixel format 
	if(SetPixelFormat(hDC, iPixelFormat, &pfd) == FALSE)
	{
		MessageBox(NULL, _T("SetPixelFormat Failed"), NULL, MB_OK);
		return false;
	}
	
	HGLRC hglrc = wglCreateContext(hDC);

	wglMakeCurrent(hDC, hglrc);

    if (!InitScene())
    {
		MessageBox(NULL, _T("Init OpenGl Failed"), NULL, MB_OK);
		return false;
    }
	
	RECT rc;
	::GetClientRect(GetHwnd(),&rc);
	OnGLSceneSize(RectWidth(rc),RectHeight(rc));
	

	while(m_Alive)
	{
		// Select first Window as the rendering context and draw to it
		GLReaction();
		//hDC = GetDC(hWnd);
		//wglMakeCurrent(hDC, hglrc);
		RenderScene();
		SwapBuffers(hDC);
		//wglMakeCurrent(NULL, NULL); 
	    //ReleaseDC (hWnd, hDC) ; 	
		Sleep(10); 	
	};
	ReleaseDC (hWnd, hDC) ; 	

	wglMakeCurrent(NULL, NULL); 
	// Cleanup
	wglDeleteContext(hglrc); 

	return false;	
}

void CTGLWin::RenderScene(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix
	glTranslatef(0,0.0f,-6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
	glBegin(GL_TRIANGLES);								// Start Drawing A Triangle
		glColor3f(1.0f,0.0f,0.0f);						// Red
		glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Front)
		glColor3f(0.0f,1.0f,0.0f);						// Green
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Left Of Triangle (Front)
		glColor3f(0.0f,0.0f,1.0f);						// Blue
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Right Of Triangle (Front)
		glColor3f(1.0f,0.0f,0.0f);						// Red
	glEnd();											// Done Drawing The Pyramid
	
};

} //namespace VISUALSPACE