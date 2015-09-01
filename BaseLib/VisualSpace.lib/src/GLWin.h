/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _GLWIN_H__
#define _GLWIN_H__

#include "PhysicSpace.h"
#include "VisibleSpace.h"
#include "WinSpace2.h"
#include <gl\gl.h>
#include <gl\glu.h>
//#include <gl\glaux.h>

namespace VISUALSPACE{

class CGLWin : public CWinSpace2
{
protected:
	HDC   m_DC;
	HGLRC m_RC;
	BOOL  m_PauseRender;
public:
	CGLWin();
	virtual ~CGLWin();

	virtual bool InitScene();
	virtual void Draw3D(float32* ParentMatrix=NULL,ePipeline* Pipe = NULL);
	virtual void CloseScene();

	bool Activation();
	void IdleRender();
	void Dead();

protected:
	virtual void Layout(bool Redraw){};
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);

};

} //namespace VISUALSPACE

#endif // _GLWIN_H__
